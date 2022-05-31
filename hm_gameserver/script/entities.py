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
import sys
import shlex

allflags = []
index = 0

def parse(src):
    global allflags
    global index
    entities = []
    f = open(src, "r")

    ri = 0

    lines = f.readlines();
    for l in lines:
        ri = ri + 1
        if(len(l) < 10):        # new lines
            continue
        if(l[0] == "#"):	# comment
            continue

        arr = shlex.split(l)

        f = ''
        att = ''
        custom = ''
        c = ''
        t = ''
        level2 = ''
        level3 = ''

        for a in arr:
            sub = a.split("=")
            if(len(sub) != 2):
                continue

            if(sub[0] == "name"):
                n = sub[1]
            elif(sub[0] == "card"):
                c = sub[1]
            elif(sub[0] == "tags"):
                t = sub[1]
            elif(sub[0] == "flags"):
                f = sub[1]
            elif(sub[0] == "attachment"):
                att = sub[1]
            elif(sub[0] == "custom"):
                custom = sub[1]
            elif(sub[0] == "level2"):
                level2 = sub[1]
            elif(sub[0] == "level3"):
                level3 = sub[1]
            else:
                print 'parsing %s failed' % sub[0]
                exit(1)

        if(len(c) == 0 or len(t) == 0):
            continue

        d = {}

        if(len(level2) > 0):
            d['level2'] = level2

        if(len(level3) > 0):
            d['level3'] = level3

        d['desc'] = n
        d['name'] = c

        if(len(f) > 0):
            flags = f.replace("\n", "")
            allflags.append([index, flags, d['name']])
            d['rawflags'] = flags

        d['flags'] = '{{0, 0, 0, 0, 0, 0, 0, 0}}'

        if(len(att) > 0):
            d['attachment'] = att

        d['tags'] = {}
        if(len(t) > 0):
            properties = t.replace(",}", "}").split(",")
            for p in properties:
                a = p.split(":")
                idx = a[0].replace("{", "")
                tz = a[1].replace("\n", "").replace("}", "")
                d['tags'][idx] = tz

        if(len(custom) > 0):
            d['custom'] = custom
            print 'Active card: ' + d['desc'] + ' [' + d['name'] + ']'

        entities.append(d)

        index = index + 1
    return entities

def gen_js_cards(e):
    out = 'var cards = [\n'
    for d in e :
        for key in d:
            if(key == 'rawflags'):
                if((d[key].find("CARD_MINION") > -1 or d[key].find("CARD_SPELL") > -1) and\
                    d[key].find("CARD_UNCOLLECTIBLE") == -1 and d[key].find("CARD_HERO") == -1 and\
                    d[key].find("CARD_HEROPOWER") == -1
                    ):
                    out += '["%s", "%s"], \n' % (d['name'], d['desc'])

    out += '];'

    f = open("cards.js", "w")
    f.write(out)
    f.close()

def gen_js_hp(e):
    out = 'var cards_hp = [\n'
    for d in e :
        for key in d:
            if(key == 'rawflags'):
                if(d[key].find("CARD_HEROPOWER") > -1):
                    out += '["%s", "%s"], \n' % (d['name'], d['desc'])

    out += '];'

    f = open("cards_hp.js", "w")
    f.write(out)
    f.close()

def gen_js_hero(e):
    out = 'var cards_hero = [\n'
    for d in e :
        for key in d:
            if(key == 'rawflags'):
                if(d[key] == "(CARD_HERO)"):
                    out += '["%s", "%s"], \n' % (d['name'], d['desc'])

    out += '];'

    f = open("cards_hero.js", "w")
    f.write(out)
    f.close()

def gen(e):
    out = '#include <ent_gen.h>\n'
    out += '#include <ent.h>\n'
    header = '#ifndef ENTGEN_H_\n#define ENTGEN_H_\n\n#define MAX_ENTITIES %d\n' % len(e)
    out += 'struct ent_s entities[MAX_ENTITIES] = {'
    index = 0
    beasts = []
    totems = []
    totemic_call = []
    hp = []

    for d in e:
        flags = ''
        attch = '(void *)0, '
        val = '{'
        level2 = "(void *)0, "
        level3 = "(void *)0, "

        for key in d:
            if(key == 'name'):
                start = '{ "%s", %d, %d, ' % (d[key], len(d[key]), len(d['tags']))
            elif(key == 'desc'):
                desc = '"%s", ' % d[key]
            elif(key == 'level2'):
                level2 = '"%s", ' % d[key]
            elif(key == 'level3'):
                level3 = '"%s", ' % d[key]
            elif(key == 'flags'):
                flags = '%s, ' % d[key]
            elif(key == 'attachment'):
                attch = "\"%s\", " % d[key]
            elif(key == 'rawflags'):
                tt = 1
            elif(key == 'custom'):
                tt = 1
            elif(key == 'tags'):
                for k, v in d[key].items():
                    val += '{ %s, %s },  ' % (k, v)

        val += ' }} // %d \n, ' % index

        out += start + level2 + level3 + desc + flags + attch + val
        index = index + 1

    out += '};'

    out += '\n\nvoid entities_init() {\n'
    for i in allflags:
        arr = i[1].split("|")
        for a in arr:
            cm = a.replace("(", "").replace(")", "")
            if(cm == "CARD_RACE_BEAST"):
                beasts.append(i[2])
            if(cm == "CARD_RACE_TOTEM"):
                totems.append(i[2])
            if(cm == "CARD_TOTEMIC_CALL_CHILD"):
                totemic_call.append(i[2])
            if(cm == "CARD_HEROPOWER"):
                hp.append(i[2])

            out += '\tflag(&entities[%s].flags, %s, FLAG_SET);\n' % (i[0], cm)

    out += '}\n'
    header += "#define MAX_BEASTS %d\n" % len(beasts)
    header += "#define MAX_TOTEMS %d\n" % len(totems)
    header += "#define MAX_TOTEMIC_CALL %d\n" % len(totemic_call)
    header += "#define MAX_HEROPOWER %d\n" % len(hp)

    out += "const char *race_beast[MAX_BEASTS] = {\n"
    for b in beasts:
        out += "\t\"%s\",\n" % b
    out += "};\n"

    out += "const char *race_totems[MAX_TOTEMS] = {\n"
    for b in totems:
        out += "\t\"%s\",\n" % b
    out += "};\n"

    out += "const char *totemic_call[MAX_TOTEMIC_CALL] = {\n"
    for b in totemic_call:
        out += "\t\"%s\",\n" % b
    out += "};\n"

    out += "const char *hp_list[MAX_HEROPOWER] = {\n"
    for b in hp:
        out += "\t\"%s\",\n" % b
    out += "};\n"

    f = open("../src/proto/ent.c", "w")
    f.write(out)
    f.close()

    header += "const char *totemic_call[MAX_TOTEMIC_CALL];\n"
    header += "const char *race_beast[MAX_BEASTS];\n"
    header += "const char *race_totems[MAX_TOTEMS];\n"
    header += "\n#endif"
    f = open("../src/include/proto/ent_gen.h", "w")
    f.write(header)
    f.close()

if __name__ == '__main__':
    basic = parse("custom_mod")
    #classic = parse("cards/classic")

    #e = basic + classic
    e = basic

    gen(e)
    #gen_js_cards(e)
    #gen_js_hp(e)
    #gen_js_hero(e)
    print 'total entities: %d' % len(e)
