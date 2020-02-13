
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
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/*
 * Local functions.
 */
void say_spell 			args ((CHAR_DATA * ch, int sn));
bool critical_spell 	args ((CHAR_DATA *ch));

/* imported functions */
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
bool    check_dispel( int dis_level, CHAR_DATA *victim, int sn );
bool is_devotion 		args ((int sn));

char *flag_string		args ( ( const struct flag_type *flag_table,
				         int bits ) );

extern char *target_name;
extern int  find_door       args( ( CHAR_DATA *ch, char *arg ) );

bool can_cast_spell(CHAR_DATA *ch, int sn)
{
	int primary_clevel = 0;
	int secondary_clevel = 0;
	bool can = FALSE;
	
	if (IS_IMMORTAL(ch))
		return TRUE;
	
	if (!IS_MCLASSED(ch) && (IS_FIGHTER(ch) || IS_THIEF(ch)))
		return FALSE;
	
	
	
	if (IS_MCLASSED(ch))
	{
		if ( ch_class_table[ch-> ch_class].can_cast != TRUE)
		{
			if ( ch_class_table[ch-> ch_class2].can_cast != TRUE)
				can = FALSE;
			else
				can = TRUE;
		}
		else
			can = TRUE;
	}
	
	//Determine casting level (and secondary casting level if they're mch_classed.)
	if (!IS_MCLASSED(ch) && ch_class_table[ch-> ch_class].can_cast == TRUE)
	{
		primary_clevel = ch->level;
		can = TRUE;
	}	
	
	if (IS_MCLASSED(ch))
	{
		//Main ch_class is caster, other is not.
		if (( ch_class_table[ch-> ch_class].can_cast == TRUE) && ( ch_class_table[ch-> ch_class2].can_cast != TRUE))
		{
			primary_clevel = ch->level;
			secondary_clevel = 0;
		}		
		//Main ch_class is caster, and secondary is too
		else if (( ch_class_table[ch-> ch_class].can_cast == TRUE) && ( ch_class_table[ch-> ch_class2].can_cast == TRUE))
		{
			primary_clevel = ch->level;
			secondary_clevel = ch->level2;
		}		
		//Main ch_class is not caster, and neither is secondary
		else if (( ch_class_table[ch-> ch_class].can_cast != TRUE) && ( ch_class_table[ch-> ch_class2].can_cast != TRUE))
		{
			primary_clevel = 0;
			secondary_clevel = 0;
		}		
		//Only secondary ch_class is caster.
		else
		{
			primary_clevel = 0;
			secondary_clevel = ch->level2;
		}		
	}
	
	//Check whether they can cast that level of spell. (0-9 like D&D)
	switch (skill_table[sn].spell_level)
	{
		case 0:
		case 1:
			if (primary_clevel > 0 || secondary_clevel > 0)
				can = TRUE; //Any level 1 or above caster can use these.
			else
				can = FALSE;
			break;
		case 2:
			if (primary_clevel < 4 && secondary_clevel < 4)
				can = FALSE;
			break;
		case 3:
			if (primary_clevel < 7 && secondary_clevel < 7)
				can = FALSE;
			break;
		case 4:
			if (primary_clevel < 11 && secondary_clevel < 11)
				can = FALSE;
			break;
		case 5:
			if (primary_clevel < 15 && secondary_clevel < 15)
				can = FALSE;
			break;
		case 6:
			if (primary_clevel < 19 )
				can = FALSE;
			break;
		case 7:
			if (primary_clevel < 23 )
				can = FALSE;
			break;
		case 8:
			if (primary_clevel < 27 )
				can = FALSE;
			break;
		case 9:
			if (primary_clevel < 31 )
				can = FALSE;
			break;
		
		default: 				
			can = FALSE;
			break;
	}
	
	return can;
}


/*
 * Lookup a skill by name.
 */
int skill_lookup (const char *name)
{
    int sn;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
        if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
            && !str_prefix (name, skill_table[sn].name))
            return sn;
    }

    return -1;
}

int find_spell (CHAR_DATA * ch, const char *name)
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC (ch))
        return skill_lookup (name);

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
        if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
            && !str_prefix (name, skill_table[sn].name))
        {
            if (found == -1)
                found = sn;
			
            if (can_cast_spell(ch, sn) && KNOWS(ch, sn))
				return sn;
        }
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup (int slot)
{
    extern bool fBootDb;
    int sn;

    if (slot <= 0)
        return -1;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (slot == skill_table[sn].slot)
            return sn;
    }

    if (fBootDb)
    {
        bug ("Slot_lookup: bad slot %d.", slot);
        abort ();
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell (CHAR_DATA * ch, int sn)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type {
        char *old;
        char *new;
    };

    static const struct syl_type syl_table[] = {
        {" ", " "},
        {"ar", "abra"},
        {"au", "kada"},
        {"bless", "fido"},
        {"blind", "nose"},
        {"bur", "mosa"},
        {"cu", "judi"},
        {"de", "oculo"},
        {"en", "unso"},
        {"light", "dies"},
        {"lo", "hi"},
        {"mor", "zak"},
        {"move", "sido"},
        {"ness", "lacri"},
        {"ning", "illa"},
        {"per", "duda"},
        {"ra", "gru"},
        {"fresh", "ima"},
        {"re", "candus"},
        {"son", "sabru"},
        {"tect", "infra"},
        {"tri", "cula"},
        {"ven", "nofo"},
        {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
        {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
        {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
        {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
        {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
        {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
        {"y", "l"}, {"z", "k"},
        {"", ""}
    };

    buf[0] = '\0';
    for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
    {
        for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
        {
            if (!str_prefix (syl_table[iSyl].old, pName))
            {
                strcat (buf, syl_table[iSyl].new);
                break;
            }
        }

        if (length == 0)
            length = 1;
    }

    sprintf (buf2, "$n utters the words, '%s'.", buf);
    sprintf (buf, "$n utters the words, '%s'.", skill_table[sn].name);

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
        if (rch != ch)
            act ((!IS_NPC (rch) && ch-> ch_class == rch-> ch_class) ? buf : buf2,
                 ch, NULL, rch, TO_VICT);
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell (int level, CHAR_DATA * victim, int dam_type)
{
    int save;

    save = 50 + (victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED (victim, AFF_BERSERK))
        save += victim->level / 2;

    switch (check_immune (victim, dam_type))
    {
        case IS_IMMUNE:
            return TRUE;
        case IS_RESISTANT:
            save += 2;
            break;
        case IS_VULNERABLE:
            save -= 2;
            break;
    }

    if (!IS_NPC (victim) && ch_class_table[victim-> ch_class].fMana)
        save = 9 * save / 10;
    save = URANGE (5, save, 95);
    return number_percent () < save;
}

/* RT save for dispels */

bool saves_dispel (int dis_level, int spell_level, int duration)
{
    int save;

    if (duration == -1)
        spell_level += 5;
    /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE (5, save, 95);
    return number_percent () < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel (int dis_level, CHAR_DATA * victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected (victim, sn))
    {
        for (af = victim->affected; af != NULL; af = af->next)
        {
            if (af->type == sn)
            {
                if (!saves_dispel (dis_level, af->level, af->duration))
                {
                    affect_strip (victim, sn);
                    if (skill_table[sn].msg_off)
                    {
                        SEND (skill_table[sn].msg_off, victim);
                        SEND ("\r\n", victim);
                    }
                    return TRUE;
                }
                else
                    af->level--;
            }
        }
    }
    return FALSE;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;
	//int chance = 40; //chance of spell failure

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if (IS_NPC (ch) && ch->desc == NULL && !is_affected(ch, gsn_silence))
        return;

    target_name = one_argument (argument, arg1);
    one_argument (target_name, arg2);

    if (arg1[0] == '\0')
    {
        SEND ("Cast which what where?\r\n", ch);
        return;
    }

	
	switch (ch->in_room->sector_type)
	{
		default: break;
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		case SECT_OCEANFLOOR:
			if (!is_affected(ch, gsn_water_breathing) && !IS_IMMORTAL(ch))
			{
				SEND("But you need your hands to swim right now...\r\n",ch);
				return;
			}
			break;
	}
	
	
	if (is_affected (ch, gsn_silence) && !IS_IMMORTAL(ch))
	{
		SEND("You can't cast anything, you're silenced!\r\n",ch);
		return;
	}
		
	if ((IS_FIGHTER(ch) || IS_THIEF(ch)) && !IS_MCLASSED(ch))	
	{
		SEND( "You don't have any clue how to cast spells.\r\n",ch);
		return;
	}
		
	if ((sn = find_spell (ch, arg1)) < 1			//spell specified doesn't exist.
		|| skill_table[sn].spell_fun == spell_null	//it's a skill, not a spell.
		|| !KNOWS(ch,sn)							//they don't know it.
		|| skill_table[sn].spell_level < 0) 		//not a valid spell.
	{
		SEND ("You don't know any spells of that name.\r\n", ch);
		return;
	}
	
    if (ch->position < skill_table[sn].minimum_position)
    {
        SEND ("You can't concentrate enough.\r\n", ch);
        return;
    }
	
	if (ch->cooldowns[sn] > 0 && !IS_IMMORTAL(ch))
	{
		SEND ("You must wait to cast that spell again.\r\n",ch);
		return;
	}
	
	if (IS_SET (ch->in_room->room_flags, ROOM_ANTI_MAGIC))
	{
		SEND( "You utter the words..... but nothing happens.\r\n",ch);
		return;
	}
		
	/*
	//Intelligence modifiers:
	chance -= (ch->level / 5);
	if (get_curr_stat(ch, STAT_INT) >= 13)
		chance -= ((get_curr_stat(ch, STAT_INT) - 12) * 2);
	if (get_curr_stat(ch, STAT_INT) <= 10)
		chance += (25 - get_curr_stat(ch, STAT_INT));	
	
	//fighters/thieves have a harder time unless multich_classed.
	if ((IS_FIGHTER(ch) || IS_THIEF(ch)) && !IS_MCLASSED(ch))			
		chance += 15;
		
	if (chance > number_range(1,100))
	{
		SEND("You stumbled over the words, the casting failed!\r\n",ch);
		return;
	}
	*/
	
	
	mana = skill_table[sn].min_mana;
	
    // if (ch->level + 2 == skill_table[sn].skill_level[ch-> ch_class])
        // mana = 40;
    // else
        // mana = UMAX (skill_table[sn].min_mana,
                     // 100 / (2 + (total_levels(ch) - );

	
	
    /*
     * Locate targets.
     */
    victim = NULL;
    obj = NULL;
    vo = NULL;
    target = TARGET_NONE;

    switch (skill_table[sn].target)
    {
        default:
            bug ("Do_cast: bad target for sn %d.", sn);
            return;

        case TAR_IGNORE:
            break;

        case TAR_CHAR_OFFENSIVE:
            if (arg2[0] == '\0')
            {
                if ((victim = ch->fighting) == NULL)
                {
                    SEND ("Cast the spell on whom?\r\n", ch);
                    return;
                }
            }
            else
            {
                if ((victim = get_char_room ( ch, NULL, target_name)) == NULL)
                {
                    SEND ("They aren't here.\r\n", ch);
                    return;
                }
            }
/*
        if ( ch == victim )
        {
            SEND( "You can't do that to yourself.\r\n", ch );
            return;
        }
*/


            if (!IS_NPC (ch))
            {

                if (is_safe (ch, victim) && victim != ch)
                {
                    SEND ("Not on that target.\r\n", ch);
                    return;
                }
                check_killer (ch, victim);
            }

            if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
            {
                SEND ("You can't do that on your own follower.\r\n",
                              ch);
                return;
            }
			
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_DEFENSIVE:
            if (arg2[0] == '\0')
            {
                victim = ch;
            }
            else
            {
                if ((victim = get_char_room ( ch, NULL, target_name)) == NULL)
                {
                    SEND ("They aren't here.\r\n", ch);
                    return;
                }
            }

            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_SELF:
            if (arg2[0] != '\0' && !is_name (target_name, ch->name))
            {
                SEND ("You cannot cast this spell on another.\r\n",
                              ch);
                return;
            }

            vo = (void *) ch;
            target = TARGET_CHAR;
            break;

        case TAR_OBJ_INV:
            if (arg2[0] == '\0')
            {
                SEND ("What should the spell be cast upon?\r\n", ch);
                return;
            }

            if ((obj = get_obj_carry (ch, target_name, ch)) == NULL)
            {
                SEND ("You are not carrying that.\r\n", ch);
                return;
            }

            vo = (void *) obj;
            target = TARGET_OBJ;
            break;

        case TAR_OBJ_CHAR_OFF:
            if (arg2[0] == '\0')
            {
                if ((victim = ch->fighting) == NULL)
                {
                    SEND ("Cast the spell on whom or what?\r\n", ch);
                    return;
                }

                target = TARGET_CHAR;
            }
            else if ((victim = get_char_room ( ch, NULL, target_name)) != NULL)
            {
                target = TARGET_CHAR;
            }

            if (target == TARGET_CHAR)
            {                    /* check the sanity of the attack */
                if (is_safe_spell (ch, victim, FALSE) && victim != ch)
                {
                    SEND ("Not on that target.\r\n", ch);
                    return;
                }

                if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
                {
                    SEND
                        ("You can't do that on your own follower.\r\n", ch);
                    return;
                }

                if (!IS_NPC (ch))
                    check_killer (ch, victim);

                vo = (void *) victim;
            }
            else if ((obj = get_obj_here( ch, NULL, target_name)) != NULL)
            {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            else
            {
                SEND ("You don't see that here.\r\n", ch);
                return;
            }
            break;

        case TAR_OBJ_CHAR_DEF:
            if (arg2[0] == '\0')
            {
                vo = (void *) ch;
                target = TARGET_CHAR;
            }
            else if ((victim = get_char_room ( ch, NULL, target_name)) != NULL)
            {
                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else if ((obj = get_obj_carry (ch, target_name, ch)) != NULL)
            {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            else
            {
                SEND ("You don't see that here.\r\n", ch);
                return;
            }
            break;
    }

	if (!IS_NPC(ch)) //So NPC's get endless mana??
	{
		if (is_devotion(sn))
		{
			if (ch->pp < mana)
			{
				SEND ("You need to be able to focus more to do that.\r\n",ch);
				return;
			}
		}		
		else if (is_affected(ch, gsn_manavert))
		{
			if (ch->hit < mana)
			{	
				SEND ("You don't have enough health.\r\n",ch);
				return;
			}
		}
		else
		{		
			if (ch->mana < mana)
			{
				SEND ("You don't have enough mana.\r\n", ch);
				return;
			}
		}
	}

    if (str_cmp (skill_table[sn].name, "ventriloquate"))
        say_spell (ch, sn);
	
	if (!IS_IMMORTAL(ch))
		WAIT_STATE (ch, skill_table[sn].beats);
	
	
	//Casting modifiers:
	int int_mod = 0;
	int i = 18;	

	for (i = 18; i <= 25; i++)
	{
		if (get_curr_stat(ch, STAT_INT) >= i)
			int_mod++;
	}
	
	//Strip globe of invulnerability; concentration is broken
	//because they are casting something else.
	if (is_affected(ch, gsn_globe_invulnerability))
	{
		affect_strip(ch, gsn_globe_invulnerability);
		SEND ("Casting another spell has broken your concentration!\r\n",ch);
		SEND ("Your globe of invulnerability shimmers then fades!\r\n",ch);
		act ("$n's globe of energy shimmers then fades!", ch, NULL, NULL, TO_ROOM);		
	}
	
    if (number_percent () - int_mod > get_skill (ch, sn))
    {
        SEND ("You lost your concentration.\r\n", ch);
        check_improve (ch, sn, FALSE, 1);
        if (is_affected(ch, gsn_manavert))
			ch->hit -= mana / 2;
		else
			ch->mana -= mana / 2;
    }
    else
    {
		int level = 0;
		level = total_levels(ch);
		
		if (GET_INT(ch) > 22)
			level++;
			
		if (get_skill(ch, gsn_spellcraft) > 1 && number_percent() < get_skill(ch, gsn_spellcraft))
		{
			level += number_range(1,2);
			check_improve (ch, gsn_spellcraft, TRUE, 3);
		}
		if (is_affected(ch, gsn_harmonic_chorus))
			level += number_range(1,2);		
		
		if (!is_devotion(sn))
		{
		
			if (is_affected(ch, gsn_manavert))
			{
				SEND("Test!\r\n",ch);
				ch->hit -= mana;
			}
			else
				ch->mana -= mana;
		}
		else
		{
			ch->pp -= mana;
		}
		
        if (IS_NPC (ch) || ch_class_table[ch-> ch_class].fMana)
		{
            /* ch_class has spells */
		    (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);
			ch->cooldowns[sn] = skill_table[sn].cooldown;
		}
        else
		{
            (*skill_table[sn].spell_fun) (sn, 3 * level / 4, ch, vo, target);
			ch->cooldowns[sn] = skill_table[sn].cooldown;
		}	
        check_improve (ch, sn, TRUE, 1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
         || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
             && target == TARGET_CHAR)) && victim != ch
        && victim->master != ch)
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        for (vch = ch->in_room->people; vch; vch = vch_next)
        {
            vch_next = vch->next_in_room;
            if (victim == vch && victim->fighting == NULL)
            {
                check_killer (victim, ch);
                multi_hit (victim, ch, TYPE_UNDEFINED);
                break;
            }
        }
    }

    return;
}

SPELL(spell_lightning_forge)
{
	CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf, af; 
    int result, fail;
    int hit_bonus, dam_bonus;
    int dam = 0;

    if ( IS_IMMORTAL(ch) && get_trust(ch) < 49 )
    {
	SEND("If you can't set it, you can't do it.\n\r",ch);
	return;
    }
    if (obj->item_type != ITEM_WEAPON)
    {
	SEND("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	SEND("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
		obj = (OBJ_DATA *) vo;
		if ( (IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL)) )
		{
			act("$p radiates a dim glow, negating your magic.",ch,obj,NULL,TO_CHAR);
			return;
		}
    }

    if (IS_WEAPON_STAT(obj,WEAPON_LIGHTNING_DAM))
    {
            act("$p already crackles with electricity.",ch,obj,NULL,TO_CHAR);
            return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

/******
	chck enchant weapon/armor to keep alignment restrictions
*****/

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
	    	fail += 3 * (hit_bonus * hit_bonus);
 	    }
	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
	    	fail += 3 * (dam_bonus * dam_bonus);
	    }
	    else fail += 10;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    fail += 3 * (hit_bonus * hit_bonus);
	}
	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    fail += 3 * (dam_bonus * dam_bonus);
	}
	else fail += 10;
    }
    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_EVIL))
	fail -= 5;
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail += 25;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail += 5;
    if (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	fail += 40;
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
	fail -= 30;
    // if (IS_WEAPON_STAT(obj,WEAPON_STUN))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_QUICK))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_SMART))
	// fail += 50;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	fail -= 10;

    fail = URANGE(5,fail,95);

    /* the moment of truth */

// yuki's flag counter so there are no longer 4-5 flag weapons in the game

    // if ( weapon_flag_counter( obj ) > 2 )
    // {
        // if ( weapon_flag_counter( obj ) > 3 )
	    // fail = 110;
	// else
	    // fail = 95;
    // }

    result = number_percent();
    if (result < (fail / 4))  /* item destroyed */
    {
	act("$p explodes in a shocking ball of electricity!",ch,obj,NULL,TO_ALL);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	     vch_next	= vch->next;
	     if ( vch->in_room == NULL )
	         continue;

             if ( IS_PET( vch ) && vch->master != NULL && vch->master == ch )
                continue;

	     if ( ch->in_room == vch->in_room )
	     {
	         dam = vch->hit / 2;
                 damage( ch, vch, dam, sn, DAM_LIGHTNING,TRUE );
		 if ( ch->fighting != NULL )
	             stop_fighting( ch, TRUE );
	     }

	     else if ( vch->in_room->area == ch->in_room->area )
	         SEND( "You hear an explosion in the distance.\n\r", vch );
        }
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows slightly, then dulls and fades.",ch,obj,NULL,TO_CHAR);
	act("$p lets out a slight crackle of electricity...",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    //if ( paf != affect_find(obj->affected,gsn_vampiric_blade))
	        affect_remove_obj(obj,paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL)
	&&   IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) )
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        act("$p does not burn with hatred yet.",ch,obj,NULL,TO_CHAR);
        return;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO,obj->level + 1);

    af.where	      = TO_WEAPON;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = WEAPON_LIGHTNING_DAM;
    affect_to_obj(obj,&af);

    act("$p crackles with electricity.",ch,obj,NULL,TO_ALL);
    return;
    
}

SPELL(spell_frost_forge)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf, af; 
    int result, fail;
    int hit_bonus, dam_bonus;
    int dam = 0;

    if ( IS_IMMORTAL(ch) && get_trust(ch) < 49 )
    {
	SEND("If you can't set it, you can't do it.\n\r",ch);
	return;
    }
    if (obj->item_type != ITEM_WEAPON)
    {
	SEND("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	SEND("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if ((IS_WEAPON_STAT(obj,WEAPON_FIRE_DAM)) || attack_table[obj->value[3]].damage == DAM_FIRE )
	{
	    act("$p radiates a dim glow, negating your magic.",ch,obj,NULL,TO_CHAR);
	    return;
	}
    }

    if (IS_WEAPON_STAT(obj,WEAPON_COLD_DAM))
    {
            act("$p already chills with hunger.",ch,obj,NULL,TO_CHAR);
            return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

/******
	chck enchant weapon/armor to keep alignment restrictions
*****/

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
	    	fail += 3 * (hit_bonus * hit_bonus);
 	    }
	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
	    	fail += 3 * (dam_bonus * dam_bonus);
	    }
	    else fail += 10;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    fail += 3 * (hit_bonus * hit_bonus);
	}
	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    fail += 3 * (dam_bonus * dam_bonus);
	}
	else fail += 10;
    }
    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_EVIL))
	fail -= 5;
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail += 25;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail += 5;
    if (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
	fail -= 30;
    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
	fail += 30;
	if (IS_WEAPON_STAT(obj,WEAPON_FLAMING))
	fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_STUN))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_QUICK))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_SMART))
	// fail += 50;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	fail += 100;

    fail = URANGE(5,fail,95);

    /* the moment of truth */

// yuki's flag counter so there are no longer 4-5 flag weapons in the game

    // if ( weapon_flag_counter( obj ) > 2 )
    // {
        // if ( weapon_flag_counter( obj ) > 3 )
	    // fail = 110;
	// else
	    // fail = 95;
    // }

    result = number_percent();
    if (result < (fail / 4))  /* item destroyed */
    {
	act("$p explodes in an freezing ball of ice shards!",ch,obj,NULL,TO_ALL);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	     vch_next	= vch->next;
	     if ( vch->in_room == NULL )
	         continue;

             if ( IS_PET( vch ) && vch->master != NULL && vch->master == ch )
                continue;

	     if ( ch->in_room == vch->in_room )
	     {
	         dam = vch->hit / 2;
                 damage( ch, vch, dam, sn, DAM_COLD,TRUE );
		 if ( ch->fighting != NULL )
	             stop_fighting( ch, TRUE );
	     }

	     else if ( vch->in_room->area == ch->in_room->area )
	         SEND( "You hear an explosion in the distance.\n\r", vch );
        }
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p chills slightly, then returns to normal.",ch,obj,NULL,TO_CHAR);
	act("$p lets out a puff of chill air...",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    //if ( paf != affect_find(obj->affected,gsn_vampiric_blade))
	        affect_remove_obj(obj,paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL)
	&&   IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) )
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        act("$p does not freeze with hunger yet.",ch,obj,NULL,TO_CHAR);
        return;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO,obj->level + 1);

    af.where	      = TO_WEAPON;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = WEAPON_COLD_DAM;
    affect_to_obj(obj,&af);

    af.where	      = TO_OBJECT;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = ITEM_ANTI_GOOD;
    affect_to_obj(obj,&af);

    act("$p freezes with hunger.",ch,obj,NULL,TO_ALL);
    return;
    
}

SPELL(spell_vampiric_forge)
{
	  CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf, af; 
    int result, fail;
    int hit_bonus, dam_bonus;
    int dam = 0;

    if ( IS_IMMORTAL(ch) && get_trust(ch) < 49 )
    {
	SEND("If you can't set it, you can't do it.\n\r",ch);
	return;
    }
    if (obj->item_type != ITEM_WEAPON)
    {
	SEND("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	SEND("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if ( (IS_OBJ_STAT(obj,ITEM_BLESS))||(IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	  || (IS_WEAPON_STAT(obj,WEAPON_LIGHT_DAM)) || attack_table[obj->value[3]].damage == DAM_HOLY ) 
	{
	    act("$p radiates a dim glow, negating your tainted magic.",ch,obj,NULL,TO_CHAR);
	    return;
	}
    }

    if (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
    {
            act("$p already lusts for the blood of others.",ch,obj,NULL,TO_CHAR);
            return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

/******
	chck enchant weapon/armor to keep alignment restrictions
*****/

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
          if ( paf->location == APPLY_HITROLL )
          {
	    	hit_bonus = paf->modifier;
	    	fail += 3 * (hit_bonus * hit_bonus);
 	    }
	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
	    	fail += 3 * (dam_bonus * dam_bonus);
	    }
	    else fail += 10;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    fail += 3 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    fail += 3 * (dam_bonus * dam_bonus);
	}
	    else fail += 10;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;
    if (IS_OBJ_STAT(obj,ITEM_EVIL))
	fail -= 5;
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail += 25;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail += 5;
    
    if (IS_OBJ_STAT(obj,ITEM_EVIL))
 	fail -= 15;
    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING))
	fail += 40;
    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
	fail += 30;
    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
	fail += 30;
    // if (IS_WEAPON_STAT(obj,WEAPON_STUN))
	// fail += 40;
    // if (IS_WEAPON_STAT(obj,WEAPON_QUICK))
	// fail += 60;
    // if (IS_WEAPON_STAT(obj,WEAPON_SMART))
	// fail += 60;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	fail -= 20;

    fail = URANGE(5,fail,95);

// yuki's flag counter so there are no longer 4-5 flag weapons in the game

    // if ( weapon_flag_counter( obj ) > 2 )
    // {
        // if ( weapon_flag_counter( obj ) > 3 )
	    // fail = 110;
	// else
	    // fail = 95;
    // }

    /* the moment of truth */

    result = number_percent();
    if (result < (fail / 4))  /* item destroyed */
    {
	act("$p hisses softly, then shatters into sharp fragments!",ch,obj,NULL,TO_CHAR);
	act("$p lets out a hissing sound... and shatters suddenly!",ch,obj,NULL,TO_ROOM);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	     vch_next	= vch->next;
	     if ( vch->in_room == NULL )
	         continue;

             if ( IS_PET( vch ) && vch->master != NULL && vch->master == ch )
                continue;

	     if ( ch->in_room == vch->in_room )
	     {
	         dam = vch->hit / 2;
                 damage( ch, vch, dam, sn, DAM_PIERCE,TRUE );
		 if ( ch->fighting != NULL )
	             stop_fighting( ch, TRUE );
	     }

	     else if ( vch->in_room->area == ch->in_room->area )
	         SEND( "You hear an explosion in the distance.\n\r", vch );
        }
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p radiates a dim red glow...then fades away.",ch,obj,NULL,TO_CHAR);
	act("$p dimly glows red...then fades away.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	   // if ( paf != affect_find(obj->affected,gsn_vampiric_blade))
	        affect_remove_obj(obj,paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL)
	&&   IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) )
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        act("$p does not yearn for blood yet.",ch,obj,NULL,TO_CHAR);
        return;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO,obj->level + 1);

    af.where	      = TO_WEAPON;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = WEAPON_VAMPIRIC;
    affect_to_obj(obj,&af);

    af.where	      = TO_OBJECT;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL;
    affect_to_obj(obj,&af);

    act("$p lusts for innocent blood.",ch,obj,NULL,TO_ALL);
    return;
}

// SPELL(spell_vampiric_touch)
// {
    // CHAR_DATA *victim = (CHAR_DATA *) vo;
    // int dam;

    // dam = dice(level, 6);
    // if ( saves_spell(ch, sn, isObj, level, victim,DAM_NEGATIVE) )
    // {
         // ch->hit += dam / 10;
	 // dam /= 2;
    // }
    // else
       // ch->hit += dam / 7;
    // damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, isObj );
    // return;
// }

SPELL(spell_flaming_forge)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf, af; 
    int result, fail;
    int hit_bonus, dam_bonus;
    int dam = 0;

    if ( IS_IMMORTAL(ch) && get_trust(ch) < 49 )
    {
	SEND("If you can't set it, you can't do it.\n\r",ch);
	return;
    }
    if (obj->item_type != ITEM_WEAPON)
    {
	SEND("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	SEND("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if ( (IS_OBJ_STAT(obj,ITEM_BLESS))||(IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	  || (IS_WEAPON_STAT(obj,WEAPON_LIGHT_DAM)) || attack_table[obj->value[3]].damage == DAM_HOLY )
	{
	    act("$p radiates a dim glow, negating your tainted magic.",ch,obj,NULL,TO_CHAR);
	    return;
	}
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FIRE_DAM))
    {
            act("$p already flames with burning hatred.",ch,obj,NULL,TO_CHAR);
            return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

/******
	chck enchant weapon/armor to keep alignment restrictions
*****/

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
	    	fail += 3 * (hit_bonus * hit_bonus);
 	    }
	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
	    	fail += 3 * (dam_bonus * dam_bonus);
	    }
	    else fail += 10;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    fail += 3 * (hit_bonus * hit_bonus);
	}
	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    fail += 3 * (dam_bonus * dam_bonus);
	}
	else fail += 10;
    }
    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_EVIL))
	fail -= 5;
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail += 25;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail += 5;
    if (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
	fail += 50;
    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
	fail += 20;
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
	fail += 30;
	if (IS_WEAPON_STAT(obj,WEAPON_FLAMING))
	fail -= 30;
    // if (IS_WEAPON_STAT(obj,WEAPON_STUN))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_QUICK))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_SMART))
	// fail += 50;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	fail += 100;

    fail = URANGE(5,fail,95);

    /* the moment of truth */

// yuki's flag counter so there are no longer 4-5 flag weapons in the game

    // if ( weapon_flag_counter( obj ) > 2 )
    // {
        // if ( weapon_flag_counter( obj ) > 3 )
	    // fail = 110;
	// else
	    // fail = 95;
    // }

    result = number_percent();
    if (result < (fail / 4))  /* item destroyed */
    {
	act("$p explodes in a fiery ball of flame!",ch,obj,NULL,TO_ALL);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	     vch_next	= vch->next;
	     if ( vch->in_room == NULL )
	         continue;

             if ( IS_PET( vch ) && vch->master != NULL && vch->master == ch )
                continue;

	     if ( ch->in_room == vch->in_room )
	     {
	         dam = vch->hit / 2;
                 damage( ch, vch, dam, sn, DAM_FIRE,TRUE );
		 if ( ch->fighting != NULL )
	             stop_fighting( ch, TRUE );
	     }

	     else if ( vch->in_room->area == ch->in_room->area )
	         SEND( "You hear an explosion in the distance.\n\r", vch );
        }
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p warms slightly, then turns chilling cold.",ch,obj,NULL,TO_CHAR);
	act("$p lets out a wisp of smoke...",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    //if ( paf != affect_find(obj->affected,gsn_vampiric_blade))
	        affect_remove_obj(obj,paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL)
	&&   IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) )
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        act("$p does not burn with hatred yet.",ch,obj,NULL,TO_CHAR);
        return;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO,obj->level + 1);

    af.where	      = TO_WEAPON;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = WEAPON_FIRE_DAM;
    affect_to_obj(obj,&af);

    af.where	      = TO_OBJECT;
    af.type	      = sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = ITEM_ANTI_GOOD;
    affect_to_obj(obj,&af);

    act("$p burns with hate.",ch,obj,NULL,TO_ALL);
    return;
    
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell (int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim,
                     OBJ_DATA * obj)
{
    void *vo;
    int target = TARGET_NONE;

    if (sn <= 0)
        return;

    if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
        bug ("Obj_cast_spell: bad sn %d.", sn);
        return;
    }

    switch (skill_table[sn].target)
    {
        default:
            bug ("Obj_cast_spell: bad target for sn %d.", sn);
            return;

        case TAR_IGNORE:
            vo = NULL;
            break;

        case TAR_CHAR_OFFENSIVE:
            if (victim == NULL)
                victim = ch->fighting;
            if (victim == NULL)
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }
            if (is_safe (ch, victim) && ch != victim)
            {
                SEND ("Something isn't right...\r\n", ch);
                return;
            }
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_DEFENSIVE:
        case TAR_CHAR_SELF:
            if (victim == NULL)
                victim = ch;
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_OBJ_INV:
            if (obj == NULL)
            {
                SEND ("You can't do that.\r\n", ch);
                return;
            }
            vo = (void *) obj;
            target = TARGET_OBJ;
            break;

        case TAR_OBJ_CHAR_OFF:
            if (victim == NULL && obj == NULL)
            {
                if (ch->fighting != NULL)
                    victim = ch->fighting;
                else
                {
                    SEND ("You can't do that.\r\n", ch);
                    return;
                }
            }

            if (victim != NULL)
            {
                if (is_safe_spell (ch, victim, FALSE) && ch != victim)
                {
                    SEND ("Somehting isn't right...\r\n", ch);
                    return;
                }

                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else
            {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            break;


        case TAR_OBJ_CHAR_DEF:
            if (victim == NULL && obj == NULL)
            {
                vo = (void *) ch;
                target = TARGET_CHAR;
            }
            else if (victim != NULL)
            {
                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else
            {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }

            break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);



    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
         || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
             && target == TARGET_CHAR)) && victim != ch
        && victim->master != ch)
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        for (vch = ch->in_room->people; vch; vch = vch_next)
        {
            vch_next = vch->next_in_room;
            if (victim == vch && victim->fighting == NULL)
            {
                check_killer (victim, ch);
                multi_hit (victim, ch, TYPE_UNDEFINED);
                break;
            }
        }
    }

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice (level, 12);
    if (saves_spell (level, victim, DAM_ACID))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_ACID, TRUE);
    return;
}



void spell_armor (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already armored.\r\n", ch);
        else
            act ("$N is already armored.", ch, NULL, victim, TO_CHAR);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = 24;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = -20;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel someone protecting you.\r\n", victim);
    if (ch != victim)
        act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
    return;
}



void spell_bless (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT (obj, ITEM_BLESS))
        {
            act ("$p is already blessed.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT (obj, ITEM_EVIL))
        {
            AFFECT_DATA *paf;

            paf = affect_find (obj->affected, gsn_curse);
            if (!saves_dispel
                (level, paf != NULL ? paf->level : obj->level, 0))
            {
                if (paf != NULL)
                    affect_remove_obj (obj, paf);
                act ("$p glows a pale blue.", ch, obj, NULL, TO_ALL);
                REMOVE_BIT (obj->extra_flags, ITEM_EVIL);
                return;
            }
            else
            {
                act ("The evil of $p is too powerful for you to overcome.",
                     ch, obj, NULL, TO_CHAR);
                return;
            }
        }

        af.where = TO_OBJECT;
        af.type = sn;
        af.level = level;
        af.duration = 6 + level;
		if (is_affected (ch, gsn_extension))
			af.duration += number_range((af.duration / 5), (af.duration * 2/5));
        af.location = APPLY_SAVES;
        af.modifier = -1;
        af.bitvector = ITEM_BLESS;
        affect_to_obj (obj, &af);

        act ("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
		if (obj->item_type == ITEM_DRINK_CON && obj->value[2] == LIQ_WATER)
			obj->value[2] = LIQ_HOLY_WATER;
        if (obj->wear_loc != WEAR_NONE)
            ch->saving_throw -= 1;
        return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if (victim->position == POS_FIGHTING || is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already blessed.\r\n", ch);
        else
            act ("$N already has divine favor.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 6 + level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.location = APPLY_HITROLL;
    af.modifier = level > 8 ? level / 8 : 1;
    af.bitvector = 0;
    affect_to_char (victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = level > 8 ? level / 8 : 1;
    affect_to_char (victim, &af);
    SEND ("You feel righteous.\r\n", victim);
    if (ch != victim)
        act ("You grant $N the favor of your god.", ch, NULL, victim,
             TO_CHAR);
    return;
}



void spell_blindness (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_BLIND)
        || saves_spell (level, victim, DAM_OTHER))
        return;


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 1 + level;
    af.bitvector = AFF_BLIND;
    affect_to_char (victim, &af);
    SEND ("You are blinded!\r\n", victim);
    act ("$n appears to be blinded.", victim, NULL, NULL, TO_ROOM);
    return;
}



void spell_burning_hands (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 4);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}



void spell_call_lightning (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
	char buf[MAX_STRING_LENGTH];

    if (!IS_OUTSIDE (ch))
    {
        SEND ("You must be out of doors.\r\n", ch);
        return;
    }

    if (weather_info.sky < SKY_RAINING)
    {
        SEND ("You need bad weather.\r\n", ch);
        return;
    }
	
	if (!PRECIPITATING(ch->in_room->area))
	{
        SEND ("You need bad weather.\r\n", ch);
        return;
    }	

    dam = dice (level / 2, 8);

    sprintf (buf, "%s's lightning strikes your foes!\r\n", god_table[ch->god].name);
	SEND(buf,ch);
    act ("$n calls their god's lightning to strike $s foes!",
         ch, NULL, NULL, TO_ROOM);

    for (vch = char_list; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room)
        {
            if (vch != ch && (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch)))
                damage (ch, vch, saves_spell (level, vch, DAM_LIGHTNING)
                        ? dam / 2 : dam, sn, DAM_LIGHTNING, TRUE);
            continue;
        }

        if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE (vch)
            && IS_AWAKE (vch))
            SEND ("Lightning flashes in the sky.\r\n", vch);
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->position == POS_FIGHTING)
        {
            count++;
            if (IS_NPC (vch))
                mlevel += vch->level;
            else
                mlevel += vch->level / 2;
            high_level = UMAX (high_level, vch->level);
        }
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL (ch))        /* always works */
        mlevel = 0;

    if (number_range (0, chance) >= mlevel)
    {                            /* hard to stop large fights */
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (IS_NPC (vch) && (IS_SET (vch->imm_flags, IMM_MAGIC) ||
                                 IS_SET (vch->act, ACT_UNDEAD)))
                return;

            if (IS_AFFECTED (vch, AFF_CALM) || IS_AFFECTED (vch, AFF_BERSERK)
                || is_affected (vch, skill_lookup ("frenzy")))
                return;

            SEND ("A wave of calm passes over you.\r\n", vch);

            if (vch->fighting || vch->position == POS_FIGHTING)
                stop_fighting (vch, FALSE);


            af.where = TO_AFFECTS;
            af.type = sn;
            af.level = level;
            af.duration = level / 4;
            af.location = APPLY_HITROLL;
            if (!IS_NPC (vch))
                af.modifier = -5;
            else
                af.modifier = -2;
            af.bitvector = AFF_CALM;
            affect_to_char (vch, &af);

            af.location = APPLY_DAMROLL;
            affect_to_char (vch, &af);
        }
    }
}

void spell_cancellation (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    level += 2;

	if (!IS_NPC(ch) && IS_IMMORTAL(ch))
		level = ch->level + 5;
	if (!IS_NPC(ch) && !str_cmp(ch->name, "upro"))
		level = 100;
	
    if ((!IS_NPC (ch) && IS_NPC (victim) &&
         !(IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)) ||
        (IS_NPC (ch) && !IS_NPC (victim)))
    {
        SEND ("You failed, try dispel magic.\r\n", ch);
        return;
    }

    /* unlike dispel magic, the victim gets NO save */

    /* begin running through the spells */

    if (check_dispel (level, victim, skill_lookup ("armor")))
        found = TRUE;
	
	if (check_dispel (level, victim, skill_lookup ("barkskin")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("bless")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("blindness")))
    {
        found = TRUE;
        act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("calm")))
    {
        found = TRUE;
        act ("$n no longer looks so peaceful...", victim, NULL, NULL,
             TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("change sex")))
    {
        found = TRUE;
        act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("charm person")))
    {
        found = TRUE;
        act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("chill touch")))
    {
        found = TRUE;
        act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("curse")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect evil")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect good")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect hidden")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect invis")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect magic")))
        found = TRUE;
	
	if (check_dispel (level, victim, skill_lookup ("detect traps")))
        found = TRUE;
	
	if (check_dispel (level, victim, skill_lookup ("detect plants")))
        found = TRUE;

	
	
    if (check_dispel (level, victim, skill_lookup ("faerie fire")))
    {
        act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("fly")))
    {
        act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("feather fall")))
    {
        act ("$n seems more in tune with gravity!", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
    if (check_dispel (level, victim, skill_lookup ("frenzy")))
    {
        act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("giant strength")))
    {
        act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("haste")))
    {
        act ("$n is no longer moving so quickly.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("infravision")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("invis")))
    {
        act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("mass invis")))
    {
        act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("pass door")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("protection evil")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("protection good")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("sanctuary")))
    {
        act ("The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("shield")))
    {
        act ("The shield protecting $n vanishes.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("sleep")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("slow")))
    {
        act ("$n is no longer moving so slowly.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("stone skin")))
    {
        act ("$n's skin regains its normal texture.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
        act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("feeblemind")))
    {
        act ("$n looks less dumbfounded.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("silence")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("detect undead")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("displacement")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("detect scrying")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("confusion")))
    {
        act ("$n looks less confused.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("enlarge person")))
    {
        act ("$n returns to their normal size.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("reduce person")))
    {
        act ("$n returns to their normal size.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("web")))
    {
        act ("$n is no longer covered in magical webbing.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("paralyzation")))
    {
        act ("$n regains control of their muscles.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("audible glamor")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("extension")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("nightmare")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("retribution")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("blink")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("mana shield")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("displacement")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("wrath of the ancients")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("motivation")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("vocal shield")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("cacophonic shield")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("aid")))
    {     
        found = TRUE;
    }
	
    if (found)
        SEND ("Ok.\r\n", ch);
    else
        SEND ("Spell failed.\r\n", ch);
}

void spell_cause_light (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    damage (ch, (CHAR_DATA *) vo, dice (1, 8) + level / 3, sn, DAM_HARM,
            TRUE);
    return;
}



void spell_cause_critical (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    damage (ch, (CHAR_DATA *) vo, dice (3, 8) + level - 6, sn, DAM_HARM,
            TRUE);
    return;
}



void spell_cause_serious (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    damage (ch, (CHAR_DATA *) vo, dice (2, 8) + level / 2, sn, DAM_HARM,
            TRUE);
    return;
}

void spell_chain_lightning (int sn, int level, CHAR_DATA * ch, void *vo,
                            int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    int dam;

    /* first strike */

    act ("A lightning bolt leaps from $n's hand and arcs to $N.",
         ch, NULL, victim, TO_ROOM);
    act ("A lightning bolt leaps from your hand and arcs to $N.",
         ch, NULL, victim, TO_CHAR);
    act ("A lightning bolt leaps from $n's hand and hits you!",
         ch, NULL, victim, TO_VICT);

    dam = dice (level, 6);
    if (saves_spell (level, victim, DAM_LIGHTNING))
        dam /= 3;
    damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    last_vict = victim;
    level -= 4;                    /* decrement damage */

    /* new targets */
    while (level > 0)
    {
        found = FALSE;
        for (tmp_vict = ch->in_room->people;
             tmp_vict != NULL; tmp_vict = next_vict)
        {
            next_vict = tmp_vict->next_in_room;
            if (!is_safe_spell (ch, tmp_vict, TRUE) && tmp_vict != last_vict)
            {
                found = TRUE;
                last_vict = tmp_vict;
                act ("The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM);
                act ("The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR);
                dam = dice (level, 6);
                if (saves_spell (level, tmp_vict, DAM_LIGHTNING))
                    dam /= 3;
                damage (ch, tmp_vict, dam, sn, DAM_LIGHTNING, TRUE);
                level -= 4;        /* decrement damage */
            }
        }                        /* end target searching loop */

        if (!found)
        {                        /* no target found, hit the caster */
            if (ch == NULL)
                return;

            if (last_vict == ch)
            {                    /* no double hits */
                act ("The bolt seems to have fizzled out.", ch, NULL, NULL,
                     TO_ROOM);
                act ("The bolt grounds out through your body.", ch, NULL,
                     NULL, TO_CHAR);
                return;
            }

            last_vict = ch;
            act ("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM);
            SEND ("You are struck by your own lightning!\r\n", ch);
            dam = dice (level, 6);
            if (saves_spell (level, ch, DAM_LIGHTNING))
                dam /= 3;
            damage (ch, ch, dam, sn, DAM_LIGHTNING, TRUE);
            level -= 4;            /* decrement damage */
            if (ch == NULL)
                return;
        }
        /* now go back and find more targets */
    }
}


void spell_change_sex (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You've already been changed.\r\n", ch);
        else
            act ("$N has already had $s(?) sex changed.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }
    if (saves_spell (level, victim, DAM_OTHER))
        return;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_SEX;
    do
    {
        af.modifier = number_range (0, 2) - victim->sex;
    }
    while (af.modifier == 0);
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel different.\r\n", victim);
    act ("$n doesn't look like $mself anymore...", victim, NULL, NULL,
         TO_ROOM);
    return;
}



void spell_charm_person (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe (ch, victim))
        return;

    if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	if (victim == ch)
    {
        SEND ("You like yourself even better!\r\n", ch);
        return;
    }
	
    if (IS_AFFECTED (victim, AFF_CHARM)
        || IS_AFFECTED (ch, AFF_CHARM)
        || level < victim->level || IS_SET (victim->imm_flags, IMM_CHARM)
        || saves_spell (level, victim, DAM_CHARM))
        return;

	if ( victim->position == POS_SLEEPING && !IS_NPC(victim)) 
	{
		SEND( "You can not get your victim's attention.\r\n", ch );
		SEND( "Your slumbers are briefly troubled.\r\n", victim );
		return;
	}

    if (IS_SET (victim->in_room->room_flags, ROOM_LAW))
    {
        SEND
            ("The mayor does not allow charming in the city limits.\r\n", ch);
        return;
    }

	if (IS_NPC(victim) && IS_SET (victim->act2, ACT2_BOSS))
	{
		SEND ("Not happening.\r\n",ch);
		return;
	}
	
    if (victim->master)
        stop_follower (victim);
    add_follower (victim, ch);
    victim->leader = ch;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy (level / 4);
	if (is_affected (ch, gsn_extension))
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char (victim, &af);
    act ("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
    if (ch != victim)
        act ("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
	ch->charms++;
    return;
}



void spell_chill_touch (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{

	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam = 0;

	if (ch->grank == G_BACK)
	{
		SEND ("You can't do that while in the back ranks.\r\n",ch);
		return;
	}
	
	dam = dice (level, 4);
    if (!saves_spell (level, victim, DAM_COLD))
    {
        act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
        af.where = TO_AFFECTS;
        af.type = sn;
        af.level = level;
        af.duration = 6;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = 0;
        affect_join (victim, &af);
    }
    else
    {
        dam /= 2;
    }

    damage (ch, victim, dam, sn, DAM_COLD, TRUE);
    return;
}



void spell_colour_spray (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 7);
    if (saves_spell (level, victim, DAM_LIGHT))
        dam /= 2;
    else
        spell_blindness (skill_lookup ("blindness"),
                         level / 2, ch, (void *) victim, TARGET_CHAR);

    damage (ch, victim, dam, sn, DAM_LIGHT, TRUE);
    return;
}



void spell_continual_light (int sn, int level, CHAR_DATA * ch, void *vo,
                            int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')
    {                            /* do a glow on some object */
        light = get_obj_carry (ch, target_name, ch);

        if (light == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
		
		if (light->item_type == ITEM_LIGHT)
		{
			if (light->value[2] < 0)
			{
				SEND("That's light burns eternally anyhow.\r\n",ch);
				return;
			}
			else
			{
				light->value[2] += number_range(level / 2, level);
				act ("$p's light source flares momentarily.", ch, light, NULL, TO_CHAR);
				return;
			}
		}
		
        if (IS_OBJ_STAT (light, ITEM_GLOW))
        {
            act ("$p is already glowing.", ch, light, NULL, TO_CHAR);
            return;
        }

        SET_BIT (light->extra_flags, ITEM_GLOW);
        act ("$p glows with a white light.", ch, light, NULL, TO_ALL);
        return;
    }

    light = create_object (get_obj_index (OBJ_VNUM_LIGHT_BALL), 0);
    obj_to_room (light, ch->in_room);
    act ("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
    act ("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
    return;
}



void spell_control_weather (int sn, int level, CHAR_DATA * ch, void *vo,
                            int target)
{	
	
    if (!str_cmp (target_name, "better"))
	{
        //weather_info.change += dice (level / 3, 4);
		ch->in_room->area->curr_precip--;
		ch->in_room->area->curr_wind--;
		if (ch->in_room->area->curr_wind < 0)
			ch->in_room->area->curr_wind = 0;
		if (ch->in_room->area->curr_precip < -1)
			ch->in_room->area->curr_precip = -1;
		if (ch->in_room->area->curr_temp > 80)
			ch->in_room->area->curr_temp -= (number_range(2,5));
		if (ch->in_room->area->curr_temp < 60)
			ch->in_room->area->curr_temp += (number_range(2,5));
	}
    else if (!str_cmp (target_name, "worse"))
	{
        //weather_info.change -= dice (level / 3, 4);	
		
		ch->in_room->area->curr_precip++;
		ch->in_room->area->curr_wind++;
		if (ch->in_room->area->curr_wind > 4)
			ch->in_room->area->curr_wind = 4;
		if (ch->in_room->area->curr_precip > 4)
			ch->in_room->area->curr_precip = 4;
		if (ch->in_room->area->curr_temp > 70)
			ch->in_room->area->curr_temp += (number_range(2,5));
		if (ch->in_room->area->curr_temp <= 50)		
			ch->in_room->area->curr_temp -= (number_range(2,5));
	}
    else
        SEND ("Do you want it to get better or worse?\r\n", ch);

    SEND ("The weather bends to your will.\r\n", ch);	
    return;
}



void spell_create_food (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object (get_obj_index (OBJ_VNUM_MUSHROOM), 0);
    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;
    obj_to_room (mushroom, ch->in_room);
    act ("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
    act ("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
    return;
}

void spell_create_rose (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    OBJ_DATA *rose;
    rose = create_object (get_obj_index (OBJ_VNUM_ROSE), 0);
    act ("$n has created a beautiful red rose.", ch, rose, NULL, TO_ROOM);
    SEND ("You create a beautiful red rose.\r\n", ch);
    obj_to_char (rose, ch);
    return;
}

void spell_create_spring (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    OBJ_DATA *spring;

    spring = create_object (get_obj_index (OBJ_VNUM_SPRING), 0);
    spring->timer = level;
    obj_to_room (spring, ch->in_room);
    act ("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
    act ("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
    return;
}



void spell_create_water (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if (obj->item_type != ITEM_DRINK_CON)
    {
        SEND ("It is unable to hold water.\r\n", ch);
        return;
    }

    if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
    {
        SEND ("It contains some other liquid.\r\n", ch);
        return;
    }

    water = UMIN (level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
                  obj->value[0] - obj->value[1]);

    if (water > 0)
    {
        obj->value[2] = LIQ_WATER;
        obj->value[1] += water;
        if (!is_name ("water", obj->name))
        {
            char buf[MAX_STRING_LENGTH];

            sprintf (buf, "%s water", obj->name);
            free_string (obj->name);
            obj->name = str_dup (buf);
        }
        act ("$p is filled.", ch, obj, NULL, TO_CHAR);
    }

    return;
}



void spell_cure_blindness (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected (victim, gsn_blindness))
    {
        if (victim == ch)
            SEND ("You aren't blind.\r\n", ch);
        else
            act ("$N doesn't appear to be blinded.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    if (check_dispel (level, victim, gsn_blindness))
    {
        SEND ("Your vision returns!\r\n", victim);
        act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }
    else
        SEND ("Spell failed.\r\n", ch);
}



void spell_cure_critical (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	char buf[MSL];
    int heal;
	bool crit;

    heal = dice (3, 8) + level - 6;
	
	if (critical_spell(ch))
	{
		heal *= 1.5;
		crit = TRUE;
	}
	
	if (IS_CLERIC(ch))
		heal += wis_app[GET_WIS(ch)].heal_adj;
	
	if (!IS_NPC(victim) && victim->bleeding > 0)
		victim->bleeding--;
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);
    sprintf (buf,"You feel better! {r[{G%d{r]{x\r\n", heal);
	SEND(buf, victim);
    if (ch != victim)
    {
        sprintf (buf, "You heal %s, restoring {G%d{x health.\r\n", victim->name, heal);
		SEND(buf, ch);
	}
    return;
}

/* RT added to cure plague */
void spell_cure_disease (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected (victim, gsn_plague))
    {
        if (victim == ch)
            SEND ("You aren't ill.\r\n", ch);
        else
            act ("$N doesn't appear to be diseased.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    if (check_dispel (level, victim, gsn_plague))
    {
        SEND ("Your sores vanish.\r\n", victim);
        act ("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
             TO_ROOM);
    }
    else
        SEND ("Spell failed.\r\n", ch);
}



void spell_cure_light (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
	char buf[MSL];
	bool crit;
	
    heal = dice (1, 8) + (level / 3);
	
	if (critical_spell(ch))
	{
		heal *= 1.5;
		crit = TRUE;
	}
	
	if (IS_CLERIC(ch))
		heal += wis_app[GET_WIS(ch)].heal_adj;
	
	if (victim->bleeding > 0 && number_percent() > 50)
		victim->bleeding--;
	
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);
    sprintf (buf,"You feel better! {r[{G%d{r]{x\r\n", heal);
	SEND(buf, victim);
	
    if (ch != victim)
	{
        sprintf (buf, "You heal %s, restoring {G%d{x health.\r\n", victim->name, heal);
		SEND(buf, ch);
	}
    return;
}



void spell_cure_poison (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected (victim, gsn_poison))
    {
        if (victim == ch)
            SEND ("You aren't poisoned.\r\n", ch);
        else
            act ("$N doesn't appear to be poisoned.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    if (check_dispel (level, victim, gsn_poison))
    {
        SEND ("A warm feeling runs through your body.\r\n", victim);
        act ("$n looks much better.", victim, NULL, NULL, TO_ROOM);
    }
    else
        SEND ("Spell failed.\r\n", ch);
}

void spell_cure_serious (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
	char buf[MSL];
	bool crit = FALSE;

    heal = dice (2, 8) + level / 2;
	
	if (critical_spell(ch))
	{
		heal *= 1.5;
		crit = TRUE;
	}
	
	if (IS_CLERIC(ch))
		heal += wis_app[GET_WIS(ch)].heal_adj;
	
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
	
	if (victim->bleeding > 0 && number_percent() > 50)
		victim->bleeding--;
		
    update_pos (victim);
    sprintf (buf,"You feel better! {r[{G%d{r]{x\r\n", heal);
	SEND(buf, victim);
    if (ch != victim)
    {
        sprintf (buf, "You heal %s, restoring {G%d{x health.\r\n", victim->name, heal);
		SEND(buf, ch);
	}
    return;
}



void spell_curse (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT (obj, ITEM_EVIL))
        {
            act ("$p is already filled with evil.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT (obj, ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find (obj->affected, skill_lookup ("bless"));
            if (!saves_dispel
                (level, paf != NULL ? paf->level : obj->level, 0))
            {
                if (paf != NULL)
                    affect_remove_obj (obj, paf);
                act ("$p glows with a red aura.", ch, obj, NULL, TO_ALL);
                REMOVE_BIT (obj->extra_flags, ITEM_BLESS);
                return;
            }
            else
            {
                act
                    ("The holy aura of $p is too powerful for you to overcome.",
                     ch, obj, NULL, TO_CHAR);
                return;
            }
        }

        af.where = TO_OBJECT;
        af.type = sn;
        af.level = level;
        af.duration = 2 * level;
		if (is_affected (ch, gsn_extension))
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
        af.location = APPLY_SAVES;
        af.modifier = +1;
        af.bitvector = ITEM_EVIL;
        affect_to_obj (obj, &af);

        act ("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
		if (obj->item_type == ITEM_DRINK_CON && obj->value[2] == LIQ_WATER)
			obj->value[2] = LIQ_UNHOLY_WATER;
        if (obj->wear_loc != WEAR_NONE)
            ch->saving_throw += 1;
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED (victim, AFF_CURSE)
        || saves_spell (level, victim, DAM_NEGATIVE))
        return;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_HITROLL;
    af.modifier = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char (victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = level / 8;
    affect_to_char (victim, &af);

    SEND ("You feel unclean.\r\n", victim);
    if (ch != victim)
        act ("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC (ch) && !IS_EVIL (ch))
    {
        victim = ch;
        SEND ("The demons turn upon you!\r\n", ch);
    }

    ch->alignment = UMAX (-1000, ch->alignment - 50);

    if (victim != ch)
    {
        act ("$n calls forth the demons of Hell upon $N!",
             ch, NULL, victim, TO_ROOM);
        act ("$n has assailed you with the demons of Hell!",
             ch, NULL, victim, TO_VICT);
        SEND ("You conjure forth the demons of hell!\r\n", ch);
    }
    dam = dice (level, 10);
    if (saves_spell (level, victim, DAM_NEGATIVE))
        dam /= 2;
	spell_curse (gsn_curse, 3 * level / 4, ch, (void *) victim, TARGET_CHAR);
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
}

void spell_detect_evil (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_DETECT_EVIL))
    {
        if (victim == ch)
            SEND ("You can already sense evil.\r\n", ch);
        else
            act ("$N can already detect evil.", ch, NULL, victim, TO_CHAR);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char (victim, &af);
    SEND ("Your eyes tingle.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}


void spell_detect_good (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_DETECT_GOOD))
    {
        if (victim == ch)
            SEND ("You can already sense good.\r\n", ch);
        else
            act ("$N can already detect good.", ch, NULL, victim, TO_CHAR);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char (victim, &af);
    SEND ("Your eyes tingle.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}



void spell_detect_hidden (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_DETECT_HIDDEN))
    {
        if (victim == ch)
            SEND ("You are already as alert as you can be. \r\n", ch);
        else
            act ("$N can already sense hidden lifeforms.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char (victim, &af);
    SEND ("Your awareness improves.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}



void spell_detect_invis (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_DETECT_INVIS))
    {
        if (victim == ch)
            SEND ("You can already see invisible.\r\n", ch);
        else
            act ("$N can already see invisible things.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char (victim, &af);
    SEND ("Your eyes tingle.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}



void spell_detect_magic (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_DETECT_MAGIC))
    {
        if (victim == ch)
            SEND ("You can already sense magical auras.\r\n", ch);
        else
            act ("$N can already detect magic.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char (victim, &af);
    SEND ("Your eyes tingle.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}



void spell_detect_poison (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD)
    {
        if (obj->value[3] != 0)
            SEND ("You smell poisonous fumes.\r\n", ch);
        else
            SEND ("It looks delicious.\r\n", ch);
    }
    else
    {
        SEND ("It doesn't look poisoned.\r\n", ch);
    }

    return;
}



void spell_dispel_evil (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC (ch) && IS_EVIL (ch))
        victim = ch;

    if (IS_GOOD (victim))
    {
        act ("$N's god protects them.", ch, NULL, victim, TO_ROOM);
        return;
    }

    if (IS_NEUTRAL (victim))
    {
        act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim->hit > (ch->level * 4))
        dam = dice (level, 4);
    else
        dam = UMAX (victim->hit, dice (level, 4));
    if (saves_spell (level, victim, DAM_HOLY))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_HOLY, TRUE);
    return;
}


void spell_dispel_good (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC (ch) && IS_GOOD (ch))
        victim = ch;

    if (IS_EVIL (victim))
    {
        act ("$N is protected by $S evil.", ch, NULL, victim, TO_ROOM);
        return;
    }

    if (IS_NEUTRAL (victim))
    {
        act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim->hit > (ch->level * 4))
        dam = dice (level, 4);
    else
        dam = UMAX (victim->hit, dice (level, 4));
    if (saves_spell (level, victim, DAM_NEGATIVE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
    return;
}


/* modified for enhanced use */

void spell_dispel_magic (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

	if (!IS_NPC(ch) && IS_IMMORTAL(ch))
		level += 5;
	if (!IS_NPC(ch) && !str_cmp(ch->name, "upro"))
		level = 100;
	
    if (saves_spell (level, victim, DAM_OTHER))
    {
        SEND ("You feel a brief tingling sensation.\r\n", victim);
        SEND ("You failed.\r\n", ch);
        return;
    }

    /* begin running through the spells */

    if (check_dispel (level, victim, skill_lookup ("armor")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("bless")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("blindness")))
    {
        found = TRUE;
        act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("calm")))
    {
        found = TRUE;
        act ("$n no longer looks so peaceful...", victim, NULL, NULL,
             TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("change sex")))
    {
        found = TRUE;
        act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("charm person")))
    {
        found = TRUE;
        act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("chill touch")))
    {
        found = TRUE;
        act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel (level, victim, skill_lookup ("curse")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect evil")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect good")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect hidden")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect invis")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("detect magic")))
        found = TRUE;
	
	if (check_dispel (level, victim, skill_lookup ("detect traps")))
        found = TRUE;
		
	if (check_dispel (level, victim, skill_lookup ("detect plants")))
        found = TRUE;
	
	if (check_dispel (level, victim, skill_lookup ("barkskin")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("faerie fire")))
    {
        act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("fly")))
    {
        act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("frenzy")))
    {
        act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("giant strength")))
    {
        act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("haste")))
    {
        act ("$n is no longer moving so quickly.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("infravision")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("invis")))
    {
        act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("mass invis")))
    {
        act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("pass door")))
        found = TRUE;


    if (check_dispel (level, victim, skill_lookup ("protection evil")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("protection good")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("sanctuary")))
    {
        act ("The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("feeblemind")))
    {
        act ("$n looks less dumbfounded.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("silence")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("detect undead")))
    {        
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("detect scrying")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("confusion")))
    {
        act ("$n looks less confused.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("enlarge person")))
    {
        act ("$n returns to their normal size.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("reduce person")))
    {
        act ("$n returns to their normal size.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("web")))
    {
        act ("$n is no longer covered in magical webbing.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("paralyzation")))
    {
        act ("$n regains control of their muscles.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("audible glamor")))
    {        
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("extension")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("nightmare")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("retribution")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("blink")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("mana shield")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("displacement")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("wrath of the ancients")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("motivation")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("vocal shield")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("cacophonic shield")))
    {     
        found = TRUE;
    }
	
	if (check_dispel (level, victim, skill_lookup ("aid")))
    {     
        found = TRUE;
    }
	
    if (IS_AFFECTED (victim, AFF_SANCTUARY)
        && !saves_dispel (level, victim->level, -1)
        && !is_affected (victim, skill_lookup ("sanctuary")))
    {
        REMOVE_BIT (victim->affected_by, AFF_SANCTUARY);
        act ("The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("shield")))
    {
        act ("The shield protecting $n vanishes.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("sleep")))
        found = TRUE;

    if (check_dispel (level, victim, skill_lookup ("slow")))
    {
        act ("$n is no longer moving so slowly.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

	if (check_dispel (level, victim, skill_lookup ("deafness")))
    {
        act ("$n seems to perk up to the world around them.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("stone skin")))
    {
        act ("$n's skin regains its normal texture.", victim, NULL, NULL,
             TO_ROOM);
        found = TRUE;
    }

    if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
        act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
        found = TRUE;
    }

    if (found)
        SEND ("Ok.\r\n", ch);
    else
        SEND ("Spell failed.\r\n", ch);
    return;
}

void spell_earthquake (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    SEND ("The earth trembles beneath your feet!\r\n", ch);
    act ("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

    for (vch = char_list; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room)
        {
            if (vch != ch && !is_safe_spell (ch, vch, TRUE))
            {
                if (IS_AFFECTED (vch, AFF_FLYING))
                    damage (ch, vch, 0, sn, DAM_BASH, TRUE);
                else
                    damage (ch, vch, level + dice (3, 9), sn, DAM_BASH, TRUE);
            }
            continue;
        }

        if (vch->in_room->area == ch->in_room->area)
            SEND ("The earth trembles and shivers.\r\n", vch);
    }

    return;
}

void spell_enchant_armor (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
        SEND ("That isn't an armor.\r\n", ch);
        return;
    }

    if (obj->wear_loc != -1)
    {
        SEND ("The item must be carried to be enchanted.\r\n", ch);
        return;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;                    /* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location == APPLY_AC)
            {
                ac_bonus = paf->modifier;
                ac_found = TRUE;
                fail += 5 * (ac_bonus * ac_bonus);
            }

            else                /* things get a little harder */
                fail += 20;
        }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        if (paf->location == APPLY_AC)
        {
            ac_bonus = paf->modifier;
            ac_found = TRUE;
            fail += 5 * (ac_bonus * ac_bonus);
        }

        else                    /* things get a little harder */
            fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT (obj, ITEM_BLESS))
        fail -= 15;
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        fail -= 5;

	if (IS_IMMORTAL(ch))
		fail -= 100;
		
    fail = URANGE (5, fail, 85);

    result = number_percent ();

    /* the moment of truth */
    if (result < (fail / 5))
    {                            /* item destroyed */
        act ("$p flares blindingly... and evaporates!", ch, obj, NULL,
             TO_CHAR);
        act ("$p flares blindingly... and evaporates!", ch, obj, NULL,
             TO_ROOM);
        extract_obj (obj);
        return;
    }

    if (result < (fail / 3))
    {                            /* item disenchanted */
        AFFECT_DATA *paf_next;

        act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
        act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
        obj->enchanted = TRUE;

        /* remove all affects */
        for (paf = obj->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next;
            free_affect (paf);
        }
        obj->affected = NULL;

        /* clear all flags */
        obj->extra_flags = 0;
        return;
    }

    if (result <= fail)
    {                            /* failed, no bad result */
        SEND ("Nothing seemed to happen.\r\n", ch);
        return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            af_new = new_affect ();

            af_new->next = obj->affected;
            obj->affected = af_new;

            af_new->where = paf->where;
            af_new->type = UMAX (0, paf->type);
            af_new->level = paf->level;
            af_new->duration = paf->duration;
            af_new->location = paf->location;
            af_new->modifier = paf->modifier;
            af_new->bitvector = paf->bitvector;
        }
    }
	if (is_affected (ch, gsn_extension))
			result -= 5;
	
    if (result <= (90 - level / 5))
    {                            /* success! */
        act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
        act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        added = -1;
    }

    else
    {                            /* exceptional enchant */

        act ("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
        act ("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        SET_BIT (obj->extra_flags, ITEM_GLOW);
        added = -2;
    }

	if (IS_IMMORTAL(ch))
	{                            /* imm enchant */
        act ("$p glows a {Mmagenta{x color!", ch, obj, NULL, TO_CHAR);
        act ("$p glows a {Mmagenta{x color!", ch, obj, NULL, TO_ROOM);
		obj->value[4]++;
    }
    /* now add the enchantments */

    if (obj->level < LEVEL_HERO)
        obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);

    if (ac_found)
    {
        for (paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location == APPLY_AC)
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX (paf->level, level);
            }
        }
    }
    else
    {                            /* add a new affect */

        paf = new_affect ();

        paf->where = TO_OBJECT;
        paf->type = sn;
        paf->level = level;
        paf->duration = -1;
        paf->location = APPLY_AC;
        paf->modifier = added;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
    }

}




void spell_enchant_weapon (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
        SEND ("That isn't a weapon.\r\n", ch);
        return;
    }

    if (obj->wear_loc != -1)
    {
        SEND ("The item must be carried to be enchanted.\r\n", ch);
        return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;                    /* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location == APPLY_HITROLL)
            {
                hit_bonus = paf->modifier;
                hit_found = TRUE;
                fail += 2 * (hit_bonus * hit_bonus);
            }

            else if (paf->location == APPLY_DAMROLL)
            {
                dam_bonus = paf->modifier;
                dam_found = TRUE;
                fail += 2 * (dam_bonus * dam_bonus);
            }

            else                /* things get a little harder */
                fail += 25;
        }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        if (paf->location == APPLY_HITROLL)
        {
            hit_bonus = paf->modifier;
            hit_found = TRUE;
            fail += 2 * (hit_bonus * hit_bonus);
        }

        else if (paf->location == APPLY_DAMROLL)
        {
            dam_bonus = paf->modifier;
            dam_found = TRUE;
            fail += 2 * (dam_bonus * dam_bonus);
        }

        else                    /* things get a little harder */
            fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level / 2;

    if (IS_OBJ_STAT (obj, ITEM_BLESS))
        fail -= 15;
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        fail -= 5;
	if (get_curr_stat(ch, STAT_INT) >= 21)
		fail -= 10;
    
		
	fail = URANGE (5, fail, 95);

    result = number_percent ();

    /* the moment of truth */
    if (result < (fail / 5) && !IS_IMMORTAL(ch))
    {                            /* item destroyed */
        act ("$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR);
        act ("$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM);
        extract_obj (obj);
        return;
    }

    if (result < (fail / 2) && !IS_IMMORTAL(ch))
    {                            /* item disenchanted */
        AFFECT_DATA *paf_next;

        act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
        act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
        obj->enchanted = TRUE;

        /* remove all affects */
        for (paf = obj->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next;
            free_affect (paf);
        }
        obj->affected = NULL;

        /* clear all flags */
        obj->extra_flags = 0;
        return;
    }

    if (result <= fail && !IS_IMMORTAL(ch))
    {                            /* failed, no bad result */
        SEND ("Nothing seemed to happen.\r\n", ch);
        return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            af_new = new_affect ();

            af_new->next = obj->affected;
            obj->affected = af_new;

            af_new->where = paf->where;
            af_new->type = UMAX (0, paf->type);
            af_new->level = paf->level;
            af_new->duration = paf->duration;
            af_new->location = paf->location;
            af_new->modifier = paf->modifier;
            af_new->bitvector = paf->bitvector;
        }
    }
	
	if (is_affected (ch, gsn_extension))
			result -= 5;	
	
    if (result <= (100 - level / 5))
    {                            /* success! */
        act ("$p glows {bblue{x.", ch, obj, NULL, TO_CHAR);
        act ("$p glows {bblue{x.", ch, obj, NULL, TO_ROOM);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        added = 1;
    }

    else
    {                            /* exceptional enchant */

        act ("$p glows a {Bbrillant blue{x!", ch, obj, NULL, TO_CHAR);
        act ("$p glows a {Bbrillant blue{x!", ch, obj, NULL, TO_ROOM);
        SET_BIT (obj->extra_flags, ITEM_MAGIC);
        SET_BIT (obj->extra_flags, ITEM_GLOW);
        added = 2;
    }

    /* now add the enchantments */

    if (obj->level < LEVEL_HERO - 1)
        obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);

    if (dam_found)
    {
        for (paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location == APPLY_DAMROLL)
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX (paf->level, level);
                if (paf->modifier > 4)
                    SET_BIT (obj->extra_flags, ITEM_HUM);
            }
        }
    }
    else
    {                            /* add a new affect */

        paf = new_affect ();

        paf->where = TO_OBJECT;
        paf->type = sn;
        paf->level = level;
        paf->duration = -1;
        paf->location = APPLY_DAMROLL;
        paf->modifier = added;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
    }

    if (hit_found)
    {
        for (paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location == APPLY_HITROLL)
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX (paf->level, level);
                if (paf->modifier > 4)
                    SET_BIT (obj->extra_flags, ITEM_HUM);
            }
        }
    }
    else
    {                            /* add a new affect */

        paf = new_affect ();

        paf->type = sn;
        paf->level = level;
        paf->duration = -1;
        paf->location = APPLY_HITROLL;
        paf->modifier = added;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
        ch->alignment = UMAX (-1000, ch->alignment - 50);

    if (saves_spell (level, victim, DAM_NEGATIVE))
    {
        SEND ("You feel a momentary chill.\r\n", victim);
        return;
    }


    if (victim->level <= 2)
    {
        dam = ch->hit + 1;
    }
    else
    {
        gain_exp (victim, 0 - number_range (level / 2, 3 * level / 2), FALSE);
        victim->mana /= 2;
        victim->move /= 2;
        dam = dice (1, level);
        ch->hit += dam;
    }

    SEND ("You feel your life slipping away!\r\n", victim);
    SEND ("Wow....what a rush!\r\n", ch);
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);

    return;
}



void spell_fireball (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 8);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}


void spell_fireproof (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
    {
        act ("$p is already protected from burning.", ch, obj, NULL, TO_CHAR);
        return;
    }

    af.where = TO_OBJECT;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy (level / 4);
	if (is_affected (ch, gsn_extension))
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = ITEM_BURN_PROOF;

    affect_to_obj (obj, &af);

    act ("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
    act ("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}



void spell_flamestrike (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice (6 + level / 2, 8);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}



void spell_faerie_fire (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
        return;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_AC;
    af.modifier = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char (victim, &af);
    SEND ("You are surrounded by a pink outline.\r\n", victim);
    act ("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
    return;
}



void spell_faerie_fog (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *ich;

    act ("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
    SEND ("You conjure a cloud of purple smoke.\r\n", ch);

    for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room)
    {
        if (ich->invis_level > 0)
            continue;

        if (ich == ch || saves_spell (level, ich, DAM_OTHER))
            continue;

        affect_strip (ich, gsn_invis);
        affect_strip (ich, gsn_mass_invis);
        affect_strip (ich, gsn_sneak);
        REMOVE_BIT (ich->affected_by, AFF_HIDE);
        REMOVE_BIT (ich->affected_by, AFF_INVISIBLE);
        REMOVE_BIT (ich->affected_by, AFF_SNEAK);
        act ("$n is revealed!", ich, NULL, NULL, TO_ROOM);
        SEND ("You are revealed!\r\n", ich);
    }

    return;
}

void spell_floating_disc (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char (ch, WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT (floating, ITEM_NOREMOVE))
    {
        act ("You can't remove $p.", ch, floating, NULL, TO_CHAR);
        return;
    }

    disc = create_object (get_obj_index (OBJ_VNUM_DISC), 0);
    disc->value[0] = ch->level * 10;    /* 10 pounds per level capacity */
    disc->value[3] = ch->level * 5;    /* 5 pounds per level max per item */
    disc->timer = ch->level * 2 - number_range (0, level / 2);
	if (is_affected (ch, gsn_extension))
			disc->timer += number_range((disc->timer * 1/5), (disc->timer * 2/5));


    act ("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
    SEND ("You create a floating disc.\r\n", ch);
    obj_to_char (disc, ch);
    wear_obj (ch, disc, TRUE);
    return;
}


void spell_fly (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_FLYING))
    {
        if (victim == ch)
            SEND ("You are already airborne.\r\n", ch);
        else
            act ("$N doesn't need your help to fly.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level + 3;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char (victim, &af);
    SEND ("Your feet rise off the ground.\r\n", victim);
    act ("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
    return;
}

/* RT clerical berserking spell */

void spell_frenzy (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_BERSERK))
    {
        if (victim == ch)
            SEND ("You are already in a frenzy.\r\n", ch);
        else
            act ("$N is already in a frenzy.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (is_affected (victim, skill_lookup ("calm")))
    {
        if (victim == ch)
            SEND ("Why don't you just relax for a while?\r\n", ch);
        else
            act ("$N doesn't look like $e wants to fight anymore.",
                 ch, NULL, victim, TO_CHAR);
        return;
    }

    if ((IS_GOOD (ch) && !IS_GOOD (victim)) ||
        (IS_NEUTRAL (ch) && !IS_NEUTRAL (victim)) ||
        (IS_EVIL (ch) && !IS_EVIL (victim)))
    {
        act ("Your god doesn't seem to like $N.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 3;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = level / 6;
    af.bitvector = 0;

    af.location = APPLY_HITROLL;
    affect_to_char (victim, &af);

    af.location = APPLY_DAMROLL;
    affect_to_char (victim, &af);

    af.modifier = 10 * (level / 12);
    af.location = APPLY_AC;
    affect_to_char (victim, &af);

    SEND ("You are filled with holy wrath!\r\n", victim);
    act ("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

/* RT ROM-style gate */

void spell_gate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    bool gate_pet;

    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || victim->in_room == NULL
        || !can_see_room (ch, victim->in_room)
        || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
        || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
        || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
		|| !IS_SET (victim->in_room->area->area_flags, AREA_LINKED)
        || victim->level >= level + 3
        || /*(is_clan (victim) &&*/ !is_same_clan (ch, victim)/*)*/
        || (!IS_NPC (victim) && victim->level >= LEVEL_HERO)    /* NOT trust */
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && saves_spell (level, victim, DAM_OTHER)))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }
    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
        gate_pet = TRUE;
    else
        gate_pet = FALSE;

    act ("$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM);
    SEND ("You step through a gate and vanish.\r\n", ch);
    char_from_room (ch);
    char_to_room (ch, victim->in_room);

    act ("$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM);
    do_function (ch, &do_look, "auto");

    if (gate_pet)
    {
        act ("$n steps through a gate and vanishes.", ch->pet, NULL, NULL,
             TO_ROOM);
        SEND ("You step through a gate and vanish.\r\n", ch->pet);
        char_from_room (ch->pet);
        char_to_room (ch->pet, victim->in_room);
        act ("$n has arrived through a gate.", ch->pet, NULL, NULL, TO_ROOM);
        do_function (ch->pet, &do_look, "auto");
    }
}



void spell_giant_strength (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already as strong as you can get!\r\n",
                          ch);
        else
            act ("$N can't get any stronger.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_STR;
    af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
	af.bitvector = 0;
	affect_to_char (victim, &af);
		
	af.location = APPLY_DAMROLL;
	af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("Your muscles surge with heightened power!\r\n", victim);
    act ("$n's muscles surge with heightened power.", victim, NULL, NULL,
         TO_ROOM);
    return;
}


void spell_wrath_ancient (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already empowered by the gods!\r\n",
                          ch);
        else
            act ("$N is already empowered by the gods.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
/*
    af.location = APPLY_STR;
    af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
	af.bitvector = 0;
	affect_to_char (victim, &af);
*/	
	af.location = APPLY_DAMROLL;
	af.modifier = 15;
    af.bitvector = 0;
    affect_to_char (victim, &af);
	af.location = APPLY_SPELL_DAM;
	af.modifier = 15;
	af.bitvector = 0;
	affect_to_char (victim, &af);
    SEND ("You feel the power of the gods surging through your veins!\r\n", victim);
    act ("$n's veins surge with ancient power.", victim, NULL, NULL,
         TO_ROOM);
    return;
}

void spell_harm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX (20, victim->hit - dice (1, 4));
    if (saves_spell (level, victim, DAM_HARM))
        dam = UMIN (50, dam / 2);
    dam = UMIN (100, dam);
    damage (ch, victim, dam, sn, DAM_HARM, TRUE);
    return;
}

/* RT haste spell */

void spell_haste (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_HASTE)
        || IS_SET (victim->off_flags, OFF_FAST))
    {
        if (victim == ch)
            SEND ("You can't move any faster!\r\n", ch);
        else
            act ("$N is already moving as fast as $E can.",
                 ch, NULL, victim, TO_CHAR);
        return;
    }

    if (IS_AFFECTED (victim, AFF_SLOW))
    {
        if (!check_dispel (level, victim, skill_lookup ("slow")))
        {
            if (victim != ch)
                SEND ("Spell failed.\r\n", ch);
            SEND ("You feel momentarily faster.\r\n", victim);
            return;
        }
        act ("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    if (victim == ch)
        af.duration = level / 2;
    else
        af.duration = level / 4;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.location = APPLY_DEX;
    af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char (victim, &af);
    SEND ("You feel yourself moving more quickly.\r\n", victim);
    act ("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}



void spell_heal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	int heal = 0;
	char buf[MSL];
	bool crit = FALSE;
	
	heal = number_range(75,125);
	
	if (IS_CLERIC(ch))
		heal += wis_app[GET_WIS(ch)].heal_adj;
	
	if (critical_spell(ch))
	{
		heal *= 1.5;
		crit = TRUE;
	}
	
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	
	if (IS_UNDEAD(victim))
	{		
		SEND ("Searing pain jolts through your body!!\r\n", victim);		
		damage (ch, victim, heal, sn, DAM_HOLY, TRUE);		
		update_pos (victim);				
	}
	else
	{
		victim->hit += heal;
		if (victim->hit > victim->max_hit)
			victim->hit = victim->max_hit;
		victim->bleeding-= 2;
		if (victim->bleeding < 0)
			victim->bleeding = 0;
		update_pos (victim);
		if (ch != victim)
		{
			sprintf (buf,"A warm feeling fills your body. {r[%s%d{r]{x\r\n", crit ? "{G" : "{g", heal);
			SEND(buf, victim);
		}
		if (ch != victim)
			sprintf (buf, "You heal %s, restoring %s%d{x health.\r\n", victim->name, crit ? "{G" : "{g", heal);
		else
			sprintf (buf, "You heal yourself, restoring %s%d{x health.\r\n", crit ? "{G" : "{g", heal);
		SEND(buf, ch);
		
	}
    return;
}

void spell_heat_metal (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;

    if (!saves_spell (level + 2, victim, DAM_FIRE)
        && !IS_SET (victim->imm_flags, IMM_FIRE))
    {
        for (obj_lose = victim->carrying;
             obj_lose != NULL; obj_lose = obj_next)
        {
            obj_next = obj_lose->next_content;
            if (number_range (1, 2 * level) > obj_lose->level
                && !saves_spell (level, victim, DAM_FIRE)
                && !IS_OBJ_STAT (obj_lose, ITEM_NONMETAL)
                && !IS_OBJ_STAT (obj_lose, ITEM_BURN_PROOF)
				&& IS_METAL(obj_lose->material))
            {
                switch (obj_lose->item_type)
                {
                    case ITEM_ARMOR:
                        if (obj_lose->wear_loc != -1)
                        {        /* remove the item */
                            if (can_drop_obj (victim, obj_lose)
                                && (obj_lose->weight / 10) <
                                number_range (1,
                                              2 * get_curr_stat (victim,
                                                                 STAT_DEX))
                                && remove_obj (victim, obj_lose->wear_loc,
                                               TRUE))
                            {
                                act ("$n yelps and throws $p to the ground!",
                                     victim, obj_lose, NULL, TO_ROOM);
                                act
                                    ("You remove and drop $p before it burns you.",
                                     victim, obj_lose, NULL, TO_CHAR);
                                dam +=
                                    (number_range (1, obj_lose->level) / 3);
                                obj_from_char (obj_lose);
                                obj_to_room (obj_lose, victim->in_room);
                                fail = FALSE;
                            }
                            else
                            {    /* stuck on the body! ouch! */

                                act ("Your skin is seared by $p!",
                                     victim, obj_lose, NULL, TO_CHAR);
                                dam += (number_range (1, obj_lose->level));
                                fail = FALSE;
                            }

                        }
                        else
                        {        /* drop it if we can */

                            if (can_drop_obj (victim, obj_lose))
                            {
                                act ("$n yelps and throws $p to the ground!",
                                     victim, obj_lose, NULL, TO_ROOM);
                                act ("You and drop $p before it burns you.",
                                     victim, obj_lose, NULL, TO_CHAR);
                                dam +=
                                    (number_range (1, obj_lose->level) / 6);
                                obj_from_char (obj_lose);
                                obj_to_room (obj_lose, victim->in_room);
                                fail = FALSE;
                            }
                            else
                            {    /* cannot drop */

                                act ("Your skin is seared by $p!",
                                     victim, obj_lose, NULL, TO_CHAR);
                                dam +=
                                    (number_range (1, obj_lose->level) / 2);
                                fail = FALSE;
                            }
                        }
                        break;
                    case ITEM_WEAPON:
                        if (obj_lose->wear_loc != -1)
                        {        /* try to drop it */
                            if (IS_WEAPON_STAT (obj_lose, WEAPON_FLAMING))
                                continue;

                            if (can_drop_obj (victim, obj_lose)
                                && remove_obj (victim, obj_lose->wear_loc,
                                               TRUE))
                            {
                                act
                                    ("$n is burned by $p, and throws it to the ground.",
                                     victim, obj_lose, NULL, TO_ROOM);
                                SEND
                                    ("You throw your red-hot weapon to the ground!\r\n",
                                     victim);
                                dam += 1;
                                obj_from_char (obj_lose);
                                obj_to_room (obj_lose, victim->in_room);
                                fail = FALSE;
                            }
                            else
                            {    /* YOWCH! */

                                SEND
                                    ("Your weapon sears your flesh!\r\n",
                                     victim);
                                dam += number_range (1, obj_lose->level);
                                fail = FALSE;
                            }
                        }
                        else
                        {        /* drop it if we can */

                            if (can_drop_obj (victim, obj_lose))
                            {
                                act
                                    ("$n throws a burning hot $p to the ground!",
                                     victim, obj_lose, NULL, TO_ROOM);
                                act ("You and drop $p before it burns you.",
                                     victim, obj_lose, NULL, TO_CHAR);
                                dam +=
                                    (number_range (1, obj_lose->level) / 6);
                                obj_from_char (obj_lose);
                                obj_to_room (obj_lose, victim->in_room);
                                fail = FALSE;
                            }
                            else
                            {    /* cannot drop */

                                act ("Your skin is seared by $p!",
                                     victim, obj_lose, NULL, TO_CHAR);
                                dam +=
                                    (number_range (1, obj_lose->level) / 2);
                                fail = FALSE;
                            }
                        }
                        break;
                }
            }
        }
    }
    if (fail)
    {
        SEND ("Your spell had no effect.\r\n", ch);
        SEND ("You feel momentarily warmer.\r\n", victim);
    }
    else
    {                            /* damage! */

        if (saves_spell (level, victim, DAM_FIRE))
            dam = 2 * dam / 3;
        damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
    }
}

/* RT really nasty high-level attack spell */
void spell_holy_word (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;

    bless_num = skill_lookup ("bless");
    curse_num = skill_lookup ("curse");
    frenzy_num = skill_lookup ("frenzy");

    act ("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
    SEND ("You utter a word of divine power.\r\n", ch);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if ((IS_GOOD (ch) && IS_GOOD (vch)) ||
            (IS_EVIL (ch) && IS_EVIL (vch)) ||
            (IS_NEUTRAL (ch) && IS_NEUTRAL (vch)))
        {
            SEND ("You feel full more powerful.\r\n", vch);
            spell_frenzy (frenzy_num, level, ch, (void *) vch, TARGET_CHAR);
            spell_bless (bless_num, level, ch, (void *) vch, TARGET_CHAR);
        }

        else if ((IS_GOOD (ch) && IS_EVIL (vch)) ||
                 (IS_EVIL (ch) && IS_GOOD (vch)))
        {
            if (!is_safe_spell (ch, vch, TRUE))
            {
                spell_curse (curse_num, level, ch, (void *) vch, TARGET_CHAR);
                SEND ("You are struck down!\r\n", vch);
                dam = dice (level, 6);
                damage (ch, vch, dam, sn, DAM_ENERGY, TRUE);
            }
        }

        else if (IS_NEUTRAL (ch) && (vch != ch))
        {
            if (!is_safe_spell (ch, vch, TRUE))
            {
                spell_curse (curse_num, level / 2, ch, (void *) vch,
                             TARGET_CHAR);
                SEND ("You are struck down!\r\n", vch);
                dam = dice (level, 4);
                damage (ch, vch, dam, sn, DAM_ENERGY, TRUE);
            }
        }
    }

    SEND ("You feel drained.\r\n", ch);
    ch->move = 0;
    ch->hit /= 2;
}

void spell_identify (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

	SEND("{r----------------------------------------------------{x\r\n",ch);
    sprintf (buf, "{r|{x Object  %40s {r|{x\r\n", obj->name);
	SEND(buf, ch);
	sprintf (buf, "{r|{x Type              %30s {r|{x\r\n", item_name(obj->item_type));
	SEND(buf, ch);
	SEND("{r----------------------------------------------------{x\r\n",ch);
	
	sprintf (buf, "{r|{x Level  %4d  {r|{x Value     %16d {Dsilver{x {r|{x\r\n", obj->level, obj->cost);
	SEND(buf, ch);
	
	sprintf (buf, "{r|{x Weight %4d  {r|{x Material %24s {r|{x\r\n", obj->weight, obj->material); 
	SEND(buf, ch);
	
	sprintf (buf, "{r|{x Slot   %12s                              {r|{x\r\n",flag_string(wear_flags, obj->pIndexData->wear_flags));
	SEND(buf, ch);	
	
	SEND("{r----------------------------------------------------{x\r\n",ch);
	
	sprintf (buf, "{r|{x Flags:                                           {r|{x\r\n{r|{x %48s {r|{x\r\n{r|{x %48s {r|{x\r\n", extra_bit_name(obj->extra_flags), extra2_bit_name(obj->extra2_flags));
	SEND(buf, ch);
	
	SEND("{r----------------------------------------------------{x\r\n",ch);
	SEND("",ch);
	//sprintf (buf, "{r|{x 
	
			 // is a type of %s.\r\nExtra flags:  %s\r\nExtra2 flags: %s.\r\nWeight is %d, value is %d, level is %d.\r\n",
             // obj->name,
             // item_name (obj->item_type),
             // extra_bit_name (obj->extra_flags),
			 // extra2_bit_name (obj->extra2_flags),
             // obj->weight, obj->cost, obj->level);
    //SEND (buf, ch);

	//sprintf (buf, "This object is made of %s.\r\n",obj->material);
	//SEND (buf, ch);
	
    switch (obj->item_type)
    {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf (buf, "Level %d spells of:", obj->value[0]);
            SEND (buf, ch);

            if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
            {
                SEND (" '", ch);
                SEND (skill_table[obj->value[1]].name, ch);
                SEND ("'", ch);
            }

            if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
            {
                SEND (" '", ch);
                SEND (skill_table[obj->value[2]].name, ch);
                SEND ("'", ch);
            }

            if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
            {
                SEND (" '", ch);
                SEND (skill_table[obj->value[3]].name, ch);
                SEND ("'", ch);
            }

            if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
            {
                SEND (" '", ch);
                SEND (skill_table[obj->value[4]].name, ch);
                SEND ("'", ch);
            }

            SEND (".\r\n", ch);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
		case ITEM_FIGURINE:
            sprintf (buf, "Has %d {r({x%d{r){x charges of level %d",
                     obj->value[2], obj->value[1], obj->value[0]);
            SEND (buf, ch);

            if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
            {
                SEND (" '", ch);
                SEND (skill_table[obj->value[3]].name, ch);
                SEND ("'", ch);
            }

            SEND (".\r\n", ch);
            break;

        case ITEM_DRINK_CON:
            sprintf (buf, "It holds %s-colored %s.\r\n",
                     liq_table[obj->value[2]].liq_color,
                     liq_table[obj->value[2]].liq_name);
            SEND (buf, ch);
            break;

        case ITEM_CONTAINER:
            sprintf (buf, "Capacity: %d#  Maximum weight: %d#  flags: %s\r\n",
                     obj->value[0], obj->value[3],
                     cont_bit_name (obj->value[1]));
            SEND (buf, ch);
            if (obj->value[4] != 100)
            {
                sprintf (buf, "Weight multiplier: %d%%\r\n", obj->value[4]);
                SEND (buf, ch);
            }
            break;
		case ITEM_LIGHT:
			if (obj->value[2] < 0)
			{
				SEND("This light will never extinquish.\r\n",ch);
				break;
			}
			break;
        case ITEM_WEAPON:
            SEND ("Weapon type is ", ch);
            switch (obj->value[0])
            {
                case (WEAPON_EXOTIC):
                    SEND ("exotic.\r\n", ch);
                    break;
                case (WEAPON_SHORT_SWORD):
                    SEND ("short sword.\r\n", ch);
                    break;
				case (WEAPON_LONG_SWORD):
					SEND ("long sword.\r\n",ch);
					break;
				case (WEAPON_BOW):
					SEND ("bow.\r\n",ch);
					break;
				case (WEAPON_CROSSBOW):
					SEND ("crossbow.\r\n",ch);
					break;
                case (WEAPON_DAGGER):
                    SEND ("dagger.\r\n", ch);
                    break;
                case (WEAPON_SPEAR):
                    SEND ("spear.\r\n", ch);
                    break;
				case (WEAPON_STAFF):
                    SEND ("staff.\r\n", ch);
                    break;
                case (WEAPON_MACE):
                    SEND ("mace/club.\r\n", ch);
                    break;
                case (WEAPON_AXE):
                    SEND ("axe.\r\n", ch);
                    break;
                case (WEAPON_FLAIL):
                    SEND ("flail.\r\n", ch);
                    break;
                case (WEAPON_WHIP):
                    SEND ("whip.\r\n", ch);
                    break;
                case (WEAPON_POLEARM):
                    SEND ("polearm.\r\n", ch);
                    break;
                default:
                    SEND ("unknown.\r\n", ch);
                    break;
            }
            if (obj->pIndexData->new_format)
                sprintf (buf, "Damage is %dd%d (average %d).\r\n",
                         obj->value[1], obj->value[2],
                         (1 + obj->value[2]) * obj->value[1] / 2);
            else
                sprintf (buf, "Damage is %d to %d (average %d).\r\n",
                         obj->value[1], obj->value[2],
                         (obj->value[1] + obj->value[2]) / 2);
            SEND (buf, ch);
            if (obj->value[4])
            {                    /* weapon flags */
                sprintf (buf, "Weapons flags: %s\r\n",
                         weapon_bit_name (obj->value[4]));
                SEND (buf, ch);
            }
            break;

        case ITEM_ARMOR:
            sprintf (buf,
                     "Armor Class is %d pierce, %d bash, %d slash, and %d vs. magic.\r\nIt has a bulk of %d\r\n",
                     obj->value[0], obj->value[1], obj->value[2],
                     obj->value[3], obj->value[4]);
            SEND (buf, ch);
            break;
    }

    if (!obj->enchanted)
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            if (paf->location != APPLY_NONE && paf->modifier != 0)
            {
                sprintf (buf, "Affects %s by %d.\r\n",
                         affect_loc_name (paf->location), paf->modifier);
                SEND (buf, ch);
                if (paf->bitvector)
                {
                    switch (paf->where)
                    {
                        case TO_AFFECTS:
                            sprintf (buf, "Adds %s affect.\n",
                                     affect_bit_name (paf->bitvector));
                            break;
                        case TO_OBJECT:
							//Ugly hack.
							if (IS_SET(obj->extra2_flags, ITEM_SHILLELAGH))
								sprintf (buf, "Adds %s object flag.\n",
                                     extra2_bit_name (paf->bitvector));
							else
								sprintf (buf, "Adds %s object flag.\n",
                                     extra_bit_name (paf->bitvector));
                            break;
                        case TO_IMMUNE:
                            sprintf (buf, "Adds immunity to %s.\n",
                                     imm_bit_name (paf->bitvector));
                            break;
                        case TO_RESIST:
                            sprintf (buf, "Adds resistance to %s.\r\n",
                                     imm_bit_name (paf->bitvector));
                            break;
                        case TO_VULN:
                            sprintf (buf, "Adds vulnerability to %s.\r\n",
                                     imm_bit_name (paf->bitvector));
                            break;
                        default:
                            sprintf (buf, "Unknown bit %d: %d\r\n",
                                     paf->where, paf->bitvector);
                            break;
                    }
                    SEND (buf, ch);
                }
            }
        }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        if (paf->location != APPLY_NONE && paf->modifier != 0)
        {
            sprintf (buf, "Affects %s by %d",
                     affect_loc_name (paf->location), paf->modifier);
            SEND (buf, ch);
            if (paf->duration > -1)
                sprintf (buf, ", %d hours.\r\n", paf->duration);
            else
                sprintf (buf, ".\r\n");
            SEND (buf, ch);
            if (paf->bitvector)
            {
                switch (paf->where)
                {
                    case TO_AFFECTS:
                        sprintf (buf, "Adds %s affect.\n",
                                 affect_bit_name (paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf (buf, "Adds %s object flag.\n",
                                 extra_bit_name (paf->bitvector));
                        break;
                    case TO_WEAPON:
                        sprintf (buf, "Adds %s weapon flags.\n",
                                 weapon_bit_name (paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf (buf, "Adds immunity to %s.\n",
                                 imm_bit_name (paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf (buf, "Adds resistance to %s.\r\n",
                                 imm_bit_name (paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf (buf, "Adds vulnerability to %s.\r\n",
                                 imm_bit_name (paf->bitvector));
                        break;
                    default:
                        sprintf (buf, "Unknown bit %d: %d\r\n",
                                 paf->where, paf->bitvector);
                        break;
                }
                SEND (buf, ch);
            }
        }
    }

	if (obj->wetness > 0)
		SEND("This object appears to be wet.\r\n",ch);
	
    return;
}



void spell_infravision (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_INFRARED))
    {
        if (victim == ch)
            SEND ("You can already see in the dark.\r\n", ch);
        else
            act ("$N already has infravision.\r\n", ch, NULL, victim,
                 TO_CHAR);
        return;
    }
    act ("$n's eyes glow red.\r\n", ch, NULL, NULL, TO_ROOM);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char (victim, &af);
    SEND ("Your eyes glow red.\r\n", victim);
    return;
}



void spell_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;

        if (IS_OBJ_STAT (obj, ITEM_INVIS))
        {
            act ("$p is already invisible.", ch, obj, NULL, TO_CHAR);
            return;
        }

        af.where = TO_OBJECT;
        af.type = sn;
        af.level = level;
        af.duration = level + 12;
		if (is_affected (ch, gsn_extension))
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = ITEM_INVIS;
        affect_to_obj (obj, &af);

        act ("$p fades out of sight.", ch, obj, NULL, TO_ALL);
        return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED (victim, AFF_INVISIBLE))
        return;

    act ("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level + 12;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char (victim, &af);
    SEND ("You fade out of existence.\r\n", victim);
    return;
}



void spell_know_alignment (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

    if (ap == ALIGN_LG)
        msg = "$N has a pure and good aura.";
    else if (ap == ALIGN_CG)
        msg = "$N is of excellent moral character, but seems rash.";
    else if (ap == ALIGN_NG)
        msg = "$N is often kind and thoughtful.";
    else if (ap == ALIGN_LN)
        msg = "$N doesn't have a firm moral commitment.";
    else if (ap == ALIGN_CN)
        msg = "$N seems a bit off the handle...";
    else if (ap == ALIGN_TN)
		msg = "$N doesn't care about others at all.";
	else if (ap == ALIGN_LE)
		msg = "$N is a methodical manipulator.";
	else if (ap == ALIGN_NE)
		msg = "$N lies to $S friends.";
	else if (ap == ALIGN_CE)
        msg = "$N is a black-hearted murderer.";
    else
        msg = "$N has a bad alignment, tell an immortal.";

    act (msg, ch, NULL, victim, TO_CHAR);
    return;
}



void spell_lightning_bolt (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 25, 28,
        31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
        44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
        51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
        58, 58, 59, 60, 60, 61, 62, 62, 63, 64
    };
    int dam;

    level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
    level = UMAX (0, level);
    dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
    if (saves_spell (level, victim, DAM_LIGHTNING))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    return;
}



void spell_locate_object (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL (ch) ? 200 : 2 * level;

    buffer = new_buf ();

    for (obj = object_list; obj != NULL; obj = obj->next)
    {
        if (!can_see_obj (ch, obj) || !is_name (target_name, obj->name)
            || IS_OBJ_STAT (obj, ITEM_NOLOCATE || IS_SET(obj->extra2_flags, ITEM_OBSCURE))
            || number_percent () > 2 * level || ch->level < obj->level)
            continue;

        found = TRUE;
        number++;

        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by))
        {
            sprintf (buf, "one is carried by %s\r\n",
                     PERS (in_obj->carried_by, ch));
        }
        else
        {
            if (IS_IMMORTAL (ch) && in_obj->in_room != NULL)
                sprintf (buf, "one is in %s [Room %ld]\r\n",
                         in_obj->in_room->name, in_obj->in_room->vnum);
            else
                sprintf (buf, "one is in %s\r\n",
                         in_obj->in_room == NULL
                         ? "somewhere" : in_obj->in_room->name);
        }

        buf[0] = UPPER (buf[0]);
        add_buf (buffer, buf);

        if (number >= max_found)
            break;
    }

    if (!found)
        SEND ("Nothing like that in heaven or earth.\r\n", ch);
    else
        page_to_char (buf_string (buffer), ch);

    free_buf (buffer);

    return;
}

void spell_magic_missile (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	int loops = 1;
	
	if (!IS_MCLASSED(ch) || ch->ch_class == WIZARD)
	{
		loops = ch->level / 4;
		if (ch->level < 8)
			loops++;
	}	
	if (IS_MCLASSED(ch) && ch->ch_class2 == WIZARD)
	{
		loops = ch->level2 / 4;
		if (ch->level2 < 8)
			loops++;
	}	
	
	if (loops > 5)
		loops = 5;
	
    while (loops > 0)
    {
		dam = dice (level, 3);
		if (saves_spell (level, victim, DAM_ENERGY))
			dam /= 2;
		damage (ch, victim, dam, sn, DAM_ENERGY, TRUE);
		if (victim->hit < 1)
			break;
		loops--;
	}
    return;
}

void spell_mass_healing (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;

    heal_num = skill_lookup ("heal");
    refresh_num = skill_lookup ("refresh");

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if ((IS_NPC (ch) && IS_NPC (gch)) || (!IS_NPC (ch) && !IS_NPC (gch)))
        {
            spell_heal (heal_num, level, ch, (void *) gch, TARGET_CHAR);
			if ( !IS_NPC(gch) && gch->bleeding > 0 )
				gch->bleeding--;
            spell_refresh (refresh_num, level, ch, (void *) gch, TARGET_CHAR);
        }
    }
}


void spell_mass_invis (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (!is_same_group (gch, ch) || IS_AFFECTED (gch, AFF_INVISIBLE))
            continue;
        act ("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
        SEND ("You slowly fade out of existence.\r\n", gch);

        af.where = TO_AFFECTS;
        af.type = sn;
        af.level = level / 2;
        af.duration = 24;
		if (is_affected (ch, gsn_extension))
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_INVISIBLE;
        affect_to_char (gch, &af);
    }
    SEND ("Ok.\r\n", ch);

    return;
}



void spell_null (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    SEND ("That's not a spell!\r\n", ch);
    return;
}



void spell_pass_door (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_PASS_DOOR))
    {
        if (victim == ch)
            SEND ("You are already out of phase.\r\n", ch);
        else
            act ("$N is already shifted out of phase.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy (level / 4);
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char (victim, &af);
    act ("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
    SEND ("You turn translucent.\r\n", victim);
    return;
}

/* RT plague spell, very nasty */

void spell_plague (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell (level, victim, DAM_DISEASE) ||
        (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD)))
    {
        if (ch == victim)
            SEND ("You feel momentarily ill, but it passes.\r\n", ch);
        else
            act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
        return;
    }

	
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level * 3 / 4;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = -5;
    af.bitvector = AFF_PLAGUE;
    affect_join (victim, &af);

    SEND
        ("You scream in agony as plague sores erupt from your skin.\r\n",
         victim);
    act ("$n screams in agony as plague sores erupt from $s skin.", victim,
         NULL, NULL, TO_ROOM);
}

void spell_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;

        if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
        {
            if (IS_OBJ_STAT (obj, ITEM_BLESS)
                || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
            {
                act ("Your spell fails to corrupt $p.", ch, obj, NULL,
                     TO_CHAR);
                return;
            }
            obj->value[3] = 1;
            act ("$p is infused with poisonous vapors.", ch, obj, NULL,
                 TO_ALL);
            return;
        }

        if (obj->item_type == ITEM_WEAPON)
        {
            if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
                || IS_WEAPON_STAT (obj, WEAPON_FROST)
                || IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)                
                || IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
                || IS_OBJ_STAT (obj, ITEM_BLESS)
                || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
            {
                act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
                return;
            }

            if (IS_WEAPON_STAT (obj, WEAPON_POISON))
            {
                act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
                return;
            }

            af.where = TO_WEAPON;
            af.type = sn;
            af.level = level / 2;
            af.duration = level / 6;
			if (is_affected (ch, gsn_extension))
				af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

            af.location = 0;
            af.modifier = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj (obj, &af);

            act ("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
            return;
        }

        act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
        return;
    }

    victim = (CHAR_DATA *) vo;

    if (saves_spell (level, victim, DAM_POISON))
    {
        act ("$n turns slightly green, but it passes.", victim, NULL, NULL,
             TO_ROOM);
        SEND ("You feel momentarily ill, but it passes.\r\n", victim);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_STR;
    af.modifier = level / 6;
    af.bitvector = AFF_POISON;
    affect_join (victim, &af);
    SEND ("You feel very sick.\r\n", victim);
    act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
    return;
}



void spell_protection_evil (int sn, int level, CHAR_DATA * ch, void *vo,
                            int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_PROTECT_EVIL)
        || IS_AFFECTED (victim, AFF_PROTECT_GOOD))
    {
        if (victim == ch)
            SEND ("You are already protected.\r\n", ch);
        else
            act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 24;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_SAVING_SPELL;
    af.modifier = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char (victim, &af);
    SEND ("You feel holy and pure.\r\n", victim);
    if (ch != victim)
        act ("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
    return;
}

void spell_protection_good (int sn, int level, CHAR_DATA * ch, void *vo,
                            int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
        || IS_AFFECTED (victim, AFF_PROTECT_EVIL))
    {
        if (victim == ch)
            SEND ("You are already protected.\r\n", ch);
        else
            act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 24;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_SAVING_SPELL;
    af.modifier = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char (victim, &af);
    SEND ("You feel aligned with darkness.\r\n", victim);
    if (ch != victim)
        act ("$N is protected from good.", ch, NULL, victim, TO_CHAR);
    return;
}


void spell_ray_of_truth (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;

    if (IS_EVIL (ch))
    {
        victim = ch;
        SEND ("The energy explodes inside you!\r\n", ch);
    }

    if (victim != ch)
    {
        act ("$n raises $s hand, and a blinding ray of light shoots forth!",
             ch, NULL, NULL, TO_ROOM);
        SEND
            ("You raise your hand and a blinding ray of light shoots forth!\r\n",
             ch);
    }

    if (IS_GOOD (victim))
    {
        act ("$n seems unharmed by the light.", victim, NULL, victim,
             TO_ROOM);
        SEND ("The light seems powerless to affect you.\r\n", victim);
        return;
    }

    dam = dice (level, 10);
    if (saves_spell (level, victim, DAM_HOLY))
        dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
        align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000;
	if (!saves_spell (level, victim, DAM_LIGHT))
		spell_blindness (gsn_blindness, 3 * level / 4, ch, (void *) victim, TARGET_CHAR);
    damage (ch, victim, dam, sn, DAM_HOLY, TRUE);
    
}


void spell_recharge (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
        SEND ("That item does not carry charges.\r\n", ch);
        return;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
        SEND ("Your skills are not great enough for that.\r\n", ch);
        return;
    }

    if (obj->value[1] == 0)
    {
        SEND ("That item has already been recharged once.\r\n", ch);
        return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3];    /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
        (obj->value[1] - obj->value[2]);

    chance = UMAX (level / 2, chance);

    percent = number_percent ();

    if (percent < chance / 2)
    {
        act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
        act ("$p glows softly.", ch, obj, NULL, TO_ROOM);
        obj->value[2] = UMAX (obj->value[1], obj->value[2]);
        obj->value[1] = 0;
        return;
    }

    else if (percent <= chance)
    {
        int chargeback, chargemax;

        act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
        act ("$p glows softly.", ch, obj, NULL, TO_CHAR);

        chargemax = obj->value[1] - obj->value[2];

        if (chargemax > 0)
            chargeback = UMAX (1, chargemax * percent / 100);
        else
            chargeback = 0;

        obj->value[2] += chargeback;
        obj->value[1] = 0;
        return;
    }

    else if (percent <= UMIN (95, 3 * chance / 2))
    {
        SEND ("Nothing seems to happen.\r\n", ch);
        if (obj->value[1] > 1)
            obj->value[1]--;
        return;
    }

    else
    {                            /* whoops! */

        act ("$p glows brightly, shivering briefly, then explodes!", ch, obj, NULL, TO_CHAR);
        act ("$p glows brightly, shivering briefly, then explodes!", ch, obj, NULL, TO_ROOM);
        extract_obj (obj);
    }
}

void spell_refresh (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN (victim->move + level, victim->max_move);
    if (victim->max_move == victim->move)
        SEND ("You feel fully refreshed!\r\n", victim);
    else
        SEND ("You feel slightly invigorated.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}

void spell_remove_curse (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;

        if (IS_OBJ_STAT (obj, ITEM_NODROP)
            || IS_OBJ_STAT (obj, ITEM_NOREMOVE) ||
			IS_SET(obj->extra2_flags, ITEM_OBSCURE))
        {
            if (!IS_OBJ_STAT (obj, ITEM_NOUNCURSE)
                && !saves_dispel (level + 2, obj->level, 0))
            {
                REMOVE_BIT (obj->extra_flags, ITEM_NODROP);
                REMOVE_BIT (obj->extra_flags, ITEM_NOREMOVE);
				REMOVE_BIT (obj->extra2_flags, ITEM_OBSCURE);
				
                act ("$p briefly glows a {Cpale blue{x.", ch, obj, NULL, TO_ALL);
                return;
            }

            act ("The curse on $p is beyond your power.", ch, obj, NULL,
                 TO_CHAR);
            return;
        }
        act ("There doesn't seem to be a curse on $p.", ch, obj, NULL,
             TO_CHAR);
        return;
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel (level, victim, gsn_curse))
    {
        SEND ("You no longer feel cursed.\r\n", victim);
        act ("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
    }

    for (obj = victim->carrying; (obj != NULL && !found);
         obj = obj->next_content)
    {
        if ((IS_OBJ_STAT (obj, ITEM_NODROP)
             || IS_OBJ_STAT (obj, ITEM_NOREMOVE))
            && !IS_OBJ_STAT (obj, ITEM_NOUNCURSE))
        {                        /* attempt to remove curse */
            if (!saves_dispel (level, obj->level, 0))
            {
                found = TRUE;
                REMOVE_BIT (obj->extra_flags, ITEM_NODROP);
                REMOVE_BIT (obj->extra_flags, ITEM_NOREMOVE);
                act ("Your $p briefly glows {Cpale blue{x.", victim, obj, NULL, TO_CHAR);
                act ("$n's $p briefly glows {Cpale blue{x.", victim, obj, NULL, TO_ROOM);
            }
        }
    }
}

void spell_sanctuary (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_SANCTUARY))
    {
        if (victim == ch)
            SEND ("You are already in sanctuary.\r\n", ch);
        else
            act ("$N is already in sanctuary.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 6;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
	if (af.duration < 1)
		af.duration = 1;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char (victim, &af);
    act ("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
    SEND ("You are surrounded by a white aura.\r\n", victim);
    return;
}



void spell_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already shielded from harm.\r\n", ch);
        else
            act ("$N is already protected by a shield.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 8 + level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    act ("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
    SEND ("You are surrounded by a force shield.\r\n", victim);
    return;
}



void spell_shocking_grasp (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

	if (ch->grank == G_BACK)
	{
		SEND ("You can't do that while in the back ranks.\r\n",ch);
		return;
	}
	
	dam = dice (level / 2, 5);        
    if (saves_spell (level, victim, DAM_LIGHTNING))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    return;
}



void spell_sleep (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, AFF_SLEEP)
        || (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
        || (level + 2) < victim->level
        || saves_spell (level - 4, victim, DAM_CHARM))
		return;

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 4 + level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SLEEP;
    affect_join (victim, &af);

    if (IS_AWAKE (victim))
    {
        SEND ("Your mind falters as sleep overwhelms you.\r\n", victim);
        act ("$n falls into a deep magical slumber.", victim, NULL, NULL, TO_ROOM);
        victim->position = POS_SLEEPING;
    }
    return;
}

void spell_slow (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_SLOW))
    {
        if (victim == ch)
            SEND ("You can't move any slower!\r\n", ch);
        else
            act ("$N can't get any slower than that.",
                 ch, NULL, victim, TO_CHAR);
        return;
    }

    if (saves_spell (level, victim, DAM_OTHER)
        || IS_SET (victim->imm_flags, IMM_MAGIC))
    {
        if (victim != ch)
            SEND ("Nothing seemed to happen.\r\n", ch);
        SEND ("You feel momentarily lethargic.\r\n", victim);
        return;
    }

    if (IS_AFFECTED (victim, AFF_HASTE))
    {
        if (!check_dispel (level, victim, skill_lookup ("haste")))
        {
            if (victim != ch)
                SEND ("Spell failed.\r\n", ch);
            SEND ("You feel momentarily slower.\r\n", victim);
            return;
        }

        act ("$n is moving less quickly.", victim, NULL, NULL, TO_ROOM);
        return;
    }


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_DEX;
    af.modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char (victim, &af);
    SEND ("You feel yourself slowing d o w n...\r\n", victim);
    act ("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
    return;
}




void spell_stone_skin (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (ch, sn))
    {
        if (victim == ch)
            SEND ("Your skin is already as hard as a rock.\r\n", ch);
        else
            act ("$N is already as hard as can be.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_AC;
    af.modifier = -40;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    act ("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
    SEND ("Your skin turns to stone.\r\n", victim);
    return;
}



void spell_summon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;

    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || victim->in_room == NULL
        || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
        || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
        || (IS_NPC (victim) && IS_SET (victim->act, ACT_AGGRESSIVE))
        || victim->level >= level + 3
        || (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL)
        || victim->fighting != NULL
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
        || (!IS_NPC (victim) && IS_SET (victim->act, PLR_NOSUMMON))
        || (IS_NPC (victim) && saves_spell (level, victim, DAM_OTHER)))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    act ("$n disappears suddenly through a glowing portal.", victim, NULL, NULL, TO_ROOM);
    char_from_room (victim);
    char_to_room (victim, ch->in_room);
    act ("$n arrives through a glowing portal.", victim, NULL, NULL, TO_ROOM);
    act ("$n has summoned you!", ch, NULL, victim, TO_VICT);
    do_function (victim, &do_look, "auto");
    return;
}



void spell_teleport (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if (victim->in_room == NULL
        || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
        || (victim != ch && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (!IS_NPC (ch) && victim->fighting != NULL)
        || (victim != ch && (saves_spell (level - 5, victim, DAM_OTHER))))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    pRoomIndex = get_random_room (victim);

    if (victim != ch)
        SEND ("You have been teleported!\r\n", victim);

    act ("$n vanishes!", victim, NULL, NULL, TO_ROOM);
    char_from_room (victim);
    char_to_room (victim, pRoomIndex);
    act ("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
    do_function (victim, &do_look, "auto");
    return;		  
	
    if ( victim->in_room == NULL
    || ( victim != ch && !IS_AFFECTED(ch, AFF_CHARM))
    || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
   // || IS_SET(victim->in_room->room_flags, ROOM_NOGATE)
    || ( victim != ch && ( saves_spell(level, victim, DAM_OTHER))) 
    || ( victim->level <= 5 )
    ||   (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE ))
    || (victim == ch && victim->fighting != NULL)
    || ( IS_AFFECTED( victim, AFF_CURSE) && number_percent() <= 50))
    {
	SEND( "You failed.\n\r", ch );
	return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	SEND("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );

    char_to_room( victim, pRoomIndex );

    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}



void spell_ventriloquate (int sn, int level, CHAR_DATA * ch, void *vo,
                          int target)
{
/*
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];    
	CHAR_DATA *vch = NULL;
	
    target_name = one_argument (target_name, speaker);

	sprintf(buf1, "%s says '%s'.\r\n",  
            IS_NPC(vch) ? vch->short_descr : vch->name, target_name);
    sprintf(buf2, "Someone makes %s say '%s'.\r\n", 
           IS_NPC(vch) ? vch->short_descr : vch->name, target_name);
	    
    buf1[0] = UPPER (buf1[0]);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (!is_exact_name (speaker, vch->name) && IS_AWAKE (vch))
            SEND (saves_spell (level, vch, DAM_OTHER) ? buf2 : buf1,
                          vch);
    }

    return;
*/
}



void spell_weaken (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
	{
		SEND("They appear to be weakened already.\r\n",ch);
        return;
	}
	
	if (saves_spell (level, victim, DAM_OTHER))
	{
		SEND ("You feel your strength momentarily slip away.\r\n", victim);
		act ("$n appears weak momentarily...", victim, NULL, NULL, TO_ROOM);
		return;
	}
	
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_STR;
    af.modifier = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char (victim, &af);
    SEND ("You feel your strength slip away.\r\n", victim);
    act ("$n looks tired and weak.", victim, NULL, NULL, TO_ROOM);
    return;
}



/* RT recall spell is back */

void spell_word_of_recall (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;

    if (IS_NPC (victim))
        return;

    if ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL)
    {
        SEND ("You are completely lost.\r\n", victim);
        return;
    }

    if (IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL) ||
        IS_AFFECTED (victim, AFF_CURSE))
    {
        SEND ("Spell failed.\r\n", victim);
        return;
    }

    if (victim->fighting != NULL)
        stop_fighting (victim, TRUE);

    ch->move /= 2;
    act ("$n disappears.", victim, NULL, NULL, TO_ROOM);
    char_from_room (victim);
    char_to_room (victim, location);
    act ("$n appears in the room.", victim, NULL, NULL, TO_ROOM);
    do_function (victim, &do_look, "auto");
}

/*
 * NPC spells.
 */
void spell_acid_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act ("$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT);
    act ("$n spits a stream of corrosive acid at you.", ch, NULL, victim,
         TO_VICT);
    act ("You spit acid at $N.", ch, NULL, victim, TO_CHAR);

    hpch = UMAX (12, ch->hit);
    hp_dam = number_range (hpch / 11 + 1, hpch / 6);
    dice_dam = dice (level, 16);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    if (saves_spell (level, victim, DAM_ACID))
    {
        acid_effect (victim, level / 2, dam / 4, TARGET_CHAR);
        damage (ch, victim, dam / 2, sn, DAM_ACID, TRUE);
    }
    else
    {
        acid_effect (victim, level, dam, TARGET_CHAR);
        damage (ch, victim, dam, sn, DAM_ACID, TRUE);
    }
}



void spell_fire_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam, hp_dam, dice_dam;
    int hpch;

    act ("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
    act ("$n breathes a cone of hot fire over you!", ch, NULL, victim,
         TO_VICT);
    act ("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX (10, ch->hit);
    hp_dam = number_range (hpch / 9 + 1, hpch / 5);
    dice_dam = dice (level, 20);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    fire_effect (victim->in_room, level, dam / 2, TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_FIRE))
            {
                fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
            }
            else
            {
                fire_effect (vch, level, dam, TARGET_CHAR);
                damage (ch, vch, dam, sn, DAM_FIRE, TRUE);
            }
        }
        else
        {                        /* partial damage */

            if (saves_spell (level - 2, vch, DAM_FIRE))
            {
                fire_effect (vch, level / 4, dam / 8, TARGET_CHAR);
                damage (ch, vch, dam / 4, sn, DAM_FIRE, TRUE);
            }
            else
            {
                fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
            }
        }
    }
}


void spell_energy_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam, hp_dam, dice_dam;
    int hpch;

    act ("$n breathes forth a cone of energy.", ch, NULL, victim, TO_NOTVICT);
    act ("$n breathes a cone of charged energy over you!", ch, NULL, victim,
         TO_VICT);
    act ("You breath forth a cone of energy.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX (10, ch->hit);
    hp_dam = number_range (hpch / 9 + 1, hpch / 5);
    dice_dam = dice (level, 18);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);    

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_ENERGY))
            {             
                damage (ch, vch, dam / 2, sn, DAM_ENERGY, TRUE);
            }
            else
            {                
                damage (ch, vch, dam, sn, DAM_ENERGY, TRUE);
            }
        }
        else
        {                        /* partial damage */

            if (saves_spell (level - 2, vch, DAM_ENERGY))
            {             
                damage (ch, vch, dam / 4, sn, DAM_ENERGY, TRUE);
            }
            else
            {             
                damage (ch, vch, dam / 2, sn, DAM_ENERGY, TRUE);
            }
        }
    }
}

void spell_frost_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam, hp_dam, dice_dam, hpch;

    act ("$n breathes out a freezing cone of frost!", ch, NULL, victim,
         TO_NOTVICT);
    act ("$n breathes a freezing cone of frost over you!", ch, NULL, victim,
         TO_VICT);
    act ("You breath out a cone of frost.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX (12, ch->hit);
    hp_dam = number_range (hpch / 11 + 1, hpch / 6);
    dice_dam = dice (level, 16);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    cold_effect (victim->in_room, level, dam / 2, TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_COLD))
            {
                cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
            }
            else
            {
                cold_effect (vch, level, dam, TARGET_CHAR);
                damage (ch, vch, dam, sn, DAM_COLD, TRUE);
            }
        }
        else
        {
            if (saves_spell (level - 2, vch, DAM_COLD))
            {
                cold_effect (vch, level / 4, dam / 8, TARGET_CHAR);
                damage (ch, vch, dam / 4, sn, DAM_COLD, TRUE);
            }
            else
            {
                cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
            }
        }
    }
}


void spell_gas_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                       int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam, hp_dam, dice_dam, hpch;

    act ("$n breathes out a cloud of poisonous gas!", ch, NULL, NULL,
         TO_ROOM);
    act ("You breath out a cloud of poisonous gas.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX (16, ch->hit);
    hp_dam = number_range (hpch / 15 + 1, 8);
    dice_dam = dice (level, 12);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    poison_effect (ch->in_room, level, dam, TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (ch) && IS_NPC (vch)
                && (ch->fighting == vch || vch->fighting == ch)))
            continue;

        if (saves_spell (level, vch, DAM_POISON))
        {
            poison_effect (vch, level / 2, dam / 4, TARGET_CHAR);
            damage (ch, vch, dam / 2, sn, DAM_POISON, TRUE);
        }
        else
        {
            poison_effect (vch, level, dam, TARGET_CHAR);
            damage (ch, vch, dam, sn, DAM_POISON, TRUE);
        }
    }
}

void spell_lightning_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                             int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act ("$n breathes a bolt of lightning at $N.", ch, NULL, victim,
         TO_NOTVICT);
    act ("$n breathes a bolt of lightning at you!", ch, NULL, victim,
         TO_VICT);
    act ("You breathe a bolt of lightning at $N.", ch, NULL, victim, TO_CHAR);

    hpch = UMAX (10, ch->hit);
    hp_dam = number_range (hpch / 9 + 1, hpch / 5);
    dice_dam = dice (level, 20);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    if (saves_spell (level, victim, DAM_LIGHTNING))
    {
        shock_effect (victim, level / 2, dam / 4, TARGET_CHAR);
        damage (ch, victim, dam / 2, sn, DAM_LIGHTNING, TRUE);
    }
    else
    {
        shock_effect (victim, level, dam, TARGET_CHAR);
        damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose (int sn, int level, CHAR_DATA * ch, void *vo,
                            int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range (25, 100);
    if (saves_spell (level, victim, DAM_PIERCE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_PIERCE, TRUE);
    return;
}

void spell_high_explosive (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range (30, 120);
    if (saves_spell (level, victim, DAM_PIERCE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_PIERCE, TRUE);
    return;
}

////////////////////////////////////////////////////////
//From here down is added on by Upro and co. 2009-2020//
////////////////////////////////////////////////////////

void spell_fear (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int chance = 0;
	
	if (victim == ch)
	{
        SEND("You aren't very scared.\r\n", ch);
		return;
	}
	chance = number_range(1,100);
	chance += (GET_INT(ch) / 4);
	chance -= (get_curr_stat (victim, STAT_INT) / 4);
	if (saves_spell (level, victim, DAM_NEGATIVE))
		chance /= 2;
        
    if (chance > 60)
	{
		do_function (victim, &do_flee, "");
		SEND("You send them fleeing in terror!\r\n",ch);
	}
	else
		SEND("They resist the wave of fear!\r\n",ch);
    return;
}

void spell_confusion (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (victim == ch)
	{
        SEND("You can't seem to confuse yourself.\r\n", ch);
		return;
	}
	if (IS_AFFECTED (victim, sn)
        || saves_spell (level, victim, DAM_MENTAL))
        return;
        
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;	
    af.bitvector = 0;
	af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);	
	af.location = APPLY_INT;
    af.modifier = -4;
    af.duration = 1 + level;
    affect_to_char (victim, &af);
	
	
    SEND ("You suddenly aren't sure of what's going on!\r\n", victim);
    act ("$n appears to be confused!", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_web (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (victim == ch)
	{
        SEND("You probably don't want to do that to yourself.\r\n", ch);
		return;
	}
	if (IS_AFFECTED (victim, sn)
        || saves_spell (level, victim, DAM_OTHER))
        return;
        
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_DEX;
    af.modifier = -4;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
	
	af.location = APPLY_MOVE;
    af.modifier = -(number_range(25,50));
    af.duration = 1 + level;
    affect_to_char (victim, &af);
	
	
    SEND ("Magical webs begin restricting your movements!\r\n", victim);
    act ("Magical webs spray forth, covering $n!", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_shadow_bolt (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level - 1, 7);
    if (saves_spell (level, victim, DAM_NEGATIVE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
    return;
}


void spell_knock ( int sn, int level, CHAR_DATA *ch, void *vo , int target)
{
    char arg[MAX_INPUT_LENGTH];
    int chance=0;
    int door;
    const       sh_int  rev_dir         []              =
        {
            2, 3, 0, 1, 5, 4, 9, 8, 7, 6
        };

    target_name = one_argument(target_name,arg);

    if (arg[0] == '\0')
    {
    SEND("Knock which door or direction.\r\n",ch);
    return;
    }

    if (ch->fighting)
    {
        SEND("Wait until the fight finishes.\r\n",ch);
        return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
            { SEND( "It's already open.\r\n",      ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
            { SEND( "Just try to open it.\r\n",     ch ); return; }
        if ( IS_SET(pexit->exit_info, EX_NOPASS) )
            { SEND( "A mystical shield protects the exit.\r\n",ch ); return; };
        //if ( IS_SET(pexit->exit_info, EX_MAGICAL) )
        //    { SEND( "A mystical shield protects the exit from being      magically opened.\r\n",ch);return; }
    chance = ch->level / 5 + get_curr_stat(ch,STAT_INT) + get_skill(ch,sn) / 5;

    act("You cast knock on the $d, and try to open the $d!",
             ch,NULL,pexit->keyword,TO_CHAR);

    if (room_is_dark(ch->in_room))
                chance /= 2;

    /* now the attack */


    if (number_percent() < chance )
     {
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        act( "$n knocks on the $d and opens the lock.", ch, NULL,
                pexit->keyword, TO_ROOM );
        SEND( "You successfully open the door.\r\n", ch );

        /* open the other side */
  
  if ( ( to_room   = pexit->u1.to_room            ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
        &&   pexit_rev->u1.to_room == ch->in_room )
        {
            CHAR_DATA *rch;
               REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
                act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
        }
     }
    else
     {
        act("You couldn't magically open the $d!",
            ch,NULL,pexit->keyword,TO_CHAR);
        act("$n failed to magically open the $d.",
            ch,NULL,pexit->keyword,TO_ROOM);
     }
    return;
    }

  SEND("You can't see that here.\r\n",ch);
  return;
}


void spell_shadow_breath (int sn, int level, CHAR_DATA * ch, void *vo,
                        int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam, hp_dam, dice_dam;
    int hpch;
	int gain;

    act ("$n breathes forth a cone of shadow.", ch, NULL, victim, TO_NOTVICT);
    act ("$n breathes a cone of shadow over you!", ch, NULL, victim,
         TO_VICT);
    act ("You breath forth a cone of shadow.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX (10, ch->hit);
    hp_dam = number_range (hpch / 9 + 1, hpch / 5);
    dice_dam = dice (level, 20);

    dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);    

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_NEGATIVE))
            {                
                damage (ch, vch, dam / 2, sn, DAM_NEGATIVE, TRUE);
            }
            else
            {             
                damage (ch, vch, dam, sn, DAM_NEGATIVE, TRUE);				
				gain = vch->mana / 2;
				vch->mana	-= gain;
				vch->move	/= 2;
				ch->hit		+= gain;
            }
        }
        else
        {                        /* partial damage */

            if (saves_spell (level - 2, vch, DAM_NEGATIVE))
            {             
                damage (ch, vch, dam / 4, sn, DAM_NEGATIVE, TRUE);
            }
            else
            {             
                damage (ch, vch, dam / 2, sn, DAM_NEGATIVE, TRUE);				
				gain = vch->mana / 2;
				vch->mana	-= gain;
				vch->move	/= 2;
				ch->hit		+= gain;
            }
        }
    }
}

void spell_extension (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only do that to yourself.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already extending your spells!\r\n",
                          ch);     
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;	
	affect_to_char (victim, &af);
	
    SEND ("You begin extending your spell lengths!\r\n", victim);    
    return;
}


void spell_audible_glamor (int sn, int level, CHAR_DATA * ch, void *vo,
                           int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only do that to yourself.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already looking much spiffier.\r\n",
                          ch);     
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_CHA;
    af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
	af.bitvector = 0;
	affect_to_char (victim, &af);
	
    SEND ("You change your outward appearance!\r\n", victim);    
    return;
}



void spell_silence (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (victim == ch)
	{
        SEND("You probably don't want to do that to yourself.\r\n", ch);
		return;
	}
	if (IS_AFFECTED (victim, sn))
	{
		SEND("They already seem to be silenced!\r\n",ch);
		return;
	}
    if ( saves_spell (level, victim, DAM_MENTAL))
	{
		SEND("Your throat feels a bit scratchy for a second.\r\n", victim);
		act ("$n coughs briefly.", victim, NULL, NULL, TO_ROOM);
        return;
    }
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;    
	af.location = -1;
    af.modifier = 0;
	af.bitvector = 0;
    af.duration = number_range(1,3);    
    affect_to_char (victim, &af);
		
	
    SEND ("Your vocal chords constrict upon themselves!\r\n", victim);
    act ("$n gets an awkward look on their face!", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_scry (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	OBJ_DATA *device;   
	
	if (target_name[0] != '\0')
	{
		SEND("Scry whom?\r\n", ch);
		return;
	}
	
	for ( device = ch->carrying; device; device = device->next_content )
    {
        if (device->item_type == ITEM_SCRY)              
			break;
    }
	
	if (!device)
    { 
		SEND( "You need a scrying device to do this.\r\n", ch );
		return;
    }
	
	return;
}

void spell_detect_scrying (int sn, int level, CHAR_DATA * ch, void *vo,
                         int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only cast that on yourself.\r\n",ch);
		return;
	}
	
    if (is_affected(ch, gsn_detect_scrying))
    {
        SEND("You already have sensitivity to scrying.\r\n",ch);        
		return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You adapt a sense of intellectual awareness.\r\n", victim);    
    return;
}

void spell_displacement (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only cast that on yourself.\r\n",ch);
		return;
	}

	if (is_affected(ch, gsn_displacement))
    {
        SEND("You're already displaced between the planes.\r\n",ch);        
		return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range(af.duration * 1/5, af.duration * 2/5);

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You displace your image between the planes.\r\n", victim);    
	act("$n seems to be displaced between the planes.",ch,NULL,victim,TO_NOTVICT);
    return;
	
}

void spell_feeblemind (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (ch->grank == G_BACK)
	{
		SEND ("You can't do that while in the back ranks.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, gsn_feeblemind))
    {
        if (victim == ch)
            SEND ("Your mind is as feeble as it gets.\r\n", ch);
        else
            act ("$N's mind can't get any more feeble.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (saves_spell (level, victim, DAM_MENTAL) || IS_SET (victim->imm_flags, IMM_MAGIC))
    {
        if (victim != ch)
		{
            SEND ("Nothing seemed to happen.\r\n", ch);
			SEND ("You briefly lose your train of thought.\r\n", victim);
			return;
		}
    }    

    af.where = TO_AFFECTS;
    af.type = gsn_feeblemind;
    af.level = level;
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

	af.bitvector = 0;
    af.location = APPLY_INT;
    af.modifier = -4 - (level >= 18) - (level >= 25) - (level >= 32);
	affect_to_char (victim, &af);
	af.location = APPLY_MANA;	
	af.modifier = -number_range(20,50);    
    affect_to_char (victim, &af);
    SEND ("Your mind is turning to mush!\r\n", victim);
    act ("$n appears to be dumbfounded.", victim, NULL, NULL, TO_ROOM);    
	return;
}

void spell_detect_traps (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only cast that on yourself.\r\n",ch);
		return;
	}
	
    if (is_affected(ch, gsn_detect_traps))
    {
        SEND("You already sense traps.\r\n",ch);        
		return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel an inner sense of traps.\r\n", victim);    
    return;
}


void spell_detect_undead (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only cast that on yourself.\r\n",ch);
		return;
	}
	
    if (is_affected(ch, gsn_detect_undead))
    {
        SEND("You already sense the living dead.\r\n",ch);        
		return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel an inner sense of the undead.\r\n", victim);    
    return;
}

void spell_etherealness (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	return;
}

void spell_paralyzation (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (victim == ch)
	{
        SEND("You probably don't want to do that to yourself.\r\n", ch);
		return;
	}
	if (IS_AFFECTED (victim, sn)
        || saves_spell (level, victim, DAM_OTHER))
        return;
        
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = 1 + (level / 4);    
    affect_to_char (victim, &af);
	
	
    SEND ("Your muscles stiffen and you realize you no longer control them!\r\n", victim);
    act ("$n stiffens suddenly, a look of horror on their face!", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_enlarge_person (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if ( is_affected(victim, gsn_reduce_person))
	{
		if ( ch != victim )
		{
			if (!saves_spell(level, victim, DAM_MENTAL))
				affect_strip(victim, gsn_reduce_person);			
		}
		else
			affect_strip(victim, gsn_reduce_person);
		return;
	}

    if ( is_affected(victim, sn) || SIZE(ch) == MAX_SIZE)
    {
	if ( ch != victim )
            act("$N cannot grow anymore.",ch,NULL,victim,TO_CHAR);
	else
	    SEND( "You cannot grow anymore.\r\n",ch);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.modifier  = 1;
    af.location  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af);
	
	if (ch != victim)
		act("$N grows before your eyes.",victim,NULL,ch,TO_NOTVICT);
	else
		act("$N grows before your eyes.",ch,NULL,victim,TO_NOTVICT);		
	
	SEND( "You feel your body grow.\r\n",victim);
    return;
}

void spell_reduce_person (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if ( is_affected(victim, gsn_enlarge_person))
	{
		if ( ch != victim )
		{
			if (!saves_spell(level, victim, DAM_MENTAL))
				affect_strip(victim, gsn_enlarge_person);			
		}
		else
			affect_strip(victim, gsn_enlarge_person);
		return;
	}

    if ( is_affected(victim, sn) || what_size(ch) == SIZE_TINY)
    {
	if ( ch != victim )
            act("$N cannot grow any smaller.",ch,NULL,victim,TO_CHAR);
	else
	    SEND( "You cannot grow any smaller.\r\n",ch);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.modifier  = 1;
    af.location  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af);
	
	if (ch != victim)
		act("$N shrinks before your eyes.",victim,NULL,ch,TO_NOTVICT);
	else
		act("$N shrinks before your eyes.",ch,NULL,victim,TO_NOTVICT);		
	
	SEND( "You feel your body shrink.\r\n",victim);
    return;
}

void spell_disrupt_undead (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	if (!IS_UNDEAD(victim))
	{
		SEND("They aren't affected by your spell!\r\n",ch);
		return;
	}
	
    dam = dice (level, 3);
    if (saves_spell (level, victim, DAM_HOLY))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_HOLY, TRUE);
    return;
}

void spell_acid_arrow (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 5);
    if (saves_spell (level, victim, DAM_ACID))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_ACID, TRUE);
    return;
}

void spell_flame_arrow (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 5);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}

void spell_frost_arrow (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 5);
    if (saves_spell (level, victim, DAM_COLD))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_COLD, TRUE);
    return;
}


void spell_mana_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND ("You cannot cast that on others.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, sn))
    {
        SEND ("You are already protected by a mana shield.\r\n", ch);
		return;
    }
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = 24;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You are surrounded by a faint translucent aura.\r\n", victim);    
    return;
}

void spell_blink (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND ("You cannot cast that on others.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, sn))
    {
        SEND ("You are already blinking in and out of existance.\r\n", ch);
		return;
    }
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = 4;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("Your form starts to shimmer and flicker.\r\n", victim);    
	act("$n's form starts to shimmer and flicker.",ch,NULL,victim,TO_NOTVICT);
    return;	
}

void spell_lay_hands (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char buf[MSL];
    //if(IS_DEITY(victim))
    //    return;   

    if (ch->position == POS_FIGHTING)
    {
		SEND( "You can't while fighting.\r\n", ch);
		return;
    }
	
    if (ch != victim)
    {
		if ( IS_UNDEAD( victim ) && !is_safe( ch, victim ) )
		{										
			damage (ch, victim, ch->level * 3, sn, DAM_HOLY, TRUE);			
			SEND("Test!\r\n",ch);
			return;
		}
		else
		{
			victim->hit += number_range(((ch->level * 2) * 8 / 10), ch->level * 2);
			update_pos( victim );
			sprintf ( buf, "%s embraces %s with healing light.\r\n", god_table[ch->god].name, victim->name);
			SEND (buf, ch);
			sprintf(buf, "You feel a warm glow emanating from %s's hands.\r\n", ch->name);
			SEND(buf, victim);
			//do_glance(ch, victim->name);
			return;
		}
    }
    if (ch == victim)
    {
  	    victim->hit += number_range(((ch->level * 2) * 8 / 10), ch->level * 2);
	    update_pos( victim );
	    sprintf(buf, "%s embraces you with healing light.\r\n", god_table[victim->god].name);
		SEND( buf, victim );
		return;
    }
    return;
}

void spell_resistance (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (is_affected (victim, sn))
    {
		if (victim == ch)
		{
			SEND ("You are already resisting magic.\r\n", ch);
			return;
		}	
		else
		{
			SEND ("They are already resisting magic.\r\n",ch);
			return;
		}
    }
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = -1;
    af.location = APPLY_SAVES;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel resistant to magic.\r\n", victim);    
    return;	
}

void spell_detect_plants (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only cast that on yourself.\r\n",ch);
		return;
	}
	
    if (is_affected(ch, gsn_detect_undead))
    {
        SEND("You already sense individual plants.\r\n",ch);        
		return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel an inner sense of individual plants.\r\n", victim);    
    return;
}

void spell_entangle (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	
	
	if (victim == ch)
	{
        SEND("You probably don't want to do that to yourself.\r\n", ch);
		return;
	}
	if (IS_AFFECTED (victim, sn) || saves_spell (level, victim, DAM_OTHER))
	{
		SEND ("You avoid the reaching roots coming to life at your feet!\r\n", victim);
		act ("$n avoids the reaching roots coming to life at their feet!", victim, NULL, NULL, TO_ROOM);
		return;
    }
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_DEX;
    af.modifier = -4;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
	
	af.location = APPLY_MOVE;
    af.modifier = -(number_range(25,50));
    af.duration = 1 + level;
    affect_to_char (victim, &af);
	
	
    SEND ("Vines and roots spring from the ground, restricting your movements!\r\n", victim);
    act ("Vines and roots spring from the ground, covering $n!", victim, NULL, NULL, TO_ROOM);
	return;
}

void spell_shillelagh (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	OBJ_DATA *obj;
	
	obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf = NULL;	
	AFFECT_DATA af;
	
	if (obj->item_type != ITEM_WEAPON)
	{
		SEND("Your target must be a staff.\r\n",ch);
		return;
	}
	
	if (obj->value[0] != WEAPON_STAFF)
	{
		SEND("Your target must be a staff.\r\n",ch);
		return;
	}
	
	if (IS_SET(obj->extra2_flags, ITEM_SHILLELAGH))
	{
		act ("$p is already magically strengthened.", ch, obj, NULL, TO_CHAR);
		return;
	}
      
    paf = affect_find (obj->affected, gsn_curse);
	if (paf)
	{
		SEND("This object is cursed.\r\n",ch);
		return;
	}
	   
    act ("$p glows a soft {Ggreen{x.", ch, obj, NULL, TO_ALL);
    SET_BIT(obj->extra2_flags, ITEM_SHILLELAGH);        	
	
	af.where 		= TO_OBJECT;
	af.type 		= sn;
	af.level 		= level;
	af.duration 	= -1;
	if (is_affected (ch, gsn_extension))
		af.duration += number_range((af.duration / 5), (af.duration * 2/5));
	
	//Give a bit of damroll
	af.location 	= APPLY_DAMROLL;
	af.modifier 	= number_range(2,4);
	af.bitvector 	= ITEM_SHILLELAGH;
	affect_to_obj (obj, &af);
	
	//Give a bit of melee crit.
	af.location 	= APPLY_MELEE_CRIT;
	af.modifier 	= number_range(10,20);
	af.bitvector 	= ITEM_SHILLELAGH;
	affect_to_obj (obj, &af);
	
	return;   
}

void spell_sunbeam (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 9);
    if (saves_spell (level, victim, DAM_LIGHT))
        dam /= 2;
	if (!saves_spell (level, victim, DAM_LIGHT))
		spell_blindness (gsn_blindness, 3 * level / 4, ch, (void *) victim, TARGET_CHAR);
    damage (ch, victim, dam, sn, DAM_LIGHT, TRUE);
    return;
}

void spell_fire_seeds (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 4);
    if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}

void spell_thorns (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	if (victim != ch)
	{
        SEND("You can't cast this spell on another.\r\n", ch);
		return;
	}	
     
	if (is_affected(ch, sn))
	{
		SEND("You're already surrounded by magical thorns!\r\n",ch);
		return;
	}
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
		
    SEND ("You are surrounded by magical thorns!\r\n", victim);
    act ("$n is surrounded by magical thorns!", victim, NULL, NULL, TO_ROOM);
	return;	
}

void spell_water_breathing (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
		
	if (is_affected(ch, sn) && victim == ch)
	{
		SEND("You're already able to breathe underwater!\r\n",ch);
		return;
	}
	
	if (is_affected(victim,sn) && victim != ch)
	{
		SEND("They're already able to breathe underwater!\r\n",ch);
		return;
	}
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
		
    SEND ("Your lungs feel magically adapted!\r\n", victim);    
	return;	
}

void spell_changestaff (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	return;
}

void spell_transmute_metal (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
    //AFFECT_DATA *paf, *tmp;
   // int i;
    //bool found = FALSE;
    
	// require some material component what should it be.
	
	if ( obj->item_type != ITEM_ARMOR )
	{
		SEND("This spell must be cast upon armor.\r\n",ch);
		return;
	}
	
	if ( !(mat_table[get_material(obj->material)].is_metal) )
	{
		SEND("This spell must be cast upon metallic objects.\r\n",ch);
		return;
	}
	
    obj->material = str_dup("wood");
    
    obj_from_char(obj);
    obj->cost /= 3;
    obj->weight /= 3;   
    obj_to_char( obj, ch );
   /*
    if ( obj->item_type == ITEM_ARMOR ){
        for ( i = 0; i < 4; i++ )
            obj->value[i] = 2 * (obj->value[i] / 3);
        obj->value[4]--;
        obj->value[4] = URANGE( 0, obj->value[4], 5 );
        if(obj->enchanted){
            for (paf = obj->affected; paf; paf = paf->next ){
                if ( paf->location == APPLY_PHYSICAL_DEFENSE ){  
                    --paf->modifier;
                    if ( paf->modifier == 0 ){
			if(paf->next){
                            tmp = paf->next;
                            paf->next = tmp->next;
                            affect_remove_obj(obj,paf);
			}
			else{
                            affect_remove_obj(obj,paf);
			    obj->affected = NULL;
			}
                        break;
                    }
                }
            }
        }
        else{
            AFFECT_DATA *af_new;
            obj->enchanted = TRUE;
            
            // need to move all affects over to obj->affected, no longer stock
            for (paf = obj->pIndexData->affected; paf; paf = paf->next){
                af_new = new_affect();
                
                if (paf->location == APPLY_PHYSICAL_DEFENSE && paf->modifier - 1 == 0)
                    continue;
                
                af_new->next = obj->affected;
                obj->affected = af_new;
                
                af_new->where       = paf->where;
                af_new->type        = UMAX(0,paf->type);
                af_new->level       = paf->level;
                af_new->duration    = paf->duration;
                af_new->location    = paf->location;
                af_new->modifier    = paf->location == APPLY_PHYSICAL_DEFENSE ? 
                                            paf->modifier - 1 : paf->modifier;
                af_new->bitvector   = paf->bitvector;
            }
        }
    }
    else if ( obj->item_type == ITEM_WEAPON )
        obj->value[2]--;
    */
    act("You turn $p to $T.",ch,obj,obj->material,TO_CHAR);
    act("$n turns $p to $T.",ch,obj,obj->material,TO_ROOM);	
	
	
	return;
}

void spell_regeneration (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
		
	if (victim != ch)
	{
		SEND("You cannot cast this spell on another.\r\n",ch);
		return;
	}
	
	if (is_affected(ch, sn))
	{
		SEND("You're already magically regenerating.\r\n",ch);
		return;
	}
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
		
    SEND ("You begin rapidly regenerating.\r\n", victim);    
	return;	
}

void spell_animal_growth (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, sn) || ch->size == MAX_SIZE)
    {
	if ( ch != victim )
            act("$N cannot grow anymore.",ch,NULL,victim,TO_CHAR);
	else
	    SEND( "You cannot grow anymore.\r\n",ch);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.modifier  = 1;
    af.location  = APPLY_SIZE;
    af.bitvector = 0;
    affect_to_char( victim, &af);

    af.modifier  = ( victim->size / 2 ) + 1;
    af.location  = APPLY_DAMROLL;
    af.bitvector = 0;
    affect_to_char( victim, &af);

    act("$N grows before your eyes.",ch,NULL,victim,TO_NOTVICT);
    SEND( "You feel your body grow.\r\n",victim);
    return;
}

void spell_barkskin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
	{
		SEND("You can only do that to yourself.\r\n",ch);
		return;
	}
	
	if (is_affected (ch, sn))
    {      
        SEND ("Your skin is already as hard as bark.\r\n", ch);        
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.location = APPLY_AC;
    af.modifier = -40;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    act ("$n's skin turns to bark.", victim, NULL, NULL, TO_ROOM);
    SEND ("Your skin turns to bark.\r\n", victim);
    return;
}

void spell_stone_meld (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
	
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
	if (victim != ch)
	{
		SEND("You can only do that to yourself.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, sn))
    {	
		SEND ("You are already melded with the stone around you.\r\n", ch);
		return;
    }
	
	if (ch->in_room->sector_type != SECT_CAVE && ch->in_room->sector_type != SECT_MOUNTAIN)
	{	
		SEND ("There is nothing to meld with here.\r\n", ch);
		return;
	}
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You meld in with the stone around you.\r\n", victim);    
    return;	
}

void spell_farsight (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    if (IS_AFFECTED (ch, AFF_BLIND))
    {
        SEND ("Maybe it would help if you could see?\r\n", ch);
        return;
    }

	if (!saves_spell (level, vo, DAM_NONE))
		do_function (ch, &do_scan, target_name);
	else
	{
		SEND ("You fail to find them.\r\n",ch);
		if (number_percent() < 20)
			SEND ("You feeling a slight tingling as the hair raises on your neck.\r\n",vo);
	}
	return;
}


void spell_portal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;


	
    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || victim->in_room == NULL
        || !can_see_room (ch, victim->in_room)
        || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
        || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
        || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
        || victim->level >= level + 3 || (!IS_NPC (victim) && victim->level >= LEVEL_HERO)    /* NOT trust */
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && saves_spell (level, victim, DAM_NONE))
        || /*(is_clan (victim) &&*/ !is_same_clan (ch, victim)/*)*/)
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    stone = get_eq_char (ch, WEAR_HOLD);
    if (!IS_IMMORTAL (ch)
        && (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        SEND ("You lack the proper component for this spell.\r\n",
                      ch);
        return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
        act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
        extract_obj (stone);
    }

    portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = 2 + level / 25;
    portal->value[3] = victim->in_room->vnum;

    obj_to_room (portal, ch->in_room);

    act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
    act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);
}

void spell_nexus (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;

    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || (to_room = victim->in_room) == NULL
        || !can_see_room (ch, to_room) || !can_see_room (ch, from_room)
        || IS_SET (to_room->room_flags, ROOM_SAFE)
        || IS_SET (from_room->room_flags, ROOM_SAFE)
        || IS_SET (to_room->room_flags, ROOM_PRIVATE)
        || IS_SET (to_room->room_flags, ROOM_SOLITARY)
        || IS_SET (to_room->room_flags, ROOM_NO_RECALL)
        || IS_SET (from_room->room_flags, ROOM_NO_RECALL)
        || victim->level >= level + 3 || (!IS_NPC (victim) && victim->level >= LEVEL_HERO)    /* NOT trust */
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && saves_spell (level, victim, DAM_NONE))
        || (/*is_clan (victim) &&*/ !is_same_clan (ch, victim)))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    stone = get_eq_char (ch, WEAR_HOLD);
    if (!IS_IMMORTAL (ch)
        && (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        SEND ("You lack the proper component for this spell.\r\n",
                      ch);
        return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
        act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
        extract_obj (stone);
    }

    /* portal one */
    portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;

    obj_to_room (portal, from_room);

    act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
    act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
        return;

    /* portal two */
    portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = 1 + level / 10;
    portal->value[3] = from_room->vnum;

    obj_to_room (portal, to_room);

    if (to_room->people != NULL)
    {
        act ("$p rises up from the ground.", to_room->people, portal, NULL,
             TO_ROOM);
        act ("$p rises up from the ground.", to_room->people, portal, NULL,
             TO_CHAR);
    }
}

SPELL( spell_vocal_shield )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo; 
    OBJ_DATA *obj = NULL;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are protected by the notes.\r\n", ch);
        else
            act ("$N is already protected by your voice.", ch, NULL, victim, TO_CHAR);
        return;
    }


    if ( obj && IS_DRUMS(obj) && get_skill(ch, gsn_drums) > 1 )	
    {
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 28;
    af.modifier = -35;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    check_improve(ch, gsn_drums, TRUE, 7);
    }
    else{
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 20;
    af.modifier = -30;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    }
    SEND ("You hear an odd pattern of musical notes.\r\n", victim);
    if (ch != victim)
        act ("$N is protected by your voice.", ch, NULL, victim, TO_CHAR);
    return;
}

SPELL( spell_nourishing_ballad )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    int heal;
    bool maxhp = FALSE;

    if ( victim->hit <= victim->max_hit - 3 )
        maxhp = TRUE;
   

    if ( obj && IS_HORN(obj) && get_skill(ch, gsn_horn) > 1 )
    {
		heal = dice(3, (level/4 + number_range(10,20)));
		check_improve(ch, gsn_horn, TRUE, 7);
    }
    else
	{
		heal = dice(3, (level/4 + number_range(3,12)));
    }

    if ( IS_UNDEAD( victim ) )
    {
        if ( IS_NPC(ch))
                return;

        if (is_safe(ch, victim))
        {
            SEND("Not on that target.\r\n", ch);
            return;
        }
        damage(ch, victim, heal+24, sn, DAM_OTHER, TRUE);
        check_killer(ch, victim);
        return;
    }

    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    SEND( "You feel better!\r\n", victim );
    if ( ch != victim )
    //do_glance(ch, victim->name);

    if( IS_GOOD( ch ) && IS_GOOD( victim )
    &&  ch != victim && !IS_NPC( victim ) && maxhp )
        gain_exp( ch, number_range( 10, 20 ), FALSE );
    return;
}

SPELL( spell_adagio )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    AFFECT_DATA af;

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          SEND("You can't move any slower!\r\n",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }

    if ((saves_spell(level,victim,DAM_OTHER)) && ch != victim )
    {
        if (victim != ch)
            SEND("Nothing seemed to happen.\r\n",ch);
        SEND("You feel momentarily lethargic.\r\n",victim);
        return;
    }

    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level-3,victim,gsn_haste))
        {
            if (victim != ch)
                SEND("Song failed.\r\n",ch);
            SEND("You feel momentarily slower.\r\n",victim);
            return;
        }
        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        if(ch == victim)
            return;
    }   

    if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )	
    {
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    check_improve(ch, gsn_harp, TRUE, 7);
    }
    else {
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/3;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    }
    SEND( "You feel yourself slowing d o w n...\r\n", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;

}

SPELL( spell_cacophonic_shield )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("A cacophonic shield already surrounds you!\r\n", ch);
        else
            act ("$N is already surrounded with chaotic noise.", ch, NULL, victim, TO_CHAR);
        return;
    }


	if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )
    {
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = number_range((level/2), (level));
    af.modifier = -45;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    check_improve(ch, gsn_harp, TRUE, 7);
    }
    else {
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = number_range((level/3), (level/2));
    af.modifier = -40;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    }
    SEND ("A growing chorus of chaotic noise surrounds you.\r\n", victim);
    if (ch != victim)
        act ("$N is surrounded by chaotic noise.", ch, NULL, victim, TO_CHAR);
    return;
}

SPELL( spell_dissonant_chord )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD) )
    //||   IS_AFFECTED(victim, AFF_PROTECT_NEUTRAL) )
    {
        if (victim == ch)
          SEND("You are already protected.\r\n",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }


    if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )	
    {
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -2;
    af.bitvector = AFF_PROTECT_EVIL;    
	affect_to_char( victim, &af );
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char (victim, &af);

    //af.bitvector = AFF_PROTECT_NEUTRAL;
    //affect_to_char (victim, &af);

    
    check_improve( ch, gsn_harp, TRUE, 7);
    }
    else{
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(24);
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_EVIL;
	affect_to_char( victim, &af );
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char (victim, &af);

    //af.bitvector = AFF_PROTECT_NEUTRAL;
    //affect_to_char (victim, &af);

    
    }
    SEND( "You feel protected from others intentions.\r\n", victim );
    if ( ch != victim )
        act("$N is protected from others intentions.",ch,NULL,victim,TO_CHAR);
    return;
}


SPELL( spell_dirge_dischord )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }


    if (is_affected (victim, sn))
    {
        if (victim == ch)
	{
            SEND ("Why would you want to confuse yourself?!\r\n", ch);
	    return;
	}
        else
	{
            act ("$N already appears as confused as can be!", ch, NULL, victim, TO_CHAR);
       	    return;
	}
    }

    if ( !saves_spell(level,victim, DAM_CHARM) )
	{
		if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )		
		{
			af.where = TO_AFFECTS;
			af.type = sn;
			af.level = level;
			af.duration = level/4 + number_range(3,6);
			af.modifier = 0;
			af.location = 0;
			af.bitvector = 0;
			check_improve(ch, gsn_harp, TRUE, 7);
		}
    else 
	{
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level;
		af.duration = level/7 + number_range(2, 6);
		af.modifier = 0;
		af.location = 0;
		af.bitvector = 0;
    }
    affect_to_char (victim, &af);
    SEND ("You feel a little confused!\r\n", victim);
    if (ch != victim){
        act ("$N appears very confused!", ch, NULL, victim, TO_CHAR);
        return;
    }
    else
    {
        SEND("Doesn't seem to be any more confused than normal.", ch);
        return;
    }}
}

SPELL( spell_harmonic_chorus )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;
    OBJ_DATA *obj = NULL;

   

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ))
		{
			act( "$n looks somewhat enlightened!", gch, NULL, NULL, TO_ROOM );
			SEND( "You feel somewhat enlightened!\r\n", gch );

			if ( obj && IS_DRUMS(obj) && get_skill(ch, gsn_drums) > 1 )			
			{
				af.where     = TO_AFFECTS;
				af.type      = sn;
				af.level     = level;
				af.duration  = number_range ((level/4), (level/3));
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = 0;
				affect_to_char (gch, &af);
				check_improve( ch, gsn_drums, TRUE, 7);
			}
			else
			{
				af.where     = TO_AFFECTS;
				af.type      = sn;
				af.level     = level;
				af.duration  = number_range ((level/5), (level/4));
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bitvector = 0;
				affect_to_char (gch, &af);
			}
			
		}
    }
    return;

}

SPELL( spell_selective_hearing )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;

    if (victim != ch)
    {
        SEND("Your singing doesn't affect them!\r\n", ch);
        return;
    }

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You listen only to what you choose.\r\n", ch);
        else
            act ("$N already tunes you out half the time anyway!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if ( obj && IS_PICCOLO(obj) && get_skill(ch, gsn_piccolo) > 1 )	
	{
	af.where     = TO_RESIST;
	af.type      = gsn_selective_hearing;
	af.level     = level;
	af.duration  = level / 3;
	af.modifier  = 35;
	af.location  = APPLY_NONE;
	af.bitvector = RES_SOUND;
	check_improve( ch, gsn_piccolo, TRUE, 7);
	}
	else{
	af.where     = TO_RESIST;
        af.type      = gsn_selective_hearing;
        af.level     = level;
        af.duration  = level / 4;
        af.modifier  = 25;
        af.location  = APPLY_NONE;
        af.bitvector = RES_SOUND;
	}
        affect_to_char (victim, &af);
	SEND ("You listen only to what you choose!\r\n", victim);
    	if (ch != victim)
        	act ("$N gets a glazed look in their eyes, seeming to ignore you.", ch, NULL, victim, TO_CHAR);
    	return;
}

SPELL( spell_sacred_euphony )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    int heal;

    //obj = get_eq_char(ch,dominant_hand(ch,TRUE));

    if (victim->hit >= victim->max_hit){
	SEND("They apparently don't need further healing.\r\n",ch);
	return;
    }
    else
    {
    if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )	
	{
		heal = dice(level,4);
		check_improve( ch, gsn_harp, TRUE, 7);
	}
	else
	heal = dice(level,3);
	victim->hit += heal;
	act("$N appears a little bit better.",ch,NULL,victim,TO_CHAR);	
    }
}

SPELL( spell_improvisation )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    int dam;
    int dam_type;
    

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }
    
    dam_type = number_range (1,15);
    if (dam_type < 5){
    	dam = dice(level, 9);
	if ( saves_spell(level, victim, DAM_SOUND) )
            dam /= 2;
	if (is_affected(victim, gsn_selective_hearing))
            dam = (dam * 4) / 5;        
        if ( obj && IS_HORN(obj) && get_skill(ch, gsn_horn) > 1 )		
        {
            dam = (dam * 5) / 4;
            check_improve(ch, gsn_horn, TRUE, 7);
        }
    	damage( ch, victim, dam, sn, DAM_SOUND ,TRUE);
    	return;
    }
    if (dam_type >= 5 && dam_type < 10){
	dam = dice(level, 10);
        if ( saves_spell(level, victim, DAM_ENERGY) )
            dam /= 2;
	if (is_affected(victim, gsn_selective_hearing))
            dam = (dam * 4) / 5;	
        if ( obj && IS_LUTE(obj) && get_skill(ch, gsn_lute) > 1 )		
        {
            dam = (dam * 5) / 4;
            check_improve(ch, gsn_lute, TRUE, 7);
        }
        damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE );
        return;
    }
    if (dam_type > 9){
  	dam = dice(level, 11);
        if ( saves_spell(level, victim, DAM_CHARM) )
            dam /= 2;
	if (is_affected(victim, gsn_selective_hearing))
            dam = (dam * 4) / 5;	
        if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )		
        {
            dam = (dam * 5) / 4;
            check_improve(ch, gsn_harp, TRUE, 7);
        }
        damage( ch, victim, dam, sn, DAM_CHARM ,TRUE);
        return;
    }
}

SPELL( spell_wail_doom )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;
    int dam = 0;
   

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    if (is_affected(victim, gsn_wail_doom)){
	SEND("They are already counting the days.\r\n", ch);
	return;
    }
    else
    if (saves_spell(level, victim, DAM_CHARM )){
	 dam = dice( level, 2 );
	 if ( saves_spell(level+1, victim, DAM_CHARM) )
            dam /= 2;
	 damage( ch, victim, dam, sn, DAM_CHARM ,TRUE);
	 return;
    }
    else
    {
	 dam = dice( level, 2 );	 
         if ( saves_spell(level+1, victim, DAM_CHARM) )

        if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )		
		{
            dam = (dam * 3) / 2;
			check_improve( ch, gsn_harp, TRUE, 7);
		}

         dam /= 2;
         damage( ch, victim, dam, sn, DAM_CHARM ,TRUE);
  	 af.where     = TO_AFFECTS;
    	 af.type      = gsn_wail_doom;
    	 af.level     = level;
    	 af.duration  = level;
    	 af.modifier  = -(number_range(3,6));
    	 af.location  = APPLY_CON;
    	 af.bitvector = 0;
	 affect_to_char (victim, &af);

	 af.modifier  = -(number_range(20,50));
         af.location  = APPLY_HIT;
	 affect_to_char (victim, &af);

	 victim->hit -= number_range(10,30);

 	 af.modifier  = -(number_range(40,80));
         af.location  = APPLY_MANA;
	 affect_to_char (victim, &af);

	 victim->mana -= number_range(20,40);

  	 SEND( "You feel quite gloomy suddenly.\r\n", victim );
         act("$N appears a little depressed.",ch,NULL,victim,TO_CHAR);
    }

}

SPELL( spell_repulsive_din )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;
    

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    if (victim == ch)
    {
        SEND("You cannot do that to yourself.\r\n", ch);
        return;
    }

    if (is_affected(victim, gsn_repulsive_din)){
	SEND("They already appear quite repulsive.\r\n", ch);
	return;
    }
    if (saves_spell(level, victim, DAM_CHARM )){
     	 af.where     = TO_AFFECTS;
         af.type      = gsn_repulsive_din;
         af.level     = level;
         af.duration  = level;
         af.bitvector = 0;

		if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )		
		{
			af.modifier  = -(number_range(7,11));
			af.location  = APPLY_CHA;
			affect_to_char (victim, &af);

			check_improve(ch, gsn_harp, TRUE, 7);
		}
        else
		{
			af.modifier  = -(number_range(5,9));
			af.location  = APPLY_CHA;
			affect_to_char (victim, &af);
		}

         SEND( "You feel your features twist and gnarl as you become repulsive.\r\n", victim );
         act("$N transforms into an ugly beast right before your eyes!",ch,NULL,victim,TO_CHAR);
     }
     return;
}


SPELL( spell_resonating_echo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;

    //Find an instrument if any
	obj = get_eq_char( ch, WEAR_HOLD );
	
	if (!obj)
	{
		obj = get_eq_char( ch, WEAR_SECONDARY );
	}
	
	if (!obj)
	{
		obj = get_eq_char( ch, WEAR_WIELD );
	}
   
    if (is_safe (ch, victim) && victim != ch)
    {
		SEND("Not on that target.\r\n",ch);
		return;
    }

    if (victim != ch)
    {    
		if ( is_affected(victim, gsn_deafness) || saves_spell(level, victim, DAM_SOUND) || is_affected(victim, gsn_resonating_echo))
		{
			act("$N can't hear you.",ch,NULL,victim,TO_CHAR);
			return;
		}

		if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )
		{
			af.where     = TO_AFFECTS;
			af.type      = gsn_deafness;
			af.level     = level;
			af.duration  = level / 3;
			af.modifier  = level / 2;
			af.location  = APPLY_AC;
			af.bitvector = 0;
			check_improve(ch, gsn_harp, TRUE, 7);
		}
		else
		{
			af.where     = TO_AFFECTS;
			af.type      = gsn_deafness;
			af.level     = level;
			af.duration  = level  / 4;
			af.modifier  = level / 3;
			af.location  = APPLY_AC;
			af.bitvector = 0;
		}
			affect_to_char (victim, &af);

			SEND( "You can't hear a thing!\r\n", victim );
			act("$N seems a bit confused about what you're saying.",ch,NULL,victim,TO_CHAR);
	}
	else
	{
		if ( IS_AFFECTED(victim, gsn_resonating_echo) )
		{
			SEND("A resonating echo already surrounds you.\r\n",ch);
			return;
		}   
		return;
	}
    return;
    
}


SPELL( spell_forgetful_lullaby )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are already shaken up!\r\n", ch);
        else
            act ("$N is already shaken up!", ch, NULL, victim, TO_CHAR);
        return;
    }


    if(!saves_spell(level+4, victim, DAM_CHARM))
    {
        if( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )		
		{
			af.where = TO_AFFECTS;
			af.type = sn;
			af.level = level;
			af.duration = number_range (level / 6, level / 2);
			af.location = APPLY_MANA;
			af.modifier = 0 - number_range(200, 250);
			af.bitvector = 0;
			affect_to_char (victim, &af);

			af.location = APPLY_SPELL_DAM;
			af.modifier = 0 - number_range (7, 12);
			af.bitvector = 0;
			affect_to_char (victim, &af);

			af.location = APPLY_INT;
			af.modifier = 0 - number_range (7, 12);
			af.bitvector = 0;
			affect_to_char (victim, &af);
			check_improve(ch, gsn_harp, TRUE, 7);
		}
	}
	else
	{
    	af.where = TO_AFFECTS;
    	af.type = sn;
    	af.level = level;
    	af.duration = number_range (level / 8, level / 4);
    	af.location = APPLY_MANA;
    	af.modifier = 0 - number_range(150, 200);
    	af.bitvector = 0;
    	affect_to_char (victim, &af);

		
    	af.location = APPLY_SPELL_DAM;
    	af.modifier = 0 - number_range (5, 10);
    	af.bitvector = 0;
    	affect_to_char (victim, &af);
		
		
    	af.location = APPLY_INT;
    	af.modifier = 0 - number_range (5, 10);
    	af.bitvector = 0;
    	affect_to_char (victim, &af);
	}
	victim->mana -= number_range(50, 100);
    
    	SEND ("You're shaken by the intense song!\r\n", victim);
    	if (ch != victim)
		{
    	    act ("$N is shaken by the intense song!", ch, NULL, victim, TO_CHAR);
			return;	
		}
		else
		{
			SEND ("You seem to resist the effects of that intense song.\r\n", victim);
			//if (ch != victim)
			act ("$N resists the effects of the intense song!", ch, NULL, victim, TO_ROOM);
		}

}

SPELL( spell_hymn_fallen )
{

    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;


    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
      {
          vch_next        = vch->next_in_room;
          if ( vch->in_room == NULL ){
              break;
          }
          if (is_safe (ch, vch) && !IS_NPC(vch))
    	  {
		SEND("Not on that target.\r\n",ch);
		break;
    	  }

          else
          {
              if ( is_same_group( vch, ch )) 
	      {

                if ( !is_affected (vch, gsn_hymn_fallen) ) 
		{
                    af.where     = TO_AFFECTS;
                    af.type      = sn;
                    af.level     = level;
                    af.duration  = level/6;
                    af.modifier  = (level / 8);
                    af.location  = APPLY_DAMROLL;
                    af.bitvector = 0;
                    
                    affect_to_char (vch, &af);

                    if ( vch != ch )
                    {
                    	act("$n seems to be mourning a lost loved one.",vch,NULL,NULL,TO_ROOM);
                    }
                    else
                    {
                        SEND("You begin mourning the loss of your loved ones.\r\n",ch);
                    }
            	}
		if (!is_affected(vch, gsn_regeneration))
	    	{	
    		    af.where     = TO_AFFECTS;
    		    af.type      = gsn_regeneration;
    		    af.level     = level;
    		    af.duration  = level/6;
   		    af.modifier  = 0;
    		    af.location  = APPLY_NONE;
    		    af.bitvector = 0;
		    affect_to_char (vch, &af);
		    if ( vch != ch )
	            {
            		SEND("Your cuts heal faster.\r\n",vch);
            		act("$n's cuts seem to close instantaneously.",vch,NULL,NULL,TO_ROOM);
        	    }
        	    else
		    {
            		SEND("Your cuts heal faster.\r\n",vch);
        	    }

	        }
	  }
	  if (!is_same_group(vch, ch) && !is_affected(vch, gsn_hymn_fallen)){
	    if(!IS_NPC(vch) && vch->level < ch->level - 8)
		SEND("Not on that target!.\r\n", ch);
            else if(!saves_spell(level, vch, DAM_CHARM)){
                  WAIT_STATE(vch, 3 * PULSE_VIOLENCE );
        if( obj && IS_DRUMS(obj) && get_skill(ch, gsn_drums) > 1 )		
		{
		  af.where = TO_AFFECTS;
		  af.type = gsn_hymn_fallen;
		  af.level = level;
		  af.duration = level / 4;
		  af.modifier = - (level / 6);
		  af.bitvector = 0;
		  af.location = APPLY_DAMROLL;
		  affect_to_char(vch, &af);

		  af.modifier = - (level / 6);
		  af.bitvector = 0;
		  af.location = APPLY_HITROLL;
		  affect_to_char (vch, &af);
                  af.where = TO_AFFECTS;
		  check_improve(ch, gsn_drums, TRUE, 7);
		  }
		  else{
                  af.type = gsn_hymn_fallen;
                  af.level = level;
                  af.duration = level / 5;
                  af.modifier = -(level / 8);
                  af.bitvector = 0;
                  af.location = APPLY_DAMROLL;
                  affect_to_char (vch, &af);
		  }
                  act("$n's seems to be affected by the song!",vch,NULL,NULL,TO_ROOM);
	          vch->fighting = ch;
              }
	  }

	  }
      }
  return;
}

SPELL( spell_contradicting_melody )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af; 
    OBJ_DATA *obj = NULL;   

    if (is_safe (ch, victim))
        return;

    if (victim == ch) {
	SEND ("You wouldn't want to do that...\r\n", ch);
	return;
    }
    if ( IS_UNDEAD( victim )){
	SEND ("You don't hold any power over the undead..\r\n", ch);
        return;
    }
    if ( IS_PET( ch ) )
	{
		SEND ("That's probably not the best idea you've had..\r\n", ch);
		return;
    }
    else
    {
	if (IS_AFFECTED(victim, AFF_CHARM) && ( victim->master != ch)){
    	    if ( saves_spell(level, victim,DAM_CHARM) ){
		SEND("They seem content with their current master.\r\n",ch);
		return;
	    }
	    else
	    {
			victim->master = ch;
			//victim->start_following( ch );
			victim->leader = ch;
			af.where     = TO_AFFECTS;
			af.type      = sn;
			af.level     = level;
			if ( IS_NPC(victim) )
				af.duration  = number_range(6, 10) + (ch->level / 10);
			else if( obj && IS_LUTE(obj) && get_skill(ch, gsn_lute) > 1 )			
			{
				af.duration = number_range(4, 7);
				check_improve(ch, gsn_lute, TRUE, 7);
			}
			else
			af.duration  = number_range( 3, 5 );
			af.location  = 0;
			af.modifier  = 0;
			af.bitvector = AFF_CHARM;
			SEND("They seem to like you more!\r\n", ch);
			act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
	    }
	}
    }
}

SPELL( spell_melody_masked_intent )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;
    

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You are protected from scrying.\r\n", ch);
        else
            act ("$N is already protected from scrying.", ch, NULL, victim, TO_CHAR);
        return;
    }
   if( obj && IS_DRUMS(obj) && get_skill(ch, gsn_drums) > 1 )   
   {
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level + 1;
    af.duration = level / 2;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    }
    else{
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level + 1;
    af.duration = level / 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    }
    affect_to_char (victim, &af);
    SEND ("You feel protected from scrying.\r\n", victim);
    if (ch != victim)
        act ("$N is protected from scrying.", ch, NULL, victim, TO_CHAR);
    return;

}

SPELL( spell_motivation )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;
    int heal;
  

    if(is_affected(victim,gsn_motivation))
    {
        if (victim == ch)
            SEND ("You are already inspired beyond normality.\r\n", ch);
        else
            act ("$N is already inspired beyond normality.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (is_affected (victim, skill_lookup ("calm")))
    {
        if (victim == ch)
            SEND ("Why don't you just relax for a while?\r\n", ch);
        else
            act ("$N doesn't look inspired anymore.",
                 ch, NULL, victim, TO_CHAR);
        return;
    }
    if( obj && IS_DRUMS(obj) && get_skill(ch, gsn_drums) > 1 )	
	{
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 3;
    af.modifier = level / 10;
    af.bitvector = 0;
    af.location = APPLY_HITROLL;
    affect_to_char(victim, &af);
    af.location = APPLY_DAMROLL;
    affect_to_char(victim, &af);
    check_improve(ch, gsn_drums, TRUE, 7);
    }
    else{
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 5;
    af.modifier = level / 12;
    af.bitvector = 0;

    af.location = APPLY_HITROLL;
    affect_to_char (victim, &af);

    af.location = APPLY_DAMROLL;
    affect_to_char (victim, &af);
    }
    SEND ("You are filled with inspiration!\r\n", victim);
    act ("$n gets an inspirational look in $s eyes!", victim, NULL, NULL, TO_ROOM);

    heal = dice (2, 10) + (level / 7);
    victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);
    return;
}

SPELL( spell_vitalizing_verse )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    OBJ_DATA *obj = NULL;


    if (check_dispel (level, victim, gsn_plague))
    {
        act ("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
             TO_ROOM);
    }
    if (check_dispel (level, victim, gsn_poison))
    {
        act ("$n looks less ill.", victim, NULL, NULL,
             TO_ROOM);
    }

    SEND ("Your body responds to the musical notes.\r\n", victim);
    act ("$ns body responds to the musical notes.", victim, NULL, NULL, TO_ROOM);

    heal = dice (7, 15) + level / 4;
    
	if( obj && IS_DRUMS(obj) && get_skill(ch, gsn_drums) > 1 )	
	{
		heal = dice (9, 15) + level /3;
		check_improve(ch, gsn_drums, TRUE, 7);
    }
	victim->hit = UMIN (victim->hit + heal, victim->max_hit);
    update_pos (victim);
    return;


}

SPELL( spell_hypnotic_verse )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;


    if (is_safe (ch, victim))
        return;

    if (victim == ch)
    {
        SEND ("You sing to yourself, how sweet!\r\n", ch);
        return;
    }
    if ( IS_UNDEAD( victim )){
        SEND ("You don't hold any power over the undead..\r\n", ch);
        return;
    }
    if ( IS_AFFECTED(victim, AFF_CHARM) || IS_AFFECTED(ch, AFF_CHARM) ||
        (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE))
        || saves_spell(level,victim,DAM_CHARM))
        return;
        

	if( (!IS_NPC(ch)) && ((ch->pet_tracker + 1) > cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	    
	if ( !IS_NPC(ch))
		ch->pet_tracker++;
	
    victim->leader = ch;
	victim->master = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if ( IS_NPC(victim))
            af.duration  = number_range(6, 10) + (ch->level / 10);
    else  if( obj && IS_LUTE(obj) && get_skill(ch, gsn_lute) > 1 )	
	{
	    af.duration = number_range ( 4, 7);
	    check_improve(ch, gsn_lute, TRUE, 7);
    }
    else
        af.duration  = number_range( 3, 5 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n a wonderful singer?", ch, NULL, victim, TO_VICT );

    if ( ch != victim )
        act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
	ch->charms++;
    return;

}

SPELL (spell_insidious_chord)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    int dam, loops;
 
    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    /*if (( !is_using_eq(victim,ITEM_INSTRUMENT,false))) {
	SEND ("You don't have the proper equipment!"\r\n", ch);
	return;
    }*/
    loops = number_range(UMAX(1,ch->level/22),UMAX(3,ch->level/12));
    for ( ; loops > 0; loops-- )
    {
        dam = dice(UMAX(1,(2 * ch->level)/3), number_range(1,4));	
	if (is_affected(victim, gsn_selective_hearing))
	    dam = (dam * 4) / 5;
	

       

	if ( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )
	{
	    dam = (dam * 5) / 4;
            check_improve(ch, gsn_harp, TRUE, 7);
  	}
	if ( saves_spell(level, victim,DAM_SOUND) )
            dam /= 2;
        damage( ch, victim, dam, sn, DAM_SOUND ,TRUE);
    }
    return;
}

SPELL (spell_banshee_ballad)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    int dam, stun;

    dam = dice( level, 9 );

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    stun =  number_percent( );
    stun = (stun + ((ch->level - victim->level) * 2) );


    if (stun >= 85)
    {
        act("A horrifying scream deafens you!",ch,NULL,victim,TO_NOTVICT);
        act("Your body goes into shock from horror!",ch,NULL,victim,TO_VICT);
        act("$N looks terrified!",ch,NULL,victim,TO_CHAR);

        DAZE_STATE(victim, (number_range(0,1)) * PULSE_VIOLENCE);
        victim->position = POS_RESTING;
    }    
    if ( obj && IS_HORN(obj) && get_skill(ch, gsn_horn) > 1 )	
	{
		dam = (dam * 5) / 4;
		check_improve(ch, gsn_horn, TRUE, 7);
	}
    if (is_affected(victim, gsn_selective_hearing))
            dam = (dam * 4) / 5;
    if ( saves_spell(level, victim, gsn_banshee_ballad ) )
        dam /= 2;
    //if they're using a piccolo, add damage
    damage( ch, victim, dam, sn,DAM_SOUND,TRUE );
   
}


SPELL (spell_crescendo)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;
    int duration = 0;
    

    if (is_affected (victim, sn))
    {
        if (victim == ch){
	    SEND ("Your voice is already in crescendo.\r\n", ch);
	    return;
	}
    }
    if( obj && IS_HORN(obj) && get_skill(ch, gsn_horn) > 1 )	
	{
		duration = (level / 3);
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level;
		af.duration = duration;
		af.modifier = -(level / 5);
		af.bitvector = 0;
		af.location = APPLY_SAVES;
		affect_to_char (victim, &af);
		af.modifier = level / 5;
		af.location = APPLY_SPELL_DAM;
		affect_to_char (victim, &af);
		check_improve(ch, gsn_horn, TRUE, 7);
    }
    else
	{ 
		duration = (level / 4) + 1;
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level;
		
		af.duration = duration;
		af.modifier = -(level / 7);
		af.bitvector = 0;
		af.location = APPLY_SAVES;
		affect_to_char (victim, &af);
		
		af.duration = duration;
		af.modifier = level / 7;
		af.location = APPLY_SPELL_DAM;
		affect_to_char (victim, &af);
    }

    SEND ("You hear your voice become deeper.\r\n", victim);
    return;
}

SPELL( spell_calming_verse)
{
    CHAR_DATA *vch;
    OBJ_DATA *obj = NULL;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;


    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->position == POS_FIGHTING)
        {
            count++;
            if (IS_NPC(vch))
              mlevel += vch->level;
            else
              mlevel += vch->level/2;
            high_level = UMAX(high_level,vch->level);
        }
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    //if( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )
	if ( obj )
	{
		mlevel = mlevel - 2;
		if (mlevel < 1)
			mlevel = 1;
    }
   
    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;
    
    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (IS_NPC(vch) && IS_UNDEAD( vch ))
                continue;

            if ( ch->spec_fun == spec_lookup( "spec_clan_guardian" ))
                continue;

            if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
            ||  is_affected(vch,gsn_frenzy) || is_affected(vch, gsn_calm) || is_safe_spell(ch,vch,FALSE))
                continue;

            if ( ch->clan > CLAN_NONE && !IS_NPC(vch))
                continue;
            if ( ch->clan > CLAN_NONE && vch->clan == CLAN_NONE && !IS_NPC( vch ))
                continue;
            if ( saves_spell(level +3, vch, DAM_CHARM ))
                continue;
            SEND("A wave of calm passes over you.\r\n",vch);
           if (vch->fighting || vch->position == POS_FIGHTING)
              stop_fighting(vch,FALSE);

            af.where = TO_AFFECTS;
            af.type = sn;
            af.level = level;
            af.duration = level/4;
            if( obj && IS_HARP(obj) && get_skill(ch, gsn_harp) > 1 )			
			{
				af.duration = level / 3;
				check_improve(ch, gsn_harp, TRUE, 7);
            }
            af.location = APPLY_HITROLL;
            if (!IS_NPC(vch))
              af.modifier = -5;
            else
              af.modifier = -2;
            af.bitvector = AFF_CALM;
           affect_to_char (vch, &af);

            af.location = APPLY_DAMROLL;
            affect_to_char (vch, &af);
        }
    }


}

SPELL( spell_diminishing_scale)
{
    ROOM_INDEX_DATA *pRoomIndex;
    CHAR_DATA *gch, *gch_next = NULL;
    OBJ_DATA *obj = NULL;

   

  for ( gch = ch->in_room->people; gch; gch = gch_next)
  //for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
   {
       if ( is_same_group( gch, ch ))
       {
           act( "$n suddenly disappears!", gch, NULL, NULL, TO_ROOM );
           SEND( "You feel strange for a moment.\r\n", gch );

    	   pRoomIndex = get_random_room(gch);

           char_to_room( gch, pRoomIndex );

    	   // If you teleport a sleeping person, extra penalty for you!
    	   if (gch->position < POS_SLEEPING)
    	   {
				//ch->pcdata->quit_counter = 10;
				if( obj && IS_PICCOLO(obj) && get_skill(ch, gsn_piccolo) > 1 )				
				{
					//ch->pcdata->quit_counter = 8;
					check_improve(ch, gsn_piccolo, TRUE, 7);
				}
    	   }
    	   do_look( gch, "auto" );
    	   return;
   	}
    }
    return;
}

SPELL( spell_assisted_coda)
{
//epic song
    CHAR_DATA *townsfolk;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj = NULL;
    int fail, i;

    

    if ( IS_NPC(ch))
        return;

    if (IS_EVIL(ch) || IS_NEUTRAL(ch))
    {
	SEND("You're not able get the lyrics right!\r\n",ch);
	return;
    }
    /*
	if( (!IS_NPC(ch)) && ch->pet_tracker >= charisma_check( ch ))
    {
        SEND("You cannot control more followers...\r\n", ch);
        return;
    }
	*/
    fail = 100 - get_skill( ch, gsn_assisted_coda );

    if( obj && IS_HORN(obj) && get_skill(ch, gsn_horn) > 1 )	
	{
		fail = 90 - get_skill( ch, gsn_assisted_coda );
		check_improve(ch, gsn_horn, TRUE, 7);
    }
    
    if ( number_percent() < fail / 2 )
    {
        pMobIndex = get_mob_index( 66 );
        townsfolk = create_mobile(pMobIndex);
        townsfolk->max_hit = (3 * ch->max_hit);
        townsfolk->hit = townsfolk->max_hit;
        townsfolk->level = (ch->level + 2);
        char_to_room(townsfolk, ch->in_room);
        townsfolk->fighting = ch;
        ch->fighting = townsfolk;
        townsfolk->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
        SET_BIT(townsfolk->form, FORM_INSTANT_DECAY);
        //townsfolk->poof_timer = 2;
        SEND( "The townsfolk were a little busy!\r\n", ch );
        act( "$n has summoned angry townsfolk!", ch, NULL, NULL, TO_ROOM );
        return;
    }
    else
    {
	pMobIndex = get_mob_index( 66 );
        townsfolk = create_mobile(pMobIndex);
        townsfolk->max_hit = (3 * ch->max_hit);
        townsfolk->hit = townsfolk->max_hit;
        townsfolk->level = (ch->level + 2);
        char_to_room(townsfolk, ch->in_room);
	SET_BIT(townsfolk->affected_by, AFF_CHARM);
        townsfolk->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
        SET_BIT(townsfolk->form, FORM_INSTANT_DECAY);
       // townsfolk->poof_timer = number_range(2 * ( level / 3), level);
        SET_BIT(townsfolk->form, FORM_INSTANT_DECAY);
        //if (!IS_NPC(ch))
            //ch->pcdata->pet_tracker += 2;
        for (i = 0; i < 3; i++)
        townsfolk->armor[i]             = townsfolk->level * (-4);
        townsfolk->armor[3]             = townsfolk->level * (-3);

        for (i = 0; i < 2; i++)
            townsfolk->damage[i]        = townsfolk->level/(number_range(6,10));
        townsfolk->damage[2]    	= townsfolk->level/2;

         townsfolk->hitroll             = 2 * (townsfolk->level/3);
         townsfolk->damroll             = townsfolk->level/2;

        for (i = 0; i < MAX_STATS; i++)
            townsfolk->perm_stat[i] = 11 + townsfolk->level/4;
	townsfolk->master = ch;
        //townsfolk->start_following( ch );
        townsfolk->leader = ch;

        SEND( "The townsfolk come quickly to your aid!\r\n", ch );
        act( "$n has summoned some townsfolk!", ch, NULL, NULL, TO_ROOM );
        return;
    }

}


SPELL( spell_aggressive_staccato)
{
//epic song
    //int drainHp, dam;
    int dam;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj = NULL;


    for ( vch = char_list; vch != NULL; vch = vch_next )
      {
          vch_next        = vch->next;
          if ( vch->in_room == NULL ){
              break;
          }
        
	  else
          {

     	  if ( saves_spell(level, vch,DAM_NEGATIVE) )
      	  {
       	 	SEND("You feel a momentary chill.\r\n",vch);
        	return;
      	  }


      	  if ( vch->level <= 2 )
      	  {
        	dam              = ch->hit + 1;
      	  }
          else
          {
              	gain_exp( vch, 0 - number_range( level/2, 3 * level / 2 ), FALSE );
        	vch->mana    /= 2;
        	vch->move    /= 2;
        	dam              = dice(1, level);
        	//ch->hit         += dam;
    	  }

   	  SEND("The tremendous cacophony drains you of energy!\r\n",vch);
    	  //SEND("The song gives you an adrenaline rush!\r\n",ch);
    	  //damage_old( ch, vch, dam, sn, DAM_NEGATIVE ,TRUE, isObj );
	  }
    	  return;
}
}

SPELL( spell_dynamic_diapason)
{
}

SPELL( spell_gnomish_tribute)
{
}

SPELL( spell_suggestive_verse)
{
//
}

SPELL( spell_Klangfarbenmelodie)
{
//random damage DoT
}

SPELL( spell_audible_intoxication)
{
  //AFFECT_DATA af;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int amount = 0;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
      {
          vch_next        = vch->next_in_room;
          if ( vch->in_room == NULL ){
              break;
	  }
	  if ( vch == ch )
	  {
	      break;
	  }
	  else
	  {
	      	if ( saves_spell(level + 3, vch, DAM_SOUND) )
			{
	      	   if ( ch->pcdata->condition[COND_DRUNK] > 1)
					amount = ch->pcdata->condition[COND_DRUNK];

				gain_condition(vch, COND_DRUNK, amount + 15);
	      	    SEND("You feel very drunk suddenly!\r\n",vch);
		}
	  }
      }
return;
}

SPELL( spell_infernal_threnody )
{
//cancellation spell
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;
    bool found = FALSE;

   // obj = get_eq_char(ch, dominant_hand(ch, TRUE));

    if (is_safe (ch, victim))
    {
	SEND("Not on that target.\r\n",ch);
	return;
    }

    level += 2;

    if ( !IS_NPC( victim ) ){
        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
            SEND("You failed, try dispel magic.\r\n",ch);
            return;
        }       
    }
    else{
        if ( victim->master == NULL ){
            SEND("You failed, try dispel magic.\r\n",ch);
            return;
        }
    }
    // no level decrement for cancellation and more tries at it
    int i = 0;
    //if( obj && IS_PICCOLO(obj) && get_skill(ch, gsn_piccolo) > 1 )
	if (obj)
       check_improve(ch, gsn_piccolo, TRUE, 7);


   while(victim->affected && i++<15){
        //if(checkIfDispelTrue(ch,victim,level)){
            found = TRUE;
        //}
    }
    if (found)
        SEND("Ok.\r\n",ch);
    else
        SEND("Spell failed.\r\n",ch);

// check_dispel( int dis_level, CHAR_DATA *victim, int sn)

}

SPELL( spell_listening_jewel )
{
    OBJ_DATA *obj;

    if ( IS_IMMORTAL(ch) && get_trust(ch) < 59 )
    {
        SEND("If you can't set it, you can't do it.\r\n",ch);
        return;
    }

    obj = get_obj_carry(ch,target_name,ch);
    if (obj == NULL)
    {
        SEND ("You don't have one of those.\r\n",ch);
        return;
    }

    if (obj->item_type == ITEM_GEM)
    {
        SEND ("You ward the object with powerful divination.\r\n", ch);
        SET_BIT(obj->extra2_flags, ITEM_SCRY);
		//obj->scryer = ch;
        return;
    }
    else
    {
        SEND("But it's not designed for that!\r\n",ch);
        return;
    }
}

SPELL( spell_orbitting_rhythm_lead)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;

    //obj = get_eq_char(ch, dominant_hand(ch, TRUE));

    // here is the perfect spot to check for AFF_SANCTUARY - it'll check all other sanc spells too! -- alex
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
    {
	// victim = ch  -- you're not assigning it, you're comparing it -- fixed, alex
        if ( victim == ch )
          SEND("You already have an orbitting rhythm and lead going, go hit the drums!\r\n",ch);
        else
          act("$N is already orbitted by rhythm and lead.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 7;
    //if( obj && IS_LUTE(obj) && get_skill(ch, gsn_lute) > 1 )
	if ( obj )
	{
		af.duration  = level / 5;
		check_improve(ch, gsn_lute, TRUE, 7);
    }
    af.location  = APPLY_NONE;
    af.modifier  = level;
    af.bitvector = AFF_SANCTUARY; // here is where the bitvector it used...  AFF_SANCTUARY! -- alex
    affect_to_char( victim, &af );
    act( "$n is surrounded by orbitting rhythm and lead trails.", victim, NULL, NULL, TO_ROOM );
    SEND( "You are surrounded by orbitting rhythm and lead trails.\r\n", victim );
    return;
}

SPELL( spell_quicken_tempo)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj = NULL;

    //obj = get_eq_char(ch, dominant_hand(ch, TRUE));

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim, AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
        if (victim == ch)
          SEND("You can't sync with the tempo any better!\r\n",ch);
        else
          act("$N is already in step with the tempo.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW) || (is_affected(victim, gsn_adagio)) )
    {
        if (!check_dispel(level,victim,gsn_slow))
        {
            if (victim != ch)
            SEND("Spell failed.\r\n",ch);
            SEND("You can't seem to hear the beat in your head.\r\n",victim);
            return;
        }
        act("$n is moving less slowly, but they're still not up to beat.",victim,NULL,NULL,TO_ROOM);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = gsn_quicken_tempo;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/4;
    else
      af.duration  = level/8;
    af.location  = APPLY_DEX;
    //if( obj && IS_PICCOLO(obj) && get_skill(ch, gsn_piccolo) > 1 )
	if ( obj )
	{
      af.modifier  = 3 + (level >= 18) + (level >= 35);
      check_improve(ch, gsn_piccolo, TRUE, 7);
    }
    else
    af.modifier  = 1 + (level >= 18) + (level >= 35);
    af.bitvector = AFF_HASTE;  //add the bitvector again -- Alex    
    affect_to_char( victim, &af );
    SEND( "You begin moving quicker, matching the tempo.\r\n", victim );
    act("$n begins moving quicker, matching the tempo.",victim,NULL,NULL,TO_ROOM);
    return;
}


SPELL ( spell_summon_monster )
{
	OBJ_DATA 		*heart;
	CHAR_DATA 		*demon;
	MOB_INDEX_DATA 	*mob;	
	int demon_type = 0;
	int i = 0;
	
	if ((heart = get_obj_carry (ch, "heart", ch)) == NULL)
    {
        SEND ("You do not have the proper the component for this spell.\r\n", ch);
        return;
    }	
	
	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	if (heart->level < 5)	
		demon_type = 1;
	else if (heart->level > 4 && heart->level < 10)
		demon_type = 2;
	else if (heart->level > 9 && heart->level < 15)
		demon_type = 3;
	else if (heart->level > 14 && heart->level < 20)
		demon_type = 4;
	else if (heart->level > 19 && heart->level < 30)
		demon_type = 5;
	else if (heart->level > 29 && heart->level < 40)
		demon_type = 6;
	else
		demon_type = 7;
	
	mob = get_mob_index( 1 ); 
	demon = create_mobile(mob);
	
	demon->max_hit = number_range ( heart->level * 20, heart->level * 100 );
	demon->hit = demon->max_hit;
	demon->max_mana = number_range ( heart->level * 50, heart->level * 200 );
	demon->mana = demon->max_mana;
	demon->level = number_fuzzy(heart->level);
	demon->armor[AC_PIERCE] = 	150 - (heart->level * 10);
    demon->armor[AC_BASH] = 	150 - (heart->level * 10);
    demon->armor[AC_SLASH] = 	150 - (heart->level * 10);
    demon->armor[AC_EXOTIC] = 	150 - (heart->level * 10);
	demon->damroll =		number_fuzzy ( heart->level );
	demon->hitroll = 		number_fuzzy ( heart->level / 2 );
	for (i = 0; i < 2; i++)
        demon->damage[i] = 	demon->level/(number_range(6,12));
    demon->damage[2]	 = 	demon->level/2;
	for (i = 0; i < MAX_STATS; i++)
            demon->perm_stat[i] = 11 + demon->level/4;
	demon->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	SET_BIT(demon->affected_by, AFF_CHARM);
	
	switch (demon_type)
	{
		default: break; return;
		case 1:
			demon->name = "lemure";
			demon->short_descr = "a lemure";
			demon->long_descr = "a lemure is here, bound by magic.\r\n";
			demon->size = size_lookup("small");
			break;
		case 2:
			demon->name = "nupperibo";
			demon->short_descr = "a nupperibo";
			demon->long_descr = "a nupperibo is here, bound by magic.\r\n";
			demon->size = size_lookup("small");
			break;
		case 3:
			demon->name = "vrock";
			demon->short_descr = "a vrock";
			demon->long_descr = "a vrock is here, bound by magic.\r\n";
			demon->size = size_lookup("medium");
			break;
		case 4:
			demon->name = "Erinyes";
			demon->short_descr = "an erinyes";
			demon->long_descr = "an erinyes is here, bound by magic.\r\n";
			demon->size = size_lookup("medium");
			break;
		case 5:
			demon->name = "Malebranche";
			demon->short_descr = "a malebranche";
			demon->long_descr = "a malebranche is here, bound by magic.\r\n";
			demon->size = size_lookup("large");
			break;
		case 6:
			demon->name = "Glabrezu";
			demon->short_descr = "a glabrezu";
			demon->long_descr = "a glabrezu is here, bound by powerful magic.\r\n";
			demon->size = size_lookup("huge");
			break;
		case 7:		
			demon->name = "Balor";
			demon->short_descr = "a balor";
			demon->long_descr = "a balor is here, bound by powerful magic.\r\n";
			demon->size = size_lookup("large");
			break;
	}
	
	SEND ("You summon a powerful demon to your aid!\r\n",ch);
	act ("$n summons a powerful demon!", ch, NULL, NULL, TO_ROOM);
	char_to_room(demon, ch->in_room);
	demon->leader = ch;
	demon->master = ch;
	//demon->start_following( ch );
	extract_obj (heart);	
	ch->charms++;
	return;
}

SPELL(spell_embalm)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( (obj->item_type != ITEM_CORPSE_NPC) 
	   && (obj->item_type != ITEM_CORPSE_PC)
	   && (obj->item_type != ITEM_FOOD)
	   && (obj->item_type != ITEM_TRASH) )
    {
		  SEND( "You cannot embalm this.\r\n", ch );
		  return;
    }

    obj->timer = 0;
    obj_from_char( obj );
    if ( obj->weight > 80 )
    	obj->weight = UMAX(20,number_range( obj->weight / 2, 2 * ( obj->weight / 3 )));
    obj_to_char( obj, ch );
    act("$p looks like it won't spoil anytime soon.",ch,obj,NULL,TO_ROOM);
    act("$p turns hard and green.",ch,obj,NULL,TO_CHAR);
    return;

}


SPELL (spell_strength)
{
	if (ch->perm_stat[STAT_STR] < 25)
	{
		ch->perm_stat[STAT_STR] += 1;
		SEND("You feel stronger!\r\n",ch);
	}
	else
	{
		SEND("You feel no different.\r\n",ch);
	}
	return;
}

SPELL (spell_intelligence)
{
	if (ch->perm_stat[STAT_INT] < 25)
	{
		ch->perm_stat[STAT_INT] += 1;
		SEND("You feel more intelligent!\r\n",ch);
	}
	else
	{
		SEND("You feel no different.\r\n",ch);
	}
	return;
}

SPELL (spell_wisdom)
{
	if (ch->perm_stat[STAT_WIS] < 25)
	{
		ch->perm_stat[STAT_WIS] += 1;
		SEND("You feel more wise!\r\n",ch);
	}
	else
	{
		SEND("You feel no different.\r\n",ch);
	}
	return;
}

SPELL (spell_dexterity)
{
	if (ch->perm_stat[STAT_DEX] < 25)
	{
		ch->perm_stat[STAT_DEX] += 1;
		SEND("You feel more agile!\r\n",ch);
	}
	else
	{
		SEND("You feel no different.\r\n",ch);
	}
	return;
}


SPELL (spell_constitution)
{
	if (ch->perm_stat[STAT_CON] < 25)
	{
		ch->perm_stat[STAT_CON] += 1;
		SEND("You feel healthier!\r\n",ch);
	}
	else
	{
		SEND("You feel no different.\r\n",ch);
	}
	return;
}

SPELL (spell_charisma)
{
	if (ch->perm_stat[STAT_CHA] < 25)
	{
		ch->perm_stat[STAT_CHA] += 1;
		SEND("You feel more charismatic!\r\n",ch);
	}
	else
	{
		SEND("You feel no different.\r\n",ch);
	}
	return;
}

SPELL(spell_disintegrate)
{
    CHAR_DATA *victim;
    //OBJ_DATA *obj;
    int dam;
    
    /*
	if(get_obj_carry(ch, obj->name, ch) == NULL)
	{
	    SEND("You must be holding the item to perform this task.\r\n",ch);
	    return;
	}
        if ((!IS_NULLSTR(obj->owner) && strcasecmp(obj->owner, ch->name)) || obj->level-2 > level)
        {
            act ("The psychic field on $p is too strong to bypass.", ch, obj, NULL, TO_CHAR);
            return;
        }
        act ("$p dissolves into dust.", ch, obj, NULL, TO_ALL);
        extract_obj(obj);
        return;
    }*/
    
    if (target == TARGET_CHAR)
    {
        victim = (CHAR_DATA *) vo;
        dam = dice(level, 7);
        if (saves_spell(level, victim, DAM_ACID))
			dam = dam / 2;		
        damage (ch, victim, dam, sn, DAM_ACID, TRUE);
        acid_effect(victim, level / 4, dam / 6, TARGET_CHAR);
        return;
    }
    else 
		SEND("It doesn't work that way.\r\n", ch);
    return;
}


SPELL (spell_blade_barrier)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	OBJ_DATA *blade;
	
	for ( blade = ch->carrying; blade; blade = blade->next_content )
    {
        if ( blade->item_type == ITEM_WEAPON )              
		{
			if ((blade->value[0]) == WEAPON_SHORT_SWORD || (blade->value[0] == WEAPON_LONG_SWORD) || (blade->value[0]) == WEAPON_DAGGER)			
				break;
		}
    }
	
	if (!blade || blade->item_type != ITEM_WEAPON)
	{
		SEND("You don't have a blade to use for this spell.\r\n",ch);
		return;
	}
	
	if (victim != ch)
	{
        SEND("You can't cast this spell on another.\r\n", ch);
		return;
	}	
     
	if (is_affected(ch, sn))
	{
		SEND("You're already surrounded by a magical blade barrier!\r\n",ch);
		return;
	}
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
	
	extract_obj(blade);
    SEND ("You are surrounded by a magical blade barrier!\r\n", victim);
    act ("$n is surrounded by a magical blade barrier!", victim, NULL, NULL, TO_NOTVICT);
	return;	
}


SPELL (spell_aid)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	int hp = 1;

    if ( is_affected( victim, gsn_aid ) )
    {
		if ( victim == ch )
			SEND("You are already aided.\r\n", ch);
		else
			act("$N is already aided.",ch,NULL,victim,TO_CHAR);

		return;
    }

    af.where	 = TO_AFFECTS;
    af.type	     = gsn_aid;
    af.level	 = level;
    af.duration	 = number_fuzzy(level/5);
    af.location	 = APPLY_HIT;
    af.modifier	 = number_range( victim->max_hit - 100, victim->max_hit + 200 )/ 20;
    af.bitvector = 0;    
    affect_to_char( victim, &af );
    victim->hit += (victim->hit / 20) + 1;

    SEND("You feel healthier than ever!\r\n", victim);
    if ( ch != victim )
	act("$N looks aided and ready for battle.",ch,NULL,victim,TO_CHAR);
}

SPELL (spell_globe_invulnerability)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND("You can only cast this spell on yourself.\r\n",ch);
		return;
	}
	
    if ( is_affected( victim, gsn_aid ) )
    {	
		SEND("You are already enveloped!\r\n", ch);	
		return;
    }

    af.where	 = TO_AFFECTS;
    af.type	     = gsn_globe_invulnerability;
    af.level	 = level;
    af.duration	 = number_fuzzy(level/6);
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = 0;    
    affect_to_char( victim, &af );    
    
	act( "$n is surrounded in a crackling translucent globe of energy!", victim, NULL, NULL, TO_ROOM );
	SEND( "You are surrounded by a crackling translucent globe of energy!\r\n", victim );	
}

SPELL (spell_gaseous_form)
{
	SEND ("Needs implementation!\r\n",ch);
	return;
}

SPELL (spell_ghoul_touch)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (ch->grank == G_BACK)
	{
		SEND ("You can't do that from the back ranks.\r\n",ch);
		return;
	}

    if (saves_spell (level, victim, DAM_NEGATIVE))
    {
        SEND ("Your skin crawls for a moment.\r\n", victim);
        return;
    }


    if (victim->level <= 2)
    {
        dam = ch->hit + 1;
    }
    else
    {
        gain_exp (victim, 0 - number_range (level, 3 * level), FALSE);
        dam = dice (1, level);		
        ch->hit += dam;
    }

    SEND ("You feel your life slipping away!\r\n", victim);
    SEND ("Your veins surge with renewed life! Wow....what a rush!\r\n", ch);
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);

	return;
}

SPELL (spell_forbiddance)
{
	if (!IS_SET(ch->in_room->area->area_flags, AREA_FORBIDDANCE))
	{
		SEND("Area has been protected from inter planar travel.\r\n",ch);
		SET_BIT(ch->in_room->area->area_flags, AREA_FORBIDDANCE);
		return;
	}
	else
	{
		SEND("This area is already protected from inter planar travel.\r\n",ch);
		return;
	}
	return;
}

SPELL (spell_explosive_runes)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	
	if (obj)
	{		
		if (IS_SET(obj->extra_flags, ITEM_FIRE_TRAP))
		{
			act ("$p is already trapped.", ch, obj, NULL, TO_CHAR);
			return;
		}

		// if (obj->item_type != ITEM_CONTAINER)
		// {
			// SEND("That's not a container!\r\n",ch);
			// return;
		// }
		
		SET_BIT(obj->extra_flags, ITEM_FIRE_TRAP);

		act ("Magical runes flare and glow across the surface of $p.", ch, obj, NULL, TO_ALL);		
		return;
	}
	else
	{
		SEND("Not a valid object.\r\n",ch);
		return;
	}
}

SPELL (spell_fire_storm)
{
	CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;	

    dam = dice (level / 2, 8);

    SEND("You unleash a fire storm upon your foes!\r\n", ch);
	
    for (vch = char_list; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room)
        {
            if (vch != ch && (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch)))
                damage (ch, vch, saves_spell (level, vch, DAM_FIRE)
                        ? dam / 2 : dam, sn, DAM_FIRE, TRUE);
            continue;
        }

        if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE (vch)
            && IS_AWAKE (vch))
            SEND ("Fire sparks flash in the sky.\r\n", vch);
    }

    return;
}

SPELL (spell_gust_of_wind)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 8);
    if (saves_spell (level, victim, DAM_AIR))
        dam /= 2;
	if (ch->in_room->area->curr_wind > 3)
		dam *= 2;
	damage (ch, victim, dam, sn, DAM_AIR, TRUE);
    return;
}

SPELL (spell_ice_storm)
{
	CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    dam = dice (level / 2, 8);

	if (ch->in_room->area->curr_precip == SNOW || ch->in_room->area->curr_precip == HAIL)
		dam *= 2;
		
    SEND("You unleash an ice storm upon your foes!\r\n", ch);
	
    for (vch = char_list; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room)
        {
            if (vch != ch && (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch)))
                damage (ch, vch, saves_spell (level, vch, DAM_COLD)
                        ? dam / 2 : dam, sn, DAM_COLD, TRUE);
            continue;
        }

        if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE (vch)
            && IS_AWAKE (vch))
            SEND ("A cold wind rushes past.\r\n", vch);
    }
	
    return;
}

SPELL (spell_meteor_swarm)
{
	CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
//	char buf[MAX_STRING_LENGTH];

    dam = dice (level * 2, 18);

    SEND("You call meteors down upon your enemies!\r\n", ch);	
    act ("$n calls meteors down up $s enemies!",
         ch, NULL, NULL, TO_ROOM);

    for (vch = char_list; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room == ch->in_room)
        {
            if (vch != ch)
			{
                int i = 0;
				for (i = 0; i < 2; i++)
				{
					damage (ch, vch, saves_spell (level, vch, DAM_FIRE)
                        ? dam / 2 : dam, sn, DAM_FIRE, TRUE);
					if (number_percent() < 50)
						i++;
				}
			}	
            continue;
        }

        if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE (vch)
            && IS_AWAKE (vch))
            SEND ("Red streaks form in the sky.\r\n", vch);
    }

    return;
}

SPELL (spell_nightmare)
{
	CHAR_DATA *victim;    
    AFFECT_DATA af;


    victim = (CHAR_DATA *) vo;

	if (IS_AWAKE(victim))
	{
		SEND ("You cannot cast this on a conscious target.\r\n",ch);
		return;
	}
	
    if (saves_spell (level, victim, DAM_MENTAL))
    {
        act ("$n appears momentarily disturbed.", victim, NULL, NULL, TO_ROOM);
        SEND ("Your dreams are momentarily disturbed.\r\n", victim);
        return;
    }

	af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.bitvector = 0;
	af.location = APPLY_NONE;
    af.modifier = 0;	
	
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
			
	affect_to_char (victim, &af);    
    SEND ("Your dreams suddenly turn to nightmares!\r\n", victim);
    act ("$n appears to be having a nightmare.", victim, NULL, NULL, TO_ROOM);
	return;
}

SPELL (spell_create_fire)
{
	OBJ_DATA *fire;
	char buf[MSL];
	
	fire = create_object (get_obj_index (OBJ_VNUM_FIRE), 0);
	obj_to_room(fire, ch->in_room);
	sprintf(buf, "You create %s.\r\n", fire->short_descr);
	SEND(buf, ch);
	return;
}

SPELL (spell_resist_scrying)
{

	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	
    if (is_affected (victim, sn))
    {
		if (victim == ch)
		{
			SEND ("You are already resistant to scrying.\r\n", ch);
			return;
		}	
		else
		{
			SEND ("They are already resistant to scrying.\r\n",ch);
			return;
		}
    }
	
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = level / 2;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = -1;
    af.location = APPLY_SAVES;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel resistant to prying eyes.\r\n", victim);    
    return;		
}

SPELL (spell_retribution)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
	{
		SEND("You can only cast that on yourself.\r\n",ch);
		return;
	}
	
	if ( is_affected( victim, gsn_retribution ) )
    {
		SEND("You are already doling out holy punishment.\r\n", ch);
		return;
    }

    af.where	 = TO_AFFECTS;
    af.type	     = gsn_retribution;
    af.level	 = level;
    af.duration	 = number_fuzzy(level/5);
    af.location	 = APPLY_HITROLL;
    af.modifier	 = number_range( 2, 4 );
    af.bitvector = 0;    
    affect_to_char( victim, &af );    

    SEND("You are ready to deal retribution!\r\n", victim);
    if ( ch != victim )
	act("$N looks ready for some retribution.\r\n",ch,NULL,victim,TO_CHAR);

	return;
}

SPELL (spell_astral_spell)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	
	if (victim != ch)
	{
		SEND("You can only travel to the astral plane alone.\r\n",ch);
		return;
	}
	
	if (is_affected(ch, gsn_dimensional_anchor))
	{
		SEND ("You are magically anchored to this plane.\r\n",ch);
		return;
	}
	
	if (IS_SET(ch->in_room->area->area_flags, AREA_FORBIDDANCE))
	{
		SEND ("Something prevents your astral travel.\r\n",ch);
		return;
	}
	
	SEND("You transport yourself to another plane.\r\n",ch);
	act("$N is suddenly drawn from the material plane.\r\n",ch, NULL,victim, TO_NOTVICT);
	char_from_room (ch);
	char_to_room (ch, get_room_index(ROOM_VNUM_ASTRAL));
	do_function (ch, &do_look, "auto");

	return;
	
}


SPELL (spell_animate_object)
{
	SEND("Spell awaiting implementation.\r\n",ch);
	return;
}

SPELL (spell_banshee_wail)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
	if (IS_AFFECTED(victim, gsn_deafness))
	{
		SEND("They seem to be unaffected!\r\n",ch);
		return;
	}
	
    dam = dice (level, 7);
    if (saves_spell (level, victim, DAM_SOUND))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_SOUND, TRUE);
    return;
}

SPELL (spell_copy)
{

	// OBJ_DATA *scroll; //pre-scribed scroll
	// OBJ_DATA *paper; //blank paper

	return;
}

SPELL (spell_chromatic_orb)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
//    OBJ_DATA *obj = NULL;
    int dam;
    int dam_type;    

    if (is_safe (ch, victim))
    {
		SEND("Not on that target.\r\n",ch);
		return;
    }
    
    dam_type = number_range (1,15);
    
	switch (dam_type)
	{
		default:
		case 1:
		case 2:
		case 3:
			dam = dice(level, 8);	
		
			if ( saves_spell(level, victim, DAM_LIGHTNING) )
				dam /= 2;
			
			damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
			return;
			
		case 4:
		case 5:
		case 6:
		case 7:
			dam = dice(level, 9);
			if ( saves_spell(level, victim, DAM_FIRE) )
				dam /= 2;
		
			damage( ch, victim, dam, sn, DAM_FIRE ,TRUE );
			return;
		
		case 8:
		case 9:
		case 10:
		case 11:
			dam = dice(level, 10);
			if ( saves_spell(level, victim, DAM_LIGHT) )
				dam /= 2;
		
			damage( ch, victim, dam, sn, DAM_LIGHT, TRUE);
			return;	
		
		case 12:
		case 13:
		case 14:
		case 15:
			dam = dice(level, 11);
			if ( saves_spell(level, victim, DAM_POISON) )
				dam /= 2;
		
			damage( ch, victim, dam, sn, DAM_POISON ,TRUE );
			return;		
	}
	return;
}

SPELL (spell_corpse_visage)
{
	return;
}

SPELL (spell_death_recall)
{
	OBJ_DATA *victim;
	char buf[MSL];
	
	victim = (OBJ_DATA *) vo;
	
	if (victim && victim->killer)
	{
		sprintf(buf, "A %s {bsays{x, 'I was killed by %s, please avenge me.'\r\n", victim->name, victim->killer);
		SEND(buf, ch);
	}
	else
	{
		SEND("You couldn't discern anything about the death.\r\n",ch);
	}
	return;
}

SPELL (spell_negative_plane_protection)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if (is_affected(victim, gsn_negative_plane_protection))
        return;

    act ("$n is surrounded in pink ectoplasmic energy.", victim, NULL, NULL, TO_ROOM);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;//gsn_negative_plane_protection;
    affect_to_char (victim, &af);
    SEND ("You are surrounded by pink ectoplasmic energy.\r\n", victim);
    return;
}

SPELL (spell_obscure_object)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	
	if (obj)
	{		
		if (IS_SET(obj->extra_flags, ITEM_NOLOCATE))
		{
			act ("$p is already obscured.", ch, obj, NULL, TO_CHAR);
			return;
		}		
		
		SET_BIT(obj->extra_flags, ITEM_NOLOCATE);
		act ("A translucent film appears all over $p.", ch, obj, NULL, TO_ALL);		
		return;
	}
	else
	{
		SEND("Not a valid object.\r\n",ch);
		return;
	}
	return;
}

SPELL (spell_timestop)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;

    act("Everything seems to slow down as $n manipulates time!",ch,NULL,victim,TO_NOTVICT);
    act("$n stops time for everyone around but them!",ch,NULL,victim,TO_VICT);
    act("You stop time for everything around you!",ch,NULL,NULL,TO_CHAR);

	//Small chance of aging the character.
	if (!system_shock(ch,0))
		ch->age++;

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_CHARM))
            {
                vch->wait += number_range(3,6);
				vch->fighting = NULL;
            }
            else
            {
                vch->wait += number_range(5,10);
				vch->fighting = NULL;
            }
        }
        else
        {
            if (saves_spell (level - 2, vch, DAM_CHARM))
            {
                vch->wait += number_range(2,4);
				vch->fighting = NULL;
            }
            else
            {
                vch->wait += number_range(4,8);
				vch->fighting = NULL;
            }
        }
    }
	return;
}

SPELL (spell_planar_door)
{	
	SEND("Spell awaiting implementation.\r\n",ch);
	return;
}

SPELL (spell_free_action)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	
	if (is_affected(victim, gsn_web))	
		affect_strip (victim, gsn_web);
	if (is_affected(victim, gsn_slow))	
		affect_strip (victim, gsn_slow);
	if (is_affected(victim, gsn_paralyzation))	
		affect_strip (victim, gsn_paralyzation);
	if (is_affected(victim, gsn_entangle))	
		affect_strip (victim, gsn_entangle);	
	SEND ("You body feels free as a bird now.\r\n",victim);
	return;
}

SPELL (spell_cone_cold)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam;

    act("$n creates a $t!",ch,skill_table[sn].name,victim,TO_NOTVICT);
    act("$n showers you with a $t!",ch,skill_table[sn].name,victim,TO_VICT);
    act("You create a $t!",ch,skill_table[sn].name,NULL,TO_CHAR);

    dam = dice(level,9);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_COLD))
            {
                cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
            }
            else
            {
                cold_effect (vch, level, dam, TARGET_CHAR);
                damage (ch, vch, dam, sn, DAM_COLD, TRUE);
            }
        }
        else
        {
            if (saves_spell (level - 2, vch, DAM_COLD))
            {
                cold_effect (vch, level / 4, dam / 8, TARGET_CHAR);
                damage (ch, vch, dam / 4, sn, DAM_COLD, TRUE);
            }
            else
            {
                cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
                damage (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
            }
        }
    }
	return;
}

SPELL (spell_cone_shadow)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam;

    act("$n creates a $t!",ch,skill_table[sn].name,victim,TO_NOTVICT);
    act("$n showers you with a $t!",ch,skill_table[sn].name,victim,TO_VICT);
    act("You create a $t!",ch,skill_table[sn].name,NULL,TO_CHAR);

    dam = dice(level,9);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_NEGATIVE))
            {                
                damage (ch, vch, dam / 2, sn, DAM_NEGATIVE, TRUE);
            }
            else
            {             
                damage (ch, vch, dam, sn, DAM_NEGATIVE, TRUE);
            }
        }
        else
        {
            if (saves_spell (level - 2, vch, DAM_NEGATIVE))
            {
                damage (ch, vch, dam / 4, sn, DAM_NEGATIVE, TRUE);
            }
            else
            {             
                damage (ch, vch, dam / 2, sn, DAM_NEGATIVE, TRUE);
            }
        }
    }
	return;
}

SPELL (spell_cone_sound)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam;

    act("The air reverberates around you as $n creates a $t!",ch,skill_table[sn].name,victim,TO_NOTVICT);
    act("$n blasts you with a $t!",ch,skill_table[sn].name,victim,TO_VICT);
    act("The air reverberates around the area as you create a $t!",ch,skill_table[sn].name,NULL,TO_CHAR);

    dam = dice(level,9);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if (vch == victim)
        {                        /* full damage */
            if (saves_spell (level, vch, DAM_SOUND))
            {                
                damage (ch, vch, dam / 2, sn, DAM_SOUND, TRUE);
            }
            else
            {             
                damage (ch, vch, dam, sn, DAM_SOUND, TRUE);
            }
        }
        else
        {
            if (saves_spell (level - 2, vch, DAM_SOUND))
            {
                damage (ch, vch, dam / 4, sn, DAM_SOUND, TRUE);
            }
            else
            {             
                damage (ch, vch, dam / 2, sn, DAM_SOUND, TRUE);
            }
        }
    }
	return;
}

SPELL (spell_cone_silence)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
	AFFECT_DATA af;
    int dam;

    act("The air reverberates around you as $n creates a $t!",ch,skill_table[sn].name,victim,TO_NOTVICT);
    act("$n blasts you with a $t!",ch,skill_table[sn].name,victim,TO_VICT);
    act("The air reverberates around the area as you create a $t!",ch,skill_table[sn].name,NULL,TO_CHAR);

    dam = dice(level,4);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (is_safe_spell (ch, vch, TRUE)
            || (IS_NPC (vch) && IS_NPC (ch)
                && (ch->fighting != vch || vch->fighting != ch)))
            continue;

                         /* full damage */
		if ( saves_spell (level, victim, DAM_MENTAL))
		{                
			//damage (ch, vch, dam / 2, sn, DAM_SOUND, TRUE);
			{
				SEND("Your throat feels a bit scratchy for a second.\r\n", victim);
				act ("$n coughs briefly.", victim, NULL, NULL, TO_ROOM);
				return;
			}				
		}
		else
		{             
			//damage (ch, vch, dam, sn, DAM_SOUND, TRUE);
			af.where = TO_AFFECTS;
			af.type = sn;
			af.level = level;    
			af.location = -1;
			af.modifier = 0;
			af.bitvector = 0;
			af.duration = number_range(1,3);    
			affect_to_char (vch, &af);
				
			
			SEND ("Your vocal chords constrict upon themselves!\r\n", vch);
			act ("$n gets an awkward look on their face!", vch, NULL, NULL, TO_ROOM);
		}
    }
	return;
}	

SPELL (spell_pass_without_trace)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if (is_affected(victim, gsn_pass_without_trace))
        return;

    act ("$n's feet don't seem to touch the gound...", victim, NULL, NULL, TO_ROOM);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You begin passing without trace.\r\n", victim);
    return;
}

SPELL (spell_searing_light)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level / 2, 6);
    if (saves_spell (level, victim, DAM_LIGHT))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_LIGHT, TRUE);
    return;
}

SPELL (spell_sunburst)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 9);
    if (saves_spell (level, victim, DAM_LIGHT))
        dam /= 2;
	else
        spell_blindness (skill_lookup ("blindness"),
                         level / 2, ch, (void *) victim, TARGET_CHAR);	
    damage (ch, victim, dam, sn, DAM_LIGHT, TRUE);
    return;
}




/*
 * Psionic spells.
 */

SPELL (spell_absorb_disease)
{
    // CHAR_DATA *victim = (CHAR_DATA *) vo;
    // AFFECT_DATA af, *paf;

    // if ( !is_affected( victim, gsn_plague ) && !is_affected( victim, gsn_poison ) )
    // {
	// if (victim == ch)
    	    // SEND("You aren't ill.\r\n",victim);
      // else
          // act("$N doesn't appear to be ill.",ch,NULL,victim,TO_CHAR);
        // return;
    // }

    // if ( is_affected( victim, gsn_plague ) )
    // {
	// paf = affect_find(victim->affected,gsn_plague);

	// if (check_dispel(level,victim,gsn_plague))
        // {
	    // SEND("The disease is forced from your cells.\r\n",victim);
	    // act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
	    // if(ch != victim)
	    // {
	        // af.where     = TO_AFFECTS;
	        // af.type           = gsn_plague;
	        // af.level        = paf->level * 2;
			// af.duration  = paf->duration * 2;
	        // af.location  = APPLY_STR;
	        // af.modifier  = -4;
	        // af.bitvector = AFF_PLAGUE;
	        // affect_to_char(ch,&af);
		// act("You have absorbed $N's disease completely.",ch,NULL,victim,TO_CHAR);
		// act("$n's skin erupts in puss oozing boils and scabs.",ch,NULL,NULL,TO_ROOM);
	    // }
        // }
	// else
	    // SEND("Your attempt to remove the disease fails.\r\n", ch);
    // }

    // if ( is_affected( victim, gsn_poison ) )
    // {
	// paf = affect_find(victim->affected,gsn_poison);

    	// if (check_dispel(level,victim,gsn_poison))
    	// {
	    // if(ch != victim)
	    // {
	        // af.where     = TO_AFFECTS;
	        // af.type      = gsn_poison;
	        // af.level     = paf->level * 2;
	        // af.duration  = paf->duration * 2;
	        // af.location  = APPLY_STR;
	        // af.modifier  = -2;
	        // af.bitvector = AFF_POISON;
	        // affect_to_char(ch,&af);
		// act("You have absorbed $N's poison completely.",ch,NULL,victim,TO_CHAR);
		// act("$n's looks deathly ill all the sudden.",ch,NULL,NULL,TO_ROOM);
	    // }
            // SEND("The poison has been purged from your veins.\r\n",victim);
            // act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    	// }
	// else
	  // SEND("You can't seem to shake the poison.\r\n", ch);
    // }
}

SPELL (spell_adrenaline_control)
{
    AFFECT_DATA af;

    if (is_affected(ch,sn) || IS_AFFECTED(ch,AFF_BERSERK) 
    ||  is_affected(ch, gsn_frenzy ))
    {
	  SEND("Don't overstress yourself.\r\n",ch);
	  return;
    }

    if (is_affected(ch,gsn_calm))
    {
	  SEND("You feel too mellow for an adrenalin rush.\r\n",ch);
	  return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    ////af.focus = 1;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 7;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    // affect_to_char(ch,&af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);


    af.location  = APPLY_DAMROLL;
    // affect_to_char(ch,&af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);

    SEND("You feel adrenalin flowing through your veins!\r\n",ch);
    act("$n looks ready to fight!",ch,NULL,NULL,TO_ROOM);
}

SPELL (spell_agitation )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	0,
	 0,  0, 12, 16, 16,      18, 20, 22, 23, 24,
	24, 24, 25, 25, 26,      26, 26, 27, 29, 31,
	33, 33, 33, 33, 35,      35, 36, 36, 38, 38,
	38, 40, 40, 42, 42,      44, 46, 46, 46, 49,
	50, 50, 52, 52, 52,      55, 56, 58, 60, 65
    };
    int dam, loops;
    int i = sizeof( dam_each ) / sizeof( dam_each[0] ) - 1;
    level    = UMIN( level, i );
    level    = UMAX( 0, level );
    loops = number_range(1,2);
    for ( ; loops > 0; loops-- )
    {
    	dam = number_range( dam_each[level], dam_each[level] * 2 );
       	if ( saves_spell(level, victim, DAM_ENERGY ) )
      	    dam /= 2;
    	damage( ch, victim, dam, sn, DAM_ENERGY, TRUE);
    }
    return;
}

SPELL (spell_animate_shadow)
{
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    CHAR_DATA *elemental;
    MOB_INDEX_DATA *pMobIndex;
    int i;
   
    if ( IS_NPC(ch))
        return;

    if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
    {
		SEND("You cannot control more followers...\r\n", ch);
		return;
    }
  
    if ( saves_spell(level + 3, victim, DAM_CHARM) && ch != victim )
    {
		SEND("Your attempt fails.\r\n", ch);
		SEND("Your shadow remains still.\r\n", victim);
		return;
    }

    {
		pMobIndex = get_mob_index( MOB_VNUM_SHADOW ); 
		elemental = create_mobile(pMobIndex);
		elemental->max_hit = UMAX( 8000, ((3 * victim->max_hit) / 4 ));
			elemental->hit = elemental->max_hit;
		elemental->level = number_range((ch->level - 3),(ch->level - 7));
		//elemental->poof_timer = number_range(2 * ( ch->level / 3), ch->level);
			SET_BIT(elemental->form, FORM_INSTANT_DECAY);

		// if (!IS_NPC(ch))
				// ch->pcdata->pet_tracker++;

		  for (i = 0; i < 3; i++)
			elemental->armor[i]           = elemental->level * (-2);
		  elemental->armor[3]               = elemental->level * (-1);
	 
		  for (i = 0; i < 2; i++)
			elemental->damage[i]		  = elemental->level/(number_range(4,7));
		  elemental->damage[2]		  = elemental->level*(2/3);

		  elemental->hitroll 		= number_fuzzy(elemental->level/2);
		  elemental->damroll		= number_fuzzy(elemental->level/2);

		  for (i = 0; i < MAX_STATS; i++)
			elemental->perm_stat[i] = 11 + elemental->level/4;
		char_to_room(elemental, ch->in_room);
		SET_BIT(elemental->affected_by, AFF_CHARM);
		SET_BIT(elemental->affected_by, AFF_PASS_DOOR);
		elemental->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
		//elemental->start_following( ch );
		elemental->leader = ch;
		sprintf(buf, "%s has animated %s's shadow!\r\n", ch->name, victim->name);
		SEND( "The shadow comes alive.\r\n", ch );
		act( buf, ch, NULL, NULL, TO_ROOM );
		ch->charms++;
    }
    return;
}

SPELL (spell_appraise)
{
    // CHAR_DATA *victim;
    // int diff;
    // char *msg;


    // if ( ch == ( victim = get_char_world( ch, target_name ) ) )
    // {
	// SEND("Why do you want to hurt yourself?\r\n", ch);
	// return;
    // }

    // if ( victim == NULL
	// || victim->in_room->area != ch->in_room->area
	// || !can_see(ch, victim) )
    // {
	// SEND("No luck.  Are you sure they're around?\r\n", ch);
	// return;
    // }

    // diff = victim->level - ch->level;

         // if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    // else if ( diff <=  -5 ) msg = "$N is no match for you.";
    // else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    // else if ( diff <=   1 ) msg = "The perfect match!";
    // else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    // else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    // else                    msg = "Death will thank you for your gift.";

    // act( msg, ch, NULL, victim, TO_CHAR );
    // return;

}

SPELL (spell_aura_sight)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) && IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        SEND("You can already see auras!\r\n", ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    ////af.focus = 1;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    if ( !IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
        af.bitvector = AFF_DETECT_EVIL;
        affect_to_char( victim, &af);
    }

    if ( !IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        af.bitvector = AFF_DETECT_GOOD;
        affect_to_char( victim, &af);
    }
    SEND( "The world around you begins to glow.\r\n", victim );
    return;
}

SPELL (spell_aversion)
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    int attempt;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    EXIT_DATA *pexit;
    int door;

    if ( victim == ch )
    {
	SEND("But you're not so bad.\r\n", ch);
	return;
    }
	
	if(IS_NPC(victim) && 
	   ( victim->spec_fun == spec_lookup( "spec_executioner")
			|| victim->spec_fun == spec_lookup( "spec_clan_guardian")
                        || victim->spec_fun == spec_lookup( "spec_restring")
			|| victim->spec_fun == spec_lookup( "spec_questmaster")
	   ))
	   return;
	   
    if ( !saves_spell(level, victim, DAM_MENTAL) && is_affected(victim, gsn_contact))
    {
	was_in = ch->in_room;
    	for ( attempt = 0; attempt < 6; attempt++ )
    	{

	    door = number_door( );
	    if ( ( pexit = was_in->exit[door] ) == 0
		||   pexit->u1.to_room == NULL
		||   IS_SET(pexit->exit_info, EX_CLOSED)
		||   number_range(0,victim->daze) != 0
		|| ( IS_NPC(victim)
		&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
		    continue;

	    move_char( victim, door, FALSE );
	    if ( ( now_in = victim->in_room ) == was_in )
		    continue;
	}

	act("$N scowls at $n and leaves the fight.", ch, NULL, victim, TO_NOTVICT);
	act("$n disgusts you.  You leave.", ch, NULL, victim, TO_VICT);
	act("$N appears disgusted and walks out.", ch, NULL, victim, TO_CHAR);

	stop_fighting( victim, TRUE );
	return;

    }
    if ( !is_affected(victim, gsn_contact))
	SEND("You need to establish contact first.\r\n", ch);
    else
	SEND("You weren't offensive enough.\r\n", ch);
}

SPELL (spell_awe)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	SEND("Why invite paranoia?\r\n", ch);
	return;
    }

    if ( is_affected(victim, gsn_awe))
    {
		SEND("They are already in awe of you.\r\n", ch);
		return;
    }

    if ( !saves_spell(level, victim, DAM_MENTAL) 
	&& is_affected(victim, gsn_contact)
	&& victim->position == POS_FIGHTING )
    {
	stop_fighting(victim, FALSE);
	stop_fighting(ch, FALSE);

	    af.where     = TO_AFFECTS;
	    af.type      = sn;
		af.level     = level;
	    af.duration  = 1;
	    af.modifier  = 0;
	    af.location  = APPLY_NONE;
	    af.bitvector = 0;
	    affect_to_char( victim, &af);

	    act("$n stops fighting $N.", ch, NULL, victim, TO_NOTVICT);
	    act("You're too scared to keep fighting $n.", ch, NULL, victim, TO_VICT);
	    act("$N stops fighting you.", ch, NULL, victim, TO_CHAR);

      if ( IS_NPC(victim) || IS_SET(victim->act, ACT_AGGRESSIVE ) )
	  REMOVE_BIT(victim->act, ACT_AGGRESSIVE);
      if ( victim->hate != NULL )
      {
          free_string( victim->hate );
          victim->hate = NULL;
      }
      return;
    }
    if ( !is_affected(victim, gsn_contact))
	SEND("You need to establish contact first.\r\n", ch);
    else
    	SEND("Your attempt to stop the fight fails.\r\n", ch);
}

SPELL (spell_ballistic_attack)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 12 );
    if ( saves_spell(level, victim, DAM_BASH ) )
	dam /= 2;
    damage(ch,victim,dam,sn,DAM_BASH,TRUE );
    return;
}

SPELL (spell_banishment)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    AFFECT_DATA af;

  
    if ( ch == victim )
    {
		SEND("You wish you were never even born.\r\n", ch );
		return;
    }
    if ( victim->master != NULL && victim->master == ch )
    {
		SEND( "Your powers are not great enough for that.\r\n", ch );
		return;
    }
    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
   // ||  IS_SET(victim->in_room->room_flags, ROOM_NOGATE)
   // || ( victim != ch && victim->isImmuneToResist(RES_SUMMON))
    ||   (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE ))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell(level - 5, victim,DAM_OTHER))))
    {
		SEND( "You failed.\r\n", ch );
		return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( victim, &af);

    pRoomIndex = get_room_index(3);

    if (victim != ch)
	SEND("You have been banished!\r\n",victim);

    act( "$n has been taken away by spirits!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    
    char_to_room( victim, pRoomIndex );
    act( "$n joins you in this realm.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

SPELL (spell_biofeedback)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
		if (victim == ch)
		  SEND("You are already controlling your body's bloodflow.\r\n",ch);
		else
		  act("$N is already controlling their own bloodflow.",ch,NULL,victim,TO_CHAR);
		return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = level;//getSancLevel(ch,level);
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af);
    act( "$n controls their body's bloodflow.", victim, NULL, NULL, TO_ROOM );
    SEND( "You feel your veins respond to your mind.\r\n", victim );
    return;
}

SPELL (spell_body_weaponry)
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	SEND("Your hands are already rigid.\r\n",ch);
	return;
    }
    af.where     = TO_AFFECTS;
    //af.focus = 1;
    af.type      = sn;
    af.level     = level;
    af.duration  = (3 * ((level / 10) + 1));
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char (ch, &af);
    SEND( "Your hands are now ready for combat.\r\n", ch );
    return;
}

SPELL (spell_body_equilibrium)
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	SEND("Your density is already adjusted.\r\n",ch);
	return;
    }
    af.where     = TO_AFFECTS;
    //af.focus = 1;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char ( ch, &af);
    SEND( "You feel lighter than water.\r\n", ch );
    return;
}

SPELL (spell_chemical_simulation)
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	SEND("Your hands are already affected.\r\n",ch);
	return;
    }
    af.where     = TO_AFFECTS;
    //af.focus = 1;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char ( ch, &af);
    SEND( "Your hands become chemically enhanced.\r\n", ch );
    return;
}

SPELL (spell_clairaudience)
{
    CHAR_DATA *victim;

    if ( ch == ( victim = get_char_world( ch, target_name ) ) )
    {
	SEND("What's the point?\r\n", ch);
	return;
    }

    if ( victim == NULL 
	|| IS_NPC(victim)
	|| is_safe(ch, victim)
	|| ch->level - victim->level > 8
	|| saves_spell(level + 4, victim, DAM_MENTAL)
 	||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
 	||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY))
    {
	SEND("You can't establish a link.\r\n", ch);
	return;
    }

    SEND("You successfully landed an incomplete spell :)\r\n", ch);
    return;
    
}
SPELL (spell_clairvoyance)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *original;

   if ( (victim = get_char_world(ch, target_name)) == NULL)
   {
	SEND("They don't seem to be around.\r\n", ch);
	return;
   } 

   if ( ch ==  victim )
   {
	SEND("Do a look.  It's cheaper.\r\n", ch);
	return;
   }

   if ( !is_affected(victim, gsn_contact))
   {
	SEND("You must establish contact first.\r\n", ch);
	return;
   }

   if ( victim == NULL ||
	is_safe(ch,victim) ||
	ch->level - victim->level > 8 ||
	saves_spell(level + 4, victim, DAM_MENTAL ) 
	|| IS_AFFECTED( victim, AFF_BLIND ))
   {
	SEND("You failed.\r\n", ch);
	return;
   }

   original = ch->in_room;
   char_from_room(ch);
   char_to_room(ch, victim->in_room);
   do_look(ch, "auto");
   char_from_room(ch);
   char_to_room(ch, original);
   SEND("You feel watched.\r\n", victim);
   return;
}

SPELL (spell_clear_thoughts)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;

    spell_cancellation(sn, level, ch, victim, target/*,isObj*/);
}

SPELL (spell_fighting_mind )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	  SEND( "You already understand battle tactics.\r\n",
		       victim );
	else
	  act( "$N already understands battle tactics.",
	      ch, NULL, victim, TO_CHAR );
	return;
    }

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level + 3;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = level / 6;
    af.bitvector = 0;
//    af.focus     = 1;
    affect_to_char( victim, &af);

    af.location	 = APPLY_AC;
    af.modifier	 = - level - 5;
    affect_to_char( victim, &af);

    if ( victim != ch )
        SEND( "OK.\r\n", ch );
    SEND( "You gain a keen understanding of battle tactics.\r\n",
		 victim );
    return;
}

SPELL (spell_contact)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	SEND("Your are always in contact with yourself.\r\n", ch);
	return;
    }

    if ( victim == ch && ch->fighting != NULL )
	victim = ch->fighting;

    if ( is_affected(victim, gsn_contact) )
    {
	act("$N has already been touched.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if ( is_safe(ch, victim) || saves_spell(level, victim, DAM_MENTAL) )
	{
	SEND("Not this target.\r\n", ch);
	return;
	}

    if ( is_affected(victim, gsn_mental_barrier ) )
    {
	SEND("You've run into a mental barrier.\r\n", ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af);
    SEND( "You have made contact.\r\n", ch );
    return;
}

SPELL (spell_control_flame )
{
	CHAR_DATA *victim       = (CHAR_DATA *) vo;
	OBJ_DATA *obj;
	int dam;

    if ( (obj = get_eq_char( ch, WEAR_LIGHT )) == NULL )
    {
	SEND( "You must be carrying a light source.\r\n", ch );
	return;
    }

    dam = dice(obj->level, 10);
    if ( saves_spell(level, victim, DAM_FIRE ) )
        dam /= 2;

    damage( ch, victim, dam, sn,DAM_FIRE,TRUE/*, isObj*/ );
    return;
}

SPELL (spell_daydream)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (ch == victim)
    {
	SEND("But what if you never woke up?", ch);
	return;
    }

    if (is_affected(victim,gsn_calm))
    {
	  SEND("You can't distract the calm.\r\n",ch);
	  return;
    }

    if (saves_spell(level, victim, DAM_MENTAL) || is_affected(victim, sn))
    {
	act("$e remains focused.", ch, NULL, victim, TO_CHAR);
	return;
    } 

    act( "$N's mind starts to wander.",ch,NULL,victim,TO_CHAR);

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 7;
    af.modifier  = 0 - level / 10;
    af.bitvector = AFF_BERSERK;

    af.location  = APPLY_HITROLL;
    //affect_to_char(victim,&af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char ( ch, &af);

    af.location  = APPLY_DAMROLL;
    //affect_to_char(victim,&af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char ( ch, &af);

    SEND( "Your mind starts to wander... .... ....\r\n",victim);
}

SPELL (spell_death_field )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dmg = 0, total = 0, damageTotalAllowed = 0;

    if ( !IS_EVIL( ch ) && !IS_IMMORTAL( ch ))
    {
	SEND( "You are not evil enough to do that!\r\n", ch);
	return;
    }

    SEND( "A black haze emanates from you!\r\n", ch );
    act ( "A black haze emanates from $n!", ch, NULL, ch, TO_ROOM );

    damageTotalAllowed = ch->hit * (( get_curr_stat(ch, STAT_INT) / 2 ) + ( ch->level / 10 ));

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_same_group(ch, vch) || (vch == ch) || is_safe_spell(ch,vch,TRUE))
	     continue;

	if ( !saves_spell(level-2, vch, DAM_NEGATIVE ) && victim->level < ch->level + 2)
        {
	    if( vch->spec_fun == spec_lookup( "spec_clan_guardian" ) || vch->spec_fun == spec_lookup( "spec_executioner" )) 
	    {
		dmg = dice(ch->level, 15);
		
		//check for damage saves
		if ( !saves_spell(level-2, vch, DAM_NEGATIVE ))
		    dmg /= 2;
		damage( ch,vch,dmg,sn,DAM_NEGATIVE,TRUE);
		total += dmg;
	    }
	    else
	    {
		dmg = number_range(vch->hit/2, 4*vch->hit/5);

		if (total > damageTotalAllowed)
		{
		    dmg = dice(ch->level,7);	
		
		    //check for damage saves
		    if ( !saves_spell(level-2, vch, DAM_NEGATIVE ))
		        dmg /= 2;
	    	
		    damage(ch,vch,0,sn,DAM_NEGATIVE,FALSE/*, isObj*/ );
		    total += dmg;
		    continue;
		}

		if ( IS_AFFECTED(vch, AFF_SANCTUARY ))
		    vch->hit -= ( 2 * dmg ) / 3;
		else
		    vch->hit -= dmg;
				
	    	SEND( "{dThe haze envelops you!{x\r\n", vch );
	    	act( "{dThe haze envelops $N!{x", ch, NULL, vch, TO_NOTVICT );
	    	act( "{dThe haze envelops $N!{x", ch, NULL, vch, TO_CHAR );
	    	damage(ch,vch,0,sn,DAM_NEGATIVE,FALSE/*, isObj*/ );
		total += dmg;
	    }
        }
	else
	{
		dmg = dice(ch->level,12);

	    //check for damage saves
	    if ( !saves_spell(level-2, vch, DAM_NEGATIVE ))
	        dmg /= 2;

	    damage(ch,vch,dmg,sn,DAM_NEGATIVE,TRUE/*, isObj*/ );
	    total += dmg;
	}
    }
}

SPELL (spell_detonate )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level, 14);
    if ( saves_spell(level, victim, DAM_ENERGY ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_ENERGY, TRUE/*, isObj*/  );
    return;
}
/*
SPELL (spell_displacement)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if (is_affected(ch, gsn_dream))
    {
	SEND("Physical travel isn't possible in your dreams.\r\n", ch);
	return;
    }

    if ( victim->in_room == NULL
    || ( victim != ch && (victim->isImmuneToResist(RES_SUMMON) && !IS_AFFECTED(ch, AFF_CHARM)))
    || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL )
    || IS_SET(victim->in_room->room_flags, ROOM_NOGATE)
    || ( victim != ch && ( saves_spell(ch, sn, isObj,level - 5, victim,DAM_OTHER ) ) )
    || ( victim->level <= 5 )
    || ( IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE ))
    || ( victim == ch && victim->fighting != NULL)
    || ( IS_AFFECTED( victim, AFF_CURSE) && number_percent() <= 50))
    {
        SEND( "You failed.\r\n", ch );
        return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
    {
    	act( "You have physically displaced $N.", ch, NULL, victim, TO_CHAR );
    	act( "$n has physically displaced $N.", ch, NULL, victim, TO_NOTVICT );
    }
    SEND("You have been displaced to a new location!\r\n",victim);

    char_from_room( victim );
    
    char_to_room( victim, pRoomIndex );

    // If you teleport a sleeping person, extra penalty for you!
    if (victim->position < POS_SLEEPING)
    {
        ch->pcdata->quit_counter = 10;
    }
        
    if( IS_MOUNT_ANY( victim ) )
    {
        clear_mount(victim, TRUE);
    }

    act( "$N vanishes before your very eyes.", ch, NULL, victim, TO_ROOM );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}
*/

SPELL (spell_domination )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	SEND( "Dominate yourself?  You're weird.\r\n", ch );
	return;
    }

    if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}

    if (   IS_AFFECTED( victim, AFF_CHARM )
	|| IS_AFFECTED( ch,     AFF_CHARM )
	|| level < victim->level
//        || victim->isImmuneToResist(RES_CHARM)
	|| IS_SET(victim->act, ACT_AGGRESSIVE )
	|| saves_spell(level, victim, DAM_CHARM ) )
        return;

    if ( victim->hate != NULL )
    {
	free_string( victim->hate );
	victim->hate = NULL;
    }

   if ( !IS_NPC(ch))
	ch->charms++;

//    if ( victim->master )
		victim->master = ch;
	//    victim->stop_following();

    //victim->start_following( ch );
    victim->leader = ch;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    if ( IS_NPC(victim))
	af.duration  = UMIN(number_fuzzy( level / 2 ), 15);
    else
	af.duration  = number_range( 2, 3 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af);

    act( "Your will dominates $N!", ch, NULL, victim, TO_CHAR );
    act( "Your will is dominated by $n!", ch, NULL, victim, TO_VICT );
    return;
}

SPELL (spell_ectoplasmic_form)
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_PASS_DOOR) )
    {
	SEND("You are already out of phase.\r\n",ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    // affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
    act( "$n's form becomes less substantial.", ch, NULL, NULL, TO_ROOM );
    SEND( "You become less substantial.\r\n", ch );
    return;
}

SPELL (spell_ego_whip )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell(level, victim, DAM_MENTAL ) )
        return;

    af.type	 = sn;
    af.level     = level;
    af.duration	 = level;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = -2;
    af.bitvector = 0;
    affect_to_char( victim, &af);

    af.location	 = APPLY_SAVING_SPELL;
    af.modifier	 = 2;
    affect_to_char( victim, &af);

    af.location	 = APPLY_AC;
    af.modifier	 = level / 2;
    affect_to_char( victim, &af);

    act( "You ridicule $N about $S childhood.", ch, NULL, victim, TO_CHAR    );
    SEND( "Your ego takes a beating.\r\n", victim );
    act( "$N's ego is crushed by $n!",          ch, NULL, victim, TO_NOTVICT );

    return;
}

SPELL (spell_ejection)
{
    bool found;
    found = FALSE;

    if (check_dispel(level,ch,gsn_contact))
        found = TRUE;
 
    if (found)
        SEND("You've broken the mental link.\r\n",ch);
    else
        SEND("Your ejection does nothing.\r\n",ch);
}

SPELL (spell_energy_containment )
{
    AFFECT_DATA af;

    if (is_affected(ch, gsn_energy_containment) /* || affect_find(ch->affected,gsn_bless)*/)
    {
	SEND( "You are already fending off energies.\r\n",ch);
        return;
    }

    af.type	 = sn;
    af.level     = level;
    //af.focus = 1;
    af.duration	 = level / 2 + 7;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier	 = -level / 7;
    af.bitvector = 0;
    // affect_to_char(ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);

    SEND( "You can fend off some forms of energy.\r\n", ch );
    return;
}

SPELL (spell_enhanced_strength)
{
    AFFECT_DATA af;

    if (is_affected(ch, sn) || is_affected(ch,gsn_giant_strength))
    {
	SEND("You are already as strong as you can get!\r\n",ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level	= level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    // affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
    SEND( "Your strength surges!\r\n", ch );
    act("$n's strength surges.",ch,NULL,NULL,TO_ROOM);
    return;
}

SPELL (spell_expansion)
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) || is_affected( ch, gsn_reduction))
    {
	SEND("You have already modified your cell size.\r\n",ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level	= level;
    af.duration  = level / 3;
    af.location  = APPLY_SIZE;
    af.modifier  = 1;
    af.bitvector = 0;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
    SEND( "Your cells expand making you larger!\r\n", ch );
    act("$n grows before your eyes.",ch,NULL,NULL,TO_ROOM);
    return;
}

SPELL (spell_flesh_armor )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        SEND("Your flesh is already tough as armor.\r\n",ch);
        return;
    }
    
    af.type	 = sn;
    // af.focus     = 1;
    af.level     = level;
    af.duration	 = level;
    af.location	 = APPLY_AC;
    af.modifier	 = -30;
    af.bitvector = 0;
    affect_to_char( victim, &af);

    SEND( "Your flesh toughens.\r\n", victim );
    act( "$N's flesh toughens.", ch, NULL, victim, TO_NOTVICT);
    return;
}

SPELL (spell_heightened_senses)
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_DETECT_HIDDEN) && IS_AFFECTED(ch, AFF_DETECT_INVIS))
    {
	SEND("Your sense for detail is already acute.\r\n",ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus 	 = 1;
    af.level	 = level;
    af.duration  = level;

  if ( !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
  {
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_HIDDEN;
    // affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
  }
  if ( !IS_AFFECTED(ch, AFF_DETECT_INVIS))
  {
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    // affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
  }
  if ( !IS_AFFECTED(ch, AFF_INFRARED))
  {
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_INFRARED;
    // affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char (ch, &af);
  }
    SEND( "You see more details in the world around you.\r\n", ch );
    return;
}

SPELL (spell_immovability )
{
    AFFECT_DATA af;

    /*if ( is_affected(ch, gsn_immovability) )
    {
	SEND("You are already anchored to the ground.\r\n", ch);
	return;
    }*/

    if ( ch->in_room->sector_type == SECT_AIR )
    {
	SEND("You'd fall out of the air!\r\n", ch);
	return;
    }

    if ( ch->in_room->sector_type == SECT_VOID )
    {
        SEND("There is nothing to anchor yourself to.\r\n",ch);
        return;
    }

    if ( ch->in_room->sector_type == SECT_LAVA )
    {
        SEND("You can't possibly anchor yourself there.\r\n",ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level     = level;
    af.duration  = 1;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    // affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
    af.where     = TO_AFFECTS;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_AC;
    af.modifier  = -100;
    af.bitvector = 0;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
    SEND( "Your feet become heavier than lead.\r\n", ch );
    act("$n's feet appear to start sinking slightly", ch, NULL, NULL, TO_ROOM);
    return;
}
    
SPELL (spell_inertial_barrier )
{
    AFFECT_DATA af;

	if ( IS_AFFECTED( ch, AFF_PROTECT_EVIL ) 
	  || IS_AFFECTED( ch, AFF_PROTECT_GOOD )
	  // || IS_AFFECTED( ch, AFF_PROTECT_NEUTRAL) 
	  || affect_find(ch->affected,gsn_inertial_barrier))
	{
	    SEND("No barrier can be formed around you.\r\n", ch);	
	    return;
	}

	SEND( "An inertial barrier forms around you.\r\n", ch );
	af.where     = TO_AFFECTS;
        //af.focus = 1;
	af.type	     = sn;
	af.level     = level;
	af.duration  = 24;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = 0;

	//affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
      return;
}

SPELL (spell_intellect_fortress )
{
    CHAR_DATA  *gch, *gch_next;
    AFFECT_DATA af;

    for ( gch = ch->in_room->people; gch; gch = gch_next)
    {
	gch_next = gch->next_in_room;

	if ( !is_same_group( gch, ch ) || is_affected( gch, sn ) )
	    continue;

	SEND( "A virtual fortress forms around you.\r\n", gch );
	act( "A virtual fortress forms around $N.", gch, NULL, gch, TO_ROOM );

	af.type	     = sn;
        af.level     = level;
        //af.focus = 1;
	af.duration  = 24;
	af.location  = APPLY_AC;
	af.modifier  = -15;
	af.bitvector = 0;
	// affect_to_char( gch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(gch, &af);
	
	af.location  = APPLY_SAVING_SPELL;
	af.modifier	 = -2;
	//affect_to_char( gch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(gch, &af);
    }
    return;
}

SPELL (spell_know_location)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "You seem to be somewhere in %s.\r\n", ch->in_room->area->name);
    SEND(buf, ch);
}

SPELL (spell_lend_health )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        hpch, manacost;

    if ( ch == victim )
    {
	SEND( "Lend health to yourself?  What a weirdo.\r\n", ch );
	return;
    }
    hpch = UMIN( 50, victim->max_hit - victim->hit );
    if ( hpch == 0 )
    {
	act( "Nice thought, but $N doesn't need healing.", ch, NULL,
	    victim, TO_CHAR );
	return;
    }
    if ( ch->hit-hpch < 50 )
    {
	SEND( "You aren't healthy enough yourself!\r\n", ch );
	return;
    }
    victim->hit += hpch;
    ch->hit     -= hpch;
    manacost = UMIN(0, (hpch / 2));
    ch->mana -= manacost;
    update_pos( victim );
    update_pos( ch );

    act( "You lend some of your health to $N.", ch, NULL, victim, TO_CHAR );
    act( "$n lends you some of $s health.",     ch, NULL, victim, TO_VICT );

    return;
}

SPELL (spell_levitate)
{
    AFFECT_DATA af;
    if ( IS_AFFECTED(ch, AFF_FLYING) )
    {
	SEND("You are already levitating.\r\n",ch);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char(ch, &af);
    SEND( "You begin to levitate.\r\n", ch );
    act( "$n begins to levitate.", ch, NULL, NULL, TO_ROOM );
    return;
}

SPELL (spell_mental_barrier )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn )) //|| victim->isImmuneToResist(RES_MENTAL))
    {
	SEND( "Your mind is already shielded from harm.\r\n",ch);
        return;
    }

    af.location  = TO_RESIST;
    af.type	 = sn;
    af.level     = level;
    //af.focus = 1;
    af.duration	 = 24;
    af.location	 = APPLY_NONE;
    af.modifier	 = 25;
    af.bitvector = RES_MENTAL;
    affect_to_char( victim, &af);

    SEND( "Your mental resistance increases.\r\n",victim );
    return;
}

SPELL (spell_metabolic_reconfiguration)
{
    if ( !IS_GOOD( ch ) && !IS_IMMORTAL( ch ))
    {
	SEND( "You are too evil to accomplish this.\r\n",ch);
	return;
    }
    ch->hit = UMIN( ch->hit + number_range( level, level + 75 ), ch->max_hit);
    update_pos( ch );
    SEND( "You will your body to heal its wounds.\r\n", ch );
    act( "$n's wounds heal before your eyes.",ch,NULL,NULL,TO_ROOM);
    return;
}

SPELL (spell_mind_over_body )
{
    if ( IS_NPC(ch))
	return;

    if (ch->pcdata->condition[COND_FULL] >= 100)
    {
        SEND("You do not feel the need for nourishment.\r\n", ch);
        return;
    }
    ch->pcdata->condition[COND_HUNGER] += 15;
    ch->pcdata->condition[COND_THIRST] += 15;
    ch->pcdata->condition[COND_FULL] += 15;
    SEND("You feel nourished.\r\n", ch);
    return;
}

SPELL (spell_mind_thrust )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    if ( victim == ch )
    {
	SEND("Cast this on others.\r\n", ch);
	return;
    }

    // if ( !saves_spell(ch, sn, /*isObj,*/level, victim, DAM_MENTAL) && !is_affected(victim, gsn_contact) )
	if (saves_spell (level, victim, DAM_MENTAL) && !is_affected(victim, gsn_contact))
    {
    	af.where     = TO_AFFECTS;
    	af.type      = gsn_contact;
    	af.level     = level;
    	af.duration  = level / 2;
    	af.location  = APPLY_NONE;
    	af.modifier  = 0;
    	af.bitvector = 0;
    	affect_to_char( victim, &af);
    	SEND( "You have made contact.\r\n", ch );
    }
    dam = dice( 1, 10 ) + (3 * (level / 2));
    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE  );
    return;

}

SPELL (spell_mindwipe )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (ch==victim)
    {
        SEND("And you would want to do this why...?\r\n",ch);
        return;
    }
    if (!is_affected(victim, gsn_contact))
    {	
	SEND("You must establish contact first.\r\n", ch);
	return;
    }

    // if (saves_spell(ch, 0, /*isObj,*/level, victim, DAM_MENTAL))
	if (saves_spell (level, victim, DAM_MENTAL))
    {
	if(ch!=victim)
	    act("You fail to wipe $S's mind", ch, NULL, victim, TO_CHAR);
	SEND("Your feel a small pain in your head.\r\n", victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 4;
    af.location  = APPLY_HITROLL;
    af.modifier  = number_fuzzy(0 - (level/5));
    af.location  = APPLY_INT;
    af.modifier  = 0 - ((level/11) + 1);
    af.bitvector = 0;
    affect_to_char( victim, &af);
    act("$n's mind is wiped clean!",victim,NULL,NULL,TO_ROOM);
    SEND( "Your ability to think decreases.\r\n",victim);
    return;
}

SPELL (spell_probe )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char buf[MAX_INPUT_LENGTH];
    int info;

    if (!is_affected(victim, gsn_contact) )
    {
	SEND("You must establish contact to probe.\r\n", ch);
	return;
    }

    if (is_affected(victim, gsn_probe) )
    {
	SEND("You've already learned as much as you can for now.\r\n", ch);
	return;
    }

    info = number_range(1,2);
    if ( IS_NPC(ch))
    {
	sprintf(buf, "%d", victim->hit);
	act("$N has $t hit points.", ch, buf, victim, TO_CHAR); 
    }
    else
    {
        switch(info)
        {
	    default:
		case '1':	sprintf(buf, "%d", victim->hit);
			act("$N has $t hit points.", ch, buf, victim, TO_CHAR); 
			break;

 	    // case '2':	sprintf(buf, "%d", 
			// //totalExpForLevel(victim,victim->level + 1,victim->pcdata->points) - victim->exp);
			// act("$N has $t experience to his next level", ch, buf, victim, TO_CHAR);
			// break;
        }
    }

    af.type	 = sn;
    af.duration	 = 10;
    af.level     	 = level;
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector 	 = 0;
    affect_to_char( victim, &af);

    act("$n seems curious about you.",ch, NULL, victim, TO_VICT );

    return;
}

SPELL (spell_psychic_drain )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (saves_spell (level, victim, DAM_NONE) || is_affected(victim, sn ))
    //if ( saves_spell(ch, 0, /*isObj,*/ level, victim, DAM_NONE ) || is_affected(victim, sn ))
    {
	SEND("You don't gain any energies.\r\n", ch);
        return;
    }

    af.type	 = sn;
    af.duration	 = level / 2;
    af.level     = level;
    af.location	 = APPLY_STR;
    af.modifier	 = -1 - ( level >= 10 ) - ( level >= 20 ) - ( level >= 30 );
    af.bitvector = 0;
    affect_to_char( victim, &af);

    SEND( "You feel drained.\r\n", victim );
    act( "$n appears drained of strength.", victim, NULL, NULL, TO_ROOM );
    return;
}

SPELL (spell_reduction)
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) || is_affected( ch, gsn_expansion))
    {
	SEND("You have already modified your cell size.\r\n",ch);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    //af.focus = 1;
    af.level	= level;
    af.duration  = level / 3;
    af.location  = APPLY_SIZE;
    af.modifier  = -1;
    af.bitvector = 0;
    //affect_to_char( ch, &af, (!isObj && isLearnedSpell(ch,sn) ? getSpellName(ch,sn) : 0x0));
	affect_to_char ( ch, &af);
    SEND( "Your cells contract making you smaller!\r\n", ch );
    act("$n shrinks before your eyes.",ch,NULL,NULL,TO_ROOM);
    return;
}

SPELL (spell_dimensional_mine)
{
	return;
}

SPELL (spell_antimagic_shell)
{
	return;
}

SPELL (spell_shocking_web)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam = 0;
	
	dam = dice (level, 6);
	
	if (victim == ch)
	{
        SEND("You probably don't want to do that to yourself.\r\n", ch);
		return;
	}
	if (IS_AFFECTED (victim, sn)
        || saves_spell (level, victim, DAM_LIGHTNING))
	{		
		dam /= 2;
		damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
		return;
	}
        
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
	af.location = APPLY_DEX;
    af.modifier = -5;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
	
	af.location = APPLY_MOVE;
    af.modifier = -(number_range(30,60));
    af.duration = 1 + level;
    affect_to_char (victim, &af);
	
	
    SEND ("Shocking magical webs begin restricting your movements!\r\n", victim);
    act ("Shocking magical webs spray forth, covering $n!", victim, NULL, NULL, TO_ROOM);	
	damage (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
	return;
}

SPELL (spell_earth_ward)
{	
	OBJ_DATA *obj;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	
	//AREA_DATA *area;		
	
	for (obj = object_list; obj != NULL; obj = obj->next)
    {
		// if (obj->in_room->area == ch->in_room->area)
		// {	
			if (is_trapped(obj))
			{
				remove_trap(obj);
				for (vch = char_list; vch != NULL; vch = vch_next)
				{
					vch_next = vch->next;
					
					if (vch->in_room == obj->in_room)
					{
						act ("$p glows a bright orange momentarily.", vch, obj, NULL, TO_ROOM);
						break;
					}
				}
			}
		// }
	}
	SEND("You ward the surrounding area.\r\n",ch);	
	return;
}

SPELL (spell_geyser)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj;
    int dam = 0;
	int mod = 7;
	
	if (IN_WATER_SECTOR(ch))
		mod += 2;
		
	for (obj = ch->in_room->contents; obj; obj = obj->next_content)
	{
		if (obj->item_type == ITEM_FOUNTAIN)
		{
			mod++;
			break;
		}
	}
	
    dam = dice (level, mod);
	
	dam /= 2;
	
	//Half fire half water damage.
	if (saves_spell (level, victim, DAM_DROWNING))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_DROWNING, TRUE);
	
	if (saves_spell (level, victim, DAM_FIRE))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_FIRE, TRUE);    
    return;
}

SPELL (spell_shocking_runes)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	
	if (obj)
	{		
		if (IS_SET(obj->extra2_flags, ITEM_SHOCK_TRAP))
		{
			act ("$p is already trapped.", ch, obj, NULL, TO_CHAR);
			return;
		}

		// if (obj->item_type != ITEM_CONTAINER)
		// {
			// SEND("That's not a container!\r\n",ch);
			// return;
		// }
		
		SET_BIT(obj->extra2_flags, ITEM_SHOCK_TRAP);

		act ("Magical runes spark and crackle across the surface of $p.", ch, obj, NULL, TO_ALL);		
		return;
	}
	else
	{
		SEND("Not a valid object.\r\n",ch);
		return;
	}

	return;
}

SPELL (spell_animate_weapon)
{
	CHAR_DATA 		*charmie;
	MOB_INDEX_DATA 	*mob;	
	OBJ_DATA		*weapon;
	AFFECT_DATA 	*paf = NULL;
	int i = 0;
	char buf [MSL];
	
	weapon = (OBJ_DATA *) vo;
	
	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	if (weapon->item_type != ITEM_WEAPON)
	{
		SEND("The item must be a weapon of some sort.\r\n",ch);
		return;
	}
	
	paf = affect_find (weapon->affected, gsn_curse);
	if (paf)
	{
		SEND("This weapon is cursed, it won't work.\r\n",ch);
		return;
	}
	
	mob = get_mob_index( 1 ); 
	charmie = create_mobile(mob);
	
	
	switch (weapon->value[0])
	{
		default:
			charmie->size = SIZE_MEDIUM;
			break;
			
		case WEAPON_DAGGER:
		case WEAPON_SHORT_SWORD:
			charmie->size = SIZE_SMALL;
			break;
		case WEAPON_AXE:
		case WEAPON_POLEARM:
		case WEAPON_SPEAR:
		case WEAPON_STAFF:
			charmie->size = SIZE_LARGE;
			break;
		case WEAPON_EXOTIC:
			charmie->size = SIZE_GIANT;
			break;			
	}
	
	charmie->short_descr = str_dup(weapon->short_descr);
	sprintf(buf, "%s.\r\n", weapon->short_descr);
	charmie->long_descr = str_dup(buf);
	charmie->name = str_dup(weapon->name);
	charmie->max_hit = number_range ( weapon->level * 20, weapon->level * 100 );
	charmie->hit = charmie->max_hit;
	charmie->max_mana = number_range ( weapon->level * 50, weapon->level * 200 );
	charmie->mana = charmie->max_mana;
	charmie->level = number_fuzzy(weapon->level);
	charmie->armor[AC_PIERCE] = 	150 - (weapon->level * 10);
    charmie->armor[AC_BASH] = 	150 - (weapon->level * 10);
    charmie->armor[AC_SLASH] = 	150 - (weapon->level * 10);
    charmie->armor[AC_EXOTIC] = 	150 - (weapon->level * 10);
	charmie->damroll =		number_fuzzy ( weapon->level );
	charmie->hitroll = 		number_fuzzy ( weapon->level / 2 );
	for (i = 0; i < 2; i++)
        charmie->damage[i] = 	charmie->level/(number_range(6,12));
    charmie->damage[2]	 = 	charmie->level/2;
	for (i = 0; i < MAX_STATS; i++)
            charmie->perm_stat[i] = 11 + charmie->level/4;
	charmie->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	SET_BIT(charmie->affected_by, AFF_CHARM);
		
	
	sprintf (buf, "You magically animate %s!\r\n", weapon->short_descr);
	SEND(buf, ch);
	act ("$n magically animates $p!", ch, weapon, NULL, TO_ROOM);
	char_to_room(charmie, ch->in_room);
	charmie->leader = ch;
	charmie->master = ch;
	ch->charms++;
	extract_obj(weapon);
	
	return;
}

SPELL (spell_hardstaff)
{	
	OBJ_DATA *obj;
	
	obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf = NULL;
	
	if (obj->item_type != ITEM_WEAPON)
	{
		SEND("Your target must be a staff.\r\n",ch);
		return;
	}
	
	if (obj->value[0] != WEAPON_STAFF)
	{
		SEND("Your target must be a staff.\r\n",ch);
		return;
	}
	
	if (IS_SET(obj->extra2_flags, ITEM_HARDSTAFF))
	{
		act ("$p is already magically protected.", ch, obj, NULL, TO_CHAR);
		return;
	}
      
    paf = affect_find (obj->affected, gsn_curse);
	if (paf)
	{
		SEND("This object is cursed.\r\n",ch);
		return;
	}
	   
    act ("$p glows a soft {Yyellow{x.", ch, obj, NULL, TO_ALL);
    SET_BIT(obj->extra2_flags, ITEM_HARDSTAFF);        	
	return;   
}

SPELL (spell_finger_death)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = victim->hit / 4 * 3; //75%
    if (saves_spell (level+2, victim, DAM_NEGATIVE))
        dam = dam * 2 / 3; //only do a quarter of their life.
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);    
	
	//They become evil from using it.	
	ch->alignment -= 10;
	if (ch->alignment < -1000)
		ch->alignment = -1000;
	
	return;
}

SPELL (spell_remove_paralysis)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (is_affected(victim, gsn_paralyzation))
	{
		if (saves_spell (level-5, victim, DAM_OTHER))
			affect_strip(victim, gsn_paralyzation);
		SEND("Ok.",ch);
	}
	else
		SEND("They aren't afflicted by paralyzation.\r\n",ch);
	
	return;
}


SPELL (spell_alarm )
{
	AREA_DATA * pArea = ch->in_room->area;
	char buf [MSL];
	
	if (!IS_SET(pArea->area_flags, AREA_ALARM))
	{
		SET_BIT (pArea->area_flags, AREA_ALARM);
		sprintf(buf, "You set a magical alarm in %s.\r\n",pArea->name);
		SEND(buf, ch);
		return;
	}
	else	
		SEND("This area already has an alarm set.\r\n",ch);
	
	return;
}

SPELL (spell_animate_dead)
{	
	OBJ_DATA *obj;	
	OBJ_DATA *obj_next;
	//char buf[MSL];
	
	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
    {
		obj_next = obj->next_content;
		
		if (!obj->item_type == ITEM_CORPSE_NPC)
			continue;
			
		
	}	
	

	return;
}

SPELL (spell_raise_dead )
{
	char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    int i;
    
    if ( IS_NPC(ch))
        return;

    obj = get_obj_here( ch, NULL, target_name );

    if ( obj == NULL )
    {
        SEND( "Raise what?\n\r", ch );
        return;
    }

    if ( (obj->item_type == ITEM_CORPSE_PC) && obj->contains != NULL)
    {
	SEND("You must empty that first.\n\r",ch);
	return;
    }
    
    if ( (obj->item_type != ITEM_CORPSE_NPC) && obj->item_type != ITEM_CORPSE_PC )
    {
	  SEND( "You can only raise the {Ddead.{x\n\r", ch );
	  return;
    }

    if( obj->level > (total_levels(ch) + 6) )
    {
        SEND( "You cannot raise such a powerful entity.\n\r", ch );
        return;
    }

    /*if( (!IS_NPC(ch)) && ch->pet_tracker >= charisma_check( ch ))
    {
        SEND("You cannot control more followers...\n\r", ch);
        return;
    }*/

    mob = create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );

    obj->material[0] = LOWER(obj->material[0]);
    free_string(mob->name);
    sprintf(buf,"zombie %s",obj->material);
    mob->name = str_dup(mob->name);

    free_string( mob->short_descr );
    sprintf(buf,"A zombie of %s", obj->material);
    mob->short_descr=str_dup(buf);

    strcat(buf," is here.\n\r");
    free_string( mob->description );
    mob->description=str_dup(buf);
    free_string( mob->long_descr );
    mob->long_descr=str_dup(buf);

    //mob->poof_timer 		= number_fuzzy( ( ch->level ));
    mob->level                  = obj->level;
    mob->max_hit                = 1 + (mob->level * (number_range(30,55)))+
				number_range( mob->level,( mob->level * 5));

    mob->hit                    = mob->max_hit;
    mob->max_mana               = 100 + dice(mob->level,10);
    mob->mana                   = mob->max_mana;
    for (i = 0; i < 3; i++)
        mob->armor[i]           = mob->level * (-4);
    mob->armor[3]               = mob->level * (-3);
 
    for (i = 0; i < 2; i++)
        mob->damage[i]		  = mob->level/(number_range(4,8));
    mob->damage[2]		  = mob->level/2;

    mob->hitroll 		= number_fuzzy(mob->level/3);
    mob->damroll		= number_fuzzy(mob->level/3);

    for (i = 0; i < MAX_STATS; i++)
        mob->perm_stat[i] = 9 + mob->level/4;
   
    char_to_room( mob, ch->in_room );
    act( "$t springs to life as a hideous zombie!", ch, obj->material, NULL, TO_ALL );

    // dump items carried

    if ( obj->item_type == ITEM_CORPSE_NPC && obj->contains != NULL)
    {
            OBJ_DATA *t_obj, *next_obj; 

            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content; 

                obj_from_obj(t_obj); 

                if (t_obj->item_type == ITEM_WEAPON)
                    obj_to_char(t_obj,mob);

                else
                    extract_obj(t_obj);
		    
     	   }
    }
    extract_obj(obj);

    //Yessssss, massssssster...
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->form, FORM_INSTANT_DECAY);
    if (!IS_NPC(ch))
		ch->pet_tracker++;
    mob->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    mob->master= ch;
    mob->leader = ch;
    //interpret(mob, "wear all");
    act( "$n says, 'Zug Zug!'\n\r", mob, NULL,NULL, TO_ALL );
    
	return;	
}

SPELL (spell_wither )
{
	if (ch->in_room->can_forage)
	{
		SEND ("The plants nearby wither up and die.\r\n",ch);		
		ch->in_room->can_forage = FALSE;
		return;
	}
	else	
		SEND ("You don't see any plants nearby to wither.\r\n",ch);	
	return;
}

SPELL (spell_blasphemy )
{
	SEND ("Tell Upro to code this shit!\r\n",ch);
	return;
}

SPELL (spell_deafness )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED (victim, gsn_deafness) || saves_spell (level, victim, DAM_SOUND))
        return;

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
	af.bitvector = 0;
    af.location = APPLY_HITROLL;
    af.modifier = -2;
    af.duration = 1 + level;    
    affect_to_char (victim, &af);
    SEND ("All the sounds of the world around are suddenly drowned out... by silence!\r\n", victim);
    act ("$n appears to be having difficulty hearing anything!", victim, NULL, NULL, TO_ROOM);
    return;
}

SPELL (spell_disenchant )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	
	
	if (IS_OBJ_STAT(obj, ITEM_INDESTRUCTABLE) || IS_OBJ_STAT(obj, ITEM_UNIQUE))
	{
		act ("$p cannot be disenchanted.", ch, obj, NULL, TO_CHAR);
		return;
	}
	
	act ("$p glows {Ybrightly{x, then fades...", ch, obj, NULL, TO_CHAR);
	act ("$p glows {Ybrightly{x, then fades...", ch, obj, NULL, TO_ROOM);
	obj->enchanted = TRUE;
	
	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{		
		paf_next = paf->next;
		
		if (!saves_dispel(total_levels(ch), paf->level, paf->duration))
			free_affect (paf);		
	}
	obj->affected = NULL;

	/* clear all flags */	
	obj->extra_flags = 0;
	obj->extra2_flags = 0;
	return;
	
}

SPELL (spell_command_undead )
{

	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe (ch, victim))
        return;

    if (victim == ch)
    {
        SEND ("You already command yourself!\r\n", ch);
        return;
    }

	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
    if (IS_AFFECTED (victim, AFF_CHARM)
        || IS_AFFECTED (ch, AFF_CHARM)
        || level < victim->level || IS_SET (victim->imm_flags, IMM_CHARM)
        || saves_spell (level, victim, DAM_CHARM))
        return;

	if ( victim->position == POS_SLEEPING && !IS_NPC(victim)) 
	{
		SEND( "You can not get your victim's attention.\r\n", ch );
		SEND( "Your slumbers are briefly troubled.\r\n", victim );
		return;
	}

    if (IS_SET (victim->in_room->room_flags, ROOM_LAW))
    {
        SEND
            ("The mayor does not allow charming in the city limits.\r\n", ch);
        return;
    }

	if (IS_NPC(victim) && IS_SET (victim->act2, ACT2_BOSS))
	{
		SEND ("Not happening.\r\n",ch);
		return;
	}
	
	if (IS_UNDEAD(victim))
	{
		if (victim->master)
			stop_follower (victim);
		add_follower (victim, ch);
		victim->leader = ch;
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = level;
		af.duration = number_fuzzy (level / 4);
		if (is_affected (ch, gsn_extension))
				af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
		af.location = 0;
		af.modifier = 0;
		af.bitvector = AFF_CHARM;
		affect_to_char (victim, &af);
		act ("Following $n would serve to greater goals.", ch, NULL, victim, TO_VICT);
		if (ch != victim)
			act ("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
	}
	else
	{
		SEND("They aren't undead.\r\n",ch);
	}
	ch->charms++;
	return;

}

SPELL (spell_commune_nature )
{
	char buf[MSL];
	int result = 0;
				
	result = number_percent();
	result += get_curr_stat(ch, STAT_WIS) / 2;
	
	if (result >= 75)
	{
		sprintf(buf, "You sense an abundance of %s in the surrounding area.\r\n", ch->in_room->area->main_resource);
		SEND(buf, ch);
	}
	else
		SEND("You were unable to commune with nature.\r\n",ch);
	
	
	
	return;
}

SPELL (spell_comprehend_languages )
{
	return;
}

SPELL (spell_darkness )
{
	if (!IS_SET(ch->in_room->room_flags, ROOM_DARK) || ch->in_room->light > 1)
	{
		ch->in_room->light = 0;
		SET_BIT(ch->in_room->room_flags, ROOM_DARK);
		SEND("You cover the area in darkness.\r\n",ch);
		act ("The area is covered in darkness.", ch, NULL, NULL, TO_ROOM);
	}
	return;
}

SPELL (spell_dimensional_anchor )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("you're already anchored to this plane.\r\n", ch);
        else
            act ("$N is already anchored to this plane.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel anchored to this plane.\r\n", victim);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}

SPELL (spell_elemental_protection )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, gsn_elemental_protection))
    {
        if (victim == ch)
            SEND ("You're already protected from the elements.\r\n", ch);
        else
            act ("$N is already protected from the elements.", ch, NULL, victim,
                 TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));

    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("A faint aura surrounds you for a moment then fades.\r\n", victim);
    if (ch != victim)
        SEND ("A faint aura surrounds them for a moment then fades.\r\n", ch);
    return;
}

SPELL (spell_erase )
{
	SEND ("Tell Upro to code this shit!\r\n",ch);
	return;
}

SPELL (spell_feather_fall )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            SEND ("You're already light as a feather!\r\n", ch);
        else
            act ("$N is already light as a feather!",
                 ch, NULL, victim, TO_CHAR);
        return;
    }    

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    
	if (victim == ch)
	{
        af.duration = level / 2;
	}
    else
	{
        af.duration = level / 4;
	}
	
	if (is_affected (ch, gsn_extension) && victim == ch)
			af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You feel light as a feather.\r\n", victim);
    act ("$n looks like they feel much lighter.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
        SEND ("Ok.\r\n", ch);
    return;
}

SPELL (spell_rehydrate )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char buf[MSL];
	
	if (!victim)
	{
		SEND("Invalid target.\r\n",ch);
		return;
	}
	
	gain_condition (victim, COND_THIRST, number_range(3,6));
	
	if (victim != ch)
		sprintf(buf, "%s appears somewhat less dehydrated.\r\n",victim->name);
	else
		sprintf(buf, "You feel less dehydrated.\r\n");
	
	SEND(buf, ch);
	return;
}

SPELL (spell_frostbite )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam = 0;
	
	dam = dice (level, 6);
    if (!saves_spell (level, victim, DAM_COLD))
    {
        act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
        af.where = TO_AFFECTS;
        af.type = sn;
        af.level = level;
		
        af.duration = number_range(4,8);
        
		af.location = APPLY_STR;
        af.modifier = -2;
        af.bitvector = 0;
        affect_join (victim, &af);
				
        af.location = APPLY_DEX;
        af.modifier = number_range(-2,-4);
        af.bitvector = 0;
        affect_join (victim, &af);
    }
    else
    {
        dam /= 2;
    }

    damage (ch, victim, dam, sn, DAM_COLD, TRUE);
    return;
}

SPELL (spell_monsoon )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 7);
    if (saves_spell (level, victim, DAM_DROWNING))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_DROWNING, TRUE);
    return;
}

SPELL (spell_frostbolt )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 8);
    if (saves_spell (level, victim, DAM_COLD))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_COLD, TRUE);
    return;
}

SPELL (spell_bleeding_touch )
{	
	CHAR_DATA *victim = (CHAR_DATA *) vo;
//	char buf[MSL];
    int dam;
	
	if (!IS_UNDEAD(victim))
	{
		victim->bleeding += (number_range(1,2));
		dam = dice (level, 4);
		if (saves_spell (level, victim, DAM_NEGATIVE))
			dam /= 2;
		damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);		
		return;
	}
	else
	{
		SEND ("They are unaffected by your spell!\r\n",ch);
		return;
	}
	
	return;
}

SPELL (spell_spectral_hand )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
	
    dam = dice (level, 5);
    if (saves_spell (level, victim, DAM_ENERGY))
        dam /= 2;
    damage (ch, victim, dam, sn, DAM_ENERGY, TRUE);
    return;
}



SPELL (spell_transfer_life)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim != ch )
    {
		SEND("You cannot transfer your life force to another.\r\n", ch);
		return;
    }

	if (victim->hit < (victim->max_hit / 4))
	{
		SEND("You are too weak to do this right now.\r\n",ch);
		return;
	}
	
	
	victim->mana += victim->hit / 3;
    victim->hit /= 2;

    update_pos( victim );
    SEND( "Your body becomes tired as you feel your cosmic powers grow.\n\r", victim );
    return;
}

SPELL (spell_summon_air)
{
	CHAR_DATA *elem;
	MOB_INDEX_DATA 	*mob;	
	int i = 0;

	
	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	mob = get_mob_index( MOB_VNUM_AIR_ELEM ); 
	elem = create_mobile(mob);
	
	elem->max_hit = number_range ( total_levels(ch) * 45, total_levels(ch) * 95 );
	elem->hit = elem->max_hit;
	elem->max_mana = number_range ( total_levels(ch) * 60, total_levels(ch) * 220 );
	elem->mana = elem->max_mana;
	elem->level = number_fuzzy(total_levels(ch));
	elem->armor[AC_PIERCE] = 	100 - (total_levels(ch) * 10);
    elem->armor[AC_BASH] = 	100 - (total_levels(ch) * 10);
    elem->armor[AC_SLASH] = 	100 - (total_levels(ch) * 10);
    elem->armor[AC_EXOTIC] = 	100 - (total_levels(ch) * 10);
	elem->damroll =		number_fuzzy ( total_levels(ch) );
	elem->hitroll = 		number_fuzzy ( total_levels(ch) / 2 ) + 8;
	for (i = 0; i < 2; i++)
        elem->damage[i] = 	elem->level/(number_range(6,12));
    elem->damage[2]	 = 	elem->level/2;
	for (i = 0; i < MAX_STATS; i++)
            elem->perm_stat[i] = 11 + elem->level/4;
	elem->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	SET_BIT(elem->affected_by, AFF_CHARM);
	
	SEND ("You summon a powerful air elemental to your aid!\r\n",ch);
	act ("$n summons a powerful air elemental!", ch, NULL, NULL, TO_ROOM);
	char_to_room(elem, ch->in_room);
	elem->leader = ch;
	elem->master = ch;
	ch->charms++;
	//elem->start_following( ch );
	return;
}

SPELL (spell_summon_water)
{
	CHAR_DATA *elem;
	MOB_INDEX_DATA 	*mob;	
	int i = 0;

	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	mob = get_mob_index( MOB_VNUM_WATER_ELEM ); 
	elem = create_mobile(mob);
	
	elem->max_hit = number_range ( total_levels(ch) * 40, total_levels(ch) * 85 );
	elem->hit = elem->max_hit;
	elem->max_mana = number_range ( total_levels(ch) * 90, total_levels(ch) * 250 );
	elem->mana = elem->max_mana;
	elem->level = number_fuzzy(total_levels(ch));
	elem->armor[AC_PIERCE] = 	170 - (total_levels(ch) * 10);
    elem->armor[AC_BASH] = 	170 - (total_levels(ch) * 10);
    elem->armor[AC_SLASH] = 	170 - (total_levels(ch) * 10);
    elem->armor[AC_EXOTIC] = 	170 - (total_levels(ch) * 10);
	elem->damroll =		number_fuzzy ( total_levels(ch) - 2 );
	elem->hitroll = 		number_fuzzy ( total_levels(ch) / 2 );
	for (i = 0; i < 2; i++)
        elem->damage[i] = 	elem->level/(number_range(6,12));
    elem->damage[2]	 = 	elem->level/2;
	for (i = 0; i < MAX_STATS; i++)
            elem->perm_stat[i] = 11 + elem->level/4;
	elem->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	SET_BIT(elem->affected_by, AFF_CHARM);
	
	SEND ("You summon a powerful water elemental to your aid!\r\n",ch);
	act ("$n summons a powerful water elemental!", ch, NULL, NULL, TO_ROOM);
	char_to_room(elem, ch->in_room);
	elem->leader = ch;
	elem->master = ch;
	ch->charms++;
	//elem->start_following( ch );
	return;
}

SPELL (spell_summon_earth)
{
	CHAR_DATA *elem;
	MOB_INDEX_DATA 	*mob;	
	int i = 0;

	
	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	mob = get_mob_index( MOB_VNUM_EARTH_ELEM ); 
	elem = create_mobile(mob);
	
	elem->max_hit = number_range ( total_levels(ch) * 75, total_levels(ch) * 125 );
	elem->hit = elem->max_hit;
	elem->max_mana = number_range ( total_levels(ch) * 25, total_levels(ch) * 100 );
	elem->mana = elem->max_mana;
	elem->level = number_fuzzy(total_levels(ch));
	elem->armor[AC_PIERCE] = 	120 - (total_levels(ch) * 10);
    elem->armor[AC_BASH] = 	120 - (total_levels(ch) * 10);
    elem->armor[AC_SLASH] = 	120 - (total_levels(ch) * 10);
    elem->armor[AC_EXOTIC] = 	120 - (total_levels(ch) * 10);
	elem->damroll =		number_fuzzy ( total_levels(ch) ) + 5;
	elem->hitroll = 		number_fuzzy ( total_levels(ch) / 2 );
	for (i = 0; i < 2; i++)
        elem->damage[i] = 	elem->level/(number_range(6,12));
    elem->damage[2]	 = 	elem->level/2;
	for (i = 0; i < MAX_STATS; i++)
            elem->perm_stat[i] = 11 + elem->level/4;
	elem->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	SET_BIT(elem->affected_by, AFF_CHARM);
	
	SEND ("You summon a powerful earth elemental to your aid!\r\n",ch);
	act ("$n summons a powerful earth elemental!", ch, NULL, NULL, TO_ROOM);
	char_to_room(elem, ch->in_room);
	elem->leader = ch;
	elem->master = ch;
	ch->charms++;
	//elem->start_following( ch );
	return;
}

SPELL (spell_summon_fire)
{
	CHAR_DATA *elem;
	MOB_INDEX_DATA 	*mob;	
	int i = 0;

	if( (!IS_NPC(ch)) && (ch->charms >= cha_app[GET_CHA(ch)].max_charms))
	{
		SEND ("You cannot control more followers...\r\n",ch);
		return;
	}
	
	mob = get_mob_index( MOB_VNUM_FIRE_ELEM ); 
	elem = create_mobile(mob);
	
	elem->max_hit = number_range ( total_levels(ch) * 50, total_levels(ch) * 100 );
	elem->hit = elem->max_hit;
	elem->max_mana = number_range ( total_levels(ch) * 50, total_levels(ch) * 200 );
	elem->mana = elem->max_mana;
	elem->level = number_fuzzy(total_levels(ch));
	elem->armor[AC_PIERCE] = 	150 - (total_levels(ch) * 10);
    elem->armor[AC_BASH] = 	150 - (total_levels(ch) * 10);
    elem->armor[AC_SLASH] = 	150 - (total_levels(ch) * 10);
    elem->armor[AC_EXOTIC] = 	150 - (total_levels(ch) * 10);
	elem->damroll =		number_fuzzy ( total_levels(ch) ) + 3;
	elem->hitroll = 		number_fuzzy ( total_levels(ch) / 2 ) + 3;
	for (i = 0; i < 2; i++)
        elem->damage[i] = 	elem->level/(number_range(6,12));
    elem->damage[2]	 = 	elem->level/2;
	for (i = 0; i < MAX_STATS; i++)
            elem->perm_stat[i] = 11 + elem->level/4;
	elem->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
	SET_BIT(elem->affected_by, AFF_CHARM);
	
	SEND ("You summon a powerful fire elemental to your aid!\r\n",ch);
	act ("$n summons a powerful fire elemental!", ch, NULL, NULL, TO_ROOM);
	char_to_room(elem, ch->in_room);
	elem->leader = ch;
	elem->master = ch;
	//elem->start_following( ch );
	ch->charms++;
	return;
}


SPELL (spell_manavert)
{

	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (victim != ch)
	{
		SEND ("You cannot cast this spell on another.\r\n",ch);
		return;
	}
	
    if (is_affected (victim, sn))
    {
        SEND ("You are already casting from your health.\r\n", ch);
        return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;			
    af.duration = 6;
	if (is_affected (ch, gsn_extension) && victim == ch)
		af.duration += number_range((af.duration * 1/5), (af.duration * 2/5));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    SEND ("You start casting from your health.\r\n", victim);
    return;	
}


SPELL(spell_holy_forge)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf, af; 
    int result, fail;
    int hit_bonus, dam_bonus;
    int dam = 0;

    if ( IS_IMMORTAL(ch) && get_trust(ch) < 49 )
    {
		SEND("If you can't set it, you can't do it.\n\r",ch);
		return;
    }
    if ( ch->ch_class != ch_class_PALADIN && !IS_GOOD( ch ))
    {
		SEND( "Only divine warriors can perform this.\n\r",ch);
		return;
    }

    if (obj->item_type != ITEM_WEAPON)
    {
		SEND("That isn't a weapon.\n\r",ch);
		return;
    }

    if (obj->wear_loc != -1)
    {
		SEND("The item must be carried for this.\n\r",ch);
		return;
    }

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
		obj = (OBJ_DATA *) vo;
		if ( (IS_OBJ_STAT(obj,ITEM_EVIL))||(IS_OBJ_STAT(obj,ITEM_ANTI_GOOD))
		 || (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)) || attack_table[obj->value[3]].damage == DAM_NEGATIVE )
		{
			act("$p hisses at you, fouling your holy powers.",ch,obj,NULL,TO_CHAR);
			return;
		}
    }

    if (IS_WEAPON_STAT(obj,WEAPON_LIGHT_DAM))
    {
		act("$p is already full of divine power.",ch,obj,NULL,TO_CHAR);
		return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
          if ( paf->location == APPLY_HITROLL )
          {
	    	hit_bonus = paf->modifier;
	    	fail += 3 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
	    	fail += 3 * (dam_bonus * dam_bonus);
	    }
	    else fail += 10;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    fail += 3 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    fail += 3 * (dam_bonus * dam_bonus);
	}
	    else fail += 10;

    }

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      	fail -= 5;
    if (ch->race == RACE_ELF)  
      	fail -= 10;
    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING))
	fail += 40;
    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	fail += 30;
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
	fail += 10;
    if (IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
	fail += 10;
    // if (IS_WEAPON_STAT(obj,WEAPON_STUN))
	// fail += 30;
    // if (IS_WEAPON_STAT(obj,WEAPON_QUICK))
	// fail += 50;
    // if (IS_WEAPON_STAT(obj,WEAPON_SMART))
	// fail += 50;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	fail -= 10;

    fail = URANGE(5,fail,95);

    /* the moment of truth */

// yuki's flag counter so there are no longer 4-5 flag weapons in the game

    // if ( weapon_flag_counter( obj ) > 2 )
    // {
        // if ( weapon_flag_counter( obj ) > 3 )
	    // fail = 110;
	// else
	    // fail = 95;
    // }

    result = number_percent();
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p hums momentarily... then shatters violently!",ch,obj,NULL,TO_CHAR);
	act("$p hums momentarily... shattering everywhere!",ch,obj,NULL,TO_ROOM);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	     vch_next	= vch->next;
	     if ( vch->in_room == NULL )
	         continue;

             if ( IS_PET( vch ) && (vch->master != NULL) && (vch->master == ch) )
                continue;

	     if ( ch->in_room == vch->in_room )
	     {
	         dam = 3 * ( vch->hit / 2 );
                 damage( ch, vch, dam, sn, DAM_PIERCE,TRUE/*, isObj*/  );
		 if ( ch->fighting != NULL )
	             stop_fighting( ch, TRUE );
	     }

	     else if ( vch->in_room->area == ch->in_room->area )
	         SEND( "You hear an explosion in the distance.\n\r", vch );
        }
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p hums with your faith...  then nothing.",ch,obj,NULL,TO_CHAR);
	act("$p hums with $n's faith...  then nothing.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next;
                affect_remove_obj(obj,paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL)
	&&   IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) )
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) &&  IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL) &&  IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_EVIL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        }
	else if ( IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
	{
	   obj->extra_flags = 0;
	   SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
        }
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        act("$p is not ready to slay the wicked.",ch,obj,NULL,TO_CHAR);
        return;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO,obj->level + 1);

    af.where	      = TO_WEAPON;
    af.type	      	= sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_HIT;
    af.modifier	      = 2;
    af.bitvector      = WEAPON_LIGHT_DAM;
    affect_to_obj(obj,&af);

    af.where	      = TO_OBJECT;
    af.type	     	 	= sn;
    af.level	      = level;
    af.duration	      = -1;
    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = ITEM_ANTI_EVIL;
    affect_to_obj(obj,&af);

    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = ITEM_ANTI_NEUTRAL;
    affect_to_obj(obj,&af);

    af.location	      = APPLY_NONE;
    af.modifier	      = 0;
    af.bitvector      = ITEM_GLOW;
    affect_to_obj(obj,&af);

    act("$p glows brightly with your holy faith.",ch,obj,NULL,TO_CHAR);
    act("$p glows brightly with $n's holy faith.",ch,obj,NULL,TO_ROOM);
    return;
}




//Determine a critical spell hit.


bool critical_spell(CHAR_DATA*ch)
{
	if (number_percent() < get_spell_crit_chance(ch))
		return TRUE;
	else
		return FALSE;	
}

bool is_devotion(int sn)
{
	if (skill_table[sn].spell_fun != spell_null && skill_table[sn].rating[PSION] > 0)
		return TRUE;
	else
		return FALSE;
}


