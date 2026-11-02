#!/usr/bin/env python3
"""Отчёт по бюджету памяти рома: IWRAM, EWRAM и ROM.

usage: memory_report.py [ELF]   (по умолчанию build/balatro-gba.elf)

Нужен arm-none-eabi-size: берётся из $DEVKITARM, иначе запускается в образе проекта
через `docker compose run gbalatro`.
"""
import os
import subprocess
import sys

LIMITS = {'IWRAM': 32 * 1024, 'EWRAM': 256 * 1024, 'ROM': 32 * 1024 * 1024}
REGION_BASE = {'IWRAM': 0x03000000, 'EWRAM': 0x02000000, 'ROM': 0x08000000}


def sections(elf, repo):
    if os.environ.get('DEVKITARM'):
        tool = os.path.join(os.environ['DEVKITARM'], 'bin', 'arm-none-eabi-size')
        out = subprocess.run([tool, '-A', elf], capture_output=True, text=True, check=True).stdout
    else:
        out = subprocess.run(['docker', 'compose', 'run', '--rm', '-T', 'gbalatro',
                              '/opt/devkitpro/devkitARM/bin/arm-none-eabi-size', '-A',
                              os.path.relpath(elf, repo)],
                             cwd=repo, capture_output=True, text=True, check=True).stdout
    res = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1].isdigit() and parts[2].isdigit():
            res.append((parts[0], int(parts[1]), int(parts[2])))
    return res


def main():
    repo = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
    elf = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else
                          os.path.join(repo, 'build', 'balatro-gba.elf'))
    used = {k: 0 for k in LIMITS}
    for name, size, addr in sections(elf, repo):
        if size == 0 or name.startswith('.debug') or name in ('.comment', '.ARM.attributes'):
            continue
        for region, base in REGION_BASE.items():
            if base <= addr < base + LIMITS[region]:
                used[region] += size
                break
    print('| Память | Занято | Всего | % |')
    print('|---|---|---|---|')
    for region, limit in LIMITS.items():
        print(f'| {region} | {used[region]} | {limit} | {100 * used[region] / limit:.1f} |')
    print()
    print(f'Свободно IWRAM под стек: {LIMITS["IWRAM"] - used["IWRAM"]} байт')


if __name__ == '__main__':
    main()
