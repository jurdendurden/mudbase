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
#include <time.h>
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

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN (spec_breath_any);
DECLARE_SPEC_FUN (spec_breath_acid);
DECLARE_SPEC_FUN (spec_breath_fire);
DECLARE_SPEC_FUN (spec_breath_frost);
DECLARE_SPEC_FUN (spec_breath_gas);
DECLARE_SPEC_FUN (spec_breath_lightning);
DECLARE_SPEC_FUN (spec_breath_energy);
DECLARE_SPEC_FUN (spec_breath_shadow);
DECLARE_SPEC_FUN (spec_cast_adept);
DECLARE_SPEC_FUN (spec_cast_cleric);
DECLARE_SPEC_FUN (spec_cast_judge);
DECLARE_SPEC_FUN (spec_cast_mage);
DECLARE_SPEC_FUN (spec_cast_undead);
DECLARE_SPEC_FUN (spec_executioner);
DECLARE_SPEC_FUN (spec_fido);
DECLARE_SPEC_FUN (spec_guard);
DECLARE_SPEC_FUN (spec_janitor);
DECLARE_SPEC_FUN (spec_mayor);
DECLARE_SPEC_FUN (spec_poison);
DECLARE_SPEC_FUN (spec_thief);
DECLARE_SPEC_FUN (spec_nasty);
DECLARE_SPEC_FUN (spec_troll_member);
DECLARE_SPEC_FUN (spec_ogre_member);
DECLARE_SPEC_FUN (spec_patrolman);
DECLARE_SPEC_FUN (spec_repair);
DECLARE_SPEC_FUN (spec_recharge);
DECLARE_SPEC_FUN (spec_dragon);
DECLARE_SPEC_FUN (spec_beholder);
DECLARE_SPEC_FUN (spec_construct); //golems, psionic weapons, animate objects, etc..
DECLARE_SPEC_FUN (spec_lich); //Oh yeah baby Upro 1/8/2020
DECLARE_SPEC_FUN (spec_cast_psion); //Muahahahaha Upro 2/6/2020


/* the function table */
const struct spec_type spec_table[] = {
    {"spec_breath_any", spec_breath_any},
    {"spec_breath_acid", spec_breath_acid},
    {"spec_breath_fire", spec_breath_fire},
    {"spec_breath_frost", spec_breath_frost},
    {"spec_breath_gas", spec_breath_gas},
    {"spec_breath_shadow", spec_breath_shadow},
    {"spec_breath_energy", spec_breath_energy},
    {"spec_breath_lightning", spec_breath_lightning},
    {"spec_cast_adept", spec_cast_adept},
    {"spec_cast_cleric", spec_cast_cleric},
    {"spec_cast_judge", spec_cast_judge},
    {"spec_cast_mage", spec_cast_mage},
    {"spec_cast_undead", spec_cast_undead},
    {"spec_executioner", spec_executioner},
    {"spec_fido", spec_fido},
    {"spec_guard", spec_guard},
    {"spec_janitor", spec_janitor},
    {"spec_mayor", spec_mayor},
    {"spec_poison", spec_poison},
    {"spec_thief", spec_thief},
    {"spec_nasty", spec_nasty},
    {"spec_troll_member", spec_troll_member},
    {"spec_ogre_member", spec_ogre_member},
    {"spec_patrolman", spec_patrolman},
	{"spec_repair", spec_repair},
	{"spec_recharge", spec_recharge},
	{"spec_dragon", spec_dragon},
	{"spec_beholder", spec_beholder},
	{"spec_construct", spec_construct},	
	{"spec_lich", spec_lich},
	{"spec_cast_psion", spec_cast_psion},
    {NULL, NULL}
};

#define WIZ_MALADICT	5
const char * wiz_maladict[] =
{
	"blindness",
	"slow",
	"weaken",
	"poison",
	"plague",
	"curse"
};

#define WIZ_OFF_LOW		6
const char * wiz_off_low[] =
{
	"frost arrow",
	"flame arrow",
	"acid arrow",
	"chill touch",
	"burning hands",
	"shocking grasp",
	"magic missile"
};

#define WIZ_OFF_MED		4
const char * wiz_off_med[] =
{
	"lightning bolt",
	"fireball",
	"frost arrow",
	"flame arrow",
	"acid arrow"
};

#define WIZ_OFF_HIGH	4
const char * wiz_off_high[] = 
{
	"acid blast",
	"shadow bolt",
	"fireball",
	"lightning bolt",
	"shadow breath"
};

#define WIZ_BUFF_LOW	4
const char * wiz_buff_low[] =
{
	"armor",
	"shield",
	"fly",
	"blink",
	"giant strength"
};

#define WIZ_BUFF_HIGH	7
const char * wiz_buff_high[] =
{
	"extension",
	"sanctuary",
	"armor",
	"shield",
	"stone skin",
	"fly",
	"blink",	
	"displacement"
};


/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup (const char *name)
{
    int i;

    for (i = 0; spec_table[i].name != NULL; i++)
    {
        if (LOWER (name[0]) == LOWER (spec_table[i].name[0])
            && !str_prefix (name, spec_table[i].name))
            return spec_table[i].function;
    }

    return 0;
}

char *spec_name (SPEC_FUN * function)
{
    int i;

    for (i = 0; spec_table[i].function != NULL; i++)
    {
        if (function == spec_table[i].function)
            return spec_table[i].name;
    }

    return NULL;
}

bool spec_troll_member (CHAR_DATA * ch)
{
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM) || ch->in_room == NULL
        || IS_AFFECTED (ch, AFF_CHARM) || ch->fighting != NULL)
        return FALSE;

    /* find an ogre to beat up */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (!IS_NPC (vch) || ch == vch)
            continue;

        if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return FALSE;

        if (vch->pIndexData->group == GROUP_VNUM_OGRES
            && ch->level > vch->level - 2 && !is_safe (ch, vch))
        {
            if (number_range (0, count) == 0)
                victim = vch;

            count++;
        }
    }

    if (victim == NULL)
        return FALSE;

    /* say something, then raise hell */
    switch (number_range (0, 6))
    {
        default:
            message = NULL;
            break;
        case 0:
            message = "$n yells 'I've been looking for you, punk!'";
            break;
        case 1:
            message = "With a scream of rage, $n attacks $N.";
            break;
        case 2:
            message =
                "$n says 'What's slimy Ogre trash like you doing around here?'";
            break;
        case 3:
            message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
            break;
        case 4:
            message = "$n says 'There's no cops to save you this time!'";
            break;
        case 5:
            message = "$n says 'Time to join your brother, spud.'";
            break;
        case 6:
            message = "$n says 'Let's rock.'";
            break;
    }

    if (message != NULL)
        act (message, ch, NULL, victim, TO_ALL);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return TRUE;
}

bool spec_ogre_member (CHAR_DATA * ch)
{
    CHAR_DATA *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM) || ch->in_room == NULL
        || IS_AFFECTED (ch, AFF_CHARM) || ch->fighting != NULL)
        return FALSE;

    /* find an troll to beat up */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (!IS_NPC (vch) || ch == vch)
            continue;

        if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return FALSE;

        if (vch->pIndexData->group == GROUP_VNUM_TROLLS
            && ch->level > vch->level - 2 && !is_safe (ch, vch))
        {
            if (number_range (0, count) == 0)
                victim = vch;

            count++;
        }
    }

    if (victim == NULL)
        return FALSE;

    /* say something, then raise hell */
    switch (number_range (0, 6))
    {
        default:
            message = NULL;
            break;
        case 0:
            message = "$n yells 'I've been looking for you, punk!'";
            break;
        case 1:
            message = "With a scream of rage, $n attacks $N.'";
            break;
        case 2:
            message =
                "$n says 'What's Troll filth like you doing around here?'";
            break;
        case 3:
            message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
            break;
        case 4:
            message = "$n says 'There's no cops to save you this time!'";
            break;
        case 5:
            message = "$n says 'Time to join your brother, spud.'";
            break;
        case 6:
            message = "$n says 'Let's rock.'";
            break;
    }

    if (message != NULL)
        act (message, ch, NULL, victim, TO_ALL);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return TRUE;
}

bool spec_patrolman (CHAR_DATA * ch)
{
    CHAR_DATA *vch, *victim = NULL;
    OBJ_DATA *obj;
    char *message;
    int count = 0;

    if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM) || ch->in_room == NULL
        || IS_AFFECTED (ch, AFF_CHARM) || ch->fighting != NULL)
        return FALSE;

    /* look for a fight in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch == ch)
            continue;

        if (vch->fighting != NULL)
        {                        /* break it up! */
            if (number_range (0, count) == 0)
                victim = (vch->level > vch->fighting->level)
                    ? vch : vch->fighting;
            count++;
        }
    }

    if (victim == NULL
        || (IS_NPC (victim)
            && victim->spec_fun == ch->spec_fun)) return FALSE;

    if (((obj = get_eq_char (ch, WEAR_NECK_1)) != NULL)
        || ((obj = get_eq_char (ch, WEAR_NECK_2)) != NULL))
    {
        act ("You blow down hard on $p.", ch, obj, NULL, TO_CHAR);
        act ("$n blows on $p, ***WHEEEEEEEEEEEET***", ch, obj, NULL, TO_ROOM);

        for (vch = char_list; vch != NULL; vch = vch->next)
        {
            if (vch->in_room == NULL)
                continue;

            if (vch->in_room != ch->in_room
                && vch->in_room->area == ch->in_room->area)
                SEND ("You hear a shrill whistling sound.\r\n", vch);
        }
    }

    switch (number_range (0, 6))
    {
        default:
            message = NULL;
            break;
        case 0:
            message = "$n yells 'All roit! All roit! break it up!'";
            break;
        case 1:
            message =
                "$n says 'Society's to blame, but what's a bloke to do?'";
            break;
        case 2:
            message = "$n mumbles 'bloody kids will be the death of us all.'";
            break;
        case 3:
            message = "$n shouts 'Stop that! Stop that!' and attacks.";
            break;
        case 4:
            message = "$n pulls out his billy and goes to work.";
            break;
        case 5:
            message =
                "$n sighs in resignation and proceeds to break up the fight.";
            break;
        case 6:
            message = "$n says 'Settle down, you hooligans!'";
            break;
    }

    if (message != NULL)
        act (message, ch, NULL, NULL, TO_ALL);

    multi_hit (ch, victim, TYPE_UNDEFINED);

    return TRUE;
}


bool spec_nasty (CHAR_DATA * ch)
{
    CHAR_DATA *victim, *v_next;
    long gold;

    if (!IS_AWAKE (ch))
    {
        return FALSE;
    }

    if (ch->position != POS_FIGHTING)
    {
        for (victim = ch->in_room->people; victim != NULL; victim = v_next)
        {
            v_next = victim->next_in_room;
            if (!IS_NPC (victim)
                && (victim->level > ch->level)
                && (victim->level < ch->level + 10))
            {
                do_function (ch, &do_backstab, victim->name);
                if (ch->position != POS_FIGHTING)
                {
                    do_function (ch, &do_murder, victim->name);
                }

                /* should steal some coins right away? :) */
                return TRUE;
            }
        }
        return FALSE;            /*  No one to attack */
    }

    /* okay, we must be fighting.... steal some coins and flee */
    if ((victim = ch->fighting) == NULL)
        return FALSE;            /* let's be paranoid.... */

    switch (number_bits (2))
    {
        case 0:
            act ("$n rips apart your coin purse, spilling your gold!",
                 ch, NULL, victim, TO_VICT);
            act ("You slash apart $N's coin purse and gather his gold.",
                 ch, NULL, victim, TO_CHAR);
            act ("$N's coin purse is ripped apart!",
                 ch, NULL, victim, TO_NOTVICT);
            gold = victim->gold / 10;    /* steal 10% of his gold */
            victim->gold -= gold;
            ch->gold += gold;
            return TRUE;

        case 1:
            do_function (ch, &do_flee, "");
            return TRUE;

        default:
            return FALSE;
    }
}

/*
 * Core procedure for dragons.
 */
bool dragon (CHAR_DATA * ch, char *spell_name)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if (ch->position != POS_FIGHTING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_percent() < 15)
            break;
    }

    if (victim == NULL)
        return FALSE;

    if ((sn = skill_lookup (spell_name)) < 0)
        return FALSE;
    (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any (CHAR_DATA * ch)
{
    if (ch->position != POS_FIGHTING)
        return FALSE;

    switch (number_bits (3))
    {
        case 0:
            return spec_breath_fire (ch);
        case 1:
        case 2:
            return spec_breath_lightning (ch);
        case 3:
            return spec_breath_gas (ch);
        case 4:
            return spec_breath_acid (ch);
        case 5:
			return spec_breath_shadow (ch);
        case 6:
			return spec_breath_energy (ch);
        case 7:
            return spec_breath_frost (ch);
    }

    return FALSE;
}



bool spec_breath_acid (CHAR_DATA * ch)
{
	dragon(ch, "acid breath");
	return TRUE;
}

bool spec_breath_fire (CHAR_DATA * ch)
{
    dragon(ch, "fire breath");
	return TRUE;
}
bool spec_breath_shadow (CHAR_DATA * ch)
{
	dragon(ch, "shadow breath");
	return TRUE;
}

bool spec_breath_energy (CHAR_DATA * ch)
{
    dragon(ch, "energy breath");
	return TRUE;
}

bool spec_breath_frost (CHAR_DATA * ch)
{
    dragon(ch, "frost breath");
	return TRUE;
}

bool spec_breath_gas (CHAR_DATA * ch)
{
    dragon(ch, "gas breath");
	return TRUE;
}

bool spec_breath_lightning (CHAR_DATA * ch)
{
	dragon(ch, "lightning breath");
	return TRUE;
}


bool spec_cast_adept (CHAR_DATA * ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if (!IS_AWAKE (ch))
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
            && !IS_NPC (victim) && victim->level < 11)
            break;
    }

    if (victim == NULL)
        return FALSE;

    switch (number_bits (4))
    {
        case 0:
            act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
            spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
                         TARGET_CHAR);
            return TRUE;

        case 1:
            act ("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
            spell_bless (skill_lookup ("bless"), ch->level, ch, victim,
                         TARGET_CHAR);
            return TRUE;

        case 2:
            act ("$n utters the words 'judicandus noselacri'.", ch, NULL,
                 NULL, TO_ROOM);
            spell_cure_blindness (skill_lookup ("cure blindness"), ch->level,
                                  ch, victim, TARGET_CHAR);
            return TRUE;

        case 3:
            act ("$n utters the words 'judicandus dies'.", ch, NULL, NULL,
                 TO_ROOM);
            spell_cure_light (skill_lookup ("cure light"), ch->level, ch,
                              victim, TARGET_CHAR);
            return TRUE;

        case 4:
            act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
                 TO_ROOM);
            spell_cure_poison (skill_lookup ("cure poison"), ch->level, ch,
                               victim, TARGET_CHAR);
            return TRUE;

        case 5:
            act ("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM);
            spell_refresh (skill_lookup ("refresh"), ch->level, ch, victim,
                           TARGET_CHAR);
            return TRUE;

        case 6:
            act ("$n utters the words 'judicandus eugzagz'.", ch, NULL, NULL,
                 TO_ROOM);
            spell_cure_disease (skill_lookup ("cure disease"), ch->level, ch,
                                victim, TARGET_CHAR);
    }

    return FALSE;
}



bool spec_cast_cleric (CHAR_DATA * ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
	int loc = 0;
	OBJ_DATA *obj;
		
	
    if (ch->position != POS_FIGHTING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_bits (2) == 0)
            break;
    }

    if (victim == NULL)
        return FALSE;

	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
			continue;
		
		if (obj->item_type == ITEM_WAND && (number_percent() < 30 || is_affected(ch, gsn_silence)))
		{
			do_function(ch, &do_zap, "");
			return TRUE;
		}
	}
		
    for (;;)
    {
        int min_level;

        switch (number_bits (4))
        {
            case 0:
                min_level = 0;
                spell = "blindness";
                break;
            case 1:
                min_level = 3;
                spell = "cause serious";
                break;
            case 2:
                min_level = 7;
                spell = "earthquake";
                break;
            case 3:
                min_level = 9;
                spell = "cause critical";
                break;
            case 4:
                min_level = 10;
                spell = "dispel evil";
                break;
            case 5:
                min_level = 12;
                spell = "curse";
                break;
            case 6:
                min_level = 12;
                spell = "change sex";
                break;
            case 7:
                min_level = 13;
                spell = "flamestrike";
                break;
            case 8:
            case 9:
            case 10:
                min_level = 15;
                spell = "harm";
                break;
            case 11:
                min_level = 15;
                spell = "plague";
                break;
            default:
                min_level = 16;
                spell = "dispel magic";
                break;
        }

        if (ch->level >= min_level)
            break;
    }

    if ((sn = skill_lookup (spell)) < 0)
        return FALSE;
	if (!is_affected(ch, gsn_silence))
		(*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}

bool spec_cast_judge (CHAR_DATA * ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if (ch->position != POS_FIGHTING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_bits (2) == 0)
            break;
    }

    if (victim == NULL)
        return FALSE;

    spell = "high explosive";
    if ((sn = skill_lookup (spell)) < 0)
        return FALSE;
	if (!is_affected(ch, gsn_silence))
		(*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}



bool spec_cast_mage (CHAR_DATA * ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    //char *spell;
    int sn;
	int i;
	int loc = 0;
	OBJ_DATA *obj;
		
	
	
	if (ch->position != POS_FIGHTING)
	{
        //return FALSE;
		
		
		if (ch->level <= 10)
		{
			for (i=0; i < WIZ_BUFF_LOW; i++)
			{
				if (is_affected(ch, skill_lookup(wiz_buff_low[i])))
					continue;
				else
				{
					sn = skill_lookup(wiz_buff_low[i]);
					(*skill_table[sn].spell_fun) (sn, ch->level, ch, ch, TARGET_CHAR);					
					return TRUE;
				}
			}
		}
		else
		{
			for (i=0; i < WIZ_BUFF_HIGH; i++)
			{
				if (is_affected(ch, skill_lookup(wiz_buff_high[i])))
					continue;
				else
				{
					sn = skill_lookup(wiz_buff_high[i]);					
					(*skill_table[sn].spell_fun) (sn, ch->level, ch, ch, TARGET_CHAR);					
					return TRUE;
				}
			}
		}		
	}		
	
    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_percent() < 75)
            break;
    }

    if (victim == NULL)
        return FALSE;

	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
			continue;
		
		if (obj->item_type == ITEM_WAND && (number_percent() < 30 || is_affected(ch, gsn_silence)))
		{
			do_function(ch, &do_zap, "");
			return TRUE;
		}
	}
	
	int x = 0;
	
	if (number_percent() < 30)
	{
		x = number_range(0, WIZ_MALADICT);
		sn = skill_lookup(wiz_maladict[x]);					
		(*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);			
		return TRUE;
	}
	else
	{
		if (ch->level <= 10)
		{
			x = number_range(0, WIZ_OFF_LOW);
			sn = skill_lookup(wiz_off_low[x]);					
		}
		else if (ch->level > 10 && ch->level < 20)
		{
			x = number_range(0, WIZ_OFF_MED);
			sn = skill_lookup(wiz_off_med[x]);					
		}
		else
		{
			x = number_range(0, WIZ_OFF_HIGH);
			sn = skill_lookup(wiz_off_high[x]);					
		}
		
		if (!is_affected(ch, gsn_silence))
			(*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);			
		return TRUE;
	}
	
}



bool spec_cast_undead (CHAR_DATA * ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if (ch->position != POS_FIGHTING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_bits (2) == 0)
            break;
    }

    if (victim == NULL)
        return FALSE;

    for (;;)
    {
        int min_level;

        switch (number_bits (4))
        {
            case 0:
                min_level = 0;
                spell = "curse";
                break;
            case 1:
                min_level = 3;
                spell = "weaken";
                break;
            case 2:
                min_level = 6;
                spell = "chill touch";
                break;
            case 3:
                min_level = 9;
                spell = "blindness";
                break;
            case 4:
                min_level = 12;
                spell = "poison";
                break;
            case 5:
                min_level = 15;
                spell = "energy drain";
                break;
            case 6:
                min_level = 18;
                spell = "harm";
                break;
            case 7:
                min_level = 21;
                spell = "teleport";
                break;
            case 8:
                min_level = 20;
                spell = "plague";
                break;
            default:
                min_level = 18;
                spell = "harm";
                break;
        }

        if (ch->level >= min_level)
            break;
    }

    if ((sn = skill_lookup (spell)) < 0)
        return FALSE;
    (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}


bool spec_executioner (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;

    if (!IS_AWAKE (ch) || ch->fighting != NULL)
        return FALSE;

    crime = "";
    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;

        if (!IS_NPC (victim) && IS_SET (victim->act, PLR_KILLER)
            && can_see (ch, victim))
        {
            crime = "KILLER";
            break;
        }

        if (!IS_NPC (victim) && IS_SET (victim->act, PLR_THIEF)
            && can_see (ch, victim))
        {
            crime = "THIEF";
            break;
        }
    }

    if (victim == NULL)
        return FALSE;

    sprintf (buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
             victim->name, crime);
    REMOVE_BIT (ch->comm, COMM_NOSHOUT);
    do_function (ch, &do_yell, buf);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return TRUE;
}



bool spec_fido (CHAR_DATA * ch)
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if (!IS_AWAKE (ch))
        return FALSE;

    for (corpse = ch->in_room->contents; corpse != NULL; corpse = c_next)
    {
        c_next = corpse->next_content;
        if (corpse->item_type != ITEM_CORPSE_NPC)
            continue;

        act ("$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);
        for (obj = corpse->contains; obj; obj = obj_next)
        {
            obj_next = obj->next_content;
            obj_from_obj (obj);
            obj_to_room (obj, ch->in_room);
        }
        extract_obj (corpse);
        return TRUE;
    }

    return FALSE;
}



bool spec_guard (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if (!IS_AWAKE (ch) || ch->fighting != NULL)
        return FALSE;

    max_evil = 300;
    ech = NULL;
    crime = "";

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;

        if (!IS_NPC (victim) && IS_SET (victim->act, PLR_KILLER)
            && can_see (ch, victim))
        {
            crime = "KILLER";
            break;
        }

        if (!IS_NPC (victim) && IS_SET (victim->act, PLR_THIEF)
            && can_see (ch, victim))
        {
            crime = "THIEF";
            break;
        }

        if (victim->fighting != NULL
            && victim->fighting != ch && victim->alignment < max_evil)
        {
            max_evil = victim->alignment;
            ech = victim;
        }
    }

    if (victim != NULL)
    {
        sprintf (buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
                 victim->name, crime);
        REMOVE_BIT (ch->comm, COMM_NOSHOUT);
        do_function (ch, &do_yell, buf);
        multi_hit (ch, victim, TYPE_UNDEFINED);
        return TRUE;
    }

    if (ech != NULL)
    {
        act ("$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
             ch, NULL, NULL, TO_ROOM);
        multi_hit (ch, ech, TYPE_UNDEFINED);
        return TRUE;
    }

    return FALSE;
}



bool spec_janitor (CHAR_DATA * ch)
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if (!IS_AWAKE (ch))
        return FALSE;

    for (trash = ch->in_room->contents; trash != NULL; trash = trash_next)
    {
        trash_next = trash->next_content;
        if (!IS_SET (trash->wear_flags, ITEM_TAKE) || !can_loot (ch, trash))
            continue;
        if (trash->item_type == ITEM_DRINK_CON
            || trash->item_type == ITEM_TRASH || trash->cost < 10)
        {
            act ("$n picks up some trash.", ch, NULL, NULL, TO_ROOM);
            obj_from_room (trash);
            obj_to_char (trash, ch);
            return TRUE;
        }
    }

    return FALSE;
}



bool spec_mayor (CHAR_DATA * ch)
{
    static const char open_path[] =
        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if (!move)
    {
        if (time_info.hour == 6)
        {
            path = open_path;
            move = TRUE;
            pos = 0;
        }

        if (time_info.hour == 20)
        {
            path = close_path;
            move = TRUE;
            pos = 0;
        }
    }

    if (ch->fighting != NULL)
        return spec_cast_mage (ch);
    if (!move || ch->position < POS_SLEEPING)
        return FALSE;

    switch (path[pos])
    {
        case '0':
        case '1':
        case '2':
        case '3':
            move_char (ch, path[pos] - '0', FALSE);
            break;

        case 'W':
            ch->position = POS_STANDING;
            act ("$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM);
            break;

        case 'S':
            ch->position = POS_SLEEPING;
            act ("$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM);
            break;

        case 'a':
            act ("$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM);
            break;

        case 'b':
            act
                ("$n says 'What a view!  I must do something about that dump!'",
                 ch, NULL, NULL, TO_ROOM);
            break;

        case 'c':
            act
                ("$n says 'Vandals!  Youngsters have no respect for anything!'",
                 ch, NULL, NULL, TO_ROOM);
            break;

        case 'd':
            act ("$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM);
            break;

        case 'e':
            act ("$n says 'I hereby declare the city of Midgaard open!'",
                 ch, NULL, NULL, TO_ROOM);
            break;

        case 'E':
            act ("$n says 'I hereby declare the city of Midgaard closed!'",
                 ch, NULL, NULL, TO_ROOM);
            break;

        case 'O':
/*    do_function(ch, &do_unlock, "gate" ); */
            do_function (ch, &do_open, "gate");
            break;

        case 'C':
            do_function (ch, &do_close, "gate");
/*    do_function(ch, &do_lock, "gate" ); */
            break;

        case '.':
            move = FALSE;
            break;
    }

    pos++;
    return FALSE;
}



bool spec_poison (CHAR_DATA * ch)
{
    CHAR_DATA *victim;

    if (ch->position != POS_FIGHTING
        || (victim = ch->fighting) == NULL
        || number_percent () > 2 * ch->level) return FALSE;

    act ("You bite $N!", ch, NULL, victim, TO_CHAR);
    act ("$n bites $N!", ch, NULL, victim, TO_NOTVICT);
    act ("$n bites you!", ch, NULL, victim, TO_VICT);
    spell_poison (gsn_poison, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}



bool spec_thief (CHAR_DATA * ch)
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long gold, silver;

    if (ch->position != POS_STANDING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;

        if (IS_NPC (victim)
            || victim->level >= LEVEL_IMMORTAL
            || number_bits (5) != 0 || !can_see (ch, victim))
            continue;

        if (IS_AWAKE (victim) && number_range (0, ch->level) == 0)
        {
            act ("You discover $n's hands in your wallet!",
                 ch, NULL, victim, TO_VICT);
            act ("$N discovers $n's hands in $S wallet!",
                 ch, NULL, victim, TO_NOTVICT);
            return TRUE;
        }
        else
        {
            gold =
                victim->gold * UMIN (number_range (1, 20),
                                     ch->level / 2) / 100;
            gold = UMIN (gold, ch->level * ch->level * 10);
            ch->gold += gold;
            victim->gold -= gold;
            silver =
                victim->silver * UMIN (number_range (1, 20),
                                       ch->level / 2) / 100;
            silver = UMIN (silver, ch->level * ch->level * 25);
            ch->silver += silver;
            victim->silver -= silver;
            return TRUE;
        }
    }

    return FALSE;
}


bool spec_repair( CHAR_DATA *ch )
{
    if (ch->fighting != NULL) return spec_cast_mage( ch );
    return FALSE;
}

bool spec_recharge( CHAR_DATA *ch )
{
    if (ch->fighting != NULL) return spec_cast_mage( ch );
    return FALSE;
}

bool spec_dragon( CHAR_DATA *ch )
{
	//	int attackChance = 0;
	
    if (ch->fighting != NULL) 
	{
		if (ch->fighting->hit < (ch->fighting->max_hit / 2))
		{
			//do breathe;
			return spec_breath_any (ch);
		}
		else
			return spec_cast_mage( ch );
	}
	
	return FALSE;
}

bool spec_construct( CHAR_DATA *ch )
{
	// CHAR_DATA *victim;
    // CHAR_DATA *v_next;
    // char *spell;
    // int sn;
	
	if (ch->position != POS_FIGHTING)
        return FALSE;
		
	if (ch->master || ch->leader/*&& ch->master->hit < (ch->master->max_hit / 2)&& ch->leader->tanking == TRUE*/)
		do_function (ch, &do_rescue, ch->master->name);
	
	return TRUE;
}

bool spec_beholder( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if (ch->position != POS_FIGHTING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_percent() < 50)
            break;
    }

    if (victim == NULL)
        return FALSE;

    for (;;)
    {
        switch ( number_range (0,8) )
        {
            case 0:
				if (!is_affected(victim, gsn_slow))
				{	
					spell =  "slow";
					break;
				}
				else
					continue;
            case 1:	
				spell = "fear";
				break;
            case 2:
                if (!is_affected(victim, gsn_feeblemind))
				{	
					spell =  "feeblemind";
					break;
				}
				else
					continue;
            case 3:
				if (!is_affected(victim, gsn_confusion))
				{					
					spell =  "confusion";
					break;
				}
				else
					continue;
            case 4:		
				spell =  "disintegrate";				
				break;
            case 5:
				if (check_immune (victim, DAM_NEGATIVE) != IS_IMMUNE)
				{			
					spell =  "shadow bolt";
					break;
				}
            case 6:
				if (check_immune (victim, DAM_LIGHT) != IS_IMMUNE)
				{			
					spell =  "colour spray";
					break;
				}
            case 7:
				if (check_immune (victim, DAM_FIRE) != IS_IMMUNE)
				{		
					spell =  "flame arrow";
					break;			
				}
            case 8:                
				if (check_immune (victim, DAM_COLD) != IS_IMMUNE)
				{
					spell =  "frost arrow";
					break;		
				}
            default:			
                spell = "harm";
                break;
        }
		break;
    }
    if ((sn = skill_lookup (spell)) < 0)
        return FALSE;
	
	act ("$n releases a spell from one of it's EYES!", ch, NULL, NULL, TO_ROOM);
    (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}

bool spec_cast_psion(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

	if (ch->position != POS_FIGHTING)
        return FALSE;

    //Pull a random victim from the room...
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_percent() < 50)
            break;
    }

	if (victim == NULL)
        return FALSE;
	
	for (;;)
    {
        switch ( number_range (0,8) )
        {
            case 0:				
				spell = "agitation";
				break;								

            case 1:	
			case 2:
				spell = "ballistic attack";
				break;

            case 3:
			case 4:
				spell = "detonate";
				break;               

			case 5:
                if (!is_affected(victim, gsn_ego_whip))
				{	
					spell =  "ego whip";
					break;
				}
				else
					continue; 			
			case 6:
				if (!is_affected(victim, gsn_mindwipe))
				{	
					spell =  "mindwipe";
					break;
				}
				else
					continue; 
            case 7:			
				if (!is_affected(victim, gsn_psychic_drain))
				{	
					spell =  "psychic drain";
					break;
				}
				else
					continue; 
			case 8:
				if (!is_affected(victim, gsn_daydream))
				{	
					spell =  "daydream";
					break;
				}
				else
					continue;                               
            default:			
                spell = "";
                break;
        }
		break;
	}
	if ((sn = skill_lookup (spell)) < 0)
        return FALSE;

	act ("A sense of desparate helplessness overtakes you as $n emits a powerful psionic devotion!", ch, NULL, NULL, TO_ROOM);
    (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}


bool spec_lich( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if (ch->position != POS_FIGHTING)
        return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_percent() < 50)
            break;
    }

    if (victim == NULL)
        return FALSE;

    for (;;)
    {
        switch ( number_range (0,8) )
        {
            case 0:
				if (!is_affected(victim, gsn_slow))
				{	
					spell =  "slow";
					break;
				}
				else
					continue;
            case 1:	
				spell = "fear";
				break;
            case 2:
                if (!is_affected(victim, gsn_curse))
				{	
					spell =  "curse";
					break;
				}
				else
					continue;
			case 3:
                if (!is_affected(victim, gsn_weaken))
				{	
					spell =  "weaken";
					break;
				}
				else
					continue; 
			case 4:
			case 5:
				spell = "geyser";
				break;
            case 6:		
			case 7:
			case 8:
				spell =  "harm";				
				break;                                 
            default:			
                spell = "";
                break;
        }
		break;
    }
    if ((sn = skill_lookup (spell)) < 0)
        return FALSE;
	
	act ("An impending doom fills your soul as $n casts a powerful spell!", ch, NULL, NULL, TO_ROOM);
    (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
    return TRUE;
}

