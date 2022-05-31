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
import json, re

import util, target, config

class parser:
    def __init__(self):
        self.allowed = self.allowed_sections()

    def allowed_sections(self):
        loaded = util.read("allowed", 0)

        read = 0
        al = {}

        for a in loaded:
            if a.startswith('#'):
                continue
            elif a.startswith('@'):
                if(a.find('section') > -1):
                    read |= config.RF_SECTION
                continue
            elif a.startswith('\n'):
                read = 0
                continue
            else:
                if((read & config.RF_SECTION) == config.RF_SECTION):
                    n = a.split(":")
                    al[n[0]] = n[1].replace("\n", "")
        return al

    def match_allowed_section(self, dst, src, parent):
        for d in dst:
            cx = src.split(" ")
            if(len(cx) > 1):
                if(cx[1] == d):
                    return [dst[d], src, d]
        return []

    def match_allowed_sub(self, dst, src):
        sub = []
        a = src.replace("\n", "").split(",")
        for d in dst:
            for e in a:
                if(e == d):
                    sub.append([e, dst[d]])
        return sub

    def card_battlecry_target(self, obj, card):
        type = obj[0]
        target = obj[1]
        print type

    def parse_card(self, b, name):

        '''
        # files
        f = util.read(c, 0)
        b = ''
        for l in f:
            b += l.replace("\n","").replace("\t","")
        '''

        # ignore first line
        b = b[b.find('\n') + 1:b.rfind('\n')]
        b = b.replace("\n","").replace("\t","")

        try:
            j = json.loads(b)
        except:
            print 'parsing json failed %s [%s]' % (name, b)
            exit(1)

        p = []

        for i in j:
            f = self.match_allowed_section(self.allowed, i, j)
            if(len(f) > 0):
                s = self.match_allowed_sub(j[f[1]], f[0])
                p.append([f[2], s])

        #crd = re.findall("[\w]+.card", c)
        #crd = name.split(".")

        for i in p:
            #print i
            for s in i:
                if s != 'battlecry' and i[0] == 'battlecry':
                    util.store_flag(config.file_ent_sections, 'CARD_BATTLECRY', name)

                    for ms in s:
                        if 'target' in ms:
                            for t in target.battlecry[ms[1]]:
                                util.store_flag(config.file_ent_sections, t, name)

                        elif 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")


                elif s != 'weapon' and i[0] == 'weapon':
                    util.store_flag(config.file_ent_sections, 'CARD_SPELL', name)
                    util.store_flag(config.file_ent_sections, 'CARD_AOE', name)
                    util.store_flag(config.file_ent_sections, 'CARD_WEAPON', name)
                    util.store_flag(config.file_ent_sections, 'CARD_NONTARGET', name)

                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")


                elif s != 'endturn' and i[0] == 'endturn':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")


                elif s != 'spell_aoe' and i[0] == 'spell_aoe':
                    util.store_flag(config.file_ent_sections, 'CARD_SPELL', name)
                    util.store_flag(config.file_ent_sections, 'CARD_AOE', name)

                    for ms in s:
                        if 'target' in ms:
                            for t in target.regular[ms[1]]:
                                util.store_flag(config.file_ent_sections, t, name)

                        elif 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")

                elif s != 'spell' and i[0] == 'spell':
                    util.store_flag(config.file_ent_sections, 'CARD_SPELL', name)
                    for ms in s:
                        if 'target' in ms:
                            for t in target.regular[ms[1]]:
                                util.store_flag(config.file_ent_sections, t, name)

                        elif 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")

                elif s != 'attack' and i[0] == 'attack':
                    util.store_flag(config.file_ent_sections, 'CARD_MINION', name)
                    util.store_flag(config.file_ent_sections, 'CARD_TARGETING', name)

                    for ms in s:
                        if 'target' in ms:
                            for t in target.regular[ms[1]]:
                                util.store_flag(config.file_ent_sections, t, name)

                        elif 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")


                elif s != 'attack_effect' and i[0] == 'attack_effect':
                    util.store_flag(config.file_ent_sections, 'CARD_MINION', name)

                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")

                elif s != 'onboard' and i[0] == 'onboard':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")

                elif s != 'destroyed' and i[0] == 'destroyed':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_defender")

                elif s != 'defender' and i[0] == 'defender':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_defender")

                elif s != 'weapon_destroyed' and i[0] == 'weapon_destroyed':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_weapon")

                elif s != 'idle' and i[0] == 'idle':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_attacker")


                elif s != 'weapon_attack' and i[0] == 'weapon_attack':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_weapon")

                elif s != 'attachment_attack' and i[0] == 'attachment_attack':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_attacker")

                elif s != 'attack_effect_attachment' and i[0] == 'attack_effect_attachment':
                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "fake_attacker")

                elif s != 'properties' and i[0] == 'properties':
                    for ms in s:
                        if 'all' in ms:
                            for mss in ms[1]:
                                util.store_flag(config.file_ent_sections, "CARD_%s" % mss.upper(), name)

                        if 'levelup' in ms:
                            self.levelup(config.file_levelup_c, name, ms[1])
                            #for mss in ms[1]:
                            #    util.store_flag(config.file_ent_sections, "CARD_%s" % mss.upper(), name)

                elif s != 'buff' and i[0] == 'buff':
                    util.store_flag(config.file_ent_sections, 'CARD_BUFF', name)
                    util.store_flag(config.file_ent_sections, 'CARD_SPELL', name)

                    for ms in s:
                        if 'code' in ms:
                            ms[1] = ms[1].replace("$", "HSL_")
                            self.store(name, i[0], ms[1], config.sections[i[0]][0], "attacker")
                        elif 'target' in ms:
                            for t in target.regular[ms[1]]:
                                util.store_flag(config.file_ent_sections, t, name)


    def levelup(self, f, dst, src):
        util.write(f, "\t{%s, \"%s\"},\n" % (src, dst), "a")
        config.no_levelup = config.no_levelup + 1

    def store(self, card, section, code, path, entity):
        #c = re.findall("[\w]+.card", card)
        #c = c[0].split(".")

        if(section == 'endturn'):
            buf = '#define %s_%s\\\n\
            {\\\n\
            %s\\\n\
            }\n\n' % (section, card, code)

        elif(section == 'idle'):
            buf = '#define %s_%s\\\n\
            { %s }\\\n\
            \n\n' % (section, card, code)

        elif(section == 'attack_effect_attachment'):
            buf = '#define %s_%s\\\n\
            {\\\n\
                %s\\\n\
            }\n\n' % (section, card,code)

        elif(section == 'attachment_attack'):
            buf = '#define %s_%s\\\n\
            {\\\n\
            int count = hsl_count_attachments(HSL_fake_deck, NULL, HSL_fake_attacker, "%s");\\\n\
            if(count == 1) {\\\n\
                %s\\\n\
            }}\n\n' % (section, card, card, code)

        else:
            buf = '#define %s_%s\\\n\
            if(HSL_%s && flag(&(HSL_%s->state), %s_%s_FLAG, FLAG_ISSET)) {\\\n\
            %s\\\n\
            }\n\n' % (section, card, entity, entity, section, card, code)


        util.write(path, buf, "a")

        config.sections[section][2].append("%s_%s" % (section, card))

        flag = "%s_%s_FLAG, 1000\n" % (section, card)
        util.write(config.file_flags_sections, flag, "a")

        flag1 = "%s_%s_FLAG" % (section, card)
        util.store_flag(config.file_ent_sections, flag1, card)
