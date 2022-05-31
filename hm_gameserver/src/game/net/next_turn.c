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

struct packet_s *net_next_turn(struct conn_client_s *c, struct card_s **drawn_card, int *zoneposition)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;

    p = c->data;

    add_tagchange(&data, STEP, 12, 1);
    add_powerstart(&data, 5, 4, 3, 0, NULL, 0);
    add_tagchange(&data, NEXT_STEP, 16, 1);
    add_powerend(&data);

    add_tagchange(&data, STEP, 16, 1);
    add_powerstart(&data, 5, 5, 3, 0, NULL, 0);
    add_tagchange(&data, NEXT_STEP, 13, 1);
    add_powerend(&data);

    add_tagchange(&data, STEP, 13, 1);
    add_powerstart(&data, 5, -1, 1, 0, NULL, 0);
    add_tagchange(&data, NUM_TURNS_LEFT, 0, p->entity_id);
    add_tagchange(&data, NUM_TURNS_LEFT, 1, p->entity_id);
    add_tagchange(&data, CURRENT_PLAYER, 0, p->entity_id);
    add_tagchange(&data, CURRENT_PLAYER, 1, (p->entity_id == 2 ? 3 : 2));
    add_tagchange(&data, TURN, p->game->turn, 1);
    add_tagchange(&data, NEXT_STEP, 6, 1);
    add_powerend(&data);

    add_tagchange(&data, STEP, 6, 1);
    add_powerstart(&data, 5, 1, 2, 0, NULL, 0);

    /*
       if(p->entity_id == 3) {
       add_tagchange(&data, NUM_TURNS_IN_PLAY, 1, 64);
       add_tagchange(&data, NUM_TURNS_IN_PLAY, 1, 65);
       } else {
       add_tagchange(&data, NUM_TURNS_IN_PLAY, 1, 66);
       add_tagchange(&data, NUM_TURNS_IN_PLAY, 1, 67);
       }
       */

    add_tagchange(&data, RESOURCES_USED, 0, (p->entity_id == 2 ? 3 : 2));
    add_tagchange(&data, RESOURCES, p->deck_copy->mana, (p->entity_id == 2 ? 3 : 2));
    add_tagchange(&data, 399, 0, (p->entity_id == 2 ? 3 : 2));
    add_tagchange(&data, NEXT_STEP, 17, 1);
    add_powerend(&data);

    add_tagchange(&data, STEP, 17, 1);
    add_powerstart(&data, 5, 8, 2, 0, NULL, 0);
    add_tagchange(&data, NEXT_STEP, 9, 1);
    add_powerend(&data);

    add_tagchange(&data, STEP, 9, 1);
    add_powerstart(&data, 5, 0, 2, 0, NULL, 0);

    //card_used_reorder(p->deck, p->held_card);
    if(*drawn_card == NULL) {
        // player who just finished his move
        struct deck_s *deck = p->opponent.deck_copy;

        *drawn_card = draw_random_card(deck, p->entity_id == 2 ? 3 : 2, -1);
        if(*drawn_card) {
            *zoneposition = (*drawn_card)->zone_position;

            hm_log(LOG_DEBUG, lg, "drawn card zone position %d for card: %d", (*drawn_card)->zone_position, (*drawn_card)->id);

            add_tagchange(&data, ZONE, 3, (*drawn_card)->id);
            add_tagchange(&data, ZONE_POSITION, *zoneposition, (*drawn_card)->id);
        } else {
            struct card_s *card = p->opponent.deck_copy->hero;
            assert(card);

            add_powerstart(&data, 8, 0, card->id, 0, NULL, 0);

            add_tagchange(&data, FATIGUE, p->opponent.deck_copy->fatigue, p->entity_id);

            add_tagchange(&data, PREDAMAGE, p->opponent.deck_copy->fatigue, p->entity_id);

            add_tagchange(&data, PREDAMAGE, 0, p->entity_id);

            add_meta(&data, 1, p->opponent.deck_copy->fatigue, card->id);

            add_tagchange(&data, LAST_AFFECTED_BY, 0, card->id);

            add_tagchange(&data, DAMAGE, card->total_health - card->health, card->id);

            add_powerend(&data);

        }
    } else {
        // next player to move
        if(*drawn_card) {
            struct powerhistory_entity_s *show = NULL;
            struct powerhistory_tag_s *card_tags = default_tags(*drawn_card, (*drawn_card)->entity, 0);
            add_full_entity(&show, card_tags, (*drawn_card)->id, (*drawn_card)->entity->name, (*drawn_card)->entity->nname);

            add_tagchange(&data, ZONE_POSITION, *zoneposition, (*drawn_card)->id);
            add_ph_data_show(&data, show);
        }
    }

    if(p->deck_copy->gameover == 1 || p->opponent.deck_copy->gameover == 1) {
        add_tagchange(&data, PLAYSTATE, 3, p->entity_id == 2 ? 3 : 2);   // needed
        add_tagchange(&data, PLAYSTATE, 5, p->entity_id == 2 ? 3 : 2);   // needed
        add_tagchange(&data, PLAYSTATE, 4, p->entity_id);                // needed
    }

    //net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, p->id);

    add_tagchange(&data, EXHAUSTED, 0, p->opponent.deck_copy->heropower->id);

    if(p->opponent.deck_copy->heroweapon) {
        add_tagchange(&data, EXHAUSTED, 0, p->opponent.deck_copy->heroweapon->id);
    }

    add_tagchange(&data, 399, 1, (p->entity_id == 2 ? 3 : 2));
    add_tagchange(&data, 198, 10, 1);
    add_powerend(&data);

    /*
       struct powerhistory_entity_s *show = NULL;
       struct ent_s *ent = find_entity("CS2_119", 7);
       add_full_entity(&show, card_tags, p->entity_id == 2 ? 24 + 30 : 24, "CS2_119", 7);
       add_ph_data_show(&data, show);
       add_tagchange(&data, 263, (p->entity_id == 2 ? 5 : 6), p->entity_id == 2 ? 24 + 30 : 24);
       add_tagchange(&data, 399, 1, (p->entity_id == 2 ? 3 : 2));
       add_tagchange(&data, 198, 10, 1);
       add_powerend(&data);
       */

    add_tagchange(&data, STEP, 10, 1);
    add_powerstart(&data, 5, 2, 2, 0, NULL, 0);
    add_tagchange(&data, NEXT_STEP, 12, 1);

    net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, p->id, NET_NEXTTURN);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    hm_log(LOG_DEBUG, lg, "Next turn %d dump", p->game->turn);
    powerhistory_dump(ph);

    return packet;
}
