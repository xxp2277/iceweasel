# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import absolute_import, print_function

import os
import sys
import shutil
import subprocess


def make_7z(source, suffix, package):
    dist_source = source + suffix
    os.rename(source, dist_source)
    user = os.environ.get('LIBPORTABLE_PATH')
    if user:
        if (suffix == '_x64'):
            path = shutil.copy(user + '/bin/portable64.dll', dist_source)
        else:
            path = shutil.copy(user + '/bin/portable32.dll', dist_source)
        path = shutil.copy(user + '/bin/portable(example).ini', dist_source)
        path = shutil.copy(user + '/bin/upcheck.exe', dist_source)
    subprocess.check_call(['7z', 'a', '-t7z', package, dist_source, '-mx9', '-r', '-y', '-x!.mkdir.done'])
    os.rename(dist_source, source)
    if user:
        os.remove(source + '/portable(example).ini')

def main(args):
    if len(args) != 3:
        print('Usage: make_7z.py <source> <suffix> <package>',
              file=sys.stderr)
        return 1
    else:
        make_7z(args[0], args[1], args[2])
        return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
