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
import os

def fwrite(f, content):
    f = open(f, "w")
    f.write(content)
    f.close()

def parse():
    f = open("./flags", "r")
    lines = f.readlines();
    f.close()

    f = open("./flags_sections", "r")
    lines = lines + f.readlines()
    f.close()

    ae = []
    for l in lines:

        elements = []
        for ele in l.split(","):
            elements.append(ele.replace(" ", "").replace("\n", ""))

        ae.append(elements)

    header = '#ifndef FLAG_DEF_H_\n#define FLAG_DEF_H_\n\n'

    header += '#define MAX_FLAGS_DEF    %d\n\n' % len(ae)
    header += 'int flag_defs[MAX_FLAGS_DEF];\n'
    header += 'enum flags_e {\n'
    i = 0
    for a in ae:
        header += '\t%s = %s,\n' % (a[0], i)
        i = i + 1
    header += '};\n'
    header += '#endif'

    body = '#include <flag_def.h>\n\n'
    body += 'int flag_defs[MAX_FLAGS_DEF] = {\n'
    i = 0
    for a in ae:
        body += '%d, // index %d\n' % (int(a[1]), i)
        i = i + 1
    body += '};\n'

    fwrite("../src/include/flag_def.h", header)
    fwrite("../src/flag_def.c", body)

if __name__ == '__main__':
    e = parse()
