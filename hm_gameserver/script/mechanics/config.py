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
RF_SECTION = 1

no_levelup = 0

file_entities           = "../../src/proto/ent.c"
file_flags_sections     = "../flags_sections"
file_ent_sections       = "../../src/proto/ent_sections.c"
file_levelup_c          = "../../src/levelup.c"
file_levelup_h          = "../../src/include/levelup.h"

sections = {
    "buff": ["./include/section_buff.h", "BUFF", []],
    "battlecry": ["./include/section_battlecry.h", "BATTLECRY", []],
    "defender": ["./include/section_defender.h", "DEFENDER", []],
    "spell_aoe": ["./include/section_spell_aoe.h", "SPELL_AOE", []],
    "spell": ["./include/section_spell.h", "SPELL", []],
    "attack": ["./include/section_attack.h", "ATTACK", []],
    "attack_effect": ["./include/section_attack_effect.h", "ATTACK_EFFECT", []],
    "weapon": ["./include/section_weapon.h", "WEAPON", []],
    "endturn": ["./include/section_endturn.h", "ENDTURN", []],
    "onboard": ["./include/section_onboard.h", "ONBOARD", []],
    "destroyed": ["./include/section_destroyed.h", "DESTROYED", []],
    "weapon_destroyed": ["./include/section_weapon_destroyed.h", "WEAPON_DESTROYED", []],
    "idle": ["./include/section_idle.h", "IDLE", []],
    "weapon_attack": ["./include/section_weapon_attack.h", "WEAPON_ATTACK", []],
    "attachment_attack": ["./include/section_attachment_attack.h", "ATTACHMENT_ATTACK", []],
    "attack_effect_attachment": ["./include/section_attack_effect_attachment.h", "ATTACK_EFFECT_ATTACHMENT", []],
}
