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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/


#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
#define 	CD CHAR_DATA
#define 	OD OBJ_DATA
bool remove_obj 	args ((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj 		args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
CD *find_keeper 	args ((CHAR_DATA * ch));
int get_cost 		args ((CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy));
void obj_to_keeper 	args ((OBJ_DATA * obj, CHAR_DATA * ch));
OD *get_obj_keeper 	args ((CHAR_DATA * ch, CHAR_DATA * keeper, char *argument));
void spell_null 	args((int sn, int level, CHAR_DATA * ch, void *vo, int target));
void raw_kill args ((CHAR_DATA * victim));

void obj_to_vault args ((OBJ_DATA *obj, ACCOUNT_DATA * acc));
void show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing, bool eq));

int total_levels 			args((CHAR_DATA *ch));


#undef 		OD
#undef    	CD

int find_door args ((CHAR_DATA * ch, char *arg));



/* RT part of the corpse looting code */

bool can_loot (CHAR_DATA * ch, OBJ_DATA * obj)
{
    //CHAR_DATA *owner;//, *wch;

    if (IS_IMMORTAL (ch))
        return TRUE;

    if (!obj->owner || obj->owner == NULL)
        return TRUE;

    // owner = NULL;
    // for (wch = char_list; wch != NULL; wch = wch->next)
        // if (!str_cmp (wch->name, obj->owner))
            // owner = wch;

    // if (owner == NULL)
        // return TRUE;

	//If it's your corpse then sure.
    if (!str_cmp (ch->name, obj->owner))
        return TRUE;
	
	//Only clanners can loot others
	if (!ch->clan)
		return FALSE;
		
	//Pets/charms can get things from their corpse, but not someone else's.
	if (IS_NPC(ch))
		return FALSE;
		
	//Gems and money lootable.
	if (obj->item_type == ITEM_GEM || obj->item_type == ITEM_MONEY)
		return TRUE;
		
	//All unique items can be looted.
	if (IS_UNIQUE(obj))
		return TRUE;
		
    //if (!IS_NPC (owner) && IS_SET (owner->act, PLR_CANLOOT))
    //   return TRUE;

    // if (is_same_group (ch, owner))
        // return TRUE;

    return FALSE;
}

int get_bulk (CHAR_DATA *ch, bool spells)
{
	int i = 0;
	int bulk = 0;
	OBJ_DATA *obj;
		
	for ( i = 0; i < MAX_WEAR; i++ )
	{
		if ( ( obj = get_eq_char( ch, i ) ) == NULL )
			continue;
		
		if (obj && obj->item_type == ITEM_ARMOR)
		{
			bulk += obj->value[4];
		}		
	}	
	if (spells)
	{
		if (is_affected(ch, gsn_stone_skin))
		{
			bulk += total_levels(ch) / 5;
		}
		
		if (is_affected(ch, gsn_barkskin))
		{
			bulk += total_levels(ch) / 6;
		}
	}
	
	return bulk;
}


int get_trap_type(OBJ_DATA *obj)
{
	int type = 0;
	
	if (IS_SET(obj->extra_flags, ITEM_FIRE_TRAP))
		type = FIRE_TRAP;
	if (IS_SET(obj->extra_flags, ITEM_POISON_TRAP))
		type = POISON_TRAP;
	if (IS_SET(obj->extra_flags, ITEM_GAS_TRAP))
		type = GAS_TRAP;
	if (IS_SET(obj->extra_flags, ITEM_DART_TRAP))
		type = DART_TRAP;
	if (IS_SET(obj->extra2_flags, ITEM_SHOCK_TRAP))
		type = SHOCK_TRAP;
	
	return type;
}

void lay_trap(OBJ_DATA *obj, int type)
{		
	switch (type)
	{
		default:
			return;
		case FIRE_TRAP:
			SET_BIT (obj->extra_flags, ITEM_FIRE_TRAP);
			break;
		case POISON_TRAP:
			SET_BIT (obj->extra_flags, ITEM_POISON_TRAP);
			break;
		case GAS_TRAP:
			SET_BIT (obj->extra_flags, ITEM_GAS_TRAP);
			break;
		case DART_TRAP:
			SET_BIT (obj->extra_flags, ITEM_DART_TRAP);
			break;
		case SHOCK_TRAP:
			SET_BIT (obj->extra2_flags, ITEM_SHOCK_TRAP);
			break;
	}
	return;
}

void remove_trap(OBJ_DATA *obj)
{
	int type;
	
	type = get_trap_type(obj);
	switch (type)
	{
		default:
			return;
		case FIRE_TRAP:
			REMOVE_BIT (obj->extra_flags, ITEM_FIRE_TRAP);
			break;
		case POISON_TRAP:
			REMOVE_BIT (obj->extra_flags, ITEM_POISON_TRAP);
			break;
		case GAS_TRAP:
			REMOVE_BIT (obj->extra_flags, ITEM_GAS_TRAP);
			break;
		case DART_TRAP:
			REMOVE_BIT (obj->extra_flags, ITEM_DART_TRAP);
			break;
		case SHOCK_TRAP:
			REMOVE_BIT (obj->extra2_flags, ITEM_SHOCK_TRAP);
			break;
		}
		return;
}

bool save_vs_trap(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int trap_type = 0;
	int reaction = 0, difficulty = 15;
	
	trap_type = get_trap_type(obj);
	
	switch (trap_type)	
	{	
		case FIRE_TRAP:
			difficulty += 5;
			break;
		case POISON_TRAP:
			difficulty += 3;
			break;
		case GAS_TRAP:
			difficulty += 8;
			break;
		case DART_TRAP:
			difficulty += 2;
			break;
		case SHOCK_TRAP:
			difficulty += 6;
			break;
		default:
			difficulty = 15;
			break;
	}
	
	difficulty += number_range(1,4); //add a little randomness to it.
	reaction = number_range(1,20);   //roll a reaction.
	
	
	if (trap_type == POISON_TRAP || trap_type == DART_TRAP)
	{
		if (get_curr_stat(ch, STAT_DEX) < 15)
			reaction -= ((get_curr_stat(ch, STAT_DEX) - 25) / 2); //max neg. of 12 w/ a Dex of 1.
		if (get_curr_stat(ch, STAT_DEX) > 15)
			reaction += ((get_curr_stat(ch, STAT_DEX)) / 5); //max bonus of 5 at Dex 25.
	}	
	
	if (trap_type == FIRE_TRAP || trap_type == SHOCK_TRAP)
	{
		if (get_curr_stat(ch, STAT_INT) > 15)
			reaction += ((get_curr_stat(ch, STAT_INT)) / 5); //max bonus of 5 at Int 25.
	}
	
	if (reaction >= difficulty)
		return TRUE;
	else
		return FALSE;
}


bool is_trapped(OBJ_DATA *obj)
{
	if (IS_SET(obj->extra_flags, ITEM_FIRE_TRAP) || IS_SET(obj->extra_flags, ITEM_POISON_TRAP) || IS_SET(obj->extra_flags, ITEM_GAS_TRAP) || IS_SET(obj->extra_flags, ITEM_DART_TRAP) || IS_SET(obj->extra2_flags, ITEM_SHOCK_TRAP))
		return TRUE;
	else
		return FALSE;
}


void spring_trap(CHAR_DATA *victim, OBJ_DATA *obj)
{
	int trap_type = 0;
	int dam = 0;
	char buf[MSL];
	AFFECT_DATA af;
	
	trap_type = get_trap_type(obj);

	switch (trap_type)
    {
        default:
			SEND ("You got lucky... the trap failed to spring.\r\n", victim);
            return;
		case FIRE_TRAP:
			dam = dice(6,8) + 5;
			act ("A gout of flames shoots forth, searing $n!", victim, NULL, NULL, TO_ROOM);
			sprintf( buf, "A gout of flames shoots forth, searing you! {r[{x%d{r]{x\r\n", dam );			
			remove_trap(obj);
			SEND(buf, victim);
			victim->hit -= dam;			
			if (victim->hit <= 0)
				raw_kill(victim);						
			return;
			
		case SHOCK_TRAP:
			dam = dice(5,8) + 5;
			act ("$n is electrocuted by $p!", victim, obj, NULL, TO_ROOM);
			sprintf( buf, "You are electrocuted by %s! {r[{x%d{r]{x\r\n", obj->short_descr, dam );			
			remove_trap(obj);
			SEND(buf, victim);			
			victim->hit -= dam;
			if (victim->hit <= 0)
				raw_kill(victim);
			return;
		case POISON_TRAP:
			dam = dice(4,8) + 5;
			act ("A poison dart springs forth, striking $n!", victim, NULL, NULL, TO_ROOM);			
			sprintf( buf, "A poison dart springs forth, striking you! {r[{x%d{r]{x\r\n", dam );			
			remove_trap(obj);
			SEND(buf, victim);
			af.where = TO_AFFECTS;
			af.type = gsn_poison;
			af.level = number_range(25,40);
			af.duration = number_range(25,40);	
			af.location = APPLY_STR;
			af.modifier = -2;
			af.bitvector = AFF_POISON;
			affect_join (victim, &af);
			SEND ("You feel very sick.\r\n", victim);
			act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
			victim->hit -= dam;
			if (victim->hit <= 0)
				raw_kill(victim);
			return;
		case GAS_TRAP:
			dam = dice(3,8) + 5;
			act ("A cloud of gas seeps out, choking $n!", victim, NULL, NULL, TO_ROOM);
			sprintf( buf, "A cloud of gas seeps out, choking you! {r[{x%d{r]{x\r\n", dam );		
			remove_trap(obj);			
			SEND(buf, victim);
			victim->hit -= dam;
			if (victim->hit <= 0)
				raw_kill(victim);
			return;
		case DART_TRAP:
			dam = dice(3,8) + 5;
			act ("A spring loaded dart leaps out, stabbing $n!", victim, NULL, NULL, TO_ROOM);
			sprintf( buf, "A spring loaded dart leaps out, stabbing you! {r[{x%d{r]{x\r\n", dam );	
			remove_trap(obj);
			SEND(buf, victim);
			victim->hit -= dam;
			if (victim->hit <= 0)
				raw_kill(victim);
			return;
	}
	return;
}

void caught_thief(CHAR_DATA * ch, CHAR_DATA * victim)
{
	char buf[MAX_STRING_LENGTH];

	SEND ("Oops.\r\n", ch);
	affect_strip (ch, gsn_sneak);
	REMOVE_BIT (ch->affected_by, AFF_SNEAK);

	act ("$n tried to steal from you.\r\n", ch, NULL, victim, TO_VICT);
	act ("$n tried to steal from $N.\r\n", ch, NULL, victim, TO_NOTVICT);
	switch (number_range (0, 3))
	{
		case 0:
			sprintf (buf, "%s is a lousy thief!", ch->name);
			break;
		case 1:
			sprintf (buf, "%s couldn't rob %s way out of a paper bag!",
					 ch->name, (ch->sex == 2) ? "her" : "his");
			break;
		case 2:
			sprintf (buf, "%s tried to rob me!", ch->name);
			break;
		case 3:
			sprintf (buf, "Keep your hands out of there, %s!", ch->name);
			break;
	}
	if (!IS_AWAKE (victim))
		do_function (victim, &do_wake, "");
	if (IS_AWAKE (victim))
		do_function (victim, &do_yell, buf);
	if (!IS_NPC (ch))
	{
		if (IS_NPC (victim))
		{
			check_improve (ch, gsn_steal, FALSE, 2);
			multi_hit (victim, ch, TYPE_UNDEFINED);
		}
		else
		{
			sprintf (buf, "$N tried to steal from %s.", victim->name);
			wiznet (buf, ch, NULL, WIZ_FLAGS, 0, 0);
			if (!IS_SET (ch->act, PLR_THIEF))
			{
				SET_BIT (ch->act, PLR_THIEF);
				SEND ("*** You are now a THIEF!! ***\r\n", ch);
				save_char_obj (ch);
			}
		}
	}
}

void get_obj (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];
	//char buf[MSL];

    if ((!CAN_WEAR (obj, ITEM_TAKE) && !IS_IMMORTAL(ch)) || obj->item_type == ITEM_TRACKS || obj->item_type == ITEM_BLOOD_POOL)
    {
        SEND ("You can't take that.\r\n", ch);
        return;
    }	
	
    if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
        act ("$d: you can't carry that many items.",
             ch, NULL, obj->name, TO_CHAR);
        return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
        && (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
    {
        act ("$d: you can't carry that much weight.",
             ch, NULL, obj->name, TO_CHAR);
        return;
    }

    /*if (!can_loot (ch, obj))
    {
        act ("Corpse looting is not permitted.", ch, NULL, NULL, TO_CHAR);
        return;
    }*/
	

    if (obj->in_room != NULL)
    {
        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
            if (gch->on == obj)
            {
                act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
                return;
            }
    }


    if (container != NULL)
    {
        /*if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && get_trust (ch) < obj->level)
        {
            SEND ("You are not powerful enough to use it.\r\n", ch);
            return;
        }
        if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && !CAN_WEAR (container, ITEM_TAKE)
            && !IS_OBJ_STAT (obj, ITEM_HAD_TIMER))
				obj->timer = 0;
		*/
		if ( IS_CLAN_PIT( container ) && !IS_IMMORTAL ( ch ) )
        {
			switch (container->pIndexData->vnum)			
			{
				default:
					SEND("That's not a valid clan pit.\r\n",ch);
					return;

				case CLAN_GUARDIAN_PIT:
					if (ch->clan != CLAN_GUARDIAN)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_CONCLAVE_PIT:
					if (ch->clan != CLAN_CONCLAVE)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_ACOLYTE_PIT:
					if (ch->clan != CLAN_ACOLYTE)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_RAVAGER_PIT:
					if (ch->clan != CLAN_RAVAGER)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_SONG_PIT:
					if (ch->clan != CLAN_SONG)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_CORSAIR_PIT:
					if (ch->clan != CLAN_CORSAIR)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_OBSIDIAN_PIT:
					if (ch->clan != CLAN_OBSIDIAN)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_JAEZRED_PIT:
					if (ch->clan != CLAN_JAEZRED)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_DAERTHE_PIT:
					if (ch->clan != CLAN_BREGAN_DAERTHE)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;				
			}
		}
			
			
		if (container->item_type == ITEM_CORPSE_PC && str_cmp(container->owner, ch->name))
		{
			act ("Corpse looting is not permitted.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		act ("You get $p from $P.", ch, obj, container, TO_CHAR);
		act ("$n gets $p from $P.", ch, obj, container, TO_ROOM);
		if (is_trapped(obj))
			spring_trap(ch,obj);
		//REMOVE_BIT (obj->extra_flags, ITEM_HAD_TIMER);
		REMOVE_BIT (obj->extra2_flags, ITEM_HIDDEN);
		//Containers that don't count toward your total item carry count.
		if (IS_SET (container->value[1], CONT_NOCOUNT))			
			ch->carry_number++;
		
		obj_from_obj (obj);	
    }
    else
    {
        act ("You get $p.", ch, obj, container, TO_CHAR);
        act ("$n gets $p.", ch, obj, container, TO_ROOM);
		if (is_trapped(obj))
			spring_trap(ch,obj);
		REMOVE_BIT (obj->extra2_flags, ITEM_HIDDEN);
		obj_from_room (obj);		
    }

    if (obj->item_type == ITEM_MONEY)
    {
        ch->silver += obj->value[0];
        ch->gold += obj->value[1];
        if (IS_SET (ch->act, PLR_AUTOSPLIT))
        {                        /* AUTOSPLIT code */
            members = 0;
            for (gch = ch->in_room->people; gch != NULL;
                 gch = gch->next_in_room)
            {
                if (!IS_AFFECTED (gch, AFF_CHARM) && is_same_group (gch, ch))
                    members++;
            }

            if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
            {
                sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
                do_function (ch, &do_split, buffer);
            }
        }

        extract_obj (obj);
    }
    else
    {
		obj_to_char (obj, ch);			
		//Quest updating.
		QUEST_DATA *quest;
		char buf2[MSL];
		for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
		{
			if (quest->pIndexData && IS_SET(quest->pIndexData->type, B)) //item_get
			{
				if (obj->pIndexData->vnum == quest->pIndexData->target_obj_vnum)
				{
					quest->obj_multiples++;
					if (!quest->obj_multiples > quest->pIndexData->obj_mult)
					{
						if (quest->pIndexData->obj_mult < 2)
						{
							sprintf(buf2, "You have now collected the item for {g%s{x!\r\n",quest->name);
							SEND(buf2,ch);
						}
						else
						{
							sprintf(buf2, "You have now collected %d out of %d items for {g%s{x!\r\n",quest->obj_multiples, quest->pIndexData->obj_mult, quest->name);					
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
		}
		//No more buried objects in their inventory
		obj->altitude = 0;
		
		//For glyphs and explosive runes. all other traps are containers only.		
		if ( IS_SET(obj->extra2_flags, ITEM_FIRE_TRAP) && obj->item_type != ITEM_CONTAINER )			
			spring_trap(ch,obj);
		
		if ( HAS_TRIGGER_OBJ( obj, TRIG_GET ) )
			p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GET );
		if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GET ) )
			p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GET );
    }

    return;
}



void do_get (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (!str_cmp (arg2, "from"))
        argument = one_argument (argument, arg2);

    /* Get type. */
    if (arg1[0] == '\0')
    {
        SEND ("Get what?\r\n", ch);
        return;
    }

    if (arg2[0] == '\0')
    {
        if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
        {
            /* 'get obj' */
            obj = get_obj_list (ch, arg1, ch->in_room->contents);
            if (obj == NULL)
            {
                act ("I see no $T here.", ch, NULL, arg1, TO_CHAR);
                return;
            }

            get_obj (ch, obj, NULL);
        }
        else
        {
            /* 'get all' or 'get all.obj' */
            found = FALSE;
            for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
                    && can_see_obj (ch, obj))
                {
                    found = TRUE;
                    get_obj (ch, obj, NULL);
                }
            }

            if (!found)
            {
                if (arg1[3] == '\0')
                    SEND ("I see nothing here.\r\n", ch);
                else
                    act ("I see no $T here.", ch, NULL, &arg1[4], TO_CHAR);
            }
        }
    }
    else
    {
        /* 'get ... container' */
        if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
        {
            SEND ("You can't do that.\r\n", ch);
            return;
        }

        if ((container = get_obj_here( ch, NULL, arg2)) == NULL)
        {
            act ("I see no $T here.", ch, NULL, arg2, TO_CHAR);
            return;
        }

        switch (container->item_type)
        {
            default:
                SEND ("That's not a container.\r\n", ch);
                return;

            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
                break;

            case ITEM_CORPSE_PC:
                {

                    if (!can_loot (ch, container))
                    {
                        SEND ("You can't do that.\r\n", ch);
                        return;
                    }
                }
        }

		if ( IS_CLAN_PIT( container ) && !IS_IMMORTAL ( ch ) )
        {
			switch (container->pIndexData->vnum)			
			{
				default:
					SEND("That's not a valid clan pit.\r\n",ch);
					return;

				case CLAN_GUARDIAN_PIT:
					if (ch->clan != CLAN_GUARDIAN)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_CONCLAVE_PIT:
					if (ch->clan != CLAN_CONCLAVE)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_ACOLYTE_PIT:
					if (ch->clan != CLAN_ACOLYTE)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_RAVAGER_PIT:
					if (ch->clan != CLAN_RAVAGER)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_SONG_PIT:
					if (ch->clan != CLAN_SONG)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_CORSAIR_PIT:
					if (ch->clan != CLAN_CORSAIR)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_OBSIDIAN_PIT:
					if (ch->clan != CLAN_OBSIDIAN)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_JAEZRED_PIT:
					if (ch->clan != CLAN_JAEZRED)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;
				case CLAN_DAERTHE_PIT:
					if (ch->clan != CLAN_BREGAN_DAERTHE)
					{
						SEND("You are not in the correct clan to do so.\r\n",ch);
						return;
					}
					break;				
			}
		}

        if (IS_SET (container->value[1], CONT_CLOSED))
        {
            act ("The $	d is closed.", ch, NULL, container->name, TO_CHAR);
            return;
        }

        if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
        {
            /* 'get obj container' */
			
			//Make sure it's not a clan vault. Upro			
            obj = get_obj_list (ch, arg1, container->contains);
            if (obj == NULL)
            {
                act ("I see nothing like that in the $T.",ch, NULL, arg2, TO_CHAR);
                return;
            }
            get_obj (ch, obj, container);
        }
        else
        {
			if (container->pIndexData->vnum > 49 && container->pIndexData->vnum < 59)
			{
				SEND("You cannot 'get all' from clan vaults.\r\n",ch);
				return;
			}
            /* 'get all container' or 'get all.obj container' */
            found = FALSE;
            for (obj = container->contains; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
                    && can_see_obj (ch, obj))
                {
                    found = TRUE;
                    /*
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
                        && !IS_IMMORTAL (ch))
                    {
                        SEND ("Don't be so greedy!\r\n", ch);
                        return;
                    }
		    */
                    get_obj (ch, obj, container);
                }
            }

            if (!found)
            {
                if (arg1[3] == '\0')
                    act ("I see nothing in the $T.", ch, NULL, arg2, TO_CHAR);
                else
                    act ("I see nothing like that in the $T.",
                         ch, NULL, arg2, TO_CHAR);
            }
        }
    }

    return;
}



void do_put (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (!str_cmp (arg2, "in") || !str_cmp (arg2, "on"))
        argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        SEND ("Put what in what?\r\n", ch);
        return;
    }

    if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
    {
        SEND ("You can't do that.\r\n", ch);
        return;
    }

    if ((container = get_obj_here( ch, NULL, arg2)) == NULL)
    {
        act ("I see no $T here.", ch, NULL, arg2, TO_CHAR);
        return;
    }

    if (container->item_type != ITEM_CONTAINER)
    {
        SEND ("That's not a container.\r\n", ch);
        return;
    }

    if (IS_SET (container->value[1], CONT_CLOSED))
    {
        act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
        return;
    }

    if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
    {
        /* 'put obj container' */
        if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
        {
            SEND ("You do not have that item.\r\n", ch);
            return;
        }

        if (obj == container)
        {
            SEND ("You can't fold it into itself.\r\n", ch);
            return;
        }

        if (!can_drop_obj (ch, obj))
        {
            SEND ("You can't let go of it.\r\n", ch);
            return;
        }

        if (WEIGHT_MULT (obj) != 100)
        {
            SEND ("You have a feeling that would be a bad idea.\r\n",
                          ch);
            return;
        }

        if (get_obj_weight (obj) + get_true_weight (container)
            > (container->value[0] * 10)
            || get_obj_weight (obj) > (container->value[3] * 10))
        {
            SEND ("It won't fit.\r\n", ch);
            return;
        }

        /*
	if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && !CAN_WEAR (container, ITEM_TAKE))
        {
            if (obj->timer)
                SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
            else
                obj->timer = number_range (100, 200);
        }
	*/
        obj_from_char (obj);
        obj_to_obj (obj, container);
		
		//Containers that don't count toward your total item carry count.
		if (IS_SET (container->value[1], CONT_NOCOUNT))
			ch->carry_number--;

        if (IS_SET (container->value[1], CONT_PUT_ON))
        {
            act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
            act ("You put $p on $P.", ch, obj, container, TO_CHAR);
        }
        else
        {
            act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
            act ("You put $p in $P.", ch, obj, container, TO_CHAR);
        }
    }
    else
    {
        /* 'put all container' or 'put all.obj container' */
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
                && can_see_obj (ch, obj)
                && WEIGHT_MULT (obj) == 100
                && obj->wear_loc == WEAR_NONE
                && obj != container && can_drop_obj (ch, obj)
                && get_obj_weight (obj) + get_true_weight (container)
                <= (container->value[0] * 10)
                && get_obj_weight (obj) < (container->value[3] * 10))
            {
		/*
                if (container->pIndexData->vnum == OBJ_VNUM_PIT
                    && !CAN_WEAR (obj, ITEM_TAKE))
                {
                    if (obj->timer)
                        SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
                    else
                        obj->timer = number_range (100, 200);
                }
		*/
                obj_from_char (obj);
                obj_to_obj (obj, container);

				//Containers that don't count toward your total item carry count.
				if (IS_SET (container->value[1], CONT_NOCOUNT))
					ch->carry_number--;

                if (IS_SET (container->value[1], CONT_PUT_ON))
                {
                    act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
                    act ("You put $p on $P.", ch, obj, container, TO_CHAR);
                }
                else
                {
                    act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
                    act ("You put $p in $P.", ch, obj, container, TO_CHAR);
                }
            }
        }
    }

    return;
}



void do_drop (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Drop what?\r\n", ch);
        return;
    }

    if (is_number (arg))
    {
        /* 'drop NNNN coins' */
        int amount, gold = 0, silver = 0;

        amount = atoi (arg);
        argument = one_argument (argument, arg);
        if (amount <= 0
            || (str_cmp (arg, "coins") && str_cmp (arg, "coin") &&
                str_cmp (arg, "gold") && str_cmp (arg, "silver")))
        {
            SEND ("Sorry, you can't do that.\r\n", ch);
            return;
        }

        if (!str_cmp (arg, "coins") || !str_cmp (arg, "coin")
            || !str_cmp (arg, "silver"))
        {
            if (ch->silver < amount)
            {
                SEND ("You don't have that much silver.\r\n", ch);
                return;
            }

            ch->silver -= amount;
            silver = amount;
        }

        else
        {
            if (ch->gold < amount)
            {
                SEND ("You don't have that much gold.\r\n", ch);
                return;
            }

            ch->gold -= amount;
            gold = amount;
        }

        for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            switch (obj->pIndexData->vnum)
            {
                case OBJ_VNUM_SILVER_ONE:
                    silver += 1;
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_GOLD_ONE:
                    gold += 1;
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_SILVER_SOME:
                    silver += obj->value[0];
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_GOLD_SOME:
                    gold += obj->value[1];
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_COINS:
                    silver += obj->value[0];
                    gold += obj->value[1];
                    extract_obj (obj);
                    break;
            }
        }

        obj_to_room (create_money (gold, silver), ch->in_room);
        act ("$n drops some coins.", ch, NULL, NULL, TO_ROOM);
        SEND ("OK.\r\n", ch);
        return;
    }

    if (str_cmp (arg, "all") && str_prefix ("all.", arg))
    {
        /* 'drop obj' */
        if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
        {
            SEND ("You do not have that item.\r\n", ch);
            return;
        }

        if (!can_drop_obj (ch, obj))
        {
            SEND ("You can't let go of it.\r\n", ch);
            return;
        }

        obj_from_char (obj);
        obj_to_room (obj, ch->in_room);
        act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
        act ("You drop $p.", ch, obj, NULL, TO_CHAR);
		
		if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
			p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
		if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
			p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );
			
        if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
        {
            act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
            act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
            extract_obj (obj);
        }
		if (obj->item_type == ITEM_POTION || !strcmp(obj->material, "glass"))
		{
			obj->condition -= number_range (25,100);
			if (obj->condition < 1 || number_percent() < 20)
			{
				act ("$p shatters as it hits the ground!", ch, obj, NULL, TO_ROOM);
				act ("$p shatters as it hits the ground!", ch, obj, NULL, TO_CHAR);
				extract_obj(obj);
			}
		}
		if (!strcmp(obj->material, "clay"))
		{
			obj->condition -= number_range (15,100);
			if (obj->condition < 1 || number_percent() < 15)
			{
				act ("$p breaks into pieces as it hits the ground!", ch, obj, NULL, TO_ROOM);
				act ("$p breaks into pieces as it hits the ground!", ch, obj, NULL, TO_CHAR);
				extract_obj(obj);
			}
		}
    }
    else
    {
        /* 'drop all' or 'drop all.obj' */
        found = FALSE;
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            if ((arg[3] == '\0' || is_name (&arg[4], obj->name))
                && can_see_obj (ch, obj)
                && obj->wear_loc == WEAR_NONE && can_drop_obj (ch, obj))
            {
                found = TRUE;
                obj_from_char (obj);
                obj_to_room (obj, ch->in_room);
                act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
                act ("You drop $p.", ch, obj, NULL, TO_CHAR);
        		
				if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
					p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
				if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
					p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );
				
				if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
                {
                    act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
                    act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
                    extract_obj (obj);
                }
				if (obj->item_type == ITEM_POTION || !strcmp(obj->material,"glass"))
				{
					obj->condition -= number_range (25,100);
					if (obj->condition < 1 || number_percent() < 20)
					{
						act ("$p shatters as it hits the ground!", ch, obj, NULL, TO_ROOM);
						act ("$p shatters as it hits the ground!", ch, obj, NULL, TO_CHAR);
						extract_obj(obj);
					}
				}
				if (!strcmp(obj->material,"clay"))
				{
					obj->condition -= number_range (15,100);
					if (obj->condition < 1 || number_percent() < 15)
					{
						act ("$p breaks into pieces as it hits the ground!", ch, obj, NULL, TO_ROOM);
						act ("$p breaks into pieces as it hits the ground!", ch, obj, NULL, TO_CHAR);
						extract_obj(obj);
					}
				}
            }
        }

        if (!found)
        {
            if (arg[3] == '\0')
                act ("You are not carrying anything.",
                     ch, NULL, arg, TO_CHAR);
            else
                act ("You are not carrying any $T.",
                     ch, NULL, &arg[4], TO_CHAR);
        }
    }

    return;
}



void do_give (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        SEND ("Give what to whom?\r\n", ch);
        return;
    }

    if (is_number (arg1))
    {
        /* 'give NNNN coins victim' */
        int amount;
        bool silver;

        amount = atoi (arg1);
        if (amount <= 0
            || (str_cmp (arg2, "coins") && str_cmp (arg2, "coin") &&
                str_cmp (arg2, "gold") && str_cmp (arg2, "silver")))
        {
            SEND ("Sorry, you can't do that.\r\n", ch);
            return;
        }

        silver = str_cmp (arg2, "gold");

        argument = one_argument (argument, arg2);
        if (arg2[0] == '\0')
        {
            SEND ("Give what to whom?\r\n", ch);
            return;
        }

        if ((victim = get_char_room ( ch, NULL, arg2)) == NULL)
        {
            SEND ("They aren't here.\r\n", ch);
            return;
        }

        if ((!silver && ch->gold < amount) || (silver && ch->silver < amount))
        {
            SEND ("You haven't got that much.\r\n", ch);
            return;
        }

        if (silver)
        {
            ch->silver -= amount;
            victim->silver += amount;
        }
        else
        {
            ch->gold -= amount;
            victim->gold += amount;
        }

        sprintf (buf, "$n gives you %d %s.", amount,
                 silver ? "silver" : "gold");
        act (buf, ch, NULL, victim, TO_VICT);
        act ("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
        sprintf (buf, "You give $N %d %s.", amount,
                 silver ? "silver" : "gold");
        act (buf, ch, NULL, victim, TO_CHAR);

        /*
         * Bribe trigger
         */
        if (IS_NPC (victim) && HAS_TRIGGER_MOB(victim, TRIG_BRIBE))
            p_bribe_trigger (victim, ch, silver ? amount : amount * 100);

        if (IS_NPC (victim) && IS_SET (victim->act, ACT_IS_CHANGER))
        {
            int change;

            change = (silver ? 95 * amount / 100 / 100 : 95 * amount);


            if (!silver && change > victim->silver)
                victim->silver += change;

            if (silver && change > victim->gold)
                victim->gold += change;

            if (change < 1 && can_see (victim, ch))
            {
                act
                    ("$n tells you 'I'm sorry, you did not give me enough to change.'",
                     victim, NULL, ch, TO_VICT);
                ch->reply = victim;
                sprintf (buf, "%d %s %s",
                         amount, silver ? "silver" : "gold", ch->name);
                do_function (victim, &do_give, buf);
            }
            else if (can_see (victim, ch))
            {
                sprintf (buf, "%d %s %s",
                         change, silver ? "gold" : "silver", ch->name);
                do_function (victim, &do_give, buf);
                if (silver)
                {
                    sprintf (buf, "%d silver %s",
                             (95 * amount / 100 - change * 100), ch->name);
                    do_function (victim, &do_give, buf);
                }
                act ("$n tells you 'Thank you, come again.'",
                     victim, NULL, ch, TO_VICT);
                ch->reply = victim;
            }
        }
        return;
    }

    if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        SEND ("You do not have that item.\r\n", ch);
        return;
    }

    if (obj->wear_loc != WEAR_NONE)
    {
        SEND ("You must remove it first.\r\n", ch);
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg2)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
    {
        act ("$N tells you 'Sorry, you'll have to sell that.'",
             ch, NULL, victim, TO_CHAR);
        ch->reply = victim;
        return;
    }

    if (!can_drop_obj (ch, obj))
    {
        SEND ("You can't let go of it.\r\n", ch);
        return;
    }

    if (victim->carry_number + get_obj_number (obj) > can_carry_n (victim))
    {
        act ("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (get_carry_weight (victim) + get_obj_weight (obj) >
        can_carry_w (victim))
    {
        act ("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!can_see_obj (victim, obj))
    {
        act ("$N can't see it.", ch, NULL, victim, TO_CHAR);
        return;
    }

    obj_from_char (obj);
    obj_to_char (obj, victim);	
    MOBtrigger = FALSE;
    act ("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
    act ("$n gives you $p.", ch, obj, victim, TO_VICT);
    act ("You give $p to $N.", ch, obj, victim, TO_CHAR);		
    MOBtrigger = TRUE;
	if ( HAS_TRIGGER_OBJ( obj, TRIG_GIVE ) )
		p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GIVE );
    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GIVE ) )
		p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GIVE );
		
    /*
     * Give trigger
     */
    if (IS_NPC (victim) && HAS_TRIGGER_MOB(victim, TRIG_GIVE))
        p_give_trigger (victim, NULL, NULL, ch, obj, TRIG_GIVE);

	//Quest updating.
	QUEST_DATA *quest;
	char buf2[MSL];
	for (quest = victim->quests; quest != NULL; quest = quest->next_quest)
	{
		if (quest->pIndexData && IS_SET(quest->pIndexData->type, B)) //item_get
		{
			if (obj->pIndexData->vnum == quest->pIndexData->target_obj_vnum)
			{
				quest->obj_multiples++;
				if (!quest->obj_multiples > quest->pIndexData->obj_mult)
				{
					if (quest->pIndexData->obj_mult < 2)
					{
						sprintf(buf2, "You have now collected the item for {g%s{x!\r\n",quest->name);
						SEND(buf2,victim);
					}
					else
					{
						sprintf(buf2, "You have now collected %d out of %d items for {g%s{x!\r\n",quest->obj_multiples, quest->pIndexData->obj_mult, quest->name);					
						SEND(buf2,victim);
					}
					
					if (has_completed_quest(victim, quest->pIndexData))
					{
						sprintf(buf2, "You've met the requirements for {g%s{x! Go turn it in!\r\n",quest->name);
						SEND(buf2,victim);
					}		
				}
			}
		}
	}
		
    return;
}


/* for poisoning weapons and food/drink */
void do_envenom (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent, skill;

    /* find out what */
    if (argument[0] == '\0')
    {
        SEND ("Envenom what item?\r\n", ch);
        return;
    }

    obj = get_obj_list (ch, argument, ch->carrying);

    if (obj == NULL)
    {
        SEND ("You don't have that item.\r\n", ch);
        return;
    }

    if ((skill = get_skill (ch, gsn_envenom)) < 1)
    {
        SEND ("Are you crazy? You'd poison yourself!\r\n", ch);
        return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
        if (IS_OBJ_STAT (obj, ITEM_BLESS)
            || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
        {
            act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (number_percent () < skill)
        {                        /* success! */
            act ("$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM);
            act ("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);
            if (!obj->value[3])
            {
                obj->value[3] = 1;
                check_improve (ch, gsn_envenom, TRUE, 4);
            }
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }

        act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
        if (!obj->value[3])
            check_improve (ch, gsn_envenom, FALSE, 4);
        WAIT_STATE (ch, skill_table[gsn_envenom].beats);
        return;
    }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
            || IS_WEAPON_STAT (obj, WEAPON_FROST)
            || IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)
            || IS_WEAPON_STAT (obj, WEAPON_SHARP)
            || IS_WEAPON_STAT (obj, WEAPON_VORPAL)
            || IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
            || IS_OBJ_STAT (obj, ITEM_BLESS)
            || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
        {
            act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (obj->value[3] < 0
            || attack_table[obj->value[3]].damage == DAM_BASH)
        {
            SEND ("You can only envenom edged weapons.\r\n", ch);
            return;
        }

        if (IS_WEAPON_STAT (obj, WEAPON_POISON))
        {
            act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
            return;
        }

        percent = number_percent ();
        if (percent < skill)
        {

            af.where = TO_WEAPON;
            af.type = gsn_poison;
            af.level = total_levels(ch) * percent / 100;
            af.duration = total_levels(ch) / 2 * percent / 100;
            af.location = 0;
            af.modifier = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj (obj, &af);

            act ("$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM);
            act ("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
            check_improve (ch, gsn_envenom, TRUE, 3);
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }
        else
        {
            act ("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
            check_improve (ch, gsn_envenom, FALSE, 3);
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }
    }

    act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
    return;
}

void do_fill (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Fill what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        SEND ("You do not have that item.\r\n", ch);
        return;
    }

    found = FALSE;
    for (fountain = ch->in_room->contents; fountain != NULL;
         fountain = fountain->next_content)
    {
        if (fountain->item_type == ITEM_FOUNTAIN)
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        SEND ("There is no fountain here!\r\n", ch);
        return;
    }

    if (obj->item_type != ITEM_DRINK_CON)
    {
        SEND ("You can't fill that.\r\n", ch);
        return;
    }

    if (obj->value[1] != 0 && obj->value[2] != fountain->value[2])
    {
        SEND ("There is already another liquid in it.\r\n", ch);
        return;
    }

    if (obj->value[1] >= obj->value[0])
    {
        SEND ("Your container is full.\r\n", ch);
        return;
    }

    sprintf (buf, "You fill $p with %s from $P.",
             liq_table[fountain->value[2]].liq_name);
    act (buf, ch, obj, fountain, TO_CHAR);
    sprintf (buf, "$n fills $p with %s from $P.",
             liq_table[fountain->value[2]].liq_name);
    act (buf, ch, obj, fountain, TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA * ch, char *argument)
{
    char arg1[MSL], buf[MAX_STRING_LENGTH];
	char arg2[MSL];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || argument[0] == '\0')
    {
        SEND ("Pour what into/onto what?\r\n", ch);
        return;
    }

    if ((out = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        SEND ("You don't have that item.\r\n", ch);
        return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
        SEND ("That's not a drink container.\r\n", ch);
        return;
    }

    if (!str_cmp (arg2, "out"))
    {
        if (out->value[1] == 0)
        {
            SEND ("It's already empty.\r\n", ch);
            return;
        }

        out->value[1] = 0;
        out->value[3] = 0;
        sprintf (buf, "You dumps $p, pouring %s all over the ground.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, out, NULL, TO_CHAR);

        sprintf (buf, "$n dumps $p, pouring %s all over the ground.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, out, NULL, TO_ROOM);
        return;
    }

    if ((in = get_obj_here( ch, NULL, arg2)) == NULL)
    {
        vch = get_char_room ( ch, NULL, arg2);

        if (vch == NULL)
        {
            SEND ("Pour into/onto what?\r\n", ch);
            return;
        }

        in = get_obj_carry (vch, arg2, vch)) == NULL));

        if (in == NULL)
        {
            SEND ("They aren't holding that.\r\n", ch);
            return;
        }
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
        SEND ("You can only pour into other drink containers.\r\n", ch);
        return;
    }

    if (in == out)
    {
        SEND ("You cannot change the laws of physics!\r\n", ch);
        return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
        SEND ("They don't hold the same liquid.\r\n", ch);
        return;
    }

    if (out->value[1] == 0)
    {
        act ("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
        return;
    }

    if (in->value[1] >= in->value[0])
    {
        act ("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
        return;
    }

    amount = UMIN (out->value[1], in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];

    if (vch == NULL)
    {
        sprintf (buf, "You pour %s from $p into $P.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, out, in, TO_CHAR);
        sprintf (buf, "$n pours %s from $p into $P.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, out, in, TO_ROOM);
    }
    else
    {
        sprintf (buf, "You pour some %s for $N.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, NULL, vch, TO_CHAR);
        sprintf (buf, "$n pours you some %s.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, NULL, vch, TO_VICT);
        sprintf (buf, "$n pours some %s for $N.",
                 liq_table[out->value[2]].liq_name);
        act (buf, ch, NULL, vch, TO_NOTVICT);

    }
}

void do_drink (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        for (obj = ch->in_room->contents; obj; obj = obj->next_content)
        {
            if (obj->item_type == ITEM_FOUNTAIN)
                break;
        }

        if (obj == NULL)
        {
            SEND ("Drink what?\r\n", ch);
            return;
        }
    }
    else
    {
        if ((obj = get_obj_here( ch, NULL, arg)) == NULL)
        {
            SEND ("You can't find it.\r\n", ch);
            return;
        }
    }

    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    {
        SEND ("You fail to reach your mouth.  *Hic*\r\n", ch);
        return;
    }

    switch (obj->item_type)
    {
        default:
            SEND ("You can't drink from that.\r\n", ch);
            return;

        case ITEM_FOUNTAIN:
            if ((liquid = obj->value[2]) < 0)
            {
                bug ("Do_drink: bad liquid number %d.", liquid);
                liquid = obj->value[2] = 0;
            }
            amount = liq_table[liquid].liq_affect[4] * 3;
            break;

        case ITEM_DRINK_CON:
            if (obj->value[1] <= 0)
            {
                SEND ("It is already empty.\r\n", ch);
                return;
            }

            if ((liquid = obj->value[2]) < 0)
            {
                bug ("Do_drink: bad liquid number %d.", liquid);
                liquid = obj->value[2] = 0;
            }

            amount = liq_table[liquid].liq_affect[4];
            amount = UMIN (amount, obj->value[1]);
            break;
    }
    if (!IS_NPC (ch) && !IS_IMMORTAL (ch)
        && ch->pcdata->condition[COND_FULL] > 45)
    {
        SEND ("You're too full to drink more.\r\n", ch);
        return;
    }

    act ("$n drinks $T from $p.",
         ch, obj, liq_table[liquid].liq_name, TO_ROOM);
    act ("You drink $T from $p.",
         ch, obj, liq_table[liquid].liq_name, TO_CHAR);

    gain_condition (ch, COND_DRUNK,
                    amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36);
    //gain_condition (ch, COND_FULL,
    //                amount * liq_table[liquid].liq_affect[COND_FULL] / 8);
    gain_condition (ch, COND_THIRST,
                    amount * liq_table[liquid].liq_affect[COND_THIRST] / 10);
    gain_condition (ch, COND_HUNGER,
                    amount * liq_table[liquid].liq_affect[COND_HUNGER] / 4);

    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        SEND ("You feel drunk.\r\n", ch);
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
        SEND ("You are full.\r\n", ch);
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] > 40)
        SEND ("Your thirst is quenched.\r\n", ch);

    if (obj->value[3] != 0)
    {
        /* The drink was poisoned ! */
        AFFECT_DATA af;

        act ("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
        SEND ("You choke and gag.\r\n", ch);
        af.where = TO_AFFECTS;
        af.type = gsn_poison;
        af.level = number_fuzzy (amount);
        af.duration = 3 * amount;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_POISON;
        affect_join (ch, &af);
    }

    if (obj->value[0] > 0)
        obj->value[1] -= amount;

    return;
}



void do_eat (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA 		*obj;	

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Eat what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        SEND ("You do not have that item.\r\n", ch);
        return;
    }

    if (!IS_IMMORTAL (ch))
    {
        if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL)
        {
            SEND ("That's not edible.\r\n", ch);
            return;
        }

        if (!IS_NPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
        {
            SEND ("You are too full to eat more.\r\n", ch);
            return;
        }
    }

    act ("$n eats $p.", ch, obj, NULL, TO_ROOM);
    act ("You eat $p.", ch, obj, NULL, TO_CHAR);

    switch (obj->item_type)
    {

        case ITEM_FOOD:
            if (!IS_NPC (ch))
            {
                int condition;

                condition = ch->pcdata->condition[COND_HUNGER];
                gain_condition (ch, COND_FULL, obj->value[0]);
                gain_condition (ch, COND_HUNGER, obj->value[1]);
                if (condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0)
                    SEND ("You are no longer hungry.\r\n", ch);
                else if (ch->pcdata->condition[COND_FULL] > 40)
                    SEND ("You are full.\r\n", ch);
            }

            if (obj->value[3] != 0)
            {
                /* The food was poisoned! */
                AFFECT_DATA af;

                act ("$n chokes and gags.", ch, 0, 0, TO_ROOM);
                SEND ("You choke and gag.\r\n", ch);

                af.where = TO_AFFECTS;
                af.type = gsn_poison;
                af.level = number_fuzzy (obj->value[0]);
                af.duration = 2 * obj->value[0];
                af.location = APPLY_NONE;
                af.modifier = 0;
                af.bitvector = AFF_POISON;
                affect_join (ch, &af);
            }
            break;

        case ITEM_PILL:
            obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
            obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
            obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);
            break;
    }

    extract_obj (obj);
    return;
}



/*
 * Remove an object.
 */
bool remove_obj (CHAR_DATA * ch, int iWear, bool fReplace)
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char (ch, iWear)) == NULL)
        return TRUE;

    if (!fReplace)
        return FALSE;

    if (IS_SET (obj->extra_flags, ITEM_NOREMOVE))
    {
        act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        return FALSE;
    }

    unequip_char (ch, obj);
    act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
    act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace)
{
    char buf[MAX_STRING_LENGTH];

    if (total_levels(ch) < (obj->level - 2))
    {
        sprintf (buf, "You must be level %d to use this object.\r\n",
                 obj->level);
        SEND (buf, ch);
        if (!IS_NPC(ch))
			act ("$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM);
        return;
    }

	if (is_trapped(obj))
		spring_trap(ch,obj);
	
    if (obj->item_type == ITEM_LIGHT)
    {
        if (!remove_obj (ch, WEAR_LIGHT, fReplace))
            return;
        act ("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
        act ("You light $p and hold it.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_LIGHT);
        return;
    }

	if (obj->item_type == ITEM_ARMOR && !IS_NPC(ch))
	{
		ch->bulk = get_bulk(ch, FALSE);		
		if (ch->bulk + obj->value[4] > ch_class_table[ch->ch_class].max_bulk)
		{
			if (!IS_MCLASSED(ch))
			{
				SEND("That would bring you over your bulk limit.\r\n",ch);
				return;
			}
			else
			{
				if (ch->bulk + obj->value[4] > ((ch_class_table[ch->ch_class].max_bulk + ch_class_table[ch->ch_class2].max_bulk) / 2))
				{
					SEND("That would bring you over your bulk limit.\r\n",ch);
					return;
				}
			}
		}
		
		if (obj->value[4] > ch_class_table[ch->ch_class].item_bulk)
		{			
			if (!IS_MCLASSED(ch))
			{
				SEND("That item is too bulky for you to wear.\r\n",ch);
				return;		
			}		
			else
			{
				if (obj->value[4] > ch_class_table[ch->ch_class2].item_bulk)
				{
					SEND("That item is too bulky for you to wear.\r\n",ch);
					return;
				}
			}
		}
	}
	
    if (CAN_WEAR (obj, ITEM_WEAR_FINGER))
    {
        if (get_eq_char (ch, WEAR_FINGER_L) != NULL
            && get_eq_char (ch, WEAR_FINGER_R) != NULL
            && !remove_obj (ch, WEAR_FINGER_L, fReplace)
            && !remove_obj (ch, WEAR_FINGER_R, fReplace))
            return;

        if (get_eq_char (ch, WEAR_FINGER_L) == NULL)
        {
            act ("$n wears $p on $s left finger.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p on your left finger.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_FINGER_L);
            return;
        }

        if (get_eq_char (ch, WEAR_FINGER_R) == NULL)
        {
            act ("$n wears $p on $s right finger.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p on your right finger.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_FINGER_R);
            return;
        }

        bug ("Wear_obj: no free finger.", 0);
        SEND ("You already wear two rings.\r\n", ch);
        return;
    }
	
	if (CAN_WEAR (obj, ITEM_WEAR_SHEATH))
	{
		if (!remove_obj (ch, WEAR_SHEATH, fReplace))
            return;
        act ("$n wears $p on $s hip.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your hip.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_SHEATH);
        return;
	}
	
    if (CAN_WEAR (obj, ITEM_WEAR_NECK))
    {
        if (get_eq_char (ch, WEAR_NECK_1) != NULL
            && get_eq_char (ch, WEAR_NECK_2) != NULL
            && !remove_obj (ch, WEAR_NECK_1, fReplace)
            && !remove_obj (ch, WEAR_NECK_2, fReplace))
            return;

        if (get_eq_char (ch, WEAR_NECK_1) == NULL)
        {
            act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_NECK_1);
            return;
        }

        if (get_eq_char (ch, WEAR_NECK_2) == NULL)
        {
            act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_NECK_2);
            return;
        }

        bug ("Wear_obj: no free neck.", 0);
        SEND ("You already wear two neck items.\r\n", ch);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_BODY))
    {
        if (!remove_obj (ch, WEAR_BODY, fReplace))
            return;
        act ("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your torso.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_BODY);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_HEAD))
    {
        if (!remove_obj (ch, WEAR_HEAD, fReplace))
            return;
        act ("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your head.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_HEAD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_LEGS))
    {
		if (get_eq_char (ch, WEAR_LEGS) != NULL
		&& get_eq_char (ch, WEAR_LEGS2) != NULL
		&& !remove_obj (ch, WEAR_LEGS, fReplace)
		&& !remove_obj (ch, WEAR_LEGS2, fReplace))
			return;

        if (get_eq_char (ch, WEAR_LEGS) == NULL)
        {
            act ("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p on your legs.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_LEGS);
            return;
        }

        if (get_eq_char (ch, WEAR_LEGS2) == NULL && IS_CENTAUR(ch))
        {
            act ("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p on your legs.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_LEGS2);
            return;
        }

        bug ("Wear_obj: no free leg slot.", 0);
        SEND ("You already wear two leg items.\r\n", ch);
        return;
		
    }

    if (CAN_WEAR (obj, ITEM_WEAR_FEET))
    {
        if (!remove_obj (ch, WEAR_FEET, fReplace))
            return;
        act ("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your feet.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_FEET);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_HANDS))
    {
        if (!remove_obj (ch, WEAR_HANDS, fReplace))
            return;
        act ("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your hands.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_HANDS);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_ARMS))
    {
        if (!remove_obj (ch, WEAR_ARMS, fReplace))
            return;
        act ("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your arms.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_ARMS);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_ABOUT))
    {
        if (!remove_obj (ch, WEAR_ABOUT, fReplace))
            return;
        act ("$n wears $p about $s body.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p about your body.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_ABOUT);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_WAIST))
    {
        if (!remove_obj (ch, WEAR_WAIST, fReplace))
            return;
        act ("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p about your waist.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_WAIST);
        return;
    }

	if (CAN_WEAR (obj, ITEM_WEAR_TAIL))
	{
		if (!HAS_TAIL(ch))
		{
			SEND("You don't have a tail to wear that on.\r\n",ch);
			return;
		}
		if (!remove_obj (ch, WEAR_TAIL, fReplace))
            return;
        act ("$n wears $p on $s tail.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your tail.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_TAIL);
        return;
	}
	
    if (CAN_WEAR (obj, ITEM_WEAR_WRIST))
    {
        if (get_eq_char (ch, WEAR_WRIST_L) != NULL
            && get_eq_char (ch, WEAR_WRIST_R) != NULL
            && !remove_obj (ch, WEAR_WRIST_L, fReplace)
            && !remove_obj (ch, WEAR_WRIST_R, fReplace))
            return;

        if (get_eq_char (ch, WEAR_WRIST_L) == NULL)
        {
            act ("$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your left wrist.",
                 ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_WRIST_L);
            return;
        }

        if (get_eq_char (ch, WEAR_WRIST_R) == NULL)
        {
            act ("$n wears $p around $s right wrist.",
                 ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your right wrist.",
                 ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_WRIST_R);
            return;
        }

        bug ("Wear_obj: no free wrist.", 0);
        SEND ("You already wear two wrist items.\r\n", ch);
        return;
    }

	
	 if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            SEND ("You cannot use a shield while using 2 weapons.\r\n",ch);
            return;
        }		
		if (get_eq_char (ch, WEAR_HOLD) != NULL)
		{
			SEND ("Remove whatever you're holding first.\r\n",ch);
			return;
		}		
		

        if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
            return;
        act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
        act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_SHIELD );
        return;
    }


    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            SEND ("You cannot hold an item while using 2 weapons.\r\n",ch);
            return;
        }
		if (get_eq_char (ch, WEAR_SHIELD) != NULL)
		{
			SEND ("Remove your shield first.\r\n",ch);
			return;
		}		

        if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
            return;
        act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
        act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_HOLD );
        return;
    }
	
    /*if (CAN_WEAR (obj, ITEM_WEAR_SHIELD))
    {
        OBJ_DATA *weapon;

        if (!remove_obj (ch, WEAR_SHIELD, fReplace))
            return;

        weapon = get_eq_char (ch, WEAR_WIELD);
        if (weapon != NULL && ch->size < SIZE_LARGE
            && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
        {
            SEND ("Your hands are tied up with your weapon!\r\n", ch);
            return;
        }

        act ("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p as a shield.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_SHIELD);
        return;
    }
*/
 

 
  if (CAN_WEAR (obj, ITEM_WIELD))
    {
        int sn, skill;

        if (!remove_obj (ch, WEAR_WIELD, fReplace))
            return;

        if (!IS_NPC (ch)
            && get_obj_weight (obj) >
            (str_app[GET_STR(ch)].wield * 10))
        {
            SEND ("It is too heavy for you to wield.\r\n", ch);
            return;
        }

        if (!IS_NPC (ch) && ch->size < SIZE_LARGE
            && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS)
            && get_eq_char (ch, WEAR_SHIELD) != NULL)
        {
            SEND ("You need two hands free for that weapon.\r\n", ch);
            return;
        }

        act ("$n wields $p.", ch, obj, NULL, TO_ROOM);
        act ("You wield $p.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_WIELD);

        sn = get_weapon_sn (ch, FALSE);

        if (sn == gsn_hand_to_hand)
            return;

        skill = get_weapon_skill (ch, sn);

        if (skill >= 100)
            act ("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);
        else if (skill > 85)
            act ("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);
        else if (skill > 70)
            act ("You are skilled with $p.", ch, obj, NULL, TO_CHAR);
        else if (skill > 50)
            act ("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);
        else if (skill > 25)
            act ("$p feels a little clumsy in your hands.", ch, obj, NULL,
                 TO_CHAR);
        else if (skill > 1)
            act ("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);
        else
            act ("You don't even know which end is up on $p.",
                 ch, obj, NULL, TO_CHAR);

        return;
    }

    if (CAN_WEAR (obj, ITEM_HOLD))
    {
		if (get_eq_char (ch, WEAR_SHIELD) != NULL)
		{
			SEND ("Remove your shield first.\r\n",ch);
			return;
		}	
		if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
		{
			SEND ("Remove your secondary weapon first.\r\n",ch);
			return;
		}
	
        if (!remove_obj (ch, WEAR_HOLD, fReplace))
            return;
        act ("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
        act ("You hold $p in your hand.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_HOLD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_FLOAT))
    {
        if (!remove_obj (ch, WEAR_FLOAT, fReplace))
            return;
        act ("$n releases $p to float next to $m.", ch, obj, NULL, TO_ROOM);
        act ("You release $p and it floats next to you.", ch, obj, NULL,
             TO_CHAR);
        equip_char (ch, obj, WEAR_FLOAT);
        return;
    }

    if (fReplace)
        SEND ("You can't wear, wield, or hold that.\r\n", ch);

    return;
}



void do_wear (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
	SHOP_DATA *pShop;
	
    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Wear, wield, or hold what?\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        OBJ_DATA *obj_next;

        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
			{
				if (IS_NPC(ch))
				{
					if (obj->item_type != ITEM_GEM)
					{
						if ((pShop = ch->pIndexData->pShop) == NULL)
							wear_obj (ch, obj, FALSE);
					}
				}
				else
				{
					wear_obj (ch, obj, FALSE);
				}
			}
        }
        return;
    }
    else
    {
        if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
        {
            SEND ("You do not have that item.\r\n", ch);
            return;
        }

        wear_obj (ch, obj, TRUE);
    }

    return;
}


void do_remove (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
	int i;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Remove what?\r\n", ch);
        return;
    }
	if (!str_cmp (arg, "all"))
	{
		if (IS_AFFECTED( ch, AFF_CHARM) && !IS_NPC(ch) )
		{
			SEND("You're too busy following your master to do that!\r\n",ch);
			return;
		}
		bool found = FALSE;
			for ( i = 0; i < MAX_WEAR; i++ )
			{
				if ( ( obj = get_eq_char( ch, i ) ) == NULL )
					continue;

				found = TRUE;            
				if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
				{
					act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
					continue;
				}
				unequip_char( ch, obj );
				act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
				act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
			}
			if(!found)
				SEND("You don't have anything to remove!\r\n",ch);
			return;
	}
	
    if ((obj = get_obj_wear( ch, arg, FALSE)) == NULL)
    {
        SEND ("You do not have that item.\r\n", ch);
        return;
    }

    remove_obj (ch, obj->wear_loc, TRUE);
    return;
}



void do_sacrifice (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
	OBJ_DATA *obj_next;
    int silver = 0, total_silver = 0;
	bool all = FALSE;
	int items = 0;

    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument (argument, arg);

    if (arg[0] == '\0' || !str_cmp (arg, ch->name))
    {
        act ("$n offers $mself to his god, who graciously declines.",
             ch, NULL, NULL, TO_ROOM);
        sprintf(buffer, "%s appreciates your offer and may accept it later.\r\n", god_table[ch->god].name);
		SEND
            (buffer, ch);
        return;
    }
	
	if (!str_cmp(arg, "all"))
	{
		all = TRUE;
	}
	else
	{
		obj = get_obj_list (ch, arg, ch->in_room->contents);
		if (obj == NULL)
		{
			SEND ("You can't find it.\r\n", ch);
			return;
		}		
		
		if (IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM))
		{
			SEND("Quest items aren't sacrificial.\r\n",ch);
			return;
		}
	}

	if (all == TRUE)
	{
		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
					
			if (obj->item_type == ITEM_CORPSE_PC)
			{
				if (obj->contains)
				{
					sprintf (buf, "%s wouldn't like that.\r\n", god_table[ch->god].name);
					SEND(buf, ch);
					continue;
				}
				
			}

			if (IS_OBJ_STAT(obj, ITEM_INDESTRUCTABLE) || IS_OBJ_STAT(obj, ITEM_UNIQUE))
				continue;
			if (IS_OBJ_STAT(obj, ITEM_QUEST_ITEM))
				continue;

			if (!CAN_WEAR (obj, ITEM_TAKE) || CAN_WEAR (obj, ITEM_NO_SAC))
			{
				continue;
				//act ("$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR);				
			}

			if (obj->in_room != NULL)
			{
				for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
					if (gch->on == obj)
					{
						act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
						return;
					}
			}

			silver = UMAX (1, obj->level * 3);

			if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
				silver = UMIN (silver, obj->cost);			

			ch->silver += silver;
			total_silver += silver;
			items++;

			if (IS_SET (ch->act, PLR_AUTOSPLIT))
			{                            /* AUTOSPLIT code */
				members = 0;
				for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
				{
					if (is_same_group (gch, ch))
						members++;
				}

				if (members > 1 && silver > 1)
				{
					sprintf (buffer, "%d", silver);
					do_function (ch, &do_split, buffer);
				}
			}			
			extract_obj (obj);			
		}
		if (total_silver == 1)
		{
			sprintf
				(buf, "%s gives you one silver coin for sacrificing %d item%s.\r\n", 
					god_table[ch->god].name, 
					items,
					items > 1 ? "s" : "");
			SEND(buf, ch);
		}
		else
		{
			sprintf (buf,
					 "%s gives you %d silver coins for sacrificing %d item%s.\r\n",
					 god_table[ch->god].name, 
					 total_silver, 
					 items,
					 items > 1 ? "s" : "");
			SEND (buf, ch);
		}
		act ("$n sacrifices everything in the vicinity to their god.", ch, NULL, NULL, TO_ROOM);
		wiznet ("$N sacrifices everything in the room.",
					ch, NULL, WIZ_SACCING, 0, 0);
		return;
	}
	else
	{
		if (obj->item_type == ITEM_CORPSE_PC)
		{
			if (obj->contains)
			{
				sprintf (buf, "%s wouldn't like that.\r\n", god_table[ch->god].name);
				SEND(buf, ch);
				return;
			}
		}


		if (!CAN_WEAR (obj, ITEM_TAKE) || CAN_WEAR (obj, ITEM_NO_SAC))
		{
			//act ("$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR);
			return;
		}
		
		if (IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM))
		{
			SEND("Quest items aren't sacrificial.\r\n",ch);
			return;
		}

		if (obj->in_room != NULL)
		{
			for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
				if (gch->on == obj)
				{
					act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
					return;
				}
		}

		silver = UMAX (1, obj->level * 3);

		if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
			silver = UMIN (silver, obj->cost);

		if (silver == 1)
		{
			sprintf
				(buf, "%s gives you one silver coin for your sacrifice.\r\n", god_table[ch->god].name);
			SEND(buf, ch);
		}
		else
		{
			sprintf (buf,
					 "%s gives you %d silver coins for your sacrifice.\r\n",
					 god_table[ch->god].name, silver);
			SEND (buf, ch);
		}

		ch->silver += silver;

		if (IS_SET (ch->act, PLR_AUTOSPLIT))
		{                            /* AUTOSPLIT code */
			members = 0;
			for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
			{
				if (is_same_group (gch, ch))
					members++;
			}

			if (members > 1 && silver > 1)
			{
				sprintf (buffer, "%d", silver);
				do_function (ch, &do_split, buffer);
			}
		}

		act ("$n sacrifices $p to their god.", ch, obj, NULL, TO_ROOM);
		wiznet ("$N sends up $p as a burnt offering.",
				ch, obj, WIZ_SACCING, 0, 0);
		extract_obj (obj);
		return;
	}
}



void do_quaff (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Quaff what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        SEND ("You do not have that potion.\r\n", ch);
        return;
    }

    if (obj->item_type != ITEM_POTION)
    {
        SEND ("You can quaff only potions.\r\n", ch);
        return;
    }

    if (total_levels(ch) < obj->level)
    {
        SEND ("This liquid is too powerful for you to drink.\r\n",
                      ch);
        return;
    }

    act ("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
    act ("You quaff $p.", ch, obj, NULL, TO_CHAR);

    obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
    obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
    obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);
	if (!IS_IMMORTAL(ch))
		WAIT_STATE (ch, PULSE_VIOLENCE);
    extract_obj (obj);
    return;
}



void do_recite (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if ((scroll = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        SEND ("You do not have that scroll.\r\n", ch);
        return;
    }

    if (scroll->item_type != ITEM_SCROLL)
    {
        SEND ("You can recite only scrolls.\r\n", ch);
        return;
    }

    if (total_levels(ch) < scroll->level)
    {
        SEND ("This scroll is too complex for you to comprehend.\r\n",
                      ch);
        return;
    }
	
	int chance = 40;  //chance of spell failure.
	//Intelligence modifiers:	
	chance -= (total_levels(ch) / 5);
	if (get_curr_stat(ch, STAT_INT) >= 13)
		chance -= ((get_curr_stat(ch, STAT_INT) - 12) * 2);
	if (get_curr_stat(ch, STAT_INT) <= 10)
		chance += (25 - get_curr_stat(ch, STAT_INT));	
		
	//fighters/thieves have a harder time unless multiclassed.
	if ((IS_FIGHTER(ch) || IS_THIEF(ch)) && !IS_MCLASSED(ch))
		chance += 15;
		
	if (chance > number_range(1,100))
	{
		SEND("You stumbled over the words, the casting failed!\r\n",ch);
		act( "$p burns brightly and is gone.", ch, scroll, NULL, TO_CHAR );
		extract_obj (scroll);
		return;
	}
	OBJ_DATA *obj;
    obj = NULL;
    if (arg2[0] == '\0')
    {
        victim = ch;
    }
    else
    {
        if ((victim = get_char_room ( ch, NULL, arg2)) == NULL
            && (obj = get_obj_here( ch, NULL, arg2)) == NULL)
        {
            SEND ("You can't find it.\r\n", ch);
            return;
        }
    }

    act ("$n recites $p.", ch, scroll, NULL, TO_ROOM);
    act ("You recite $p.", ch, scroll, NULL, TO_CHAR);

    if (number_percent () <= get_skill (ch, gsn_scrolls) * 9 / 10)
    {
        SEND ("You mispronounce a syllable.\r\n", ch);
		act( "$p burns brightly and is gone.", ch, scroll, NULL, TO_CHAR );
		extract_obj (scroll);
        check_improve (ch, gsn_scrolls, FALSE, 2);
    }
    else
    {
        obj_cast_spell (scroll->value[1], scroll->value[0], ch, victim, obj);
        obj_cast_spell (scroll->value[2], scroll->value[0], ch, victim, obj);
        obj_cast_spell (scroll->value[3], scroll->value[0], ch, victim, obj);
        check_improve (ch, gsn_scrolls, TRUE, 3);
    }

    extract_obj (scroll);
    return;
}



void do_brandish (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ((staff = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
        SEND ("You hold nothing in your hand.\r\n", ch);
        return;
    }

    if (staff->item_type != ITEM_STAFF)
    {
        SEND ("You can brandish only with a staff.\r\n", ch);
        return;
    }

    if ((sn = staff->value[3]) < 0
        || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
        bug ("Do_brandish: bad sn %d.", sn);
        return;
    }

    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);

    if (staff->value[2] > 0)
    {
        act ("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
        act ("You brandish $p.", ch, staff, NULL, TO_CHAR);
        if (total_levels(ch) < staff->level
            || number_percent () >= 20 + get_skill (ch, gsn_staves) * 4 / 5)
        {
            act ("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
            act ("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
            check_improve (ch, gsn_staves, FALSE, 2);
        }

        else
            for (vch = ch->in_room->people; vch; vch = vch_next)
            {
                vch_next = vch->next_in_room;

                switch (skill_table[sn].target)
                {
                    default:
                        bug ("Do_brandish: bad target for sn %d.", sn);
                        return;

                    case TAR_IGNORE:
                        if (vch != ch)
                            continue;
                        break;

                    case TAR_CHAR_OFFENSIVE:
                        if (IS_NPC (ch) ? IS_NPC (vch) : !IS_NPC (vch))
                            continue;
                        break;

                    case TAR_CHAR_DEFENSIVE:
                        if (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch))
                            continue;
                        break;

                    case TAR_CHAR_SELF:
                        if (vch != ch)
                            continue;
                        break;
                }

                obj_cast_spell (staff->value[3], staff->value[0], ch, vch,
                                NULL);
                check_improve (ch, gsn_staves, TRUE, 2);
            }
    }

    if (--staff->value[2] <= 0)
    {
        act ("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
        act ("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
        extract_obj (staff);
    }

    return;
}



void do_zap (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
	char arg2[MIL];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;
	int i = 0;

	argument = one_argument (argument, arg);
	argument = one_argument (argument, arg2);

	if (ch->cooldowns[gsn_wands] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if (arg[0] == '\0' && ch->fighting == NULL)
    {
        SEND ("Zap whom or what?\r\n", ch);
        return;
    }
	
	bool found = FALSE;
	for ( i = 0; i < MAX_WEAR; i++ )
	{
		if ( ( wand = get_eq_char( ch, i ) ) != NULL && (wand->item_type == ITEM_WAND || wand->item_type == ITEM_FIGURINE || wand->item_type == ITEM_STAFF))
		{
			if (arg2[0] == '\0')
			{
				found = TRUE;
				break;
			}
			else
			{			
				if ( (wand = get_eq_char_by_name(ch, arg2)) != NULL && (wand->item_type == ITEM_WAND || wand->item_type == ITEM_FIGURINE || wand->item_type == ITEM_STAFF))
				{
					found = TRUE;
					break;
				}
			}
		}
		else
			continue;				
	}
    if (!found)
	{
        SEND ("You hold no zapping device.\r\n", ch);
        return;
    }


    obj = NULL;
    if (arg[0] == '\0')
    {
        if (ch->fighting != NULL)
        {
            victim = ch->fighting;
        }
        else
        {
            SEND ("Zap whom or what?\r\n", ch);
            return;
        }
    }
    else
    {
        if ((victim = get_char_room ( ch, NULL, arg)) == NULL
            && (obj = get_obj_here( ch, NULL, arg)) == NULL)
        {
            SEND ("You can't find it.\r\n", ch);
            return;
        }
    }
	
	if (ch->boon != BOON_GUNSLINGER)		
	{
		WAIT_STATE (ch, PULSE_VIOLENCE * 2);
		COOLDOWN(ch, gsn_wands);
	}
    if (wand->value[2] > 0 || wand->value[2] == -1)
    {
        if (victim != NULL)
        {
            act ("$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT);
            act ("You zap $N with $p.", ch, wand, victim, TO_CHAR);
            act ("$n zaps you with $p.", ch, wand, victim, TO_VICT);
        }
        else
        {
            act ("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
            act ("You zap $P with $p.", ch, wand, obj, TO_CHAR);
        }

		if (!IS_NPC(ch))
		{
			if (total_levels(ch) < (wand->level - 2)
				|| number_percent () >= 20 + get_skill (ch, gsn_wands) * 4 / 5)
			{
				if (wand->item_type == ITEM_WAND)
				{
					act ("Your efforts with $p produce only smoke and sparks.",
						ch, wand, NULL, TO_CHAR);
					act ("$n's efforts with $p produce only smoke and sparks.",
						ch, wand, NULL, TO_ROOM);
				}
				else
				{
					act ("The magic in $p fails to activate.", ch, wand, NULL, TO_CHAR);					
					act ("$n's looks at $p in profound frustration.",ch, wand, NULL, TO_ROOM);
				}
				check_improve (ch, gsn_wands, FALSE, 2);
			}
			else
			{
				obj_cast_spell (wand->value[3], wand->value[0], ch, victim, obj);
				check_improve (ch, gsn_wands, TRUE, 2);
			}
		}
		else
		{
			if ( number_percent () >= 75 )
			{
			
				if (wand->item_type == ITEM_WAND)
				{
					act ("Your efforts with $p produce only smoke and sparks.",
						 ch, wand, NULL, TO_CHAR);
					act ("$n's efforts with $p produce only smoke and sparks.",
						 ch, wand, NULL, TO_ROOM);		

				}
				else
				{
					act ("The magic in $p fails to activate.", ch, wand, NULL, TO_CHAR);					
					act ("$n's looks at $p in profound frustration.",ch, wand, NULL, TO_ROOM);
				}					 
			}
			else
			{
				obj_cast_spell (wand->value[3], wand->value[0], ch, victim, obj);				
			}
		}
    }
	if (wand->value[2] == -1)
		return;
    else 
		--wand->value[2];
	if (wand->value[2] <= 0)
    {
        act ("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
        act ("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
        extract_obj (wand);
    }

    return;
}

void do_steal (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

	if (ch->cooldowns[gsn_steal] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        SEND ("Steal what from whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_room (ch, NULL, arg2)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim == ch)
    {
        SEND ("That's pointless.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
    {
        SEND ("Kill stealing is not permitted.\r\n"
                      "You'd better not -- you might get hit.\r\n", ch);
        return;
    }

    WAIT_STATE (ch, skill_table[gsn_steal].beats);
    percent = number_percent ();

    if (!IS_AWAKE (victim))
        percent -= 10;
    else if (!can_see (victim, ch))
        percent += 25;
    else
        percent += 50;

    if (((total_levels(ch) + 7 < total_levels(victim) || total_levels(ch) - 7 > total_levels(victim))
         && !IS_NPC (victim) && !IS_NPC (ch))
        || (!IS_NPC (ch) && percent > get_skill (ch, gsn_steal))
        || (!IS_NPC (ch) && !is_clan (ch)))
	{
		/*
		* Failure.
		*/
		caught_thief(ch, victim);      
		return;
	}

    if (!str_cmp (arg1, "coin")
        || !str_cmp (arg1, "coins")
        || !str_cmp (arg1, "gold") || !str_cmp (arg1, "silver"))
    {
        int gold, silver;

        gold = victim->gold * number_range (1, total_levels(ch)) / MAX_LEVEL;
        silver = victim->silver * number_range (1, total_levels(ch)) / MAX_LEVEL;
        if (gold <= 0 && silver <= 0)
        {
            SEND ("You couldn't get any coins.\r\n", ch);
            return;
        }

        ch->gold += gold;
        ch->silver += silver;
        victim->silver -= silver;
        victim->gold -= gold;
        if (silver <= 0)
            sprintf (buf, "Bingo!  You got %d gold coins.\r\n", gold);
        else if (gold <= 0)
            sprintf (buf, "Bingo!  You got %d silver coins.\r\n", silver);
        else
            sprintf (buf, "Bingo!  You got %d silver and %d gold coins.\r\n",
                     silver, gold);

        SEND (buf, ch);
        check_improve (ch, gsn_steal, TRUE, 2);
        return;
    }

    if ((obj = get_obj_carry (victim, arg1, ch)) == NULL)
    {
        SEND ("You can't find it.\r\n", ch);
        return;
    }

    if (!can_drop_obj (ch, obj)
        || IS_SET (obj->extra_flags, ITEM_INVENTORY)
        || obj->level > total_levels(ch))
    {
        SEND ("You can't pry it away.\r\n", ch);
        return;
    }

    if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
        SEND ("You have your hands full.\r\n", ch);
        return;
    }

    if (ch->carry_weight + get_obj_weight (obj) > can_carry_w (ch))
    {
        SEND ("You can't carry that much weight.\r\n", ch);
        return;
    }

    obj_from_char (obj);
    obj_to_char (obj, ch);
    act ("You pocket $p.", ch, obj, NULL, TO_CHAR);
    check_improve (ch, gsn_steal, TRUE, 2);
	ch->cooldowns[gsn_steal] = skill_table[gsn_steal].cooldown;
    SEND ("Got it!\r\n", ch);
    return;
}


/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper (CHAR_DATA * ch)
{
    /*char buf[MAX_STRING_LENGTH]; */
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room)
    {
        if (IS_NPC (keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
            break;
    }

    if (pShop == NULL)
    {
        SEND ("You can't do that here.\r\n", ch);
        return NULL;
    }

    /*
     * Undesirables.
     *
     if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
     {
     do_function(keeper, &do_say, "Killers are not welcome!");
     sprintf(buf, "%s the KILLER is over here!\r\n", ch->name);
     do_function(keeper, &do_yell, buf );
     return NULL;
     }

     if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
     {
     do_function(keeper, &do_say, "Thieves are not welcome!");
     sprintf(buf, "%s the THIEF is over here!\r\n", ch->name);
     do_function(keeper, &do_yell, buf );
     return NULL;
     }
     */
    /*
     * Shop hours.
     */
    if (time_info.hour < pShop->open_hour)
    {
        do_function (keeper, &do_say, "Sorry, I am closed. Come back later.");
        return NULL;
    }

    if (time_info.hour > pShop->close_hour)
    {
        do_function (keeper, &do_say,
                     "Sorry, I am closed. Come back tomorrow.");
        return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if (!can_see (keeper, ch))
    {
        do_function (keeper, &do_say,
                     "I don't trade with folks I can't see.");
        return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper (OBJ_DATA * obj, CHAR_DATA * ch)
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
        t_obj_next = t_obj->next_content;

        if (obj->pIndexData == t_obj->pIndexData
            && !str_cmp (obj->short_descr, t_obj->short_descr))
        {
            /* if this is an unlimited item, destroy the new one */
            if (IS_OBJ_STAT (t_obj, ITEM_INVENTORY))
            {
                extract_obj (obj);
                return;
            }
            obj->cost = t_obj->cost;    /* keep it standard */
            break;
        }
    }

    if (t_obj == NULL)
    {
        obj->next_content = ch->carrying;
        ch->carrying = obj;
    }
    else
    {
        obj->next_content = t_obj->next_content;
        t_obj->next_content = obj;
    }

    obj->carried_by = ch;
    obj->in_room = NULL;
    obj->in_obj = NULL;
    ch->carry_number += get_obj_number (obj);
    ch->carry_weight += get_obj_weight (obj);
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper (CHAR_DATA * ch, CHAR_DATA * keeper, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
    for (obj = keeper->carrying; obj != NULL; obj = obj->next_content)
    {
        if (obj->wear_loc == WEAR_NONE && can_see_obj (keeper, obj)
            && can_see_obj (ch, obj) && is_name (arg, obj->name))
        {
            if (++count == number)
                return obj;

            /* skip other objects of the same name */
            while (obj->next_content != NULL
                   && obj->pIndexData == obj->next_content->pIndexData
                   && !str_cmp (obj->short_descr,
                                obj->next_content->short_descr)) obj =
                    obj->next_content;
        }
    }

    return NULL;
}

int get_cost (CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy)
{
    SHOP_DATA *pShop;
    int cost;

    if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
        return 0;

    if (fBuy)
    {
        cost = obj->cost * pShop->profit_buy / 100;
    }
    else
    {
        OBJ_DATA *obj2;
        int itype;

        cost = 0;
        for (itype = 0; itype < MAX_TRADE; itype++)
        {
            if (obj->item_type == pShop->buy_type[itype])
            {
                cost = obj->cost * pShop->profit_sell / 100;
                break;
            }
        }

        if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
            for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content)
            {
                if (obj->pIndexData == obj2->pIndexData
                    && !str_cmp (obj->short_descr, obj2->short_descr))
                {
                    if (IS_OBJ_STAT (obj2, ITEM_INVENTORY))
                        cost /= 2;
                    else
                        cost = cost * 3 / 4;
                }
            }
    }

    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    {
        if (obj->value[1] == 0)
            cost /= 4;
        else
            cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int cost, roll;
	CHAR_DATA *keeper = NULL;	
	
    if (argument[0] == '\0')
    {
		if ((keeper = find_keeper (ch)) == NULL)
		{			
			return;
		}
	
        SEND ("Buy what?\r\n", ch);
        return;
    }

    if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
        char arg[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
        CHAR_DATA *pet;
        ROOM_INDEX_DATA *pRoomIndexNext;
        ROOM_INDEX_DATA *in_room;

        smash_tilde (argument);

        if (IS_NPC (ch))
            return;

        argument = one_argument (argument, arg);

		//Need to fix this pet shop hack: Upro
        pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
        if (pRoomIndexNext == NULL)
        {
            bug ("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
            SEND ("Sorry, you can't buy that here.\r\n", ch);
            return;
        }

        in_room = ch->in_room;
        ch->in_room = pRoomIndexNext;
        pet = get_char_room ( ch, NULL, arg);
        ch->in_room = in_room;

        if (pet == NULL || !IS_SET (pet->act, ACT_PET))
        {
            SEND ("Sorry, you can't buy that here.\r\n", ch);
            return;
        }

        if (ch->pet != NULL)
        {
            SEND ("You already own a pet.\r\n", ch);
            return;
        }

        cost = 10 * pet->level * pet->level;

        if ((ch->silver + 100 * ch->gold) < cost)
        {
            SEND ("You can't afford it.\r\n", ch);
            return;
        }

        if (total_levels(ch) < pet->level)
        {
            SEND
                ("You're not powerful enough to master this pet.\r\n", ch);
            return;
        }

        /* haggle */
        roll = number_percent ();
        if (roll < get_skill (ch, gsn_haggle))
        {
            cost -= cost / 2 * roll / 100;
            sprintf (buf, "You haggle the price down to %d coins.\r\n", cost);
            SEND (buf, ch);
            check_improve (ch, gsn_haggle, TRUE, 4);

        }

        deduct_cost (ch, cost);
        pet = create_mobile (pet->pIndexData);
        SET_BIT (pet->act, ACT_PET);
        SET_BIT (pet->affected_by, AFF_CHARM);
        pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

        argument = one_argument (argument, arg);
        if (arg[0] != '\0')
        {
            sprintf (buf, "%s %s", pet->name, arg);
            free_string (pet->name);
            pet->name = str_dup (buf);
        }

        sprintf (buf, "%sA neck tag says 'I belong to %s'.\r\n",
                 pet->description, ch->name);
        free_string (pet->description);
        pet->description = str_dup (buf);

        char_to_room (pet, ch->in_room);
        add_follower (pet, ch);
        pet->leader = ch;
        ch->pet = pet;
        SEND ("Enjoy your pet.\r\n", ch);
        act ("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
        return;
    }
    else
    {        
        OBJ_DATA *obj, *t_obj;
        char arg[MAX_INPUT_LENGTH];
        int number, count = 1;        

		if ((keeper = find_keeper (ch)) == NULL)
		{						
			return;
		}
		
        number = mult_argument (argument, arg);
        obj = get_obj_keeper (ch, keeper, arg);
        cost = get_cost (keeper, obj, TRUE);

        if (number < 1 || number > 99)
        {
            act ("$n tells you 'Get real!", keeper, NULL, ch, TO_VICT);
            return;
        }

        if (cost <= 0 || !can_see_obj (ch, obj))
        {
            act ("$n tells you 'I don't sell that -- try 'list''.",
                 keeper, NULL, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        if (!IS_OBJ_STAT (obj, ITEM_INVENTORY))
        {
            for (t_obj = obj->next_content;
                 count < number && t_obj != NULL; t_obj = t_obj->next_content)
            {
                if (t_obj->pIndexData == obj->pIndexData
                    && !str_cmp (t_obj->short_descr, obj->short_descr))
                    count++;
                else
                    break;
            }

            if (count < number)
            {
                act ("$n tells you 'I don't have that many in stock.",
                     keeper, NULL, ch, TO_VICT);
                ch->reply = keeper;
                return;
            }
        }

        if ((ch->silver + ch->gold * 100) < cost * number)
        {
            if (number > 1)
                act ("$n tells you 'You can't afford to buy that many.", keeper, obj, ch, TO_VICT);
            else
                act ("$n tells you 'You can't afford to buy $p'.",
                     keeper, obj, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        if (obj->level > total_levels(ch))
        {
            act ("$n tells you 'You can't use $p yet'.",
                 keeper, obj, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        if (ch->carry_number + number * get_obj_number (obj) >
            can_carry_n (ch))
        {
            SEND ("You can't carry that many items.\r\n", ch);
            return;
        }

        if (ch->carry_weight + number * get_obj_weight (obj) >
            can_carry_w (ch))
        {
            SEND ("You can't carry that much weight.\r\n", ch);
            return;
        }

        /* haggle */
        roll = number_percent ();
        if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
            && roll < get_skill (ch, gsn_haggle))
        {
            cost -= obj->cost / 2 * roll / 100;
            act ("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
            check_improve (ch, gsn_haggle, TRUE, 4);
        }

        if (number > 1)
        {
            sprintf (buf, "$n buys $p[%d].", number);
            act (buf, ch, obj, NULL, TO_ROOM);
            sprintf (buf, "You buy $p[%d] for %d silver.", number,
                     cost * number);
            act (buf, ch, obj, NULL, TO_CHAR);
        }
        else
        {
            act ("$n buys $p.", ch, obj, NULL, TO_ROOM);
            sprintf (buf, "You buy $p for %d silver.", cost);
            act (buf, ch, obj, NULL, TO_CHAR);
        }
        deduct_cost (ch, cost * number);
        keeper->gold += cost * number / 100;
        keeper->silver += cost * number - (cost * number / 100) * 100;

        for (count = 0; count < number; count++)
        {
            if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
                t_obj = create_object (obj->pIndexData, obj->level);
            else
            {
                t_obj = obj;
                obj = obj->next_content;
                obj_from_char (t_obj);
            }

            if (t_obj->timer > 0 && !IS_OBJ_STAT (t_obj, ITEM_HAD_TIMER))
                t_obj->timer = 0;
            REMOVE_BIT (t_obj->extra_flags, ITEM_HAD_TIMER);
            obj_to_char (t_obj, ch);
            if (cost < t_obj->cost)
                t_obj->cost = cost;
        }
    }
}



void do_list (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
	int gold = 0, silver = 0;
	
    if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
        //ROOM_INDEX_DATA *pRoomIndexNext;
        CHAR_DATA *pet;
        bool found;

        /* hack to make new thalos pets work */
        /*if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index (9706);
        else
            pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);

        if (pRoomIndexNext == NULL)
        {
            bug ("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
            SEND ("You can't do that here.\r\n", ch);
            return;
        }*/

        found = FALSE;
        for (pet = ch->in_room->people; pet; pet = pet->next_in_room)
        {
            if (IS_SET (pet->act, ACT_PET))
            {
                if (!found)
                {
                    found = TRUE;
                    SEND ("Pets for sale:\r\n", ch);
                }
                sprintf (buf, "[%2d] %8d - %s\r\n",
                         pet->level,
                         10 * pet->level * pet->level, pet->short_descr);
                SEND (buf, ch);
            }
        }
        if (!found)
            SEND ("Sorry, we're out of pets right now.\r\n", ch);
        return;
    }
    else
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost, count;		
        bool found;
        char arg[MAX_INPUT_LENGTH];

        if ((keeper = find_keeper (ch)) == NULL)
            return;
        one_argument (argument, arg);

        found = FALSE;
        for (obj = keeper->carrying; obj; obj = obj->next_content)
        {
            if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj)
                && (cost = get_cost (keeper, obj, TRUE)) > 0
                && (arg[0] == '\0' || is_name (arg, obj->name)))
            {
                if (!found)
                {
                    found = TRUE;
                    SEND ("[Lv     Price  Qty] Item\r\n", ch);
                }

                if (cost > 99)
				{
					gold = cost / 100;
					silver = cost % 100;
				}
				
				if (gold > 0)
				{
					if (IS_OBJ_STAT (obj, ITEM_INVENTORY))
					sprintf (buf, "[%2d %5dg %2ds -- ] %s\r\n",
							 obj->level, gold, silver, obj->short_descr);
					else
					{
						count = 1;

						while (obj->next_content != NULL
							   && obj->pIndexData == obj->next_content->pIndexData
							   && !str_cmp (obj->short_descr,
											obj->next_content->short_descr))
						{
							obj = obj->next_content;
							count++;
						}
						sprintf (buf, "[%2d %5dg %2ds %2d ] %s\r\n",
								 obj->level, gold, silver, count, obj->short_descr);
					}
				}
				else
				{
					if (IS_OBJ_STAT (obj, ITEM_INVENTORY))
						sprintf (buf, "[%2d %8ds -- ] %s\r\n",
								 obj->level, cost, obj->short_descr);
					else
					{
						count = 1;

						while (obj->next_content != NULL
							   && obj->pIndexData == obj->next_content->pIndexData
							   && !str_cmp (obj->short_descr,
											obj->next_content->short_descr))
						{
							obj = obj->next_content;
							count++;
						}
						sprintf (buf, "[%2d %8ds %2d ] %s\r\n",
								 obj->level, cost, count, obj->short_descr);
					}
				}
                SEND (buf, ch);
            }
        }

        if (!found)
            SEND ("You can't buy anything here.\r\n", ch);
        return;
    }
}



void do_sell (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost, roll;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Sell what?\r\n", ch);
        return;
    }

    if ((keeper = find_keeper (ch)) == NULL)
        return;

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        act ("$n tells you 'You don't have that item'.",
             keeper, NULL, ch, TO_VICT);
        ch->reply = keeper;
        return;
    }

    if (!can_drop_obj (ch, obj))
    {
        SEND ("You can't let go of it.\r\n", ch);
        return;
    }

    if (!can_see_obj (keeper, obj))
    {
        act ("$n doesn't see what you are offering.", keeper, NULL, ch,
             TO_VICT);
        return;
    }

	if (IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM))
	{
		SEND("Quest items aren't sellable.\r\n",ch);
		return;
	}
	
    if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
        act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
        return;
    }
    if (cost > (keeper->silver + 100 * keeper->gold))
    {
        act ("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
             keeper, obj, ch, TO_VICT);
        return;
    }

    act ("$n sells $p.", ch, obj, NULL, TO_ROOM);
    /* haggle */
    roll = number_percent ();
    if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
        && roll < get_skill (ch, gsn_haggle))
    {
        SEND ("You haggle with the shopkeeper.\r\n", ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN (cost, 95 * get_cost (keeper, obj, TRUE) / 100);
        cost = UMIN (cost, (keeper->silver + 100 * keeper->gold));
        check_improve (ch, gsn_haggle, TRUE, 4);
    }
    sprintf (buf, "You sell $p for %d silver and %d gold piece%s.",
             cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
    act (buf, ch, obj, NULL, TO_CHAR);
    ch->gold += cost / 100;
    ch->silver += cost - (cost / 100) * 100;
    deduct_cost (keeper, cost);
    if (keeper->gold < 0)
        keeper->gold = 0;
    if (keeper->silver < 0)
        keeper->silver = 0;

    if (obj->item_type == ITEM_TRASH || IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
    {
        extract_obj (obj);
    }
    else
    {
        obj_from_char (obj);
        if (obj->timer)
            SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        else
            obj->timer = number_range (50, 100);
        obj_to_keeper (obj, keeper);
    }

    return;
}



void do_value (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Value what?\r\n", ch);
        return;
    }

    if ((keeper = find_keeper (ch)) == NULL)
        return;

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        act ("$n tells you 'You don't have that item'.",
             keeper, NULL, ch, TO_VICT);
        ch->reply = keeper;
        return;
    }

    if (!can_see_obj (keeper, obj))
    {
        act ("$n doesn't see what you are offering.", keeper, NULL, ch,
             TO_VICT);
        return;
    }

    if (!can_drop_obj (ch, obj))
    {
        SEND ("You can't let go of it.\r\n", ch);
        return;
    }

    if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
        act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
        return;
    }

    sprintf (buf,
             "$n tells you 'I'll give you %d silver and %d gold coins for $p'.",
             cost - (cost / 100) * 100, cost / 100);
    act (buf, keeper, obj, ch, TO_VICT);
    ch->reply = keeper;

    return;
}

//Anything past here was added by Upro & co. 2009

void do_learn( CHAR_DATA *ch, char *argument ) 
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
		SEND( "Study what?\r\n", ch );
		return;
    }
	
    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
		SEND( "You do not have that item.\r\n", ch );
		return;
    }
		
    if ( obj->item_type != ITEM_SCROLL )
    {
		SEND( "You can only study scrolls and books.\r\n", ch );
		return;
    }	
	
    if ( (ch->ch_class == FIGHTER || ch->ch_class == THIEF) && !IS_MCLASSED(ch) ) 
	{
		SEND( "You cannot learn from scrolls, you must be {btaught{x magic.\r\n", ch);
		return;
	}	
	
	if ( get_skill (ch, gsn_scrolls) < 1 )
	{
		SEND("You don't have the knowledge to do that yet.\r\n",ch);
		return;
	}
	
	act( "$n studies $p.", ch, obj, NULL, TO_ROOM );
    act( "You study $p.", ch, obj, NULL, TO_CHAR );

    //This next line also prevents imms from making scrolls for peeps.
	if (total_levels(ch) + 2 < obj->level)
	{
		SEND("You cannot glean any knowledge from it.\r\n",ch);
		act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
		extract_obj( obj );
		return;
	}

    if (obj->item_type == ITEM_SCROLL)
	{
		sn = obj->value[1];
		if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
			{
			bug( "Do_learn: bad sn %d.", sn );
			return;
			}
		if (IS_MCLASSED(ch) && skill_table[sn].rating[ch->ch_class2] < 1 && skill_table[sn].rating[ch->ch_class] < 1)
		{
			SEND ("You wouldn't be able to learn that spell anyway.\r\n",ch);
			return;
		}
		
		if (!IS_MCLASSED(ch) && skill_table[sn].rating[ch->ch_class] < 1 )
		{
			SEND ("You wouldn't be able to learn that spell anyway.\r\n",ch);
			return;
		}
		
		if ( number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
		{
			SEND("You cannot glean any knowledge from it.\r\n",ch);
			check_improve(ch,gsn_scrolls,FALSE,3);
			act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
			extract_obj( obj );			
			return;
		}
		
		if ( IS_KNOWN(ch,sn) )
		{
			if (get_skill(ch, sn) >= 75)
			{
				SEND ("You've already learned what you can from reading... practice casting it now!\r\n",ch);
				return;
			}			
		}
		
		if (!IS_KNOWN(ch,sn))
		{
			ch->pcdata->learned[sn] = 1;
			act("You have learned the art of $t!",ch,skill_table[sn].name,NULL,TO_CHAR);
			check_improve(ch,gsn_scrolls,TRUE,2);
		}
		else
		{
			ch->pcdata->learned[sn] += number_range(5,15);
			act("You have improved your knowledge in the art of $t!",ch,skill_table[sn].name,NULL,TO_CHAR);
			check_improve(ch,gsn_scrolls,TRUE,2);
		}
				
		act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
		extract_obj( obj );
		return;
	}

}

void do_vault( CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *banker;
	ACCOUNT_DATA *acc;
	
	char buf  [MSL];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
	
	if (IS_NPC(ch))
		return;
	
	for ( banker = ch->in_room->people; banker != NULL; banker = banker->next_in_room)
    {
        if (!IS_NPC(banker))
            continue;
        if (banker->spec_fun == spec_lookup( "spec_banker" ))
            break;
    }
    if (banker == NULL || banker->spec_fun != spec_lookup("spec_banker"))
    {
        SEND("You don't see any bankers here, what were you thinking?\r\n",ch);
        return;
    }
	
	//Vault stuff.
	if (!str_cmp(arg1, "withdraw"))
    {
		return;
	}
	
	if (!str_cmp(arg1, "deposit"))
	{
		return;
	}
	
	//No initial argument just shows what's in the vault.	
	
    SEND ("In your vault:\r\n", ch);
    show_list_to_char (acc->vault, ch, TRUE, TRUE, FALSE);
	
	/*sprintf (buf,
             "\r\nYou are carrying %s%d{x/%d items with weight %s%ld{x/%d pounds.\r\n",
             (ch->carry_number > (can_carry_n (ch) / 10) * 8) ? "{R" : "",				
			 ch->carry_number, 			 
			 can_carry_n (ch),
			 (get_carry_weight(ch) > (can_carry_w (ch) / 10) * 8) ? "{R" : "",
             get_carry_weight (ch) / 10, 
			 can_carry_w (ch) / 10);
    SEND (buf, ch);*/
	
	return;
}

void do_bank( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    int gold_amt = 0;
    int fee 	 = 0;
    char buf  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );

    if (IS_NPC(ch))
        return;
    if ( arg1[0] == '\0' )
    {
		SEND("Syntax: bank <withdraw/open/deposit/statement>\r\n", ch);
		return;
    }

    for ( banker = ch->in_room->people; banker != NULL; banker = banker->next_in_room)
    {
        if (!IS_NPC(banker))
            continue;
        if (banker->spec_fun == spec_lookup( "spec_banker" ))
            break;
    }
    if (banker == NULL || banker->spec_fun != spec_lookup("spec_banker"))
    {
        SEND("You don't see any bankers here, what were you thinking?\r\n",ch);
        return;
    }
    if (!str_cmp(arg1, "open"))
    {
		if (ch->gold < 100)
		{
			SEND ("You need 100 gold to start an account.\r\n",ch);
			return;
		}
		if (ch->bank_act == 1)
		{
			SEND("You already have a bank account.\r\n",ch);
			return;
		}
	
	    ch->gold -= 100;
	    ch->bank_act = 1;
	    ch->bank_amt = 10;
	    SEND ("You open a bank account.\r\n",ch);
	    return;
	
    }
    if (!str_cmp(arg1, "statement"))
    {
        if (ch->bank_act == 0)
        {
			SEND("You don't have an open account yet.\r\n", ch);
			return;
		}
		sprintf(buf,
			"You currently have %ld {ygold{x pieces in your bank account.\r\n",
				ch->bank_amt );
		SEND("________________________________________________________\r\n",ch);
		SEND("\r\n",ch);
			SEND(buf,ch);
		SEND("\r\n",ch);
		SEND("________________________________________________________\r\n",ch);
		return;
    }

    if (!str_cmp(arg1, "withdraw"))
    {
		if (ch->bank_act == 0)
			{
				SEND("You don't have an open account yet.\r\n", ch);
				return;
			}
		if ( arg2[0] == '\0' )
			{
			SEND("Please specify an amount.\r\n", ch);
			return;
		}	
			if ( is_number( arg2 ) )
			{
				gold_amt = atoi(arg2);
			if (gold_amt < 0)
			gold_amt = 0;
			fee = (gold_amt / 25);
			if (ch->bank_amt < (gold_amt + fee))
			{
			SEND("You don't have that much in the bank!\r\n",ch);
			sprintf(buf,
					  "({cCurrent Available Funds): %ld {ygold{x pieces.\r\n",
					(ch->bank_amt - (ch->bank_amt / 25)) );
					SEND(buf,ch);
			return;
			}
				if ( gold_amt/5 + ch->carry_weight > can_carry_w(ch) )
				{
					SEND( "You can't carry that much weight.\r\n", ch );
					return;
				}
			if (gold_amt > 10000)
				{
					SEND("You may only withdraw so much at once!\r\n",ch);
					return;
				}
			else
			{
			ch->gold += gold_amt;
			ch->bank_amt -= (gold_amt + fee);
			sprintf(buf,
					  "You withdraw %d {ygold{x pieces from your bank account.\r\n",
					gold_amt );
					SEND(buf,ch);
				sprintf(buf,
					  "This transaction incurred a fee of %d {ygold{x pieces.\r\n",
					fee );
					SEND(buf,ch);
			return;
			}
		
		}
    }

    if (!str_cmp(arg1, "deposit"))
    {
	if (ch->bank_act == 0)
        {
            SEND("You don't have an open account yet.\r\n", ch);
            return;
        }
	if ( arg2[0] == '\0' )
        {
            SEND("Please specify an amount.\r\n", ch);
            return;
        }
        if ( is_number( arg2 ) )
        {
            gold_amt = atoi(arg2);
	    if (gold_amt < 0)
		gold_amt = 0;
            if (ch->bank_amt + gold_amt > 50000)
            {
                SEND("Bank accounts may not exceed 50,000 gold pieces.\r\n",ch);
                return;
            }
	    if (gold_amt > 10000)
	    {
		SEND("You may only deposit so much at once!\r\n",ch);
		return;
	    }
	    if (ch->gold < gold_amt)
	    {
		SEND("You don't have that much gold!\r\n",ch);
		return;
	    }
            else
            {
                ch->gold -= gold_amt;
                ch->bank_amt += gold_amt;
		sprintf(buf,
        	  "You deposit %d {ygold{x pieces into your bank account.\r\n",
            	gold_amt );
		SEND(buf,ch);
                return;
            }

        }

    }
	
	SEND("That's not a valid banking command, see help bank.\r\n",ch);
	return;
}    


void do_dual (CHAR_DATA *ch, char *argument)
/* wear object as a secondary weapon */
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH]; /* overkill, but what the heck */

	if (!IS_KNOWN(ch, gsn_dual_wield))
	{
		SEND ("You have no idea what you're doing.\r\n",ch);
		return;
	}
	
    if (argument[0] == '\0') /* empty */
    {
        SEND ("Wear which weapon in your off-hand?\r\n",ch);
        return;
    }

    obj = get_obj_carry (ch, argument, ch); /* find the obj withing ch's inventory */

    if (obj == NULL)
    {
        SEND ("You have no such thing in your backpack.\r\n",ch);
        return;
    }


    /* check if the char is using a shield or a held weapon */

    if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
         (get_eq_char (ch,WEAR_HOLD)   != NULL) )
    {
        SEND ("You cannot use a secondary weapon while using a shield or holding an item.\r\n",ch);
        return;
    }

	if (obj->item_type != ITEM_WEAPON ) 
	{
		SEND("You can only dual wield weapons.\r\n",ch);
		return;
	}
	
    if ( total_levels(ch) < (obj->level - 2) )
    {
        sprintf( buf, "You must be level %d to use this object.\r\n",
            obj->level );
        SEND( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
    }

/* check that the character is using a first weapon at all */
    if (get_eq_char (ch, WEAR_WIELD) == NULL) /* oops - != here was a bit wrong :) */
    {
        SEND ("You need to wield a primary weapon, before using a secondary one!\r\n",ch);
        return;
    }

/* check for str - secondary weapons have to be lighter */
    if ( get_obj_weight( obj ) > ( str_app[get_curr_stat(ch, STAT_STR)].wield) && ch->size < SIZE_HUGE)
    {
        SEND( "This weapon is too heavy to be used as a secondary weapon by you.\r\n", ch );
        return;
    }

    if ( (get_obj_weight (obj) * 4) / 3 > get_obj_weight(get_eq_char(ch,WEAR_WIELD)) && ch->size < SIZE_LARGE )
    {
		if (!IS_RANGER(ch))
		{
			SEND("Your secondary weapon has to be considerably lighter than the primary one.\r\n",ch);
			return;
		}
    }

/* at last - the char uses the weapon */

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
        return;                                /* remove obj tells about any no_remove */

/* char CAN use the item! that didn't take long at aaall */

    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);
    return;
}


void do_skin(CHAR_DATA *ch, char *argument)
{
	char arg1[MSL];	
	OBJ_DATA *skin, *obj;
	char buf[MSL];
	 
	
	//skinning also runs off the butcher skill:
	if(!get_skill(ch,gsn_butcher) || IS_NPC(ch))
	{
		printf_to_char(ch, "You do not have the knowledge to skin a corpse.\r\n");
		return;
	}
	
	one_argument(argument, arg1);

	if(arg1[0]=='\0')
	{
		printf_to_char(ch, "What would you like to skin?\r\n");
		return;
	}
	/* check if the corpse is in the room */
	obj = get_obj_list( ch, arg1, ch->in_room->contents );
	 

	/* If no corpse found */

	if ( obj == NULL )
	{
		printf_to_char(ch, "You don't see any corpse like that.\r\n");
		return;
	}
	 
	/* check item types for corpse NPC */

	if(obj->item_type != ITEM_CORPSE_NPC)
	{
		SEND( "You can only skin the corpses of NPCs.\r\n", ch );
		return;
	}	
	 
	if (WAS_ANGELIC(obj) || WAS_UNDEAD(obj))
	{
		SEND("You cannot skin that corpse.\r\n",ch);
		return;
	}	
	
	if ( obj->contains != NULL )
    {
        SEND( "You can't skin it while it's full!\r\n",ch);
        return;   
    }
	 
	/* Check the skill roll. */

	//gnolls are good at it.
	if(number_percent( ) < (IS_GNOLL(ch) ? get_skill(ch,gsn_butcher) + 10 : get_skill(ch,gsn_butcher)))
	{	
		/* Create the skin and place it in the room */
		skin = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 );	
		
		sprintf(buf, "the skin of %s %s", IS_VOWEL(race_table[obj->corpse_race].name[0]) ? "an" : "a", race_table[obj->corpse_race].name);
		free_string(skin->short_descr);
		skin->short_descr = str_dup(buf);
		
		free_string (skin->description);
		skin->description = str_dup (buf);
		
		skin->cost = 250;
		
		if (WAS_BIPED(obj))
			skin->cost *= 1.25;
		if (WAS_DRAGON(obj))
			skin->cost *= 20;
		if (WAS_GIANT(obj))
			skin->cost *= 4;
		
		obj_to_room( skin, ch->in_room );
		act( "$n expertly skins a corpse.", ch, NULL, NULL, TO_ROOM );
		SEND("You successfully skin the corpse.\r\n",ch);
	}
	else
	{	
		act( "$n botches a corpse with $s knife.\r\n", ch, NULL, NULL, TO_ROOM );
		printf_to_char(ch, "You try to skin the corpse, but botch the job.\r\n");
	}
	
	check_improve(ch,gsn_skin,TRUE,3);	
	extract_obj(obj); //remove corpse
	WAIT_STATE( ch, PULSE_VIOLENCE );
	return;
}


void do_butcher(CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	char arg1[MSL];
	int number_of_steaks = 0, sLoop;
	OBJ_DATA *steak, *obj;
	 
	/* Check if they have the skill butcher */

	if(!get_skill(ch,gsn_butcher) || IS_NPC(ch))
	{
	printf_to_char(ch, "You do not khave the knowledge to butcher a corpse.\r\n");
	return;
	}
	
	one_argument(argument, arg1);

	if(arg1[0]=='\0')
	{
	printf_to_char(ch, "What would you like to butcher?\r\n");
	return;
	}
	/* check if the corpse is in the room */
	obj = get_obj_list( ch, arg1, ch->in_room->contents );
	 

	/* If no corpse found */

	if ( obj == NULL )
	{
	printf_to_char(ch, "You don't see any corpse like that.\r\n");
	return;
	}
	 
	/* check item types for corpse NPC */

	if(obj->item_type != ITEM_CORPSE_NPC)
	{
	SEND( "You can only butcher corpses of NPCs.\r\n", ch );
	return;
	}	
	 
	if ( obj->contains != NULL )
    {
        SEND( "You can't butcher it while it's full!\r\n",ch);
        return;   
    }
	 
	/* Check the skill roll. */

	if(number_percent( ) < (IS_GNOLL(ch) ? get_skill(ch,gsn_butcher) + 10 : get_skill(ch,gsn_butcher)))
	{
	/* Allow 1 to 6 Steaks */

	number_of_steaks = dice(1,6);
	for(sLoop = 0; sLoop < number_of_steaks; sLoop++)
	{
		/* Create the steak and place it in the room */
		steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );	
		steak->cost = 150;				
		steak->value[0] = total_levels(ch) / 2;
		steak->value[1] = total_levels(ch);
		obj_to_room( steak, ch->in_room );
	}
	/* build message to room */

	buf[0]='\0';
	strcat(buf, "$n expertly butchers a corpse and creates ");
	if(number_of_steaks < 2)
	strcat(buf, "a steak");
	else
	strcat(buf, "some steaks");
	act( buf, ch, NULL, NULL, TO_ROOM );
	/* message to butcher */

	printf_to_char(ch, "You butcher the corpse and create %s %s.\r\n",
	number_of_steaks == 1 ? "a" : number_of_steaks == 2 ? "two" : number_of_steaks == 3 ? "three" :
	number_of_steaks == 4 ? "four" : number_of_steaks == 5 ? "five" : "six",
	number_of_steaks < 2 ? "steak" : "steaks");
	}
	else
	{
	/* if failed */
	act( "$n destroys a corpse with $s knife.\r\n", ch, NULL, NULL, TO_ROOM );
	printf_to_char(ch, "You fail to butcher the corpse!\r\n");
	}
	
	check_improve(ch,gsn_butcher,TRUE,3);	
	extract_obj(obj); //remove corpse
	WAIT_STATE( ch, PULSE_VIOLENCE );
	return;

}


bool has_thieves_tools(CHAR_DATA *ch)
{
	OBJ_DATA *obj;	
	bool found = FALSE;
	
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
		if (obj->item_type == ITEM_THIEVES_TOOLS && can_see_obj (ch, obj))                    
        {
			found = TRUE;            
			break;
        }
    }
		
	return found;
}

void do_traps (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;    
	int chance = 0;
	char arg1[MSL];
	char arg2[MSL];
	char arg3[MSL];
	int trap_type = -1;
	
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );	
	argument = one_argument( argument, arg3 );
	
	if (arg1[0] == '\0')
    {
        SEND("Syntax: trap <set/remove> <object>", ch);
        return;
    }
	
	if (arg2[0] == '\0')
	{
		SEND("Specify an object.\r\n",ch);
		return;
	}
	
	if ((obj = get_obj_here( ch, NULL, arg2)) == NULL || !can_see_obj(ch,obj))
	{
		SEND("You don't see that object here.\r\n",ch);
		return;
	}
	
	if (!has_thieves_tools(ch))
	{
		SEND("You need some sort of tool to do that!\r\n",ch);
		return;
	}
	
	if (!str_cmp(arg1, "remove") || !str_cmp(arg1, "rem"))
	{
		if (can_see_obj (ch, obj))
		{			
			if (!IS_KNOWN(ch,gsn_remove_trap))
			{
				SEND("You don't know how to remove traps.\r\n",ch);
				return;
			}				
						
			if (ch->cooldowns[gsn_remove_trap] > 0)
			{
				SEND("You must wait to use this skill again.\r\n",ch);
				return;
			}
						
			if (!is_trapped(obj))
			{
				SEND("That item doesn't appear to have any trap on it.\r\n",ch);
				return;
			}
						
			chance = number_percent();
			if (obj->level > total_levels(ch))
				chance += obj->level - total_levels(ch) * 2;
			if (get_curr_stat(ch, STAT_DEX) > 15)
				chance -= 25 - get_curr_stat(ch, STAT_DEX);
			if (chance < get_skill(ch,gsn_remove_trap))
			{
				remove_trap(obj);
				act( "$n successfully removes the trap from $p.\r\n", ch, obj, NULL, TO_ROOM );
				act( "You successfully remove the trap from $p.\r\n", ch, obj, NULL, TO_CHAR );	
				int xp = 0;				
				xp = number_range(100,150);
				if (total_levels(ch) > 5)
				{
					xp += (total_levels(ch) / 2) * number_range(25,35);
				}
				
				give_exp(ch, xp, TRUE);				
				check_improve(ch,gsn_remove_trap,TRUE,3);	
				return;								
			}
			else
			{				
				act( "$n failed to remove the trap from $p.\r\n", ch, obj, NULL, TO_ROOM );
				act( "You failed to remove the trap from $p.\r\n", ch, obj, NULL, TO_CHAR );
				if (number_percent() < 25)
					spring_trap(ch, obj);
				return;								
			}
		}
	
	}	
	else if (!str_cmp(arg1, "set") || !str_cmp(arg1, "lay"))
	{
		if(!IS_KNOWN(ch,gsn_lay_trap))
		{
			SEND("You don't know how to lay traps.\r\n",ch);
			return;
		}				

		if (obj->item_type != ITEM_CONTAINER)
		{
			SEND ("That's not a container.\r\n", ch);
			return;
		}
		
		if (ch->cooldowns[gsn_lay_trap] > 0)
		{
			SEND("You must wait to use this skill again.\r\n",ch);
			return;
		}
		
		if (is_trapped(obj))
		{
			SEND("That item already has a trap on it.\r\n",ch);
			return;
		}
		
		if (arg3[0] == '\0')
		{
			SEND("What kind of trap will you lay?\r\n", ch);
			return;
		}
		
		if (!str_cmp(arg3, "poison"))
			trap_type = POISON_TRAP;
		if (!str_cmp (arg3, "dart"))
			trap_type = DART_TRAP;
		if (!str_cmp (arg3, "gas"))
			trap_type = GAS_TRAP;
		
		
		if (trap_type != POISON_TRAP && trap_type != GAS_TRAP && trap_type != DART_TRAP)				
		{
			SEND("Invalid trap type.\r\n",ch);
			return;
		}
		
		chance = number_percent();
		if (obj->level > total_levels(ch))
			chance += obj->level - total_levels(ch) * 2;
		if (get_curr_stat(ch, STAT_DEX) > 15)
			chance -= 25 - get_curr_stat(ch, STAT_DEX);
		if (chance < get_skill(ch,gsn_lay_trap))
		{
			lay_trap(obj, trap_type);
			act( "$n successfully places a trap on $p.\r\n", ch, obj, NULL, TO_ROOM );
			act( "You successfully place a trap on $p.\r\n", ch, obj, NULL, TO_CHAR );
			int xp = 0;				
			xp = number_range(100,150);
			if (total_levels(ch) > 5)
			{
				xp += (total_levels(ch) / 2) * number_range(25,35);
			}
			
			give_exp(ch, xp, TRUE);				
			check_improve(ch,gsn_lay_trap,TRUE,3);	
			return;								
		}
		else if (chance > get_skill(ch,gsn_lay_trap) && (number_percent() < 75))
		{				
			act( "$n failed to place a trap on $p.\r\n", ch, obj, NULL, TO_ROOM );
			act( "You failed to place a trap on $p.\r\n", ch, obj, NULL, TO_CHAR );				
			return;								
		}
		else
		{
			act( "$n failed to place a trap on $p.\r\n", ch, obj, NULL, TO_ROOM );
			act( "You failed to place a trap on $p.\r\n", ch, obj, NULL, TO_CHAR );				
			act( "$n failed to place a trap on $p, springing it on themself!\r\n", ch, obj, NULL, TO_ROOM );
			act( "You failed to place a trap on $p, springing it on yourself!\r\n", ch, obj, NULL, TO_CHAR );
			lay_trap(obj, trap_type);
			spring_trap(ch, obj);
			return;								
		}
	}
    
}


void do_recharge (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *recharger;
	char arg1[MSL];
	
	one_argument(argument, arg1);
	
	if (arg1[0] == '\0')
    {
        SEND("Recharge what?\r\n", ch);
        return;
    }
		
	if ((obj = get_obj_here( ch, NULL, arg1)) == NULL)
	{
		SEND("You don't see that object here.\r\n",ch);
		return;
	}
	
	for ( recharger = ch->in_room->people; recharger != NULL; recharger = recharger->next_in_room )
    {
        if (!IS_NPC(recharger))
			continue;
        if (recharger->spec_fun == spec_lookup( "spec_recharge" ))			
            break;
    }
		
	if (obj->item_type != ITEM_FIGURINE && obj->item_type != ITEM_WAND)
	{
		SEND("Object is not rechargeable.\r\n",ch);
		return;
	}
		
	if (obj->condition > 99)
	{
		act( "$p is already fully recharged.\r\n", ch, obj, NULL, TO_CHAR );	
		return;
	}
	
	if (recharger)
	{	
		if (IS_SET(obj->extra2_flags, ITEM_NO_RECHARGE))
		{
			SEND("You can't recharge that item.\r\n",ch);
			return;
		}
		
		if ( (ch->gold * 100) + ch->silver < 500 )
		{
			act( "You cannot afford to recharge a $p.\r\n", ch, obj, NULL, TO_CHAR );
			return;
		}
		deduct_cost (ch, 500);		
		act( "$n manages to slightly recharge $p.",recharger,obj,NULL,TO_ROOM);
		obj->condition += 10;		
		if (obj->condition >= 100)
		{
			obj->condition = 100;
			act( "$p appears to be fully recharged!\r\n", ch, obj, NULL, TO_CHAR );				
		}
		return;
	}
	else
	{
		SEND("You need someone to do that for you.\r\n",ch);
		return;
	}
	return;
}

void do_repair (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *smith;
	char arg1[MSL];
	char buf[MSL];
	
	
	one_argument(argument, arg1);
	
	for ( smith = ch->in_room->people; smith != NULL; smith = smith->next_in_room )
    {
        if (!IS_NPC(smith))
			continue;
        if (smith->spec_fun == spec_lookup( "spec_repair" ))			
            break;
    }
	
	if(get_skill(ch,gsn_repair) < 1 && !smith)
	{
		SEND("You don't know how to repair items.\r\n",ch);
		return;
	}				
	
	
	if (arg1[0] == '\0')
    {
        SEND("Repair what?\r\n", ch);
        return;
    }
	
	if (!str_cmp(arg1, "all"))
	{
		int total_cost = 0;
		int i = 0;
		
		for ( i = 0; i < MAX_WEAR; i++ )
		{
			if ( ( obj = get_eq_char( ch, i ) ) == NULL )
				continue;
			
			if (obj && obj->condition < 99)
			{
				total_cost += obj->level * 10;				
			}		
		}	
		
		
		if ( (ch->gold * 100) + ch->silver < total_cost )
		{
			SEND( "You cannot afford to repair your equipment.\r\n", ch);
			return;
		}
		deduct_cost (ch, total_cost);		
		sprintf(buf, "%s manages to slightly repair all of your equipment, how amazing.\r\n",smith->short_descr);
		SEND(buf, ch);
		act ("$n gets his equipment repaired.", ch, NULL, NULL, TO_NOTVICT);
		WAIT_STATE (ch, skill_table[gsn_repair].beats);
		
		for ( i = 0; i < MAX_WEAR; i++ )
		{
			if ( ( obj = get_eq_char( ch, i ) ) == NULL )
				continue;
			if (obj && obj->condition < 99)
			{
				obj->condition += 10;		
				if (obj->condition >= 100)
				{
					obj->condition = 100;
					act( "\r\n$p appears to be fully repaired!", ch, obj, NULL, TO_CHAR );				
				}
			}
		}
		return;
	}
	
	
	if ((obj = get_obj_here( ch, NULL, arg1)) == NULL)
	{
		SEND("You don't see that object here.\r\n",ch);
		return;
	}
	
	if (obj->condition > 99)
	{
		act( "$p is already fully repaired.\r\n", ch, obj, NULL, TO_CHAR );	
		return;
	}
	
	if (smith)
	{	
		if ( (ch->gold * 100) + ch->silver < (obj->level * 10) )
		{
			act( "You cannot afford to repair a $p.\r\n", ch, obj, NULL, TO_CHAR );
			return;
		}
		deduct_cost (ch, obj->level * 10);		
		act( "$n manages to slightly repair $p.",smith,obj,NULL,TO_ROOM);
		obj->condition += 10;		
		if (obj->condition >= 100)
		{
			obj->condition = 100;
			act( "$p appears to be fully repaired!\r\n", ch, obj, NULL, TO_CHAR );				
		}
		return;
	}
	
	if (ch->move < 6)
	{
		SEND("You are too exhausted!\r\n",ch);
		return;
	}
	
	if (number_percent() < get_skill(ch, gsn_repair))
	{	
		act( "You slightly repair $p.\r\n", ch, obj, NULL, TO_CHAR );	
		act( "$n manages to slightly repair $p.",ch,obj,NULL,TO_ROOM);
		obj->condition += number_range(5,10);
		if (!IS_IMMORTAL(ch))
			WAIT_STATE (ch, skill_table[gsn_repair].beats);
		check_improve(ch,gsn_repair,TRUE,2);	
		if (obj->condition >= 100)
		{
			obj->condition = 100;
			act( "$p appears to be fully repaired!\r\n", ch, obj, NULL, TO_CHAR );	
		}
	}
	else
	{
		act( "You fail to repair $p.\r\n", ch, obj, NULL, TO_CHAR );	
		act( "$n seems frustrated, trying to repair $p.",ch,obj,NULL,TO_ROOM);
		if (number_percent() < 50)
		{
			obj->condition -= number_range(1,3);
			act( "$p seems even worse now!\r\n", ch, obj, NULL, TO_CHAR );	
			if (obj->condition < 1)
			{
				act( "$p shatters, showering the immediate area with sparks and debris!\r\n", ch, obj, NULL, TO_ROOM );
				extract_obj(obj);
			}
		}
		check_improve(ch,gsn_repair,FALSE,1);
	}
	ch->move -= 6;
	
	return;
	
}



int grass_herb ()
{

	int herb = -1;
	int rarity_seed = 0;
	int rarity = -1;

	rarity_seed = number_range(1,100);
	if (rarity_seed < 40)
		rarity = VERY_COMMON;
	else if (rarity_seed > 39 && rarity_seed < 70)
		rarity = COMMON;
	else if (rarity_seed > 69 && rarity_seed < 90)
		rarity = UNCOMMON;
	else
		rarity = RARE;
		
	switch (rarity)
	{
		default: 			herb = -1; 						break;
		case VERY_COMMON:	herb = number_range(0,1);		break;
		case COMMON:		herb = number_range(2,4);		break;
		case UNCOMMON:		herb = number_range(5,10);		break;
		case RARE:			herb = 11;						break;
	}
	
	return herb;
}


int water_herb ()
{

	int herb = -1;
	int rarity_seed = 0;
	int rarity = -1;

	rarity_seed = number_range(1,100);
	if (rarity_seed < 65)
		rarity = COMMON;
	else
		rarity = UNCOMMON;
		
	switch (rarity)
	{
		default: 			herb = -1; 						break;		
		case COMMON:		herb = 12;						break;
		case UNCOMMON:		herb = number_range(13,15);		break;
		
	}
	
	return herb;
}


int forest_herb ()
{

	int herb = -1;
	int rarity_seed = 0;
	int rarity = -1;

	rarity_seed = number_range(1,100);
	if (rarity_seed < 50)
		rarity = COMMON;
	else if (rarity_seed > 49 && rarity_seed < 85)
		rarity = UNCOMMON;
	else 
		rarity = RARE;	
		
	switch (rarity)
	{
		default: 			herb = -1; 						break;
		case COMMON:		herb = number_range(16,21);		break;
		case UNCOMMON:		herb = number_range(22,28);		break;
		case RARE:			herb = number_range(29,32);		break;
		
	}
	
	return herb;
}


int swamp_herb ()
{

	int herb = -1;
	int rarity_seed = 0;
	int rarity = -1;

	rarity_seed = number_range(1,100);
	if (rarity_seed < 50)
		rarity = COMMON;
	else if (rarity_seed > 49 && rarity_seed < 80)
		rarity = UNCOMMON;
	else if (rarity_seed > 79 && rarity_seed < 95)
		rarity = RARE;
	else
		rarity = VERY_RARE;
		
	switch (rarity)
	{
		default: 			herb = -1; 						break;
		case COMMON:		herb = 33;						break;
		case UNCOMMON:		herb = 34;						break;
		case RARE:			herb = number_range(35,36);		break;
		case VERY_RARE:		herb = number_range(37,38);		break;
	}
	
	return herb;
}


int desert_herb ()
{

	int herb = -1;
	int rarity_seed = 0;
	int rarity = -1;

	rarity_seed = number_range(1,100);
	if (rarity_seed < 85)
		rarity = VERY_COMMON;
	else
		rarity = RARE;
		
	switch (rarity)
	{
		default: 			herb = -1; 						break;		
		case VERY_COMMON:	herb = 39;						break;
		case RARE:			herb = 40;						break;
		
	}
	
	return herb;
}


int mountain_herb ()
{

	int herb = -1;
	int rarity_seed = 0;
	int rarity = -1;

	rarity_seed = number_range(1,100);
	if (rarity_seed < 65)
		rarity = COMMON;
	else
		rarity = UNCOMMON;
		
	switch (rarity)
	{
		default: 			herb = -1; 						break;		
		case COMMON:		herb = number_range(41,46);		break;
		case UNCOMMON:		herb = 47;						break;
		
	}
	
	return herb;
}

void do_forage(CHAR_DATA *ch, char *argument)
{
	
	OBJ_DATA *obj; 	
	int chance = 0;
	int herb = -1;
	char buf[MSL];
	
	
	chance = number_percent();
	
	if(get_skill(ch,gsn_forage) < 1)
	{
		SEND("You don't know how to forage.\r\n",ch);
		return;
	}
	
	
	switch (ch->in_room->sector_type)
	{
		default:		
			break;
		case SECT_FIELD:
		case SECT_SNOW:		
			herb = grass_herb(); 		break;		
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		case SECT_OCEANFLOOR:
			herb = water_herb();		break;			
		case SECT_FOREST:
			herb = forest_herb();		break;			
		case SECT_SWAMP:
			herb = swamp_herb();		break;			
		case SECT_DESERT:
			herb = desert_herb();		break;		
		case SECT_HILLS:
		case SECT_MOUNTAIN:			
			herb = mountain_herb();		break;
	}

	if (herb > -1 && ch->in_room->can_forage == TRUE)
	{
		if (chance < get_skill(ch, gsn_forage))
		{
			act("$n forages around, finding something.\r\n",ch,NULL,NULL,TO_NOTVICT);
			sprintf (buf, "You found a bit of %s.\r\n", herb_table[herb].name);
			SEND(buf, ch);
			obj = create_object (get_obj_index (OBJ_VNUM_HERB), 0);
			obj->level = total_levels(ch) / 2;					
			obj->cost = number_fuzzy(total_levels(ch) * 100);
			sprintf (buf, "a bit of %s", herb_table[herb].name);
			obj->short_descr = str_dup(buf);
			sprintf (buf, "%s herb", herb_table[herb].name);
			obj->name = str_dup(buf);
			obj_to_char ( obj, ch );
			check_improve (ch, gsn_forage, TRUE, 4);
			ch->in_room->can_forage = FALSE;
			WAIT_STATE(ch,skill_table[gsn_forage].beats);
			return;
		}
		else
		{
			act("$n forages around, destroying everything in their path.\r\n",ch,NULL,NULL,TO_NOTVICT);
			SEND("You botched the job, destroying the plant.\r\n",ch);
			ch->in_room->can_forage = FALSE;
			int xp = 0;				
			xp = number_range(25, 50);
			if (total_levels(ch) > 5)
			{
				xp += (total_levels(ch) / 2) * number_range(25,35);
			}
			
			give_exp(ch, xp, TRUE);
				
			WAIT_STATE(ch,skill_table[gsn_forage].beats);
			check_improve (ch, gsn_forage, TRUE, 2);
			return;
		}
	}
	else
	{
		act("$n forages around unsuccessfully.\r\n",ch,NULL,NULL,TO_NOTVICT);
		SEND("You couldn't find anything useful.\r\n",ch);		
		WAIT_STATE(ch,skill_table[gsn_forage].beats);
		return;
	}
}

void do_mine(CHAR_DATA *ch, char *argument)
{
	char arg1 [MIL];
	int i;	
	bool found = FALSE;
	OBJ_DATA * tool = NULL;

	if (IS_NPC(ch))
		return;

	argument = one_argument( argument, arg1 );
		
	if (!get_skill(ch,gsn_mine))
    {
        SEND( "You have no idea what you're doing.\r\n",ch );
        return;
    }
	
	if (IS_AFFECTED(ch,AFF_CHARM))
	{
		SEND( "You don't have the free will to do so!\r\n",ch );
		return;
	}
	
	for ( i = 0; i < MAX_WEAR; i++ )
	{
		if ( ( tool = get_eq_char( ch, i ) ) == NULL )
			continue;
		
		if (tool && tool->item_type == ITEM_MINING_TOOL)
		{
			found = TRUE;
			break;
		}		
	}	
	
	if ( arg1[0] == '\0' )
    {
		if (ch->event == EVENT_NONE)
		{
			if (!IN_MINING_SECTOR(ch))
			{
				SEND("It's not possible to mine here!\r\n",ch);
				return;
			}
			if (!found)
			{
				SEND("You need something to mine with first!\r\n",ch);
				return;
			}
			ch->event = EVENT_MINING;
			SEND("You begin mining.\r\n",ch);
			return;
		}
		else
		{
			SEND("You're already in the middle of something!\r\n",ch);
			return;
		}
    }

	if (!str_cmp (arg1, "stop"))
	{
		if (ch->event != EVENT_MINING)
		{
			SEND("You weren't mining to begin with!\r\n",ch);
			return;
		}
		else
		{
			stop_event(ch);				
			return;
		}
	}
	
	return;
}


void do_prospect(CHAR_DATA *ch, char *argument)
{
	char arg1 [MIL];
	int i;	
	bool found = FALSE;
	OBJ_DATA * tool = NULL;

	if (IS_NPC(ch))
		return;

	argument = one_argument( argument, arg1 );
		
	if (!get_skill(ch,gsn_prospecting))
    {
        SEND( "You have no idea what you're doing.\r\n",ch );
        return;
    }
	
	if (IS_AFFECTED(ch,AFF_CHARM))
	{
		SEND( "You don't have the free will to do so!\r\n",ch );
		return;
	}
	
	for ( i = 0; i < MAX_WEAR; i++ )
	{
		if ( ( tool = get_eq_char( ch, i ) ) == NULL )
			continue;
		
		if (tool && tool->item_type == ITEM_SIEVE)
		{
			found = TRUE;
			break;
		}		
	}	
	
	if ( arg1[0] == '\0' )
    {
		if (ch->event == EVENT_NONE)
		{
			if (!IN_PROSPECTING_SECTOR(ch))
			{
				SEND("It's not possible to prospect here!\r\n",ch);
				return;
			}
			if (!found)
			{
				SEND("You need something to prospect with first!\r\n",ch);
				return;
			}
			ch->event = EVENT_PROSPECTING;
			SEND("You begin prospecting for precious rocks that you're sure exist.\r\n",ch);
			return;
		}
		else
		{
			SEND("You're already in the middle of something!\r\n",ch);
			return;
		}
    }

	if (!str_cmp (arg1, "stop"))
	{
		if (ch->event != EVENT_PROSPECTING)
		{
			SEND("You weren't prospecting to begin with!\r\n",ch);
			return;
		}
		else
		{
			stop_event(ch);				
			return;
		}
	}
	
	return;
}

void do_plant( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if ( IS_NPC(ch))
        return;

    if (!get_skill(ch,gsn_plant))
    {
        SEND( "You have no idea what you're doing.\r\n",ch );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        SEND( "Plant what on whom?\r\n", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
        SEND( "They aren't here.\r\n", ch );
        return;
    }

    if ( (IS_PET( victim )) && victim->master == ch )
    {
        SEND( "You can't plant objects on your own pets.\r\n",ch );
        return;
    }
    
    if ( IS_IMMORTAL(victim))
    {
        SEND("Riiiiiight. Do you have a death wish?\r\n",ch);
        act("You have the sudden urge to smite $n.\r\n",ch,NULL,victim,TO_VICT);
        return;
    }
    
    if ( !IS_NPC(victim) && !IS_NPC(ch) && total_levels(ch) - victim->level > 8 )
    {  
        SEND("Pick on someone your own size.\r\n",ch);
        return;
    }
    
    if ( !can_see( ch, victim ) )
    {
        SEND( "Plant what on whom?\r\n",ch);
        return;
    }
    
    if ( victim->level - total_levels(ch) > 16 )
    {  
        SEND("They're too wise to your tricks.\r\n",ch);
        return;
    }
    
    if ( victim == ch )
    {
        SEND( "That's pointless.\r\n", ch );
        return;
    }
    
    if (IS_SET(victim->in_room->room_flags,ROOM_NOFIGHT) || is_safe(ch,victim))
    {
        SEND("A strange force keeps your hands in your own pockets.\r\n",ch); 
        return;
    }
    
    if ( victim->position == POS_FIGHTING)
    {
        SEND(  "You'd better not -- you might get hit.\r\n",ch );
        return;
    }
    
	/*
    if (IS_SET(victim->in_room->room_flags,ROOM_ARENA))
    {
        SEND("You think twice about pissing the gods off extremely...\r\n",ch); 
        return;
    }   
    */
	
    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
        SEND( "You can't seem to find that thing anywhere.\r\n", ch );
        return;
    }
    
    if ( !can_drop_obj( ch, obj )
        ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
        ||   obj->level > total_levels(ch) )
    {
        SEND( "You can't bear to lose it.\r\n", ch );
        return;
    }
    
    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
        SEND( "They have their hands full.\r\n", ch );
        return;
    }
    
    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
        SEND( "They have their hands full.\r\n", ch );
        return;
    }
    
    percent  = number_percent();
    
    if (!IS_AWAKE(victim))
        percent -= 50;
    else if (!can_see(victim,ch))
        percent += 25;
    else 
        percent += 50;
    if ( ch->race == RACE_HALFLING )
        percent -= 20;    
    
    if ( victim->fighting != NULL )
        percent /= 3;
    
    percent -= ( total_levels(ch) - victim->level ) * 2;
    //percent -= luc_app[get_curr_stat(ch, STAT_LUC)].chance;
    
    percent -= ( get_curr_stat( ch, STAT_DEX ) - 20 ) * 2;
    percent += ( get_curr_stat( victim, STAT_INT ) - 21 ) * 3;
    
    if(get_skill(ch,gsn_plant)<2)
        percent += 50;  /* extra penalty if you don't know how */
    
    WAIT_STATE( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL ? ch->master : ch,
        skill_table[gsn_plant].beats );
            
    if ( obj->item_type == ITEM_CONTAINER )
       //|| obj->item_type == ITEM_QUIVER )
        percent += 20;
    
    if (percent > get_skill( ch, gsn_plant) )
    {
        caught_thief( ch, victim );
        return;
    }

	/*
    if (!IS_NPC( victim ) && !IS_NULLSTR( obj->owner ) && !str_cmp( obj->owner, ch->name ) )
    {
        free_string( obj->owner );
        obj->owner =  str_dup("");
    }
	*/
	
    obj_from_char( obj );
    obj_to_char( obj, victim );
        
    check_improve(ch,gsn_plant,TRUE,2);
    gain_exp(ch,number_range(5,25),FALSE);
    SEND( "Bingo! You plant the item successfully.\r\n", ch );
    return;
}

//Pulled because it never worked right :( - Upro 8/10/2010

/*
void do_morgue( CHAR_DATA *ch, char *argument )
{	
	OBJ_DATA			*obj;
	CHAR_DATA 			*mob;
	
	if ( argument[0] != '\0' )
	{
		SEND("There are no arguments with this command.\r\n",ch);
		return;			
	}
	
	if (ch->in_room == NULL)
		return;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
        if (IS_NPC (mob) && IS_SET (mob->act, ACT_IS_HEALER))
            break;
    }

    if (mob == NULL)
    {
        SEND ("You need a healer to do that!\r\n", ch);
        return;
    }
		
	if (ch->gold < (total_levels(ch) * 2) && ch->silver < (total_levels(ch) * 200))
	{
		SEND("You don't have enough money to cover that.\r\n", ch);
		return;
	}	
	
	for (obj = object_list; obj != NULL; obj = obj->next)
    {
        if (is_name ("corpse", obj->name) && obj->item_type == ITEM_CORPSE_PC)            
		{
			if (!str_cmp (obj->owner, ch->name))
			{
				SEND("You have paid to have your corpse transported to you.\r\n",ch);
				obj_from_room (obj);		
				obj_to_room(obj, ch->in_room);
				if (total_levels(ch) <= 10)
				{
					if (ch->silver >= (total_levels(ch) * 100))
						ch->silver -= (total_levels(ch) *  100);						
					else						
						ch->gold -= (total_levels(ch) * 1);							
				}
				else
				{
					if (ch->silver >= (total_levels(ch) * 200))
						ch->silver -= (total_levels(ch) *  200);						
					else						
						ch->gold -= (total_levels(ch) * 2);		
				}
				return;
			}			
		}
	}
	SEND("You apparently don't have a corpse to be found.\r\n",ch);	
	return;	
}
*/

void do_climb (CHAR_DATA *ch, char * argument)
{
	return;
}


void do_imbue (CHAR_DATA *ch, char * argument)
{
	OBJ_DATA *obj; // what are we imbuing.
	char arg1[MSL];	
	char arg2[MSL];	
	char buf[MSL];
	AFFECT_DATA *paf;
	AFFECT_DATA *affect_free = NULL;
	int value = 0;
	int affect = 0;
	int num_aff = 0;
	int aff_type = 0;
	int chance = 0;
	

	if (IS_NPC(ch))
		return;
	if (!KNOWS(ch, gsn_imbue))
	{
		SEND("You have no idea what you're doing.\r\n",ch);
		return;
	}
	
	if (CHECK_COOLDOWN(ch, gsn_sharpen))
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}

	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0')
    {
        SEND ("Imbue what?\r\n", ch);
        return;
    }

	if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
	{
		SEND("You aren't carrying that.\r\n",ch);
		return;
	}

	if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR)
	{
		SEND("You must imbue only weapons or armor.\r\n",ch);
		return;
	}
	
	if (ch->move < 50 || ch->mana < 100)
	{
		SEND("You don't have the energy to do so at the moment.\r\n",ch);
		return;
	}

	ch->move -= 50;
	ch->mana -= 100;
	COOLDOWN(ch, gsn_imbue);

	for (paf = obj->affected; paf != NULL; paf = paf->next)		
		num_aff++;

	chance = number_percent();

	//Skill modifiers:
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
			chance -= 5;
	if (IS_OBJ_STAT(obj,ITEM_EVIL))
			chance += 5;
	if (IS_WIZARD(ch))
			chance -= 5;
	if (num_aff > 1)
		chance += ((num_aff - 1) * 10); //every existing affect above the 2nd adds 10% chance of failure.

	if (chance > get_skill(ch,gsn_imbue))
	{		
		act ("You fail to imbue $p with magical energy.\r\n",ch, obj, NULL, TO_CHAR);
		act ("$n fails to imbue $p with magical energy.\r\n",ch, obj, NULL, TO_ROOM);		
		check_improve(ch,gsn_imbue,FALSE,1);	
		chance = number_percent();
		if (chance < (10 + (num_aff * 10)))
		{
			act ("$p explodes, showering sparks and debris everywhere!\r\n",ch, obj, NULL, TO_CHAR);
			act ("$p explodes, showering sparks and debris everywhere!\r\n",ch, obj, NULL, TO_ROOM);		
			extract_obj(obj);
		}
		return;
	}

	aff_type = number_range(1,22);

	if (aff_type > 6 && aff_type < 13)
		value = number_range((obj->level / 5),(obj->level - 10));
	else 
	{
		if (obj->level < 15)
			value = number_range(1,2);
		else if ( obj->level > 14 && obj->level < 25 )
			value = number_range(1,3);
		else if ( obj->level > 24 && obj->level < 35 )
			value = number_range(1,4);
		else
			value = number_range(2,5);		
	}

	if (aff_type == 20 || aff_type == 21)
	{
		value *= (obj->level / 8);
	}

	switch( aff_type )
	{
		 default: 
		 case 1:  			
			affect = APPLY_STR;
			break;
		 case 2:  			
			affect = APPLY_DEX;
			break;
		 case 3:			
			affect = APPLY_INT;
			break;
		 case 4:  			
			affect = APPLY_WIS;
			break;
		 case 5:  			
			affect = APPLY_CON;
			break;
		 case 6:  			
			affect = APPLY_CHA;			
			break;
		 case 7:  
		 case 8:			
			affect = APPLY_MANA;			
			break;						  
		 case 9:  
		 case 10:			
			affect = APPLY_HIT;
			break;
		 case 11:  
		 case 12:
			affect = APPLY_MOVE;
			break;
		 case 13: 
		 case 14:
		 case 15:			
			affect = APPLY_AC;
			break;
		 case 16: 			
			affect = APPLY_HITROLL;
			break;
		 case 17: 			
			affect = APPLY_DAMROLL;
			break;
		 case 18: 
		 case 19:			
			affect = APPLY_SAVES;
			break;		
		 case 20:			
			affect = APPLY_SPELL_CRIT;
			break;
		 case 21:			
			affect = APPLY_MELEE_CRIT;
			break;
		 case 22:			
			affect = APPLY_SPELL_DAM;
			break;
	}

	
	
	if( affect_free == NULL )
		paf = alloc_perm(sizeof(*paf));

	else
	{
		paf = affect_free;
		affect_free = affect_free->next;
	}

	paf->type = 0;
	paf->duration = -1;
	paf->location = affect;
	paf->modifier = value;
	paf->bitvector = 0;
	paf->next = obj->affected;
	obj->affected = paf;
	act ("You imbue $p with magical energy.\r\n",ch, obj, NULL, TO_CHAR);
	act ("$n imbues $p with magical energy.\r\n",ch, obj, NULL, TO_ROOM);		
	check_improve(ch,gsn_imbue,TRUE,1);	
	return;
}

void do_cook (CHAR_DATA *ch, char * argument)
{

	OBJ_DATA *recipe;
	OBJ_DATA *fire; //needed to cook with.
	OBJ_DATA *ing1 = NULL, *ing2 = NULL, *ing3 = NULL;
	OBJ_DATA *food;
	char arg1[MSL];	
	//char arg2[MSL];	
	char buf[MSL];
	int chance = number_percent();
	
	argument = one_argument( argument, arg1 );
    //argument = one_argument( argument, arg2 );
	
	if (IS_NPC(ch))
		return;
	if (!KNOWS(ch, gsn_cooking))
	{
		SEND("You have no idea what you're doing.\r\n",ch);
		return;
	}

	if (arg1[0] == '\0')
    {
        SEND ("Cook what?\r\n", ch);
        return;
    }
	
	for (fire = ch->in_room->contents; fire; fire = fire->next_content)
	{
		if (fire->item_type == ITEM_FIRE)
			break;
	}
	
	if (!fire)
	{
		SEND("You need a source of heat.\r\n",ch);
		return;
	}	
	
	if ((recipe = get_obj_carry (ch, arg1, ch)) == NULL || recipe->item_type != ITEM_COOKING_RECIPE)
	{
		SEND("You aren't carrying that recipe.\r\n",ch);
		return;
	}
	
	/*if (recipe->value[0] > 0)
	{
		ing1 = recipe->value[0];
		ing1 = get_obj_carry(ch,ing1->short_descr,ch);

		if (!ing1 || ing1->item_type != ITEM_INGREDIENT)
		{
			SEND ("You are missing the main ingredient!\r\n",ch);
			return;
		}
	}
	if (recipe->value[1] > 0)
	{
		ing2 = get_obj_index(recipe->value[1]);
		ing2 = get_obj_carry(ch,ing2->short_descr,ch);
		
		if (!ing2 || ing2->item_type != ITEM_INGREDIENT)
		{
			SEND ("You are missing the secondary ingredient!\r\n",ch);
			return;
		}
	}
	if (recipe->value[2] > 0)
	{
		ing3 = get_obj_index(recipe->value[2]);
		ing3 = get_obj_carry(ch,ing3->short_descr,ch);
		
		if (!ing3 || ing3->item_type != ITEM_INGREDIENT)
		{
			SEND ("You are missing the tertiary ingredient!\r\n",ch);
			return;
		}
	}*/
	
	chance = number_percent();
	if (recipe->level > total_levels(ch))
		chance += recipe->level - total_levels(ch);
	if (total_levels(ch) > recipe->level)
		chance -= total_levels(ch) - recipe->level;	
	
	WAIT_STATE (ch, skill_table[gsn_cooking].beats);
	
	if (chance < get_skill(ch, gsn_cooking))
	{	
		sprintf(buf, "You cook up %s.\r\n", food_table[recipe->value[3]].name);		
		SEND (buf, ch);		
		//put act here
		food = create_object (get_obj_index (OBJ_VNUM_MUSHROOM), 0);
		if (ing1)
			extract_obj(ing1);
		if (ing2)
			extract_obj(ing2);
		if (ing3)
			extract_obj(ing3);
		//extract_obj(flask);
		food->value[0] = IS_HUMAN(ch) ? food_table[recipe->value[3]].food_hours + 2 : food_table[recipe->value[3]].food_hours;
		food->value[1] = IS_HUMAN(ch) ? food_table[recipe->value[3]].full_hours + 2 : food_table[recipe->value[3]].full_hours;
		food->value[3] = food_table[recipe->value[3]].poisoned;
		food->value[0] = IS_HUMAN(ch) ? total_levels(ch) + 1 : total_levels(ch);
		
		sprintf (buf, "%s", food_table[recipe->value[3]].name);		
		food->short_descr = str_dup(buf);		
		food->name = str_dup(buf);
		food->description = str_dup(food_table[recipe->value[3]].long_descr);
		obj_to_char(food,ch);
		int xp = 0;				
		xp = number_range(50, 100);
		if (total_levels(ch) > 5)
		{
			xp += (total_levels(ch) / 2) * number_range(25,35);
		}
		
		give_exp(ch, xp, TRUE);
		
		check_improve(ch,gsn_cooking,TRUE,3);	
		return;
	}
	else
	{
		sprintf(buf, "You failed to cook up %s.\r\n", food_table[recipe->value[3]].name);		
		SEND (buf, ch);		
		if (ing1)
			extract_obj(ing1);
		if (ing2)
			extract_obj(ing2);
		if (ing3)
			extract_obj(ing3);		
		check_improve(ch,gsn_cooking,FALSE,1);	
		return;
	}
	
	
	return;
}


void do_tailor (CHAR_DATA *ch, char * argument)
{
	char arg1[MIL];
    char arg2[MIL];
	char buf[MSL];
	OBJ_DATA *obj;
	int chance = 0;
	
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (!get_skill(ch, gsn_tailoring) > 1)
	{
		SEND("You have no clue what you're doing.\r\n",ch);
		return;
	}
	
	if (arg1[0] == '\0')
    {
        SEND ("Tailor what?\r\n", ch);
        return;
    }

    if (!str_cmp(arg1, "cost") && arg2[0] == '\0')
	{
		//leave room to specify cost per slot later on... just doing basic
		//functionality for now.
		
		SEND ("Slot:               Bolts of Cloth:\r\n", ch);
		
		//List of crafted eq material costs goes here :P
		
		return;
	}
	
	if (!str_cmp(arg1, "fabric"))
	{
		int amount = 0;
	
		if (arg2[0] == '\0')
		{
			SEND("What would you like to process?\r\n",ch);
			return;
		}

		if ((obj = get_obj_carry (ch, arg2, ch)) == NULL)
		{
			SEND("You aren't carrying that.\r\n",ch);
			return;
		}
		
		if (!get_obj_list_by_type(ch, ITEM_LOOM, ch->in_room->contents))
		{
			SEND("You need a loom to do that.\r\n",ch);
			return;
		}
		
		//Check for scissors here later....
		
		if (!(mat_table[get_material(obj->material)].is_cloth))
		{
			SEND("That's not made of any kind of cloth.\r\n",ch);
			return;
		}
		
		if ((chance = number_percent()) <= get_skill(ch, gsn_tailoring))
		{
			amount = obj->weight / 5;
			if (obj->value[4] < 1)
				amount *= 2;
			
			if (amount < 1)
				amount = 1;
			sprintf(buf, "You turn the loom and create %d bolts of %s cloth.\r\n", amount, obj->material);			
			SEND(buf, ch);
			ch->mats[get_material(obj->material)] += amount;
			ch->move -= number_range(2,5);					
			int xp = 0;				
			xp = number_range(15, 30);
			if (total_levels(ch) > 5)
			{
				xp += (total_levels(ch) / 2) * number_range(25,35);
			}
			
			give_exp(ch, xp, TRUE);
			check_improve (ch, gsn_tailoring, TRUE, 2);
		}
		else
		{
			sprintf(buf, "You botch the job, ruining %s.\r\n", obj->short_descr);
			SEND (buf, ch);		
			check_improve (ch, gsn_tailoring, TRUE, 1);			
		}
		extract_obj(obj);
		return;
	}
	return;
}

void do_jewelcraft (CHAR_DATA *ch, char * argument)
{
	char arg1[MIL];
    char arg2[MIL];
	char buf[MSL];
	OBJ_DATA *obj;
	int chance = 0;
	
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (!get_skill(ch, gsn_jewelcrafting) > 1)
	{
		SEND("You have no clue what you're doing.\r\n",ch);
		return;
	}
	
	if (arg1[0] == '\0')
    {
        SEND ("Jewelcraft what?\r\n", ch);
        return;
    }

    if (!str_cmp(arg1, "cost") && arg2[0] == '\0')
	{
		//leave room to specify cost per slot later on... just doing basic
		//functionality for now.
		
		SEND ("Slot:               Shards of Gem:\r\n", ch);
		
		//List of crafted eq material costs goes here :P
		
		return;
	}
	
	if (!str_cmp(arg1, "cut"))
	{
		int amount = 0;
	
		if (arg2[0] == '\0')
		{
			SEND("What would you like to cut?\r\n",ch);
			return;
		}

		if ((obj = get_obj_carry (ch, arg2, ch)) == NULL)
		{
			SEND("You aren't carrying that.\r\n",ch);
			return;
		}
		
		//Check for jewelcrafting kit here later....
		
		if (!(mat_table[get_material(obj->material)].is_gem))
		{
			SEND("That's not made of any kind of gem.\r\n",ch);
			return;
		}
		
		if ((chance = number_percent()) <= get_skill(ch, gsn_jewelcrafting))		
		{
			amount = obj->weight / 5;
			if (obj->value[4] < 1)
				amount *= 2;
				
			if (amount < 1)
				amount = 1;
			sprintf(buf, "You meticulously cut the %s, producing %d usable shards of %s.\r\n", obj->short_descr, amount, obj->material);			
			SEND(buf, ch);
			ch->mats[get_material(obj->material)] += amount;
			ch->move -= number_range(2,4);		
			
			
			int xp = 0;				
			xp = number_range(15, 30);
			if (total_levels(ch) > 5)
			{
				xp += (total_levels(ch) / 2) * number_range(25,35);
			}
			
			give_exp(ch, xp, TRUE);
			check_improve (ch, gsn_jewelcrafting, TRUE, 2);
		}
		
		else
		{
			sprintf(buf, "You botch the job, ruining %s.\r\n", obj->short_descr);
			check_improve (ch, gsn_jewelcrafting, TRUE, 1);
			SEND (buf, ch);	
		}
		extract_obj(obj);
		return;
	}
	
	return;
}


void push_object (CHAR_DATA *ch, OBJ_DATA *obj, int door)
{
	
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	
	in_room = ch->in_room;
    if ((pexit = in_room->exit[door]) == NULL
        || (to_room = pexit->u1.to_room) == NULL
        || !can_see_room (ch, pexit->u1.to_room))
    {
        SEND ("That's not possible.\r\n", ch);
        return;
    }
	
	    if (IS_SET (pexit->exit_info, EX_CLOSED)
        && (!IS_AFFECTED (ch, AFF_PASS_DOOR)
            || IS_SET (pexit->exit_info, EX_NOPASS))
        && !IS_IMMORTAL(ch))
    {
        act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        return;
    }
	
	if (to_room->sector_type == SECT_OCEANFLOOR  || in_room->sector_type == SECT_WATER_SWIM || to_room->sector_type == SECT_WATER_SWIM || to_room->sector_type == SECT_WATER_NOSWIM)
	{	
		act ("You push $p out into the water.\r\n",ch, obj, NULL, TO_CHAR);
		act ("$n pushes $p out into the water.\r\n",ch, obj, NULL, TO_ROOM);		
		if (IS_BUOYANT(obj))
			act ("The $p bobs out on the water.\r\n",ch, obj, NULL, TO_ROOM);
		else
			act ("The $p starts to sink in the water.\r\n",ch, obj, NULL, TO_ROOM);
		obj_from_room(obj);		
		obj_to_room (obj, to_room);
		
		return;			
	}	
	else
	{	
		act ("You push $p $T.\r\n", ch, obj, dir_name[door], TO_CHAR);
		act ("$n pushes $p $T.\r\n", ch, obj, dir_name[door], TO_ROOM);
		obj_from_room(obj);		
		obj_to_room (obj, to_room);
		return;
	}
	
	return;	
}



void pull_object (CHAR_DATA *ch, OBJ_DATA *obj, int door)
{
	
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	
	in_room = ch->in_room;
    if ((pexit = in_room->exit[door]) == NULL
        || (to_room = pexit->u1.to_room) == NULL
        || !can_see_room (ch, pexit->u1.to_room))
    {
        SEND ("That's not possible.\r\n", ch);
        return;
    }
	
	    if (IS_SET (pexit->exit_info, EX_CLOSED)
        && (!IS_AFFECTED (ch, AFF_PASS_DOOR)
            || IS_SET (pexit->exit_info, EX_NOPASS))
        && !IS_IMMORTAL(ch))
    {
        act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        return;
    }
	
	if (in_room->sector_type == SECT_AIR
            || to_room->sector_type == SECT_AIR)
	{
		if (!IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL (ch))
		{
			SEND ("You can't fly.\r\n", ch);
			return;
		}
	}
	
	if ((in_room->sector_type == SECT_WATER_NOSWIM
		 || to_room->sector_type == SECT_WATER_NOSWIM)
		&& !IS_AFFECTED (ch, AFF_FLYING))
	{
		OBJ_DATA *obj;
		bool found;

		/*
		 * Look for a boat.
		 */
		found = FALSE;

		if (IS_IMMORTAL (ch))
			found = TRUE;

		for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		{
			if (obj->item_type == ITEM_BOAT)
			{
				found = TRUE;
				break;
			}
		}
		if (!found)
		{				
			SEND ("You need a boat to go there.\r\n", ch);
			return;
		}
	}

		
	if (in_room->sector_type == SECT_OCEANFLOOR || to_room->sector_type == SECT_OCEANFLOOR || in_room->sector_type == SECT_WATER_SWIM || to_room->sector_type == SECT_WATER_SWIM)
	{
		if (get_skill (ch, gsn_swim) < 1)
		{
			SEND("You need to be able to swim to go there.\r\n",ch);
			return;
		}
	}
	
		
	act ("You pull $p $T.\r\n", ch, obj, dir_name[door], TO_CHAR);
	act ("$n pulls $p $T.\r\n", ch, obj, dir_name[door], TO_ROOM);
	obj_from_room(obj);		
	obj_to_room (obj, to_room);
	return;
}

void pry_exit(CHAR_DATA *ch, int door)
{
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	
	in_room = ch->in_room;
    if ((pexit = in_room->exit[door]) == NULL
        || (to_room = pexit->u1.to_room) == NULL
        || !can_see_room (ch, pexit->u1.to_room))
    {
        SEND ("That's not possible.\r\n", ch);
        return;
    }
	 
	pexit = ch->in_room->exit[door];
	if (!IS_SET (pexit->exit_info, EX_CLOSED))
	{
		SEND ("It's already open.\r\n", ch);
		return;
	}
	int chance = 0;
	
	chance += str_app[GET_STR(ch)].carry / 10;
	
	if (ch->size > SIZE_MEDIUM)
	{
		chance += (2 * (ch->size - SIZE_MEDIUM));
	}
	
	if (!IS_FIGHTER(ch))
		chance /= 2;
	
	if (number_percent() < chance)
	{
		REMOVE_BIT (pexit->exit_info, EX_CLOSED);
		REMOVE_BIT (pexit->exit_info, EX_LOCKED);
		act ("$n pries open the $d!", ch, NULL, pexit->keyword, TO_ROOM);
		WAIT_STATE (ch, PULSE_SECOND);
		SEND ("You manage to pry it open.\r\n", ch);

		/* open the other side */
		if ((to_room = pexit->u1.to_room) != NULL
			&& (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
			&& pexit_rev->u1.to_room == ch->in_room)
		{
			CHAR_DATA *rch;

			REMOVE_BIT (pexit_rev->exit_info, EX_CLOSED);
			for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
				act ("The $d is pried open!", rch, NULL, pexit_rev->keyword, TO_CHAR);
		}
	} 
	else
	{
		act ("$n fails to pry open the $d!", ch, NULL, pexit->keyword, TO_ROOM);
		WAIT_STATE (ch, (PULSE_SECOND * 2));
		SEND ("You fail to pry it open.\r\n", ch);	
	}
	ch->move -= 5;
	return;
}

void do_push (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;
	char arg1[MIL];
    char arg2[MIL];
	int door = 0;
	
	
	argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
	
	if (arg1[0] == '\0')
	{
		SEND ("What did you want to push?\r\n",ch);
		return;
	}
	
	if (ch->event != EVENT_NONE)
	{
		SEND("Stop whatever you're doing first.\r\n",ch);
		return;
	}
	
	obj = get_obj_list (ch, arg1, ch->in_room->contents);
	if (obj == NULL)
	{
		act ("I see no $T here.\r\n", ch, NULL, arg1, TO_CHAR);
		return;
	}
	
	if (!CAN_PUSH(obj))
	{
		SEND("There's no sense in trying to push that.\r\n",ch);
		return;
	}
	
	if (arg2[0] == '\0')
	{
		SEND ("Which direction will you push it?\r\n",ch);
		return;
	}
	
	
	if (!str_cmp(arg2, "north"))
	{
		door = DIR_NORTH;
	}
	else if (!str_cmp(arg2, "east"))
	{
		door = DIR_EAST;
	}
	else if (!str_cmp(arg2, "south"))
	{
		door = DIR_SOUTH;
	}
	else if (!str_cmp(arg2, "west"))
	{
		door = DIR_WEST;
	}
	else 
	{
		SEND("Not a valid direction.\r\n",ch);
		return;
	}
	
	push_object(ch, obj, door);
	
	return;
	
}


void do_pull (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;
	char arg1[MIL];
    char arg2[MIL];
	int door = 0;
	
	
	argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
	
	if (arg1[0] == '\0')
	{
		SEND ("What did you want to pull?\r\n",ch);
		return;
	}
	
	if (ch->event != EVENT_NONE)
	{
		SEND("Stop whatever you're doing first.\r\n",ch);
		return;
	}
	
	obj = get_obj_list (ch, arg1, ch->in_room->contents);
	if (obj == NULL)
	{
		act ("I see no $T here.\r\n", ch, NULL, arg1, TO_CHAR);
		return;
	}
	
	if (!CAN_PULL(obj))
	{
		SEND("There's no sense in trying to pull that.\r\n",ch);
		return;
	}
	
	if (arg2[0] == '\0')
	{
		SEND ("Which direction will you pull it?\r\n",ch);
		return;
	}
	
	
	if (!str_cmp(arg2, "north"))
	{
		door = DIR_NORTH;
	}
	else if (!str_cmp(arg2, "east"))
	{
		door = DIR_EAST;
	}
	else if (!str_cmp(arg2, "south"))
	{
		door = DIR_SOUTH;
	}
	else if (!str_cmp(arg2, "west"))
	{
		door = DIR_WEST;
	}
	else 
	{
		SEND("Not a valid direction.\r\n",ch);
		return;
	}
	
	pull_object(ch, obj, door);
	move_char(ch, door, FALSE);
	
	return;
	
}


void do_pry (CHAR_DATA *ch, char *argument)
{	
	int door = 0;
	
	if (argument[0] == '\0')
	{
		SEND ("Which direction/door do you want to pry?\r\n",ch);
		return;
	}
	
	if (ch->move < 5)
	{
		SEND("You are too exhausted!\r\n",ch);
		return;
	}

	if (ch->event != EVENT_NONE)
	{
		SEND("Stop whatever you're doing first.\r\n",ch);
		return;
	}

	
	
	if (!str_cmp(argument, "north"))
	{
		door = DIR_NORTH;
	}
	else if (!str_cmp(argument, "east"))
	{
		door = DIR_EAST;
	}
	else if (!str_cmp(argument, "south"))
	{
		door = DIR_SOUTH;
	}
	else if (!str_cmp(argument, "west"))
	{
		door = DIR_WEST;
	}
	else if (!str_cmp(argument, "up"))
	{
		door = DIR_UP;
	}
	else if (!str_cmp(argument, "down"))
	{
		door = DIR_DOWN;
	}
	else 
	{
		SEND("Not a valid direction.\r\n",ch);
		return;
	}
	

	pry_exit(ch, door);
	return;
}


void do_press (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->event != EVENT_NONE)
	{
		SEND("Stop whatever you're doing first.\r\n",ch);
		return;
	}
	
	if (!CAN_PRESS(obj))
	{
		SEND("There's nothing to press.\r\n",ch);
		return;
	}
	
	
	return;
}


void do_sharpen(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA * wpn = NULL;	
	char buf[MSL];	
	
	if (CHECK_COOLDOWN(ch, gsn_sharpen))
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
	//if (KNOWS(ch, gsn_sharpen))
	if (KNOWS(ch,gsn_sharpen) && CAN_USE_SKILL(ch, gsn_sharpen))
	{
		int chance = 0, x = 0;
	
		if ((wpn = get_obj_carry(ch,argument,ch)) == NULL)
		{
		  act("You don't have a $T.", ch, NULL, argument, TO_CHAR);
		  return;
		}
		
		if (!IS_WEAPON(wpn))
		{
			SEND("That's not a weapon.\r\n",ch);
			return;
		}
		
		if (wpn->value[0] == WEAPON_MACE || wpn->value[0] == WEAPON_STAFF || wpn->value[0] == WEAPON_CROSSBOW || wpn->value[0] == WEAPON_BOW)
		{
			SEND("That's not a bladed weapon.\r\n",ch);
			return;
		}
		
		chance = (get_skill(ch, gsn_sharpen) / 2);
		sprintf(buf,"{R%d{x\r\n",get_skill(ch, gsn_sharpen));
		SEND(buf, ch);
		sprintf(buf,"{B%d{x\r\n",chance);
		SEND(buf, ch);
		//Dwarves are naturally good at this.
		if (IS_DWARF(ch))
			chance += 5;
		
		if (wpn->level > total_levels(ch))
		{
			x = wpn->level;
			
			while (x >= total_levels(ch))
			{			
				x--;
				chance--;
			}
		}
		
		if (total_levels(ch) > wpn->level)
		{
			x = total_levels(ch);
			
			while (x >= wpn->level)
			{			
				x--;
				chance++;
			}
		}
		
		//Strength Bonus
		for (x = 20; x < 26; x++)
		{
			if (GET_STR(ch) >= x)
				chance++;
		}
		
		if (IS_OBJ_STAT(wpn,ITEM_BLESS))
			chance += 5;
		
		if (IS_OBJ_STAT(wpn,ITEM_EVIL))
			chance /= 2;
					
		if (number_percent() < chance)
		{
			if (wpn->value[2] < 10)
				wpn->value[2] += 1;
			
			if (wpn->value[1] < 4)
			{
				if (number_percent() < (IS_DWARF(ch) ? 10 : 5))
					wpn->value[1] += 1;							
			}
				
			wpn->level++;
			act ("You sharpen $p.\r\n",ch, wpn, NULL, TO_CHAR);
			act ("$n meticulously sharpens $p.\r\n",ch, wpn, NULL, TO_ROOM);				
		}
		else
		{
			wpn->level++;
			act ("You fail to sharpen $p.\r\n",ch, wpn, NULL, TO_CHAR);
			act ("$n fails to sharpen $p.\r\n",ch, wpn, NULL, TO_ROOM);				
		}
		
		COOLDOWN(ch, gsn_sharpen);
		
		
	}
	else
	{
		SEND("You haven't the slightest idea what you're doing.\r\n",ch);
		return;
	}	

	return;
}


void do_woodcut(CHAR_DATA *ch, char *argument)
{
	char arg1 [MIL];
	//int i;	
	OBJ_DATA *axe = NULL;
	OBJ_DATA *tree = NULL;

	if (IS_NPC(ch))
		return;

	argument = one_argument( argument, arg1 );
		
	if (!get_skill(ch,gsn_woodcutting))
    {
        SEND( "You have no idea what you're doing.\r\n",ch );
        return;
    }
	
	if (IS_AFFECTED(ch,AFF_CHARM))
	{
		SEND( "You don't have the free will to do so!\r\n",ch );
		return;
	}
	
	axe = get_eq_char(ch, WEAR_WIELD);
	if (!axe)
	{
		SEND("You need something to chop it down with!\r\n",ch);
		return;
	}
	
	if (axe->value[0] != WEAPON_AXE)
	{
		SEND("You need an AXE to chop down trees...\r\n",ch);
		return;
	}
	
	if ((tree = get_obj_here( ch, NULL, arg1)) == NULL || tree->item_type != ITEM_TREE)
	{
		
		if (!str_cmp (arg1, "stop"))
		{
			if (ch->event != EVENT_WOOD_CUTTING)
			{
				SEND("You weren't doing that to begin with!\r\n",ch);
				return;
			}
			else
			{
				stop_event(ch);		
				return;
			}
		}
		
		SEND("You don't see any trees that like that here.\r\n",ch);
		return;
	}
	
	if (ch->event == EVENT_NONE)
	{	
		ch->event = EVENT_WOOD_CUTTING;
		ch->chopping = tree;
		SEND("You begin chopping at the tree.\r\n",ch);
		return;
	}
	else
	{
		SEND("You're already in the middle of something!\r\n",ch);
		return;
	}

	
	return;
}


void do_obj_dump(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *container, *into, *temp_obj, *temp_next;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  /* First, check to ensure they can dump.  */
  argument = one_argument( argument, arg1 );

  if (arg1[0] == '\0')
    {
      send_to_char("What do you want to dump out?\n\r",ch);
      return;
    }

  if ((container = get_obj_carry(ch,arg1,ch)) == NULL)
    {
      act("You don't have a $T.", ch, NULL, arg1, TO_CHAR);
      return;
    }

  if (container->item_type != ITEM_CONTAINER)
    {
      send_to_char("You can't dump that.\n\r",ch);
      return;
    }

  if (IS_SET(container->value[1], CONT_CLOSED))
    {
      act("$d is closed.", ch, NULL, container->short_descr, TO_CHAR);
      return;
    }

  /* Next, check to see if they want to dump into another container.  */
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"into") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if (arg2[0] != '\0')
    {
      /* Prefer obj in inventory other than object in room.  */
      if ((into = get_obj_carry(ch, arg2, ch)) == NULL)
        {
          if ((into = get_obj_here(ch, NULL, arg2)) == NULL)
            {
              send_to_char("You don't see that here.\n\r",ch);
              return;
            }
        }

      if (into->item_type != ITEM_CONTAINER)
        {
          send_to_char("You can't dump into that.\n\r",ch);
          return;
        }

      if (IS_SET(into->value[1], CONT_CLOSED))
        {
          act("$d is closed.", ch, NULL, into->short_descr, TO_CHAR);
          return;
        }

      act("You dump out the contents of $p into $P.",ch,container,into,TO_CHAR);
      act("$n dumps out the contents of $p into $P.",ch,container,into,TO_ROOM);

      for (temp_obj=container->contains; temp_obj != NULL; temp_obj=temp_next)
        {
          temp_next = temp_obj->next_content;

          if ((get_obj_weight(temp_obj) + get_true_weight(into)
                > (into->value[0] * 10))
              ||  (get_obj_weight(temp_obj) > (into->value[3] * 10)))
            act("$P won't fit into $p.",ch,into,temp_obj,TO_CHAR);

          else 
            {
              obj_from_obj(temp_obj);
              obj_to_obj(temp_obj, into);
            }
        }
    }

  /* Dumping to the floor.  */
  else
    {
      act("You dump out the contents of $p.",ch,container,NULL,TO_CHAR);
      act("$n dumps out the contents of $p.",ch,container,NULL,TO_ROOM);

      for (temp_obj=container->contains; temp_obj != NULL; temp_obj=temp_next)
        {
          temp_next = temp_obj->next_content;
          act("  ... $p falls out onto the ground.",ch,temp_obj,NULL,TO_CHAR);
          act("  ... $p falls out onto the ground.",ch,temp_obj,NULL,TO_ROOM);
  
          obj_from_obj(temp_obj);
          obj_to_room(temp_obj,ch->in_room);
  
          if (IS_OBJ_STAT(temp_obj,ITEM_MELT_DROP))
            {
              act("$p dissolves into smoke.",ch,temp_obj,NULL,TO_ROOM);
              act("$p dissolves into smoke.",ch,temp_obj,NULL,TO_CHAR);
              extract_obj(temp_obj);
            }
        }
    }
}

void do_bandage (CHAR_DATA * ch, char *argument)
{
	char buf[MSL];
	char arg[MIL];	
	char arg2[MIL];
    OBJ_DATA *bandage;
	OBJ_DATA *salve;
	OBJ_DATA *new_bandage;
	CHAR_DATA *victim;	
	//int i = 0;	
	
	
	argument = one_argument (argument, arg);
	argument = one_argument (argument, arg2);
	
	if (IS_NPC (ch))
        return; 
	
	if (!IS_AWAKE (ch))
    {
        SEND ("You are asleep yourself!\r\n", ch);
        return;
    }

	if (arg[0] == '\0')
    {
        SEND ("Bandage someone? Or create a bandage?\r\n", ch);
        return;
    }
	
	for ( salve = ch->carrying; salve != NULL; salve = salve->next_content )
    {
        if (salve->item_type == ITEM_SALVE)              
			break;		
    }	
	
	if (!str_cmp(arg, "create"))
	{
		if (get_skill(ch, gsn_first_aid) < 2)
		{
			SEND("You have no idea what you're doing!\r\n",ch);
			return;
		}
	
		if (!salve)
		{
			SEND("You need some sort of salve to do this.\r\n",ch);
			return;
		}
			
	
		if (arg2[0] == '\0')
		{			
			SEND("You must choose what type of cloth to use.\r\n",ch);
			return;
		}
		else
		{
			if (valid_material(arg2) == TRUE)					
			{		
				if (mat_table[get_material(arg2)].is_cloth != TRUE)
				{
					SEND("You must make it from cloth of some sort.\r\n",ch);							
					return;
				}
				if (ch->material[get_material(arg2)] >= 5)
				{
					//Dispose of components regardless of success.
					ch->material[get_material(arg2)] -= 5;
					extract_obj( salve );
					
					if (number_percent() < get_skill(ch, gsn_first_aid))
					{			
						new_bandage = create_object (get_obj_index(OBJ_VNUM_BANDAGE), total_levels(ch));									
						sprintf(buf, "a %s bandage", mat_table[get_material(arg2)].material);								
						new_bandage->short_descr = str_dup(buf);									
						new_bandage->name = str_dup(buf);
						sprintf(buf, "an unused %s bandage.", mat_table[get_material(arg2)].material);									
						new_bandage->description = str_dup(buf);
						obj_to_char(new_bandage, ch);
						check_improve (ch, gsn_first_aid, FALSE, 6);
						WAIT_STATE (ch, skill_table[gsn_first_aid].beats);
						return;
					}
					else
					{
						SEND("You botched the bandage, ruining it.\r\n",ch);
						check_improve (ch, gsn_first_aid, FALSE, 6);
						WAIT_STATE (ch, skill_table[gsn_first_aid].beats);
						return;									
					}
				}							
				else
				{								
					sprintf(buf, "You don't have enough %s to make a bandage.\r\n",mat_table[get_material(arg)].material);
					SEND(buf, ch);
					return;
				}
			}
			else
			{
				SEND ("That's not a valid material... try again.\r\n",ch);
				sprintf(buf, "%s", arg2);
				SEND (buf, ch);
				return;
			}
		}
	}
	else
	{
		if ((victim = get_char_room ( ch, NULL, arg)) == NULL )
		{
			SEND ("They aren't here.\r\n", ch);
			return;
		}
		
		for ( bandage = ch->carrying; bandage != NULL; bandage = bandage->next_content )
		{
			if (bandage->item_type == ITEM_BANDAGE)              
				break;
		}	
		
		if (!bandage)
		{
			SEND("You have nothing to bandage with.\r\n",ch);
			return;
		}
		
		if (victim->bleeding < 1)
		{
			SEND ("They aren't even bleeding though!\r\n",ch);
			return;
		}
		
		
		if (number_percent() < ((get_skill(ch, gsn_first_aid) < 1 ? 5 : get_skill(ch, gsn_first_aid)) + bandage->value[0]))
		{
			victim->bleeding -= number_range(1,2);
			victim->hit += number_fuzzy(bandage->value[1]);
			
			if (victim->hit > victim->max_hit)
				victim->hit = victim->max_hit;
			
			if (victim != ch)
			{
				sprintf(buf, "You manage to somewhat staunch %s's bloodflow.\r\n", victim->name);
				SEND(buf,ch);
				sprintf(buf, "%s tends your wounds, slightly staunching the bloodflow.\r\n", ch->name);
				SEND(buf,victim);
			}
			else
			{
				SEND("You tend to your own wounds, slightly staunching the bloodflow.\r\n",ch);				
				act( "$n tends to their wounds with a bandage.", ch, NULL, NULL, TO_ROOM);
			}
			extract_obj( bandage );
			check_improve (ch, gsn_first_aid, TRUE, 2);
		}
		else
		{		
			if (victim != ch)
			{
				sprintf(buf, "You fail to slow %s's bloodflow.\r\n", victim->name);
				SEND(buf,ch);
				sprintf(buf, "%s tends your wounds, but is unable to slow the bleeding.\r\n", ch->name);
				SEND(buf,victim);				
			}
			else
				SEND("You tend to your own wounds, but fail to slow the bleeding.\r\n",ch);
				
			if (number_percent() < 50)
				extract_obj( bandage );
			
			check_improve (ch, gsn_first_aid, FALSE, 3);
		}
		
		
		WAIT_STATE (ch, skill_table[gsn_first_aid].beats);
		return;
	}
}