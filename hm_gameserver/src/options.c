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

static int log_enable = 1;

static struct option_s *fill_option(u64 type, u64 id, struct suboption_target_s *targets)
{
    struct option_s *option;

    option = malloc(sizeof(*option));
    memset(option, 0, sizeof(*option));

    option->type = type;

    if(id != -1) {
        option->mainoption = malloc(sizeof(*(option->mainoption)));

        memset(option->mainoption, 0, sizeof(*(option->mainoption)));
        option->mainoption->id = id;
        option->mainoption->target = targets;
    }

    return option;
}

static void add_suboption_target(struct suboption_target_s **so, u64 id)
{
    struct suboption_target_s *s;
    s = malloc(sizeof(*s));

    s->value = id;

    s->next = *so;
    *so = s;
}

int set_options_cards(struct card_list_s *cl, char *buffer, char *endbuffer, int turn)
{
    struct packet_s *packet;
    struct alloptions_s *all;
    struct option_s *option = NULL, *o;
    struct card_list_s *c;

    for(c = cl; c != NULL; c = c->next) {
        o = fill_option(6, c->card->id, NULL);
        o->next = option;
        option = o;
    }

    o = fill_option(2, -1, NULL);
    o->next = option;
    option = o;

    all = malloc(sizeof(*all));

    all->id = turn;
    all->options = option;

    alloptions_dump(all);

    add_packet(&packet, all, P_ALLOPTIONS);

    char *ptr = buffer;
    int n = serialize(packet, &ptr, endbuffer);

    //hm_log(LOG_NOTICE, lg, "set options:");
    //bin_dump(buffer, n);

    return n;

}

static void set_options_board(struct option_s **option, struct card_s *card, struct deck_s *opponent, const int taunt_count)
{
    struct option_s *o;
    int j;
    struct suboption_target_s *so = NULL;

    if(

            (flag(&card->state, CARD_BOARD, FLAG_ISSET) &&
             !flag(&card->state, CARD_DESTROYED, FLAG_ISSET) &&
             !flag(&card->state, CARD_SPELL, FLAG_ISSET)   &&
             !flag(&card->state, CARD_FROZEN, FLAG_ISSET) ) &&
            /*( flag(&card->state, CARD_HERO, FLAG_ISSET) && card->attack > 0) */
            card->attack > 0
      ) {

        if(!flag(&card->state, CARD_TARGETING, FLAG_ISSET) && !flag(&card->state, CARD_TARGETING_BC, FLAG_ISSET)) {
            goto add;
        }

        if(log_enable) hm_log(LOG_NOTICE, lg, "setting targets for card %d", card->id);
        for(j = 0; j < opponent->ncards; j++) {

            if((flag(&opponent->cards[j]->state, CARD_BOARD, FLAG_ISSET) ||
                        flag(&opponent->cards[j]->state, CARD_HERO, FLAG_ISSET) ) &&
                    !flag(&opponent->cards[j]->state, CARD_DESTROYED, FLAG_ISSET)
              ) {

                // if taunt cards on board, does enemy have taunt
                if(taunt_count > 0 && !flag(&card->state, CARD_HEROPOWER, FLAG_ISSET)) {
                    if(flag(&opponent->cards[j]->state, CARD_TAUNT, FLAG_ISSET)) {
                        hm_log(LOG_DEBUG, lg, "\tadding opponent taunt card %d as a target", opponent->cards[j]->id);
                        add_suboption_target(&so, opponent->cards[j]->id);
                    }
                } else {
                    hm_log(LOG_DEBUG, lg, "\tadding opponent card %d as a target", opponent->cards[j]->id);

                    add_suboption_target(&so, opponent->cards[j]->id);
                }
            }
        }

add:
        o = fill_option(3, card->id, so);
        o->next = *option;
        *option = o;
    } /*else {
        hm_log(LOG_ALERT, lg, "Card %d: board %d desttroyed %d frozen %d spell %d", card->id,
        flag(&card->state, CARD_BOARD, FLAG_ISSET),
        flag(&card->state, CARD_DESTROYED, FLAG_ISSET),
        flag(&card->state, CARD_FROZEN, FLAG_ISSET),
        flag(&card->state, CARD_SPELL, FLAG_ISSET)
        );
        } */
}

enum side_e {
    SIDE_PLAYER = 0,
    SIDE_OPPONENT
};

static void calc_targets(struct deck_s *deck, struct card_s *attacker, enum side_e side, struct suboption_target_s **so)
{
    int i;
    struct card_s *defender;
    int debug = 0;

    for(i = 0; i < deck->ncards; i++) {

        defender = deck->cards[i];

        // ignore destroyed, spell, not board and not hero
        if(flag(&defender->state, CARD_SPELL, FLAG_ISSET) ||
                (!flag(&defender->state, CARD_BOARD, FLAG_ISSET) && !flag(&defender->state, CARD_HERO, FLAG_ISSET)) ||
                flag(&defender->state, CARD_DESTROYED, FLAG_ISSET)
          ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", defender->id);
            continue;
        }

        // culling blade
        if(flag(&attacker->state, CARD_TARGET_30PERCENT_LESS, FLAG_ISSET) &&
                (defender->health * 3) > defender->total_health) {
            continue;
        }

        // magic immunity
        if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) &&
                !flag(&attacker->state, CARD_PIERCE_MAGIC_IMMUNITY, FLAG_ISSET) &&
                flag(&defender->state, CARD_DIVINE_SHIELD, FLAG_ISSET)) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // battlecry
        /*
           if(flag(&attacker->state, CARD_TARGET_BC_AUTO, FLAG_ISSET)) {
           if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
           continue;
           }
           */

        // allies
        if(flag(&attacker->state, CARD_TARGET_ALLY, FLAG_ISSET) && side == SIDE_OPPONENT ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // enemies
        if(flag(&attacker->state, CARD_TARGET_ENEMY, FLAG_ISSET) && side == SIDE_PLAYER) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // ally bosses
        if(flag(&attacker->state, CARD_TARGET_ALLY_BOSSES, FLAG_ISSET) &&
                (side == SIDE_OPPONENT || !flag(&defender->state, CARD_BOSS, FLAG_ISSET))
          ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // enemy bosses
        if(flag(&attacker->state, CARD_TARGET_ENEMY_BOSSES, FLAG_ISSET) &&
                (side == SIDE_PLAYER || !flag(&defender->state, CARD_BOSS, FLAG_ISSET))
          ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // bosses
        if(flag(&attacker->state, CARD_TARGET_BOSSES, FLAG_ISSET) &&
                (!flag(&defender->state, CARD_BOSS, FLAG_ISSET))
          ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // board target (exc. heroes)
        if(flag(&attacker->state, CARD_TARGET_BOARD, FLAG_ISSET) &&
                (flag(&defender->state, CARD_HERO, FLAG_ISSET))) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // ally board target (exc. heroes)
        if(flag(&attacker->state, CARD_TARGET_ALLY_BOARD, FLAG_ISSET) &&
                (side == SIDE_OPPONENT || flag(&defender->state, CARD_HERO, FLAG_ISSET))) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // enemy board target (exc. heroes)
        if(flag(&attacker->state, CARD_TARGET_ENEMY_BOARD, FLAG_ISSET) &&
                (side == SIDE_PLAYER || flag(&defender->state, CARD_HERO, FLAG_ISSET))) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // skip non-hero targets if only heroes are valid targets
        if(flag(&attacker->state, CARD_TARGET_HERO, FLAG_ISSET) &&
                !flag(&defender->state, CARD_HERO, FLAG_ISSET)) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // enemy hero
        if(flag(&attacker->state, CARD_TARGET_ENEMY_HERO, FLAG_ISSET) &&
                (side == SIDE_PLAYER || !flag(&defender->state, CARD_HERO, FLAG_ISSET))
          ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // ally hero
        if(flag(&attacker->state, CARD_TARGET_ALLY_HERO, FLAG_ISSET) &&
                (side == SIDE_OPPONENT || !flag(&defender->state, CARD_HERO, FLAG_ISSET))) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // minions
        if(flag(&attacker->state, CARD_TARGET_MINIONS, FLAG_ISSET) &&
                (flag(&defender->state, CARD_HERO, FLAG_ISSET) || flag(&defender->state, CARD_BOSS, FLAG_ISSET))) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // enemy minions
        if(flag(&attacker->state, CARD_TARGET_ENEMY_MINIONS, FLAG_ISSET) &&
                (side == SIDE_PLAYER || flag(&defender->state, CARD_HERO, FLAG_ISSET) || flag(&defender->state, CARD_BOSS, FLAG_ISSET))) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // ally minions
        if(flag(&attacker->state, CARD_TARGET_ALLY_MINIONS, FLAG_ISSET) &&
                (side == SIDE_OPPONENT || flag(&defender->state, CARD_HERO, FLAG_ISSET) || flag(&defender->state, CARD_BOSS, FLAG_ISSET))
          ) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // allies
        if((flag(&attacker->state, CARD_TARGET_ALLY, FLAG_ISSET) && side == SIDE_OPPONENT)) {
            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // undamaged minion eg. backstab
        if(flag(&attacker->state, CARD_TARGET_UNDAMAGED_MINION, FLAG_ISSET)
                &&
                (defender->total_health != defender->health ||
                 flag(&defender->state, CARD_HERO, FLAG_ISSET)) ) {

            continue;
        }

        // undamaged minion eg. execute
        if(flag(&attacker->state, CARD_TARGET_DAMAGED_MINION, FLAG_ISSET)
                &&
                (defender->total_health == defender->health ||
                 flag(&defender->state, CARD_HERO, FLAG_ISSET))) {

            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // shadow word: pain
        if(flag(&attacker->state, CARD_TARGET_3ATTACK_LESS, FLAG_ISSET) &&
                ( defender->attack > 3 ||
                  flag(&defender->state, CARD_HERO, FLAG_ISSET) )) {

            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // shadow word: death
        if(flag(&attacker->state, CARD_TARGET_5ATTACK_MORE, FLAG_ISSET) &&
                (defender->attack < 5 ||
                 flag(&defender->state, CARD_HERO, FLAG_ISSET))) {

            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // Ursa
        if(flag(&attacker->state, CARD_TARGET_URSA, FLAG_ISSET) &&
                strcmp(defender->entity->name, CN_URSA) != 0) {

            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // Sven
        if(flag(&attacker->state, CARD_TARGET_SVEN, FLAG_ISSET) &&
                strcmp(defender->entity->name, CN_SVEN) != 0) {

            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        // sacr. pact
        if(flag(&attacker->state, CARD_TARGET_DEMON, FLAG_ISSET) &&
                ( !flag(&defender->state, CARD_RACE_DEMON, FLAG_ISSET) ||
                  flag(&defender->state, CARD_HERO, FLAG_ISSET))) {

            if(debug) hm_log(LOG_DEBUG, lg, "target exit for card %d", deck->cards[i]->id);
            continue;
        }

        if(debug) hm_log(LOG_DEBUG, lg, "\tadding card %d as a target of [%s](%d)", defender->id, attacker->entity->desc, attacker->id);
        add_suboption_target(so, defender->id);
    }
}

int set_options(struct deck_s *deck, struct deck_s *opponent, char *buffer, char *endbuffer, int turn)
{
    int i;
    int taunt_count;

    struct packet_s *packet;
    struct alloptions_s *all;
    struct option_s *option = NULL, *o = NULL;

    assert(opponent);

    // does enemy board contain taunt?
    taunt_count = deck_get_taunts(opponent);

    // find playable cards
    for(i = 0; i < deck->ncards; i++) {
        // exhausted or frozen
        if(flag(&deck->cards[i]->state, CARD_EXHAUSTED, FLAG_ISSET) ||
                flag(&deck->cards[i]->state, CARD_FROZEN, FLAG_ISSET)) {
            if(log_enable) hm_log(LOG_DEBUG, lg, "skipping card %d as it's exhausted or frozen", deck->cards[i]->id);
            continue;
        }

        // not on board, in hand, heropower or frozen
        if(!flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_TARGETING_BC, FLAG_ISSET)) {
            if(log_enable) hm_log(LOG_DEBUG, lg, "skipping card %d as it's not: on board, in hand, heropower or hero with attack", deck->cards[i]->id);
            continue;
        }

        // set options to cards placed on board
        if(!flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) {
            set_options_board(&option, deck->cards[i], opponent, taunt_count);
        }

        int board_count = cards_get_board_count(deck, NULL, 0, -1);

        // set playable in hand cards that are not spells/bc
        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_SPELL, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_TARGETING_BC, FLAG_ISSET) &&
                deck->cards[i]->cost <= (deck->mana - deck->mana_used) &&
                board_count < MAX_BOARD
          ) {

            o = fill_option(3, deck->cards[i]->id, NULL);
            o->next = option;
            option = o;

            if(log_enable) hm_log(LOG_DEBUG, lg, "setting playable card [%s](%d) in hand and it's not a spell, board count %d", deck->cards[i]->entity->desc, deck->cards[i]->id, board_count);
            continue;
        }

        // set in-hand spells, targeting battlecries
        if((flag(&deck->cards[i]->state, CARD_SPELL, FLAG_ISSET) &&
                    flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET))
                ||
                (flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) &&
                 flag(&deck->cards[i]->state, CARD_TARGETING_BC, FLAG_ISSET))
                ||
                flag(&deck->cards[i]->state, CARD_WEAPON, FLAG_ISSET)
                ||
                flag(&deck->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)
          ) {

            if(log_enable) hm_log(LOG_DEBUG, lg, "Considering Spell/Battlecry card %d", deck->cards[i]->id);

            if(flag(&deck->cards[i]->state, CARD_TARGETING_BC, FLAG_ISSET) && board_count >= MAX_BOARD) {
                if(log_enable) hm_log(LOG_DEBUG, lg, "Battlecry card %d igrnoed as board is full", deck->cards[i]->id);
                continue;
            }

            if(deck->cards[i]->cost > (deck->mana - deck->mana_used)) {
                if(log_enable) hm_log(LOG_DEBUG, lg, "Spell card %d too expensive", deck->cards[i]->id);
                continue;
            }

            if(log_enable) hm_log(LOG_DEBUG, lg, "Card [%s](%d) with cost %d accepted", deck->cards[i]->entity->desc, deck->cards[i]->id, deck->cards[i]->cost);

            struct suboption_target_s *so = NULL;
            if(flag(&deck->cards[i]->state, CARD_TARGETING, FLAG_ISSET) || flag(&deck->cards[i]->state, CARD_TARGETING_BC, FLAG_ISSET)) {

                if(log_enable) hm_log(LOG_NOTICE, lg, "setting targets for card [%s](%d)", deck->cards[i]->entity->desc, deck->cards[i]->id);

                calc_targets(deck, deck->cards[i], SIDE_PLAYER, &so);
                calc_targets(opponent, deck->cards[i], SIDE_OPPONENT, &so);
            }

            if(so != NULL ||
                    flag(&deck->cards[i]->state, CARD_NONTARGET, FLAG_ISSET)
              ) {
                o = fill_option(/*deck->cards[i]->cardtype*/ 3, deck->cards[i]->id, so);
                o->next = option;
                option = o;
            }
        }
    }

    o = fill_option(2, -1, NULL);
    o->next = option;
    option = o;

    all = malloc(sizeof(*all));

    all->id = turn;
    all->options = option;

    alloptions_dump(all);

    add_packet(&packet, all, P_ALLOPTIONS);

    char *ptr = buffer;
    int n = serialize(packet, &ptr, endbuffer);

    packet_free(packet);

    //hm_log(LOG_NOTICE, lg, "set options:");
    //bin_dump(buffer, n);

    return n;
}
