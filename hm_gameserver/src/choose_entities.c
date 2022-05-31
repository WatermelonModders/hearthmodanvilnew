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

/*
   static void summon_obelisk(struct powerhistory_data_s **data, struct deck_s *deck, struct card_s *new_card)
   {
   struct powerhistory_tag_s *card_tags;
   struct powerhistory_entity_s *show;


   add_powerstart(data, 3, -1, new_card->id, 0, NULL, 0);
   int pos = cards_board_last(deck, new_card);
   card_tags = default_tags(new_card, new_card->entity, 0);

   add_tag(&card_tags, CONTROLLER, deck->controller);
   add_tag(&card_tags, ENTITY_ID, new_card->id);

   add_full_entity(&show, card_tags, new_card->id, new_card->entity->name, new_card->entity->nname);
   add_ph_data_full(data, show);

   add_tagchange(data, ZONE_POSITION, pos, new_card->id);
   add_tagchange(data, ZONE, 1, new_card->id);
   add_powerend(data);
   }
   */

static int get_card_start_play(int entity_id, struct deck_s *deck, char *output, const int noutput, int *drawn_card, struct deck_s *opponent, struct card_s ***obelisk, struct hs_holder_s *h)
{
    // FIXME: send this to both players??
    struct card_s *new_card;
    struct powerhistory_tag_s *card_tags;
    struct powerhistory_entity_s *show;

    struct powerhistory_data_s *data = NULL;
    struct powerhistory_s *ph = NULL;
    struct packet_s *p = NULL;

    char *ptr;

    add_tagchange(&data, STEP, 6, 1);
    add_powerstart(&data, 5, 1, entity_id, 0, NULL, 0);
    //add_tagchange(&data, NUM_TURNS_IN_PLAY, 1, 66);			// hero
    //add_tagchange(&data, NUM_TURNS_IN_PLAY, 1, 67);			// hero power
    add_tagchange(&data, RESOURCES, 1, entity_id);
    add_tagchange(&data, 399, 0, entity_id);
    add_tagchange(&data, NEXT_STEP, 17, 1);
    add_powerend(&data);
    add_tagchange(&data, STEP, 17, 1);
    add_powerstart(&data, 5, 8, entity_id, 0, NULL, 0);
    add_tagchange(&data, NEXT_STEP, 9, 1);
    add_powerend(&data);
    add_tagchange(&data, STEP, 9, 1);

    /*
       if((*obelisk)[0] == NULL && (*obelisk)[1] == NULL) {
       (*obelisk)[0] = add_new_card(h->game, deck, -1, "EX1_614", -1);
       (*obelisk)[1] = add_new_card(h->game, opponent, -1, "EX1_614", -1);
       flag(&((*obelisk)[0])->state, CARD_BOARD, FLAG_SET);
       flag(&((*obelisk)[1])->state, CARD_BOARD, FLAG_SET);

       deck->board_position[0] = (*obelisk)[0]->id;
       opponent->board_position[0] = (*obelisk)[1]->id;

       summon_obelisk(&data, deck, (*obelisk)[0]);
       summon_obelisk(&data, opponent, (*obelisk)[1]);
       } else {
       summon_obelisk(&data, opponent, (*obelisk)[0]);
       summon_obelisk(&data, deck, (*obelisk)[1]);
       }

       assert((*obelisk)[0] && (*obelisk)[1]);

*/

    add_powerstart(&data, 5, 0, entity_id, 0, NULL, 0);

    if(*drawn_card != 0) {
        add_tagchange(&data, ZONE_POSITION, 4, *drawn_card);
        add_tagchange(&data, ZONE, ZONE_HAND, *drawn_card);
    } else {
        hm_log(LOG_DEBUG, lg, "about to draw card player %d", entity_id);
        new_card = draw_random_card(deck, entity_id, -1);
        card_tags = default_tags(new_card, new_card->entity, 0);
        add_full_entity(&show, card_tags, new_card->id, new_card->entity->name, new_card->entity->nname);
        add_ph_data_show(&data, show);
        add_tagchange(&data, ZONE, ZONE_HAND, new_card->id);
        add_tagchange(&data, ZONE_POSITION, new_card->zone_position, new_card->id);
        *drawn_card = new_card->id;

        hm_log(LOG_DEBUG, lg, "p2: zone position: %d card: %d", new_card->zone_position, new_card->id);
    }

    add_tagchange(&data, 399, 1, entity_id);
    add_tagchange(&data, NEXT_STEP, 10, 1);
    add_powerend(&data);
    add_tagchange(&data, STEP, 10, 1);
    add_powerstart(&data, 5, 2, entity_id, 0, NULL, 0);
    add_tagchange(&data, NEXT_STEP, 12, 1);
    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&p, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    ptr = output;
    int n = serialize(p, &ptr, ptr + noutput);

    packet_free(p);

    int o = 0;
    // only player needs options
    if(entity_id == 3) {
        o = set_options(deck, opponent, output + n, ptr + noutput - n, 1);
    }

    return n + o;
}

void choose_one_entity(struct conn_client_s *c, struct chooseentities_s *e)
{
    int id = 0;
    struct chooseentities_ent_s *ent;

    for(ent = e->entity; ent != NULL; ent = ent->next) {
        id = ent->entity;
        break;
    }

    assert(id > 0);

    //card = card_get();

    //flag(&card->state, CARD_DECK, FLAG_UNSET);
    //flag(&card->state, CARD_HAND, FLAG_SET);
    //card->zone_position = ++deck->handposition;

}

void choose_entities(struct conn_client_s *c, struct chooseentities_s *e)
{
    struct chooseentities_ent_s *ent;
    struct hs_holder_s *h;
    //struct card_s *unwanted[4];
    int nunwanted = 0;
    //struct card_s *new_card;
    //struct powerhistory_tag_s *card_tags;
    //struct powerhistory_entity_s *show;

    struct powerhistory_data_s *data = NULL, *data1 = NULL;
    struct powerhistory_s *ph = NULL, *ph1 = NULL;
    struct packet_s *p = NULL, *p1 = NULL;
    char out[2048], *ptr, out1[2048], *ptr1;
    int i, cw;

    h = c->data;
    assert(h);


    if(h->id == 1) {
        //max = 4;		// p1 4 cards
        assert(e->nentity <= 4);
    } else {
        //max = 3;		// p2 3 cards
        assert(e->nentity <= 3);
    }

    hm_log(LOG_EMERG, lg, "Unwanted cards");

    for(i = 0; i < h->deck->ncards; i++) {
        if(flag(&h->deck->cards[i]->state, CARD_HAND, FLAG_ISSET)) {

            hm_log(LOG_ALERT, c->log, "player %d hand card: %d zone_position: %d", h->entity_id, h->deck->cards[i]->id, h->deck->cards[i]->zone_position);

            cw = 0;

            for(ent = e->entity; ent != NULL; ent = ent->next) {
                if(ent->entity == h->deck->cards[i]->id) {
                    hm_log(LOG_ALERT, c->log, "breaking with card: %d", h->deck->cards[i]->id);
                    cw = 1;
                    break;
                }
            }

            if(cw == 0 && h->deck->cards[i]->id != 68) {
                //unwanted[nunwanted++] = h->deck->cards[i];

                hm_log(LOG_EMERG, c->log, "player %d unwanted card: %d zone_position: %d", h->entity_id, h->deck->cards[i]->id, h->deck->cards[i]->zone_position);
            }
        }
    }

    add_tagchange(&data, MULLIGAN_STATE, 2, h->entity_id);
    add_tagchange(&data1, MULLIGAN_STATE, 2, h->entity_id);

    add_powerstart(&data, 5, 6, h->entity_id, 0, NULL, 0);
    add_powerstart(&data1, 5, 6, h->entity_id, 0, NULL, 0);

    for(i = 0; i < nunwanted; i++) {

        /*
           new_card = draw_random_card(h->deck, h->entity_id, unwanted[i]->zone_position);

           hm_log(LOG_DEBUG, c->log, "playerID: %d new card: %d position: %d", h->entity_id, new_card->id, new_card->zone_position);

           card_tags = default_tags(new_card, new_card->entity, 0);
           add_full_entity(&show, card_tags, new_card->id, new_card->entity->name, new_card->entity->nname);
           add_ph_data_show(&data, show);

           add_tagchange(&data1, ZONE, 3, new_card->id);

           add_tagchange(&data, ZONE_POSITION, new_card->zone_position, new_card->id);
           add_tagchange(&data1, ZONE_POSITION, new_card->zone_position, new_card->id);

           add_hide(&data, unwanted[i]->id, 2);

           add_tagchange(&data, ZONE, 2, unwanted[i]->id);
           add_tagchange(&data1, ZONE, 2, unwanted[i]->id);

           add_tagchange(&data, ZONE_POSITION, 0, unwanted[i]->id);
           add_tagchange(&data1, ZONE_POSITION, 0, unwanted[i]->id);

           flag(&unwanted[i]->state, CARD_HAND, FLAG_UNSET);
           flag(&unwanted[i]->state, CARD_DECK, FLAG_SET);
           */
    }

    add_tagchange(&data, MULLIGAN_STATE, 3, h->entity_id);
    add_tagchange(&data1, MULLIGAN_STATE, 3, h->entity_id);

    add_powerend(&data);
    add_powerend(&data1);

    add_powerstart(&data, 5, 7, h->entity_id, 0, NULL, 0);
    add_powerstart(&data1, 5, 7, h->entity_id, 0, NULL, 0);

    add_tagchange(&data, MULLIGAN_STATE, 4, h->entity_id);
    add_tagchange(&data1, MULLIGAN_STATE, 4, h->entity_id);

    if(h->id == 1) {
        h->game->flags |= G_P1_CARDS_CHOSEN;
    } else {
        h->game->flags |= G_P2_CARDS_CHOSEN;
    }

    if(EQFLAG(h->game->flags, (G_P1_CARDS_CHOSEN|G_P2_CARDS_CHOSEN))) {
        add_tagchange(&data, NEXT_STEP, 6, 1);
        add_tagchange(&data1, NEXT_STEP, 6, 1);
    }

    add_powerend(&data);
    add_powerend(&data1);

    add_ph(&ph, data);
    add_packet(&p, ph, P_POWERHISTORY);

    add_ph(&ph1, data1);
    add_packet(&p1, ph1, P_POWERHISTORY);

    powerhistory_dump(ph);
    powerhistory_dump(ph1);

    ptr = out;
    int n = serialize(p, &ptr, ptr + sizeof(out));

    packet_free(p);

    ptr1 = out1;
    int n1 = serialize(p1, &ptr1, ptr1 + sizeof(out1));

    packet_free(p1);

    //powerhistory_dump(ph);
    //powerhistory_dump(ph1);

    // p2 draws a card and notifies p1
    if(EQFLAG(h->game->flags, (G_P1_CARDS_CHOSEN|G_P2_CARDS_CHOSEN))) {
        char out2[2048], out3[2048];
        int new_card_id = 0;
        int enc, enc3;
        struct card_s **obelisk;

        obelisk = malloc(sizeof(void *) * 2);
        memset(obelisk, 0, sizeof(void *) * 2);

        if(h->entity_id == 3) {
            enc = get_card_start_play(3, h->deck, out2, sizeof(out2), &new_card_id, h->opponent.deck, &obelisk, h);
            enc3 = get_card_start_play(2, h->opponent.deck, out3, sizeof(out3), &new_card_id, h->deck, &obelisk, h);
        } else {
            enc3 = get_card_start_play(3, h->opponent.deck, out3, sizeof(out3), &new_card_id, h->deck, &obelisk, h);
            enc = get_card_start_play(2, h->deck, out2, sizeof(out2), &new_card_id, h->opponent.deck, &obelisk, h);
        }

        memcpy(out + n, out2, enc);
        memcpy(out1 + n1, out3, enc3);


        hm_send(c, out, n + enc);
        hm_send(h->opponent.client, out1, n1 + enc3);

        register_turntimer(c);
    } else {
        hm_send(c, out, n);
        hm_send(h->opponent.client, out1, n1);
    }

}
