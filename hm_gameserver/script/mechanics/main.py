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
from couchbase.bucket import Bucket

import config, util, target, parser

def start_sections():
    os.system("rm %s" % config.file_flags_sections)

    util.write(config.file_ent_sections, "#include <ent_gen.h>\n#include <ent.h>\n\n", "w")
    util.write(config.file_ent_sections, "void entities_init2() {\n", "a")

    print 'Starting sections:'
    for s in config.sections:
        b = "#ifndef SECTION_%s_H_\n" % config.sections[s][1].upper()
        b += "#define SECTION_%s_H_\n\n" % config.sections[s][1].upper()
        util.write(config.sections[s][0], b, "w")
        print '\t%s' % config.sections[s][0]

    util.write(config.file_levelup_c, "#include <hsl_cardname.h>\n#include <levelup.h>\n\nstruct levelup_s levelup[MAX_LEVELUP] = {\n", "w")

def end_sections():
    print 'Ending sections:'
    for s in config.sections:
        b = '#define __SECTION_%s\\\n' % s.upper()
        for t in config.sections[s][2]:
            b += '\t%s\\\n' % t

        util.write(config.sections[s][0], b, "a")
        b = "\n#endif"
        util.write(config.sections[s][0], b, "a")

        print '\t%s' % config.sections[s][0]

    util.write(config.file_ent_sections, "}\n", "a")

    util.write(config.file_levelup_c, "};", "a")

    lh = "#ifndef LEVELUP_H_\n#define LEVELUP_H_\n\
#define MAX_LEVELUP %d\n\
struct levelup_s {\n\
    const char *dst;\n\
    const char *src;\n\
};\n\
extern struct levelup_s levelup[MAX_LEVELUP];\n\
#endif" % config.no_levelup
    util.write(config.file_levelup_h, lh, "w")

if __name__ == "__main__":
    start_sections()

    rootDir = './cards/'

    p = parser.parser()

    cb = Bucket('couchbase://localhost/hbs', password='aci')
    loaded = cb.get('u:mod_dota').value


    for c in loaded['cards']:
        name = ''
        for k in c:
            if k[0] == '__name':
                name = k[1]
            if k[0] == '__code':        # code comes later than name
                p.parse_card(k[1], name)

    '''
    for c in loaded:
        for r in loaded[c]:
            for i in loaded[c][r]:
                print i

    # files
    for dir_, dirnames, files in os.walk(rootDir):
        dirnames.sort()
        files.sort()
        for fileName in files:
            if(fileName.endswith(".card")):
                relDir = os.path.relpath(dir_, rootDir)
                relFile = os.path.join(relDir, fileName)
                p.parse_card(rootDir + relFile)
    '''

    end_sections()
