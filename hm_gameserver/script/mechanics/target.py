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
regular = {
"$all" :            ["CARD_TARGET_ALL", "CARD_TARGETING"],
"$minions" :        ["CARD_TARGET_MINIONS", "CARD_TARGETING"],
"$hero" :           ["CARD_TARGET_HERO", "CARD_TARGETING"],
"$enemy_hero" :     ["CARD_TARGET_ENEMY_HERO", "CARD_TARGETING"],
"$enemy_minions" :  ["CARD_TARGET_ENEMY_MINIONS", "CARD_TARGETING"],
"$ally_hero" :      ["CARD_TARGET_ALLY_HERO", "CARD_TARGETING"],
"$ally_minions" :   ["CARD_TARGET_ALLY_MINIONS", "CARD_TARGETING"],
"$ally_bosses" :    ["CARD_TARGET_ALLY_BOSSES", "CARD_TARGETING"],
"$enemy_bosses" :   ["CARD_TARGET_ENEMY_BOSSES", "CARD_TARGETING"],
"$bosses" :         ["CARD_TARGET_BOSSES", "CARD_TARGETING"],
"$ally_board" :     ["CARD_TARGET_ALLY_BOARD", "CARD_TARGETING"],
"$enemy_board" :    ["CARD_TARGET_ENEMY_BOARD", "CARD_TARGETING"],
"$board" :          ["CARD_TARGET_BOARD", "CARD_TARGETING"],
"$ally" :           ["CARD_TARGET_ALLY", "CARD_TARGETING"],
"$enemy" :          ["CARD_TARGET_ENEMY", "CARD_TARGETING"],
"$auto" :           ["CARD_NONTARGET"],
"$undamaged_minions":["CARD_TARGET_UNDAMAGED_MINION", "CARD_TARGETING"],
"$damaged_minions":["CARD_TARGET_DAMAGED_MINION", "CARD_TARGETING"],
"$attack5more_minions":["CARD_TARGET_5ATTACK_MORE", "CARD_TARGETING"],
"$attack3less_minions":["CARD_TARGET_3ATTACK_LESS", "CARD_TARGETING"],
"$health_below_30p_minions":["CARD_TARGET_30PERCENT_LESS", "CARD_TARGETING"],
"$URSA":["CARD_TARGET_URSA", "CARD_TARGETING"],
"$SVEN":["CARD_TARGET_SVEN", "CARD_TARGETING"],
}

battlecry = {
"$all" :            ["CARD_TARGET_ALL", "CARD_TARGETING_BC"],
"$minions" :        ["CARD_TARGET_MINIONS", "CARD_TARGETING_BC"],
"$hero" :           ["CARD_TARGET_HERO", "CARD_TARGETING_BC"],
"$enemy_hero" :     ["CARD_TARGET_ENEMY_HERO", "CARD_TARGETING_BC"],
"$enemy_minions" :  ["CARD_TARGET_ENEMY_MINIONS", "CARD_TARGETING_BC"],
"$ally_hero" :      ["CARD_TARGET_ALLY_HERO", "CARD_TARGETING_BC"],
"$ally_minions" :   ["CARD_TARGET_ALLY_MINIONS", "CARD_TARGETING_BC"],
"$ally" :           ["CARD_TARGET_ALLY", "CARD_TARGETING_BC"],
"$enemy" :          ["CARD_TARGET_ENEMY", "CARD_TARGETING_BC"],
"$ally_bosses" :    ["CARD_TARGET_ALLY_BOSSES", "CARD_TARGETING_BC"],
"$enemy_bosses" :   ["CARD_TARGET_ENEMY_BOSSES", "CARD_TARGETING_BC"],
"$bosses" :         ["CARD_TARGET_BOSSES", "CARD_TARGETING_BC"],
"$ally_board" :     ["CARD_TARGET_ALLY_BOARD", "CARD_TARGETING_BC"],
"$enemy_board" :    ["CARD_TARGET_ENEMY_BOARD", "CARD_TARGETING_BC"],
"$board" :          ["CARD_TARGET_BOARD", "CARD_TARGETING_BC"],
"$auto" :           ["CARD_NONTARGET_BC"],
"$undamaged_minions":["CARD_TARGET_UNDAMAGED_MINION", "CARD_TARGETING_BC"],
"$damaged_minions":["CARD_TARGET_DAMAGED_MINION", "CARD_TARGETING_BC"],
}
