'''
hm_gameserver -  hearthmod gameserver
Copyright (C) 2016 Filip Pancik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
'''
import re

import config

def write(f, buf, m):
    l = open(f, m)
    l.write(buf)
    l.close()

def read(f, t):
    f = open(f, "r")
    if(t == 0):
        b = []
        c = f.readlines()
        for l in c:
            if l.startswith('#'):
                continue
            b.append(l)
        return b
    else:
        b = f.read()
    f.close()

    return b

def store_flag(dst, f, i):
    if(f.find("ENABLED") > -1):
        return

    lines = read(config.file_entities, 0)
    for l in lines:
        if(l.find("{ \"%s\"" % i) > -1):
            index = re.findall("// [\w]+", l)[0].split(" ")[1]
            write(dst, "\tflag(&entities[%s].flags, %s, FLAG_SET);\n" % (index, f), "a")
            break
