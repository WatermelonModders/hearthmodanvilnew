/*
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
   */
#include <game.h>

struct entitychoices_s *chooseone_choices(struct card_list_s *defenders, int player_id, int attacker_id)
{
    struct entitychoices_s *ec;
    struct entitychoices_entities_s *etmp;
    struct card_list_s *def;

    ec = malloc(sizeof(*ec));

    ec->id = 3;
    ec->type = 2;
    ec->countmin = 1;
    ec->countmax = 1;
    ec->source = attacker_id;
    ec->player_id = player_id;
    ec->entities = NULL;

    for(def = defenders; def != NULL; def = def->next) {
        etmp = malloc(sizeof(*etmp));
        etmp->entity = def->card->id;
        etmp->next = ec->entities;
        ec->entities = etmp;
    }


    return ec;
}

struct entitychoices_s *prepare_choices(struct deck_s *deck, int player_id)
{
    struct entitychoices_s *ec;
    struct entitychoices_entities_s *etmp;
    int i, drawn = 0;

    ec = malloc(sizeof(*ec));

    ec->id = 1;
    ec->type = 1;
    ec->countmin = 0;
    ec->source = 1;
    ec->player_id = player_id;
    ec->entities = NULL;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET)) {
            etmp = malloc(sizeof(*etmp));
            etmp->entity = deck->cards[i]->id;
            etmp->next = ec->entities;
            ec->entities = etmp;
            drawn++;
        }
    }

    ec->countmax = drawn;

    return ec;
}
