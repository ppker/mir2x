""" check lua syntax with: luac -p <filename>
"""

import os
import sys
import subprocess
import re
from tempfile import mkstemp


def create_lua_tmpfile(s):
    fd, path = mkstemp()
    with os.fdopen(fd, 'w') as f:
        f.write(s)
    return path


def check_lua_str(s):
    path = create_lua_tmpfile(s)
    subprocess.run(['luac', '-p', path], check=True)

check_lua_str('reture')
