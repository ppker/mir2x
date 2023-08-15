""" check lua syntax with: luac -p <filename>
"""

import os
import sys
import subprocess
import re
from tempfile import mkstemp

def parse_lua_substr(s, level):
    pattern = r'(?<=\[' + (level * r'=') + r'\[).+?(?=\]' + (level * r'=') + r'\])'
    return re.findall(pattern, s, re.DOTALL)


def create_lua_tmpfile(s):
    fd, path = mkstemp()
    with os.fdopen(fd, 'w') as f:
        f.write(s)
    return path


def check_lua_str(s, level=0):
    path = create_lua_tmpfile(s)
    subprocess.run(['luac', '-p', path], check=True)

    subslist = parse_lua_substr(s, level)
    if subslist:
        for subs in subslist:
            check_lua_str(subs, level + 1)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: pyluacheck.py <filename>')
        sys.exit(1)

    path = sys.argv[1]
    with open(path) as f:
        s = f.read()
    check_lua_str(s)
