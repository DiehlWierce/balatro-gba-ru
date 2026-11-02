#!/usr/bin/env python3
"""Compares the screenshots of two run.py output directories.

usage: compare.py DIR_A DIR_B

Exit code 0 when every screenshot of DIR_A exists in DIR_B and is pixel-identical, 1 otherwise.
"""
import glob
import os
import sys

from PIL import Image, ImageChops


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    dir_a, dir_b = sys.argv[1:]
    shots = sorted(glob.glob(os.path.join(dir_a, '*.png')))
    different = []
    for a in shots:
        b = os.path.join(dir_b, os.path.basename(a))
        if not os.path.exists(b):
            different.append(os.path.basename(a) + ' (missing)')
            continue
        diff = ImageChops.difference(Image.open(a).convert('RGB'), Image.open(b).convert('RGB'))
        if diff.getbbox() is not None:
            different.append(f'{os.path.basename(a)} (differs in {diff.getbbox()})')
    print(f'{len(shots)} screenshots compared, {len(different)} different')
    for name in different:
        print('  ' + name)
    sys.exit(1 if different or not shots else 0)


if __name__ == '__main__':
    main()
