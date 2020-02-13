/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"

/*
 * Local functions.
 */
int	 defense_mods		args ((CHAR_DATA * ch));
void check_assist 		args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge 		args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_presence		args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_blink		args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_displacement	args ((CHAR_DATA * ch, CHAR_DATA * victim));
void check_killer 		args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_parry 		args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_hand_parry	args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_shield_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_counter		args ((CHAR_DATA * ch, CHAR_DATA *victim, int dam, int dt));
bool check_riposte		args ((CHAR_DATA * ch, CHAR_DATA *victim, int dam, int dt));
bool check_evasion		args ((CHAR_DATA * ch, CHAR_DATA * victim));
void dam_message		args ((CHAR_DATA * ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool immune, bool crit ));
void death_cry 			args ((CHAR_DATA * ch));
void group_gain 		args ((CHAR_DATA * ch, CHAR_DATA * victim));
int xp_compute 			args ((CHAR_DATA * gch, CHAR_DATA * victim, int total_levels));
bool is_safe 			args ((CHAR_DATA * ch, CHAR_DATA * victim));
void make_corpse 		args ((CHAR_DATA * ch, CHAR_DATA * killer));
void one_hit	     	args ((CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary));
void do_palm_strike     args ((CHAR_DATA *ch, char * argument));
void mob_hit 			args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
int what_size			args ((CHAR_DATA *ch));


void set_fighting 		args ((CHAR_DATA * ch, CHAR_DATA * victim));
void disarm 			args ((CHAR_DATA * ch, CHAR_DATA * victim));
void spell_null 		(int sn, int level, CHAR_DATA * ch, void *vo, int target);
int wpnSpecLookup		args((const char *name));
int get_bulk			args((CHAR_DATA *ch));
int get_material		args((char *argument));
int				get_spell_crit_chance args	( (CHAR_DATA *ch) );
int				get_melee_crit_chance args	( (CHAR_DATA *ch) );


//Event funcs:
void mining_update			args ((CHAR_DATA *ch));
void prospecting_update		args ((CHAR_DATA *ch));


	

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//																//
//			Magical Spell Effects, used to be effects.c			//
//																//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void acid_effect (void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {                            /* nail objects on the floor */
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {			
            obj_next = obj->next_content;
			
			if (!IS_SET(obj->extra2_flags, ITEM_HARDSTAFF))
			{
				obj->condition -= number_range(5,25);
				acid_effect (obj, level, dam, TARGET_OBJ);
			}
        }
        return;
    }

    if (target == TARGET_CHAR)
    {                            /* do the effect on a victim */
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* let's toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
			if (!IS_SET(obj->extra2_flags, ITEM_HARDSTAFF))
			{
				obj->condition -= number_range(5,25);
				acid_effect (obj, level, dam, TARGET_OBJ);
			}
        }
        return;
    }

    if (target == TARGET_OBJ)
    {                            /* toast an object */
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        OBJ_DATA *t_obj, *n_obj;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

		if (!IS_SET(obj->extra2_flags, ITEM_HARDSTAFF))
			return;
			
        chance = level / 4 + dam / 10;

        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
		
		if (obj->condition < 40)
			chance += 40 - obj->condition;
			
        chance -= obj->level * 2;

        switch (obj->item_type)
        {
            default:
                return;
            case ITEM_CONTAINER:
            case ITEM_CORPSE_PC:
            case ITEM_CORPSE_NPC:
			case ITEM_SKIN:			
                msg = "$p fumes and dissolves.";
                break;
            case ITEM_ARMOR:
			case ITEM_WEAPON:
                msg = "$p is pitted and etched.";
                break;
            case ITEM_CLOTHING:
                msg = "$p is corroded into scrap.";
                break;
            case ITEM_STAFF:
            case ITEM_WAND:
                chance -= 10;
                msg = "$p corrodes and breaks.";
                break;
            case ITEM_SCROLL:
			case ITEM_PAPER:
			case ITEM_MAP:
			case ITEM_FOOD:			
                chance += 10;
                msg = "$p is burned into waste.";
                break;
			case ITEM_POTION:
			case ITEM_DRINK_CON:
				chance -= 5;
				msg = "$p's container melts, wasting the contents!";
				break;
        }

        chance = URANGE (5, chance, 95);

        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        if (obj->item_type == ITEM_ARMOR)
        {                        /* etch it */
            AFFECT_DATA *paf;
            bool af_found = FALSE;
            int i;

            affect_enchant (obj);			
            for (paf = obj->affected; paf != NULL; paf = paf->next)
            {
                if (paf->location == APPLY_AC)
                {
                    af_found = TRUE;
                    paf->type = -1;
                    paf->modifier += 1;
                    paf->level = UMAX (paf->level, level);
                    break;
                }
            }

            if (!af_found)
                /* needs a new affect */
            {
                paf = new_affect ();

                paf->type = -1;
                paf->level = level;
                paf->duration = -1;
                paf->location = APPLY_AC;
                paf->modifier = 1;
                paf->bitvector = 0;
                paf->next = obj->affected;
                obj->affected = paf;
            }

            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            return;
        }

        /* get rid of the object */
        if (obj->contains)
        {                        /* dump contents */
            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj (t_obj);
                if (obj->in_room != NULL)
                    obj_to_room (t_obj, obj->in_room);
                else if (obj->carried_by != NULL)
                    obj_to_room (t_obj, obj->carried_by->in_room);
                else
                {
                    extract_obj (t_obj);
                    continue;
                }

                acid_effect (t_obj, level / 2, dam / 2, TARGET_OBJ);
            }
        }

        extract_obj (obj);
        return;
    }
}


void cold_effect (void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {                            /* nail objects on the floor */
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)
    {                            /* whack a character */
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* chill touch effect */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_COLD))
        {
            AFFECT_DATA af;

            act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
            act ("A chill sinks deep into your bones.", victim, NULL, NULL,
                 TO_CHAR);
            af.where = TO_AFFECTS;
            af.type = skill_lookup ("chill touch");
            af.level = level;
            af.duration = 6;
            af.location = APPLY_STR;
            af.modifier = -1;
            af.bitvector = 0;
            affect_join (victim, &af);
        }

        /* hunger! (warmth sucked out */
        if (!IS_NPC (victim))
            gain_condition (victim, COND_HUNGER, dam / 20);

        /* let's toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ)
    {                            /* toast an object */
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;

        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;

        chance -= obj->level * 2;

        switch (obj->item_type)
        {
            default:
                return;
            case ITEM_POTION:
                msg = "$p freezes and shatters!";
                chance += 25;
                break;
            case ITEM_DRINK_CON:
                msg = "$p freezes and shatters!";
                chance += 5;
                break;
        }

        chance = URANGE (5, chance, 95);

        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        extract_obj (obj);
        return;
    }
}



void fire_effect (void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {                            /* nail objects on the floor */
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            fire_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)
    {                            /* do the effect on a victim */
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* chance of blindness */
        if (!IS_AFFECTED (victim, AFF_BLIND)
            && !saves_spell (level / 4 + dam / 20, victim, DAM_FIRE))
        {
            AFFECT_DATA af;
            act ("$n is blinded by smoke!", victim, NULL, NULL, TO_ROOM);
            act ("Your eyes tear up from smoke...you can't see a thing!",
                 victim, NULL, NULL, TO_CHAR);

            af.where = TO_AFFECTS;
            af.type = skill_lookup ("fire breath");
            af.level = level;
            af.duration = number_range (0, level / 10);
            af.location = APPLY_HITROLL;
            af.modifier = -4;
            af.bitvector = AFF_BLIND;

            affect_to_char (victim, &af);
        }

        /* getting thirsty */
        if (!IS_NPC (victim))
            gain_condition (victim, COND_THIRST, dam / 20);

        /* let's toast some gear! */
        for (obj = victim->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            fire_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ)
    {                            /* toast an object */
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        OBJ_DATA *t_obj, *n_obj;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

		if (IS_SET(obj->extra2_flags, ITEM_HARDSTAFF) || IS_SET(obj->extra2_flags, ITEM_SHILLELAGH))
			return;
		
        chance = level / 4 + dam / 10;

        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch (obj->item_type)
        {
            default:
				if (IS_WOOD(obj->material))				
				{
					chance += 10;
					msg = "$p smokes and chars!";
					break;
				}
				else
					return;
			case ITEM_ARMOR:
			case ITEM_WEAPON:
				if (IS_METAL(obj->material))				
					msg = "$p begins glowing a bright orange!";
				else if (IS_CLOTH(obj->material))
					msg = "$p burns to ash!";
				else if (IS_WOOD(obj->material))
					msg = "$p burns to ash!";
				else
					msg = "$p ignites and burns!";
				break;
            case ITEM_CONTAINER:
                msg = "$p ignites and burns!";
                break;
            case ITEM_POTION:
                chance += 25;
                msg = "$p bubbles and boils!";
                break;
            case ITEM_SCROLL:
			case ITEM_MAP:			
                chance += 50;
                msg = "$p crackles and burns!";
                break;
            case ITEM_STAFF:
                chance += 10;
                msg = "$p smokes and chars!";
                break;
            case ITEM_WAND:
                msg = "$p sparks and sputters!";
                break;
            case ITEM_FOOD:
                msg = "$p blackens and crisps!";
                break;
            case ITEM_PILL:
                msg = "$p melts and drips!";
                break;
			case ITEM_PAPER:
				msg = "$p burns to ash!";
				break;
			case ITEM_PEN:
				msg = "$p melts and drips!";
				break;
        }

        chance = URANGE (5, chance, 95);

        if (number_percent () > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        if (obj->contains)
        {
            /* dump the contents */

            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj (t_obj);
                if (obj->in_room != NULL)
                    obj_to_room (t_obj, obj->in_room);
                else if (obj->carried_by != NULL)
                    obj_to_room (t_obj, obj->carried_by->in_room);
                else
                {
                    extract_obj (t_obj);
                    continue;
                }
                fire_effect (t_obj, level / 2, dam / 2, TARGET_OBJ);
            }
        }

        extract_obj (obj);
        return;
    }
}

void poison_effect (void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {                            /* nail objects on the floor */
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)
    {                            /* do the effect on a victim */
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* chance of poisoning */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_POISON))
        {
            AFFECT_DATA af;

            SEND ("You feel poison coursing through your veins.\r\n",
                          victim);
            act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);

            af.where = TO_AFFECTS;
            af.type = gsn_poison;
            af.level = level;
            af.duration = level / 2;
            af.location = APPLY_STR;
            af.modifier = level / 6;
            af.bitvector = AFF_POISON;
            affect_join (victim, &af);
        }

        /* equipment */
        for (obj = victim->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ)
    {                            /* do some poisoning */
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        int chance;


        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_BLESS) || number_range (0, 4) == 0)
            return;

        chance = level / 4 + dam / 10;
        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        chance -= obj->level * 2;

        switch (obj->item_type)
        {
            default:
                return;
            case ITEM_FOOD:
                break;
            case ITEM_DRINK_CON:
                if (obj->value[0] == obj->value[1])
                    return;
                break;
        }

        chance = URANGE (5, chance, 95);

        if (number_percent () > chance)
            return;

        obj->value[3] = 1;
        return;
    }
}


void shock_effect (void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            shock_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        /* daze and confused? */
        if (!saves_spell (level / 4 + dam / 20, victim, DAM_LIGHTNING))
        {
            SEND ("Your muscles stop responding.\r\n", victim);
            DAZE_STATE (victim, UMAX (12, level / 4 + dam / 20));
        }

        /* toast some gear */
        for (obj = victim->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            shock_effect (obj, level, dam, TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_OBJ)
    {
        OBJ_DATA *obj = (OBJ_DATA *) vo;
        int chance;
        char *msg;

        if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF)
            || IS_OBJ_STAT (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
            return;

        chance = 15;
		
		if (obj->level > 25)
			chance -= 5;
				
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
            chance -= 5;        

        switch (obj->item_type)
        {
            default:
				if (IS_METAL(obj->material))
				{					
					msg = "$p melts into worthless ore!";
					chance += 10;
					break;
				}
				else
					return;
            case ITEM_WAND:
            case ITEM_STAFF:
                chance += 5;
                msg = "$p overloads and explodes!";
                break;
            case ITEM_JEWELRY:
                chance -= 10;
                msg = "$p is fused into a worthless lump.";
        }        		
		
        if (number_percent() > chance)
            return;

        if (obj->carried_by != NULL)
            act (msg, obj->carried_by, obj, NULL, TO_ALL);
        else if (obj->in_room != NULL && obj->in_room->people != NULL)
            act (msg, obj->in_room->people, obj, NULL, TO_ALL);

        extract_obj (obj);
        return;
    }
}


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
	OBJ_DATA *obj, *obj_next;
    bool room_trig = FALSE;
	
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;        

		if (!ch->in_room)
			continue;
		
		//Mana upkeep for holding a globe of invulnerability up.
		if (is_affected(ch, gsn_globe_invulnerability))
		{
			ch->mana -= number_range(1,2);
			if (ch->mana < 1)
			{
				affect_strip(ch, gsn_globe_invulnerability);
				SEND ("Your globe of invulnerability has just failed.\r\n",ch);
				act ("$n is no longer enveloped in a globe of energy!", ch, NULL, NULL, TO_ROOM);
			}
		}	
		
		if(ch->position == POS_SLEEPING)
			continue;
			
		if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
            continue;        
			
		if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
            multi_hit (ch, victim, TYPE_UNDEFINED);
        else
            stop_fighting (ch, FALSE);

        if ((victim = ch->fighting) == NULL)
            continue;
			
        /*
         * Fun for the whole family!
         */
        check_assist (ch, victim);

        if (IS_NPC (ch))
        {
            if (HAS_TRIGGER_MOB(ch, TRIG_FIGHT))
                p_percent_trigger (ch, NULL, NULL, victim, NULL, NULL, TRIG_FIGHT);
            if (HAS_TRIGGER_MOB(ch, TRIG_HPCNT))
                p_hprct_trigger (ch, victim);
        }
		for ( obj = ch->carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
	 
			if ( obj->wear_loc != WEAR_NONE && HAS_TRIGGER_OBJ( obj, TRIG_FIGHT ) )
			p_percent_trigger( NULL, obj, NULL, victim, NULL, NULL, TRIG_FIGHT );
		}
	 
		if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_FIGHT ) && room_trig == FALSE )
		{
			room_trig = TRUE;
			p_percent_trigger( NULL, NULL, ch->in_room, victim, NULL, NULL, TRIG_FIGHT );
		}
    }

    return;
}

/* for auto assisting */
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
        rch_next = rch->next_in_room;

        if (IS_AWAKE (rch) && rch->fighting == NULL)
        {

            /* quick check for ASSIST_PLAYER */
            if (!IS_NPC (ch) && IS_NPC (rch)
                && IS_SET (rch->off_flags, ASSIST_PLAYERS)
                && rch->level + 6 > victim->level && (!IS_AFFECTED (ch, AFF_BLIND)))
            {
                do_function (rch, &do_emote, "screams and attacks!");
                multi_hit (rch, victim, TYPE_UNDEFINED);
                continue;
            }

            /* PCs next */
            if (!IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM))
            {
                if (((!IS_NPC (rch) && IS_SET (rch->act, PLR_AUTOASSIST))
                     || IS_AFFECTED (rch, AFF_CHARM))
                    && is_same_group (ch, rch) && !is_safe (rch, victim) && (!IS_AFFECTED (rch, AFF_BLIND)))
                    multi_hit (rch, victim, TYPE_UNDEFINED);

                continue;
            }

            /* now check the NPC cases */

            if (IS_NPC (ch) && !IS_AFFECTED (ch, AFF_CHARM) && (!IS_AFFECTED (ch, AFF_BLIND)))
            {
                if ((IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALL))
                    || (IS_NPC (rch) && rch->group && rch->group == ch->group)
                    || (IS_NPC (rch) && rch->race == ch->race
                        && IS_SET (rch->off_flags, ASSIST_RACE))
                    || (IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALIGN)
                        && ((IS_GOOD (rch) && IS_GOOD (ch))
                            || (IS_EVIL (rch) && IS_EVIL (ch))
                            || (IS_NEUTRAL (rch) && IS_NEUTRAL (ch))))
                    || (rch->pIndexData == ch->pIndexData
                        && IS_SET (rch->off_flags, ASSIST_VNUM)))
                {
                    CHAR_DATA *vch;
                    CHAR_DATA *target;
                    int number;

                    if (number_bits (1) == 0)
                        continue;

                    target = NULL;
                    number = 0;
                    for (vch = ch->in_room->people; vch; vch = vch->next)
                    {
                        if (can_see (rch, vch)
                            && is_same_group (vch, victim)
                            && number_range (0, number) == 0)
                        {
                            target = vch;
                            number++;
                        }
                    }

                    if (target != NULL)
                    {
                        do_function (rch, &do_emote, "screams and attacks!");
                        multi_hit (rch, target, TYPE_UNDEFINED);
                    }
                }
            }
        }
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    //int chance;
	int attacks = 1, i;
	OBJ_DATA *obj;
	
    /* decrement the wait */
    if (ch->desc == NULL)
        ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
        ch->daze = UMAX (0, ch->daze - PULSE_VIOLENCE);


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
        return;
		
    if (IS_NPC (ch))
    {
		if (IS_SET (ch->act2, ACT2_INANIMATE))
			return;
        mob_hit (ch, victim, dt);
        return;
    }
	
	//Launch palm_strike
	if ( ch->pcdata->fight_pos == FIGHT_PALM && ch->wait <= 0 )
		do_palm_strike( ch, "" );
	
	//Being hasted helps.
	if (IS_AFFECTED (ch, AFF_HASTE))
        attacks += number_range(1,2);    
		
	//Fighters get an extra attack starting at level 10
	if (IS_FIGHTER(ch) && total_levels(ch) > 9)
		attacks += 1;	
	
	if (IS_FIGHTER(ch) && total_levels(ch) > 25 && !IS_MCLASSED(ch))
		attacks += 1;	
	
	if (IS_FIGHTER(ch) && total_levels(ch) > 39 && !IS_MCLASSED(ch))
		attacks += 1;
	
	if (GET_STR(ch) >= 18 && IS_PALADIN(ch))
		attacks += 1;

	if (GET_STR(ch) >= 22 && IS_PALADIN(ch))
		attacks += 1;
	
	if (IS_WIZARD(ch) || IS_PSION(ch))
		attacks -= 1;
	
	//Better dex yields more attacks...for some
	if (IS_FIGHTER(ch) || IS_MONK(ch))
		attacks += dex_app[GET_DEX(ch)].attacks;	
	if ((IS_RANGER(ch) || IS_THIEF(ch)) && !IS_MCLASSED(ch)) //The mclass check is just so they don't add to the fighter/monk attacks.
	{
		if (GET_DEX(ch) > 15)
			attacks += dex_app[GET_DEX(ch)].attacks - 1;
	}
	
	if (IS_DRUID(ch) && KNOWS(ch, gsn_hapkido) && number_percent() < get_skill(ch, gsn_hapkido))
	{
		check_improve (ch, gsn_hapkido, TRUE, 2);		
		attacks++;
	}
	//If you're carrying too much it'll be hard to swing a weapon around.
	if (ENCUMBERED(ch))
		attacks /= 2;
	
	//Being slowed sucks.
	if (IS_AFFECTED (ch, AFF_SLOW))
		attacks /= 2;
	
	if ( is_affected(ch, gsn_web))
		attacks--;
	
	if (get_eq_char (ch, WEAR_SECONDARY) && IS_WEAPON(get_eq_char(ch, WEAR_SECONDARY)) && KNOWS(ch, gsn_dual_wield))
		
		attacks++;
	
	attacks = number_range(attacks/2,attacks);

	if (attacks > 8) //fighter can get up to 11, so this gives them a buffer for slow/web/etc.
		attacks = 8;

	if (attacks < 1) //nobody should get 0 attacks... cmon...
		attacks = 1;
	
	
	if (IS_MONK(ch)) //Monks need to be wearing a staff or nothing at all in their hands.
	{
		if (get_eq_char (ch, WEAR_WIELD))
		{
			if (IS_WEAPON(get_eq_char(ch, WEAR_WIELD)))
			{			
				obj = get_eq_char (ch, WEAR_WIELD);
				if (obj->value[0] != WEAPON_STAFF)
					attacks = 1;
			}
			else //Let's not allow monks to dual wield wands and get 7 attacks / round.			
				attacks = 1;			
		}
		if (get_eq_char (ch, WEAR_SECONDARY))
		{
			if (IS_WEAPON(get_eq_char(ch, WEAR_SECONDARY)))
			{			
				obj = get_eq_char (ch, WEAR_SECONDARY);
				if (obj->value[0] != WEAPON_STAFF)
					attacks = 1;
			}
			else //Let's not allow monks to dual wield wands and get 7 attacks / round.			
				attacks = 1;
		}
		//if (get_eq_char (ch, WEAR_HOLD))
		//	attacks = 1;
	}
	
	bool main = TRUE; //main hand?
	
	for (i = 0; i < attacks; i++)
    {
		if (ch->in_room->area->curr_precip_amount > 0 && number_percent() < 40)
		{	
			if (!dex_check(ch, (ch->in_room->area->curr_precip_amount + what_size(ch) > SIZE_MEDIUM ? 1 : 0)) && !is_affected(ch, gsn_fly) && !IS_AFFECTED(ch, AFF_FLYING))
			{
				if (ch->in_room->area->curr_precip == RAIN)
				{	
					SEND("You slip and fall in the rain!\r\n",ch);
					act( "$n loses their footing in the rain.",ch,NULL,NULL,TO_ROOM);
					WAIT_STATE (ch, PULSE_VIOLENCE / 2);
					ch->position = POS_SITTING;
					continue;
				}
				if (ch->in_room->area->curr_precip == SNOW)
				{	
					SEND("You slip and fall in the snow!\r\n",ch);
					act( "$n loses their footing in the snow.",ch,NULL,NULL,TO_ROOM);
					WAIT_STATE (ch, PULSE_VIOLENCE / 2);
					ch->position = POS_SITTING;
					continue;
				}
			}
		}
		
		if (main == TRUE)
		{												
			one_hit (ch, victim, dt, FALSE);			
			if (get_eq_char (ch, WEAR_SECONDARY) && IS_WEAPON(get_eq_char(ch, WEAR_SECONDARY)) && KNOWS(ch, gsn_dual_wield))
				main = FALSE;
			else
				main = TRUE;
			if (ch->fighting != victim)
				return;
			continue;
		}
		else
		{
			//Dual wield.
			if (number_percent() < get_skill(ch, gsn_dual_wield))
			{								
				one_hit( ch, victim, dt, TRUE );		
				check_improve (ch, gsn_dual_wield, TRUE, 1);		
				main = TRUE;
			}
			check_improve (ch, gsn_dual_wield, FALSE, 1);	
			if ( ch->fighting != victim )
				return;
			continue;
		}
    }
	
	

    return;
}

bool dex_check (CHAR_DATA * ch, int modifier)
{
	int die = 0;
	
	die = number_range(1,20); //20 sided die.
	die += modifier;
	
	if (GET_DEX(ch) > die)
		return TRUE;

	return FALSE;
}

bool str_check (CHAR_DATA * ch, int modifier)
{
	int die = 0;
	
	die = number_range(1,20); //20 sided die.
	die += modifier;
	
	if (GET_STR(ch) > die)
		return TRUE;

	return FALSE;
}

bool con_check (CHAR_DATA * ch, int modifier)
{
	int die = 0;
	
	die = number_range(1,20); //20 sided die.
	die += modifier;
	
	if (GET_CON(ch) > die)
		return TRUE;

	return FALSE;
}

bool int_check (CHAR_DATA * ch, int modifier)
{
	int die = 0;
	
	die = number_range(1,20); //20 sided die.
	die += modifier;
	
	if (GET_INT(ch) > die)
		return TRUE;

	return FALSE;
}

bool wis_check (CHAR_DATA * ch, int modifier)
{
	int die = 0;
	
	die = number_range(1,20); //20 sided die.
	die += modifier;
	
	if (GET_WIS(ch) > die)
		return TRUE;

	return FALSE;
}

bool cha_check (CHAR_DATA * ch, int modifier)
{
	int die = 0;
	
	die = number_range(1,20); //20 sided die.
	die += modifier;
	
	if (GET_CHA(ch) > die)
		return TRUE;

	return FALSE;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance = 0, number = 0;
    CHAR_DATA *vch, *vch_next;

    one_hit (ch, victim, dt, FALSE);

    if (ch->fighting != victim)
        return;

    /* Area attack -- BALLS nasty! */

    if (IS_SET (ch->off_flags, OFF_AREA_ATTACK))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
            vch_next = vch->next;
            if ((vch != victim && vch->fighting == ch))
                one_hit (ch, vch, dt, FALSE);
        }
    }

    if (IS_AFFECTED (ch, AFF_HASTE)
        || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
        one_hit (ch, victim, dt, FALSE);

    if (ch->fighting != victim || dt == gsn_backstab)
        return;

    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

	if (IS_NPC (ch) && IS_SET(ch->off_flags, EXTRA_ATTACK))
		chance += 100;
		
	if (IS_NPC (ch) && IS_SET(ch->off_flags, EXTRA_ATTACK2))
		chance += 100;
		
    if (number_percent () < chance)
    {
        one_hit (ch, victim, dt, FALSE);
        if (ch->fighting != victim)
            return;
    }

    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance = 0;

    if (number_percent () < chance)
    {
        one_hit (ch, victim, dt, FALSE);
        if (ch->fighting != victim)
            return;
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
        return;

    number = number_range (0, 2);

    if (number == 1 && IS_SET (ch->act, ACT_MAGE))
    {
        /*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET (ch->act, ACT_CLERIC))
    {
        /* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range (0, 8);

    switch (number)
    {
        case (0):
            if (IS_SET (ch->off_flags, OFF_BASH))
                do_function (ch, &do_bash, "");
            break;

        case (1):
            if (IS_SET (ch->off_flags, OFF_BERSERK)
                && !IS_AFFECTED (ch, AFF_BERSERK))
                do_function (ch, &do_berserk, "");
            break;


        case (2):
            if (IS_SET (ch->off_flags, OFF_DISARM)
                || (get_weapon_sn (ch, FALSE) != gsn_hand_to_hand
                    && (IS_SET (ch->act, ACT_WARRIOR)
                        || IS_SET (ch->act, ACT_THIEF))))
                do_function (ch, &do_disarm, "");
            break;

        case (3):
            if (IS_SET (ch->off_flags, OFF_KICK))
                do_function (ch, &do_kick, "");
            break;

        case (4):
            if (IS_SET (ch->off_flags, OFF_KICK_DIRT))
                do_function (ch, &do_dirt, "");
            break;

        case (5):
            if (IS_SET (ch->off_flags, OFF_TAIL))
            {
                /* do_function(ch, &do_tail, "") */ ;
            }
            break;

        case (6):
            if (IS_SET (ch->off_flags, OFF_TRIP))
                do_function (ch, &do_trip, "");
            break;

        case (7):
            if (IS_SET (ch->off_flags, OFF_CRUSH))
            {
                /* do_function(ch, &do_crush, "") */ ;
            }
            break;
        case (8):
            if (IS_SET (ch->off_flags, OFF_BACKSTAB))
            {
                do_function (ch, &do_backstab, "");
            }
    }
}

bool check_critical_hit(CHAR_DATA *ch, bool spell, OBJ_DATA *wpn)
{	
	int num = 0;	
	
	
	num = number_percent();
	
	//Vorpal weapons add 5% crit chance.
	if (wpn && wpn->item_type == ITEM_WEAPON && (IS_WEAPON_STAT (wpn, WEAPON_VORPAL)))
		num -= 5;
	
	//Monk's with pressure point knowledge can cause more critical hits. Upro 2/26/2010
	//5% bonus.
	if (get_skill(ch, gsn_pressure_point) > 1)
	{
		if (number_range(1,99) < get_skill(ch, gsn_pressure_point))
		{
			num -= 5;
		}
	}	
	
	if (spell != TRUE)
	{
		if (num < get_melee_crit_chance(ch))
			return TRUE;
	}
	else
	{
		if (num < get_spell_crit_chance(ch))
			return TRUE;
	}
		
	return FALSE;	
}



/*
 * Hit one guy once.
 */
void one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn, skill;
    int dam_type;
	int chance;
    bool result;
	
    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
        return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
        return;

	if (victim->hit < 1 && IS_NPC(victim))
		return;
		
	if (is_affected(ch, gsn_paralyzation) && !con_check(ch, 4))
	{
		SEND("You can't move, you're paralyzed!\r\n",ch);
		return;
	}
    /*
     * Figure out the type of damage message.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY ); //dual wield
		

    if (dt == TYPE_UNDEFINED)
    {
        dt = TYPE_HIT;        
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dt += wield->value[3];
        else
            dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
	{
        if (wield != NULL)
            dam_type = attack_table[wield->value[3]].damage;
        else
            dam_type = attack_table[ch->dam_type].damage;
	}
    else
        dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
        dam_type = DAM_BASH;

    /* get the weapon skill */
	if (!secondary)
		sn = get_weapon_sn (ch, FALSE);
	else
		sn = get_weapon_sn (ch, TRUE);
		
    skill = 20 + get_weapon_skill (ch, sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if (IS_NPC (ch))
    {
        thac0_00 = 20;
        thac0_32 = -4;            /* as good as a thief */
        if (IS_SET (ch->act, ACT_WARRIOR))
            thac0_32 = -10;
        else if (IS_SET (ch->act, ACT_THIEF))
            thac0_32 = -4;
        else if (IS_SET (ch->act, ACT_CLERIC))
            thac0_32 = 2;
        else if (IS_SET (ch->act, ACT_MAGE))
            thac0_32 = 6;
    }
    else
    {
        thac0_00 =ch_class_table[ch->ch_class].thac0_00;
        thac0_32 =ch_class_table[ch->ch_class].thac0_32;
    }
    thac0 = interpolate (ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
        thac0 = thac0 / 2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill / 100;
    thac0 += 5 * (100 - skill) / 100; 			//crappier skill in weapon = crappier thac0

    if (dt == gsn_backstab)
        thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

    switch (dam_type)
    {
        case (DAM_PIERCE):
            victim_ac = GET_AC (victim, AC_PIERCE) / 10;
            break;
        case (DAM_BASH):
            victim_ac = GET_AC (victim, AC_BASH) / 10;
            break;
        case (DAM_SLASH):
            victim_ac = GET_AC (victim, AC_SLASH) / 10;
            break;
        default:
            victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
            break;
    };

    if (victim_ac < -15)
        victim_ac = (victim_ac + 15) / 5 - 15;

    if (!can_see (ch, victim))
        victim_ac -= 4;

    if (victim->position < POS_FIGHTING)
        victim_ac += 4;

    if (victim->position < POS_RESTING)
        victim_ac += 6;

	//Check for confusion spell - updated 9/2/2010 to include other group members
	//being hit than just yourself.
	if (is_affected(ch, gsn_confusion) && (number_percent() < 20))
    {
		victim = ch;		
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
		for (vch = char_list; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if (vch->in_room == NULL)
				continue;
			if (vch->in_room == ch->in_room)
			{
				if (is_same_group(vch, ch) && number_percent() < 50)
				{
					victim = vch;
					break;
				}
			}
		}
	}
	
    /*
     * The moment of excitement!
     */
    while ((diceroll = number_bits (5)) >= 20);

    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {
        /* Miss. */
        damage (ch, victim, 0, dt, dam_type, TRUE);
        tail_chain ();
        return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
        if (!ch->pIndexData->new_format)
        {
            dam = number_range (ch->level / 2, ch->level * 3 / 2);
            if (wield != NULL)
                dam += dam / 2;
        }
        else
            dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

    else
    {
        if (sn != -1)
            check_improve (ch, sn, TRUE, 2);		
		
        if (wield != NULL)
        {
            if (wield->pIndexData->new_format)
                dam = dice (wield->value[1], wield->value[2]) * skill / 100;
            else
                dam = number_range (wield->value[1] * skill / 100,
                                    wield->value[2] * skill / 100);
		
            if (get_eq_char (ch, WEAR_SHIELD) == NULL)    /* no shield = more */
                dam = dam * 11 / 10;
			
			
            /* sharpness! */
            if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
            {
                int percent;

                if ((percent = number_percent ()) <= (skill / 8))
                    dam = 2 * dam + (dam * 2 * percent / 100);
            }
			
			/* vorpal! */
            // if (IS_WEAPON_STAT (wield, WEAPON_VORPAL))
            // {
                // int percent;

                // if ((percent = number_percent ()) <= (skill / 8))
                    // dam = 2 * dam + (dam * 2 * percent / 100);
            // }
        }
        else
            dam =
                number_range (1 + 4 * skill / 100,
                              2 * ch->level / 3 * skill / 100);
    }

    /*
     * Bonuses.
     */
    if (get_skill (ch, gsn_enhanced_damage) > 0)
    {
        diceroll = number_percent ();
        if (diceroll <= get_skill (ch, gsn_enhanced_damage))
        {
            check_improve (ch, gsn_enhanced_damage, TRUE, 4);
            dam = (dam / 9) * 10;
        }
    }

	if (is_affected(victim, gsn_barkskin))
		dam -= number_range (1,6);
	  
	
    if (!IS_AWAKE (victim))
        dam *= 2;
    else if (victim->position < POS_FIGHTING)
        dam = dam * 3 / 2;    
	
	//bad condition of weapon lowers damage -Upro 11/24/09	
	if (wield)
	{
		if (wield->condition < 85 )			
			dam -= ( (100 - wield->condition) / 5);		
		
		//check to see if the weapon is damaged. -Upro 11/24/09	
		chance = 2;
		
		if (IS_SET(wield->extra_flags, ITEM_MAGIC))
			chance -= 1;
		if (IS_SET(wield->extra2_flags, ITEM_RUSTED))
			chance *= 2;
		
		if (number_percent() < chance && !(IS_SET(wield->extra2_flags, ITEM_INDESTRUCTABLE)))				
			wield->condition -= number_range(1, IS_SET(wield->extra_flags, ITEM_MAGIC) ? 2 : 3);	
				
		if (dam <= 0)
			dam = 1;
			
		if (wield->condition < 25 && number_percent() < 25 && wield->level > 10 )
			act( "You realize $p might need some repairing soon.",ch,wield,NULL,TO_CHAR);
	}
	
	if (what_size(ch) > what_size(victim))
	{
		dam += dam * (what_size(ch) - what_size(victim)) / 10;
	}
	
	//Strength modifier for damage.
	int i = 14;
	while (i++ <= get_curr_stat(ch, STAT_STR))
	{
		dam++;
	}	
	
	if (dt == gsn_backstab && wield != NULL)
    {			
        if (wield->value[0] != 2 && wield->value[0] != 1) //not a dagger or short sword
            dam *= (2 + (total_levels(ch) / 10));
        else
            dam *= (2 + (total_levels(ch) / 8));
    }
	

	
	
	
	if ( check_counter( ch, victim, dam, dt ) )
		return;
	else
		result = damage( ch, victim, dam, dt, dam_type, TRUE );
	
	
	if (IS_THIEF(victim) && check_riposte( ch, victim, dam, dt ))
		return;
	else
		result = damage( ch, victim, dam, dt, dam_type, TRUE );
	
	

    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    {
        int dam;

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON))
        {
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;

            if (!saves_spell (level / 2, victim, DAM_POISON))
            {
                SEND ("You feel poison coursing through your veins.",
                              victim);
                act ("$n is poisoned by the venom on $p.",
                     victim, wield, NULL, TO_ROOM);

                af.where = TO_AFFECTS;
                af.type = gsn_poison;
                af.level = level * 3 / 4;
                af.duration = level / 2;
                af.location = APPLY_STR;
                af.modifier = -1;
                af.bitvector = AFF_POISON;
                affect_join (victim, &af);
            }

            /* weaken the poison if it's temporary */
            if (poison != NULL)
            {
                poison->level = UMAX (0, poison->level - 2);
                poison->duration = UMAX (0, poison->duration - 1);

                if (poison->level == 0 || poison->duration == 0)
                    act ("The poison on $p has worn off.", ch, wield, NULL,
                         TO_CHAR);
            }
        }


        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC))
        {
            dam = number_range (1, wield->level / 5 + 1);
            act ("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
            act ("You feel $p drawing your life away.",
                 victim, wield, NULL, TO_CHAR);
            damage (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
            ch->alignment = UMAX (-1000, ch->alignment - 1);
            ch->hit += dam / 2;
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING))
        {
            dam = number_range (1, wield->level / 4 + 1);
            act ("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
            act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
            fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_LIGHT_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p glows with blinding light!", victim, wield, NULL, TO_ROOM);
				act ("$p glows with blinding light!", victim, wield, NULL, TO_CHAR);				
				damage (ch, victim, dam, 0, DAM_LIGHT, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_NEGATIVE_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p radiates negative energy.", victim, wield, NULL, TO_ROOM);
				act ("$p radiates negative energy.", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_NEGATIVE, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FIRE_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p flares with magical flame!", victim, wield, NULL, TO_ROOM);
				act ("$p flares with magical flame!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_FIRE, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_COLD_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("The hilt on $p is suddenly VERY cold!", victim, wield, NULL, TO_ROOM);
				act ("$p freezes over momentarily!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_COLD, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_LIGHTNING_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p crackles with electricity!", victim, wield, NULL, TO_ROOM);
				act ("$p crackles with electricity!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_LIGHTNING, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_AIR_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p manipulates the air around it!", victim, wield, NULL, TO_ROOM);
				act ("$p manipulates the air around it!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_AIR, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_EARTH_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p suddenly grows stone spikes!", victim, wield, NULL, TO_ROOM);
				act ("$p suddenly grows stone spikes!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_EARTH, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_HOLY_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p glows with holy light!", victim, wield, NULL, TO_ROOM);
				act ("$p glows with holy light!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_HOLY, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_WATER_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p suddenly grows watery spikes!", victim, wield, NULL, TO_ROOM);
				act ("$p suddenly grows watery spikes!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_DROWNING, TRUE);
			}
        }
		
		if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_ENERGY_DAM))
        {
			if (number_percent() < 20)
            {
				dam = number_range (1, 6);
				act ("$p flares with magical energy!", victim, wield, NULL, TO_ROOM);
				act ("$p flares with magical energy!", victim, wield, NULL, TO_CHAR);
				damage (ch, victim, dam, 0, DAM_ENERGY, TRUE);
			}
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST))
        {
            dam = number_range (1, wield->level / 6 + 2);
            act ("$p freezes $n.", victim, wield, NULL, TO_ROOM);
            act ("The cold touch of $p surrounds you with ice.",
                 victim, wield, NULL, TO_CHAR);
            cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_COLD, FALSE);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_SHOCKING))
        {
            dam = number_range (1, wield->level / 5 + 2);
            act ("$n is struck by lightning from $p.", victim, wield, NULL,
                 TO_ROOM);
            act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
            shock_effect (victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
        }
    }
    tail_chain ();
    return;
}

bool check_iron_will(CHAR_DATA *ch)
{
	if (get_skill(ch, gsn_iron_will) < 2)
		return FALSE;

	if (number_percent() < con_app[get_curr_stat(ch, STAT_CON)].shock)
	{
		check_improve (ch, gsn_iron_will, TRUE, 2);
		return TRUE;
	}
	
	return FALSE;
}

/*
 * Inflict damage from a hit.
 */
bool damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
             int dam_type, bool show)
{
    OBJ_DATA *corpse;
	OBJ_DATA *obj = NULL;	
    bool immune;
	bool crit = FALSE;
	int mana = 0;
	
    if (victim->position == POS_DEAD)
        return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if (dam > 1200 && dt >= TYPE_HIT)
    {
        bug ("Damage: %d: more than 1200 points!", dam);
        dam = 1200;
        if (!IS_IMMORTAL (ch))
        {
            OBJ_DATA *obj;
            obj = get_eq_char (ch, WEAR_WIELD);
            SEND ("You really shouldn't cheat.\r\n", ch);
            if (obj != NULL)
                extract_obj (obj);
        }

    }
	
	/* damage reduction */
    if (dam > 55)
        dam = (dam - 55) / 2 + 55;
    //if (dam > 80)
    //    dam = (dam - 80) / 2 + 80;
	
	
	if ( dam_type <= DAM_SLASH ) //40 melee damage = 4-16 extra damage.
	{
		dam += number_range((GET_DAMROLL(ch) / 10), ((GET_DAMROLL(ch) * 4) / 10));
	}
	else
	{
		if (IS_WIZARD(ch))
			dam = dam * 10 / 9;
		dam += number_range((get_spell_damage_bonus(ch) / 8), get_spell_damage_bonus(ch) / 2);
	}


    if (victim != ch)
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if (is_safe (ch, victim))
            return FALSE;
        check_killer (ch, victim);

        if (victim->position > POS_STUNNED)
        {
            if (victim->fighting == NULL)
            {
                set_fighting (victim, ch);
                if (IS_NPC (victim) && HAS_TRIGGER_MOB(victim, TRIG_KILL))                    
					p_percent_trigger (victim, NULL, NULL, ch, NULL, NULL, TRIG_KILL);
            }
            if (victim->timer <= 4)
                victim->position = POS_FIGHTING;
        }

        if (victim->position > POS_STUNNED)
        {
            if (ch->fighting == NULL)
                set_fighting (ch, victim);
        }

        /*
         * More charm stuff.
         */
        if (victim->master == ch)
            stop_follower (victim);
    }

    /*
     * Inviso attacks ... not.
     */
    if (IS_AFFECTED (ch, AFF_INVISIBLE))
    {
        affect_strip (ch, gsn_invis);
        affect_strip (ch, gsn_mass_invis);		
        REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
        act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

	//Thieves
	if (is_affected (ch, gsn_hide))
	{
		affect_strip (ch, gsn_hide);
		act ("$n steps forth from the shadows...", ch, NULL, NULL, TO_ROOM);
	}
    /*
     * Damage modifiers.
     */

	 //Elemental spec stuff here - Upro 8/1/07
    if (!IS_NPC(victim) && victim->pcdata->elementalSpec > 0 && dam_type == specTable[victim->pcdata->elementalSpec].damType)
	dam = 9 * dam / 10;

    if (!IS_NPC(ch) && ch->pcdata->elementalSpec > 0 && dam_type == specTable[ch->pcdata->elementalSpec].damType)
		dam = (11 * dam) / 10;	
	
	
	if (!IS_NPC(ch))
	{
		//Ranger favored enemy.
	
		if ((victim->race == ch->favored_enemy) && IS_RANGER(ch))
		{			
			dam = (10 * dam) / 9;
		}
		
		//Weapon specialization stuff here - Upro 1/7/2010		
		
		
		if ( ch->pcdata->weaponSpec > 0 && ch->pcdata->weaponSpec < 9)
		{
			obj = get_eq_char (ch, WEAR_WIELD);
			if (obj && obj->item_type == ITEM_WEAPON)
			{
				if (obj->value[0] == ch->pcdata->weaponSpec)
				{
					dam = (10 * dam) / 9;				
				}			
			}
		}
		
		
		
		//// Hapkido for Druids and Monks 12/12/2011 ////
		
		if (IS_DRUID(ch) || IS_MONK(ch))
		{
			if (KNOWS(ch, gsn_hapkido))
			{
				obj = get_eq_char (ch, WEAR_WIELD);
				if (IS_WEAPON(obj))
				{
					if (obj->value[0] == WEAPON_STAFF)
						dam = (dam * 10) / 9;
				}
			}
		}
		
		//// Weapon expertise 11/2011 ////
		
		if (IS_FIGHTER(ch))
		{
			int i = 0;
			
			obj = get_eq_char (ch, WEAR_WIELD);
			if (obj && obj->item_type == ITEM_WEAPON)
			{
				for (i = 0; i < MAX_WEAPON; i++)
				{
					if (obj->value[0] == ch->expertise[i])
					{
						dam = (10 * dam) / 9;	
						break;
					}
				}
			}
		}
		//
		// Monk damage mod: 2/13/2010
		if (get_skill(ch, gsn_iron_fist) > 1 && number_percent() < get_skill(ch, gsn_iron_fist))
		{
			obj = get_eq_char (ch, WEAR_WIELD);
			
			if (!obj)
			{
				dam = (11 * dam) / 10;
			}
		}
	}
	//Bulk damage reduction
	if (!IS_NPC(ch) && !IS_NPC(victim))	
	{
		if (get_bulk(victim) > 30)
		   dam -= (15 + ((get_bulk(victim) - 30) / 5));
		else dam -= (get_bulk(victim) / 2);
	}
	
    if (dam > 1 && !IS_NPC (victim)
        && victim->pcdata->condition[COND_DRUNK] > 10)
        dam = 9 * dam / 10;

    if (dam > 1 && IS_AFFECTED (victim, AFF_SANCTUARY))
        dam /= 2;

    if (dam > 1 && ((IS_AFFECTED (victim, AFF_PROTECT_EVIL) && IS_EVIL (ch))
                    || (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
                        && IS_GOOD (ch))))
        dam -= dam / 4;
	if (!IS_NPC(ch))
	{
		switch (ch->grank)	
		{
			default: break;
			case G_FRONT:
				dam = dam * 11 / 10; break;
			case G_BACK:
				dam = dam * 9 / 10; break;
			case G_FLANK:	
				if (IS_THIEF(ch)) 
					dam = dam * 5 / 4;
				else
					dam = dam * 10 / 9; 
				
				break;
				
		}
	}
	
	if (!IS_NPC(victim))
	{
		switch (victim->grank)	
		{
			default: break;
			case G_FRONT:
				break;
			case G_BACK:
				dam = dam * 9 / 10; break;
			case G_FLANK:	
				break;
				
		}
	}
	
	immune = FALSE;

	if (dam < 0)
		dam = 0;
    /*
     * Check for parry, and dodge. (and now blink, displacement, shield block, hand parry, and evasion. - Upro)
     */
    if (dt >= TYPE_HIT && ch != victim)
    {			
        if (check_parry (ch, victim))
            return FALSE;
        if (check_dodge (ch, victim))
            return FALSE;
		if (check_presence (ch, victim))
			return FALSE;
        if (check_blink	(ch, victim))
			return FALSE;		
		if (check_shield_block (ch, victim))
            return FALSE;
		if (check_displacement (ch, victim))
			return FALSE;
		if (check_hand_parry (ch, victim))
			return FALSE;
		if (check_evasion (ch, victim))
			return FALSE;
    }

	//The following is updated based on the new resistances table for PC characters. Upro 2/11/2020
	if (!IS_NPC(victim))
	{
		int dam_percent = 100; //normal damage.
		dam_percent += resist_table[victim->race].percent[dam_type]; //adjust based on resistances.
		
		if (dam_type == DAM_FIRE || dam_type == DAM_COLD || dam_type == DAM_EARTH || dam_type == DAM_LIGHTNING || dam_type == DAM_AIR || dam_type == DAM_DROWNING)
		{
			if (is_affected(victim, gsn_elemental_protection))
				dam_percent -= 10;
		}
		
		if (dam_type == DAM_NEGATIVE)
		{
			if (is_affected(victim, gsn_negative_plane_protection))
				dam_percent -= 25;
		}		
		
		if (dam_percent == 0) //avoid trying to divide by/with zero. This also means they are immune.
			dam = 0;
		else
			dam = (dam * dam_percent) / 100;
	}
	else
	{
		//old school resistances for NPCs
		switch (check_immune (victim, dam_type))
		{
			case (IS_IMMUNE):
				immune = TRUE;
				dam = 0;
				break;
			case (IS_RESISTANT):
				dam -= dam / 4;
				break;
			case (IS_VULNERABLE):
				dam += dam / 4;
				break;
		}
	}
	//Do a critical hit 2/1/2010 Upro, updated june 2010 for spell crits	
	if ( dam_type <= DAM_SLASH )
	{
		if (check_critical_hit(ch, FALSE, obj))
		{
			//cause disablement
			//perm damage
			if (IS_NPC(victim) && (victim->pIndexData) && IS_SET(victim->act2, ACT2_INANIMATE))
				crit = FALSE;
			else
			{
				victim->bleeding += number_range(1,2);
									
				if (victim->bleeding > 10)
					victim->bleeding = 10;
				
				crit = TRUE;
			}
		}
	}
	else
	{
		if (check_critical_hit(ch, TRUE, obj))
		{
			crit = TRUE;
		}
	}
	
	if (crit)
		dam *= 1.5;
	
    if (show)
        dam_message (ch, victim, dam, dt, dam_type, immune, crit);

    if (dam == 0)
        return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
	 
	//Figure out hit location: 2/1/2010 Upro
	int hitloc = 0;
	char hitbuf[MSL];
	hitloc = number_range(0, 8);
	switch (hitloc)
	{
		default: break;
		case ARM:
			if (number_percent() < 50)
				sprintf (hitbuf, "left arm");
			else
				sprintf (hitbuf, "right arm");
			break;
		case LEG:
			if (number_percent() < 50)
				sprintf (hitbuf, "left leg");
			else
				sprintf (hitbuf, "right leg");
			break;
		case HAND:
			if (number_percent() < 50)
				sprintf (hitbuf, "left hand");
			else
				sprintf (hitbuf, "right hand");
			break;
		case FOOT:
			if (number_percent() < 50)
				sprintf (hitbuf, "left foot");
			else
				sprintf (hitbuf, "right foot");
			break;
		case CHEST:
			sprintf (hitbuf, "chest");
			break;
		case STOMACH:
			sprintf (hitbuf, "stomach");
			break;
		case HEAD:
			sprintf (hitbuf, "head");
			break;
		case FACE:
			sprintf (hitbuf, "face");
			break;
		case NECK:
			sprintf (hitbuf, "neck");
			break;		
	}
	
	//Mana shield 12/21/2009 Upro
	if (is_affected(victim, gsn_mana_shield))
	{
		mana = dam / 5;
		dam = dam * 4 / 5;
		victim->mana -= mana;
		if (victim->mana < 1)
		{
			affect_strip(victim, gsn_mana_shield);
			SEND ("Your mana shield shatters into a million pieces!\r\n",victim);
		}
		victim->hit -= dam;
	}
	else
		victim->hit -= dam;
	
	if (is_affected(victim, gsn_thorns))
	{
		ch->hit -= dam / 15;
		act("$n is damaged by the thorns surrounding $N!", ch, NULL, victim, TO_ROOM);
	}
	
	if (is_affected(victim, gsn_blade_barrier))
	{
		ch->hit -= dam / 15;
		act("$n is damaged by the blade barrier surrounding $N!", ch, NULL, victim, TO_ROOM);
	}
	
    if (!IS_NPC (victim) && total_levels(victim) >= LEVEL_IMMORTAL && victim->hit < 1)
        victim->hit = 1;
	
	if (!IS_NPC(ch) && ch->mercy == TRUE && victim->hit < 1)
	{		
		victim->hit = 1;
		SEND ("You show some {Cmercy{x\r\n",ch);
		SEND ("You have been shown {Cmercy{x!",victim);
		ch->fighting = NULL;
		victim->fighting = NULL;
		return FALSE;
	}
	
    update_pos (victim);

	if (!IS_NPC(victim) || !IS_SET(victim->act2, ACT2_INANIMATE))	
	{
		switch (victim->position)
		{
			case POS_MORTAL:
				act ("$n is mortally wounded, and will die soon, if not aided.",
					 victim, NULL, NULL, TO_ROOM);
				SEND
					("You are mortally wounded, and will die soon, if not aided.\r\n",
					 victim);
				break;

			case POS_INCAP:
				act ("$n is incapacitated and will slowly die, if not aided.",
					 victim, NULL, NULL, TO_ROOM);
				SEND
					("You are incapacitated and will slowly die, if not aided.\r\n",
					 victim);
				break;

			case POS_STUNNED:
				act ("$n is stunned, but will probably recover.",
					 victim, NULL, NULL, TO_ROOM);
				SEND ("You are stunned, but will probably recover.\r\n",
							  victim);
				break;

			case POS_DEAD:
				if (!IS_NPC (victim))
				{			
					if ( IS_MONK(victim) )
					{
						if (KNOWS(victim, gsn_iron_will) && check_iron_will(victim) && ress_shock(victim, ch->hit < -50 ? 25 : 5)) 
						//once ch's hp gets into negatives, start getting a penalty to the ress_shock rolls.
						{						
							victim->position = POS_STANDING;
							victim->hit = 1;
							SEND("Your iron will keeps you alive!!\r\n",victim);
							act("$n forces their body to continue on!!\r\n",victim,NULL,ch,TO_ROOM);							
							return FALSE;
						}
					}										
					act ("{R$n {Ris DEAD!!{x", victim, 0, 0, TO_ROOM);
					SEND ("{RYou have been KILLED!!{x\r\r\n\n", victim);
					if (victim == ch && total_levels(victim) < 20)	
					{
						SEND("\r\n\r\n",ch);
						SEND ("{BYour corpse has been transported to the center of {BPiety Square{x.\r\n",ch);			
						SEND("\r\n\r\n",ch);
					}
				}
				
				break;

			default:
				if (dam > victim->max_hit / 4)
				{
					SEND ("{RThat really did HURT!{x\r\n", victim);
					victim->bleeding++;
					if (!system_shock(victim, 0))				
						victim->position = POS_STUNNED;										
				}
				if ( victim->hit < (victim->max_hit / 10) )
				{
					if (number_percent() < 15)									
						victim->bleeding++;									
					SEND ("You sure are {RBLEEDING{x!\r\n", victim);
				}
				if (victim->bleeding > 10)
					victim->bleeding = 10;
				break;
		}
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
	 
    if (!IS_AWAKE (victim) || victim->position == POS_DEAD)
        stop_fighting (victim, FALSE);

    /*
     * Payoff for killing things.
     */
    if (victim->position == POS_DEAD)
    {
        group_gain (ch, victim);
			       
		//Award pkill/pdeath.
		if (!IS_NPC(ch) && !IS_NPC(victim) && ch != victim)
		{
			ch->pkill += 1;
			victim->pdeath += 1;				
		}
				
		sprintf (log_buf, "%s killed by %s at %ld (%s)", victim->name, (IS_NPC (ch) ? ch->short_descr : ch->name),
                     ch->in_room->vnum, ch->in_room->name);
		log_string (log_buf);			
		
            /*
             * Dying penalty:
             * Lose exp.
             */            
            gain_exp (victim, (-100 * victim->level), FALSE);
			if (IS_MCLASSED (ch))
			{
				gain_exp (victim, (-100 * victim->level2), TRUE);
			}
    

        sprintf (log_buf, "%s got toasted by %s at %s [room %ld]",
                 (IS_NPC (victim) ? victim->short_descr : victim->name),
                 (IS_NPC (ch) ? ch->short_descr : ch->name),
                 ch->in_room->name, ch->in_room->vnum);

        if (IS_NPC (victim))
		{
            wiznet (log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);
			ch->mkill += 1;
		}
        else
            wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);
		
        /*
         * Death trigger
         */
        if (IS_NPC (victim) && HAS_TRIGGER_MOB(victim, TRIG_DEATH))
        {
            victim->position = POS_STANDING;            
			p_percent_trigger (victim, NULL, NULL, ch, NULL, NULL, TRIG_DEATH);
        }
		
		/*if (IS_NPC(victim))
		{
			QUEST_DATA *quest;
			char buf2[MSL];
			for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
			{
				if (quest->pIndexData && IS_SET(quest->pIndexData->type, A))
				{
					if (victim->pIndexData->vnum == quest->pIndexData->target_mob_vnum)
					{						
						quest->mob_multiples++;
						if (!quest->mob_multiples > quest->pIndexData->mob_mult)
						{
							sprintf(buf2, "You have killed %d of the %d monsters necessary for the quest {g%s{x!\r\n", quest->mob_multiples, quest->pIndexData->mob_mult, quest->pIndexData->name);
							SEND(buf2,ch);
						}
						if (has_completed_quest(ch, quest->pIndexData))
						{
							sprintf(buf2, "You've met the requirements for {g%s{x! Go turn it in!\r\n",quest->name);
							SEND(buf2,ch);
						}
					}
				}
			}
		}*/
		
		raw_kill (victim);
        /* dump the flags */		
        if (ch != victim && !IS_NPC (ch) && !is_same_clan (ch, victim))
        {
            if (IS_SET (victim->act, PLR_KILLER))
                REMOVE_BIT (victim->act, PLR_KILLER);
            else
                REMOVE_BIT (victim->act, PLR_THIEF);
        }
		
		int i = 0;
		char buf[MSL];
		//can't tell on yourself, attacker must be PC, must be in a city, victim must be NPC.
		if (ch != victim && !IS_NPC(ch) && IS_NPC(victim) && ch->in_room->sector_type == SECT_CITY)
		{		
			for (i = 0; i <= MAX_FACTION; i++)
			{
				CHAR_DATA *rch;
				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
				{
					if (rch != ch && rch != victim && rch->master != ch)
					{
						if (!str_cmp(ch->in_room->area->name, faction_table[i].name))
						{
							ch->faction_rep[i] -= (number_fuzzy(total_levels(victim) * number_range(1,3)));
							sprintf(buf, "You lose %sfaction reputation{x for killing in the city!\r\n",BG_BLACK);
							SEND(buf, ch);
							//SEND("You lose {2faction reputation{x for killing in the city!\r\n",ch);
							break;
						}
						break;
					}
				}
			}
		}
        /* RT new auto commands */

        if (!IS_NPC (ch)
            && (corpse =
                get_obj_list (ch, "corpse", ch->in_room->contents)) != NULL
            && corpse->item_type == ITEM_CORPSE_NPC
            && can_see_obj (ch, corpse))
        {
            OBJ_DATA *coins;

            corpse = get_obj_list (ch, "corpse", ch->in_room->contents);
			
			if (!IS_NPC(ch))
				corpse->killer = str_dup(ch->name);
			else
				corpse->killer = str_dup(ch->pIndexData->short_descr);
				
			
            if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)
            {                    /* exists and not empty */
                do_function (ch, &do_get, "all corpse");				
            }
			
			if (IS_SET (ch->act, PLR_AUTO_SCROLL_LOOT) && corpse && corpse->contains)
			{
				do_function (ch, &do_get, "all.scroll corpse");
			}

            if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&    /* exists and not empty */
                !IS_SET (ch->act, PLR_AUTOLOOT))
            {
                if ((coins = get_obj_list (ch, "gcash", corpse->contains))
                    != NULL)
                {
                    do_function (ch, &do_get, "all.gcash corpse");
                }
            }

            if (IS_SET (ch->act, PLR_AUTOSAC))
            {
                if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
                    && corpse->contains)
                {
                    return TRUE;    /* leave if corpse has treasure */
                }
                else
                {
                    do_function (ch, &do_sacrifice, "corpse");
                }
            }
        }
        return TRUE;
	}
    if (victim == ch)
        return TRUE;

    /*
     * Take care of link dead people.
     */
    if (!IS_NPC (victim) && victim->desc == NULL)
    {
        if (number_range (0, victim->wait) == 0)
        {
            do_function (victim, &do_recall, "");
            return TRUE;
        }
    }

    /*
     * Wimp out?
     */
    if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
        if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
             && victim->hit < victim->max_hit / 5)
            || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
                && victim->master->in_room != victim->in_room))
        {
            do_function (victim, &do_flee, "");
        }
    }

    if (!IS_NPC (victim)
        && victim->hit > 0
        && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
    {
        do_function (victim, &do_flee, "");
    }

    tail_chain ();
    return TRUE;
}

bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char buf[MSL];

    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim->fighting == ch || victim == ch)
        return FALSE;

    if (IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL)
        return FALSE;

	if (IS_NPC(victim) && IS_SET(victim->act2, ACT2_MOBINVIS))
		return TRUE;
		
	if (is_affected(victim, gsn_globe_invulnerability))
		return FALSE;
		
    /* killing mobiles */
    if (IS_NPC (victim))
    {

        /* safe room? */
        if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
        {
            SEND ("Not in this room.\r\n", ch);
            return TRUE;
        }
		
		if (IS_SET (victim->act2, ACT2_PEACEFUL))
		{
			SEND ("They're a pacifist.\r\n", ch);
			return TRUE;
		}
		
		if (IS_SET (victim->in_room->room_flags, ROOM_NOFIGHT))
		{
			SEND ("Fighting is not allowed in this room.\r\n",ch);
			return TRUE;
		}

        if (victim->pIndexData->pShop != NULL)
        {
            SEND ("The shopkeeper wouldn't like that.\r\n", ch);
            return TRUE;
        }

        /* no killing healers, trainers, etc */
        if (IS_SET (victim->act, ACT_TRAIN)
            || IS_SET (victim->act, ACT_PRACTICE)
            || IS_SET (victim->act, ACT_IS_HEALER)
            || IS_SET (victim->act, ACT_IS_CHANGER))
        {
            sprintf (buf, "I don't think %s would approve.\r\n", god_table[ch->god].name);
			SEND(buf,ch);
            return TRUE;
        }

        if (!IS_NPC (ch))
        {
            /* no pets */
            if (IS_SET (victim->act, ACT_PET))
            {
                act ("But $N looks so cute and cuddly...",
                     ch, NULL, victim, TO_CHAR);
                return TRUE;
            }

            /* no charmed creatures unless owner */
            if (IS_AFFECTED (victim, AFF_CHARM) && ch != victim->master)
            {
                SEND ("You don't own that monster.\r\n", ch);
                return TRUE;
            }
        }
    }
    /* killing players */
    else
    {
        /* NPC doing the killing */
        if (IS_NPC (ch))
        {
            /* safe room check */
            if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
            {
                SEND ("Not in this room.\r\n", ch);
                return TRUE;
            }

            /* charmed mobs and pets cannot attack players while owned */
            if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
                && ch->master->fighting != victim)
            {
                SEND ("Players are your friends!\r\n", ch);
                return TRUE;
            }
        }
        /* player doing the killing */
        else
        {
            if (!is_clan (ch))
            {
                SEND ("Join a clan if you want to kill players.\r\n",
                              ch);
                return TRUE;
            }

            if (IS_SET (victim->act, PLR_KILLER)
                || IS_SET (victim->act, PLR_THIEF))
                return FALSE;

            if (!is_clan (victim))
            {
                SEND ("They aren't in a clan, leave them alone.\r\n",
                              ch);
                return TRUE;
            }

            if (ch->level > victim->level + 8)
            {
                SEND ("Pick on someone your own size.\r\n", ch);
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool is_safe_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
        return TRUE;

    if (victim->fighting == ch || victim == ch)
        return FALSE;

    if (IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL && !area)
        return FALSE;

    /* killing mobiles */
    if (IS_NPC (victim))
    {
        /* safe room? */
        if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
            return TRUE;

        if (victim->pIndexData->pShop != NULL)
            return TRUE;

        /* no killing healers, trainers, etc */
        if (IS_SET (victim->act, ACT_TRAIN)
            || IS_SET (victim->act, ACT_PRACTICE)
            || IS_SET (victim->act, ACT_IS_HEALER)
            || IS_SET (victim->act, ACT_IS_CHANGER))
            return TRUE;

        if (!IS_NPC (ch))
        {
            /* no pets */
            if (IS_SET (victim->act, ACT_PET))
                return TRUE;

            /* no charmed creatures unless owner */
            if (IS_AFFECTED (victim, AFF_CHARM)
                && (area || ch != victim->master))
                return TRUE;

            /* legal kill? -- cannot hit mob fighting non-group member */
            if (victim->fighting != NULL
                && !is_same_group (ch, victim->fighting)) return TRUE;
        }
        else
        {
            /* area effect spells do not hit other mobs */
            if (area && !is_same_group (victim, ch->fighting))
                return TRUE;
        }
    }
    /* killing players */
    else
    {
        if (area && IS_IMMORTAL (victim) && victim->level > LEVEL_IMMORTAL)
            return TRUE;

        /* NPC doing the killing */
        if (IS_NPC (ch))
        {
            /* charmed mobs and pets cannot attack players while owned */
            if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
                && ch->master->fighting != victim)
                return TRUE;

            /* safe room? */
            if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
                return TRUE;

            /* legal kill? -- mobs only hit players grouped with opponent */
            if (ch->fighting != NULL && !is_same_group (ch->fighting, victim))
                return TRUE;
        }

        /* player doing the killing */
        else
        {
            if (!is_clan (ch))
                return TRUE;

            if (IS_SET (victim->act, PLR_KILLER)
                || IS_SET (victim->act, PLR_THIEF))
                return FALSE;

            if (!is_clan (victim))
                return TRUE;

            if (ch->level > victim->level + 8)
                return TRUE;
        }

    }
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer (CHAR_DATA * ch, CHAR_DATA * victim)
{
    char buf[MAX_STRING_LENGTH];
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL)
        victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if (IS_NPC (victim)
        || IS_SET (victim->act, PLR_KILLER)
        || IS_SET (victim->act, PLR_THIEF))
        return;

    /*
     * Charm-o-rama.
     */
    if (IS_SET (ch->affected_by, AFF_CHARM))
    {
        if (ch->master == NULL)
        {
            char buf[MAX_STRING_LENGTH];

            sprintf (buf, "Check_killer: %s bad AFF_CHARM",
                     IS_NPC (ch) ? ch->short_descr : ch->name);
            bug (buf, 0);
            affect_strip (ch, gsn_charm_person);
            REMOVE_BIT (ch->affected_by, AFF_CHARM);
            return;
        }
/*
    SEND( "*** You are now a KILLER!! ***\r\n", ch->master );
      SET_BIT(ch->master->act, PLR_KILLER);
*/

        stop_follower (ch);
        return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if (IS_NPC (ch)
        || ch == victim || ch->level >= LEVEL_IMMORTAL || !is_clan (ch)
        || IS_SET (ch->act, PLR_KILLER) || ch->fighting == victim)
        return;

    //SEND ("*** You are now a KILLER!! ***\r\n", ch);
    //SET_BIT (ch->act, PLR_KILLER);
    sprintf (buf, "$N is attempting to murder %s", victim->name);
    wiznet (buf, ch, NULL, WIZ_FLAGS, 0, 0);
    save_char_obj (ch);
    return;
}



/*
 * Check for evasion (monks)
 */

bool check_evasion (CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;
	
	if (!IS_KNOWN(ch, gsn_evasion));
		return FALSE;
	
	if (!IS_AWAKE (victim));
		return FALSE;
		
	chance = get_skill (victim, gsn_evasion) / 3;	

	chance += defense_mods(victim);	
		
	if (number_percent() >= chance + victim->level - ch->level)
		return FALSE;
		
	act ("You reflexively evade $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N evades your attack!", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_evasion, TRUE, 1);
	
	return TRUE;
}
 
/*
 * Check for parry.
 */
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;	
	
	OBJ_DATA * wpn;
	
	if (get_skill (victim, gsn_parry) < 1)
		return FALSE;
	
    if (!IS_AWAKE (victim))
        return FALSE;
	/* always parries with main weapon... need to fix that -Upro */
    chance = get_skill (victim, gsn_parry) / 2;
	
    if (get_eq_char (victim, WEAR_WIELD) == NULL && get_eq_char (victim, WEAR_SECONDARY) == NULL)
    {
        if (IS_NPC (victim))
            chance /= 2;
        else
            return FALSE;
    }
	wpn = get_eq_char(victim, WEAR_WIELD);
	

    if (!can_see (ch, victim))
        chance /= 2;

	chance += defense_mods(victim);	

	chance -= 15; // offset the weapon skills
    chance += get_weapon_skill(victim, get_weapon_sn(victim, FALSE) ) / 10;
    chance += get_weapon_skill(victim, get_weapon_sn(ch, FALSE) ) / 10;	

    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;
		
    act ("You parry $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N parries your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_parry, TRUE, 2);
	
	/* Check for secondary weapon, then check to see if either weapon is damaged - Upro */
	chance = 1;
	if (number_percent () < 50 && get_eq_char(victim, WEAR_SECONDARY) != NULL)
	{
		wpn = get_eq_char(victim, WEAR_SECONDARY);
		if (wpn)
		{
			if (IS_SET(wpn->extra2_flags, ITEM_RUSTED))
				chance *= 2;
			if (wpn && number_percent() <= chance)
				wpn->condition -= 1;
		}
	}
	else
	{
		
		wpn = get_eq_char(victim, WEAR_WIELD);
		if (wpn)
		{
			if (IS_SET(wpn->extra2_flags, ITEM_RUSTED))
				chance *= 2;
			if (wpn && number_percent() <= chance)
				wpn->condition -= 1;
		}
	}
	
    return TRUE;
}

/*
 * Check for parry.
 */
bool check_hand_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;	
	
	//OBJ_DATA * wpn;
	
	if (!KNOWS(victim, gsn_hand_parry))
		return FALSE;
	
    if (!IS_AWAKE (victim))
        return FALSE;
		
    chance = get_skill (victim, gsn_hand_parry) / 2;
	
    if ( get_eq_char (victim, WEAR_WIELD) != NULL )
		return FALSE;
    

    if (!can_see (ch, victim))
        chance *= 2;
	if (!can_see (victim, ch))
		chance /= 2;

	chance += defense_mods(victim);	
		
    if (number_percent () >= chance + (victim->level - ch->level))
	{
		check_improve (victim, gsn_hand_parry, FALSE, 1);
        return FALSE;
	}
    act ("You parry $n's attack with your hands.", ch, NULL, victim, TO_VICT);
    act ("$N parries your attack with their hands.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_hand_parry, TRUE, 1);
		
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;
	OBJ_DATA *shield;
	
    if (!IS_AWAKE (victim))
        return FALSE;

	if (get_skill (victim, gsn_shield_block) < 1)
		return FALSE;
		
	/* this basically equivalates to a 23% chance total - Upro */
    chance = get_skill (victim, gsn_shield_block) / 5 + 3;

	if (what_size(victim) < SIZE_MEDIUM)
		chance -= 5;
	if (what_size(victim) > SIZE_MEDIUM)
		chance += 5;
	if (what_size(victim) > SIZE_LARGE)
		chance += 5;
	if (what_size(victim) > SIZE_HUGE)
		chance += 5;
	
	chance += defense_mods(victim);	
	
    if (get_eq_char (victim, WEAR_SHIELD) == NULL)
        return FALSE;

    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You block $n's attack with your shield.", ch, NULL, victim,
         TO_VICT);
    act ("$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR);	
	
	/* Possible chance of damaging shield by blocking - Upro */
	shield = get_eq_char( victim, WEAR_SHIELD );
	int breakage = 0;
	breakage = number_percent();
	//If they have a durable shield, reduce chance of it taking damage.
	if (number_percent() < (mat_table[get_material(shield->material)].durability))
	{
		breakage += 5;
	}
	if (shield && breakage < 10)	
		shield->condition -= number_range(1,3);
		
    check_improve (victim, gsn_shield_block, TRUE, 2);
	
	//Retribution check.
	if (is_affected(victim, gsn_retribution) && number_percent() < 10)
	{		
		one_hit (victim, ch, gsn_retribution, FALSE);		
		// ch->hit -= number_range( (victim->level / 8 + 1), (victim->level / 5 + 1) );		
		act("$n is damaged by $N's retribution!", ch, NULL, victim, TO_ROOM);
	}
    return TRUE;
}

// Check for displacement
bool check_displacement (CHAR_DATA *ch, CHAR_DATA * victim)
{
	int chance = 0;
	
	if (!IS_AWAKE (victim))
        return FALSE;
		
	if (is_affected(victim, gsn_displacement))
	{
		chance = number_percent();
		//Attackers with high int/wis can see through the spell.
		if (get_curr_stat(ch, STAT_INT) > 18)
		{
			chance += (25 - get_curr_stat(ch, STAT_INT));			
		}
		if (get_curr_stat(ch, STAT_WIS) > 18)
		{
			chance += (25 - get_curr_stat(ch, STAT_WIS));			
		}
		if (ch->level > victim->level)
		{
			chance += (ch->level - victim->level);
		}
		if (chance < 40)
		{
			act ("$n swings at you but is quite out of range.", ch, NULL, victim, TO_VICT);
			act ("you miss $N, apparently thinking they were closer than that.", ch, NULL, victim, TO_CHAR);
			return TRUE;
		}
	}
	else
		return FALSE;
		
	return FALSE;
}

// Check for blink spell
bool check_blink (CHAR_DATA *ch, CHAR_DATA * victim)
{
	int chance = 0;
	
	if (!IS_AWAKE (victim))
        return FALSE;
		
	if (is_affected(victim, gsn_blink))
	{
		chance = number_percent();
		if (get_curr_stat(victim, STAT_DEX) > 18)
		{
			chance -= (25 - get_curr_stat(victim, STAT_DEX));			
		}
		if (ch->level > victim->level)
		{
			chance += (ch->level - victim->level);
		}
		if (chance < 40)
		{
			act ("You blink out of existance for a moment, avoiding $n's attack.", ch, NULL, victim, TO_VICT);
			act ("$N blinks out of existance, avoiding your attack.", ch, NULL, victim, TO_CHAR);
			return TRUE;
		}
	}
	else
		return FALSE;
		
	return FALSE;
}


int defense_mods(CHAR_DATA *ch)
{
	int mod = 0;

	//Wearing a lot of heavy armor reduces mod.
	if (ch->bulk > 15)
		mod -= 5;
	if (ch->bulk > 25)
		mod -= 5;
	if (ch->bulk > 35)
		mod -= 5;
	
	//Dex mod.
	mod += dex_app[GET_DEX(ch)].reaction_adj;
	
	//Encumbrance.
	if (ENCUMBERED(ch))
		mod -= 10;
		
	if (!IS_NPC(ch))
	{
		//Being drunk sucks when fighting.
		if (IS_DRUNK(ch))
			mod -= 5;
		if (IS_HAMMERED(ch))
			mod -= 10;
	}
	
	return mod;
}

//Presence of mind for monks.
bool check_presence (CHAR_DATA *ch, CHAR_DATA * victim)
{
	int chance;
	
	if (!IS_AWAKE (victim))
        return FALSE;

    chance = get_skill (victim, gsn_presence_of_mind) / 2;

    if (!can_see (victim, ch))
        chance /= 2;

	chance += defense_mods(victim);	
		
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N dodges your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_presence_of_mind, TRUE, 2);
    return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;

    chance = get_skill (victim, gsn_dodge) / 2;

	if (get_skill (victim, gsn_presence_of_mind) > 0)
		chance += (get_skill(victim, gsn_presence_of_mind) / 10);		
	
	
    if (!can_see (victim, ch))
        chance /= 2;

	chance += defense_mods(victim);	
		
    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N dodges your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_dodge, TRUE, 2);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos (CHAR_DATA * victim)
{
    if (victim->hit > 0)
    {
        if (victim->position <= POS_STUNNED)
            victim->position = POS_STANDING;
        return;
    }

    if (IS_NPC(victim) && victim->hit < 1)
    {
        victim->position = POS_DEAD;		
        return;
    }    
	
	if (victim->hit < -10)
    {
        victim->position = POS_DEAD;		
        return;
    }
    else if (victim->hit <= -6)
        victim->position = POS_MORTAL;
    else if (victim->hit <= -3)
        victim->position = POS_INCAP;
    else
        victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting (CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (ch->fighting != NULL)
    {
        bug ("Set_fighting: already fighting", 0);
        return;
    }

    if (IS_AFFECTED (ch, AFF_SLEEP))
        affect_strip (ch, gsn_sleep);

    ch->fighting = victim;
    ch->position = POS_FIGHTING;
	if (!IS_NPC(victim) && !IS_NPC(ch))
	{
		ch->pk_timer = number_range(5,7);
		victim->pk_timer = number_range(3,5);
	}
	

    return;
}



/*
 * Stop fights.
 */
void stop_fighting (CHAR_DATA * ch, bool fBoth)
{
    CHAR_DATA *fch;

    for (fch = char_list; fch != NULL; fch = fch->next)
    {
        if (fch == ch || (fBoth && fch->fighting == ch))
        {
            fch->fighting = NULL;
            fch->position = IS_NPC (fch) ? fch->default_pos : POS_STANDING;
            update_pos (fch);
        }
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse (CHAR_DATA * ch, CHAR_DATA *killer)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
	OBJ_DATA *random;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
	int chance;
	int sn;
	ROOM_INDEX_DATA *location;
	
	location = get_room_index ( ROOM_VNUM_MORGUE );
	
    if (IS_NPC(ch))
    {
		if (IS_SET(ch->act2, ACT2_INANIMATE))
			return;
        name = ch->short_descr;
		
        corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);		
        corpse->timer = number_range (4, 8);
        /* Only bipeds and dragons can drop money now */ 
		/* Part of the economy changes - Upro		  */
		if ( (IS_SET (ch->form, FORM_BIPED) || IS_SET (ch->form, FORM_DRAGON)) )
		{
			if (ch->gold > 0 || ch->silver > 0)
			{
				if (double_gold == TRUE && IS_NPC(ch))
				{
					ch->gold *= 2;
					ch->silver += 2;
				}
				obj_to_obj (create_money (ch->gold, ch->silver), corpse);
				ch->gold = 0;
				ch->silver = 0;
			}
		}
		/* randomly drop scrolls, but must be a bipedal/dragon creature (aka a humanoid/dragon) */
		/* i don't think wolves and shit would be carrying around scrolls. -Upro */
		chance = number_range(1,200);		
		
		if (ch->spec_fun == spec_lookup("spec_cast_mage") || ch->spec_fun == spec_lookup("spec_cast_cleric"))
			chance -= 5;
		
		if ( (IS_SET (ch->act2, ACT2_ELITE)) )
			chance -= 5;
		if ( (IS_SET (ch->act2, ACT2_BOSS)) )
			chance -= 10;
		
		if ( (IS_SET (ch->form, FORM_BIPED) || IS_SET (ch->form, FORM_DRAGON)) && chance <= 16 )
		{	
			sn = (number_range(1,MAX_SKILL));					
			while ( skill_table[sn].spell_fun == spell_null || skill_table[sn].name == NULL || skill_table[sn].can_scribe == FALSE || (skill_table[sn].spell_level < (total_levels(ch) / 5)) || skill_table[sn].spell_level > 8 )
			{			
				if (number_percent() < 60)
					sn = (number_range(1,MAX_SKILL));																	
				else
					sn = get_random_healing_spell(total_levels(ch));
			}		
			int type;
			type = number_range(1,4);
			
			obj_to_obj ( create_random_consumable(type, number_fuzzy(total_levels(ch))), corpse );
		}
        corpse->cost = 0;
    }
    else
    {
        name = ch->name;
        corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer = number_range (25, 40);
        REMOVE_BIT (ch->act, PLR_CANLOOT);
		SET_BIT(corpse->extra2_flags, ITEM_CAN_PUSH);
		SET_BIT(corpse->extra2_flags, ITEM_CAN_PULL);
        
		corpse->owner = str_dup (ch->name);
		if (ch->gold > 1 || ch->silver > 1)
		{
			obj_to_obj (create_money (ch->gold / 2, ch->silver / 2),
						corpse);
			ch->gold -= ch->gold / 2;
			ch->silver -= ch->silver / 2;
		}
        
        corpse->cost = 0;
    }

    corpse->level = ch->level;
	corpse->corpse_race = ch->race;
    sprintf (buf, corpse->short_descr, name);
    free_string (corpse->short_descr);
    corpse->short_descr = str_dup (buf);

    sprintf (buf, corpse->description, name);
    free_string (corpse->description);
    corpse->description = str_dup (buf);

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
        bool floating = FALSE;

        obj_next = obj->next_content;
        //if (obj->wear_loc == WEAR_FLOAT)
        //    floating = TRUE;
        obj_from_char (obj);
        if (obj->item_type == ITEM_POTION)
            obj->timer = number_range (500, 1000);
        if (obj->item_type == ITEM_SCROLL)
            obj->timer = number_range (1000, 2500);
        if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH) && !floating)
        {
            obj->timer = number_range (5, 10);
            REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
        }
        REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

        if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
            extract_obj (obj);
        else if (floating)
        {
            if (IS_OBJ_STAT (obj, ITEM_ROT_DEATH))
            {                    /* get rid of it! */
                if (obj->contains != NULL)
                {
                    OBJ_DATA *in, *in_next;

                    act ("$p evaporates,scattering its contents.",
                         ch, obj, NULL, TO_ROOM);
                    for (in = obj->contains; in != NULL; in = in_next)
                    {
                        in_next = in->next_content;
                        obj_from_obj (in);
                        obj_to_room (in, ch->in_room);
                    }
                }
                else
                    act ("$p evaporates.", ch, obj, NULL, TO_ROOM);
                extract_obj (obj);
            }
            else
            {
                act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
                obj_to_room (obj, ch->in_room);
            }
        }
        else
            obj_to_obj (obj, corpse);
    }	
	
	if (killer)
	{
		if (!IS_NPC(ch))
			corpse->killer = str_dup(killer->name);
		else
			corpse->killer = str_dup(killer->short_descr);
	}
	
    if ( !IS_NPC(ch) && total_levels(ch) < 20 )
		obj_to_room (corpse, location);
	else
		obj_to_room (corpse, ch->in_room);
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry (CHAR_DATA * ch)
{
    ROOM_INDEX_DATA *was_in_room;   
	char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

	//Doesn't have a death cry if it's an inanimate object.
	if (IS_NPC(ch) && IS_SET(ch->act2, ACT2_INANIMATE))
		return;

	/* Make the head for a bountied PC */

	if(!IS_NPC(ch) && is_bountied(ch))
	{
		char buf[MAX_STRING_LENGTH];
		char *name;
		OBJ_DATA *obj;

		name = str_dup(ch->name);
		obj = create_object(get_obj_index(OBJ_VNUM_SEVERED_HEAD),0);
		obj->timer = number_range (4, 7);
		obj->extra_flags |= ITEM_NODROP|ITEM_NOUNCURSE;
		msg = "$n's severed head plops on the ground.";
		act (msg, ch, NULL, NULL, TO_ROOM);
		sprintf(buf, "%s %s", "head", name);
		free_string( obj->name );
		obj->name = str_dup( buf);
		sprintf( buf, obj->short_descr, name );
		free_string( obj->short_descr );
		obj->short_descr = str_dup( buf );
		sprintf( buf, obj->description, name );
		free_string( obj->description );
		obj->description = str_dup( buf );
		obj_to_char(obj,ch);
		free_string(name);
		if (IS_NPC (ch))
			msg = "You hear something's death cry.";
		else
			msg = "You hear someone's death cry.";
			
		was_in_room = ch->in_room;
		for (door = 0; door <= 5; door++)
		{
			EXIT_DATA *pexit;

			if ((pexit = was_in_room->exit[door]) != NULL
				&& pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
			{
				ch->in_room = pexit->u1.to_room;
				act (msg, ch, NULL, NULL, TO_ROOM);
			}
		}
		ch->in_room = was_in_room;
		return;

	}

    switch (number_range(0,12))
    {
		default:
        case 0:
            msg = "$n hits the ground ... DEAD.";
            break;
        case 1:
		case 2:
            if (ch->material == 0)
            {
                msg = "$n splatters blood on your armor.";
                break;
            }
        case 3:
            if (IS_SET (ch->parts, PART_GUTS))
            {
                msg = "$n spills $s guts all over the floor.";
                vnum = OBJ_VNUM_GUTS;
            }
            break;
        case 4:
            if (IS_SET (ch->parts, PART_HEAD) && !is_bountied(ch))
            {
                msg = "$n's severed head plops on the ground.";
                vnum = OBJ_VNUM_SEVERED_HEAD;
            }
            break;
        case 5:
		case 6:
		case 7:
		case 8:
            if (IS_SET (ch->parts, PART_HEART))
            {
                msg = "$n's heart is torn from $s chest.";
                vnum = OBJ_VNUM_TORN_HEART;
            }
            break;
        case 9:
            if (IS_SET (ch->parts, PART_ARMS))
            {
                msg = "$n's arm is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_ARM;
            }
            break;
        case 10:
            if (IS_SET (ch->parts, PART_LEGS))
            {
                msg = "$n's leg is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_LEG;
            }
            break;
        case 11:
		case 12:
            if (IS_SET (ch->parts, PART_BRAINS))
            {
                msg =
                    "$n's head is shattered, and $s brains splash all over you.";
                vnum = OBJ_VNUM_BRAINS;
            }
    }

    act (msg, ch, NULL, NULL, TO_ROOM);

	
    if (vnum != 0)
    {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;

        name = IS_NPC (ch) ? ch->short_descr : ch->name;
        obj = create_object (get_obj_index (vnum), 0);
        obj->timer = number_range (4, 7);
		
		//Level set for summon monster.
		obj->level = total_levels(ch);
			
		
        sprintf (buf, obj->short_descr, name);
        free_string (obj->short_descr);
        obj->short_descr = str_dup (buf);

        sprintf (buf, obj->description, name);
        free_string (obj->description);
        obj->description = str_dup (buf);

        if (obj->item_type == ITEM_FOOD)
        {
            if (IS_SET (ch->form, FORM_POISON))
                obj->value[3] = 1;
            else if (!IS_SET (ch->form, FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }

        obj_to_room (obj, ch->in_room);
    }

    if (IS_NPC (ch))
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";

	
	
    was_in_room = ch->in_room;
    for (door = 0; door <= 5; door++)
    {
        EXIT_DATA *pexit;

        if ((pexit = was_in_room->exit[door]) != NULL
            && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
        {
            ch->in_room = pexit->u1.to_room;
            act (msg, ch, NULL, NULL, TO_ROOM);
        }
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill (CHAR_DATA * victim)
{
    int i;
	CHAR_DATA *vch;
	CHAR_DATA * vch_next;
	CHAR_DATA *killer;
		
	killer = victim->fighting;
	
    stop_fighting (victim, TRUE);
    death_cry (victim);	
	make_corpse (victim, killer);
	victim->bleeding = 0;
	victim->pk_timer = 0;
	
    if (IS_NPC (victim))
    {
        victim->pIndexData->killed++;
        kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
        extract_char (victim, TRUE);
        return;
    }
	if (!IS_NPC (victim) && victim->leader && victim->grank == G_FRONT)
	{
		for ( vch = victim->in_room->people; vch != NULL; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( is_same_group( victim, vch ) && vch->grank != G_FRONT)
			{
				vch->grank = G_FRONT;
				SEND("Your group leader has {RDIED{x!\r\n",vch);
				SEND("You have been moved to the front of the group.\r\n",vch);
				break;
			}
		}	
	}
    extract_char (victim, FALSE);
    while (victim->affected)
        affect_remove (victim, victim->affected);
    victim->affected_by = race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
        victim->armor[i] = 100;
    victim->position = POS_RESTING;
    victim->hit = UMAX (1, victim->hit);
    victim->mana = UMAX (1, victim->mana);
    victim->move = UMAX (1, victim->move);
    return;
}



void group_gain (CHAR_DATA * ch, CHAR_DATA * victim)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if (victim == ch)
        return;

    members = 0;
    group_levels = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (is_same_group (gch, ch))
		{
            members++;
            group_levels += IS_NPC (gch) ? gch->level / 2 : gch->level;
        }
    }

    if (members == 0)
    {
        bug ("Fight.c -> Group_gain: members = 0.", members);
        members = 1;
        group_levels = ch->level;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;

        if (!is_same_group (gch, ch) || IS_NPC (gch))
            continue;
		
	
/*    Taken out, add it back if you want it
    if ( gch->level - lch->level >= 5 )
    {
        SEND( "You are too high for this group.\r\n", gch );
        continue;
    }

    if ( gch->level - lch->level <= -5 )
    {
        SEND( "You are too low for this group.\r\n", gch );
        continue;
    }
*/
        xp = xp_compute (gch, victim, group_levels);
		
		
        if (!IS_MCLASSED(gch) && gch->level < LEVEL_HERO)
		{		
			if (gch->no_exp[0] == TRUE)
				continue;
			xp = xp;
			sprintf (buf, "You receive {g%d{x experience points.\r\n", xp);
			SEND (buf, gch);
			gain_exp (gch, xp, FALSE);
		}
		else 
		{
			if ( total_levels(gch) >= LEVEL_HERO )
				continue;
				
			if ( gch->level >= MCLASS_ONE ) //25
			{
				if ( gch->level2 < MCLASS_TWO )
				{
					if (gch->no_exp[1] == TRUE)
						continue;
					sprintf (buf, "You receive {g%d{x experience points in your secondary class.\r\n", xp);
					SEND (buf, gch);
					gain_exp (gch, xp, TRUE);
				}
				else
				{
					continue;
				}
			}			
			else
			{
				if ( gch->level2 < MCLASS_TWO ) //15
				{
					if (gch->no_exp[0] == TRUE && gch->no_exp[1] == FALSE)
					{
						sprintf (buf, "You receive {g%d{x experience points in your secondary class.\r\n", xp);
						SEND (buf, gch);			
						gain_exp (gch, xp, TRUE);
					}
					else if (gch->no_exp[0] == FALSE && gch->no_exp[1] == TRUE)
					{
						sprintf (buf, "You receive {g%d{x experience points in your primary class.\r\n", xp);
						SEND (buf, gch);						
						gain_exp (gch, xp, FALSE);									
					}	
					else if (gch->no_exp[0] == FALSE && gch->no_exp[1] == FALSE)
					{
						sprintf (buf, "You receive {g%d{x / {g%d{x experience points.\r\n", (xp / 2), (xp / 2));
						SEND (buf, gch);			
						gain_exp (gch, xp / 2, FALSE);
						gain_exp (gch, xp / 2, TRUE);										
					}	
					else
					{
						continue;
					}
					
				}
				else
				{
					if (gch->no_exp[0] == FALSE)
					{
						sprintf (buf, "You receive {g%d{x experience points.\r\n", xp);
						SEND (buf, gch);						
						gain_exp (gch, xp, FALSE);													
					}						
				}
			}
		}


        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE)
                continue;

            if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (ch))
                || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (ch))
                || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
            {
                act ("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
                act ("$n is zapped by $p.", ch, obj, NULL, TO_ROOM);
                obj_from_char (obj);
                obj_to_room (obj, ch->in_room);
            }
        }
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int tot_levels)
{
    int xp = 0;
	int group_mult = 1;
	CHAR_DATA * vch;
	CHAR_DATA * vch_next;	
	int i = 0;
    //int time_per_level;
	
	//Discourage higher levels from saccing areas before lower levels can get to it. 11/29/2011 Upro.
	if (total_levels(gch) - 9 > victim->level)
	{	
		xp = 0;
		return xp;
	}
	
	if (victim->level < 8)
		xp = number_range(victim->level * 60, victim->level * 80);	
	else 
	{
		for (i = 0; i < victim->level; i++)
		{
			xp += 180;
		}
	}
	
	
	if IS_NPC(victim)
	{
		if (IS_SET(victim->act2, ACT2_ELITE))
		{
			xp *= 2;
		}
		if (IS_SET(victim->act2, ACT2_BOSS))
		{
			xp *= 3;
		}
	}
	
	// Xp bonus for having high primary/secondary stats.
	// Can only get secondary if they get primary bonus.
	if (IS_MCLASSED(gch))
	{
		if (GET_STAT(gch, PRIME_STAT(gch, (gch->level < MCLASS_ONE ? FALSE : TRUE))) >= 16)
		{	
			xp += (xp * 1/10);	
				
			if (GET_STAT(gch, SECOND_STAT(gch, (gch->level < MCLASS_ONE ? FALSE : TRUE))) >= 16)	
				xp += (xp * 1/20);
		}
	}	
	else
	{
		if (GET_STAT(gch, PRIME_STAT(gch, FALSE)) >= 16)
		{	
			xp += (xp * 1/10);	
				
			if (GET_STAT(gch, SECOND_STAT(gch, FALSE)) >= 16)	
				xp += (xp * 1/20);
		}
	}
	
	//Small bonus for humans
	if (IS_HUMAN(gch))
		xp = xp * 10 / 9;    
    
	// Small exp boost for groups... Upro (8/2007)   
    for ( vch = gch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( is_same_group( gch, vch ) && gch != vch )
        	group_mult += 1;				
    }
    if (group_mult > 5)
		group_mult = 5;
    xp += (xp * group_mult) / 10;
	
	
    /* randomize the rewards */
    xp = number_range (xp * 3 / 4, xp * 5 / 4);	
 
	if (double_exp)
	{
		xp *= 2;
	}    
	
    return xp;
	
}

void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt, int dam_type, bool immune, bool crit)
{
    char buf1[256], buf2[256], buf3[256], buf4[256];
    const char *vs;  
    const char *vp;
    const char *res;
    const char *attack;
	char * dam_color;
	char punct;
    bool same_room = FALSE;
	OBJ_DATA *obj;
        
    if (ch == NULL || victim == NULL)
        return;
                
    if ( dam < 0 )
		dam = 0;
    if(dam > 3000)
		dam = 3000;
    if(dam_type < 0 || dam_type > MAX_DAM_TYPE)
		dam_type = DAM_NONE;
	

    //int color = COLOR_DULL;
    //if (!IS_NPC(ch) && ch->pcdata->colorpref == COLOR_BRIGHT)
    //	color = COLOR_BRIGHT;

	obj = get_eq_char (ch, WEAR_WIELD);
	
	if(ch->race == RACE_REVENANT && get_eq_char (ch, WEAR_WIELD) == NULL && get_eq_char (ch, WEAR_SECONDARY) == NULL && !obj)	
	{
		dam_type = DAM_NEGATIVE;
		dt = DAM_NEGATIVE + TYPE_HIT;
	}
	if((IS_LIZARDMAN(ch) || IS_BIRDFOLK(ch)) && get_eq_char (ch, WEAR_WIELD) == NULL && get_eq_char (ch, WEAR_SECONDARY) == NULL && !obj)	
	{
		dam_type = DAM_SLASH;
		dt = DAM_SLASH + TYPE_HIT;
	}

    if (dam_type == DAM_PIERCE)
    {
        if      ( dam ==   0  ) { vs = "{Dmiss{x";	vp = "{Dmisses{x";    }
    	else if ( dam <=   7  ) { vs = "{Dprod{x";     	vp = "{Dprods{x";     }
    	else if ( dam <=   15 ) { vs = "{Bpoke{x"; 	vp = "{Bpokes{x";     }
    	else if ( dam <=   23 ) { vs = "{Bsting{x";    	vp = "{Bstings{x";    }
    	else if ( dam <=   30 ) { vs = "{Gdig{x";      	vp = "{Gdigs{x";      }
    	else if ( dam <=   38 ) { vs = "{Gjab{x";      	vp = "{Gjabs{x";      }
    	else if ( dam <=   46 ) { vs = "{Gprick{x";    	vp = "{Gpricks{x";    }
    	else if ( dam <=   55 ) { vs = "{Ypuncture{x"; 	vp = "{Ypunctures{x"; }
    	else if ( dam <=   64 ) { vs = "{Yslit{x"; 	vp = "{Yslits{x";     }
    	else if ( dam <=   72 ) { vs = "{Yincise{x";   	vp = "{Yincises{x";   }
    	else if ( dam <=   81 ) { vs = "{Rstab{x";     	vp = "{Rstabs{x";     }
    	else if ( dam <=   91 ) { vs = "{Rimpale{x";   	vp = "{Rimpales{x";   }
    	else                    { vs = "{RIMPALE{x";   	vp = "{RIMPALES{x";   }

	if      ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", leaving a mark"; }
    	else if ( dam <= 10  ) { res = ", scratching deeply"; }
    	else if ( dam <= 20  ) { res = ", breaking skin"; }
    	else if ( dam <= 40  ) { res = ", causing a minor flesh wound"; }
    	else if ( dam <= 75  ) { res = ", causing blood to flow"; }
    	else if ( dam <= 100  ) { res = ", causing severe bleeding"; }
    	else if ( dam <= 150  ) { res = ", leaving an open gash"; }
    	else { res = ", exposing innards"; } 
    }

    else if (dam_type == DAM_SLASH)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";    vp = "{Dmisses{x";    }
    	else if ( dam <=   7  ) { vs = "{Dgrazes{x";  vp = "{Dgrazes{x";    }
    	else if ( dam <=   15 ) { vs = "{Bscrape{x";  vp = "{Bscrapes{x";   }
    	else if ( dam <=   23 ) { vs = "{Bscratch{x"; vp = "{Bscratches{x"; }
    	else if ( dam <=   30 ) { vs = "{Bclip{x";    vp = "{Bclips{x";     }
    	else if ( dam <=   38 ) { vs = "{Gcut{x";     vp = "{Gcuts{x";      }
    	else if ( dam <=   46 ) { vs = "{Gslash{x";   vp = "{Gslashes{x";   }
    	else if ( dam <=   55 ) { vs = "{Gslice{x";   vp = "{Gslices{x";    }
    	else if ( dam <=   64 ) { vs = "{Ygash{x";    vp = "{Ygashes{x";    }
    	else if ( dam <=   72 ) { vs = "{Ycarve{x";   vp = "{Ycarves{x";    }
    	else if ( dam <=   81 ) { vs = "{Ypare{x";    vp = "{Ypares{x";     }
    	else if ( dam <=   91 ) { vs = "{Rcleave{x";  vp = "{Rcleaves{x";   }
    	else { vs = "{RBUTCHER{x";  vp = "{RBUTCHERS{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", leaving a mark"; }
    	else if ( dam <= 10  ) { res = ", scratching deeply"; }
    	else if ( dam <= 20  ) { res = ", cutting skin"; }
    	else if ( dam <= 40  ) { res = ", causing a minor flesh wound"; }
    	else if ( dam <= 75  ) { res = ", leaving a decent gash"; }
    	else if ( dam <= 100  ) { res = ", slicing flesh open"; }
    	else if ( dam <= 150  ) { res = ", leaving an open cleave"; }
    	else { res = ", exposing innards"; } 
    }

    else if (dam_type == DAM_BASH)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";   vp = "{Dmisses{x";   }
    	else if ( dam <=   7  ) { vs = "{Dbump{x";   vp = "{Dbumps{x";    }
    	else if ( dam <=   15 ) { vs = "{Bthump{x";  vp = "{Bthumps{x";   }
    	else if ( dam <=   23 ) { vs = "{Bsmack{x";  vp = "{Bsmacks{x";   }
    	else if ( dam <=   30 ) { vs = "{Bjolt{x";   vp = "{Bjolts{x";    }
    	else if ( dam <=   38 ) { vs = "{Gpound{x";  vp = "{Gpounds{x";   }
    	else if ( dam <=   46 ) { vs = "{Gblasts{x"; vp = "{Gblasts{x";   }
    	else if ( dam <=   55 ) { vs = "{Gbatter{x"; vp = "{Gbatters{x";  }
    	else if ( dam <=   64 ) { vs = "{Yhammer{x"; vp = "{Yhammers{x";  }
    	else if ( dam <=   72 ) { vs = "{Ysmash{x";  vp = "{Ysmashes{x";  }
    	else if ( dam <=   81 ) { vs = "{Ysquash{x"; vp = "{Ysquashes{x"; }
    	else if ( dam <=   91 ) { vs = "{Rcrush{x";  vp = "{Ycrushes{x";  }
    	else { vs = "{RPULVERIZE{x";  vp = "{RPULVERIZES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing a wince"; }
    	else if ( dam <= 10  ) { res = ", leaving a bruise"; }
    	else if ( dam <= 20  ) { res = ", leaving a deep bruise"; }
    	else if ( dam <= 40  ) { res = ", splitting skin"; }
    	else if ( dam <= 75  ) { res = ", making a loud crack"; }
    	else if ( dam <= 100  ) { res = ", breaking skin open"; }
    	else if ( dam <= 150  ) { res = ", leaving a dent"; }
    	else { res = ", crushing bone"; } 
    }

    else if (dam_type == DAM_FIRE)
    {
         if ( dam ==   0   ) { vs = "{Dmiss{x";   vp = "{Dmisses{x";   }
    	else if ( dam <=   15  ) { vs = "{Bwarm{x";   vp = "{Bwarms{x";    }
    	else if ( dam <=   25  ) { vs = "{Bsinge{x";  vp = "{Bsinges{x";   }
    	else if ( dam <=   40  ) { vs = "{Gscald{x";  vp = "{Gscalds{x";   }
    	else if ( dam <=   55  ) { vs = "{Gburn{x";   vp = "{Gburns{x";    }
    	else if ( dam <=   70  ) { vs = "{rsear{x";   vp = "{rsears{x";    }
   	else if ( dam <=   90  ) { vs = "{rchar{x";   vp = "{rchars{x";    }
   	else if ( dam <=   120 ) { vs = "{Rscorch{x"; vp = "{Rscorches{x"; }
   	else if ( dam <=   150 ) { vs = "{RCAUTERIZE{x"; vp = "{RCAUTERIZES{x";}
  	else { vs = "{RINCINERATE{x";  vp = "{RINCINERATES{x"; }

	 if ( dam == 0   ) { res = ""; }
	else if ( dam <= 5   ) { res = ", reddening skin"; }
    	else if ( dam <= 10  ) { res = ", causing irritation"; }
    	else if ( dam <= 20  ) { res = ", making blisters"; }
    	else if ( dam <= 40  ) { res = ", charring flesh"; }
    	else if ( dam <= 75  ) { res = ", causing very deep burns"; }
    	else if ( dam <= 100  ) { res = ", scorching flesh"; }
    	else if ( dam <= 150  ) { res = ", searing through skin"; }
    	else if ( dam <= 180  ) { res = ", consuming with flames"; }
    	else { res = ", leaving ash behind"; } 
    }

    else if (dam_type == DAM_COLD)
    {
         if ( dam ==   0  )  { vs = "{Dmiss{x";   vp = "{Dmisses{x";  }
    	else if ( dam <=   15  ) { vs = "{bcool{x";   vp = "{bcools{x";   }
    	else if ( dam <=   25 )  { vs = "{Gchill{x";  vp = "{Gchills{x";  }
    	else if ( dam <=   40 )  { vs = "{Gnip{x";    vp = "{Gnips{x";    }
    	else if ( dam <=   65 )  { vs = "{bfrost{x";  vp = "{bfrosts{x";  }
    	else if ( dam <=   100 ) { vs = "{bfreeze{x"; vp = "{bfreezes{x"; }
    	else { vs = "{BDEEP FREEZE{x";  vp = "{BDEEP FREEZES{x"; }
	
	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", giving a chill"; }
    	else if ( dam <= 10  ) { res = ", causing a shiver"; }
    	else if ( dam <= 20  ) { res = ", numbing some skin"; }
    	else if ( dam <= 40  ) { res = ", causing minor nerve damage"; }
    	else if ( dam <= 75  ) { res = ", freezing flesh"; }
    	else if ( dam <= 100  ) { res = ", deep freezing skin"; }
    	else if ( dam <= 150  ) { res = ", causing frostbite"; }
    	else { res = ", causing traumatic frostbite"; } 
    }

    else if (dam_type == DAM_LIGHTNING)
    {
         if ( dam ==   0  )  { vs = "{Dmiss{x";   vp = "{Dmisses{x"; }
    	else if ( dam <=   15  ) { vs = "{Bspark{x"; vp = "{Bsparks{x";  }
    	else if ( dam <=   25 )  { vs = "{Gzap{x"; vp = "{Gzaps{x";      }
    	else if ( dam <=   40 )  { vs = "{Gshock{x"; vp = "{Gshocks{x";  }
    	else if ( dam <=   65 )  { vs = "{Ystun{x"; vp = "{Ystuns{x";    }
    	else if ( dam <=   100 ) { vs = "{YZOT{x"; vp = "{YZOTS{x";      }
    	else { vs = "{BELECTROCUTE{x";  vp = "{BELECTROCUTES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", giving a small zot"; }
    	else if ( dam <= 10  ) { res = ", making a spark"; }
    	else if ( dam <= 20  ) { res = ", making a loud spark"; }
    	else if ( dam <= 40  ) { res = ", nearly making thunder"; }
    	else if ( dam <= 75  ) { res = ", followed by a loud clap"; }
    	else if ( dam <= 100  ) { res = ", arcing senselessly"; }
    	else if ( dam <= 150  ) { res = ", making a gigantic boom"; }
    	else { res = ", making even Thor jealous"; } 
    }

    else if (dam_type == DAM_ACID)
    {
         if ( dam ==   0   ) { vs = "{Dmiss{x";     vp = "{Dmisses{x";    }
    	else if ( dam <=   15  ) { vs = "{Birritate{x"; vp = "{Birritates{x"; }
    	else if ( dam <=   35  ) { vs = "{Gburns{x";    vp = "{Gburns{x";     }
    	else if ( dam <=   60  ) { vs = "{Geat away at{x";vp = "{Geats away at{x"; }
    	else if ( dam <=   90  ) { vs = "{yetch{x";     vp = "{yetches{x";	  }
    	else if ( dam <=   120 ) { vs = "{ycorrode{x";  vp = "{ycorrodes{x";  }
    	else { vs = "{YDISINTEGRATE{x";  vp = "{YDISINTEGRATES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", leaving a mark"; }
    	else if ( dam <= 10  ) { res = ", irritating skin"; }
    	else if ( dam <= 20  ) { res = ", making a small burn"; }
    	else if ( dam <= 40  ) { res = ", causing blisters"; }
    	else if ( dam <= 75  ) { res = ", leaving a deep burn"; }
    	else if ( dam <= 100  ) { res = ", eating away flesh"; }
    	else if ( dam <= 150  ) { res = ", corroding the skin"; }
    	else { res = ", dissolving flesh away"; } 
    }

    else if (dam_type == DAM_POISON)
    {
         if ( dam ==    0 ) { vs = "{Dmiss{x";   vp = "{Dmisses{x";  }
    	else if ( dam <=   15 ) { vs = "{Btaint{x";  vp = "{Btaints{x";  }
    	else if ( dam <=   35 ) { vs = "{Bblight{x"; vp = "{Bblights{x"; }
    	else if ( dam <=   60 ) { vs = "{Yinfect{x"; vp = "{Yinfects{x"; }
    	else if ( dam <=   90 ) { vs = "{Ypoison{x"; vp = "{Ypoisons{x"; }
    	else if ( dam <=  120 ) { vs = "{Gsicken{x"; vp = "{Gsickens{x"; }
    	else { vs = "{GCONTAMINATE{x";  vp = "{GCONTAMINATES{x"; }
	
	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing mild queasiness"; }
    	else if ( dam <= 10  ) { res = ", causing some sickness"; }
    	else if ( dam <= 20  ) { res = ", causing mild nausea"; }
    	else if ( dam <= 40  ) { res = ", nearly inducing vomitting"; }
    	else if ( dam <= 75  ) { res = ", causing strong sickness"; }
    	else if ( dam <= 100  ) { res = ", causing extreme nausea"; }
    	else if ( dam <= 150  ) { res = ", forcing extreme sickness"; }
    	else { res = ", causing intense illness"; } 
    }

    else if (dam_type == DAM_NEGATIVE)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";    vp = "{Dmisses{x";    }
    	else if ( dam <=   15 ) { vs = "{Btaint{x";   vp = "{Btaints{x";    }
    	else if ( dam <=   25 ) { vs = "{Bstain{x";   vp = "{Bstains{x";    }
    	else if ( dam <=   40 ) { vs = "{Ytarnish{x"; vp = "{Ytarnishes{x"; }
    	else if ( dam <=   60 ) { vs = "{Rtorment{x"; vp = "{Rtorments{x";  }
    	else if ( dam <=   90 ) { vs = "{Rdamn{x";    vp = "{Rdamns{x";	}
    	else if ( dam <=  120 ) { vs = "{Dvex{x";     vp = "{Dvexes{x";	}
    	else { vs = "{RDEFILE{x";  vp = "{DDEFILES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing mild discomfort"; }
    	else if ( dam <= 10  ) { res = ", causing some squeamishness"; }
    	else if ( dam <= 20  ) { res = ", making a reddish glow"; }
    	else if ( dam <= 40  ) { res = ", causing severe discomfort"; }
    	else if ( dam <= 75  ) { res = ", causing some cursedness"; }
    	else if ( dam <= 100  ) { res = ", making the air glow red about"; }
    	else if ( dam <= 150  ) { res = ", inviting hell's spirits about"; }
    	else { res = ", causing extreme damnation"; } 
    }

    else if (dam_type == DAM_HOLY)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";    vp = "{Dmisses{x";   }
    	else if ( dam <=   15 ) { vs = "{Bcleanse{x"; vp = "{Bcleanses{x"; }
    	else if ( dam <=   35 ) { vs = "{Gpurify{x";  vp = "{Gpurifies{x"; }
    	else if ( dam <=   60 ) { vs = "{Gbless{x";   vp = "{Gblesses{x";  }
    	else if ( dam <=   90 ) { vs = "{Yabsolve{x"; vp = "{Yabsolves{x"; }
    	else if ( dam <=  120 ) { vs = "{Wpurge{x";   vp = "{Rpurges{x";   }
	else { vs = "{WSANCTIFY{x";  vp = "{WSANCTIFIES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing a wince"; }
    	else if ( dam <= 10  ) { res = ", causing a cringe"; }
    	else if ( dam <= 20  ) { res = ", cleaning evil thoughts"; }
    	else if ( dam <= 40  ) { res = ", cleansing away malice"; }
    	else if ( dam <= 75  ) { res = ", purging away evil"; }
    	else if ( dam <= 100  ) { res = ", baning evil spirits"; }
    	else if ( dam <= 150  ) { res = ", rendering visions of holy spirits"; }
    	else { res = ", inviting the power of the heavens"; } 
    }

    else if (dam_type == DAM_ENERGY)
    {
         if ( dam ==    0 ) { vs = "{Dmiss{x";   vp = "{Dmisses{x";     }
    	else if ( dam <=   10 ) { vs = "{Dwarm{x";   vp = "{Dwarms{x";      }
    	else if ( dam <=   20 ) { vs = "{Bshock{x";  vp = "{Bshocks{x";     }
    	else if ( dam <=   35 ) { vs = "{Bshake{x";  vp = "{Bshakes{x";     }
    	else if ( dam <=   65 ) { vs = "{Gburn{x";   vp = "{Gburns{x";      }
    	else if ( dam <=   90 ) { vs = "{Ysear{x";   vp = "{Ysears{x";      }
    	else if ( dam <=  115 ) { vs = "{Ychar{x";   vp = "{Ychars{x";      }
    	else if ( dam <=  130 ) { vs = "{Rscorch{x"; vp = "{Rscorches{x";   }
    	else { vs = "{RRADIATE THROUGH{x";  vp = "{RRADIATES THROUGH{x";  }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", reddening skin"; }
    	else if ( dam <= 10  ) { res = ", causing irritation"; }
    	else if ( dam <= 20  ) { res = ", making blisters"; }
    	else if ( dam <= 40  ) { res = ", charring flesh"; }
    	else if ( dam <= 75  ) { res = ", causing very deep burns"; }
    	else if ( dam <= 100  ) { res = ", scorching flesh"; }
    	else if ( dam <= 150  ) { res = ", searing through skin"; }
    	else { res = ", causing an eerie glow all around"; } 
    }

    else if (dam_type == DAM_MENTAL)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";     vp = "{Dmisses{x";    }
    	else if ( dam <=   15 ) { vs = "{Bbaffle{x";   vp = "{Bbaffles{x";   }
    	else if ( dam <=   25 ) { vs = "{Gfluster{x";  vp = "{Gflusters{x";  }
    	else if ( dam <=   50 ) { vs = "{Gdaze{x";     vp = "{Gdazes{x";     }
    	else if ( dam <=   75 ) { vs = "{Ymuddle{x";   vp = "{Ymuddles{x";	 }
    	else if ( dam <=  100 ) { vs = "{Ybewilder{x"; vp = "{Ybewilders{x"; }
    	else if ( dam <=  125 ) { vs = "{Rperplex{x";  vp = "{Rperplexes{x"; }
    	else { vs = "{RMYSTIFY{x";  vp = "{RMYSTIFIES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing a second thought"; }
    	else if ( dam <= 10  ) { res = ", causing mild bewilderment"; }
    	else if ( dam <= 20  ) { res = ", dazing thoughts"; }
    	else if ( dam <= 40  ) { res = ", scrambling brain cells"; }
    	else if ( dam <= 75  ) { res = ", damaging memories"; }
    	else if ( dam <= 100  ) { res = ", causing severe confusion"; }
    	else if ( dam <= 150  ) { res = ", causing great mental anguish"; }
    	else { res = ", damaging neurons"; } 
    }

    else if (dam_type == DAM_DISEASE)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";    vp = "{Dmisses{x";   }
    	else if ( dam <=   15 ) { vs = "{Boffend{x";  vp = "{Boffends{x";  }
    	else if ( dam <=   35 ) { vs = "{Ydisgust{x"; vp = "{Gdisgusts{x"; }
    	else if ( dam <=   55 ) { vs = "{Ysicken{x";  vp = "{Gsickens{x";  }
    	else if ( dam <=   80 ) { vs = "{Gafflict{x"; vp = "{Yafflicts{x"; }
    	else if ( dam <=  100 ) { vs = "{Gplague{x";  vp = "{Yplagues{x";  }
    	else { vs = "{GNAUSEATE{x";  vp = "{GNAUSEATES{x"; }
	
	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing mild illness"; }
    	else if ( dam <= 10  ) { res = ", causing moderate illness"; }
    	else if ( dam <= 20  ) { res = ", opening small sores"; }
    	else if ( dam <= 40  ) { res = ", causing sores to pus"; }
    	else if ( dam <= 75  ) { res = ", causing severe illness"; }
    	else if ( dam <= 100  ) { res = ", opening large sores"; }
    	else if ( dam <= 150  ) { res = ", causing skin to blister and peel"; }
    	else { res = ", rotting away at flesh"; } 
    }

    else if (dam_type == DAM_DROWNING)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";        vp = "{Dmisses{x";       }
    	else if ( dam <=   15 ) { vs = "{Bmoisten{x";     vp = "{Bmoistens{x";     }
    	else if ( dam <=   30 ) { vs = "{Bwetten{x";      vp = "{Bwettens{x";      }
    	else if ( dam <=   50 ) { vs = "{Gshower{x";      vp = "{Gshowers{x";      }
    	else if ( dam <=   80 ) { vs = "{Ydownpour on{x"; vp = "{Ydownpours on{x"; }
    	else if ( dam <=  105 ) { vs = "{Yflood{x";       vp = "{Yfloods{x";       }
    	else { vs = "{RNEARLY DROWN{x";  vp = "{RNEARLY DROWNS{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", chapping skin"; }
    	else if ( dam <= 10  ) { res = ", causing prunage"; }
    	else if ( dam <= 20  ) { res = ", weighing clothing down"; }
    	else if ( dam <= 40  ) { res = ", causing slight coughing"; }
    	else if ( dam <= 75  ) { res = ", allowing water to enter lungs"; }
    	else if ( dam <= 100  ) { res = ", making air scarce"; }
    	else if ( dam <= 150  ) { res = ", submersing the body"; }
    	else { res = ", filling the lungs"; } 
    }

    else if (dam_type == DAM_LIGHT)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";   	vp = "{Dmisses{x";        }
    	else if ( dam <=   15 ) { vs = "{Bbrighten{x"; 	vp = "{Bbrightens{x";     }
    	else if ( dam <=   25 ) { vs = "{Bglow on{x"; 	vp = "{Bglows on{x";      }
    	else if ( dam <=   55 ) { vs = "{Gglistens upon{x"; vp = "{Gglistens upon{X"; }
    	else if ( dam <=   80 ) { vs = "{Gshimmer on{x"; 	vp = "{Gshimmers on{x";   }
    	else if ( dam <=   95 ) { vs = "{Yshine on{x"; 	vp = "{Yshines on{x";     }
    	else if ( dam <=  125 ) { vs = "{Rilluminate{x"; 	vp = "{Rilluminates{x";   }
    	else { vs = "{RIRRADIATE{x";  vp = "{RIRRADIATES{x"; }

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing a squint"; }
    	else if ( dam <= 10  ) { res = ", making the room glow a bit"; }
    	else if ( dam <= 20  ) { res = ", causing light burns"; }
    	else if ( dam <= 40  ) { res = ", causing a heavy sweat"; }
    	else if ( dam <= 75  ) { res = ", heating the skin intensely"; }
    	else if ( dam <= 100  ) { res = ", causing hefty light burns"; }
    	else if ( dam <= 150  ) { res = ", making eyes hurt badly"; }
    	else { res = ", causing severe retinal damage"; } 
    }
	
    else if (dam_type == DAM_HARM)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";   vp = "{Dmisses{x";  }
    	else if ( dam <=   5  ) { vs = "{Bhit{x"; 	 vp = "{Bhits{x";    }
    	else if ( dam <=   15 ) { vs = "{Ghurt{x"; 	 vp = "{Ghurts{x";   }
    	else if ( dam <=   35 ) { vs = "{Ginjure{x"; vp = "{Ginjures{x"; }
    	else if ( dam <=   65 ) { vs = "{Ywound{x";  vp = "{Ywounds{x";  }
    	else if ( dam <=   95 ) { vs = "{Yharm{x"; 	 vp = "{Yharms{x";   }
    	else { vs = "{RDESTROY{x";  vp = "{RDESTROYS{x"; }
	
	if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing light wounds"; }
    	else if ( dam <= 10  ) { res = ", causing acute pain"; }
    	else if ( dam <= 20  ) { res = ", causing serious wounds"; }
    	else if ( dam <= 40  ) { res = ", making muscles ache badly"; }
    	else if ( dam <= 75  ) { res = ", causing convulsions"; }
    	else if ( dam <= 100  ) { res = ", causing critical wounds"; }
    	else if ( dam <= 150  ) { res = ", causing small seizures"; }
    	else { res = ", doing nasty amounts of harm"; } 
    }

    else if (dam_type == DAM_CHARM)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";     vp = "{Dmisses{x";    }
    	else if ( dam <=   15 ) { vs = "{Ballure{x";   vp = "{Ballures{x";   }
    	else if ( dam <=   25 ) { vs = "{Genchant{x";  vp = "{Genchants{x";  }
    	else if ( dam <=   50 ) { vs = "{Gbeguile{x";   vp = "{Gbeguiles{x";     }
    	else if ( dam <=   75 ) { vs = "{Ymesmerize{x"; vp = "{Ymesmerizes{x";	 }
    	else if ( dam <=  100 ) { vs = "{Yhypnotize{x"; vp = "{Yhypnotizes{x"; }
    	else if ( dam <=  125 ) { vs = "{Rentrance{x";  vp = "{Rentrances{x"; }
    	else { vs = "{RENTHRALL{x";  vp = "{RENTHRALLS{x"; }
	
	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing a second thought"; }
    	else if ( dam <= 10  ) { res = ", causing mild confusion"; }
    	else if ( dam <= 20  ) { res = ", dazing mentality"; }
    	else if ( dam <= 40  ) { res = ", scrambling brain cells"; }
    	else if ( dam <= 75  ) { res = ", damaging thought patterns"; }
    	else if ( dam <= 100  ) { res = ", causing severe bewilderment"; }
    	else if ( dam <= 150  ) { res = ", causing great mental suffering"; }
    	else { res = ", causing hemorrhage"; } 
    }
    else if (dam_type == DAM_SOUND)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";   	      vp = "{Dmisses{x";  }
    	else if ( dam <=   15 ) { vs = "{Brattle{x"; 	      vp = "{Brattles{x"; }
    	else if ( dam <=   30 ) { vs = "{Bshake{x";               vp = "{Bshakes{x";  }
    	else if ( dam <=   50 ) { vs = "{Gresound through{x";     vp = "{Gresounds through{x"; }
    	else if ( dam <=   75 ) { vs = "{Yreverberate through{x"; vp = "{Yreverberates through{x"; }
    	else if ( dam <=  100 ) { vs = "{Yresonate through{x";    vp = "{Yresonates through{x"; }
  	else { vs = "{RNEARLY DEAFEN{x";  vp = "{RNEARLY DEAFENS{x"; }	

	 if ( dam == 0   ) { res = ""; }
    	else if ( dam <= 5   ) { res = ", causing a wince"; }
    	else if ( dam <= 10  ) { res = ", causing slight ear annoyance"; }
    	else if ( dam <= 20  ) { res = ", making ears hurt"; }
    	else if ( dam <= 40  ) { res = ", causing a small headache"; }
    	else if ( dam <= 75  ) { res = ", making the skull pound"; }
    	else if ( dam <= 100  ) { res = ", shaking everything around"; }
    	else if ( dam <= 150  ) { res = ", making ears bleed"; }
    	else { res = ", inducing a severe migraine headache"; } 
    }	
    else if (dam_type == DAM_AIR)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";                vp = "{Dmisses{x";  }
        else if ( dam <=   15 ) { vs = "{Bwaft by{x";          vp = "{Bwafts by{x"; }
        else if ( dam <=   30 ) { vs = "{Bbreeze by{x";               vp = "{Bbreezes by{x";  }
        else if ( dam <=   50 ) { vs = "{Gshake{x";     vp = "{Gshakes{x"; }
        else if ( dam <=   75 ) { vs = "{Yflurry{x"; vp = "{Yflurries{x"; }
        else if ( dam <=  100 ) { vs = "{Rgust{x";    vp = "{Rgusts{x"; }
        else { vs = "{RGALE{x";  vp = "{RGALES{x"; }

         if ( dam == 0   ) { res = ""; }
        else if ( dam <= 5   ) { res = ", causing you to blink"; }
        else if ( dam <= 10  ) { res = ", barely pushing you back"; }
        else if ( dam <= 20  ) { res = ", making skin red"; }
        else if ( dam <= 40  ) { res = ", causing a small headache"; }
        else if ( dam <= 75  ) { res = ", making the air swirl violently"; }
        else if ( dam <= 100  ) { res = ", shaking everything around"; }
        else if ( dam <= 150  ) { res = ", ripping hair from skin"; }
        else { res = ", peeling skin back"; }
    }
    else if (dam_type == DAM_EARTH)
    {
         if ( dam ==   0  ) { vs = "{Dmiss{x";                vp = "{Dmisses{x";  }
        else if ( dam <=   15 ) { vs = "{Brattle{x";          vp = "{Brattles{x"; }
        else if ( dam <=   30 ) { vs = "{Bpelt{x";               vp = "{Bpelts{x";  }
        else if ( dam <=   50 ) { vs = "{Gclobber{x";     vp = "{Gclobbers{x"; }
        else if ( dam <=   75 ) { vs = "{Ywallop{x"; vp = "{Ywallops{x"; }
        else if ( dam <=  100 ) { vs = "{Ypummel{x";    vp = "{Ypummels{x"; }
        else { vs = "{RASSAULT{x";  vp = "{RASSAULTS{x"; }

         if ( dam == 0   ) { res = ""; }
        else if ( dam <= 5   ) { res = ", causing a wince"; }
        else if ( dam <= 10  ) { res = ", causing a small bruise"; }
        else if ( dam <= 20  ) { res = ", causing contusions"; }
        else if ( dam <= 40  ) { res = ", causing dizziness"; }
        else if ( dam <= 75  ) { res = ", making the earth shake"; }
        else if ( dam <= 100  ) { res = ", shaking everything around"; }
        else if ( dam <= 150  ) { res = ", making the ground rumble"; }
        else { res = ", making the earth split"; }
    }
    else
    {
		if (dam == 0)
		{
			vs = "{Dmiss{x";
			vp = "{Dmisses{x";
			res = "";
		}
		else
		{
			// why not have a table for null damage/any damage?
					 if ( dam ==   0  ) { vs = "{Dmiss{x";       vp = "{Dmisses{x";  }
					else if ( dam <=   15 ) { vs = "{Whurt{x";   vp = "{Whurts{x"; }
					else if ( dam <=   30 ) { vs = "{Wpain{x";   vp = "{Wpains{x";  }
					else if ( dam <=   50 ) { vs = "{Wharm{x";   vp = "{Wharms{x"; }
					else if ( dam <=   75 ) { vs = "{Wimpair{x"; vp = "{Wimpairs{x"; }
					else if ( dam <=  100 ) { vs = "{Wmar{x";    vp = "{Wmars{x"; }
					else {                    vs = "{WDAMAGE{x";  vp = "{RDAMAGES{x"; }
			
					 if ( dam == 0   ) { res = ""; }
					else if ( dam <= 5   ) { res = ", creating slight distress"; }
					else if ( dam <= 10  ) { res = ", causing a small annoyance"; }
					else if ( dam <= 20  ) { res = ", producing a small ache"; }
					else if ( dam <= 40  ) { res = ", burning skin"; }
					else if ( dam <= 75  ) { res = ", singing hair and skin"; }
					else if ( dam <= 100  ) { res = ", melting small digits"; }
					else if ( dam <= 150  ) { res = ", exploding toes"; }
					else { res = ", skin bubbling and blistering"; }
		}
    }

    punct   = (dam <= 24) ? '.' : '!';
        
    if ( ch->in_room != NULL && victim->in_room != NULL && ch->in_room == victim->in_room )
		same_room = TRUE;	
	
	dam_color = "{x";
	
	if (crit && dam > 1)		
		dam_color = "{Y";
		
    if ( dt == TYPE_HIT )
    {
        if (ch == victim)
        {
            sprintf( buf1, "$n %s $melf%s%c {r[%s%d{r]{x",vp,res,punct, dam_color, dam);
            sprintf( buf2, "You %s yourself%s%c {r[%s%d{r]{x",vs,res,punct, dam_color, dam);
        }
        else
        {
			//
            sprintf( buf1, "$n %s $N%s%c {r[%s%d{r]{x",  vp, res, punct, dam_color, dam );
            sprintf( buf2, "You %s $N%s%c {r[%s%d{r]{x", vs, res, punct, dam_color, dam );
            sprintf( buf3, "$n %s you%s%c {r[%s%d{r]{x", vp, res, punct, dam_color, dam );
        }
    }
    else
    {			
		int rnd = number_range(1,3);
		
		if (dam_type == DAM_BASH)
		{
			if ( IS_WEAPON(obj) )	
			{
				switch (rnd)
				{
					default:
					case 1:
						attack = str_dup( "overhead crush" );
						break;
					case 2:
						attack = str_dup( "rising smash" );
						break;
					case 3:
						attack = str_dup( "massive crush" );
						break;
				}				
			}
			else
			{
				switch (rnd)
				{
					default:
					case 1:
						attack = str_dup( "hook" );
						break;
					case 2:
						attack = str_dup( "rising uppercut" );
						break;
					case 3:
						attack = str_dup( "haymaker" );
						break;
				}
			}
		}
		else if (dam_type == DAM_SLASH)
		{
			if ( IS_WEAPON(obj) )
			{
				switch (rnd)
				{
					default:
					case 1:
						attack = str_dup( "downward slash" );	
						break;
					case 2:
						attack = str_dup( "upward slash" );	
						break;
					case 3:
						attack = str_dup( "spinning slash" );
						break;
				}	
			}
			else
			{
				switch (rnd)
				{
					default:
					case 1:
						attack = str_dup( "falling claw" );
						break;
					case 2:
						attack = str_dup( "rising claw" );
						break;
					case 3:
						attack = str_dup( "claw swipe" );
						break;
				}
					
			}	
		}
		else if (dam_type == DAM_PIERCE)
		{
			switch (rnd)
			{
				default:
				case 1:
					attack = str_dup( "thrusting stab" );
					break;
				case 2:
					attack = str_dup( "tearing stab" );
					break;
				case 3:
					attack = str_dup( "ripping thrust" );
					break;
			}
		}

		else if ( IS_MONK(ch) && !IS_NPC( ch ) )
		{
			if ( obj && IS_WEAPON(obj) )
			{
				if ( dam_type == DAM_SLASH )
					attack = str_dup( "slash" );
				else if ( dam_type == DAM_PIERCE )
					attack = str_dup( "stab" );
				else if ( dam_type == DAM_BASH )
					attack = str_dup( "crush" );
				else if ( dam_type > 0 && dam_type < MAX_DAMAGE_MESSAGE )
					attack = str_dup( attack_table[obj->value[3]].noun );
				else
				{ 
					attack = str_dup ( "attack" );
					bug( "Dam_message: bad dt (%d).", dam_type );
				}		
			}
			else
			{
				if (get_skill(ch, gsn_iron_fist) > 1 && number_percent() < 50)
				{
					attack = "iron fist";
				}
				else
				{
					int x;
					x = number_range(1,11);
					switch ( x )
					{
						default:  attack = "monk special"; break;
						case 1:   
						case 2:
							attack = "elbow smash"; break;
						case 3:   
						case 4:					
							attack = "rising knee"; break;					
						case 5:
						case 6:
						case 7:
							attack = "open handed strike"; break;
						case 8:   attack = "hand chop"; break;
						case 9:   attack = "fierce punch"; break;
						case 10:  attack = "roundhouse"; break;
						case 11:  attack = "sweep kick"; break;
					}
				}
			}
		}		
		//else if ( dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
		//	attack      = attack_table[dt - TYPE_HIT].noun;
		else if ( dt >= 0 && dt < MAX_SKILL )
			attack      = skill_table[dt].noun_damage;
		else
		{
			bug( "Dam_message: bad dt %d.", dt );
			dt  = TYPE_HIT;
			attack  = str_dup( "hit" );
		}
		
		if ( IS_NULLSTR( attack ) )
			attack  = str_dup( "hit" );
		
		if (immune)
		{
			if (ch == victim)
			{
				sprintf(buf1,"$n is unaffected by $s own %s.",attack);
				sprintf(buf2,"Luckily, you are immune to that.");
			}
			else
			{
				sprintf(buf1,"$N is unaffected by $n's %s!",attack);
				sprintf(buf2,"$N is unaffected by your %s!",attack);
				sprintf(buf3,"$n's %s is powerless against you.",attack);
				sprintf(buf4,"$N is unaffected by someones's %s!",attack);
			}
		}
		else
		{
			if (ch == victim)
			{
				sprintf( buf1, "$n's %s %s $m%s%c {r[%s%d{r]{x",attack,vp,res,punct, dam_color, dam);
				sprintf( buf2, "Your %s %s you%s%c {r[%s%d{r]{x",attack,vp,res,punct, dam_color, dam);
			}
			else
			{
				sprintf( buf1, "$n's %s %s $N%s%c {r[%s%d{r]{x",  attack, vp, res, punct, dam_color, dam );
				sprintf( buf2, "Your %s %s $N%s%c {r[%s%d{r]{x",  attack, vp, res, punct, dam_color, dam );

				sprintf( buf3, "$n's %s %s you%s%c {r[%s%d{r]{x", attack, vp, res, punct, dam_color, dam );
				sprintf( buf4, "Someone's %s %s $N%s%c {r[%s%d{r]{x",  attack, vp, res, punct, dam_color, dam );
			}
		}
	}
            
    if (ch == victim) 
	{
        act(buf1,ch,NULL,NULL,TO_ROOM);
        act(buf2,ch,NULL,NULL,TO_CHAR);
    } 
	else 
	{
        if ( same_room ) 
		{
            act(buf1, ch, NULL, victim, TO_NOTVICT );
            act(buf2, ch, NULL, victim, TO_CHAR );
        } 
		else 
		{
            act(buf4, victim, NULL, victim, TO_NOTVICT );
            act(buf2, ch, NULL, victim, TO_CHAR );
        }
        act(buf3, ch, NULL, victim, TO_VICT );
    }             
    return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int chance = get_skill(victim, gsn_power_grip);
    OBJ_DATA *obj;

    if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
	{
		SEND("But they aren't wielding anything!\r\n",ch);
        return;
	}
	
	if (KNOWS(victim,gsn_power_grip)) // they must have power grip to use it.
	{
		if (get_curr_stat (ch, STAT_STR) > 15)
			chance += (get_curr_stat (victim, STAT_STR) - 15);
		
		if (number_percent() < (chance / 2))
		{
			act ("{5$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
			act ("{5$n tries to disarm you, but your grip is too tight!{x",
				 ch, NULL, victim, TO_VICT);
			act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
				 TO_NOTVICT);
			check_improve (victim, gsn_power_grip, TRUE, 1);
			return;
		}
		else
			check_improve (victim, gsn_power_grip, FALSE, 1);
	}
	
    if (IS_OBJ_STAT (obj, ITEM_NOREMOVE))
    {
        act ("{5$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but your weapon won't budge!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_NOTVICT);
        return;
    }

	chance = get_skill(ch, gsn_disarm);
	if (get_curr_stat (ch, STAT_DEX) > 15)
			chance += (get_curr_stat (victim, STAT_DEX) - 15);
	
	if (number_percent() < chance)
	{
		act ("{5$n DISARMS you and sends your weapon flying!{x",
         ch, NULL, victim, TO_VICT);
		act ("{5You disarm $N!{x", ch, NULL, victim, TO_CHAR);
		act ("{5$n disarms $N!{x", ch, NULL, victim, TO_NOTVICT);

		obj_from_char (obj);
		if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY))
			obj_to_char (obj, victim);
		else
		{
			obj_to_room (obj, victim->in_room);
			if (IS_NPC (victim) && victim->wait == 0 && can_see_obj (victim, obj))
				get_obj (victim, obj, NULL);
		}
		check_improve (ch, gsn_disarm, TRUE, 1);
	}
	else
	{
		act ("You fail to disarm {5$S{x.", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but fails!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_NOTVICT);
		check_improve (ch, gsn_disarm, FALSE, 1);	
	}
    return;
}


void do_berserk (CHAR_DATA * ch, char *argument)
{
    int chance, hp_percent;

	if (ch->cooldowns[gsn_berserk] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if ((chance = get_skill (ch, gsn_berserk)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BERSERK))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_berserk].skill_level[ch->ch_class]))
    {
        SEND ("You turn red in the face, but nothing happens.\r\n",
                      ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_BERSERK) || is_affected (ch, gsn_berserk)
        || is_affected (ch, skill_lookup ("frenzy")))
    {
        SEND ("You get a little madder.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CALM))
    {
        SEND ("You're feeling to mellow to berserk.\r\n", ch);
        return;
    }

    if (ch->mana < 40)
    {
        SEND ("You can't get up enough energy.\r\n", ch);
        return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
        chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 2;

    if (number_percent () < chance)
    {
        AFFECT_DATA af;

        WAIT_STATE (ch, PULSE_VIOLENCE);
        ch->mana -= 40;
        ch->move /= 2;

        /* heal a little damage */
        ch->hit += ch->level * 2;
        ch->hit = UMIN (ch->hit, ch->max_hit);

        SEND ("Your pulse races as you are consumed by rage!\r\n",
                      ch);
        act ("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_ROOM);
        check_improve (ch, gsn_berserk, TRUE, 1);

        af.where = TO_AFFECTS;
        af.type = gsn_berserk;
        af.level = ch->level;
        af.duration = number_fuzzy (ch->level / 8);
        af.modifier = UMAX (1, ch->level / 5);
        af.bitvector = AFF_BERSERK;

        af.location = APPLY_HITROLL;
        affect_to_char (ch, &af);

        af.location = APPLY_DAMROLL;
        affect_to_char (ch, &af);

        af.modifier = UMAX (10, 10 * (ch->level / 5));
        af.location = APPLY_AC;
        affect_to_char (ch, &af);		
		COOLDOWN(ch, gsn_berserk);
    }

    else
    {
        WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
        ch->mana -= 25;
        ch->move /= 2;

        SEND ("Your pulse speeds up, but nothing happens.\r\n", ch);
        check_improve (ch, gsn_berserk, FALSE, 1);
    }
}


void do_battlecry (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	char buf[MSL];
	int chance = 0;
	
	if (IS_NPC(ch))
		return;
	
	if CHECK_COOLDOWN(ch, gsn_battlecry)	
	{	
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
		
	if ((chance = get_skill (ch, gsn_battlecry)) == 0 ||
        ch->level < skill_table[gsn_battlecry].skill_level[ch->ch_class])
    {
        SEND ("Your voice cracks, how embarassing!\r\n", ch);
        return;
    }    
	
	check_improve (ch, gsn_battlecry, TRUE, 1);
	COOLDOWN(ch, gsn_battlecry);
	
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {	
		if (is_same_group(gch,ch))
		{
			if (ch == gch)
				SEND ("You let out a powerful battlecry!\r\n",gch);
			else
			{
				sprintf(buf, "You are inspired by %s's battlecry!\r\n",ch->name);
				SEND(buf, gch);
			}
				
			act ("$n appears bolstered and ready for battle!", gch, NULL, NULL, TO_ROOM);

			if (!is_affected(gch, gsn_battlecry))
			{
				af.where = TO_AFFECTS;
				af.type = gsn_battlecry;
				af.level = ch->level;
				af.duration = number_fuzzy (ch->level / 8);
				af.modifier = UMAX (1, ch->level / 10);
				af.bitvector = 0;

				af.location = APPLY_DAMROLL;
				affect_to_char (gch, &af);
			}
		}
		else
		{
			if (!is_safe(ch,gch))
			{
				
				sprintf (buf,"You feel a little shaken up after hearing %s's battle cry!\r\n",ch->name);
				SEND(buf, gch);
				act ("$n looks a little shaken up!", gch, NULL, NULL, TO_ROOM);

				af.where = TO_AFFECTS;
				af.type = gsn_battlecry;
				af.level = ch->level;
				af.duration = number_fuzzy (ch->level / 8);
				af.modifier = UMAX (-1, -(ch->level / 10));
				af.bitvector = 0;

				af.location = APPLY_DAMROLL;
				affect_to_char (gch, &af);
			}
			
		}
	}
	return;
}

void do_bloodlust (CHAR_DATA * ch, char *argument)
{
    int chance, hp_percent;

	if (IS_NPC(ch))
		return;
	
	if (ch->cooldowns[gsn_bloodlust] > 0)
	{
		SEND ("You must wait to use that skill again.\r\n",ch);
		return;
	}
	
	if (is_affected(ch, gsn_bloodlust))
	{
		SEND ("You already lust for blood.\r\n",ch);
		return;
	}
	
	if (!is_affected(ch, gsn_berserk))
	{
		SEND ("You must first be berserking!\r\n",ch);
		return;
	}
	
    if ((chance = get_skill (ch, gsn_bloodlust)) == 0 ||
        !can_use_skill(ch,gsn_bloodlust))
    {
        SEND ("You turn red in the face, but nothing happens.\r\n",ch);
        return;
    }    

    if (IS_AFFECTED (ch, AFF_CALM))
    {
        SEND ("You're feeling too mellow to bloodlust.\r\n", ch);
        return;
    }

    if (ch->mana < 40)
    {
        SEND ("You can't get up enough energy.\r\n", ch);
        return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
        chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 2;

    if (number_percent () < chance)
    {
        AFFECT_DATA af;

        WAIT_STATE (ch, PULSE_VIOLENCE);
        ch->mana -= 40;
        ch->move /= 2;

        /* heal a little damage */
        ch->hit += ch->level * 2;
        ch->hit = UMIN (ch->hit, ch->max_hit);

        SEND ("You howl in rage as you enter a bloodlust!\r\n",
                      ch);
        act ("$n howls as they get a bloodshot look in $s eyes.", ch, NULL, NULL, TO_ROOM);
        check_improve (ch, gsn_bloodlust, TRUE, 1);

        af.where = TO_AFFECTS;
        af.type = gsn_bloodlust;
        af.level = ch->level;
        af.duration = number_fuzzy (ch->level / 8);
        af.modifier = ch->level / 3;
        af.bitvector = 0;

        af.location = APPLY_MELEE_CRIT;
        affect_to_char (ch, &af);        

        af.modifier = UMAX (10, 10 * (ch->level / 5));
        af.location = APPLY_AC;
        affect_to_char (ch, &af);
		
		COOLDOWN(ch, gsn_bloodlust);
    }

    else
    {
        WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
        ch->mana -= 25;
        ch->move /= 2;

        SEND ("Your pulse speeds up, but nothing happens.\r\n", ch);
        check_improve (ch, gsn_bloodlust, FALSE, 1);
    }
}


void do_energy (CHAR_DATA * ch, char *argument)
{
	int chance = 0;
	int dam = 0;
	int level = 0;
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	
	if (IS_NPC(ch))
		return;

	if ((chance = get_skill (ch, gsn_energy_blast)) == 0
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_energy_blast].skill_level[ch->ch_class]))
    {
        SEND("You don't know how to do that.\r\n", ch);
        return;
    }
	
	if (ch->ki < 10)
	{
		SEND("You must channel more energy first.\r\n",ch);
		return;
	}
	
    if (arg[0] == '\0')
    {
        if (!ch->fighting)
        {
            SEND ("But you aren't fighting anyone!\r\n", ch);
            return;
        }
		else
		{
			victim = ch->fighting;
		}
    }
    else
    {
		if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
		{
			SEND ("They aren't here.\r\n", ch);
			return;
		}
    }

    if (victim == ch)
    {
        SEND ("Impossible.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
        return;
    }
		
	chance = number_range(1,100);
	chance += (get_skill(victim, gsn_energy_blast) / 5);
	chance += ((GET_WIS(ch) / 2) + (get_curr_stat (ch, STAT_DEX) / 2));
		
	if (number_percent() < chance)
	{
		ch->ki -= 10;
		dam = dice (level, 8);
		if (saves_spell (level, victim, DAM_ENERGY))
			dam /= 2;
		damage (ch, victim, dam, gsn_energy_blast, DAM_ENERGY, TRUE);		
	}
	else
	{
		SEND("You fail to channel your ki into a proper energy blast.\r\n",ch);
		return;
	}

	return;
}


void do_throw( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, dam, dam_type = 0, fcatch;
        
    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);

    if (IS_NPC(ch))
		return;

    if (arg[0] == '\0')
    {
        SEND("Throw what?\n\r",ch);
        return;
    }
        
    if ( arg2[0] == '\0' )
    {
        victim = ch->fighting;
		if (victim == NULL)
		{
			SEND("You aren't fighting anyone!\n\r", ch);
			return;
		}
    }
    else if ((victim = get_char_room(ch,NULL, arg2)) == NULL)
    {
        SEND("They aren't here.\n\r", ch);
        return;
    }

    if ( !IS_NPC(ch) && !dex_check(ch, -2))
    {
		SEND("You clumsily trip over your own feet!\n\r",ch);
		ch->daze += number_range(0,1);
		return;
    }

    if (is_safe(ch, victim))
    {
		SEND("Don't even think of it.\n\r",ch);
		return;
    }    

    if (victim == NULL)
    {
        SEND("You aren't fighting anyone.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
	SEND("You can't throw things at yourself!\n\r",ch);
	return;
    }
     
    if ( (obj = get_obj_carry(ch, arg, ch)) == NULL )
    {
        SEND("You aren't carrying that.\n\r", ch);
        return;
    }

    if ( IS_SET(obj->extra_flags, ITEM_NODROP) )
    {
        SEND("You can't let go of it!\n\r", ch);
        return;
    }
    if ( obj->contains != NULL )
    {
		act( "You can't seem to get a good hold on $p.",ch,obj,NULL,TO_CHAR);
		return;
    }

    if ( IS_NPC(victim) && 
	victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        SEND("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }

    if (( obj->item_type == ITEM_CORPSE_NPC )
        && (get_curr_stat(ch,STAT_STR) != 25))
    {
        SEND("You are too weak to perform such a feat!\n\r",ch);
        return;
    }

    if ( ch->move < total_levels(ch) / 10)
    {
	SEND("You are too exhausted to go on.\n\r",ch);
	return;
    }
    ch->move -= number_range( 0, (total_levels(ch) / 10));

    check_killer(ch, victim);

    dam = (get_curr_stat(ch,STAT_STR));
     
    dam += (obj->weight / 10 ) * (total_levels(ch) / 50);
        
/* check all the different variable objects */

    if ( obj->item_type == ITEM_WEAPON )
                dam *= 2;
    if ( obj->item_type == ITEM_ARMOR
        || obj->item_type == ITEM_FURNITURE )
                dam = 3 * dam / 2;
    if ( obj->item_type == ITEM_SCROLL
        || obj->item_type == ITEM_FOOD
        || obj->item_type == ITEM_CONTAINER   
        // || obj->item_type == ITEM_GEM_CONTAINER   
        // || obj->item_type == ITEM_QUIVER
        || obj->item_type == ITEM_CORPSE_NPC )
                dam /= 2;
     
    if ( obj->item_type == ITEM_MINING_TOOL)
	dam /= 2;

    if ( obj->item_type == ITEM_FISH_POLE)
	dam /= 5;

/* compute the chance */
     
    chance = number_percent( );
    chance += get_curr_stat(ch,STAT_DEX) / 2;
    chance += (get_curr_stat(ch,STAT_STR) / 2);
    chance -= ((get_curr_stat(victim,STAT_DEX)) * 2);

    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
            chance += 10;
    if (is_affected(ch, gsn_quicken_tempo))
	    chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
            chance -= 20;

    chance += (ch->level - victim->level) * 4;
    chance = URANGE(5, chance, 95);

/* this is the ability to catch the obj modifiers */

    fcatch = number_percent( );
    fcatch -= (ch->level - victim->level ) * 5;
    fcatch += 2 *(get_curr_stat(victim, STAT_STR) / 5);        
    fcatch += (get_curr_stat(victim, STAT_DEX) / 5);        
    fcatch -= (chance / 5);

/* this is for the non-assassins */   
    //Get rid of owner
    //assign_owner(ch, obj, NULL);
    
    if (ch->ch_class != THIEF )
    {        
        WAIT_STATE(ch,skill_table[gsn_throw].beats);
/* successful hit */
     
        if ( chance >= 40 ) 
        {
           act("You throw $p at $N in desperaton!",ch,obj,victim,TO_CHAR);
           act("$p comes hurling at you!",ch,obj,victim,TO_VICT);
           act("$n throws whatever he can find at $N!",ch,obj,
                victim,TO_NOTVICT);
            
/* damage types */

           if (obj->item_type == ITEM_POTION )
           {
                act("$p shatters, pouring it's contents on you!",ch,obj,victim,TO_VICT);
                act("$p shatters, pouring it's contents on $N!",ch,obj,victim,TO_NOTVICT);
    		obj_cast_spell( obj->value[1], obj->value[0]/2, ch, victim, NULL );
    		obj_cast_spell( obj->value[2], obj->value[0]/2, ch, victim, NULL );
    		obj_cast_spell( obj->value[3], obj->value[0]/2, ch, victim, NULL );
                extract_obj(obj);
		if(!victim->fighting && victim->position <= POS_SLEEPING){
		    stop_fighting(ch,FALSE);
		    stop_fighting(victim,TRUE);
		    if(victim->position > POS_SLEEPING)
				victim->position = POS_SLEEPING;
		}
		else
                    damage(ch,victim,3,gsn_throw,DAM_PIERCE,FALSE);
                return;
           }
           else if (obj->item_type == ITEM_WEAPON)
                dam_type = attack_table[obj->value[3]].damage;

           else 
				dam_type = DAM_BASH;
                
/* a check to see if the other person catches it */
        
           if ( fcatch >= 50 && /*hasArms(victim)*/ dex_check(victim, 2))
           {
                obj_from_char(obj);
                obj_to_char(obj, victim);
                act("$N manages to catch $p!",ch,obj,victim,TO_CHAR);
                act("$N manages to catch $p!",ch,obj,victim,TO_NOTVICT);
                act("You manage to hold onto $p.",ch,obj,victim,TO_VICT);
                return;
           }
           damage(ch,victim,dam,gsn_throw,DAM_BASH,TRUE);
        
		   act( "$p lands on the ground.",ch,obj,NULL,TO_ALL);
           obj_from_char(obj);
           obj_to_room(obj, ch->in_room);
           return;
        }   
                
/* a total miss */
        else    
        {
           if (obj->item_type == ITEM_POTION )
           {
                act("$p shatters on the ground!",ch,obj,victim,TO_ALL);
                extract_obj(obj);
                return;
           }
     
           act("You throw $p aimlessly at $N!",ch,obj,victim,TO_CHAR);
           act("A thrown $p misses you miserably!",ch,obj,victim,
                TO_VICT);
           act("$n throws $p to the ground.",ch,obj, victim,TO_NOTVICT);
           
           obj_from_char(obj);
           obj_to_room(obj, ch->in_room);
           return;
        }       
    }
           
/* assassin with the skill throwing */
                
    else
    {
        WAIT_STATE(ch,skill_table[gsn_throw].beats);

/* checks to see if the assassin hits */

        if (chance >= 96);
			dam *= 2;
			
		chance += 15;
        chance = URANGE(5, chance, 95);

           if (chance >= 100 - get_skill(ch,gsn_throw) )
           {
              act("You throw $p skillfully at $N!",ch,obj,victim,TO_CHAR);
              act("A thrown $p hits you forcefully!",ch,obj,victim,
                        TO_VICT);
              act("$n hurls $p at his enemies.",ch,obj,victim,TO_NOTVICT);
     
/* the normal default thrown item */

/* checks for special cases - potions and weapons */
           
              if (obj->item_type == ITEM_POTION )
              {
                act("$p shatters, pouring it's contents on you!",ch,obj,victim,TO_VICT);
                act("$p shatters, pouring it's contents on $N!",ch,obj,victim,TO_NOTVICT);
    		obj_cast_spell( obj->value[1], 3 * obj->value[0]/2, ch, victim, NULL );
    		obj_cast_spell( obj->value[2], 3 * obj->value[0]/2, ch, victim, NULL );
    		obj_cast_spell( obj->value[3], 3 * obj->value[0]/2, ch, victim, NULL );
                extract_obj(obj);
                check_improve(ch,gsn_throw,TRUE,2);
		if(!victim->fighting && victim->position <= POS_SLEEPING){
		    stop_fighting(ch,FALSE);
		    stop_fighting(victim,TRUE);
		    if(victim->position > POS_SLEEPING)
			victim->position = POS_SLEEPING;
		}
                else
		    damage(ch,victim,3,gsn_throw,DAM_PIERCE,FALSE);
                return;
              }

              if (obj->item_type == ITEM_WEAPON)
                  dam_type = attack_table[obj->value[3]].damage;

              else
                  dam_type = DAM_BASH;

/* a check to see if you catch it */
               
              if ( fcatch >= 50 && /*hasArms(victim)*/ dex_check(victim, 2))
              {
                obj_from_char(obj);
                obj_to_char(obj, victim);
                check_improve(ch,gsn_throw,TRUE,2);
                act("$N manages to catch $p!",ch,obj,victim,TO_CHAR);
                act("$N manages to catch $p!",ch,obj,victim,TO_NOTVICT);
                act("You manage to hold onto $p.",ch,obj,victim,TO_VICT);
                return;
              }
              damage(ch,victim,dam,gsn_throw,dam_type,TRUE);
                
	      act( "$p lands on the ground.",ch,obj,NULL,TO_ALL);
              check_improve(ch,gsn_throw,TRUE,2);
              obj_from_char(obj);
              obj_to_room(obj, ch->in_room);
              return;
           }

/* an assassin missing with a throw skill, a rarity */
                
           else
           {   

/* another bunch of checks */

              if (obj->item_type == ITEM_POTION )
              {
                 act("$p shatters on the ground!",ch,obj,victim,TO_ALL);
                 check_improve(ch,gsn_throw,FALSE,2);
                 extract_obj(obj);
                 return;
              } 
              act("You throw $p at $N missing him!",ch,obj,victim,TO_CHAR);
              act("A thrown $p misses you miserably!",ch,obj,victim,
                TO_VICT);
              act("$n throws $p to the ground.",ch,obj,victim,
                TO_NOTVICT);

              check_improve(ch,gsn_throw,FALSE,2);               
              obj_from_char(obj);
              obj_to_room(obj, ch->in_room);
              return;
           }
    }
}



void do_shield_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    sh_int chance, dam, ch_size, vic_size, daze;
    bool spiked = FALSE;

    one_argument(argument,arg);
 
	if (IS_NPC(ch))
		return;

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    SEND("But you aren't fighting anyone!\r\n",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,NULL, arg)) == NULL)
    {
	SEND("They aren't here.\r\n",ch);
	return;
    }

    if ((obj = get_eq_char (ch, WEAR_SHIELD)) == NULL)
    {
		SEND("You have to be wearing a shield to bash with it, obviously.\r\n",ch);
		act( "$n flexes one of $s arms, looking at it strangely.\r\n",ch,NULL,NULL,TO_ROOM);
		//WAIT_STATE(ch, PULSE_VIOLENCE);
		return;
    }    

    if (victim == ch)
    {
		act("You hit yourself in the head with $p.",ch,obj,NULL,TO_CHAR);
		act("$n hits $mself in the head with $p.",ch,obj,NULL,TO_ROOM);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		DAZE_STATE(ch, 2 * PULSE_VIOLENCE);
		damage(ch,ch,dice(ch->level, obj->value[4]),gsn_shield_bash,DAM_BASH,TRUE);
		return;
    }
	
    if (is_safe(ch,victim))
	return;
    check_killer(ch,victim);    

    if ( IS_NPC(victim) && 
	victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        SEND("Kill stealing is not permitted.\r\n",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }

    if ( ch->move < total_levels(ch) / 5)
    {
		SEND("You are too exhausted to go on.\r\n",ch);
		return;
    }

    ch->move -= number_range( 2, (total_levels(ch) / 5));

    if ( !IS_NPC(ch) && !dex_check(ch, -4))
    {
		SEND("You clumsily trip over your own feet!\r\n",ch);
		ch->daze += number_range(0,1);
		return;
    }

    ch_size = what_size(ch);
    vic_size = what_size(victim);

    // if ( ch->riding != 0 )
    // {
		// if ( check_mount_inroom(ch))
			// ch_size++;
    // }
    // if ( victim->riding != 0 )
    // {
	// if ( check_mount_inroom(victim))
	    // vic_size++;
    // }

    if ( IS_SET(obj->extra2_flags, ITEM_SPIKED))
		spiked = TRUE;

    dam = dice(ch->level, obj->value[4] + spiked ? 1 : 0);

    chance = get_skill(ch,gsn_shield_bash);
    dam = (dam * chance) / 110;
    dam = (dam * obj->condition)/100;

    chance -= spiked ? 30 : 20 - (obj->value[4] * 5 );

    // if ( IS_RIDING(ch))
	// dam += ch_size * 5;

    // if ( is_affected(victim,gsn_immovability) )
	// dam /= 2;

    /* size  and weight */

    daze = (obj->value[4] / 2) + 1; 

    if (ch_size > vic_size)
    {
	chance += (ch_size - vic_size) * 15;
	dam += 5;
	daze++;
    }
    else
    {
	chance += (ch_size - vic_size) * 10; 
	daze--;
    }
    //daze = UMAX(1, daze);
    if (daze > 4)
    {
	daze = 4;
    }

    // if ( !IS_AFFECTED(victim, AFF_FLYING))
    	// chance -= get_skill(victim,gsn_duck) / 10;
	    
    /* stats */
    chance += STAT_MOD(20,ch,STAT_STR,4);
    chance -= STAT_MOD(20,victim,STAT_DEX,5);
    chance -= GET_AC(victim,AC_BASH) /25;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (is_affected(ch, gsn_quicken_tempo))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level) * 3;

    chance = URANGE(5,chance,95);

    if (number_percent() < chance )
    {
	/*if ( victim->riding != 0 && number_percent() > 
		(get_skill(victim, gsn_riding ) / 2) + get_curr_stat(victim, STAT_DEX ))
	{
	    if ( IS_RIDING(victim))
	    {
		act( "$n crushes you with $p, knocking you off of your mount.",ch,obj,victim,TO_VICT);
		act( "$n crushes $N with $p, sending $m flying off $s mount!",ch,obj,victim,TO_NOTVICT);
		act( "You crush $p into $N, sending $M flying off $S mount!",ch,obj,victim,TO_CHAR);

		clear_mount(victim, TRUE);
		check_improve(ch,gsn_shield_bash,TRUE,5);

		DAZE_STATE(victim, daze * PULSE_VIOLENCE);
		WAIT_STATE(ch,number_range(1,2) * PULSE_VIOLENCE);
		WAIT_STATE(victim, PULSE_VIOLENCE);
		check_improve(ch,gsn_shield_bash,TRUE,5);
		damage(ch,victim,dam,gsn_shield_bash,spiked ? DAM_PIERCE : DAM_BASH,TRUE);
		will_break(ch,victim,obj,1);
	        return;
	    }
        }*/

    	act("$n smashes $p into you!",ch,obj,victim,TO_VICT);
		act("You smash $p into $N!",ch,obj,victim,TO_CHAR);
		act("$n smashes $p into $N violently!",ch,obj,victim,TO_NOTVICT);
		check_improve(ch,gsn_shield_bash,TRUE,3);
		
		if (IS_SET(obj->extra2_flags, ITEM_SPIKED) && number_percent() < 50)
		{
			victim->bleeding+= number_range(1,3);
			act("The spikes on $p stab into you, causing bloodflow!",ch,obj,victim,TO_VICT);
			act("The spikes on $p stab into $N!",ch,obj,victim,TO_CHAR);
			act("$p stabs into $N tearing open flesh!",ch,obj,victim,TO_NOTVICT);
		}

		if ( number_percent() < chance / 2 )
			DAZE_STATE(victim, daze * PULSE_VIOLENCE);

		WAIT_STATE(ch,number_range(1,2) * PULSE_VIOLENCE);
		damage(ch,victim,dam,gsn_shield_bash,spiked ? DAM_PIERCE : DAM_BASH, TRUE);
	}
	else
	{
		damage(ch,victim,0,gsn_shield_bash,DAM_BASH,FALSE);
		act("You miss with $p.", ch,obj,victim,TO_CHAR);
		act("$n swings $p awfully close to hitting $N.", ch,obj,victim,TO_NOTVICT);
		act("$n swings $p awfully close to hitting you.", ch,obj,victim,TO_VICT);
		check_improve(ch,gsn_shield_bash,FALSE,5);
		WAIT_STATE(ch,2*PULSE_VIOLENCE); 
	}
    //will_break(ch,victim,obj,1);
    return;
}

void do_caltrops( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, dam;

    one_argument(argument,arg);

	if (IS_NPC(ch))
		return;

    if ( (chance = get_skill(ch,gsn_caltrops)) == 0 || total_levels(ch) < skill_table[gsn_caltrops].skill_level[ch->ch_class])
    {
		SEND("Throw spikes on the ground? No way!\r\n",ch);
		return;
    }

    if (arg[0] == '\0')
    {
		victim = ch->fighting;
		if (victim == NULL)
		{
			SEND("But you aren't in combat!\r\n",ch);
			return;
		}
    }

    else if ((victim = get_char_room(ch,NULL, arg)) == NULL)
    {
		SEND("They aren't here.\r\n",ch);
		return;
    }
    if (victim == ch)
    {
		SEND("That might hurt.\r\n",ch);
		return;
    }    

    if (is_safe(ch,victim))
		return;
	check_killer(ch,victim);

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        SEND("Kill stealing is not permitted.\r\n",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }


    // if ( victim->mount != NULL && check_mount_inroom( ch ))
	// victim = victim->mount;

    if (!dex_check(ch, 0) )
    {
		SEND("You clumsily throw caltrops everywhere!\r\n",ch);
		ch->daze += number_range(0,1);
		return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (is_affected(ch, gsn_quicken_tempo))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (total_levels(ch) - total_levels(victim) * 2);

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance += 15;	break;
	case(SECT_CITY):		chance += 10;	break;
	case(SECT_ROAD):		chance += 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance -= 10;   break;
        case(SECT_BEACH):               chance -= 10;   break;
        case(SECT_SHORELINE):           chance -=  5;   break;
        case(SECT_CAVE):                chance -= 10;   break;
        case(SECT_SWAMP):               chance -=  5;   break;
        case(SECT_SNOW):                chance -= 10;   break;
        case(SECT_RUINS):               chance += 10;   break;
        case(SECT_WASTELAND):           chance -= 15;   break;
        case(SECT_JUNGLE):              chance += 15;   break;
    }

    if (ch->in_room->sector_type == SECT_AIR
         || ch->in_room->sector_type == SECT_VOID
         || ch->in_room->sector_type == SECT_LAVA
         || ch->in_room->sector_type == SECT_ICE
         || ch->in_room->sector_type == SECT_OCEANFLOOR
	 || ch->in_room->sector_type == SECT_WATER_SWIM
	 || ch->in_room->sector_type == SECT_WATER_NOSWIM )
    {
	SEND("Your caltrops aren't effective here.\r\n",ch);
	return;
    }

	if ( IS_AFFECTED( victim, AFF_FLYING ) || is_affected (victim, gsn_fly))
    {
		SEND("They're flying, that won't affect them.\r\n",ch);
		return;
    }
	
    dam = dice( total_levels(ch), 7 );

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	int level;

	act("$n starts to limp from the spikes in $s feet!",victim,NULL,NULL,
		TO_ROOM);
	act("$n throws a handful of spikes at your feet!",ch,NULL,victim,
		TO_VICT);

	level = total_levels(ch);
	
    if (!is_affected(victim,gsn_caltrops))
	{
	   	af.where	= TO_AFFECTS;
		af.type 	= gsn_caltrops;
		af.level 	= level;
		af.duration	= level/10;
		af.modifier	= -(level/8);
		af.bitvector 	= 0;

		af.location = APPLY_HITROLL;
		affect_to_char(victim,&af);

                af.location 	= APPLY_DEX;
        	af.duration     = level/10;
        	af.modifier     = -(level/8);
        	af.bitvector    = 0;                
    		affect_to_char(victim,&af);
	}

	damage(ch,victim,dam,gsn_caltrops,DAM_PIERCE,TRUE);
	SEND("You start to limp!\r\n",victim);
	check_improve(ch,gsn_caltrops,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_caltrops].beats);
    }
    else
    {
	act("You miss $N with your caltrops!",ch,NULL,victim,
		TO_CHAR);
	act("$n misses $N with a handful of spikes!",ch,NULL,victim,
		TO_NOTVICT);
	act("$n throws a handful of spikes at you and misses!",ch,NULL,victim,
		TO_VICT);
	damage(ch,victim,0,gsn_caltrops,DAM_PIERCE,TRUE);
	check_improve(ch,gsn_caltrops,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_caltrops].beats);
    }
    return;
	
}


void do_bash (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if ((chance = get_skill (ch, gsn_bash)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BASH))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_bash].skill_level[ch->ch_class]))
    {
        SEND ("Bashing? What's that?\r\n", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            SEND ("But you aren't fighting anyone!\r\n", ch);
            return;
        }
    }

    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act ("You'll have to let $M get back up first.", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        SEND ("You try to bash your brains out, but fail.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (what_size(ch) < what_size(victim))
        chance += (what_size(ch) - what_size(victim)) * 15;
    else
        chance += (what_size(ch) - what_size(victim)) * 10;

		
    /* stats */
    chance += GET_STR(ch);
    chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC (victim, AC_BASH) / 25;
    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 30;
		
	if (KNOWS(victim, gsn_balance) && (number_percent() < get_skill(victim, gsn_balance)))
	{
		chance -= (get_skill(victim, gsn_balance) / 5);
		check_improve (ch, gsn_balance, TRUE, 2);
	}

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
    {                            
	   act("{5$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
	   act("{5$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
	   WAIT_STATE(ch,skill_table[gsn_bash].beats);
	   return;         
    }

    /* now the attack */
    if (number_percent () < chance)
    {

        act ("{5$n sends you sprawling with a powerful bash!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5You slam into $N, and send $M flying!{x", ch, NULL, victim,
             TO_CHAR);
        act ("{5$n sends $N sprawling with a powerful bash.{x", ch, NULL,
             victim, TO_NOTVICT);
        check_improve (ch, gsn_bash, TRUE, 2);

        DAZE_STATE (victim, 3 * PULSE_VIOLENCE);
        WAIT_STATE (ch, skill_table[gsn_bash].beats);
        victim->position = POS_RESTING;
        damage (ch, victim, number_range (2, 2 + 2 * what_size(ch) + chance / 20),
                gsn_bash, DAM_BASH, FALSE);

    }
    else
    {
        damage (ch, victim, 0, gsn_bash, DAM_BASH, FALSE);
        act ("{5You fall flat on your face!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n falls flat on $s face.{x", ch, NULL, victim, TO_NOTVICT);
        act ("{5You evade $n's bash, causing $m to fall flat on $s face.{x",
             ch, NULL, victim, TO_VICT);
        check_improve (ch, gsn_bash, FALSE, 2);
        ch->position = POS_RESTING;
        WAIT_STATE (ch, skill_table[gsn_bash].beats * 3 / 2);
    }
    check_killer (ch, victim);
}

void do_dirt (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if ((chance = get_skill (ch, gsn_dirt)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK_DIRT))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_dirt].skill_level[ch->ch_class]))
    {
        SEND ("You get your feet dirty.\r\n", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            SEND ("But you aren't in combat!\r\n", ch);
            return;
        }
    }

    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (victim, AFF_BLIND))
    {
        act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        SEND ("Very funny.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= 2 * get_curr_stat (victim, STAT_DEX);

    /* speed  */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
        chance += 1;

    /* terrain */

    switch (ch->in_room->sector_type)
    {
        case (SECT_INSIDE):
            chance -= 20;
            break;
        case (SECT_CITY):
            chance -= 10;
            break;
        case (SECT_FIELD):
            chance += 5;
            break;
        case (SECT_FOREST):
            break;
        case (SECT_HILLS):
            break;
        case (SECT_MOUNTAIN):
            chance -= 10;
            break;
        case (SECT_WATER_SWIM):
            chance = 0;
            break;
        case (SECT_WATER_NOSWIM):
            chance = 0;
            break;
		case (SECT_OCEANFLOOR):
			chance = 0;
			break;
        case (SECT_AIR):
            chance = 0;
            break;
        case (SECT_DESERT):
            chance += 10;
            break;
    }

    if (chance == 0)
    {
        SEND ("There isn't any dirt to kick.\r\n", ch);
        return;
    }

    /* now the attack */
    if (number_percent () < chance)
    {
        AFFECT_DATA af;
        act ("{5$n is blinded by the dirt in $s eyes!{x", victim, NULL, NULL,
             TO_ROOM);
        act ("{5$n kicks dirt in your eyes!{x", ch, NULL, victim, TO_VICT);        
        SEND ("{5You can't see a thing!{x\r\n", victim);
        check_improve (ch, gsn_dirt, TRUE, 2);
        WAIT_STATE (ch, skill_table[gsn_dirt].beats);

        af.where = TO_AFFECTS;
        af.type = gsn_dirt;
        af.level = ch->level;
        af.duration = 0;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.bitvector = AFF_BLIND;

        affect_to_char (victim, &af);
		damage (ch, victim, number_range (2, 5), gsn_dirt, DAM_NONE, FALSE);
    }
    else
    {
        damage (ch, victim, 0, gsn_dirt, DAM_NONE, TRUE);
        check_improve (ch, gsn_dirt, FALSE, 2);
        WAIT_STATE (ch, skill_table[gsn_dirt].beats);
    }
    check_killer (ch, victim);
}

void do_trip (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if ((chance = get_skill (ch, gsn_trip)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_TRIP))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_trip].skill_level[ch->ch_class]))
    {
        SEND ("Tripping?  What's that?\r\n", ch);
        return;
    }


    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            SEND ("But you aren't fighting anyone!\r\n", ch);
            return;
        }
    }

    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (victim, AFF_FLYING))
    {
        act ("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act ("$N is already down.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        SEND ("{5You fall flat on your face!{x\r\n", ch);
        WAIT_STATE (ch, 2 * skill_table[gsn_trip].beats);
        act ("{5$n trips over $s own feet!{x", ch, NULL, NULL, TO_ROOM);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* size */
    if (what_size(ch) < what_size(victim))
        chance += (what_size(ch) - what_size(victim)) * 10;    /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;
	
	if (get_skill(victim, gsn_balance) > 1)
	{
		chance -= (get_skill(victim, gsn_balance) / 5);
	}
	
    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent () < chance)
    {
        act ("{5$n trips you and you go down!{x", ch, NULL, victim, TO_VICT);
        act ("{5You trip $N and $N goes down!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n trips $N, sending $M to the ground.{x", ch, NULL, victim,
             TO_NOTVICT);
        check_improve (ch, gsn_trip, TRUE, 1);

        DAZE_STATE (victim, 2 * PULSE_VIOLENCE);
        WAIT_STATE (ch, skill_table[gsn_trip].beats);
        victim->position = POS_RESTING;
        damage (ch, victim, number_range (2, 2 + 2 * what_size(victim)), gsn_trip,
                DAM_BASH, TRUE);		
    }
    else
    {
        damage (ch, victim, 0, gsn_trip, DAM_BASH, TRUE);
        WAIT_STATE (ch, skill_table[gsn_trip].beats * 2 / 3);
        check_improve (ch, gsn_trip, FALSE, 1);
    }
    check_killer (ch, victim);
}



void do_kill (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Kill whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }
/*  Allow player killing
    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
        &&   !IS_SET(victim->act, PLR_THIEF) )
        {
            SEND( "You must MURDER a player.\r\n", ch );
            return;
        }
    }
*/
    if (victim == ch)
    {
        SEND ("You hit yourself.  Ouch!\r\n", ch);
        multi_hit (ch, ch, TYPE_UNDEFINED);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
        SEND ("You do the best you can!\r\n", ch);
        return;
    }

    WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
    check_killer (ch, victim);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return;
}



void do_murde (CHAR_DATA * ch, char *argument)
{
    SEND ("If you want to MURDER, spell it out.\r\n", ch);
    return;
}



void do_murder (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Murder whom?\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM)
        || (IS_NPC (ch) && IS_SET (ch->act, ACT_PET)))
        return;

    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim == ch)
    {
        SEND ("Suicide is a mortal sin.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
        SEND ("You do the best you can!\r\n", ch);
        return;
    }

    WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
    if (IS_NPC (ch))
        sprintf (buf, "Help! I am being attacked by %s!", ch->short_descr);
    else
        sprintf (buf, "Help!  I am being attacked by %s!", ch->name);
    do_function (victim, &do_yell, buf);
    check_killer (ch, victim);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return;
}



void do_backstab (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument (argument, arg);

	if (ch->cooldowns[gsn_backstab] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if (arg[0] == '\0')
    {
        SEND ("Backstab whom?\r\n", ch);
        return;
    }

    if (ch->fighting != NULL)
    {
        SEND ("You're facing the wrong end.\r\n", ch);
        return;
    }

    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim == ch)
    {
        SEND ("How can you sneak up on yourself?\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
        SEND ("You need to wield a weapon to backstab.\r\n", ch);
        return;
    }

    if (victim->hit < victim->max_hit / 3)
    {
        act ("$N is hurt and suspicious ... you can't sneak up.",
             ch, NULL, victim, TO_CHAR);
        return;
    }

    check_killer (ch, victim);
    WAIT_STATE (ch, skill_table[gsn_backstab].beats);
	OBJ_DATA * dual;
	int hits = 1;
	dual = get_eq_char(ch, WEAR_SECONDARY);
	if (dual != NULL && dual->item_type == ITEM_WEAPON && (number_percent() < (get_skill(ch, gsn_dual_wield)) / 2))	
	{
		hits++;	
		check_improve (ch, gsn_dual_wield, FALSE, 1);
	}
	if (is_affected(ch, gsn_quicken_tempo) || IS_AFFECTED(ch, AFF_HASTE))
		hits++;
	while (hits > 0)
	{		
		if (number_percent() < get_skill(ch, gsn_backstab) || (get_skill(ch, gsn_backstab) >= 2 && !IS_AWAKE(victim)))
		{
			one_hit (ch, victim, gsn_backstab, FALSE);
			check_improve (ch, gsn_backstab, TRUE, 1);		
		}
		else
		{
			check_improve (ch, gsn_backstab, FALSE, 1);				
		}		
		hits--;
	}	
	ch->cooldowns[gsn_backstab] = skill_table[gsn_backstab].cooldown;
    return;
}



void do_flee (CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
    int attempt;

    if ((victim = ch->fighting) == NULL)
    {
        if (ch->position == POS_FIGHTING)
            ch->position = POS_STANDING;
        SEND ("You aren't fighting anyone.\r\n", ch);
        return;
    }

	if (is_affected(ch, gsn_web))
	{
		SEND ("Magical webs prevent your movement.\r\n",ch);
		return;
	}
	
	if (is_affected(ch, gsn_entangle))
	{
		SEND ("Magical vines and roots prevent your movement.\r\n",ch);
		return;
	}

	if (is_affected(ch, gsn_paralyzation))
	{
		SEND ("You can't, you're paralyzed!\r\n",ch);
		return;
	}
	
	if (is_affected(ch, gsn_stun_fist))
	{
		SEND ("You are still stunned from impact!\r\n",ch);
		return;
	}
	
    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++)
    {
        EXIT_DATA *pexit;
        int door;

        door = number_door ();
        if ((pexit = was_in->exit[door]) == 0
            || pexit->u1.to_room == NULL
            || IS_SET (pexit->exit_info, EX_CLOSED)
            || number_range (0, ch->daze) != 0 || (IS_NPC (ch)
                                                   && IS_SET (pexit->u1.
                                                              to_room->
                                                              room_flags,
                                                              ROOM_NO_MOB)))
            continue;

        move_char (ch, door, FALSE);
			
	
	
		
        if ((now_in = ch->in_room) == was_in)
            continue;

        ch->in_room = was_in;
        act ("$n has fled from combat!", ch, NULL, NULL, TO_ROOM);
        ch->in_room = now_in;
		
		if (!IS_NPC(ch) && ch->grank == G_FRONT)
		{
			for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( is_same_group( ch, vch ) && vch->grank != G_FRONT)
				{
					vch->grank = G_FRONT;
					SEND("Your group leader has {YFLED{x!\r\n",vch);
					SEND("You have been moved to the front of the group.\r\n",vch);
					break;
				}
			}	
		}

        if (!IS_NPC (ch))
        {
            SEND ("You flee from combat!\r\n", ch);
            if ((ch->ch_class == 2) && (number_percent () < 3 * (total_levels(ch) / 2)))
                SEND ("You snuck away safely.\r\n", ch);
            else
            {
                SEND ("You lost {B10{x experience points.\r\n", ch);               
				gain_exp (ch, -10, FALSE);
            }
        }

        stop_fighting (ch, TRUE);
        return;
    }

    SEND ("PANIC! You couldn't escape!\r\n", ch);
    return;
}



void do_rescue (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

	if (ch->cooldowns[gsn_rescue] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Rescue whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim == ch)
    {
        SEND ("What about fleeing instead?\r\n", ch);
        return;
    }

    if (!IS_NPC (ch) && IS_NPC (victim))
    {
        SEND ("Doesn't need your help!\r\n", ch);
        return;
    }

    if (ch->fighting == victim)
    {
        SEND ("Too late.\r\n", ch);
        return;
    }

    if ((fch = victim->fighting) == NULL)
    {
        SEND ("That person is not fighting right now.\r\n", ch);
        return;
    }

    if (IS_NPC (fch) && !is_same_group (ch, victim))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

	if (!IS_NPC(ch))
	{
		WAIT_STATE (ch, skill_table[gsn_rescue].beats);
		if (number_percent () > get_skill (ch, gsn_rescue))
		{
			SEND ("You fail the rescue.\r\n", ch);
			check_improve (ch, gsn_rescue, FALSE, 1);
			return;
		}
	}
	else
	{
		if (number_percent() > 50)
		{
			act ("{5$n tries to rescue you, but couldn't make it in time!{x", ch, NULL, victim, TO_VICT);
			return;
		}
	}
    act ("{5You rescue $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{5$n rescues you!{x", ch, NULL, victim, TO_VICT);
    act ("{5$n rescues $N!{x", ch, NULL, victim, TO_NOTVICT);
    check_improve (ch, gsn_rescue, TRUE, 1);

	ch->cooldowns[gsn_rescue] = skill_table[gsn_rescue].cooldown;
	
    stop_fighting (fch, FALSE);
    stop_fighting (victim, FALSE);

    check_killer (ch, fch);
    set_fighting (ch, fch);
    set_fighting (fch, ch);
    return;
}


void do_gore (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char buf[MSL];
	char arg[MSL];

	one_argument (argument, arg);
	
	if (IS_NPC (ch))
        return;
	
	if (ch->cooldowns[gsn_gore] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if (!IS_NPC (ch) && (ch->level < skill_table[gsn_gore].skill_level[ch->ch_class] || get_skill(ch, gsn_kick < 1)))
    {
        SEND ("You have some tusks or horns somewhere I don't know about?\r\n", ch);
        return;
    }

	if (arg[0] == '\0')
    {
		victim = ch->fighting;
		if (victim == NULL)
		{
			SEND("But you aren't in combat!\r\n",ch);
			return;
		}
	}
    else if ((victim = get_char_room(ch,NULL, arg)) == NULL)
    {
		SEND("They aren't here.\r\n",ch);
		return;
    }
    
    if (is_safe(ch,victim))
	{
		SEND ("They are not PvP.\r\n",ch);
		return;
	}

    WAIT_STATE (ch, skill_table[gsn_gore].beats);
    if (number_percent() < get_skill (ch, gsn_gore))
    {
        damage (ch, victim, number_range (total_levels(ch) / 2, total_levels(ch)), gsn_gore, DAM_BASH, TRUE);
        check_improve (ch, gsn_gore, TRUE, 2);
    }
    else
    {
        sprintf (buf, "Your gore attack {Dmisses{x %s.\r\n", victim->name);
		SEND(buf, ch);
        check_improve (ch, gsn_gore, FALSE, 2);
		set_fighting(ch,victim);
    }
    check_killer (ch, victim);
	COOLDOWN(ch, gsn_gore);	
	return;
}


void do_kick (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
	char buf[MSL];
	char arg[MSL];

	one_argument (argument, arg);
	
	if (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK))
        return;
	
	if (ch->cooldowns[gsn_kick] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if (!IS_NPC (ch)
        && (ch->level < skill_table[gsn_kick].skill_level[ch->ch_class] || get_skill(ch, gsn_kick < 1)))
    {
        SEND ("You better leave the martial arts to fighters.\r\n",
                      ch);
        return;
    }

	if (arg[0] == '\0')
    {
		victim = ch->fighting;
		if (victim == NULL)
		{
			SEND("But you aren't in combat!\r\n",ch);
			return;
		}
	}
    else if ((victim = get_char_room(ch,NULL, arg)) == NULL)
    {
		SEND("They aren't here.\r\n",ch);
		return;
    }
    
    if (is_safe(ch,victim))
	{
		SEND ("They are not PvP.\r\n",ch);
		return;
	}

    WAIT_STATE (ch, skill_table[gsn_kick].beats);
    if (number_percent() < get_skill (ch, gsn_kick))
    {
		int dam = number_range (total_levels(ch) / 2, total_levels(ch));
		if (IS_CENTAUR(ch))
			dam = dam * 5 / 4;
		
        damage (ch, victim,dam, gsn_kick, DAM_BASH, TRUE);
		
        check_improve (ch, gsn_kick, TRUE, 2);
    }
    else
    {
        sprintf (buf, "Your kick {Dmisses{x %s.\r\n", victim->name);
		SEND(buf, ch);
        check_improve (ch, gsn_kick, FALSE, 2);
		set_fighting(ch,victim);
    }
    check_killer (ch, victim);
	COOLDOWN(ch, gsn_kick);
    return;
}




void do_disarm (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

    hth = 0;

	if (ch->cooldowns[gsn_disarm] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if ((chance = get_skill (ch, gsn_disarm)) == 0)
    {
        SEND ("You don't know how to disarm opponents.\r\n", ch);
        return;
    }

    if ((get_eq_char (ch, WEAR_WIELD) == NULL && ((hth = get_skill (ch, gsn_hand_to_hand)) == 0 )) || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_DISARM)))
    {
        SEND ("You must wield a weapon to disarm.\r\n", ch);
        return;
    }

    if ((victim = ch->fighting) == NULL)
    {
        SEND ("You aren't fighting anyone.\r\n", ch);
        return;
    }

    if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
    {
        SEND ("Your opponent is not wielding a weapon.\r\n", ch);
        return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill (ch, get_weapon_sn (ch, FALSE));
    vict_weapon = get_weapon_skill (victim, get_weapon_sn (victim, FALSE));
    ch_vict_weapon = get_weapon_skill (ch, get_weapon_sn (victim, FALSE));

    /* modifiers */

    /* skill */
    if (get_eq_char (ch, WEAR_WIELD) == NULL)
        chance = chance * hth / 150;
    else
        chance = chance * ch_weapon / 100;

    chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

    /* dex vs. strength */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= 2 * get_curr_stat (victim, STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* and now the attack */
    if (number_percent () < chance)
    {
        WAIT_STATE (ch, skill_table[gsn_disarm].beats);
        disarm (ch, victim);
        check_improve (ch, gsn_disarm, TRUE, 1);
    }
    else
    {
        WAIT_STATE (ch, skill_table[gsn_disarm].beats);
        act ("{5You fail to disarm $N.{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but fails.{x", ch, NULL, victim,
             TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_NOTVICT);
        check_improve (ch, gsn_disarm, FALSE, 1);
    }
    check_killer (ch, victim);
    return;
}

void do_surrender (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *mob;
    if ((mob = ch->fighting) == NULL)
    {
        SEND ("But you're not fighting!\r\n", ch);
        return;
    }
    act ("You surrender to $N!", ch, NULL, mob, TO_CHAR);
    act ("$n surrenders to you!", ch, NULL, mob, TO_VICT);
    act ("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
    stop_fighting (ch, TRUE);

    if (!IS_NPC (ch) && IS_NPC (mob)
        && (!HAS_TRIGGER_MOB(mob, TRIG_SURR)
            || !p_percent_trigger (mob, NULL, NULL, ch, NULL, NULL, TRIG_SURR)))
    {
        act ("$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR);
        multi_hit (mob, ch, TYPE_UNDEFINED);
    }
}

void do_sla (CHAR_DATA * ch, char *argument)
{
    SEND ("If you want to SLAY, spell it out.\r\n", ch);
    return;
}



void do_slay (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Slay whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (ch == victim)
    {
        SEND ("Suicide is a mortal sin.\r\n", ch);
        return;
    }

    if (!IS_NPC (victim) && victim->level >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    act ("{1You slay $M in cold blood!{x", ch, NULL, victim, TO_CHAR);
    act ("{1$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT);
    act ("{1$n slays $N in cold blood!{x", ch, NULL, victim, TO_NOTVICT);
    raw_kill (victim);
    return;
}


bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
	int chance;
	int dam_type;
	OBJ_DATA *wield, *wield2;

	wield = get_eq_char(victim, WEAR_WIELD);
	wield2 = get_eq_char(victim, WEAR_SECONDARY);
	
	if (IS_NPC(victim))
		return FALSE;
	
	if (( !wield && !wield2 ) || (!IS_AWAKE(victim)) || (dt == gsn_backstab) || get_skill(victim,gsn_counter) < 1 )
	   return FALSE;

    chance = 10;
	
	if (KNOWS(victim,gsn_dual_wield))
	{
		if ( wield && wield2 && IS_WEAPON(wield) && IS_WEAPON(wield2) )
			chance += 6;
	}	
	
	if ( !can_see(victim,ch))
	{
	    //if (!check_blind_fighting(ch,victim,0))
		chance /= 4;
	}
	
	if (!can_see(ch,victim))
	    chance += 10;
			
	
	chance += (( victim->level - ch->level ) * 3 ) / 2;
	chance += (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
	//chance += get_weapon_skill(victim,get_weapon_sn(victim, FALSE)) -
	//				get_weapon_skill(ch,get_weapon_sn(ch, FALSE));
	chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR));

		    
    //chance += get_weapon_skill(ch, get_weapon_sn(victim, FALSE) ) / (wield ? 14 : 50);


    if ( number_percent() <= chance )
	{
		dam_type = attack_table[dt - TYPE_HIT].damage;

		dt = gsn_counter;

		if (dam_type == -1)
			dam_type = DAM_BASH;

		act( "You reverse $n's attack and counter with your own!", ch, NULL, victim, TO_VICT    );
		act( "$N counters your attack!", ch, NULL, victim, TO_CHAR    );

		damage(victim,ch,dam/4, gsn_counter , dam_type ,TRUE ); 
		/* DAM MSG NUMBER!! */

		check_improve(victim,gsn_counter,TRUE,1);
		return TRUE;
	}
	check_improve(victim,gsn_counter,FALSE,1);
	return FALSE;
}


bool check_riposte( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
	int chance;
	int dam_type;
	OBJ_DATA *wield, *wield2;

	if (IS_NPC(victim))
		return FALSE;

	wield = get_eq_char(victim, WEAR_WIELD);
	wield2 = get_eq_char(victim, WEAR_SECONDARY);
	
	if (( !wield && !wield2 ) || (!IS_AWAKE(victim)) || (dt == gsn_backstab) || !KNOWS(victim, gsn_riposte) )
	   return FALSE;

    chance = 10;	
		
	if (KNOWS(victim,gsn_dual_wield))
	{
		if ( wield && wield2 && IS_WEAPON(wield) && IS_WEAPON(wield2) )
			chance += 6;
	}	

	if ( !can_see(victim,ch))
	{
	    //if (!check_blind_fighting(ch,victim,0))
			chance /= 4;
	}
	
	if (!can_see(ch,victim))
	    chance += 10;
			
	
	chance += (( victim->level - ch->level ) * 3 ) / 2;
	chance += (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
	//chance += get_weapon_skill(victim,get_weapon_sn(victim, FALSE)) -
	//				get_weapon_skill(ch,get_weapon_sn(ch, FALSE));
	//chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

		    
    //chance += get_weapon_skill(ch, get_weapon_sn(victim, FALSE) ) / (wield ? 14 : 50);


    if ( number_percent( ) >= chance )
        return FALSE;

    dam_type = attack_table[dt - TYPE_HIT].damage;

    dt = gsn_riposte;

    if (dam_type == -1)
        dam_type = DAM_BASH;

    act( "You reverse $n's attack and riposte with your own!", ch, NULL, victim, TO_VICT    );
    act( "$N ripostes your attack!", ch, NULL, victim, TO_CHAR    );

    damage(victim,ch,dam/4, gsn_riposte , dam_type ,TRUE ); 
    /* DAM MSG NUMBER!! */

    check_improve(victim,gsn_riposte,TRUE,1);
    return TRUE;
}

void do_engage(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  /* Check for skill.  */
  if (   (get_skill(ch,gsn_engage) == 0 )
      || (    !IS_NPC(ch)
          &&  (ch->level < skill_table[gsn_engage].skill_level[ch->ch_class])))
    {
      SEND("You know nothing about that skill.\r\n",ch);  
      return;
    }

  /* Must be fighting.  */
  if (ch->fighting == NULL)
    {
      SEND("But you aren't fighting anyone!\r\n",ch);
      return;
    }

  one_argument( argument, arg );

  /* Check for argument.  */
  if (arg[0] == '\0')
    {
      SEND("Engage whom?\r\n",ch);
      return;
    }

  /* Check for victim.  */
  if ((victim = get_char_room(ch,NULL,arg)) == NULL)
    {
      SEND("They aren't here.\r\n",ch);
      return;
    }

  if (victim == ch)
    {
      SEND("Why would you do battle with yourself?\r\n",ch);
      return;
    }

  if (ch->fighting == victim)
    {
      SEND("You're already engaged in battle with them!\r\n",ch);
      return;
    }

  /* Check for safe.  */
  if (is_safe(ch, victim))
    return;

  /* Check for charm.  */
  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  /* This lets higher-level characters engage someone that isn't already fighting them.
     Quite powerful.  Raise level as needed.  */
  if ((victim->fighting != ch) && (ch->level > victim->level - 8))
    {
      SEND("But they're not fighting you!\r\n",ch);
      return;
    }

  /* Get chance of success, and allow max 95%.  */
  chance = get_skill(ch,gsn_engage);
  chance = UMIN(chance,95);

	if (number_percent() < chance)
    {
      /* It worked!  */
      stop_fighting(ch,FALSE);

      set_fighting(ch,victim);
      if (victim->fighting == NULL)
        set_fighting(victim,ch);

      check_improve(ch,gsn_engage,TRUE,2);
      act("$n has turned $s attacks toward you!",ch,NULL,victim,TO_VICT);
      act("You turn your attacks toward $N.",ch,NULL,victim,TO_CHAR);
      act("$n has turned $s attacks toward $N!",ch,NULL,victim,TO_NOTVICT);
	  return;
    }
	else
    {
      /* It failed!  */
      SEND("You couldn't get your attack in.\r\n",ch);
      check_improve(ch,gsn_engage,FALSE,2);
	  return;
    }
	return;
}

void do_stun_fist( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	char arg[MSL];

	one_argument (argument, arg);
	
	if (IS_NPC(ch))
		return;
	
	if (ch->cooldowns[gsn_stun_fist] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
		
    if (!IS_NPC(ch) && (!KNOWS(ch, gsn_stun_fist) || !CAN_USE_SKILL(ch, gsn_stun_fist)))
    {
        SEND ("You better leave the martial arts to monks.\r\n",ch);
        return;
    }    

	if (ch->ki < 15)
	{
		SEND("You must channel more energy first.\r\n",ch);
		return;
	}
	
	if (arg[0] == '\0')
    {
		victim = ch->fighting;
		if (victim == NULL)
		{
			SEND("But you aren't in combat!\r\n",ch);
			return;
		}
	}
    else if ((victim = get_char_room(ch,NULL, arg)) == NULL)
    {
		SEND("They aren't here.\r\n",ch);
		return;
    }
    
    if (is_safe(ch,victim))
		return;

	if (ch->move < 25)
	{
		SEND("You need to rest a bit first.\r\n",ch);
		return;
	}
	
    WAIT_STATE (ch, skill_table[gsn_stun_fist].beats);
    if (get_skill (ch, gsn_stun_fist) > number_percent ())
    {
        damage (ch, victim, dice (ch->level, 3), gsn_stun_fist, DAM_BASH, TRUE);
        check_improve (ch, gsn_stun_fist, TRUE, 2);
		
		if (IS_AFFECTED (victim, gsn_stun_fist) || saves_spell (ch->level, victim, DAM_OTHER))
			return;
        
		af.where = TO_AFFECTS;
		af.type = gsn_stun_fist;
		af.level = ch->level;
		af.bitvector = 0;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.duration = number_range(1,2);    
		affect_to_char (victim, &af);
		
		SEND ("Your world is spinning from the force of that blow!\r\n", victim);
		act ("$n's world starts spinning from the force of your blow!", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_STUNNED;
		ch->move -= 25;
		ch->ki -= 15;
    }
    else
    {
        //damage (ch, victim, 0, gsn_kick, DAM_BASH, TRUE);
        check_improve (ch, gsn_stun_fist, FALSE, 2);
    }
    check_killer (ch, victim);

	return;
}

void do_palm_strike( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
	int dam = 0, skill = 0, chance = 0;

    if ( !ch || IS_NPC( ch ) )
		return;
	
	if (get_eq_char (ch, WEAR_WIELD))
	{
		if (IS_WEAPON(get_eq_char(ch, WEAR_WIELD)) || IS_SHIELD(get_eq_char(ch, WEAR_WIELD)))
		{
			SEND("Your abilities falter while holding such items.\r\n",ch);
			ch->pcdata->fight_pos = 0;
			return;
		}
	}
	if (get_eq_char (ch, WEAR_SECONDARY))
	{
		if (IS_WEAPON(get_eq_char(ch, WEAR_SECONDARY)) || IS_SHIELD(get_eq_char(ch, WEAR_SECONDARY)))
		{
			SEND("Your abilities falter while holding such items.\r\n",ch);
			ch->pcdata->fight_pos = 0;
			return;
		}
	}
	
    if ( ( victim = ch->fighting ) == NULL )
    {
		SEND( "You aren't fighting anyone!\r\n",ch);
		ch->pcdata->fight_pos = FIGHT_NONE;
		return;
    }
	if (ch->cooldowns[gsn_palm_strike] > 0)
	{
		SEND("You must wait to use palm strike again.\r\n",ch);
		return;
	}
    if (ch->pcdata->fight_pos == FIGHT_NONE)
	{
		if ( ch->mana < 50 || ch->move < 50 )
		{
			SEND( "You cannot focus that much.\r\n",ch);
			return;
		}
		SEND( "You focus your mind and body, preparing to strike.\r\n",ch);
		act( "$n closes $s eyes, humming with a low tone...",ch,NULL,NULL,TO_ROOM );
			ch->mana -= 25;
			ch->move -= 25;
		ch->pcdata->fight_pos = FIGHT_PALM;
		
		if (!IS_IMMORTAL(ch))
			WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
		return;
    }
	skill = get_skill( ch, gsn_palm_strike );
    if ( ch->mana < 50 || ch->move < 50 || skill <= 1 )
    {
    	SEND( "Your concentration falters.\r\n",ch);
		ch->pcdata->fight_pos = FIGHT_NONE;
		ch->wait = 0;
    	return;
    }	
    ch->mana -= 25;
    ch->move -= 25;
    ch->pcdata->fight_pos = FIGHT_NONE;
    dam = dice( ch->level, number_range( (ch->level / 2) + 20, (ch->level * 2) + 5 ));
    
    skill += STAT_MOD(19,ch,STAT_CON,5);
    skill -= STAT_MOD(20,victim,STAT_CON,5);
    skill += ( ch->level - victim->level ) * 4;    

    if ( get_eq_char( ch, WEAR_WIELD) || get_eq_char( ch, WEAR_SECONDARY))
		skill /= 2;

    skill = URANGE( 1, skill, 150 );

    dam = ( dam * 100 ) / skill;

    if ( ch->wait == 1 )
		dam = ( 3 * dam ) / 4;

    else if ( ch->wait >= 2 )
		dam = ( 4 * dam ) / 5;

    chance = number_percent();

    if ( skill < chance )
    {
		SEND( "Your concentration falters, ending your preparation for a palm strike.\r\n",ch);
		act( "$n grumbles to $mself about concentration and focus.", ch,NULL,NULL,TO_ROOM );
		check_improve( ch, gsn_palm_strike, FALSE, 4 );
	return;
    }
    

    act( "You unleash your palm strike attack upon $N!",ch,NULL,victim,TO_CHAR );
    act( "You feel a shiver down your spine as $n strikes $N with a deadly blow.",ch,NULL,victim,TO_NOTVICT );
    act( "You clutch your chest as $n's palm strike resonates through you!",ch,NULL,victim,TO_VICT );

	ch->cooldowns[gsn_palm_strike] = skill_table[gsn_palm_strike].cooldown;
	
    if ( skill / 3 < chance )
    {
		DAZE_STATE( victim, PULSE_VIOLENCE );
		act( "$N looks dazed from the impact of your strike.",ch,NULL,victim,TO_CHAR );
		act( "$N looks dazed from $n's powerful strike.",ch,NULL,victim,TO_NOTVICT );
		act( "You feel your head start to spin.",ch,NULL,victim,TO_VICT );
    }

    damage( ch, victim, dam, gsn_palm_strike, DAM_ENERGY, TRUE );	
    //do_handaffects( ch, victim );

    check_improve( ch, gsn_palm_strike, TRUE, 2);
    return;
}


void do_gouge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance = 0;

    one_argument (argument, arg);

	if (IS_NPC(ch))
		return;
		
    // if (!IS_NPC (ch)
        // && (thief_levels(ch) < skill_table[gsn_gouge].skill_level[THIEF] || !KNOWS(ch, gsn_gouge)))
	if (!KNOWS(ch, gsn_gouge) || !CAN_USE_SKILL(ch, gsn_gouge))
    {
        SEND ("You don't know what you're doing.\r\n", ch);
        return;
    }

	if (ch->cooldowns[gsn_gouge] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            SEND ("But you aren't in combat!\r\n", ch);
            return;
        }
    }

    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    // if (IS_AFFECTED (victim, AFF_BLIND))
    // {
        // act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
        // return;
    // }

    if (victim == ch)
    {
        SEND ("You decide against gouging your own eyes out.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= 2 * get_curr_stat (victim, STAT_DEX);

    /* speed  */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
        chance += 1;    

    /* now the attack */
    if (number_percent () < chance)
    {
        AFFECT_DATA af;
		if (!IS_AFFECTED (victim, AFF_BLIND))
		{
			act ("{5$n tears up from you gouging $s eyes!{x", victim, NULL, NULL, TO_ROOM);
			act ("{5$n gouges your eyes!{x", ch, NULL, victim, TO_VICT);        
			SEND ("{5You can't see a thing!{x\r\n", victim);
		
			af.where = TO_AFFECTS;
			af.type = gsn_gouge;
			af.level = ch->level;
			af.duration = 0;
			af.location = APPLY_HITROLL;
			af.modifier = -4;
			af.bitvector = AFF_BLIND;

			affect_to_char (victim, &af);
		}
		
		check_improve (ch, gsn_gouge, TRUE, 2);
        WAIT_STATE (ch, skill_table[gsn_gouge].beats);
        
		damage (ch, victim, number_range (2, 5), gsn_gouge, DAM_NONE, FALSE);
    }
    else
    {
        damage (ch, victim, 0, gsn_gouge, DAM_NONE, TRUE);
        check_improve (ch, gsn_gouge, FALSE, 2);
        WAIT_STATE (ch, skill_table[gsn_gouge].beats);
    }
	COOLDOWN(ch, gsn_gouge);
	check_killer (ch, victim);
	return;
}

void do_kidney_punch( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MSL];

	one_argument (argument, arg);
	
	if (ch->cooldowns[gsn_kidney_punch] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	    
	if (!KNOWS(ch, gsn_kidney_punch) || !CAN_USE_SKILL(ch, gsn_kidney_punch))
    {
        SEND ("You better leave the martial arts to fighters and monks.\r\n", ch);
        return;
    }    

	if (arg[0] == '\0')
    {
		victim = ch->fighting;
		if (victim == NULL)
		{
			SEND("But you aren't in combat!\r\n",ch);
			return;
		}
	}
    else if ((victim = get_char_room(ch,NULL, arg)) == NULL)
    {
		SEND("They aren't here.\r\n",ch);
		return;
    }
    
    if (is_safe(ch,victim))
		return;

    WAIT_STATE (ch, skill_table[gsn_kidney_punch].beats);
    if (get_skill (ch, gsn_kidney_punch) > number_percent ())
    {
        damage (ch, victim, number_range (ch->level / 2, ch->level), gsn_kidney_punch, DAM_BASH,
                TRUE);
        check_improve (ch, gsn_kidney_punch, TRUE, 1);
    }
    else
    {
        damage (ch, victim, 0, gsn_kidney_punch, DAM_BASH, TRUE);
        check_improve (ch, gsn_kidney_punch, FALSE, 1);
    }
    check_killer (ch, victim);
	COOLDOWN(ch, gsn_kidney_punch);
    return;
}

void do_earclap( CHAR_DATA *ch, char *argument )
{

	char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if (!KNOWS(ch,gsn_earclap) || !CAN_USE_SKILL(ch, gsn_earclap))      
    {
        SEND ("You aren't sure how to pull off an earclap.\r\n", ch);
        return;
    }

	chance = (get_skill(ch, gsn_sharpen));
	
    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            SEND ("But you aren't fighting anyone!\r\n", ch);
            return;
        }
    }

    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act ("You'll have to let $M get back up first.", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        SEND ("You try to bash your brains out, but fail.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        SEND ("Kill stealing is not permitted.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (what_size(ch) < what_size(victim))
        chance += (what_size(ch) - what_size(victim)) * 15;
    else
        chance += (what_size(ch) - what_size(victim)) * 10;

		
    /* stats */
    chance += GET_STR(ch);
    chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC (victim, AC_BASH) / 25;
    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
    {                            
	   act("{5$n tries to deliver an earclap, but you manage to dodge it.{x",ch,NULL,victim,TO_VICT);
	   act("{5$N dodges your earclap, you end up in a prone position!{x",ch,NULL,victim,TO_CHAR);
	   WAIT_STATE(ch,skill_table[gsn_earclap].beats);
	   return; 
        //chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent () < chance)
    {

        act ("{5$n rings your ears, dazing you!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5You clap $N's ears, dazing $M!{x", ch, NULL, victim,
             TO_CHAR);
        act ("{5$n dazes $N with a powerful earclap!{x", ch, NULL,
             victim, TO_NOTVICT);
        check_improve (ch, gsn_earclap, TRUE, 1);

        DAZE_STATE (victim, 3 * PULSE_VIOLENCE);
        WAIT_STATE (ch, skill_table[gsn_earclap].beats);
        victim->position = POS_RESTING;
        damage (ch, victim, number_range (2, 2 + 2 * what_size(ch) + chance / 20),
                gsn_earclap, DAM_BASH, FALSE);

    }
    else
    {
        damage (ch, victim, 0, gsn_earclap, DAM_BASH, FALSE);
        act ("{5You miss, making yourself prone!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n misses their earclap, becoming prone!{x", ch, NULL, victim, TO_NOTVICT);
        act ("{5You evade $n's earclap, causing $m to become prone!{x",
             ch, NULL, victim, TO_VICT);
        check_improve (ch, gsn_earclap, FALSE, 1);
        ch->position = POS_RESTING;
        WAIT_STATE (ch, skill_table[gsn_earclap].beats * 3 / 2);
    }
    //check_killer (ch, victim);
	return;	
}