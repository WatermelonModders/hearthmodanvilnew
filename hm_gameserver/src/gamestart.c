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

int game_start(struct conn_client_s *c)
{
    struct powerhistory_entity_s *full, *mana_card = NULL;
    struct powerhistory_game_entity_s *ge1, *ge2, *ge_main;
    struct powerhistory_tag_s *p1_tags, *p2_tags, *game_tags, *card_tags;
    struct powerhistory_player_s *players = NULL;
    struct powerhistory_creategame_s *game_start;
    struct powerhistory_data_s *data = NULL;
    struct powerhistory_s *ph;
    struct packet_s *p, *p1;
    const struct ent_s *ent1, *ent2, *ent_game, *ent_nocard;
    char out[8192], *ptr;
    int i;

    ent1 = find_entity("PLAYER1", 7);
    ent2 = find_entity("PLAYER2", 7);

    if(ent1 == NULL || ent2 == NULL) {
        return -1;
    }

    p1_tags = default_tags(NULL, ent1, 0);
    p2_tags = default_tags(NULL, ent2, 0);
    add_tag(&p1_tags, ENTITY_ID, 2);
    add_tag(&p2_tags, ENTITY_ID, 3);
    add_tag(&p1_tags, CONTROLLER, 1);
    add_tag(&p2_tags, CONTROLLER, 2);
    add_game_entity(&ge1, p1_tags, 2);
    add_game_entity(&ge2, p2_tags, 3);

    struct hs_holder_s *holder;
    struct deck_s *deck;
    holder = c->data;

    add_player(&players, 1, 0x200000257544347, holder->deck->bnet_hi, 0, ge1);
    add_player(&players, 2, 0x200000257544347, holder->opponent.deck->bnet_hi, 0, ge2);

    //add_player(&players, 1, 0x200000257544347, 0x4ab3967, 0, ge1);
    //add_player(&players, 2, 0x200000257544347, 0x49E8Eb5, 0, ge2);

    ent_game = find_entity("GAME_ENTITY", 11);
    game_tags = default_tags(NULL, ent_game, 0);
    add_tag(&game_tags, ENTITY_ID, 1);
    add_game_entity(&ge_main, game_tags, 1);

    add_game_start(&game_start, ge_main, players);

    add_ph_data(&data, NULL, game_start);

    //prepare_decks(c);
    //setup_cards(c);


    assert(holder->deck != holder->opponent.deck);

    ent_nocard = find_entity("NO_CARD", 7);
    assert(ent_nocard);

    deck = holder->deck;
    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HERO, FLAG_ISSET) || flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) {
            continue;
        }

        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET)) {
            assert(deck->cards[i]->entity);
            card_tags = default_tags(deck->cards[i], deck->cards[i]->entity, 0);
            add_tag(&card_tags, ZONE, ZONE_HAND);
            add_tag(&card_tags, ZONE_POSITION, deck->cards[i]->zone_position);
        } else {
            card_tags = default_tags(NULL, ent_nocard, 0);
        }

        add_tag(&card_tags, CONTROLLER, deck->controller);
        add_tag(&card_tags, ENTITY_ID, deck->cards[i]->id);

        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET)) {
            add_full_entity(&full, card_tags, deck->cards[i]->id, deck->cards[i]->entity->name, deck->cards[i]->entity->nname);
        } else {
            add_full_entity(&full, card_tags, deck->cards[i]->id, NULL, 0);
        }

        if(deck->cards[i]->id == 68) {	
            mana_card = full;
        } else {
            add_ph_data(&data, full, NULL);
        }
    }

    deck = holder->opponent.deck;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HERO, FLAG_ISSET) || flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) {
            continue;
        }

        card_tags = NULL;
        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET)) {
            add_tag(&card_tags, ZONE, ZONE_HAND);
            add_tag(&card_tags, ZONE_POSITION, deck->cards[i]->zone_position);
        } else {
            add_tag(&card_tags, ZONE, ZONE_DECK);
        }
        add_tag(&card_tags, CONTROLLER, deck->controller);
        add_tag(&card_tags, ENTITY_ID, deck->cards[i]->id);

        add_full_entity(&full, card_tags, deck->cards[i]->id, NULL, 0);

        if(deck->cards[i]->id == 68) {	
            mana_card = full;
        } else {
            add_ph_data(&data, full, NULL);
        }
    }

    deck = holder->deck;
    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HERO, FLAG_ISSET) || flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) {
            card_tags = NULL;
            card_tags = default_tags(deck->cards[i], deck->cards[i]->entity, 0);
            add_tag(&card_tags, CONTROLLER, deck->controller);
            add_tag(&card_tags, ENTITY_ID, deck->cards[i]->id);
            add_tag(&card_tags, ZONE, 1);

            if(flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) add_tag(&card_tags, CREATOR, deck->hero->id);

            add_full_entity(&full, card_tags, deck->cards[i]->id, deck->cards[i]->entity->name, deck->cards[i]->entity->nname);
            add_ph_data(&data, full, NULL);
        }

    }

    deck = holder->opponent.deck;
    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HERO, FLAG_ISSET) || flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) {
            card_tags = NULL;
            card_tags = default_tags(deck->cards[i], deck->cards[i]->entity, 0);
            add_tag(&card_tags, CONTROLLER, deck->controller);
            add_tag(&card_tags, ENTITY_ID, deck->cards[i]->id);
            add_tag(&card_tags, ZONE, 1);

            if(flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) add_tag(&card_tags, CREATOR, deck->hero->id);

            add_full_entity(&full, card_tags, deck->cards[i]->id, deck->cards[i]->entity->name, deck->cards[i]->entity->nname);
            add_ph_data(&data, full, NULL);
        }

    }

    /*
       ent_nocard = find_entity("HERO_08", 7);
       card_tags = default_tags(ent_nocard);
       add_tag(&card_tags, CONTROLLER, 1);
       add_tag(&card_tags, ENTITY_ID, 64);
       add_full_entity(&full, card_tags, 64, "TU4e_001", 8);
       add_ph_data(&data, full, NULL);

       ent_nocard = find_entity("HERO_09", 7);
       card_tags = default_tags(ent_nocard);
       add_tag(&card_tags, CONTROLLER, 2);
       add_tag(&card_tags, ENTITY_ID, 66);
       add_full_entity(&full, card_tags, 66, "HERO_09", 7);
       add_ph_data(&data, full, NULL);
       */

    /*
       ent_nocard = find_entity("CS2_034", 7);
       card_tags = default_tags(ent_nocard);
       add_tag(&card_tags, CONTROLLER, 1);
       add_tag(&card_tags, ENTITY_ID, 65);
       add_full_entity(&full, card_tags, 65, "CS2_034", 7);
       add_ph_data(&data, full, NULL);

       ent_nocard = find_entity("CS1h_001", 8);
       card_tags = default_tags(ent_nocard);
       add_tag(&card_tags, CONTROLLER, 2);
       add_tag(&card_tags, ENTITY_ID, 67);
       add_full_entity(&full, card_tags, 67, "CS1h_001", 8);
       add_ph_data(&data, full, NULL);
       */

    /*
       ent_nocard = find_entity("GAME_005", 8);
       card_tags = default_tags(ent_nocard);
       add_tag(&card_tags, CONTROLLER, 1);
       add_tag(&card_tags, ENTITY_ID, 68);


       if(holder->id == 1) {
       add_tag(&card_tags, CREATOR, 1);
       add_tag(&card_tags, CARDTYPE, 5);
       add_full_entity(&full, card_tags, 68, "GAME_005", 8);
       } else {
       add_full_entity(&full, card_tags, 68, NULL, 0);
       }
       add_ph_data(&data, full, NULL);
       */

    add_ph_data(&data, mana_card, NULL);

    add_ph(&ph, data);
    add_packet(&p, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    ptr = out;
    int n = serialize(p, &ptr, ptr + sizeof(out));
    packet_free(p);

    data = NULL;
    ph = NULL;
    add_tagschange(&data, ent_game, 1);
    add_tagchange(&data, STEP, 4, 1);

    add_tagschange(&data, ent1, 2);
    add_tagchange(&data, CONTROLLER, 1, 2);
    add_tagchange(&data, MULLIGAN_STATE, 1, 2);

    add_tagschange(&data, ent2, 3);
    add_tagchange(&data, CONTROLLER, 2, 3);

    add_powerstart(&data, 5, -1, 1, 0, NULL, 0);

    add_ph(&ph, data);
    add_packet(&p1, ph, P_POWERHISTORY);

    ptr = out + n;
    int n1 = serialize(p1, &ptr, ptr + sizeof(out) - n);
    packet_free(p1);

    struct entitychoices_s *ec;
    ec = prepare_choices(holder->deck, holder->id);
    add_packet(&p1, ec, P_ENTITYCHOICES);

    ptr = out + n + n1;
    int n2 = serialize(p1, &ptr, ptr + sizeof(out) - n - n1);
    packet_free(p1);

    //bin_dump(ptr - n2, n2);

    if(holder->id == 1) {
        ec = prepare_choices(holder->opponent.deck, 2);
    } else {
        ec = prepare_choices(holder->opponent.deck, 1);
    }
    add_packet(&p1, ec, P_ENTITYCHOICES);
    ptr = out + n + n1 + n2;
    int n3 = serialize(p1, &ptr, ptr + sizeof(out) - n - n1 - n2);
    packet_free(p1);

    //bin_dump(ptr - n3, n3);
    const char common[19] = { 0x13, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x0a, 0x09, 0x22, 0x07, 0x08, 0x03, 0x10, 0xb1, 0x02, 0x18, 0x01 };
    memcpy(out + n + n1 + n2 + n3, common, sizeof(common));

    hm_send(c, out, (int)(n + n1 + n2 + n3 + sizeof(common)));

    return 0;
}
