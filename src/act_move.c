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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_move.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#include <string.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"



char *const dir_name[] = {
    "north", "east", "south", "west", "up", "down"
};

const sh_int rev_dir[] = {
    2, 3, 0, 1, 5, 4
};





/*
 * Local functions.
 */
int find_door args ((CHAR_DATA * ch, char *arg));
bool has_key args ((CHAR_DATA * ch, int key));
OBJ_DATA * get_key  args ((CHAR_DATA *ch, int key));
bool check_single_use args ((OBJ_DATA * key));

//Other funcs
bool valid_material			args((char *argument));
int total_levels			args(( CHAR_DATA *ch));
AREA_DATA *get_ship_area 	args((char *argument));
bool has_thieves_tools		args((CHAR_DATA *ch));

void move_ship (CHAR_DATA * ch, int door)
{
		
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
	AREA_DATA *area;
	DESCRIPTOR_DATA *d;

    if (door < 0 || door > 5)
    {
        bug ("move_ship: bad door %d.", door);
        return;
    }

    /*
     * Exit trigger, if activated, bail out. Only PCs are triggered.
     */
    if ( !IS_NPC(ch) 
      && (p_exit_trigger( ch, door, PRG_MPROG ) 
      ||  p_exit_trigger( ch, door, PRG_OPROG )
      ||  p_exit_trigger( ch, door, PRG_RPROG )) )
	return;

    in_room = ch->ship->in_room;
    if ((pexit = in_room->exit[door]) == NULL
        || (to_room = pexit->u1.to_room) == NULL
        || !can_see_room (ch, pexit->u1.to_room))
    {
        SEND ("Alas, you cannot sail that way.\r\n", ch);
        return;
    }   

    if (!is_room_owner (ch, to_room) && room_is_private (to_room))
    {
        SEND ("That room is private right now.\r\n", ch);
        return;
    }

    if (!IS_NPC (ch))
    {        
		char buf[MSL];
        switch (to_room->sector_type)
		{
			default: SEND("You can't sail there.\r\n",ch); return; break;
			case SECT_WATER_NOSWIM:
			case SECT_WATER_SWIM:
			{								
				area = get_ship_area(ch->ship->name);
				if (!area)
					return;
				else
				{
					area->ship_vnum = to_room->vnum;
				}
				sprintf(buf, "The ship bobs gently along the water to the %s.\r\n", dir_name[door]);				
				for (d = descriptor_list; d; d = d->next)
				{
					if (d->connected == CON_PLAYING && d->character->in_room->area == ch->in_room->area)					
					{
						d->character->ship->in_room = to_room;						
						SEND(buf, d->character);	
						do_function (d->character, &do_look, "overboard");
					}
					
					
				}								
				//act ("The ship '$n' bobs gently $T.", ch->ship->name, NULL, dir_name[door], TO_ROOM);
				break;
			}
			case SECT_DOCK:
			{				
				area = get_ship_area(ch->ship->name);
				if (!area)
					return;
				else
				{
					area->ship_vnum = to_room->vnum;
				}				
				for (d = descriptor_list; d; d = d->next)
				{
					if (d->connected == CON_PLAYING && d->character->in_room->area == ch->in_room->area)					
					{
						d->character->ship->in_room = to_room;
						if (d->character == ch)
							SEND("You ease the ship into the dock.\r\n", ch);
						else					
							SEND("The ship eases into the dock.\r\n", d->character);
					}
				}				
				break;
			}
		}
              
		if (!IS_IMMORTAL(ch))
			WAIT_STATE (ch, PULSE_VIOLENCE);		
    }	
	
    if (in_room == to_room)        /* no circular follows */
        return;

	if (IS_SET (ch->ship->in_room->room_flags, ROOM_ANTI_MAGIC))
	{
		spell_cancellation (skill_lookup ("cancellation"), MAX_LEVEL+50, ch, ch, TARGET_CHAR);
	}    
    return;
}

void move_char (CHAR_DATA * ch, int door, bool follow)
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
	OBJ_DATA  *obj;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
	char buf[MSL];

    if (door < 0 || door > 5)
    {
        bug ("Do_move: bad door %d.", door);
        return;
    }

	if (ch->event != EVENT_NONE)
	{
		SEND("Stop whatever you're doing first.\r\n",ch);
		return;
	}
	
    /*
     * Exit trigger, if activated, bail out. Only PCs are triggered.
     */
    if ( !IS_NPC(ch) 
      && (p_exit_trigger( ch, door, PRG_MPROG ) 
      ||  p_exit_trigger( ch, door, PRG_OPROG )
      ||  p_exit_trigger( ch, door, PRG_RPROG )) )
	return;	
	
    in_room = ch->in_room;
    if ((pexit = in_room->exit[door]) == NULL
        || (to_room = pexit->u1.to_room) == NULL
        || !can_see_room (ch, pexit->u1.to_room))
    {
        SEND ("Alas, you cannot go that way.\r\n", ch);
        return;
    }
	
	//Don't let them in unlinked areas. Academy is excepted.
	if (!IS_SET(to_room->area->area_flags, AREA_LINKED) && !IS_IMMORTAL(ch) && str_cmp(to_room->area->name, "Adventurer's Academy"))
	{
		SEND ("That area is not linked to the game world.\r\n",ch);
		return;
	}
	
	if (IS_NPC(ch) && IS_SET(ch->act2, ACT2_STAY_SECTOR) && to_room->sector_type != ch->in_room->sector_type)
		return;
	
    if (IS_SET (pexit->exit_info, EX_CLOSED)
        && (!IS_AFFECTED (ch, AFF_PASS_DOOR)
            || IS_SET (pexit->exit_info, EX_NOPASS))
        && !IS_IMMORTAL(ch))
    {
        act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM)
        && ch->master != NULL && in_room == ch->master->in_room)
    {
        SEND ("What?  And leave your beloved master?\r\n", ch);
        return;
    }

    if (!is_room_owner (ch, to_room) && room_is_private (to_room))
    {
        SEND ("That room is private right now.\r\n", ch);
        return;
    }

    if (!IS_NPC (ch))
    {
        int iClass, iGuild;
        int move;

        for (iClass = 0; iClass < MAX_CLASS; iClass++)
        {
            for (iGuild = 0; iGuild < MAX_GUILD; iGuild++)
            {
                if (iClass != ch->ch_class && to_room->vnum ==ch_class_table[iClass].guild[iGuild])
				{					
					if (IS_MCLASSED(ch) && iClass != ch->ch_class2 && to_room->vnum ==ch_class_table[iClass].guild[iGuild])
					{
						if (!IS_IMMORTAL(ch))
						{
							SEND ("You aren't allowed in there.\r\n", ch);
							return;
						}
					}
				}
            }
        }

		
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_WIZARD) && IS_WIZARD(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_CLERIC) && IS_CLERIC(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_THIEF) && IS_THIEF(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_FIGHTER) && IS_FIGHTER(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_DRUID) && IS_DRUID(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}		
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_RANGER) && IS_RANGER(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_PALADIN) && IS_PALADIN(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_BARD) && IS_BARD(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
		if (IS_SET (pexit->u1.to_room->room_flags, ROOM_ANTI_MONK) && IS_MONK(ch) && !IS_IMMORTAL(ch))
		{
			SEND ("You aren't allowed in there.\r\n", ch);
			return;
		}
			
		if (is_affected(ch, gsn_web) && !dex_check(ch, (20 - GET_DEX(ch)) && !str_check(ch, (20 - GET_STR(ch)))))
		{
			SEND ("Magical webs prevent your movement.\r\n",ch);
			return;
		}
		
		if (is_affected(ch, gsn_paralyzation) && !wis_check(ch, (20 - GET_WIS(ch)) && !con_check(ch, (20 - GET_CON(ch)))))
		{
			SEND ("Panic! You are paralyzed!\r\n",ch);
			return;
		}
		
		if (is_affected(ch, gsn_entangle) && !dex_check(ch, (20 - GET_DEX(ch))))
		{
			SEND ("Magical vines and roots prevent your movement.\r\n",ch);
			return;
		}
		
		if (is_affected(ch, gsn_stun_fist))
		{
			SEND ("You are still stunned from impact!\r\n",ch);
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
		
		if (in_room->sector_type == SECT_LAVA && !is_affected(ch, gsn_fly) && !is_affected(ch, gsn_elemental_protection))
		{
			ch->hit -= number_range(1,8);
			if (ch->hit < 1)
			{				
				if (IS_IMMORTAL(ch))
				{
					ch->hit = 1;
					return;
				}
				
				if (IS_NPC(ch))	
				{
					act ( "$n dies from burn wounds.\r\n",ch, NULL, NULL, TO_ROOM);
					ch->fighting = NULL;
					raw_kill(ch);
				}
				else
				{
					if (IS_KNOWN(ch, gsn_iron_will))
					{
						if (check_iron_will(ch))
						{				
							SEND ("Your body continues to ignore the pull of death.\r\n",ch);
							return;
						}
						else
						{						
							SEND ("Your body can no longer resist the searing heat.\r\n",ch);
							SEND ("You have {RDIED{x from searing heat and burn wounds!!\r\n", ch );															
							ch->position = POS_DEAD;
							ch->fighting = NULL;
							raw_kill (ch);
							update_pos(ch);
							return;
						}
					}
					else
					{						
						SEND("You have {RDIED{x from searing heat and burn wounds!!\r\n", ch );								
						raw_kill(ch);				
					}
				}
			
			}
		}
        switch (to_room->sector_type)
		{
            default:
				move = 1;
				break;
			case SECT_INSIDE:
			case SECT_VOID:
			case SECT_SHIP:
				move = 1;
				break;
			
			case SECT_CITY:
			case SECT_ROAD:
			case SECT_UNDERGROUND:
			case SECT_CAVE:
			case SECT_DOCK:
			case SECT_BEACH:
			case SECT_SHORELINE:
			case SECT_FIELD:
				move = 2;
				break;
				
			case SECT_FOREST:
			case SECT_HILLS:
			case SECT_SWAMP:
				move = 4;
				break;
				
			case SECT_MOUNTAIN:
			case SECT_JUNGLE:
			case SECT_DESERT:
			case SECT_ICE:
				move = 5;
				break;
				
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
			case SECT_SNOW:
				move = 8;
				break;
		
			case SECT_OCEANFLOOR:
				move = 10;
				break;
			
			case SECT_LAVA:
				move = 12;
				break;
		}


		if (ENCUMBERED(ch))
			move += 3;
		
        /* conditional effects */
        if (IS_AFFECTED (ch, AFF_FLYING) || (is_affected(ch, gsn_fly)))
		{
			if (to_room->sector_type != SECT_WATER_SWIM && to_room->sector_type != SECT_WATER_NOSWIM && to_room->sector_type == SECT_OCEANFLOOR)
				move /= 2;
		}
		if (IS_AFFECTED (ch, AFF_HASTE) || (is_affected(ch, gsn_quicken_tempo)))
			move /= 2;
	
        if (IS_AFFECTED (ch, AFF_SLOW) || (is_affected(ch, gsn_slow)))
            move *= 2;
		
		if (IS_AFFECTED (ch, AFF_WEAKEN) || (is_affected(ch, gsn_weaken)))
            move *= 2;		
		
		if (is_affected(ch, gsn_web))
			move *= 2;
		
		if (move < 1 && number_percent() < 50)
			move = 1;

        if (ch->move < move)
        {
            SEND ("You are too exhausted.\r\n", ch);
            return;
        }

		
		if ( !IS_IMMORTAL ( ch ) )
		{
			if (IS_AFFECTED (ch, AFF_SLOW))
				WAIT_STATE (ch, 5);
			else
				WAIT_STATE (ch, 1);
				
			ch->move -= move;
		}		
    }	
	

	if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
	{		
		switch (ch->in_room->sector_type)
		{
			default:
				if (IS_AFFECTED(ch, AFF_FLYING))
				{
					if (!IS_NULLSTR(ch->walk_desc) && strcmp(ch->walk_desc, "(null)"))
						act ("$t flies $T.", ch, ch->walk_desc, dir_name[door], TO_NOTVICT);
					else
					{
						if (ch->sex == SEX_NEUTRAL)
							act ("the $t flies $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
						else if (ch->sex == SEX_MALE)
							act ("a male $t flies $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
						else
							act ("a female $t flies $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
					}
					break;
				}
				else
				{				
					if (!IS_NULLSTR(ch->walk_desc) && strcmp(ch->walk_desc, "(null)") )
						if (ch->size >= SIZE_LARGE)
							act ("$t lumbers $T.", ch, ch->walk_desc, dir_name[door], TO_NOTVICT);
						else
							act ("$t walks $T.", ch, ch->walk_desc, dir_name[door], TO_NOTVICT);
					else
					{
						if (ch->sex == SEX_NEUTRAL)
							act ("the $t walks $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
						else if (ch->sex == SEX_MALE)
							act ("a male $t walks $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
						else
							act ("a female $t walks $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);			
					}
					break;
				}
			case SECT_WATER_SWIM:
			case SECT_OCEANFLOOR:							
				if (!IS_NULLSTR(ch->walk_desc) && strcmp(ch->walk_desc, "(null)") )
					act ("$t swims $T.", ch, ch->walk_desc, dir_name[door], TO_NOTVICT);
				else
				{
					if (ch->sex == SEX_NEUTRAL)
						act ("the $t swims $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
					else if (ch->sex == SEX_MALE)
						act ("a male $t swims $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
					else
						act ("a female $t swims $T.", ch, race_table[ch->race].name, dir_name[door], TO_NOTVICT);				
				}
				break;
				
		}
	}
	////Tracks code. Upro 2/19/2010
	if (!IS_NPC(ch))
	{
		switch (in_room->sector_type)
		{
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
			case SECT_AIR:
			case SECT_INSIDE:
			case SECT_ROAD:
			case SECT_CITY:
			case SECT_SHIP:
			case SECT_VOID:
				break;
			default:
				if (!is_affected(ch, gsn_pass_without_trace) && !IS_AFFECTED (ch, AFF_FLYING) && !IS_IMMORTAL(ch) && (!IS_AFFECTED (ch, AFF_SNEAK) || (IS_AFFECTED(ch, AFF_SNEAK) && number_percent() < 50)))
				{				
					obj = create_object (get_obj_index (OBJ_VNUM_TRACKS), 0);
					obj->timer = 5;
					sprintf (buf, "A set of %s tracks can be seen here.", race_table[ch->race].name);
					obj->description = str_dup(buf);
					obj_to_room(obj, ch->in_room);
				}
				break;
		}	
	}
	////End tracks code.
	
	char exit_desc[MSL];

	switch (rev_dir[door])
	{
		default:
			sprintf(exit_desc, "the %s", dir_name[rev_dir[door]]);
			break;
		case (DIR_UP):
			sprintf(exit_desc, "above");
			break;
		case (DIR_DOWN):
			sprintf(exit_desc, "below");
			break;
	}

    char_from_room (ch);	
    char_to_room (ch, to_room);	

	if (is_affected(ch, gsn_stone_meld))
		affect_strip(ch, gsn_stone_meld);
	
    if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
	{		
		if (!IS_NULLSTR(ch->walk_desc) && strcmp(ch->walk_desc, "(null)") )
		{	
			if (rev_dir[door] == DIR_UP)
				act ("$t arrives from $T.", ch, ch->walk_desc, exit_desc, TO_NOTVICT);
			if (rev_dir[door] == DIR_DOWN)
				act ("$t arrives from $T.", ch, ch->walk_desc, exit_desc, TO_NOTVICT);
			else
				act ("$t arrives from $T.", ch, ch->walk_desc, exit_desc, TO_NOTVICT);
		}
		else
		{
			if (ch->sex == SEX_NEUTRAL)
				act ("the $t arrives from $T.", ch, race_table[ch->race].name, exit_desc, TO_NOTVICT);				
			else if (ch->sex == SEX_MALE)
				act ("a male $t arrives from $T.", ch, race_table[ch->race].name, exit_desc, TO_NOTVICT);				
			else
				act ("a female $t arrives from $T.", ch, race_table[ch->race].name, exit_desc, TO_NOTVICT);				
		}
	}
	
    do_function (ch, &do_look, "auto");
	
	if (IS_SET (ch->in_room->room_flags, ROOM_LAW || ch->in_room->sector_type == SECT_CITY))
	{
		if (IS_SET (ch->act, PLR_AUTO_SHEATHE))
		{
			SEND ("\r\nYou sheathe your weapon.\r\n",ch);
		}
	}
	/*
	OBJ_DATA *wield;
	OBJ_DATA *wield2;

	wield = get_eq_char (ch, WEAR_WIELD);	
	wield2 = get_eq_char (ch, WEAR_SECONDARY);	

	if (wield && IS_WEAPON(wield) || wield2 && IS_WEAPON(wield2))
	{
		for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
		{
			fch_next = fch->next_in_room;			
			if (IS_OBJ_STAT (wield, ITEM_ANTI_EVIL) && IS_EVIL(fch) && number_percent() < 50)		
			{				
				act("$p glows a faint cool blue color suddenly.", ch, wield->short_descr, NULL, TO_ROOM);
				break; //we don't want to see this message 15 times if that many evil creatures are here.
			}
			if (IS_OBJ_STAT (wield2, ITEM_ANTI_EVIL) && IS_EVIL(fch) && number_percent() < 50)			
			{
				act("$p glows a faint cool blue color suddenly.", ch, wield2->short_descr, NULL, TO_ROOM);
				break; //we don't want to see this message 15 times if that many evil creatures are here.
			}			
			
			if (IS_OBJ_STAT (wield, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(fch) && number_percent() < 50)		
			{				
				act("$p glows a faint tarnished {yyellow{x color suddenly.", ch, wield->short_descr, NULL, TO_ROOM);
				break; //we don't want to see this message 15 times if that many evil creatures are here.
			}
			if (IS_OBJ_STAT (wield2, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(fch) && number_percent() < 50)			
			{
				act("$p glows a faint tarnished {yyellow{x color suddenly.", ch, wield2->short_descr, NULL, TO_ROOM);
				break; //we don't want to see this message 15 times if that many evil creatures are here.
			}			

			if (IS_OBJ_STAT (wield, ITEM_ANTI_GOOD) && IS_GOOD(fch) && number_percent() < 50)		
			{				
				act("$p glows a faint dark {rred{x color suddenly.", ch, wield->short_descr, NULL, TO_ROOM);
				break; //we don't want to see this message 15 times if that many evil creatures are here.
			}
			if (IS_OBJ_STAT (wield2, ITEM_ANTI_GOOD) && IS_GOOD(fch) && number_percent() < 50)			
			{
				act("$p glows a faint dark {rred{x color suddenly.", ch, wield2->short_descr, NULL, TO_ROOM);
				break; //we don't want to see this message 15 times if that many evil creatures are here.
			}			

		}
	}

	*/
    if (in_room == to_room)        /* no circular follows */
        return;

    for (fch = in_room->people; fch != NULL; fch = fch_next)
    {
        fch_next = fch->next_in_room;

        if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
            && fch->position < POS_STANDING)
            do_function (fch, &do_stand, "");

        if (fch->master == ch && fch->position == POS_STANDING
            && can_see_room (fch, to_room))
        {

            if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
                && (IS_NPC (fch) && IS_SET (fch->act, ACT_AGGRESSIVE)))
            {
                act ("You can't bring $N into the city.",
                     ch, NULL, fch, TO_CHAR);
                act ("You aren't allowed in the city.",
                     fch, NULL, NULL, TO_CHAR);
                continue;
            }

            act ("You follow $N.", fch, NULL, ch, TO_CHAR);
            move_char (fch, door, TRUE);
        }
    }


	if (IS_SET (ch->in_room->room_flags, ROOM_ANTI_MAGIC))
	{
		spell_cancellation (skill_lookup ("cancellation"), MAX_LEVEL+50, ch, ch, TARGET_CHAR);
	}
	
    /* 
     * If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way...
     */
    if (IS_NPC (ch) && HAS_TRIGGER_MOB(ch, TRIG_ENTRY))
        p_percent_trigger (ch, NULL, NULL, ch, NULL, NULL, TRIG_ENTRY);
    if ( !IS_NPC( ch ) )
    {
    	p_greet_trigger( ch, PRG_MPROG );
		p_greet_trigger( ch, PRG_OPROG );
		p_greet_trigger( ch, PRG_RPROG );
    }

    return;
}



void do_north (CHAR_DATA * ch, char *argument)
{
	move_char (ch, DIR_NORTH, FALSE);
	return;
}

void do_east (CHAR_DATA * ch, char *argument)
{
	move_char (ch, DIR_EAST, FALSE);
	return;
}

void do_south (CHAR_DATA * ch, char *argument)
{
	move_char (ch, DIR_SOUTH, FALSE);
	return;
}

void do_west (CHAR_DATA * ch, char *argument)
{
	move_char (ch, DIR_WEST, FALSE);
	return;
}

void do_up (CHAR_DATA * ch, char *argument)
{	
	move_char (ch, DIR_UP, FALSE);
	return;
}

void do_down (CHAR_DATA * ch, char *argument)
{
	move_char (ch, DIR_DOWN, FALSE);
	return;
}

void do_sail (CHAR_DATA *ch, char *arg)
{
	AREA_DATA *area;
	ROOM_INDEX_DATA *room;
	
	if (!ch->onBoard)
	{
		SEND("You need to be on a ship to sail!\r\n",ch);
		return;
	}
	
	area = get_ship_area(ch->ship->name);
	if (!area)
	{	
		SEND("Something isn't right. Contact an immortal.\r\n",ch);
		return;
	}
	
	room = ch->in_room;
	if (str_cmp(room->owner,ch->name))
	{
		SEND("That's not your boat to steer!\r\n",ch);
		return;
	}
	
	if (ch->ship->anchored)
	{
		SEND("Raise the anchor first!\r\n",ch);
		return;
	}
	
	if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
	{
        move_ship (ch, DIR_NORTH);
		check_improve (ch, gsn_seafaring, FALSE, 6);
		return;
	}
    else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
	{
        move_ship (ch, DIR_EAST);
		check_improve (ch, gsn_seafaring, FALSE, 6);
		return;
	}
    else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
	{
       	move_ship (ch, DIR_SOUTH);
		check_improve (ch, gsn_seafaring, FALSE, 6);
		return;
	}
    else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
	{
        move_ship (ch, DIR_WEST);
		check_improve (ch, gsn_seafaring, FALSE, 6);
		return;
	}
    else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
	{
        move_ship (ch, DIR_UP);
		check_improve (ch, gsn_seafaring, FALSE, 6);
		return;
	}
    else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
	{
        move_ship (ch, DIR_DOWN);
		check_improve (ch, gsn_seafaring, FALSE, 6);
		return;
	}
	else
	{
		SEND("You can't sail whichever way you just specified...\r\n",ch);
		return;
	}

}

int find_door (CHAR_DATA * ch, char *arg)
{
    EXIT_DATA *pexit;
    int door;

    if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
        door = 0;
    else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
        door = 1;
    else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
        door = 2;
    else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
        door = 3;
    else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
        door = 4;
    else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
        door = 5;
    else
    {
        for (door = 0; door <= 5; door++)
        {
            if ((pexit = ch->in_room->exit[door]) != NULL
                && IS_SET (pexit->exit_info, EX_ISDOOR)
                && pexit->keyword != NULL && is_name (arg, pexit->keyword))
                return door;
        }
        act ("I see no $T here.", ch, NULL, arg, TO_CHAR);
        return -1;
    }

    if ((pexit = ch->in_room->exit[door]) == NULL)
    {
        act ("I see no door $T here.", ch, NULL, arg, TO_CHAR);
        return -1;
    }

    if (!IS_SET (pexit->exit_info, EX_ISDOOR))
    {
        SEND ("You can't do that.\r\n", ch);
        return -1;
    }

    return door;
}



void do_open (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Open what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_here( ch, NULL, arg)) != NULL)
    {
        /* open portal */
        if (obj->item_type == ITEM_PORTAL)
        {
            if (!IS_SET (obj->value[1], EX_ISDOOR))
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }

            if (!IS_SET (obj->value[1], EX_CLOSED))
            {
                SEND ("It's already open.\r\n", ch);
                return;
            }

            if (IS_SET (obj->value[1], EX_LOCKED))
            {
                SEND ("It's locked.\r\n", ch);
                return;
            }

            REMOVE_BIT (obj->value[1], EX_CLOSED);
            act ("You open $p.", ch, obj, NULL, TO_CHAR);
            act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
            return;
        }

        /* 'open object' */
        if (obj->item_type != ITEM_CONTAINER)
        {
            SEND ("That's not a container.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_CLOSED))
        {
            SEND ("It's already open.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_CLOSEABLE))
        {
            SEND ("You can't do that.\r\n", ch);
            return;
        }
        if (IS_SET (obj->value[1], CONT_LOCKED))
        {
            SEND ("It's locked.\r\n", ch);
            return;
        }

        REMOVE_BIT (obj->value[1], CONT_CLOSED);
        act ("You open $p.", ch, obj, NULL, TO_CHAR);
        act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
		if (is_trapped(obj))
		{
				if (!save_vs_trap(ch, obj))
				{
					spring_trap(ch, obj);
					remove_trap(obj);	
					return;
				}
				else
				{
					SEND ("You got lucky... the trap failed to spring.\r\n", ch);
					remove_trap(obj);	
					return;
				}
		}
        return;
    }

    if ((door = find_door (ch, arg)) >= 0)
    {
        /* 'open door' */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if (!IS_SET (pexit->exit_info, EX_CLOSED))
        {
            SEND ("It's already open.\r\n", ch);
            return;
        }
        if (IS_SET (pexit->exit_info, EX_LOCKED))
        {
            SEND ("It's locked.\r\n", ch);
            return;
        }

        REMOVE_BIT (pexit->exit_info, EX_CLOSED);
        act ("$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
        SEND ("Ok.\r\n", ch);

        /* open the other side */
        if ((to_room = pexit->u1.to_room) != NULL
            && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
            && pexit_rev->u1.to_room == ch->in_room)
        {
            CHAR_DATA *rch;

            REMOVE_BIT (pexit_rev->exit_info, EX_CLOSED);
            for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
                act ("The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
        }
    }

    return;
}



void do_close (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Close what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_here( ch, NULL, arg)) != NULL)
    {
        /* portal stuff */
        if (obj->item_type == ITEM_PORTAL)
        {

            if (!IS_SET (obj->value[1], EX_ISDOOR)
                || IS_SET (obj->value[1], EX_NOCLOSE))
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }

            if (IS_SET (obj->value[1], EX_CLOSED))
            {
                SEND ("It's already closed.\r\n", ch);
                return;
            }

            SET_BIT (obj->value[1], EX_CLOSED);
            act ("You close $p.", ch, obj, NULL, TO_CHAR);
            act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
            return;
        }

        /* 'close object' */
        if (obj->item_type != ITEM_CONTAINER)
        {
            SEND ("That's not a container.\r\n", ch);
            return;
        }
        if (IS_SET (obj->value[1], CONT_CLOSED))
        {
            SEND ("It's already closed.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_CLOSEABLE))
        {
            SEND ("You can't do that.\r\n", ch);
            return;
        }

        SET_BIT (obj->value[1], CONT_CLOSED);
        act ("You close $p.", ch, obj, NULL, TO_CHAR);
        act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
        return;
    }

    if ((door = find_door (ch, arg)) >= 0)
    {
        /* 'close door' */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if (IS_SET (pexit->exit_info, EX_CLOSED))
        {
            SEND ("It's already closed.\r\n", ch);
            return;
        }

        SET_BIT (pexit->exit_info, EX_CLOSED);
        act ("$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM);
        SEND ("Ok.\r\n", ch);

        /* close the other side */
        if ((to_room = pexit->u1.to_room) != NULL
            && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
            && pexit_rev->u1.to_room == ch->in_room)
        {
            CHAR_DATA *rch;

            SET_BIT (pexit_rev->exit_info, EX_CLOSED);
            for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
                act ("The $d closes.", rch, NULL, pexit_rev->keyword,
                     TO_CHAR);
        }
    }

    return;
}



bool has_key (CHAR_DATA * ch, int key)
{
    OBJ_DATA *obj;

	if (ch == NULL)
        return FALSE;
	
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
        if (obj->pIndexData->vnum == key)
            return TRUE;
    }

    return FALSE;
}

OBJ_DATA *get_key(CHAR_DATA *ch, int key)
{
    OBJ_DATA *obj;

	if (ch == NULL)
        return NULL;
	
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
        if (obj->pIndexData->vnum == key)
            return obj;
    }

    return NULL;
}

bool check_single_use (OBJ_DATA *key)
{
	if (key == NULL)
		return FALSE;

	if (key->value[0] == 1 && key->item_type == ITEM_KEY)
	{
		return TRUE;
		
    }
    return FALSE;
}

void do_lock (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Lock what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_here( ch, NULL, arg)) != NULL)
    {
        /* portal stuff */
        if (obj->item_type == ITEM_PORTAL)
        {
            if (!IS_SET (obj->value[1], EX_ISDOOR)
                || IS_SET (obj->value[1], EX_NOCLOSE))
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }
            if (!IS_SET (obj->value[1], EX_CLOSED))
            {
                SEND ("It's not closed.\r\n", ch);
                return;
            }

            if (obj->value[4] < 0 || IS_SET (obj->value[1], EX_NOLOCK))
            {
                SEND ("It can't be locked.\r\n", ch);
                return;
            }

            if (!has_key (ch, obj->value[4]))
            {
                SEND ("You lack the key.\r\n", ch);
                return;
            }

            if (IS_SET (obj->value[1], EX_LOCKED))
            {
                SEND ("It's already locked.\r\n", ch);
                return;
            }

            SET_BIT (obj->value[1], EX_LOCKED);
            act ("You lock $p.", ch, obj, NULL, TO_CHAR);
            act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
            return;
        }

        /* 'lock object' */
        if (obj->item_type != ITEM_CONTAINER)
        {
            SEND ("That's not a container.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_CLOSED))
        {
            SEND ("It's not closed.\r\n", ch);
            return;
        }
        if (obj->value[2] < 0)
        {
            SEND ("It can't be locked.\r\n", ch);
            return;
        }
        if (!has_key (ch, obj->value[2]))
        {
            SEND ("You lack the key.\r\n", ch);
            return;
        }
        if (IS_SET (obj->value[1], CONT_LOCKED))
        {
            SEND ("It's already locked.\r\n", ch);
            return;
        }

        SET_BIT (obj->value[1], CONT_LOCKED);
        act ("You lock $p.", ch, obj, NULL, TO_CHAR);
        act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
        return;
    }

    if ((door = find_door (ch, arg)) >= 0)
    {
        /* 'lock door' */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if (!IS_SET (pexit->exit_info, EX_CLOSED))
        {
            SEND ("It's not closed.\r\n", ch);
            return;
        }
        if (pexit->key < 0)
        {
            SEND ("It can't be locked.\r\n", ch);
            return;
        }
        if (!has_key (ch, pexit->key))
        {
            SEND ("You lack the key.\r\n", ch);
            return;
        }
        if (IS_SET (pexit->exit_info, EX_LOCKED))
        {
            SEND ("It's already locked.\r\n", ch);
            return;
        }

        SET_BIT (pexit->exit_info, EX_LOCKED);
        SEND ("*Click*\r\n", ch);
        act ("$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

        /* lock the other side */
        if ((to_room = pexit->u1.to_room) != NULL
            && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
            && pexit_rev->u1.to_room == ch->in_room)
        {
            SET_BIT (pexit_rev->exit_info, EX_LOCKED);
        }
    }

    return;
}



void do_unlock (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Unlock what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_here( ch, NULL, arg)) != NULL)
    {
        /* portal stuff */
        if (obj->item_type == ITEM_PORTAL)
        {
            if (!IS_SET (obj->value[1], EX_ISDOOR))
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }

            if (!IS_SET (obj->value[1], EX_CLOSED))
            {
                SEND ("It's not closed.\r\n", ch);
                return;
            }

            if (obj->value[4] < 0)
            {
                SEND ("It can't be unlocked.\r\n", ch);
                return;
            }

            if (!has_key (ch, obj->value[4]))
            {
                SEND ("You lack the key.\r\n", ch);
                return;
            }

            if (!IS_SET (obj->value[1], EX_LOCKED))
            {
                SEND ("It's already unlocked.\r\n", ch);
                return;
            }

			
			
            REMOVE_BIT (obj->value[1], EX_LOCKED);
            act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
            act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);			
			OBJ_DATA *theKey = NULL;
			if ((theKey = get_key(ch, obj->value[4])) != NULL)
			{
				if (check_single_use(theKey))
					extract_obj(theKey);
			}
            return;
        }

        /* 'unlock object' */
        if (obj->item_type != ITEM_CONTAINER)
        {
            SEND ("That's not a container.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_CLOSED))
        {
            SEND ("It's not closed.\r\n", ch);
            return;
        }
        if (obj->value[2] < 0)
        {
            SEND ("It can't be unlocked.\r\n", ch);
            return;
        }
        if (!has_key (ch, obj->value[2]))
        {
            SEND ("You lack the key.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_LOCKED))
        {
            SEND ("It's already unlocked.\r\n", ch);
            return;
        }

        REMOVE_BIT (obj->value[1], CONT_LOCKED);
        act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
        act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
		OBJ_DATA *theKey = NULL;
		if ((theKey = get_key(ch, obj->value[2])) != NULL)
		{
			if (check_single_use(theKey))
				extract_obj(theKey);
		}
        return;
    }

    if ((door = find_door (ch, arg)) >= 0)
    {
        /* 'unlock door' */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if (!IS_SET (pexit->exit_info, EX_CLOSED))
        {
            SEND ("It's not closed.\r\n", ch);
            return;
        }
        if (pexit->key < 0)
        {
            SEND ("It can't be unlocked.\r\n", ch);
            return;
        }
        if (!has_key (ch, pexit->key))
        {
            SEND ("You lack the key.\r\n", ch);
            return;
        }
        if (!IS_SET (pexit->exit_info, EX_LOCKED))
        {
            SEND ("It's already unlocked.\r\n", ch);
            return;
        }

        REMOVE_BIT (pexit->exit_info, EX_LOCKED);
        SEND ("*Click*\r\n", ch);
        act ("$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
		OBJ_DATA *theKey = NULL;
		if ((theKey = get_key(ch, pexit->key)) != NULL)
		{
			if (check_single_use(theKey))
				extract_obj(theKey);
		}
        /* unlock the other side */
        if ((to_room = pexit->u1.to_room) != NULL
            && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
            && pexit_rev->u1.to_room == ch->in_room)
        {
            REMOVE_BIT (pexit_rev->exit_info, EX_LOCKED);
        }
    }

    return;
}



void do_pick (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;
	int percent = 0;

    one_argument (argument, arg);

	if ( get_skill (ch, gsn_pick_lock) < 1 )
	{
		SEND ("You have no clue of what you're doing.\r\n",ch);
		return;
	}

	if (ch->cooldowns[gsn_pick_lock] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
	if (!has_thieves_tools(ch))
	{
		SEND ("You don't have the right tools for that!\r\n",ch);
		return;
	}
	
    if (arg[0] == '\0')
    {
        SEND ("Pick what?\r\n", ch);
        return;
    }

    WAIT_STATE (ch, skill_table[gsn_pick_lock].beats);

    /* look for guards */
    for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
    {
        if (IS_NPC (gch) && IS_AWAKE (gch) && ch->level + 5 < gch->level)
        {
            act ("$N is standing too close to the lock.",
                 ch, NULL, gch, TO_CHAR);
            return;
        }
    }
	
	if (get_curr_stat(ch, STAT_DEX) >= 15) //dex bonus Upro
		percent += (get_curr_stat(ch, STAT_DEX) - 14);
	
    if (!IS_NPC (ch) && number_percent () > get_skill (ch, gsn_pick_lock) + percent)
    {
        SEND ("You failed.\r\n", ch);
        check_improve (ch, gsn_pick_lock, FALSE, 2);
        return;
    }

    if ((obj = get_obj_here( ch, NULL, arg)) != NULL)
    {
        /* portal stuff */
        if (obj->item_type == ITEM_PORTAL)
        {
            if (!IS_SET (obj->value[1], EX_ISDOOR))
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }

            if (!IS_SET (obj->value[1], EX_CLOSED))
            {
                SEND ("It's not closed.\r\n", ch);
                return;
            }

            if (obj->value[4] < 0)
            {
                SEND ("It can't be unlocked.\r\n", ch);
                return;
            }

            if (IS_SET (obj->value[1], EX_PICKPROOF))
            {
                SEND ("You failed.\r\n", ch);
                return;
            }

            REMOVE_BIT (obj->value[1], EX_LOCKED);
            act ("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
            act ("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
			ch->cooldowns[gsn_pick_lock] = skill_table[gsn_pick_lock].cooldown;
            check_improve (ch, gsn_pick_lock, TRUE, 2);
            return;
        }





        /* 'pick object' */
        if (obj->item_type != ITEM_CONTAINER)
        {
            SEND ("That's not a container.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_CLOSED))
        {
            SEND ("It's not closed.\r\n", ch);
            return;
        }
        if (obj->value[2] < 0)
        {
            SEND ("It can't be unlocked.\r\n", ch);
            return;
        }
        if (!IS_SET (obj->value[1], CONT_LOCKED))
        {
            SEND ("It's already unlocked.\r\n", ch);
            return;
        }
        if (IS_SET (obj->value[1], CONT_PICKPROOF))
        {
            SEND ("You failed.\r\n", ch);
            return;
        }

        REMOVE_BIT (obj->value[1], CONT_LOCKED);
        act ("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
        act ("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
		ch->cooldowns[gsn_pick_lock] = skill_table[gsn_pick_lock].cooldown;
        check_improve (ch, gsn_pick_lock, TRUE, 2);
        return;
    }

    if ((door = find_door (ch, arg)) >= 0)
    {
        /* 'pick door' */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if (!IS_SET (pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL (ch))
        {
            SEND ("It's not closed.\r\n", ch);
            return;
        }
        if (pexit->key < 0 && !IS_IMMORTAL (ch))
        {
            SEND ("It can't be picked.\r\n", ch);
            return;
        }
        if (!IS_SET (pexit->exit_info, EX_LOCKED))
        {
            SEND ("It's already unlocked.\r\n", ch);
            return;
        }
        if (IS_SET (pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL (ch))
        {
            SEND ("You failed.\r\n", ch);
            return;
        }

        REMOVE_BIT (pexit->exit_info, EX_LOCKED);
        SEND ("*Click*\r\n", ch);
        act ("$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
		ch->cooldowns[gsn_pick_lock] = skill_table[gsn_pick_lock].cooldown;
        check_improve (ch, gsn_pick_lock, TRUE, 2);

        /* pick the other side */
        if ((to_room = pexit->u1.to_room) != NULL
            && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
            && pexit_rev->u1.to_room == ch->in_room)
        {
            REMOVE_BIT (pexit_rev->exit_info, EX_LOCKED);
        }
    }

    return;
}




void do_stand (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
        if (ch->position == POS_FIGHTING)
        {
            SEND ("Maybe you should finish fighting first?\r\n", ch);
            return;
        }
        obj = get_obj_list (ch, argument, ch->in_room->contents);
        if (obj == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
        if (obj->item_type != ITEM_FURNITURE
            || (!IS_SET (obj->value[2], STAND_AT)
                && !IS_SET (obj->value[2], STAND_ON)
                && !IS_SET (obj->value[2], STAND_IN)))
        {
            SEND ("You can't seem to find a place to stand.\r\n", ch);
            return;
        }
        if (ch->on != obj && count_users (obj) >= obj->value[0])
        {
            act_new ("There's no room to stand on $p.",
                     ch, obj, NULL, TO_CHAR, POS_DEAD);
            return;
        }
        ch->on = obj;
    }

    switch (ch->position)
    {
        case POS_SLEEPING:
            if (IS_AFFECTED (ch, AFF_SLEEP))
            {
                SEND ("You can't wake up!\r\n", ch);
                return;
            }

            if (obj == NULL)
            {
                SEND ("You wake and stand up.\r\n", ch);
                act ("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
                ch->on = NULL;
            }
            else if (IS_SET (obj->value[2], STAND_AT))
            {
                act_new ("You wake and stand at $p.", ch, obj, NULL, TO_CHAR,
                         POS_DEAD);
                act ("$n wakes and stands at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], STAND_ON))
            {
                act_new ("You wake and stand on $p.", ch, obj, NULL, TO_CHAR,
                         POS_DEAD);
                act ("$n wakes and stands on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act_new ("You wake and stand in $p.", ch, obj, NULL, TO_CHAR,
                         POS_DEAD);
                act ("$n wakes and stands in $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_STANDING;
            do_function (ch, &do_look, "auto");
            break;

        case POS_RESTING:
        case POS_SITTING:
            if (obj == NULL)
            {
                SEND ("You stand up.\r\n", ch);
                act ("$n stands up.", ch, NULL, NULL, TO_ROOM);
                ch->on = NULL;
            }
            else if (IS_SET (obj->value[2], STAND_AT))
            {
                act ("You stand at $p.", ch, obj, NULL, TO_CHAR);
                act ("$n stands at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], STAND_ON))
            {
                act ("You stand on $p.", ch, obj, NULL, TO_CHAR);
                act ("$n stands on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act ("You stand in $p.", ch, obj, NULL, TO_CHAR);
                act ("$n stands on $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_STANDING;
            break;

        case POS_STANDING:
            SEND ("You are already standing.\r\n", ch);
            break;

        case POS_FIGHTING:
            SEND ("You are already fighting!\r\n", ch);
            break;
    }

    return;
}



void do_rest (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
        SEND ("You are already fighting!\r\n", ch);
        return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
        obj = get_obj_list (ch, argument, ch->in_room->contents);
        if (obj == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
    }
    else
        obj = ch->on;

		
		
    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE
            || (!IS_SET (obj->value[2], REST_ON)
                && !IS_SET (obj->value[2], REST_IN)
                && !IS_SET (obj->value[2], REST_AT)))
        {
            SEND ("You can't rest on that.\r\n", ch);
            return;
        }

        if (obj != NULL && ch->on != obj
            && count_users (obj) >= obj->value[0])
        {
            act_new ("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
                     POS_DEAD);
            return;
        }

        ch->on = obj;
		if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
			p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }

    switch (ch->position)
    {
        case POS_SLEEPING:
            if (IS_AFFECTED (ch, AFF_SLEEP))
            {
                SEND ("You can't wake up!\r\n", ch);
                return;
            }

            if (obj == NULL)
            {
                SEND ("You wake up and start resting.\r\n", ch);
                act ("$n wakes up and starts resting.", ch, NULL, NULL,
                     TO_ROOM);
            }
            else if (IS_SET (obj->value[2], REST_AT))
            {
                act_new ("You wake up and rest at $p.",
                         ch, obj, NULL, TO_CHAR, POS_SLEEPING);
                act ("$n wakes up and rests at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], REST_ON))
            {
                act_new ("You wake up and rest on $p.",
                         ch, obj, NULL, TO_CHAR, POS_SLEEPING);
                act ("$n wakes up and rests on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act_new ("You wake up and rest in $p.",
                         ch, obj, NULL, TO_CHAR, POS_SLEEPING);
                act ("$n wakes up and rests in $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_RESTING;
            break;

        case POS_RESTING:
            SEND ("You are already resting.\r\n", ch);
            break;

        case POS_STANDING:
            if (obj == NULL)
            {
                SEND ("You rest.\r\n", ch);
                act ("$n sits down and rests.", ch, NULL, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], REST_AT))
            {
                act ("You sit down at $p and rest.", ch, obj, NULL, TO_CHAR);
                act ("$n sits down at $p and rests.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], REST_ON))
            {
                act ("You sit on $p and rest.", ch, obj, NULL, TO_CHAR);
                act ("$n sits on $p and rests.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act ("You rest in $p.", ch, obj, NULL, TO_CHAR);
                act ("$n rests in $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_RESTING;
            break;

        case POS_SITTING:
            if (obj == NULL)
            {
                SEND ("You rest.\r\n", ch);
                act ("$n rests.", ch, NULL, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], REST_AT))
            {
                act ("You rest at $p.", ch, obj, NULL, TO_CHAR);
                act ("$n rests at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], REST_ON))
            {
                act ("You rest on $p.", ch, obj, NULL, TO_CHAR);
                act ("$n rests on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act ("You rest in $p.", ch, obj, NULL, TO_CHAR);
                act ("$n rests in $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_RESTING;
            break;
    }


    return;
}


void do_sit (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
        SEND ("Maybe you should finish this fight first?\r\n", ch);
        return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
        obj = get_obj_list (ch, argument, ch->in_room->contents);
        if (obj == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
    }
    else
        obj = ch->on;

    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE
            || (!IS_SET (obj->value[2], SIT_ON)
                && !IS_SET (obj->value[2], SIT_IN)
                && !IS_SET (obj->value[2], SIT_AT)))
        {
            SEND ("You can't sit on that.\r\n", ch);
            return;
        }

        if (obj != NULL && ch->on != obj
            && count_users (obj) >= obj->value[0])
        {
            act_new ("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
                     POS_DEAD);
            return;
        }

        ch->on = obj;
		if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
			p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }
    switch (ch->position)
    {
        case POS_SLEEPING:
            if (IS_AFFECTED (ch, AFF_SLEEP))
            {
                SEND ("You can't wake up!\r\n", ch);
                return;
            }

            if (obj == NULL)
            {
                SEND ("You wake and sit up.\r\n", ch);
                act ("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], SIT_AT))
            {
                act_new ("You wake and sit at $p.", ch, obj, NULL, TO_CHAR,
                         POS_DEAD);
                act ("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], SIT_ON))
            {
                act_new ("You wake and sit on $p.", ch, obj, NULL, TO_CHAR,
                         POS_DEAD);
                act ("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act_new ("You wake and sit in $p.", ch, obj, NULL, TO_CHAR,
                         POS_DEAD);
                act ("$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
            }

            ch->position = POS_SITTING;
            break;
        case POS_RESTING:
            if (obj == NULL)
                SEND ("You stop resting.\r\n", ch);
            else if (IS_SET (obj->value[2], SIT_AT))
            {
                act ("You sit at $p.", ch, obj, NULL, TO_CHAR);
                act ("$n sits at $p.", ch, obj, NULL, TO_ROOM);
            }

            else if (IS_SET (obj->value[2], SIT_ON))
            {
                act ("You sit on $p.", ch, obj, NULL, TO_CHAR);
                act ("$n sits on $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_SITTING;
            break;
        case POS_SITTING:
            SEND ("You are already sitting down.\r\n", ch);
            break;
        case POS_STANDING:
            if (obj == NULL)
            {
                SEND ("You sit down.\r\n", ch);
                act ("$n sits down on the ground.", ch, NULL, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], SIT_AT))
            {
                act ("You sit down at $p.", ch, obj, NULL, TO_CHAR);
                act ("$n sits down at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET (obj->value[2], SIT_ON))
            {
                act ("You sit on $p.", ch, obj, NULL, TO_CHAR);
                act ("$n sits on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
                act ("You sit down in $p.", ch, obj, NULL, TO_CHAR);
                act ("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
            }
            ch->position = POS_SITTING;
            break;
    }
    return;
}


void do_sleep (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    switch (ch->position)
    {
        case POS_SLEEPING:
            SEND ("You are already sleeping.\r\n", ch);
            break;

        case POS_RESTING:
        case POS_SITTING:
        case POS_STANDING:
            if (argument[0] == '\0' && ch->on == NULL)
            {
                SEND ("You go to sleep.\r\n", ch);
                act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
                ch->position = POS_SLEEPING;
            }
            else
            {                    /* find an object and sleep on it */

                if (argument[0] == '\0')
                    obj = ch->on;
                else
                    obj = get_obj_list (ch, argument, ch->in_room->contents);

                if (obj == NULL)
                {
                    SEND ("You don't see that here.\r\n", ch);
                    return;
                }
                if (obj->item_type != ITEM_FURNITURE
                    || (!IS_SET (obj->value[2], SLEEP_ON)
                        && !IS_SET (obj->value[2], SLEEP_IN)
                        && !IS_SET (obj->value[2], SLEEP_AT)))
                {
                    SEND ("You can't sleep on that!\r\n", ch);
                    return;
                }

                if (ch->on != obj && count_users (obj) >= obj->value[0])
                {
                    act_new ("There is no room on $p for you.",
                             ch, obj, NULL, TO_CHAR, POS_DEAD);
                    return;
                }

                ch->on = obj;
				if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
					p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
                if (IS_SET (obj->value[2], SLEEP_AT))
                {
                    act ("You go to sleep at $p.", ch, obj, NULL, TO_CHAR);
                    act ("$n goes to sleep at $p.", ch, obj, NULL, TO_ROOM);
                }
                else if (IS_SET (obj->value[2], SLEEP_ON))
                {
                    act ("You go to sleep on $p.", ch, obj, NULL, TO_CHAR);
                    act ("$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM);
                }
                else
                {
                    act ("You go to sleep in $p.", ch, obj, NULL, TO_CHAR);
                    act ("$n goes to sleep in $p.", ch, obj, NULL, TO_ROOM);
                }
                ch->position = POS_SLEEPING;
            }
            break;

        case POS_FIGHTING:
            SEND ("You are already fighting!\r\n", ch);
            break;
    }

    return;
}



void do_wake (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        do_function (ch, &do_stand, "");
        return;
    }

    if (!IS_AWAKE (ch))
    {
        SEND ("You are asleep yourself!\r\n", ch);
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_AWAKE (victim))
    {
        act ("$N is already awake.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (IS_AFFECTED (victim, AFF_SLEEP))
    {
        act ("You can't wake $M!", ch, NULL, victim, TO_CHAR);
        return;
    }

    act_new ("$n wakes you.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
    do_function (victim, &do_stand, "");
    return;
}



void do_sneak (CHAR_DATA * ch, char *argument)
{
    AFFECT_DATA af;

    if (IS_AFFECTED (ch, AFF_SNEAK))
	{
		SEND("You're already sneaking silly!\r\n",ch);
        return;
	}

	SEND ("You attempt to move silently.\r\n", ch);
    affect_strip (ch, gsn_sneak);
		
    if (number_percent () < get_skill (ch, gsn_sneak))
    {
        check_improve (ch, gsn_sneak, TRUE, 3);
        af.where = TO_AFFECTS;
        af.type = gsn_sneak;
        af.level = ch->level;
        af.duration = ch->level;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_SNEAK;
        affect_to_char (ch, &af);
		
		SEND("You begin sulking through the shadows...\r\n",ch);
		check_improve (ch, gsn_sneak, FALSE, 2);
    }
    else
        check_improve (ch, gsn_sneak, FALSE, 1);

    return;
}



void do_hide (CHAR_DATA * ch, char *argument)
{
	int chance = number_percent();

	if (IS_NPC(ch))
		return;

	if (ch->fighting != NULL)
	{
		SEND("You can't hide from anything right now!\r\n",ch);
		return;
	}
	
    SEND ("You attempt to hide.\r\n", ch);

    if (IS_AFFECTED (ch, AFF_HIDE))
	{
        REMOVE_BIT (ch->affected_by, AFF_HIDE);
		SEND("You step out of the shadows...\r\n",ch);
		return;
	}

	//Modifiers:
	chance += ((what_size(ch) - SIZE_MEDIUM) * 10);

	if (IS_ELF(ch))
		chance -= 5;
	if (IS_HALFLING(ch))
		chance -= 10;

	if (time_info.hour < 5 && time_info.hour >= 20) //still dark outside.
		chance -= 5;
	else	//daytime makes it harder.
		chance += 5;
	

    if (chance < get_skill (ch, gsn_hide))
    {
        SET_BIT (ch->affected_by, AFF_HIDE);
		SEND("You slip into the shadows...\r\n",ch);
        check_improve (ch, gsn_hide, TRUE, 2);
    }
    else
	{
        check_improve (ch, gsn_hide, FALSE, 2);
		SEND("You fail to properly hide yourself in your surroundings.\r\n",ch);
	}

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible (CHAR_DATA * ch, char *argument)
{
    affect_strip (ch, gsn_invis);
    affect_strip (ch, gsn_mass_invis);
    affect_strip (ch, gsn_sneak);
	affect_strip (ch, gsn_stone_meld);
    REMOVE_BIT (ch->affected_by, AFF_HIDE);
    REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
    REMOVE_BIT (ch->affected_by, AFF_SNEAK);
    SEND ("Ok.\r\n", ch);
    return;
}



void do_recall (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC (ch) && !IS_SET (ch->act, ACT_PET))
    {
        SEND ("Only players can recall.\r\n", ch);
        return;
    }
	
	if (argument[0] != '\0' && !str_cmp(argument, "here"))
	{
		if (ch->in_room->sector_type != SECT_CITY)
		{
			SEND("You must be in the city to do this.\r\n",ch);
			return;
		}
		
		SEND ("This will be your new recall point.\r\n",ch);
		ch->pcdata->recall = ch->in_room->vnum;
		return;
	}
	
	if (ch->cooldowns[gsn_recall] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
	if (ch->position == POS_FIGHTING)
	{
		SEND ("Not in combat!\r\n",ch);
		return;
	}
	
	if (ch->pk_timer > 0)
	{
		SEND ("Give it a moment.\r\n",ch);
		return;
	}
	
	if (ch->onBoard)
	{
		SEND ("You can't recall while on a ship.\r\n",ch);
		return;
	}
	
    act ("$n prays for magical transportation...", ch, 0, 0, TO_ROOM);
		
	if ((location = get_room_index (ch->pcdata->recall)) == NULL)
	{
		SEND ("You are completely lost.\r\n", ch);
		return;
	}	

    if (ch->in_room == location)
        return;

    if (IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
        || IS_AFFECTED (ch, AFF_CURSE))
    {
        		
		sprintf (buf, "%s has forsaken you at this time.\r\n", god_table[ch->god].name);
        SEND (buf, ch);
        return;
    }

    if ((victim = ch->fighting) != NULL)
    {
        int lose;		

        if (number_percent () < 15)
        {            
            WAIT_STATE (ch, 4);            
            SEND ("You failed!\r\n", ch);
            return;
        }

        lose = number_range(150,200);
        gain_exp (ch, 0 - lose, FALSE);
		//Give lowbies unlimited recalling.
        if (total_levels(ch) > 7)
			COOLDOWN(ch, gsn_recall);
        sprintf (buf, "You recall from combat!  You lose {G%d{x experience points.\r\n", lose);
        SEND (buf, ch);
        stop_fighting (ch, TRUE);

    }

    ch->move /= 2;	
    act ("$n disappears in a crackling {bblue{x portal!", ch, NULL, NULL, TO_ROOM);
	ch->cooldowns[gsn_recall] = skill_table[gsn_recall].cooldown;	
    char_from_room (ch);
    char_to_room (ch, location);
    act ("$n appears from a crackling {bblue{x portal!", ch, NULL, NULL, TO_ROOM);
    do_function (ch, &do_look, "auto");

    if (ch->pet != NULL)
        do_function (ch->pet, &do_recall, "");

    return;
}


void do_heal (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost, sn;
    SPELL_FUN *spell;
    char *words;

    /* check for healer */
    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
        if (IS_NPC (mob) && IS_SET (mob->act, ACT_IS_HEALER))
            break;
    }

    if (mob == NULL)
    {
        SEND ("You can't do that here.\r\n", ch);
        return;
    }

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        /* display price list */
        act ("$N says 'I offer the following spells:'", ch, NULL, mob,
             TO_CHAR);
		SEND ("\r\n",ch);
		SEND ("  TYPE:     SPELL:                  COST (in gold):\r\n", ch);
		SEND ("  {r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n", ch);
        SEND ("  light     cure light wounds                    2\r\n", ch);
        SEND ("  serious   cure serious wounds                  6\r\n", ch);
        SEND ("  critic    cure critical wounds                10\r\n", ch);
        SEND ("  heal      healing spell                       15\r\n", ch);
        SEND ("  blind     cure blindness                      10\r\n", ch);
        SEND ("  disease   cure disease                        10\r\n", ch);
        SEND ("  poison    cure poison                          5\r\n", ch);
        SEND ("  uncurse   remove curse                        20\r\n", ch);
        SEND ("  refresh   restore movement                     1\r\n", ch);
        SEND ("  mana      restore mana                         7\r\n", ch);	
		SEND ("\r\n",ch);
        SEND ("Syntax:  heal {r<type>{x\r\n", ch);
		SEND ("Example: heal {rrefresh{x\r\n", ch);
        return;
    }

    if (!str_prefix (arg, "light"))
    {
        spell = spell_cure_light;
        sn = skill_lookup ("cure light");
        words = "judicandus dies";
        cost = 200;
    }

    else if (!str_prefix (arg, "serious"))
    {
        spell = spell_cure_serious;
        sn = skill_lookup ("cure serious");
        words = "judicandus gzfuajg";
        cost = 600;
    }

    else if (!str_prefix (arg, "critical"))
    {
        spell = spell_cure_critical;
        sn = skill_lookup ("cure critical");
        words = "judicandus qfuhuqar";
        cost = 1000;
    }

    else if (!str_prefix (arg, "heal"))
    {
        spell = spell_heal;
        sn = skill_lookup ("heal");
        words = "pzar";
        cost = 1500;
    }

    else if (!str_prefix (arg, "blindness"))
    {
        spell = spell_cure_blindness;
        sn = skill_lookup ("cure blindness");
        words = "judicandus noselacri";
        cost = 1000;
    }

    else if (!str_prefix (arg, "disease"))
    {
        spell = spell_cure_disease;
        sn = skill_lookup ("cure disease");
        words = "judicandus eugzagz";
        cost = 1000;
    }

    else if (!str_prefix (arg, "poison"))
    {
        spell = spell_cure_poison;
        sn = skill_lookup ("cure poison");
        words = "judicandus sausabru";
        cost = 500;
    }

    else if (!str_prefix (arg, "uncurse") || !str_prefix (arg, "curse"))
    {
        spell = spell_remove_curse;
        sn = skill_lookup ("remove curse");
        words = "candussido judifgz";
        cost = 2000;
    }

    else if (!str_prefix (arg, "mana") || !str_prefix (arg, "energize"))
    {
        spell = NULL;
        sn = -1;
        words = "nezregier"; //used to be energizer ;) Upro 1/7/2020
        cost = 700;
    }


    else if (!str_prefix (arg, "refresh") || !str_prefix (arg, "moves"))
    {
        spell = spell_refresh;
        sn = skill_lookup ("refresh");
        words = "candusima";
        cost = 100;
    }

    else
    {
        act ("$N says 'Type 'heal' for a list of spells.'",
             ch, NULL, mob, TO_CHAR);
        return;
    }

    if (cost > (ch->gold * 100 + ch->silver))
    {
        act ("$N says 'You do not have enough gold for my services.'",
             ch, NULL, mob, TO_CHAR);
        return;
    }

    WAIT_STATE (ch, PULSE_VIOLENCE);

    deduct_cost (ch, cost);
    mob->gold += cost / 100;
    mob->silver += cost % 100;
    act ("$n utters the words '$T'.", mob, NULL, words, TO_ROOM);

    if (spell == NULL)
    {                            /* restore mana trap...kinda hackish */
        ch->mana += dice (2, 8) + mob->level / 3;
        ch->mana = UMIN (ch->mana, ch->max_mana);
        SEND ("A warm glow passes through you.\r\n", ch);
        return;
    }

    if (sn == -1)
        return;

    spell (sn, mob->level, mob, ch, TARGET_CHAR);
}



/*
void do_grank (CHAR_DATA * ch, char *argument)
{
	int count = 0;
	char buf[MSL];
	CHAR_DATA * vch;
	CHAR_DATA * vch_next;
	
	count = group_size(ch);
	
	if (argument[0] == '\0')
    {
		SEND ("Syntax: grank <front/back/flank>\r\n",ch);
		return;
	}
	
	//if ( count < 2 )
	if (!ch->leader && count < 1)
	{
		SEND ("You can't switch ranks unless you're in a group.\r\n",ch);
		return;
	}
	
	if (!str_cmp(argument, "front"))
	{	
		for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( is_same_group( ch, vch ) && vch->grank == G_FRONT)
			{
				if (vch != ch)
				{
					sprintf(buf, "%s is already leading the group.\r\n",vch->name);
					SEND (buf,ch);
				}
				else
					SEND ("You're already leading the group!\r\n",ch);
				return;
			}
		}	
		SEND ("You move to the front of the ranks.\r\n",ch);
		act ("$n moves to take the lead.", ch, NULL, NULL, TO_ROOM);
		ch->grank = G_FRONT;
		return;
	}
	
	if (!str_cmp(argument, "back"))
	{		
		if (ch->grank == G_FRONT)
		{			
			for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( is_same_group( ch, vch ) && ch != vch)
				{
					vch->grank = G_FRONT;
					break;
				}
			}
		}
		SEND ("You move to the back of the ranks.\r\n",ch);
		act ("$n moves to the back of the group.", ch, NULL, NULL, TO_ROOM);		
		ch->grank = G_BACK;
		return;
	}
	
	if (!str_cmp(argument, "flank"))
	{
		if (ch->grank == G_FRONT)
		{			
			for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( is_same_group( ch, vch ) && ch != vch)
				{
					vch->grank = G_FRONT;
					break;
				}
			}
		}
		SEND ("You begin flanking the group.\r\n",ch);
		act ("$n moves to take the flank.", ch, NULL, NULL, TO_ROOM);
		ch->grank = G_FLANK;
		return;
	}
}
*/

void do_nudge(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	
	one_argument(argument, arg);
	
	if ( arg[0] == '\0' )
	{
		SEND("Nudge whom?\r\n",ch);
		return;
	}    

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    { 
		SEND( "They aren't here.\r\n",ch ); 
		return; 
    }
    
    if (victim->position == POS_SLEEPING)
    {
		SEND("You feel someone nudging you.\r\n", victim);
		act("$n nudges $N.", ch, NULL, victim, TO_ROOM);
		act("You nudge $N.", ch, NULL, victim, TO_CHAR);
    }
    else
		act("$N isn't sleeping", ch, NULL, victim, TO_CHAR);

    return;
	
}

void do_bury (CHAR_DATA *ch, char*argument)
{
	char arg1[MIL];
	char buf[MSL];
	int i;	
	OBJ_DATA *shovel = NULL;	
	OBJ_DATA *obj = NULL;
	
	bool found = FALSE;

	if (IS_NPC(ch))
		return;

	argument = one_argument( argument, arg1 );	
	
	if (IS_AFFECTED(ch,AFF_CHARM))
	{
		SEND( "You don't have the free will to do so!\r\n",ch );
		return;
	}
	
	for ( i = 0; i < MAX_WEAR; i++ )
	{
		if ( ( shovel = get_eq_char( ch, i ) ) == NULL )
			continue;
		
		if (shovel && shovel->item_type == ITEM_SHOVEL)
		{
			found = TRUE;
			break;
		}		
	}
	
	
	if (!shovel || !found)
	{
		SEND("You need something to bury it with!\r\n",ch);
		return;
	}	
	
	found = FALSE;
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{        
		if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
			&& can_see_obj (ch, obj)
			&& obj->wear_loc == WEAR_NONE && can_drop_obj (ch, obj))
		{
			found = TRUE;
			break;
		}
	}
	
	if (!found)
	{				
		SEND ("Invalid or nonexistent item.\r\n", ch);
		return;
	}
	
	if (ch->event == EVENT_NONE)
	{	
	
		if (!IN_DIGGING_SECTOR(ch))
		{
			SEND("You can't bury that here.\r\n",ch);
			return;
		}			
		sprintf(buf, "You bury in %s the dirt.\r\n",obj->short_descr);
		SEND(buf, ch);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		obj->altitude = number_range(-5,-2);
		return;
	}
	else
	{
		SEND("You're already in the middle of something!\r\n",ch);
		return;
	}

	return;
}

void do_dig (CHAR_DATA *ch, char *argument)
{
	char arg1 [MIL];
	int i;	
	OBJ_DATA *shovel = NULL;	
	bool found = FALSE;

	if (IS_NPC(ch))
		return;

	argument = one_argument( argument, arg1 );	
	
	if (IS_AFFECTED(ch,AFF_CHARM))
	{
		SEND( "You don't have the free will to do so!\r\n",ch );
		return;
	}
	
	
	
	for ( i = 0; i < MAX_WEAR; i++ )
	{
		if ( ( shovel = get_eq_char( ch, i ) ) == NULL )
			continue;
		
		if (shovel && shovel->item_type == ITEM_SHOVEL)
		{
			found = TRUE;
			break;
		}		
	}
	
	
	if (!shovel || !found)
	{
		SEND("You need something to dig with!\r\n",ch);
		return;
	}
	
	
		
	if (!str_cmp (arg1, "stop"))
	{
		if (ch->event != EVENT_DIGGING)
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
	
	if (ch->event == EVENT_NONE)
	{	
	
		if (!IN_DIGGING_SECTOR(ch))
		{
			SEND("You can't dig here.\r\n",ch);
			return;
		}	
		ch->event = EVENT_DIGGING;		
		SEND("You begin digging in the dirt.\r\n",ch);
		return;
	}
	else
	{
		SEND("You're already in the middle of something!\r\n",ch);
		return;
	}

	return;

}



void do_arena(CHAR_DATA *ch, char *argument)
{
	
	char arg[MIL];
	char buf[MSL];
	
	argument = one_argument(argument, arg);
	
	if ( arg[0] == '\0' )
	{
		SEND ("Specify a bit more please.\r\n",ch);
		return;
	}
	
	if (!str_cmp(arg, "join"))
	{
		if (ch->arena)
		{
			SEND("You are already in the queue.\r\n",ch );
			return;
		}
		SEND ("You join the queue for arena combat.\r\n",ch);
		ch->arena = TRUE;
		sprintf (buf, "There are %d people in the queue (including you).\r\n", get_num_queued());
		SEND (buf, ch);
		//Message to the world that they joined the arena.		
		return;
	}
	
	if (!str_cmp(arg, "leave"))
	{
		if (ch->arena == TRUE)
		{
			SEND ("You leave the arena queue.\r\n",ch);		
			ch->arena = FALSE;
		}
		else
			SEND ("You're not even in the arena queue.\r\n",ch);		
			
		return;
	}
	
	return;	
}




//Check to see if a vnum has been explored
bool explored_vnum(CHAR_DATA *ch, int vnum)
{	int mask = vnum / 32; //Get which bucket the bit is in
	unsigned int bit = vnum % 32; //Get which bit in the bucket we're playing with
	EXPLORE_HOLDER *pExp; //The buckets bucket.
 
	if(bit == 0 ) // % 32 will return 0 if vnum == 32, instead make it the last bit of the previous mask
	{	mask--;
		bit = 32;
	}
 
	for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next ) //Iterate through the buckets
	{	if(pExp->mask != mask)
			continue;
		//Found the right bucket, might be explored.
		if(IS_SET(pExp->bits, ( 1 << bit ) ) ) //Convert bit to 2^(bit-1) and see if it's set.
			return TRUE;
		return FALSE; //Return immediately. This value wont be in any other bucket.
	}
	return FALSE;
}
//Explore a vnum. Assume it's not explored and just set it.
void explore_vnum(CHAR_DATA *ch, int vnum )
{	int mask = vnum / 32; //Get which bucket it will be in
	unsigned int bit = vnum % 32; // Get which bit to set
	EXPLORE_HOLDER *pExp; //The buckets bucket.
 
	if(bit == 0 ) // % 32 will return 0 if vnum is a multiple 32, instead make it the last bit of the previous mask
	{	mask--;
		bit = 32;
	}
 
	//Find the bucket.
	for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
		if(pExp->mask == mask)
			break;
 
	if(!pExp) //If it's null, bucket not found, we'll whip one up.
	{	pExp = (EXPLORE_HOLDER *)calloc(sizeof(*pExp), 1); //Alloc and zero
		pExp->mask = mask;
		pExp->next = ch->pcdata->explored->bits; //Add to
		ch->pcdata->explored->bits = pExp;       //the list
	}
 
	SET_BIT(pExp->bits, ( 1 << bit ) ); //Convert bit to 2^(bit-1) and set
	ch->pcdata->explored->set++; //Tell how many rooms we've explored
	give_exp(ch, number_range(1,10), FALSE);
}
 
 
//Explore a vnum.
void check_explore( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom )
{	if(IS_NPC(ch) ) return;
 
 
	if(explored_vnum(ch, pRoom->vnum) )
		return;
 
	explore_vnum(ch, pRoom->vnum);	
	/*if (ch->pcdata->explored->set > game_data.explored)
	{
		game_data.explored = ch->pcdata->explored->set;
		game_data.explorer = ch->name;
		save_game_data();
	}*/
}
 
void do_explored(CHAR_DATA *ch, char *argument )
{	
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "You have explored %d room%s!{x\r\n", ch->pcdata->explored->set, (ch->pcdata->explored->set == 1 ? "" : "s") );
	SEND(buf,ch);

 
//This shows all the rooms they've explored. Probably don't need mortals seeing this, and for immortals... it'd get really spammy. Mostly an example.
	
	
/*	for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
	{	for(bit = 1 ; bit <= 32 ; ++bit )
		{	if(IS_SET(pExp->bits, (1 << bit) ) )
			{	sprintf(buf, "[%-5d]", (pExp->mask * 32 + bit) );
				SEND(buf,ch);
			}
		}
		SEND("\r\n",ch);
	}
*/
	return;
}


void do_land (CHAR_DATA *ch, char *argument)
{	
	if (is_affected(ch, gsn_fly))
	{
		affect_strip (ch, gsn_fly);
		SEND("You return to the ground.\r\n",ch);
		return;
	}
	else
		SEND("But you're already grounded!\r\n",ch);
	
	return;
}



void do_capture (CHAR_DATA *ch, char *argument)
{
	if (argument[0] == '\0')
	{
		SEND ("Capture what?\r\n",ch);
		return;
	}
	
	SEND ("Still in development.\r\n",ch);
	return;
}

void do_fly (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	
	if (!IS_BIRDFOLK(ch))
	{
		SEND("With what wings?!\r\n",ch);
		return;
	}
	
	if (argument[0] != '\0')
	{
		SEND ("Just type fly, nothing more or less\r\n",ch);
		return;
	}
	
	if (IS_AFFECTED(ch, gsn_fly))
	{
		affect_strip(ch, gsn_fly);
		SEND ("You slowly descend to the ground.\r\n",ch);
		act ("$n folds their wings in and descends to the ground.", ch, NULL, NULL, TO_ROOM);
		return;
	}
	else
	{
		af.where = TO_AFFECTS;		
		af.level = ch->level;
		af.type = gsn_fly;
		af.duration = -1;	
		af.location = 0;
		af.modifier = 0;
		af.bitvector = gsn_fly;
		affect_to_char (ch, &af);
		SEND ("Your enormous wings extend out to allow you to fly.\r\n", ch);
		act ("$n extends out their wings and begins flying.", ch, NULL, NULL, TO_ROOM);
		return;
	}
	
	return;
}