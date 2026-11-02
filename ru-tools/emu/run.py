#!/usr/bin/env python3
"""Runs a gbarun scenario on build/balatro-gba.gba and saves the screenshots.

usage: run.py SCENARIO OUTDIR [--repo DIR] [--gbarun PATH] [-D NAME=VALUE ...]

Placeholders in the scenario are replaced before running:
  {sym:NAME}      address of the ELF symbol NAME (static symbols too, e.g. s_seed_str)
  {gv:FIELD}      address of g_game_vars.FIELD (e.g. {gv:score}, {gv:money})
  {def:NAME}      VALUE given with -D NAME=VALUE (e.g. -D boss=20)

The ARM toolchain (nm, gcc, objdump) is taken from $DEVKITARM when it is set, otherwise the
commands run in the project's docker image (docker compose run --rm gbalatro).
"""
import argparse
import os
import re
import shlex
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def toolchain_run(repo, cmd):
    """Runs a shell command with the ARM toolchain in PATH, from the repository root."""
    env = dict(os.environ)
    if env.get('DEVKITARM'):
        env['PATH'] = os.path.join(env['DEVKITARM'], 'bin') + os.pathsep + env['PATH']
        argv = ['sh', '-c', cmd]
    else:
        argv = ['docker', 'compose', 'run', '--rm', '-T', 'gbalatro', 'sh', '-c',
                f'export PATH=/opt/devkitpro/devkitARM/bin:$PATH; {cmd}']
    res = subprocess.run(argv, cwd=repo, env=env, capture_output=True, text=True)
    if res.returncode != 0:
        sys.exit(f'toolchain command failed: {cmd}\n{res.stderr}')
    return res.stdout


def elf_symbols(repo):
    out = toolchain_run(repo, 'arm-none-eabi-nm build/balatro-gba.elf')
    syms = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            syms[parts[2]] = int(parts[0], 16)
    return syms


def game_vars_offsets(repo, fields):
    """Offsets of GameVariables fields, computed by the ARM compiler."""
    src = '#include "game_variables.h"\n#include <stddef.h>\n'
    src += 'const unsigned offsets[] = {%s};\n' % ', '.join(
        f'offsetof(GameVariables, {f})' for f in fields)
    os.makedirs(os.path.join(repo, 'build'), exist_ok=True)
    with open(os.path.join(repo, 'build', 'emu_offsets.c'), 'w') as f:
        f.write(src)
    devkitpro = os.environ.get('DEVKITPRO', '/opt/devkitpro')
    out = toolchain_run(repo, ' '.join([
        'arm-none-eabi-gcc -std=gnu23 -mthumb -fms-extensions',
        '-iquote include -iquote include/game -Ibuild',
        f'-I{devkitpro}/libtonc/include -I{devkitpro}/libgba/include',
        '-c build/emu_offsets.c -o "${TMPDIR:-/tmp}/emu_offsets.o" &&',
        'arm-none-eabi-objdump -s -j .rodata "${TMPDIR:-/tmp}/emu_offsets.o"']))
    data = b''
    for line in out.splitlines():
        m = re.match(r'^ [0-9a-f]{4} ((?:[0-9a-f]{2,8} ?){1,4})', line)
        if m:
            data += bytes.fromhex(m.group(1).replace(' ', ''))
    return dict(zip(fields, struct.unpack(f'<{len(fields)}I', data[:4 * len(fields)])))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('scenario')
    ap.add_argument('outdir')
    ap.add_argument('--repo', default=os.path.normpath(os.path.join(HERE, '..', '..')))
    ap.add_argument('--gbarun', default=os.path.join(HERE, 'gbarun'))
    ap.add_argument('-D', dest='defines', action='append', default=[], metavar='NAME=VALUE')
    args = ap.parse_args()

    repo = os.path.abspath(args.repo)
    rom = os.path.join(repo, 'build', 'balatro-gba.gba')
    if not os.path.exists(rom):
        sys.exit(f'{rom} not found, build the ROM first')
    if not os.path.exists(args.gbarun):
        subprocess.run(['cc', '-O2', '-o', args.gbarun, os.path.join(HERE, 'gbarun.c'),
                        '-lmgba', '-lpng'], check=True)

    text = open(args.scenario).read()
    for define in args.defines:
        name, _, value = define.partition('=')
        text = text.replace('{def:%s}' % name, value)
    missing = re.findall(r'\{def:(\w+)\}', text)
    if missing:
        sys.exit('missing -D for: ' + ', '.join(sorted(set(missing))))
    if '{sym:' in text or '{gv:' in text:
        syms = elf_symbols(repo)
        fields = sorted(set(re.findall(r'\{gv:(\w+)\}', text)))
        if fields:
            offsets = game_vars_offsets(repo, fields)
            text = re.sub(r'\{gv:(\w+)\}',
                          lambda m: hex(syms['g_game_vars'] + offsets[m.group(1)]), text)
        text = re.sub(r'\{sym:(\w+)\}', lambda m: hex(syms[m.group(1)]), text)

    os.makedirs(args.outdir, exist_ok=True)
    resolved = os.path.join(args.outdir, 'scenario.txt')
    with open(resolved, 'w') as f:
        f.write(text)
    print('$', ' '.join(shlex.quote(a) for a in [args.gbarun, rom, resolved, args.outdir]))
    subprocess.run([args.gbarun, rom, resolved, args.outdir], check=True)


if __name__ == '__main__':
    main()
