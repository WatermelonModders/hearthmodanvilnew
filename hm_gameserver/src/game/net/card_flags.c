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

static void net_card_summon_entity(struct powerhistory_data_s **data, struct card_s *card, int id, int zone, int target_player)
{
    struct powerhistory_entity_s *full = NULL;

    // if summon to hand, only show it to owner
    if(zone == 1 || (zone == 3 && target_player == card->parent->deck_copy->controller) || zone == 6) {
        struct powerhistory_tag_s *card_tags = default_tags(card, card->entity, 0);
        add_tag(&card_tags, CONTROLLER, card->parent->deck_copy->controller);
        add_tag(&card_tags, ENTITY_ID, card->id);
        if(id != 0) {
            add_tag(&card_tags, CREATOR, id);
        }
        add_full_entity(&full, card_tags, card->id, card->entity->name, card->entity->nname);
        add_ph_data_full(data, full);

        hm_log(LOG_ALERT, lg, "Generating full entity zone %d for card [%s](%d) player %d card parent %d", zone, card->entity->desc, card->id, target_player, card->parent->deck_copy->controller);
    } else {
        struct powerhistory_tag_s *card_tags = NULL;
        add_tag(&card_tags, CONTROLLER, card->parent->deck_copy->controller);
        add_tag(&card_tags, ENTITY_ID, card->id);
        if(id != 0) {
            add_tag(&card_tags, CREATOR, id);
        }
        add_full_entity(&full, card_tags, card->id, NULL, 0);
        add_ph_data_full(data, full);

        hm_log(LOG_ALERT, lg, "Generating full enemy entity zone %d for card [%s](%d) player %d card parent %d", zone, card->entity->desc, card->id, target_player, card->parent->deck_copy->controller);
    }

    // if card is attached to any other
    /*
       for(att = card->attachments; att != NULL; att = att->next) {
       add_tagchange(data, ATTACHED, att->id, card->id);
       }
       */
    if(card->attached_to > 0) {
        add_tagchange(data, ATTACHED, card->attached_to, card->id);
    }

    add_tagchange(data, ZONE_POSITION, card->zone_position, card->id);
    add_tagchange(data, ZONE, zone, card->id);
    if(id != 0) {
        add_tagchange(data, 334, id, card->parent->deck_copy->controller);
    }
}

static void net_card_to_hand(struct powerhistory_data_s **data, struct card_s *card, int target_player, struct hs_holder_s *p)
{
    struct powerhistory_entity_s *show = NULL;
    struct powerhistory_tag_s *card_tags = default_tags(card, card->entity, 0);

    assert(card);
    assert(card->parent);
    assert(card->parent->deck_copy);
    assert(card->parent->deck_copy->controller);

    hm_log(LOG_ALERT, lg, "Card to hand deck controller %d target %d p->id %d", card->parent->deck_copy->controller, target_player, p->id);

    if(target_player == card->controller) {
        add_full_entity(&show, card_tags, card->id, card->entity->name, card->entity->nname);
        add_ph_data_show(data, show);
    }
}

static void net_spell_tracking(struct powerhistory_data_s **data, struct card_s *card, int target_player)
{
    struct card_list_s *t;

    if(card->target.card != NULL) {
        add_powerstart(data, 3, -1, card->id, 0, NULL, 0);
    }

    if(card->target.card != NULL) {
        add_meta(data, 0, 0, card->id);
    }

    if(target_player != 1) {
        for(t = card->target.card; t != NULL; t = t->next) {
            /*
               struct powerhistory_entity_s *full = NULL;
               struct powerhistory_tag_s *card_tags = NULL;
               add_tag(&card_tags, CONTROLLER, t->card->parent->controller);
               add_tag(&card_tags, ENTITY_ID, t->card->id);
               add_tag(&card_tags, ZONE, 6);
               add_full_entity(&full, card_tags, t->card->id, NULL, 0);
               add_ph_data_full(data, full);
               */

            add_tagchange(data, ZONE, 6, t->card->id);
        }
    } else {
        for(t = card->target.card; t != NULL; t = t->next) {
            struct powerhistory_entity_s *full = NULL;
            struct powerhistory_tag_s *card_tags = default_tags(t->card, t->card->entity, 6);
            add_tag(&card_tags, CONTROLLER, t->card->parent->deck_copy->controller);
            add_tag(&card_tags, ENTITY_ID, t->card->id);
            add_full_entity(&full, card_tags, t->card->id, t->card->entity->name, t->card->entity->nname);
            add_ph_data_show(data, full);
        }
    }

    if(card->target.card != NULL) {
        // add_powerend(data);
    }
}

static void net_spell_type(struct powerhistory_data_s **data, struct card_s *card)
{
    struct card_list_s *t;

    if(card->target.card && card->target.card->next == NULL) {
        add_powerstart(data, 3, -1, card->id, card->target.card->card->id, NULL, 0);
    } else {
        add_powerstart(data, 3, -1, card->id, 0, NULL, 0);
    }

    if(card->target.card != NULL) {
        struct powerhistory_info_s *info = NULL;

        for(t = card->target.card; t != NULL; t = t->next) {
            add_info(&info, t->card->id);
        }
        assert(info);
        add_meta_info(data, 0, 0, info);
    }

    for(t = card->target.card; t != NULL; t = t->next) {
        if(flag(&t->card->state, MECHANICS_ATTACK_HEAL, FLAG_ISSET)) {
            add_tagchange(data, 425, t->card->receive.heal, t->card->id);
            add_tagchange(data, 425, 0, t->card->id);
            add_meta(data, 2, t->card->receive.heal, t->card->id);
            add_tagchange(data, LAST_AFFECTED_BY, card->id, t->card->id);
        } else if(flag(&t->card->state, MECHANICS_ATTACK_DAMAGE, FLAG_ISSET)) {
            add_tagchange(data, PREDAMAGE, t->card->receive.damage, t->card->id);
            add_tagchange(data, PREDAMAGE, 0, t->card->id);
            add_meta(data, 1, t->card->receive.damage, t->card->id);
            add_tagchange(data, LAST_AFFECTED_BY, card->id, t->card->id);
        } else if(flag(&t->card->state, MECHANICS_DESTROY_WEAPON, FLAG_ISSET)) {

            add_tagchange(data, DURABILITY, 0, t->card->id);
            add_tagchange(data, LAST_AFFECTED_BY, card->id, t->card->id);
            add_tagchange(data, EXHAUSTED, 0, t->card->id);
            add_tagchange(data, ZONE, 4, t->card->id);
        }
    }


    add_powerend(data);
}

static int qscb(const struct flag_pr_s **src, const struct flag_pr_s **dst)
{
    return ((*src)->priority - (*dst)->priority);
}

static int merge(struct flag_pr_s ***dst, int ndst, struct flag_pr_s **src, int nsrc)
{
    int i;

    if(src == NULL && *dst != NULL) {
        return ndst;
    } else if(src != NULL && *dst == NULL) {
        *dst = src;
        return nsrc;
    } else {
        assert(dst && src);
    }

    *dst = realloc(*dst, (ndst + nsrc) * sizeof(void *));

    for(i = 0; i < nsrc; i++) {
        (*dst)[i + ndst] = src[i];
    }

    free(src);

    return (ndst + nsrc);
}

static void net_card_flags_iter(struct powerhistory_data_s **data, struct flag_pr_s **src, int nsrc, int target_player, struct hs_holder_s *p)
{
    int i;

#define TAG(m_id, m_num)\
    add_tagchange(data, m_id, m_num, src[i]->parent_card->id);

    for(i = 0; i < nsrc; i++) {
        switch(src[i]->flag) {
            case MECHANICS_DAMAGE:
                if(src[i]->type == F_ADDED) {
                    TAG(DAMAGE, src[i]->parent_card->total_health - src[i]->parent_card->health)
                }
                break;

            case MECHANICS_HEALTH:
                if(src[i]->type == F_ADDED) {
                    TAG(HEALTH, src[i]->parent_card->total_health)
                        TAG(DAMAGE, src[i]->parent_card->total_health - src[i]->parent_card->health);
                }
                break;

            case MECHANICS_ATTACK:
                if(src[i]->type == F_ADDED) {
                    TAG(ATK, src[i]->parent_card->attack)
                }
                break;

            case MECHANICS_ARMOR:
                if(src[i]->type == F_ADDED) {
                    TAG(ARMOR, src[i]->parent_card->armor)
                }
                break;

            case MECHANICS_DETACH:
                if(src[i]->type == F_ADDED) {
                    TAG(ATTACHED, 0)
                        //TAG(314, 0)
                        //TAG(323, 0)
                        //TAG(324, 0)
                }
                break;

            case MECHANICS_DURABILITY:
                if(src[i]->type == F_ADDED) {
                    TAG(DAMAGE, src[i]->parent_card->maxdurability - src[i]->parent_card->durability)
                }
                break;

            case MECHANICS_CHANGESIDES:
                if(src[i]->type == F_ADDED) {
                    TAG(CONTROLLER, src[i]->parent_card->controller)
                }
                break;

            case MECHANICS_COST:
                if(src[i]->type == F_ADDED) {
                    TAG(COST, src[i]->parent_card->cost)
                }
                break;


            case MECHANICS_SELECTED:
                if(src[i]->type == F_ADDED) {
                    net_spell_tracking(data, src[i]->parent_card, target_player);
                }
                break;

            case CARD_WINDFURY:
                if(src[i]->type == F_ADDED) {
                    TAG(WINDFURY, 1)
                } else {
                    TAG(WINDFURY, 0)
                }
                break;

            case CARD_DIVINE_SHIELD:
                if(src[i]->type == F_REMOVED) {
                    TAG(DIVINE_SHIELD, 0)
                } else {
                    TAG(DIVINE_SHIELD, 1)
                }
                break;

            case CARD_TAUNT:
                if(src[i]->type == F_REMOVED) {
                    TAG(TAUNT, 0)
                } else {
                    TAG(TAUNT, 1)
                }
                break;

            case CARD_EXHAUSTED:
                if(src[i]->type == F_REMOVED) {
                    TAG(EXHAUSTED, 0)
                } else {
                    TAG(EXHAUSTED, 1)
                }
                break;

            case CARD_FROZEN:
                if(src[i]->type == F_REMOVED) {
                    TAG(FROZEN, 0)
                } else {
                    TAG(FROZEN, 1)
                }
                break;

            case CARD_HIDE:
                if(src[i]->type == F_ADDED) {
                    add_hide(data, src[i]->parent_card->id, 6);
                }
                break;

            case CARD_LINKED:
                if(src[i]->type == F_ADDED) {
                    TAG(LINKEDCARD, src[i]->parent_card->linked)
                }
                break;

            case CARD_DESTROYED:
                if(src[i]->type == F_ADDED) {
                    add_powerstart(data, 6, 0, 1, 0, NULL, 0);
                    TAG(ZONE, 4)
                        TAG(ZONE_POSITION, 0)
                        TAG(DAMAGE, 0)
                        add_powerend(data);
                }
                break;

            case CARD_DISCARD:
                if(src[i]->type == F_ADDED) {
                    TAG(ZONE, 4)
                }
                break;

            case CARD_HAND:
                if(src[i]->type == F_ADDED) {
                    net_card_to_hand(data, src[i]->parent_card, target_player, p);
                    TAG(ZONE, 3)
                        TAG(ZONE_POSITION, src[i]->parent_card->zone_position)     // force new position as well
                }
                break;

            case CARD_BOARD:
                if(src[i]->type == F_ADDED) {
                    TAG(ZONE, 1)
                        TAG(ZONE_POSITION, src[i]->parent_card->zone_position)     // force new position as well
                        if(flag(&src[i]->parent_card->state, CARD_SPELL, FLAG_ISSET) &&
                                !flag(&src[i]->parent_card->state, CARD_BUFF, FLAG_ISSET) &&
                                !flag(&src[i]->parent_card->state, CARD_WEAPON, FLAG_ISSET)
                                /*
                                   !flag(&src[i]->parent_card->state, CARD_SUMMON_CS2_MIRROR, FLAG_ISSET) && // FIXME: tmp


                                   !flag(&src[i]->parent_card->state, CARD_DRAW_CARD, FLAG_ISSET) && // FIXME: tmp

                                   !flag(&src[i]->parent_card->state, CARD_TRACKING, FLAG_ISSET) // FIXME: tmp
                                   */
                          ) {
                            net_spell_type(data, src[i]->parent_card);
                        }
                }
                break;

            case MECHANICS_BATTLECRY_TRIGGER:
                if(src[i]->type == F_ADDED) {
                    /*
                       if(
                    // non-target
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_DAMAGE3_ENEMY_HERO, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_DESTROY_ENEMY_WEAPON, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_DAMAGE1_ALL, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_HEAL2_ALIES, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_HEAL6_ALLY_HERO, FLAG_ISSET) ||

                    // targetable
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_HEAL2_TARGET, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_DAMAGE1_TARGET, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_DAMAGE2_TARGET, FLAG_ISSET) ||
                    flag(&src[i]->parent_card->state, CARD_BATTLECRY_DAMAGE3_TARGET, FLAG_ISSET)
                    ) {
                    */
                    net_spell_type(data, src[i]->parent_card);
                    //}
                }
                break;

            case MECHANICS_TURN_TRIGGER:
                if(src[i]->type == F_ADDED) {
                    // TODO; fix this
                    /*
                       if(
                    // non-target
                    flag(&src[i]->parent_card->state, CARD_TURN_HEAL1_MINIONS, FLAG_ISSET)
                    ) {
                    net_spell_type(data, src[i]->parent_card);
                    }
                    */

                    net_spell_type(data, src[i]->parent_card);
                }
                break;


            case MECHANICS_HEROPOWER:
                if(src[i]->type == F_ADDED) {
                    net_spell_type(data, src[i]->parent_card);
                }
                break;


            default:
                hm_log(LOG_DEBUG, lg, "Flag %d not supported", src[i]->flag);
                break;
        }
    }
}

static void change_zone_positions(struct powerhistory_data_s **data, struct deck_s *old, struct deck_s *new)
{
    int i;

    for(i = 0; i < old->ncards && i < new->ncards; i++) {
        if(old->cards[i]->zone_position != new->cards[i]->zone_position) {
            add_tagchange(data, ZONE_POSITION, new->cards[i]->zone_position, new->cards[i]->id);
        }
    }
}

static void net_card_flags_step(struct powerhistory_data_s **data, struct card_s *old_card, struct card_s *new_card, struct hs_holder_s *p, struct flag_pr_s ***dst, int *ndst)
{
    struct flag_pr_s **old = NULL;
    struct flag_pr_s **new = NULL;
    int nold = 0, nnew = 0;
    int n;

    //flag(&new_card->state, -1, FLAG_DUMP);
    flags_diff(&old_card->state, &new_card->state, &new, &nnew, &old, &nold, new_card);
    if(nnew != 0 || nold != 0) {
        hm_log(LOG_DEBUG, lg, "Considering card [%s](%d)", new_card->entity->desc, new_card->id);
        n = merge(&new, nnew, old, nold);
        *ndst = merge(dst, *ndst, new, n);

        assert(new);
    }

    // game over FIXME: wrong position
    if(flag(&new_card->state, CARD_HERO, FLAG_ISSET) && new_card->health == 0) {
        add_tagchange(data, PLAYSTATE, 3, p->entity_id == 2 ? 3 : 2);   // needed
        add_tagchange(data, PLAYSTATE, 5, p->entity_id == 2 ? 3 : 2);   // needed
        add_tagchange(data, PLAYSTATE, 4, p->entity_id);                // needed
    }
}

static void net_card_summon(struct powerhistory_data_s **data, struct deck_s *deck_old, struct deck_s *deck_new, struct card_list_s *attacker, int target_player, int nettype)
{
    int i;
    int att;

    hm_log(LOG_DEBUG, lg, "Attempt to summon from deck %p:%p", deck_old, deck_new);

    if(!(deck_old && deck_new && deck_new->ncards != deck_old->ncards)) {
        if(deck_new && deck_old) {
            hm_log(LOG_DEBUG, lg, "Not summoning from deck %p:%p:%d new: %d old: %d", deck_new, deck_old, deck_new->controller, deck_new->ncards, deck_old->ncards);
        }
        return;
    }


    hm_log(LOG_DEBUG, lg, "Summoning as deck new %p:%d and deck old %p:%d", deck_new, deck_new->ncards, deck_old, deck_old->ncards);

    if(attacker && attacker->card) {
        att = attacker->card->id;
    } else {
        att = 0;
    }

    // it's very important to set this when various user actions summon units
    if(nettype == NET_PUTONBOARD || nettype == NET_BUFFAOE) {
        add_powerstart(data, 3, -1, att, 0, NULL, 0);
    } else if(nettype == NET_SPELL || nettype == NET_SPELLAOE) {
        add_powerstart(data, 6, 0, att, 0, NULL, 0);
    }
    // NET_BUFF, NET_SPELLAOE - no power start

    // we never remove cards, so diff is always up
    for(i = deck_old->ncards; i < deck_new->ncards; i++) {
        // if card just changed sides, ignore it
        if(flag(&deck_new->cards[i]->state, MECHANICS_CHANGESIDES, FLAG_ISSET)) {
            continue;
        }

        // summon attachment
        if(flag(&deck_new->cards[i]->state, CARD_ATTACHMENT, FLAG_ISSET)) {
            //hm_log(LOG_DEBUG, lg, "Summoning attachment card [%s](%d)", deck_new->cards[i]->entity->name, deck_new->cards[i]->id);
            //net_card_summon_entity(data, deck_new->cards[i], att, 6, target_player);
            // summon cards on board
        } else if(flag(&deck_new->cards[i]->state, CARD_BOARD, FLAG_ISSET)) {
            hm_log(LOG_DEBUG, lg, "Summoning card [%s](%d) to board", deck_new->cards[i]->entity->name, deck_new->cards[i]->id);
            net_card_summon_entity(data, deck_new->cards[i], att, 1, target_player);
            // summon cards in hand
        } else if(flag(&deck_new->cards[i]->state, CARD_HAND, FLAG_ISSET)
                ) {
            hm_log(LOG_DEBUG, lg, "Summoning card [%s](%d) to hand", deck_new->cards[i]->entity->name, deck_new->cards[i]->id);
            net_card_summon_entity(data, deck_new->cards[i], att, 3, target_player);
        } else {
            hm_log(LOG_DEBUG, lg, "Card [%s](%d) set to be summoned, but no destination set, aborting", deck_new->cards[i]->entity->name, deck_new->cards[i]->id);
            abort();
        }
    }

    if(nettype == NET_PUTONBOARD || nettype == NET_SPELL || nettype == NET_BUFFAOE || nettype == NET_SPELLAOE) {
        add_powerend(data);
    }

    for(i = deck_old->ncards; i < deck_new->ncards; i++) {
        // if card just changed sides, ignore it
        if(flag(&deck_new->cards[i]->state, MECHANICS_CHANGESIDES, FLAG_ISSET)) {
            continue;
        }

        // summon attachment
        if(flag(&deck_new->cards[i]->state, CARD_ATTACHMENT, FLAG_ISSET)) {
            hm_log(LOG_DEBUG, lg, "Summoning attachment card [%s](%d)", deck_new->cards[i]->entity->name, deck_new->cards[i]->id);
            net_card_summon_entity(data, deck_new->cards[i], att, 6, target_player);
        }
    }

    // NET_BUFF, NET_SPELLAOE - no power end
}

void net_card_flags(struct powerhistory_data_s **data, struct deck_s *deck_old, struct deck_s *deck_new,
        struct deck_s *opp_old, struct deck_s *opp_new, struct card_list_s *src, struct hs_holder_s *p, int target_player, int nettype)
{
    struct card_s *old_card = NULL;
    struct card_list_s *c;
    int i;
    struct flag_pr_s **summary = NULL;
    int nsummary = 0;

    //add_tagchange(&data, TEMP_RESOURCES, 0, p->entity_id);

#define d1 (p->deck)
#define c1 (p->deck_copy)
#define d2 (p->opponent.deck)
#define c2 (p->opponent.deck_copy)

    assert(p);

    assert(deck_old && deck_new);

    change_zone_positions(data, d1, c1);
    change_zone_positions(data, d2, c2);

    if(deck_old->mana_used != deck_new->mana_used) {
        add_tagchange(data, RESOURCES_USED, deck_new->mana_used, p->entity_id);
    }

    if(deck_old->spellpower != deck_new->spellpower) {
        add_tagchange(data, CURRENT_SPELLPOWER, deck_new->spellpower, p->entity_id);
    }

    if(opp_old && opp_new && opp_old->spellpower != opp_new->spellpower) {
        add_tagchange(data, CURRENT_SPELLPOWER, opp_new->spellpower, p->entity_id == 3 ? 2 : 3);
    }

    if(src != NULL) {
        for(c = src; c != NULL; c = c->next) {
            old_card = card_get(deck_old, opp_old, c->card->id);

            assert(old_card);

            net_card_flags_step(data, old_card, c->card, p, &summary, &nsummary);
        }

        net_card_summon(data, deck_old, deck_new, src, target_player, nettype);
        net_card_summon(data, opp_old, opp_new, src, target_player, nettype);

    } else {

        for(i = 0; i < deck_old->ncards && i < deck_new->ncards; i++) {
            net_card_flags_step(data, deck_old->cards[i], deck_new->cards[i], p, &summary, &nsummary);
        }

        for(i = 0; i < opp_old->ncards && i < opp_new->ncards; i++) {
            net_card_flags_step(data, opp_old->cards[i], opp_new->cards[i], p, &summary, &nsummary);
        }

        net_card_summon(data, deck_old, deck_new, NULL, target_player, nettype);
        net_card_summon(data, opp_old, opp_new, NULL, target_player, nettype);
    }

    if(summary) {
        qsort(summary, nsummary, sizeof(*summary), (int (*)(const void *, const void *))qscb);
        net_card_flags_iter(data, summary, nsummary, target_player, p);
    }

    if(nsummary > 0) {
        for(i = 0; i < nsummary; i++) {
            free(summary[i]);
        }
        free(summary);
    }
}
