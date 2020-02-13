/*************************************************************************
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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_wiz.c,v 1.3 2000/12/01 10:48:33 ring0 Exp $ */

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
#include <unistd.h>                /* For execl in copyover() */
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "olc.h"

/*
 * Stolen from save.c for reading in QuickMUD config stuff
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )			\
                if ( !str_cmp( word, literal ) )	\
		{					\
			field  = value;			\
			fMatch = TRUE;			\
			break;				\
		}

int colorstrlen args 		((const char *argument));
void spell_null args 		((int sn, int level, CHAR_DATA * ch, void *vo, int target));
bool check_parse_name args	((char* name));  /* comm.c */
bool    double_exp = FALSE;   
bool    double_gold = FALSE;   
bool    double_skill = FALSE;   

char *const where_name2[] = {
    "{r<{x  light  {r>{x ",
    "{r<{x finger  {r>{x ",
    "{r<{x finger  {r>{x ",
    "{r<{x  neck   {r>{x ",
    "{r<{x  neck   {r>{x ",
    "{r<{x  torso  {r>{x ",
    "{r<{x  head   {r>{x ",
    "{r<{x  legs   {r>{x ",
    "{r<{x  feet   {r>{x ",
    "{r<{x  hands  {r>{x ",
    "{r<{x  arms   {r>{x ",    
    "{r<{x  body   {r>{x ",
    "{r<{x  waist  {r>{x ",
    "{r<{x  wrist  {r>{x ",
    "{r<{x  wrist  {r>{x ",
    "{r<{x wielded {r>{x ",
	"{r<{x shield  {r>{x ",
	"{r<{x offhand {r>{x ",
    "{r<{x  held   {r>{x ",
    "{r<{x floating{r>{x ",
};

		
/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));

void do_wiznet (CHAR_DATA * ch, char *argument)
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->wiznet, WIZ_ON))
        {
            SEND ("Signing off of Wiznet.\r\n", ch);
            REMOVE_BIT (ch->wiznet, WIZ_ON);
        }
        else
        {
            SEND ("Welcome to Wiznet!\r\n", ch);
            SET_BIT (ch->wiznet, WIZ_ON);
        }
        return;
    }

    if (!str_prefix (argument, "on"))
    {
        SEND ("Welcome to Wiznet!\r\n", ch);
        SET_BIT (ch->wiznet, WIZ_ON);
        return;
    }

    if (!str_prefix (argument, "off"))
    {
        SEND ("Signing off of Wiznet.\r\n", ch);
        REMOVE_BIT (ch->wiznet, WIZ_ON);
        return;
    }

    //Show wiznet status and options, updated Upro 1/7/2020

     if (!str_prefix (argument, "show") ||  !str_prefix (argument, "status") ||  !str_prefix (argument, "list") ||  !str_prefix (argument, "options"))
        /* list of all wiznet options */
    {
        buf[0] = '\0';

        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust (ch))
            {
				if (IS_SET (ch->wiznet, wiznet_table[flag].flag))
				{ 
				strcat (buf, "({rON{x) ");
				}
				else
				{
				strcat (buf, "(--) ");
				}
                strcat (buf, wiznet_table[flag].name);				
                strcat (buf, "\r\n");
            }
        }

        strcat (buf, "\r\n");

        SEND ("Wiznet options available to you are:\r\n", ch);
        SEND (buf, ch);
        return;
    }

    flag = wiznet_lookup (argument);

    if (flag == -1 || get_trust (ch) < wiznet_table[flag].level)
    {
        SEND ("No such option.\r\n", ch);
        return;
    }

    if (IS_SET (ch->wiznet, wiznet_table[flag].flag))
    {
        sprintf (buf, "You will no longer see %s on wiznet.\r\n",
                 wiznet_table[flag].name);
        SEND (buf, ch);
        REMOVE_BIT (ch->wiznet, wiznet_table[flag].flag);
        return;
    }
    else
    {
        sprintf (buf, "You will now see %s on wiznet.\r\n",
                 wiznet_table[flag].name);
        SEND (buf, ch);
        SET_BIT (ch->wiznet, wiznet_table[flag].flag);
        return;
    }

}

void wiznet (char *string, CHAR_DATA * ch, OBJ_DATA * obj,
             long flag, long flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING && IS_IMMORTAL (d->character)
            && IS_SET (d->character->wiznet, WIZ_ON)
            && (!flag || IS_SET (d->character->wiznet, flag))
            && (!flag_skip || !IS_SET (d->character->wiznet, flag_skip))
            && get_trust (d->character) >= min_level && d->character != ch)
        {
            if (IS_SET (d->character->wiznet, WIZ_PREFIX))
                SEND ("{Z--> ", d->character);
            else
                SEND ("{Z", d->character);
            act_new (string, d->character, obj, ch, TO_CHAR, POS_DEAD);
            SEND ("{x", d->character);
        }
    }

    return;
}

/* equips a character */
void do_outfit (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    int i, sn, vnum;
	char buf[MSL];

    if (IS_NPC(ch))
		return;
	
	if (ch->level > 5)
    {
        SEND ("You've advanced beyond outfitting.\r\n", ch);
        return;
    }

	if (ch->carry_number+4 > can_carry_n(ch))
    {
		SEND("You are carrying too much, try dropping some items.\r\n",ch);
        return;
    }
	
    if ((obj = get_eq_char (ch, WEAR_LIGHT)) == NULL)
    {
        obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_BANNER), 0);
        obj->cost = 0;
        obj_to_char (obj, ch);
        equip_char (ch, obj, WEAR_LIGHT);
    }

    if ((obj = get_eq_char (ch, WEAR_BODY)) == NULL)
    {
        obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_VEST), 0);
        obj->cost = 0;
        obj_to_char (obj, ch);
        equip_char (ch, obj, WEAR_BODY);
    }

    /* do the weapon thing */
    if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
        sn = 0;
        vnum = OBJ_VNUM_SCHOOL_SWORD;    /* just in case! */

		
        for (i = 0; weapon_table[i].name != NULL; i++)
        {
            if (IS_FIGHTER(ch))
			{
				int x;
				for (x = 0; weapon_table[x].name != NULL; x++)
				{
					if (wpnSpecTable[ch->pcdata->weaponSpec].wpnType == weapon_table[x].type)
					{
						vnum = weapon_table[x].vnum;
						break;
					}
				}
			}
			else
			{
				if (ch->pcdata->learned[sn] <
					ch->pcdata->learned[*weapon_table[i].gsn])
				{
					sn = *weapon_table[i].gsn;
					vnum = weapon_table[i].vnum;
				}
			}
        }

        obj = create_object (get_obj_index (vnum), 0);
        obj_to_char (obj, ch);
        equip_char (ch, obj, WEAR_WIELD);
    }

    if (((obj = get_eq_char (ch, WEAR_WIELD)) == NULL
         || !IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
        && (obj = get_eq_char (ch, WEAR_SHIELD)) == NULL)
    {
        obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_SHIELD), 0);
        obj->cost = 0;
        obj_to_char (obj, ch);
        equip_char (ch, obj, WEAR_SHIELD);
    }

    sprintf (buf, "You have been equipped by %s.\r\n", god_table[ch->god].name);
	SEND(buf,ch);
}


/* RT nochannels command, for those spammers */
void do_nochannels (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Nochannel whom?", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_NOCHANNELS))
    {
        REMOVE_BIT (victim->comm, COMM_NOCHANNELS);
        SEND ("The gods have restored your channel priviliges.\r\n",
                      victim);
        SEND ("NOCHANNELS removed.\r\n", ch);
        sprintf (buf, "$N restores channels to %s", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
    else
    {
        SET_BIT (victim->comm, COMM_NOCHANNELS);
        SEND ("The gods have revoked your channel priviliges.\r\n",
                      victim);
        SEND ("NOCHANNELS set.\r\n", ch);
        sprintf (buf, "$N revokes %s's channels.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}


void do_smote (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *vch;
    char *letter, *name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
        SEND ("You can't show your emotions.\r\n", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        SEND ("Emote what?\r\n", ch);
        return;
    }

    if (strstr (argument, ch->name) == NULL)
    {
        SEND ("You must include your name in an smote.\r\n", ch);
        return;
    }

    SEND (argument, ch);
    SEND ("\r\n", ch);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;

        if ((letter = strstr (argument, vch->name)) == NULL)
        {
            SEND (argument, vch);
            SEND ("\r\n", vch);
            continue;
        }

        strcpy (temp, argument);
        temp[strlen (argument) - strlen (letter)] = '\0';
        last[0] = '\0';
        name = vch->name;

        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen (vch->name))
            {
                strcat (temp, "r");
                continue;
            }

            if (*letter == 's' && matches == strlen (vch->name))
            {
                matches = 0;
                continue;
            }

            if (matches == strlen (vch->name))
            {
                matches = 0;
            }

            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen (vch->name))
                {
                    strcat (temp, "you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat (last, letter, 1);
                continue;
            }

            matches = 0;
            strcat (temp, last);
            strncat (temp, letter, 1);
            last[0] = '\0';
            name = vch->name;
        }

        SEND (temp, vch);
        SEND ("\r\n", vch);
    }

    return;
}

void do_bamfin (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC (ch))
    {
        smash_tilde (argument);

        if (argument[0] == '\0')
        {
            sprintf (buf, "Your poofin is %s\r\n", ch->pcdata->bamfin);
            SEND (buf, ch);
            return;
        }

        /*if (strstr (argument, ch->name) == NULL)
        {
            SEND ("You must include your name.\r\n", ch);
            return;
        }*/

        free_string (ch->pcdata->bamfin);
        ch->pcdata->bamfin = str_dup (argument);

        sprintf (buf, "Your poofin is now %s\r\n", ch->pcdata->bamfin);
        SEND (buf, ch);
    }
    return;
}

void do_bamfout (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC (ch))
    {
        smash_tilde (argument);

        if (argument[0] == '\0')
        {
            sprintf (buf, "Your poofout is %s\r\n", ch->pcdata->bamfout);
            SEND (buf, ch);
            return;
        }

		/*
        if (strstr (argument, ch->name) == NULL)
        {
            SEND ("You must include your name.\r\n", ch);
            return;
        }*/

        free_string (ch->pcdata->bamfout);
        ch->pcdata->bamfout = str_dup (argument);

        sprintf (buf, "Your poofout is now %s\r\n", ch->pcdata->bamfout);
        SEND (buf, ch);
    }
    return;
}



void do_deny (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Deny whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    SET_BIT (victim->act, PLR_DENY);
    SEND ("You are denied access!\r\n", victim);
    sprintf (buf, "$N denies access to %s", victim->name);
    wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    SEND ("OK.\r\n", ch);
    save_char_obj (victim);
    stop_fighting (victim, TRUE);
    do_function (victim, &do_quit, "");

    return;
}



void do_disconnect (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Disconnect whom?\r\n", ch);
        return;
    }

    if (is_number (arg))
    {
        int desc;

        desc = atoi (arg);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->descriptor == desc)
            {
                close_socket (d);
                SEND ("Ok.\r\n", ch);
                return;
            }
        }
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->desc == NULL)
    {
        act ("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
        return;
    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d == victim->desc)
        {
            close_socket (d);
            SEND ("Ok.\r\n", ch);
            return;
        }
    }

    bug ("Do_disconnect: desc not found.", 0);
    SEND ("Descriptor not found!\r\n", ch);
    return;
}



void do_pardon (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        SEND ("Syntax: pardon <character> <killer|thief>.\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    if (!str_cmp (arg2, "killer"))
    {
        if (IS_SET (victim->act, PLR_KILLER))
        {
            REMOVE_BIT (victim->act, PLR_KILLER);
            SEND ("Killer flag removed.\r\n", ch);
            SEND ("You are no longer a KILLER.\r\n", victim);
        }
        return;
    }

    if (!str_cmp (arg2, "thief"))
    {
        if (IS_SET (victim->act, PLR_THIEF))
        {
            REMOVE_BIT (victim->act, PLR_THIEF);
            SEND ("Thief flag removed.\r\n", ch);
            SEND ("You are no longer a THIEF.\r\n", victim);
        }
        return;
    }

    SEND ("Syntax: pardon <character> <killer|thief>.\r\n", ch);
    return;
}



void do_echo (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;
	
    if (argument[0] == '\0')
    {
        SEND ("Global echo what?\r\n", ch);
        return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
        if (d->connected == CON_PLAYING)
        {
            if (!ch || get_trust (d->character) >= get_trust (ch))
                SEND ("global> ", d->character);
            SEND (argument, d->character);
            SEND ("\r\n", d->character);
        }
    }

    return;
}



void do_recho (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        SEND ("Local echo what?\r\n", ch);

        return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
        if (d->connected == CON_PLAYING
            && d->character->in_room == ch->in_room)
        {
            if (get_trust (d->character) >= get_trust (ch))
                SEND ("local> ", d->character);
            SEND (argument, d->character);
            SEND ("\r\n", d->character);
        }
    }

    return;
}

void do_zecho (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        SEND ("Zone echo what?\r\n", ch);
        return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
        if (d->connected == CON_PLAYING
            && d->character->in_room != NULL && ch->in_room != NULL
            && d->character->in_room->area == ch->in_room->area)
        {
            if (get_trust (d->character) >= get_trust (ch))
                SEND ("zone> ", d->character);
            SEND (argument, d->character);
            SEND ("\r\n", d->character);
        }
    }
}

void do_pecho (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument (argument, arg);

    if (argument[0] == '\0' || arg[0] == '\0')
    {
        SEND ("Personal echo what?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("Target not found.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch) && get_trust (ch) != MAX_LEVEL)
        SEND ("personal> ", victim);

    SEND (argument, victim);
    SEND ("\r\n", victim);
    SEND ("personal> ", ch);
    SEND (argument, ch);
    SEND ("\r\n", ch);
}


ROOM_INDEX_DATA *find_location (CHAR_DATA * ch, char *arg)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if (is_number (arg))
        return get_room_index (atoi (arg));

    if ((victim = get_char_world (ch, arg)) != NULL)
        return victim->in_room;

    if ((obj = get_obj_world (ch, arg)) != NULL)
        return obj->in_room;

    return NULL;
}



void do_transfer (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0')
    {
        SEND ("Transfer whom (and where)?\r\n", ch);
        return;
    }

    if (!str_cmp (arg1, "all"))
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->connected == CON_PLAYING
                && d->character != ch
                && d->character->in_room != NULL
                && can_see (ch, d->character))
            {
                char buf[MAX_STRING_LENGTH];
                sprintf (buf, "%s %s", d->character->name, arg2);
                do_function (ch, &do_transfer, buf);
            }
        }
        return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if (arg2[0] == '\0')
    {
        location = ch->in_room;
    }
    else
    {
        if ((location = find_location (ch, arg2)) == NULL)
        {
            SEND ("No such location.\r\n", ch);
            return;
        }

        if (!is_room_owner (ch, location) && room_is_private (location)
            && get_trust (ch) < MAX_LEVEL)
        {
            SEND ("That room is private right now.\r\n", ch);
            return;
        }
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->in_room == NULL)
    {
        SEND ("They are in limbo.\r\n", ch);
        return;
    }

	if (victim->onBoard)
		victim->onBoard = FALSE;
	
    if (victim->fighting != NULL)
        stop_fighting (victim, TRUE);
    act ("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
    char_from_room (victim);
    char_to_room (victim, location);
    act ("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
        act ("$n has transferred you.", ch, NULL, victim, TO_VICT);
    do_function (victim, &do_look, "auto");
    SEND ("Ok.\r\n", ch);
}


void do_create(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL];
	char arg2[MIL];
	char arg3[MIL];
	char arg4[MIL];
	int amount = 0;	
	
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	argument = one_argument(argument, arg4);
	
	if (arg1[0] == '\0')
	{
		SEND("Syntax: create <item type> <amount> <material> optional:<level>\r\n", ch);
		return;
	}
	
	if (!str_cmp(arg1, "weapon") || !str_cmp(arg1, "armor") || !str_cmp(arg1, "gem") || !str_cmp(arg1, "light"))
	{
		if (arg2[0] == '\0')
		{
			SEND("Need an amount.\r\n",ch);
			return;
		}
		if (arg3[0] == '\0')
		{
			SEND("What material will it/they be?\r\n",ch);
			return;
		}		
		if (!valid_material(arg3))
		{
			SEND("Not a valid material.\r\n",ch);
			return;
		}
		
		amount = atoi(arg2);		

		if (amount > 1)
		{
			while (amount-- >= 0)
			{
				create_random_obj(ch, NULL, NULL, arg4[0] != '\0' ? atoi(arg4) : number_range(1, MAX_LEVEL), arg1, arg3, TARG_MOB, "");
			}
			SEND("You create multiple objects.\r\n",ch);
		}
		else
		{
			create_random_obj(ch, NULL, NULL, arg4[0] != '\0' ? atoi(arg4) : number_range(1, MAX_LEVEL), arg1, arg3, TARG_MOB, "");
			SEND("You create an object.\r\n",ch);
		}	
		
		return;
	}	
	else if (!str_cmp(arg1, "money"))
	{
		if  (arg2[0] == '\0')
		{
			SEND("Need an amount.\r\n",ch);
			return;
		}
		amount = atoi(arg2);		
		
		if (amount < 1)
		{
			SEND ("You can't create nothing.\r\n",ch);
			return;
		}
		
		if (amount > 0)		
			create_random_obj(ch, NULL, NULL, 1, arg1, "", TARG_MOB, "");
		
		return;
			
	}
	else if (!str_cmp(arg1, "figurine"))
	{
		if  (arg2[0] == '\0')
		{
			SEND("Need an amount.\r\n",ch);
			return;
		}
		if (arg3[0] == '\0')
		{
			SEND("What material will it/they be?\r\n",ch);
			return;
		}		
		if (!valid_material(arg3))
		{
			SEND("Not a valid material.\r\n",ch);
			return;
		}
		
		amount = atoi(arg2);		
		
		if (amount > 1)
		{
			while (amount-- >= 0)
			{
				create_random_obj(ch, NULL, NULL, number_range(1, MAX_LEVEL), arg1, arg3, TARG_MOB, "");
			}
			SEND("You create multiple objects.\r\n",ch);
		}
		else
		{
			create_random_obj(ch, NULL, NULL, number_range(1, MAX_LEVEL), arg1, arg3, TARG_MOB, "");
			SEND("You create an object.\r\n",ch);
		}	
		
		return;
	}
	else if	(!str_cmp(arg1, "tree"))	
	{
		if (arg2[0] == '\0')
		{
			SEND("Need an amount.\r\n",ch);
			return;
		}
		amount = atoi(arg2);		

		if (amount > 1)
		{
			while (amount-- >= 0)
			{
				create_random_obj(NULL, NULL, ch->in_room, number_range(1, MAX_LEVEL), arg1, "", TARG_ROOM, "");
			}
			SEND("You create multiple objects.\r\n",ch);
		}
		else
		{
			create_random_obj(NULL, NULL, ch->in_room, number_range(1, MAX_LEVEL), arg1, "", TARG_ROOM, "");
			SEND("You create an object.\r\n",ch);
		}	
			
		return;
		
	}
	else
	{
		SEND("Not a valid item type. (armor/weapon/light/tree/figurine/gem/money)\r\n",ch);
		return;
	}
	
	return;
}



void do_at (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;

    argument = one_argument (argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        SEND ("At where what?\r\n", ch);
        return;
    }

    if ((location = find_location (ch, arg)) == NULL)
    {
        SEND ("No such location.\r\n", ch);
        return;
    }

    if (!is_room_owner (ch, location) && room_is_private (location)
        && get_trust (ch) < MAX_LEVEL)
    {
        SEND ("That room is private right now.\r\n", ch);
        return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room (ch);
    char_to_room (ch, location);
    interpret (ch, argument);

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for (wch = char_list; wch != NULL; wch = wch->next)
    {
        if (wch == ch)
        {
            char_from_room (ch);
            char_to_room (ch, original);
            ch->on = on;
            break;
        }
    }

    return;
}


bool check_at_all( CHAR_DATA *ch, char *argument )
{
    char arg [MIL];
	char arg2[MIL];
    
    argument = one_argument( argument, arg );
    one_argument(argument, arg2);

    if (!str_prefix( arg, "restore" ) ||  !str_prefix( arg, "goto" ) ||  !str_prefix( arg, "slay" ) ||  !str_prefix( arg, "atall" ))
		return FALSE;

    return TRUE;
}

void do_at_all( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch, *wch_next, *vch, *vch_next;

	bool legal;

    if ( argument[0] == '\0' )
    {
	SEND( "At everyone do what?\r\n", ch );
	return;
    }

	legal = check_at_all(ch, argument);

    if (!legal)
    {
		SEND("That's not allowed.\r\n",ch);
		return;
    }


    original = ch->in_room;
    on = ch->on;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
		wch_next = wch->next;

		if ( !ch )
			break;

		if ( /*wch == ch ||*/ wch->in_room == NULL )
			continue;

		if ( IS_NPC( wch ))
			continue;

		if ( ( location = get_room_index( wch->in_room->vnum )) == NULL)
			continue;

		if (!is_room_owner(ch,location) && room_is_private( location ) &&  get_trust(ch) < MAX_LEVEL)
			continue;

		legal = FALSE;

		for ( vch = location->people; vch != NULL; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			//if ( /*ch != vch &&*/ !IS_NPC(vch) )
			if ( ch != vch && !IS_NPC(vch) && (!strcasecmp( vch->name, "Upro")))
				legal = TRUE;
		}

		if ( legal )
			continue;

		char_from_room( ch );
		char_to_room( ch, location );
		sprintf( buf, "%s %s", argument, wch->name );
		interpret( ch, buf );
    }
    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
		if ( wch == ch )
		{
			char_from_room( ch );
			char_to_room( ch, original );
			ch->on = on;
			break;
		}
    }
    return;
}



void do_goto (CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if (argument[0] == '\0')
    {
        SEND ("Goto where?\r\n", ch);
        return;
    }

    if ((location = find_location (ch, argument)) == NULL)
    {
        SEND ("No such location.\r\n", ch);
        return;
    }

    count = 0;
    for (rch = location->people; rch != NULL; rch = rch->next_in_room)
        count++;

    if (!is_room_owner (ch, location) && room_is_private (location)
        && (count > 1 || get_trust (ch) < MAX_LEVEL))
    {
        SEND ("That room is private right now.\r\n", ch);
        return;
    }

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust (rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
            else
                act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
        }
    }

    char_from_room (ch);
    char_to_room (ch, location);


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust (rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
            else
                act ("$n appears in a swirling mist.", ch, NULL, rch,
                     TO_VICT);
        }
    }

    do_function (ch, &do_look, "auto");
    return;
}

void do_violate (CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

    if (argument[0] == '\0')
    {
        SEND ("Goto where?\r\n", ch);
        return;
    }

    if ((location = find_location (ch, argument)) == NULL)
    {
        SEND ("No such location.\r\n", ch);
        return;
    }

    if (!room_is_private (location))
    {
        SEND ("That room isn't private, use goto.\r\n", ch);
        return;
    }

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust (rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
            else
                act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
        }
    }

    char_from_room (ch);
    char_to_room (ch, location);


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust (rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
            else
                act ("$n appears in a swirling mist.", ch, NULL, rch,
                     TO_VICT);
        }
    }

    do_function (ch, &do_look, "auto");
    return;
}

/* RT to replace the 3 stat commands */

void do_stat (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    string = one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  stat <name>\r\n", ch);
        SEND ("  stat obj <name>\r\n", ch);
        SEND ("  stat mob <name>\r\n", ch);
        SEND ("  stat room <number>\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "room"))
    {
        do_function (ch, &do_rstat, string);
        return;
    }

    if (!str_cmp (arg, "obj"))
    {
        do_function (ch, &do_ostat, string);
        return;
    }

    if (!str_cmp (arg, "char") || !str_cmp (arg, "mob"))
    {
        do_function (ch, &do_mstat, string);
        return;
    }

    /* do it the old way */

    obj = get_obj_world (ch, argument);
    if (obj != NULL)
    {
        do_function (ch, &do_ostat, argument);
        return;
    }

    victim = get_char_world (ch, argument);
    if (victim != NULL)
    {
        do_function (ch, &do_mstat, argument);
        return;
    }

    location = find_location (ch, argument);
    if (location != NULL)
    {
        do_function (ch, &do_rstat, argument);
        return;
    }

    SEND ("Nothing by that name found anywhere.\r\n", ch);
}

void do_rstat (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument (argument, arg);
    location = (arg[0] == '\0') ? ch->in_room : find_location (ch, arg);
    if (location == NULL)
    {
        SEND ("No such location.\r\n", ch);
        return;
    }

    if (!is_room_owner (ch, location) && ch->in_room != location
        && room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        SEND ("That room is private right now.\r\n", ch);
        return;
    }

    sprintf (buf, "Name: '%s'\r\nArea: '%s'\r\n",
             location->name, location->area->name);
    SEND (buf, ch);

    sprintf (buf,
             "Vnum: %ld  Sector: %d  Light: %d  Healing: %d  Mana: %d\r\n",
             location->vnum,
             location->sector_type,
             location->light, location->heal_rate, location->mana_rate);
    SEND (buf, ch);

    sprintf (buf,
             "Room flags: %d.\r\nDescription:\r\n%s",
             location->room_flags, location->description);
    SEND (buf, ch);

    if (location->extra_descr != NULL)
    {
        EXTRA_DESCR_DATA *ed;

        SEND ("Extra description keywords: '", ch);
        for (ed = location->extra_descr; ed; ed = ed->next)
        {
            SEND (ed->keyword, ch);
            if (ed->next != NULL)
                SEND (" ", ch);
        }
        SEND ("'.\r\n", ch);
    }

    SEND ("Characters:", ch);
    for (rch = location->people; rch; rch = rch->next_in_room)
    {
        if (can_see (ch, rch))
        {
            SEND (" ", ch);
            one_argument (rch->name, buf);
            SEND (buf, ch);
        }
    }

    SEND (".\r\nObjects:   ", ch);
    for (obj = location->contents; obj; obj = obj->next_content)
    {
        SEND (" ", ch);
        one_argument (obj->name, buf);
        SEND (buf, ch);
    }
    SEND (".\r\n", ch);

    for (door = 0; door <= 5; door++)
    {
        EXIT_DATA *pexit;

        if ((pexit = location->exit[door]) != NULL)
        {
            sprintf (buf,
                     "Door: %d.  To: %ld.  Key: %d.  Exit flags: %d.\r\nKeyword: '%s'.  Description: %s",
                     door,
                     (pexit->u1.to_room ==
                      NULL ? -1 : pexit->u1.to_room->vnum), pexit->key,
                     pexit->exit_info, pexit->keyword,
                     pexit->description[0] !=
                     '\0' ? pexit->description : "(none).\r\n");
            SEND (buf, ch);
        }
    }

    return;
}



void do_ostat (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Stat what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_world (ch, arg)) == NULL)
    {
        SEND ("Nothing like that in hell, earth, or heaven.\r\n", ch);
        return;
    }

    sprintf (buf, "Name(s): %s\r\n", obj->name);
    SEND (buf, ch);

	sprintf (buf, "Material: %s\r\n", obj->material);
	SEND (buf, ch);
	
    sprintf (buf, "Vnum: %ld  Format: %s  Type: %s  Resets: %d\r\n",
             obj->pIndexData->vnum,
             obj->pIndexData->new_format ? "new" : "old",
             item_name (obj->item_type), obj->pIndexData->reset_num);
    SEND (buf, ch);

    sprintf (buf, "Short description: %s\r\nLong description: %s\r\n",
             obj->short_descr, obj->description);
    SEND (buf, ch);

    sprintf (buf, "Wear bits: %s\r\nExtra bits: %s\r\nExtra2 bits: %s\r\n",
             wear_bit_name (obj->wear_flags),
             extra_bit_name (obj->extra_flags),
			 extra2_bit_name (obj->extra2_flags));
    SEND (buf, ch);

    sprintf (buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\r\n",
             1, get_obj_number (obj),
             obj->weight, get_obj_weight (obj), get_true_weight (obj));
    SEND (buf, ch);

    sprintf (buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\r\n",
             obj->level, obj->cost, obj->condition, obj->timer);
    SEND (buf, ch);
	
	sprintf (buf, "Altitude %d%s\r\n", obj->altitude, (obj->altitude < 0 ? " (buried)" : ""));
	SEND(buf, ch);

    sprintf (buf,
             "In room: %ld  In object: %s  Carried by: %s  Wear_loc: %d\r\n",
             obj->in_room == NULL ? 0 : obj->in_room->vnum,
             obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
             obj->carried_by == NULL ? "(none)" :
             can_see (ch, obj->carried_by) ? obj->carried_by->name
             : "someone", obj->wear_loc);
    SEND (buf, ch);

    sprintf (buf, "Values: %d %d %d %d %d\r\n",
             obj->value[0], obj->value[1], obj->value[2], obj->value[3],
             obj->value[4]);
    SEND (buf, ch);

    /* now give out vital statistics as per identify */

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
            sprintf (buf, "Has %d(%d) charges of level %d",
                     obj->value[1], obj->value[2], obj->value[0]);
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


        case ITEM_WEAPON:
            SEND ("Weapon type is ", ch);
            switch (obj->value[0])
            {
                case (WEAPON_EXOTIC):
                    SEND ("exotic\r\n", ch);
                    break;
                case (WEAPON_SHORT_SWORD):
                    SEND ("short sword\r\n", ch);
                    break;
				case (WEAPON_LONG_SWORD):
                    SEND ("long sword\r\n", ch);
                    break;
				case (WEAPON_BOW):
                    SEND ("bow\r\n", ch);
                    break;
				case (WEAPON_CROSSBOW):
                    SEND ("crossbow\r\n", ch);
                    break;
				case (WEAPON_STAFF):
                    SEND ("staff\r\n", ch);
                    break;
                case (WEAPON_DAGGER):
                    SEND ("dagger\r\n", ch);
                    break;
                case (WEAPON_SPEAR):
                    SEND ("spear\r\n", ch);
                    break;
                case (WEAPON_MACE):
                    SEND ("mace/club\r\n", ch);
                    break;
                case (WEAPON_AXE):
                    SEND ("axe\r\n", ch);
                    break;
                case (WEAPON_FLAIL):
                    SEND ("flail\r\n", ch);
                    break;
                case (WEAPON_WHIP):
                    SEND ("whip\r\n", ch);
                    break;
                case (WEAPON_POLEARM):
                    SEND ("polearm\r\n", ch);
                    break;
                default:
                    SEND ("unknown\r\n", ch);
                    break;
            }
            if (obj->pIndexData->new_format)
                sprintf (buf, "Damage is %dd%d (average %d)\r\n",
                         obj->value[1], obj->value[2],
                         (1 + obj->value[2]) * obj->value[1] / 2);
            else
                sprintf (buf, "Damage is %d to %d (average %d)\r\n",
                         obj->value[1], obj->value[2],
                         (obj->value[1] + obj->value[2]) / 2);
            SEND (buf, ch);

            sprintf (buf, "Damage noun is %s.\r\n",
                     (obj->value[3] > 0
                      && obj->value[3] <
                      MAX_DAMAGE_MESSAGE) ? attack_table[obj->value[3]].noun :
                     "undefined");
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
                     "Armorch_class is %d pierce, %d bash, %d slash, and %d vs. magic\r\nBulk: %d\r\n",
                     obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);
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
    }


    if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
        EXTRA_DESCR_DATA *ed;

        SEND ("Extra description keywords: '", ch);

        for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
        {
            SEND (ed->keyword, ch);
            if (ed->next != NULL)
                SEND (" ", ch);
        }

        for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next)
        {
            SEND (ed->keyword, ch);
            if (ed->next != NULL)
                SEND (" ", ch);
        }

        SEND ("'\r\n", ch);
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        sprintf (buf, "Affects %s by %d, level %d",
                 affect_loc_name (paf->location), paf->modifier, paf->level);
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
                case TO_WEAPON:
                    sprintf (buf, "Adds %s weapon flags.\n",
                             weapon_bit_name (paf->bitvector));
                    break;
                case TO_OBJECT:
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

    if (!obj->enchanted)
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
        {
            sprintf (buf, "Affects %s by %d, level %d.\r\n",
                     affect_loc_name (paf->location), paf->modifier,
                     paf->level);
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

    return;
}



void do_mstat (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Stat whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, argument)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

	sprintf (buf,"\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{r|{x Name    %34s {r|{x\r\n{r|{x Short    %33s {r|{x\r\n{r|{x Long    %34s {r|{x\r\n",
	victim->name,
	victim->short_descr[0] != '\0' ? victim->short_descr : "(none)",
    victim->long_descr[0] != '\0' ? victim->long_descr : "(none)"
			);

	SEND(buf, ch);    

    sprintf (buf,
             "\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{r|{x Vnum  %6ld {r|{x Level  %6d {r|{x Format %6s {r|\r\n|{x Room  %6ld {r|{x  Group %6d {r|{x Race %s {r|\r\n|{x Sex: %s  \r\n",
             IS_NPC (victim) ? victim->pIndexData->vnum : 0,
             IS_NPC (victim) ? victim->level : IS_MCLASSED(victim) ? total_levels(victim) : total_levels(victim),
			 IS_NPC (victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",             
             victim->in_room == NULL ? 0 : victim->in_room->vnum,
			 IS_NPC (victim) ? victim->group : 0,
			 race_table[victim->race].name,
			 sex_table[victim->sex].name);             
    SEND (buf, ch);

	SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n", ch);
	
	if (!IS_NPC(victim) && victim->onBoard)
	{
		sprintf (buf, "Ship location: %ld\r\n", victim->ship->in_room->vnum);
		SEND(buf, ch);
	}
	
    if (IS_NPC (victim))
    {
        sprintf (buf, "Count: %d  Killed: %d\r\n",
                 victim->pIndexData->count, victim->pIndexData->killed);
        SEND (buf, ch);
    }

    sprintf (buf,
             "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)  Cha: %d(%d)\r\n",
             victim->perm_stat[STAT_STR],
             get_curr_stat (victim, STAT_STR),
             victim->perm_stat[STAT_INT],
             get_curr_stat (victim, STAT_INT),
             victim->perm_stat[STAT_WIS],
             get_curr_stat (victim, STAT_WIS),
             victim->perm_stat[STAT_DEX],
             get_curr_stat (victim, STAT_DEX),
             victim->perm_stat[STAT_CON], 
			 get_curr_stat (victim, STAT_CON),
			 victim->perm_stat[STAT_CHA], 
			 get_curr_stat (victim, STAT_CHA)
			 );
    SEND (buf, ch);

    sprintf (buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\r\n",
             victim->hit, victim->max_hit,
             victim->mana, victim->max_mana,
             victim->move, victim->max_move,             
			 IS_NPC(victim) ? 0 : victim->practice ); 
    SEND (buf, ch);
	
	if (!IS_NPC(victim))
	{
		sprintf(buf, "Bulk: %d\r\n", victim->bulk);
		SEND(buf, ch);
	}
	
	if (!IS_NPC(victim))
	{
		if (!IS_MCLASSED(victim))
		{ 
			sprintf(buf, "Level: %d ch_class: {g%s{x  ", victim->level,ch_class_table[victim->ch_class].name);
			SEND(buf, ch);
		}
		else
		{
			sprintf(buf, "Level: %d / %d ch_class: {g%s {x/{g %s{x  ", victim->level, victim->level2,ch_class_table[victim->ch_class].name,ch_class_table[victim->ch_class2].name);
			SEND(buf, ch);
		}
	}
	else
	{
		sprintf(buf, "Level: %d ch_class: {g%s{x  ", victim->level, "mobile");
		SEND(buf, ch);
	}
	
    sprintf (buf,
             "Align: {g%d{x  Gold: {y%ld{x  Silver: {D%ld{x  Exp: {g%ld{x\r\n",
             victim->alignment, victim->gold, victim->silver, victim->exp);
    SEND (buf, ch);
	if (!IS_NPC(victim))
	{
		sprintf (buf,
				"Guildpoints: %d ",
				victim->guildpoints);				
		SEND (buf, ch);
	}
	
    sprintf (buf, "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\r\n",
             GET_AC (victim, AC_PIERCE), GET_AC (victim, AC_BASH),
             GET_AC (victim, AC_SLASH), GET_AC (victim, AC_EXOTIC));
    SEND (buf, ch);

    sprintf (buf,
             "Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\r\n",
             GET_HITROLL (victim), GET_DAMROLL (victim), victim->saving_throw,
             size_table[victim->size].name,
             position_table[victim->position].name, victim->wimpy);
    SEND (buf, ch);

	if (!IS_NPC(ch))
	{
		sprintf (buf, "PKills: %d    PDeaths: %d    MKills: %d\r\n", victim->pkill, victim->pdeath, victim->mkill);
		SEND(buf,ch);
	}
	
    if (IS_NPC (victim) && victim->pIndexData->new_format)
    {
        sprintf (buf, "Damage: %dd%d  Message:  %s\r\n",
                 victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
                 attack_table[victim->dam_type].noun);
        SEND (buf, ch);
    }
    sprintf (buf, "Fighting: %s\r\n",
             victim->fighting ? victim->fighting->name : "(none)");
    SEND (buf, ch);

    if (!IS_NPC (victim))
    {
        sprintf (buf,
                 "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\r\n",
                 victim->pcdata->condition[COND_THIRST],
                 victim->pcdata->condition[COND_HUNGER],
                 victim->pcdata->condition[COND_FULL],
                 victim->pcdata->condition[COND_DRUNK]);
        SEND (buf, ch);
    }

	if (victim->bleeding > 0)
	{
		SEND ("They're bleeding.\r\n",ch);
	}
	
	
    sprintf (buf, "Carry number: %d  Carry weight: %ld\r\n",
             victim->carry_number, get_carry_weight (victim) / 10);
    SEND (buf, ch);


    if (!IS_NPC (victim))
    {
        sprintf (buf,
                 "Age: %d  Played: %d  Last Level: %d  Timer: %d\r\n",
                 get_age (victim),
                 (int) (victim->played + current_time - victim->logon) / 3600,
                 victim->pcdata->last_level, victim->timer);
        SEND (buf, ch);
    }

    sprintf (buf, "Act: %s\r\n", act_bit_name (victim->act));
	SEND (buf, ch);
	if (IS_NPC(victim))	
	{
		sprintf (buf, "Act2: %s\r\n", act2_bit_name (victim->act2));
		SEND (buf, ch);
	}
	
    if (victim->comm)
    {
        sprintf (buf, "Comm: %s\r\n", comm_bit_name (victim->comm));
        SEND (buf, ch);
    }

    if (IS_NPC (victim) && victim->off_flags)
    {
        sprintf (buf, "Offense: %s\r\n", off_bit_name (victim->off_flags));
        SEND (buf, ch);
    }

    if (victim->imm_flags)
    {
        sprintf (buf, "Immune: %s\r\n", imm_bit_name (victim->imm_flags));
        SEND (buf, ch);
    }

    if (victim->res_flags)
    {
        sprintf (buf, "Resist: %s\r\n", imm_bit_name (victim->res_flags));
        SEND (buf, ch);
    }

    if (victim->vuln_flags)
    {
        sprintf (buf, "Vulnerable: %s\r\n",
                 imm_bit_name (victim->vuln_flags));
        SEND (buf, ch);
    }

    sprintf (buf, "Form: %s\r\nParts: %s\r\n",
             form_bit_name (victim->form), part_bit_name (victim->parts));
    SEND (buf, ch);

    if (victim->affected_by)
    {
        sprintf (buf, "Affected by %s\r\n",
                 affect_bit_name (victim->affected_by));
        SEND (buf, ch);
    }

    sprintf (buf, "Master: %s  Leader: %s  Pet: %s\r\n",
             victim->master ? victim->master->name : "(none)",
             victim->leader ? victim->leader->name : "(none)",
             victim->pet ? victim->pet->name : "(none)");
    SEND (buf, ch);

    if (!IS_NPC (victim))
    {
        sprintf (buf, "Security: %d.\r\n", victim->pcdata->security);    /* OLC */
        SEND (buf, ch);    /* OLC */
    }

    sprintf (buf, "Short description: %s\r\nLong  description: %s",
             victim->short_descr,
             victim->long_descr[0] !=
             '\0' ? victim->long_descr : "(none)\r\n");
    SEND (buf, ch);

    if (IS_NPC (victim) && victim->spec_fun != 0)
    {
        sprintf (buf, "Mobile has special procedure %s.\r\n",
                 spec_name (victim->spec_fun));
        SEND (buf, ch);
    }

    for (paf = victim->affected; paf != NULL; paf = paf->next)
    {
        sprintf (buf,
                 "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\r\n",
                 skill_table[(int) paf->type].name,
                 affect_loc_name (paf->location),
                 paf->modifier,
                 paf->duration, affect_bit_name (paf->bitvector), paf->level);
        SEND (buf, ch);
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  vnum obj <name>\r\n", ch);
        SEND ("  vnum mob <name>\r\n", ch);
        SEND ("  vnum skill <skill or spell>\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "obj"))
    {
        do_function (ch, &do_ofind, string);
        return;
    }

    if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
    {
        do_function (ch, &do_mfind, string);
        return;
    }

    if (!str_cmp (arg, "skill") || !str_cmp (arg, "spell"))
    {
        do_function (ch, &do_slookup, string);
        return;
    }
    /* do both */
    do_function (ch, &do_mfind, argument);
    do_function (ch, &do_ofind, argument);
}


void do_mfind (CHAR_DATA * ch, char *argument)
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Find whom?\r\n", ch);
        return;
    }

    fAll = FALSE;                /* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_mob_index; vnum++)
    {
        if ((pMobIndex = get_mob_index (vnum)) != NULL)
        {
            nMatch++;
            if (fAll || is_name (argument, pMobIndex->player_name))
            {
                found = TRUE;
                sprintf (buf, "{r[{x%5ld{r]{x MOB %s\r\n",
                         pMobIndex->vnum, pMobIndex->short_descr);
                SEND (buf, ch);
            }
        }
    }

    if (!found)
        SEND ("No mobiles by that name.\r\n", ch);

    return;
}



void do_ofind (CHAR_DATA * ch, char *argument)
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Find what?\r\n", ch);
        return;
    }

    fAll = FALSE;                /* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for (vnum = 0; nMatch < top_obj_index; vnum++)
    {
        if ((pObjIndex = get_obj_index (vnum)) != NULL)
        {
            nMatch++;
            if (fAll || is_name (argument, pObjIndex->name))
            {
                found = TRUE;
                sprintf (buf, "{r[{x%5ld{r]{x OBJ %s\r\n",
                         pObjIndex->vnum, pObjIndex->short_descr);
                SEND (buf, ch);
            }
        }
    }

    if (!found)
        SEND ("No objects by that name.\r\n", ch);

    return;
}


void do_owhere (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf ();

    if (argument[0] == '\0')
    {
        SEND ("Find what?\r\n", ch);
        return;
    }

    for (obj = object_list; obj != NULL; obj = obj->next)
    {
        if (!can_see_obj (ch, obj) || !is_name (argument, obj->name)
            || ch->level < obj->level)
            continue;

        found = TRUE;
        number++;

        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by)
            && in_obj->carried_by->in_room != NULL)
            sprintf (buf, "%3d) {b[{xRoom %ld{b]{x {d({x%ld{d){x %s is carried by %s\r\n",
                     number, in_obj->carried_by->in_room->vnum, obj->pIndexData->vnum, obj->short_descr, PERS (in_obj->carried_by, ch));
						
        else if (in_obj->in_room != NULL
                 && can_see_room (ch, in_obj->in_room)) sprintf (buf,
                                                                 "%3d) {b[{xRoom %ld{b]{x {d({x%ld{d){x %s is in %s\r\n",
                                                                 number,
																 in_obj->in_room->vnum, obj->pIndexData->vnum,
                                                                 obj->short_descr,
                                                                 in_obj->in_room->name);
        else
            sprintf (buf, "%3d) %s is somewhere\r\n", number,
                     obj->short_descr);

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
}

void do_rwhere (CHAR_DATA * ch, char *argument)
{

	char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoomIndex;
    bool found;
    int count = 0;
	int vnum = 0;

    if (argument[0] == '\0')
    {
		SEND("What is the name of the room you are searching for?\r\n",ch);
		return;
    }

    found = FALSE;
    buffer = new_buf ();
	pArea = ch->in_room->area;
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
	{
		if ((pRoomIndex = get_room_index (vnum)) != NULL)
		{			
			if (!str_cmp(argument, pRoomIndex->name))
			{
				found = TRUE;
				count++;
				sprintf (buf, "%3d) [%6ld] %-28s\r\n", 
					count,
					pRoomIndex->vnum,
					pRoomIndex->name);
				add_buf (buffer, buf);
			}			
		}
	}
    

    if (!found)
        act ("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
    else
        page_to_char (buf_string (buffer), ch);

    free_buf (buffer);

	return;
}

void do_mwhere (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if (argument[0] == '\0')
    {
        DESCRIPTOR_DATA *d;

        /* show characters logged */

        buffer = new_buf ();
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->character != NULL && d->connected == CON_PLAYING
                && d->character->in_room != NULL && can_see (ch, d->character)
                && can_see_room (ch, d->character->in_room))
            {
                victim = d->character;
                count++;
                if (d->original != NULL)
                    sprintf (buf,
                             "%3d) %s (in the body of %s) is in %s [%ld]\r\n",
                             count, d->original->name, victim->short_descr,
                             victim->in_room->name, victim->in_room->vnum);
                else
                    sprintf (buf, "%3d) %s is in %s [%ld]\r\n", count,
                             victim->name, victim->in_room->name,
                             victim->in_room->vnum);
                add_buf (buffer, buf);
            }
        }

        page_to_char (buf_string (buffer), ch);
        free_buf (buffer);
        return;
    }

    found = FALSE;
    buffer = new_buf ();
    for (victim = char_list; victim != NULL; victim = victim->next)
    {
        if (victim->in_room != NULL && is_name (argument, victim->name))
        {
            found = TRUE;
            count++;
            sprintf (buf, "%3d) [%5ld] %-28s [%5ld] %s\r\n", count,
                     IS_NPC (victim) ? victim->pIndexData->vnum : 0,
                     IS_NPC (victim) ? victim->short_descr : victim->name,
                     victim->in_room->vnum, victim->in_room->name);
            add_buf (buffer, buf);
        }
    }

    if (!found)
        act ("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
    else
        page_to_char (buf_string (buffer), ch);

    free_buf (buffer);

    return;
}



void do_reboo (CHAR_DATA * ch, char *argument)
{
    SEND ("If you want to REBOOT, spell it out.\r\n", ch);
    return;
}



void do_reboot (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    {
        sprintf (buf, "Reboot by %s.", ch->name);
        do_function (ch, &do_echo, buf);
    }

    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next)
    {
        d_next = d->next;
        vch = d->original ? d->original : d->character;
        if (vch != NULL)
            save_char_obj (vch);
        close_socket (d);
    }

    return;
}

void do_shutdow (CHAR_DATA * ch, char *argument)
{
    SEND ("If you want to SHUTDOWN, spell it out.\r\n", ch);
    return;
}

void do_shutdown (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
        sprintf (buf, "Shutdown by %s.", ch->name);
    append_file (ch, SHUTDOWN_FILE, buf);
    strcat (buf, "\r\n");
    if (ch->invis_level < LEVEL_HERO)
    {
        do_function (ch, &do_echo, buf);
    }
    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next)
    {
        d_next = d->next;
        vch = d->original ? d->original : d->character;
        if (vch != NULL)
            save_char_obj (vch);
        close_socket (d);
    }
    return;
}

void do_protect (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
        SEND ("Protect whom from snooping?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, argument)) == NULL)
    {
        SEND ("You can't find them.\r\n", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_SNOOP_PROOF))
    {
        act_new ("$N is no longer snoop-proof.", ch, NULL, victim, TO_CHAR,
                 POS_DEAD);
        SEND ("Your snoop-proofing was just removed.\r\n", victim);
        REMOVE_BIT (victim->comm, COMM_SNOOP_PROOF);
    }
    else
    {
        act_new ("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
                 POS_DEAD);
        SEND ("You are now immune to snooping.\r\n", victim);
        SET_BIT (victim->comm, COMM_SNOOP_PROOF);
    }
}



void do_snoop (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Snoop whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->desc == NULL)
    {
        SEND ("No descriptor to snoop.\r\n", ch);
        return;
    }

    if (victim == ch)
    {
        SEND ("Cancelling all snoops.\r\n", ch);
        wiznet ("$N stops being such a snoop.",
                ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust (ch));
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->snoop_by == ch->desc)
                d->snoop_by = NULL;
        }
        return;
    }

    if (victim->desc->snoop_by != NULL)
    {
        SEND ("Busy already.\r\n", ch);
        return;
    }

    if (!is_room_owner (ch, victim->in_room) && ch->in_room != victim->in_room
        && room_is_private (victim->in_room) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        SEND ("That character is in a private room.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch)
        || IS_SET (victim->comm, COMM_SNOOP_PROOF))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    if (ch->desc != NULL)
    {
        for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
        {
            if (d->character == victim || d->original == victim)
            {
                SEND ("No snoop loops.\r\n", ch);
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    sprintf (buf, "$N starts snooping on %s",
             (IS_NPC (ch) ? victim->short_descr : victim->name));
    wiznet (buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust (ch));
    SEND ("Ok.\r\n", ch);
    return;
}



void do_switch (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Switch into whom?\r\n", ch);
        return;
    }

    if (ch->desc == NULL)
        return;

    if (ch->desc->original != NULL)
    {
        SEND ("You are already switched.\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim == ch)
    {
        SEND ("Ok.\r\n", ch);
        return;
    }

    if (!IS_NPC (victim))
    {
        SEND ("You can only switch into mobiles.\r\n", ch);
        return;
    }

    if (!is_room_owner (ch, victim->in_room) && ch->in_room != victim->in_room
        && room_is_private (victim->in_room) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        SEND ("That character is in a private room.\r\n", ch);
        return;
    }

    if (victim->desc != NULL)
    {
        SEND ("Character in use.\r\n", ch);
        return;
    }

    sprintf (buf, "$N switches into %s", victim->short_descr);
    wiznet (buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust (ch));

    ch->desc->character = victim;
    ch->desc->original = ch;
    victim->desc = ch->desc;
    ch->desc = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup (ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
	ch->switched = TRUE;
    SEND ("Ok.\r\n", victim);
    return;
}



void do_return (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->desc == NULL)
        return;

    if (ch->desc->original == NULL)
    {
        SEND ("You aren't switched.\r\n", ch);
        return;
    }

    SEND
        ("You return to your original body. Type replay to see any missed tells.\r\n",
         ch);
    if (ch->prompt != NULL)
    {
        free_string (ch->prompt);
        ch->prompt = NULL;
    }

    sprintf (buf, "$N returns from %s.", ch->short_descr);
    wiznet (buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
            get_trust (ch));
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    ch->desc->character->desc = ch->desc;
	ch->switched = FALSE;
    ch->desc = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA * ch, OBJ_DATA * obj)
{
    if (IS_TRUSTED (ch, GOD)
        || (IS_TRUSTED (ch, IMMORTAL) && obj->level <= 20
            && obj->cost <= 1000) || (IS_TRUSTED (ch, DEMI)
                                      && obj->level <= 10 && obj->cost <= 500)
        || (IS_TRUSTED (ch, ANGEL) && obj->level <= 5 && obj->cost <= 250)
        || (IS_TRUSTED (ch, AVATAR) && obj->level == 0 && obj->cost <= 100))
        return TRUE;
    else
        return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
        if (obj_check (ch, c_obj))
        {
            t_obj = create_object (c_obj->pIndexData, 0);
            clone_object (c_obj, t_obj);
            obj_to_obj (t_obj, clone);
            recursive_clone (ch, c_obj, t_obj);
        }
    }
}

/* command that is similar to load */
void do_clone (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA *obj;

    rest = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Clone what?\r\n", ch);
        return;
    }

    if (!str_prefix (arg, "object"))
    {
        mob = NULL;
        obj = get_obj_here( ch, NULL, rest);
        if (obj == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
    }
    else if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
    {
        obj = NULL;
        mob = get_char_room ( ch, NULL, rest);
        if (mob == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
    }
    else
    {                            /* find both */

        mob = get_char_room ( ch, NULL, argument);
        obj = get_obj_here( ch, NULL, argument);
        if (mob == NULL && obj == NULL)
        {
            SEND ("You don't see that here.\r\n", ch);
            return;
        }
    }

    /* clone an object */
    if (obj != NULL)
    {
        OBJ_DATA *clone;

        if (!obj_check (ch, obj))
        {
            SEND
                ("Your powers are not great enough for such a task.\r\n", ch);
            return;
        }

        clone = create_object (obj->pIndexData, 0);
        clone_object (obj, clone);
        if (obj->carried_by != NULL)
            obj_to_char (clone, ch);
        else
            obj_to_room (clone, ch->in_room);
        recursive_clone (ch, obj, clone);

        act ("$n has created $p.", ch, clone, NULL, TO_ROOM);
        act ("You clone $p.", ch, clone, NULL, TO_CHAR);
        wiznet ("$N clones $p.", ch, clone, WIZ_LOAD, WIZ_SECURE,
                get_trust (ch));
        return;
    }
    else if (mob != NULL)
    {
        CHAR_DATA *clone;
        OBJ_DATA *new_obj;
        char buf[MAX_STRING_LENGTH];

        if (!IS_NPC (mob))
        {
            SEND ("You can only clone mobiles.\r\n", ch);
            return;
        }

        if ((mob->level > 20 && !IS_TRUSTED (ch, GOD))
            || (mob->level > 10 && !IS_TRUSTED (ch, IMMORTAL))
            || (mob->level > 5 && !IS_TRUSTED (ch, DEMI))
            || (mob->level > 0 && !IS_TRUSTED (ch, ANGEL))
            || !IS_TRUSTED (ch, AVATAR))
        {
            SEND
                ("Your powers are not great enough for such a task.\r\n", ch);
            return;
        }

        clone = create_mobile (mob->pIndexData);
        clone_mobile (mob, clone);

        for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
        {
            if (obj_check (ch, obj))
            {
                new_obj = create_object (obj->pIndexData, 0);
                clone_object (obj, new_obj);
                recursive_clone (ch, obj, new_obj);
                obj_to_char (new_obj, clone);
                new_obj->wear_loc = obj->wear_loc;
            }
        }
        char_to_room (clone, ch->in_room);
        act ("$n has created $N.", ch, NULL, clone, TO_ROOM);
        act ("You clone $N.", ch, NULL, clone, TO_CHAR);
        sprintf (buf, "$N clones %s.", clone->short_descr);
        wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  load mob <vnum>\r\n", ch);
        SEND ("  load obj <vnum> <level>\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
    {
        do_function (ch, &do_mload, argument);
        return;
    }

    if (!str_cmp (arg, "obj"))
    {
        do_function (ch, &do_oload, argument);
        return;
    }
    /* echo syntax */
    do_function (ch, &do_load, "");
}


void do_mload (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument (argument, arg);

    if (arg[0] == '\0' || !is_number (arg))
    {
        SEND ("Syntax: load mob <vnum>.\r\n", ch);
        return;
    }

    if ((pMobIndex = get_mob_index (atoi (arg))) == NULL)
    {
        SEND ("No mob has that vnum.\r\n", ch);
        return;
    }

    victim = create_mobile (pMobIndex);
    char_to_room (victim, ch->in_room);
    act ("$n has created $N!", ch, NULL, victim, TO_ROOM);
    sprintf (buf, "$N loads %s.", victim->short_descr);
    wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
    SEND ("Ok.\r\n", ch);
    return;
}



void do_oload (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level = 0;
	char buf[MIL];

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || !is_number (arg1))
    {
        SEND ("Syntax: load obj <vnum> <level>.\r\n", ch);
        return;
    }

    if ((pObjIndex = get_obj_index (atoi (arg1))) == NULL)
    {
        SEND ("No object has that vnum.\r\n", ch);
        return;
    }

	level = pObjIndex->level;
	
	//level = get_trust (ch);        /* default */

    if (arg2[0] != '\0')
    {                            /* load with a level */
        if (!is_number (arg2))
        {
            SEND ("Syntax: oload <vnum> <level>.\r\n", ch);
            return;
        }
        level = atoi (arg2);
        if (level < 0 || level > get_trust(ch))
        {
            SEND ("Level must be be between 0 and your level.\r\n", ch);
            return;
        }
    }
	
	
	CHECK_UNIQUE(pObjIndex);
	
	
    obj = create_object (pObjIndex, level);	
	
    if (CAN_WEAR (obj, ITEM_TAKE))
        obj_to_char (obj, ch);
    else
        obj_to_room (obj, ch->in_room);
		
    act ("$n has created $p!", ch, obj, NULL, TO_ROOM);
    wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
    sprintf(buf, "You load up %s ({Bvnum{x: %ld) ({Blevel{x: %d)\r\n", obj->short_descr, pObjIndex->vnum, obj->level);
	SEND (buf, ch);
    return;
}



void do_purge (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA *obj_next;

        for (victim = ch->in_room->people; victim != NULL; victim = vnext)
        {
            vnext = victim->next_in_room;
            if (IS_NPC (victim) && !IS_SET (victim->act, ACT_NOPURGE)
                && victim != ch /* safety precaution */ )
                extract_char (victim, TRUE);
        }

        for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (!IS_OBJ_STAT (obj, ITEM_NOPURGE))
                extract_obj (obj);
        }

        act ("$n purges the room!", ch, NULL, NULL, TO_ROOM);
        SEND ("Ok.\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (!IS_NPC (victim))
    {

        if (ch == victim)
        {
            SEND ("Ho ho ho.\r\n", ch);
            return;
        }

        if (get_trust (ch) <= get_trust (victim))
        {
            SEND ("Maybe that wasn't a good idea...\r\n", ch);
            sprintf (buf, "%s tried to purge you!\r\n", ch->name);
            SEND (buf, victim);
            return;
        }

        act ("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);

        if (victim->level > 1)
            save_char_obj (victim);
        d = victim->desc;
        extract_char (victim, TRUE);
        if (d != NULL)
            close_socket (d);

        return;
    }

    act ("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
    extract_char (victim, TRUE);
    return;
}



void do_advance (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
    {
        SEND ("Syntax: advance <char> <level>.\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("That player is not here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    if ((level = atoi (arg2)) < 1 || level > MAX_LEVEL)
    {
        sprintf (buf, "Level must be 1 to %d.\r\n", MAX_LEVEL);
        SEND (buf, ch);
        return;
    }

    if (level > get_trust (ch))
    {
        SEND ("Limited to your trust level.\r\n", ch);
        return;
    }

	if (IS_MCLASSED(victim) && level > 25)
	{
		SEND ("Nope.\r\n",ch);
		return;
	}
	
	
	
    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if (level <= victim->level)
    {
        int temp_prac;

        SEND ("Lowering a player's level!\r\n", ch);
        SEND ("**** OOOOHHHHHHHHHH  NNNNOOOO ****\r\n", victim);
        temp_prac = victim->practice;
        victim->level = 1;
        victim->exp = exp_per_level (victim, FALSE);       
		victim->max_hit = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->hit = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move;
        advance_level (victim, TRUE, FALSE);
        victim->practice = temp_prac;
		
		if (IS_SET (victim->act, PLR_HOLYLIGHT) && level < LEVEL_IMMORTAL)
		{
			REMOVE_BIT (victim->act, PLR_HOLYLIGHT);
			SEND ("Holy light mode off.\r\n", ch);
		}
    }
    else
    {
        SEND ("Raising a player's level!\r\n", ch);
        SEND ("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\r\n", victim);
    }

    for (iLevel = victim->level; iLevel < level; iLevel++)
    {
        victim->level += 1;
        advance_level (victim, TRUE, FALSE);
    }
    sprintf (buf, "You are now level %d.\r\n", victim->level);
    SEND (buf, victim);
    victim->exp = exp_per_level (victim, FALSE);
    victim->trust = 0;
    save_char_obj (victim);
    return;
}



void do_trust (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
    {
        SEND ("Syntax: trust <char> <level>.\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("That player is not here.\r\n", ch);
        return;
    }

    if ((level = atoi (arg2)) < 0 || level > MAX_LEVEL)
    {
        sprintf (buf, "Level must be 0 (reset) or 1 to %d.\r\n", MAX_LEVEL);
        SEND (buf, ch);
        return;
    }

    if (level > get_trust (ch))
    {
        SEND ("Limited to your trust.\r\n", ch);
        return;
    }

    victim->trust = level;
    return;
}



void do_restore (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument (argument, arg);
    if (arg[0] == '\0' || !str_cmp (arg, "room"))
    {
        /* cure room */

        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip (vch, gsn_plague);
            affect_strip (vch, gsn_poison);
            affect_strip (vch, gsn_blindness);
            affect_strip (vch, gsn_sleep);
            affect_strip (vch, gsn_curse);

            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
			vch->pp = vch->max_pp;
			vch->bleeding = 0;
            update_pos (vch);
            act ("$n has restored you.", ch, NULL, vch, TO_VICT);
        }

        sprintf (buf, "$N restored room %ld.", ch->in_room->vnum);
        wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));

        SEND ("Room restored.\r\n", ch);
        return;

    }

    if (get_trust (ch) >= MAX_LEVEL - 1 && !str_cmp (arg, "all"))
    {
        /* cure all */

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            victim = d->character;

            if (victim == NULL || IS_NPC (victim))
                continue;

            affect_strip (victim, gsn_plague);
            affect_strip (victim, gsn_poison);
            affect_strip (victim, gsn_blindness);
            affect_strip (victim, gsn_sleep);
            affect_strip (victim, gsn_curse);

            victim->hit = victim->max_hit;
            victim->mana = victim->max_mana;
            victim->move = victim->max_move;
			victim->bleeding = 0;
            update_pos (victim);
            if (victim->in_room != NULL)
                act ("$n has restored you.", ch, NULL, victim, TO_VICT);
        }
        SEND ("All active players restored.\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    affect_strip (victim, gsn_plague);
    affect_strip (victim, gsn_poison);
    affect_strip (victim, gsn_blindness);
    affect_strip (victim, gsn_sleep);
    affect_strip (victim, gsn_curse);
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
	victim->bleeding = 0;
    update_pos (victim);
    act ("$n has restored you.", ch, NULL, victim, TO_VICT);
    sprintf (buf, "$N restored %s",
             IS_NPC (victim) ? victim->short_descr : victim->name);
    wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));
    SEND ("Ok.\r\n", ch);
    return;
}


void do_freeze (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Freeze whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    if (IS_SET (victim->act, PLR_FREEZE))
    {
        REMOVE_BIT (victim->act, PLR_FREEZE);
        SEND ("You can play again.\r\n", victim);
        SEND ("FREEZE removed.\r\n", ch);
        sprintf (buf, "$N thaws %s.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
    else
    {
        SET_BIT (victim->act, PLR_FREEZE);
        SEND ("You can't do ANYthing!\r\n", victim);
        SEND ("FREEZE set.\r\n", ch);
        sprintf (buf, "$N puts %s in the deep freeze.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    save_char_obj (victim);

    return;
}



void do_log (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Log whom?\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        if (fLogAll)
        {
            fLogAll = FALSE;
            SEND ("Log ALL off.\r\n", ch);
        }
        else
        {
            fLogAll = TRUE;
            SEND ("Log ALL on.\r\n", ch);
        }
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if (IS_SET (victim->act, PLR_LOG))
    {
        REMOVE_BIT (victim->act, PLR_LOG);
        SEND ("LOG removed.\r\n", ch);
    }
    else
    {
        SET_BIT (victim->act, PLR_LOG);
        SEND ("LOG set.\r\n", ch);
    }

    return;
}



void do_noemote (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Noemote whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }


    if (get_trust (victim) >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_NOEMOTE))
    {
        REMOVE_BIT (victim->comm, COMM_NOEMOTE);
        SEND ("You can emote again.\r\n", victim);
        SEND ("NOEMOTE removed.\r\n", ch);
        sprintf (buf, "$N restores emotes to %s.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
    else
    {
        SET_BIT (victim->comm, COMM_NOEMOTE);
        SEND ("You can't emote!\r\n", victim);
        SEND ("NOEMOTE set.\r\n", ch);
        sprintf (buf, "$N revokes %s's emotes.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}



void do_noshout (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Noshout whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_NOSHOUT))
    {
        REMOVE_BIT (victim->comm, COMM_NOSHOUT);
        SEND ("You can shout again.\r\n", victim);
        SEND ("NOSHOUT removed.\r\n", ch);
        sprintf (buf, "$N restores shouts to %s.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
    else
    {
        SET_BIT (victim->comm, COMM_NOSHOUT);
        SEND ("You can't shout!\r\n", victim);
        SEND ("NOSHOUT set.\r\n", ch);
        sprintf (buf, "$N revokes %s's shouts.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}



void do_notell (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Notell whom?", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (get_trust (victim) >= get_trust (ch))
    {
        SEND ("You failed.\r\n", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_NOTELL))
    {
        REMOVE_BIT (victim->comm, COMM_NOTELL);
        SEND ("You can tell again.\r\n", victim);
        SEND ("NOTELL removed.\r\n", ch);
        sprintf (buf, "$N restores tells to %s.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
    else
    {
        SET_BIT (victim->comm, COMM_NOTELL);
        SEND ("You can't tell!\r\n", victim);
        SEND ("NOTELL set.\r\n", ch);
        sprintf (buf, "$N revokes %s's tells.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}



void do_peace (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *rch;

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (rch->fighting != NULL)
            stop_fighting (rch, TRUE);
        if (IS_NPC (rch) && IS_SET (rch->act, ACT_AGGRESSIVE))
            REMOVE_BIT (rch->act, ACT_AGGRESSIVE);
    }

    SEND ("Ok.\r\n", ch);
    return;
}

void do_wizlock (CHAR_DATA * ch, char *argument)
{    
    wizlock = !wizlock;

    if (wizlock)
    {
        wiznet ("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
        SEND ("Game wizlocked.\r\n", ch);
    }
    else
    {
        wiznet ("$N removes wizlock.", ch, NULL, 0, 0, 0);
        SEND ("Game un-wizlocked.\r\n", ch);
    }

    return;
}

/* RT anti-newbie code */

void do_newlock (CHAR_DATA * ch, char *argument)
{
    newlock = !newlock;

    if (newlock)
    {
        wiznet ("$N locks out new characters.", ch, NULL, 0, 0, 0);
        SEND ("New characters have been locked out.\r\n", ch);
    }
    else
    {
        wiznet ("$N allows new characters back in.", ch, NULL, 0, 0, 0);
        SEND ("Newlock removed.\r\n", ch);
    }

    return;
}


void do_slookup (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Lookup which skill or spell?\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
            sprintf (buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\r\n",
                     sn, skill_table[sn].slot, skill_table[sn].name);
            SEND (buf, ch);
        }
    }
    else
    {
        if ((sn = skill_lookup (arg)) < 0)
        {
            SEND ("No such skill or spell.\r\n", ch);
            return;
        }

        sprintf (buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\r\n",
                 sn, skill_table[sn].slot, skill_table[sn].name);
        SEND (buf, ch);
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  set mob       <name> <field> <value>\r\n", ch);
        SEND ("  set character <name> <field> <value>\r\n", ch);
        SEND ("  set obj       <name> <field> <value>\r\n", ch);
        SEND ("  set room      <room> <field> <value>\r\n", ch);
        SEND ("  set skill     <name> <spell or skill> <value>\r\n", ch);
        return;
    }

    if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
    {
        do_function (ch, &do_mset, argument);
        return;
    }

    if (!str_prefix (arg, "skill") || !str_prefix (arg, "spell"))
    {
        do_function (ch, &do_sset, argument);
        return;
    }

    if (!str_prefix (arg, "object"))
    {
        do_function (ch, &do_oset, argument);
        return;
    }

    if (!str_prefix (arg, "room"))
    {
        do_function (ch, &do_rset, argument);
        return;
    }
    /* echo syntax */
    do_function (ch, &do_set, "");
}


void do_sset (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  set skill <name> <spell or skill> <value>\r\n", ch);
        SEND ("  set skill <name> all <value>\r\n", ch);
        SEND ("   (use the name of the skill, not the number)\r\n",
                      ch);
        return;
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        SEND ("Not on NPC's.\r\n", ch);
        return;
    }

    fAll = !str_cmp (arg2, "all");
    sn = 0;
    if (!fAll && (sn = skill_lookup (arg2)) < 0)
    {
        SEND ("No such skill or spell.\r\n", ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if (!is_number (arg3))
    {
        SEND ("Value must be numeric.\r\n", ch);
        return;
    }

    value = atoi (arg3);
    if (value < 0 || value > 100)
    {
        SEND ("Value range is 0 to 100.\r\n", ch);
        return;
    }

    if (fAll)
    {
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name != NULL)
                victim->pcdata->learned[sn] = value;
        }
    }
    else
    {
        victim->pcdata->learned[sn] = value;
    }

    return;
}


void do_mset (CHAR_DATA * ch, char *argument)
{
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde (argument);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  set char <name> <field> <value>\r\n", ch);
        SEND ("  Field being one of:\r\n", ch);
        SEND ("    str int wis dex con cha sex class class2 level level2 god\r\n", ch);
        SEND ("    race group gold silver hp mana move pp prac guildpoints\r\n", ch);
        SEND ("    align train thirst hunger drunk full\r\n", ch);
        SEND ("    security hours age mkills society talent faction\r\n", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }
    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number (arg3) ? atoi (arg3) : -1;

    /*
     * Set something.
     */	
	 
	if (!str_cmp (arg2, "mkill") || !str_cmp (arg2, "mkills"))
	{
		if (value < -50000 || value > 50000)
		{
			SEND ("Value must be between -50000 and 50000",ch);
			return;
		}
		
		victim->mkill = value;
		sprintf (buf, "%s has creditted you with %d mob kills.\r\n", ch->name, value);
		SEND (buf, ch);
		return;
	}
	
	if (!str_cmp (arg2, "faction"))
	{
		if (value < 0 || value > MAX_FACTION)
		{
			sprintf(buf, "Value must be between 0 and %d\r\n", MAX_FACTION);
			SEND (buf, ch);
			return;
		}
		
			victim->faction = value;
			sprintf (buf, "%s has set your faction to %s.\r\n", victim->name, faction_table[value].name);		
			SEND (buf, ch);	
			sprintf (buf, "You have set %s faction to %s.\r\n", ch->name, faction_table[value].name);
			SEND (buf, ch);
			return;
		
	}
	
	if (!str_cmp (arg2, "society"))
	{
		if (value < 0 || value > 23)
		{
			SEND ("Value must be between 0 and 23",ch);
			return;
		}
				
		victim->society_rank = value;		
		sprintf (buf, "%s has set your society rank to %s.\r\n", victim->name, society_table[value].male_name);		
		SEND (buf, ch);	
		sprintf (buf, "You have set %s society rank to %s.\r\n", ch->name, society_table[value].male_name);
		SEND (buf, ch);
		return;
	}
	
	if (!str_cmp (arg2, "talent"))
	{
		if (value < 0 || value > 1)
		{
			SEND ("Value must be between 0 and 1 (OFF / ON)",ch);
			return;
		}
		
		if (value == 0)		
		{
			sprintf (buf, "%s has taken your psionic talent.\r\n", ch->name);
			SEND (buf, victim);
			victim->has_talent = 0;
		}
		else
		{	
			sprintf (buf, "%s has given you psionic talent.\r\n", ch->name);
			SEND (buf, victim);
			victim->has_talent = 1;
		}
	
		return;
	}
	 
	if (!str_cmp (arg2, "age"))
	{
		if (value < -5000 || value > 5000)
		{
			SEND ("Age range must be -5000 to 5000.\r\n",ch);
			return;
		}
		
		victim->age = value;
		sprintf (buf, "%s has changed your age to %d.\r\n",ch->name, value);
		SEND(buf, victim);
		return;
	}
	 
    if (!str_cmp (arg2, "str"))
    {
        if (value < 3 || value > get_max_train (victim, STAT_STR))
        {
            sprintf (buf,
                     "Strength range is 3 to %d.\r\n",
                     get_max_train (victim, STAT_STR));
            SEND (buf, ch);
            return;
        }

        victim->perm_stat[STAT_STR] = value;
        return;
    }
	
	if (!str_prefix(arg2,"god"))
    {
        int god;

        god = god_lookup(arg3);

        if( god == 0 )
        {
            SEND("That is not a valid god for that character.\r\n",ch);
            return;
        }

        victim->god = god;
        return;
    }


    if (!str_cmp (arg2, "security"))
    {                            /* OLC */
        if (IS_NPC (ch))
        {
            SEND ("NPC's can't set this value.\r\n", ch);
            return;
        }

        if (IS_NPC (victim))
        {
            SEND ("Not on NPC's.\r\n", ch);
            return;
        }

        if (value > ch->pcdata->security || value < 0)
        {
            if (ch->pcdata->security != 0)
            {
                sprintf (buf, "Valid security is 0-%d.\r\n",
                         ch->pcdata->security);
                SEND (buf, ch);
            }
            else
            {
                SEND ("Valid security is 0 only.\r\n", ch);
            }
            return;
        }
        victim->pcdata->security = value;
        return;
    }

    if (!str_cmp (arg2, "int"))
    {
        if (value < 3 || value > get_max_train (victim, STAT_INT))
        {
            sprintf (buf,
                     "Intelligence range is 3 to %d.\r\n",
                     get_max_train (victim, STAT_INT));
            SEND (buf, ch);
            return;
        }

        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if (!str_cmp (arg2, "wis"))
    {
        if (value < 3 || value > get_max_train (victim, STAT_WIS))
        {
            sprintf (buf,
                     "Wisdom range is 3 to %d.\r\n", get_max_train (victim,
                                                                    STAT_WIS));
            SEND (buf, ch);
            return;
        }

        victim->perm_stat[STAT_WIS] = value;
        return;
    }

    if (!str_cmp (arg2, "dex"))
    {
        if (value < 3 || value > get_max_train (victim, STAT_DEX))
        {
            sprintf (buf,
                     "Dexterity range is 3 to %d.\r\n",
                     get_max_train (victim, STAT_DEX));
            SEND (buf, ch);
            return;
        }

        victim->perm_stat[STAT_DEX] = value;
        return;
    }

    if (!str_cmp (arg2, "con"))
    {
        if (value < 3 || value > get_max_train (victim, STAT_CON))
        {
            sprintf (buf,
                     "Constitution range is 3 to %d.\r\n",
                     get_max_train (victim, STAT_CON));
            SEND (buf, ch);
            return;
        }

        victim->perm_stat[STAT_CON] = value;
        return;
    }

	if (!str_cmp (arg2, "cha"))
    {
        if (value < 3 || value > get_max_train (victim, STAT_CHA))
        {
            sprintf (buf,
                     "Charisma range is 3 to %d\r\n.",
                     get_max_train (victim, STAT_CHA));
            SEND (buf, ch);
            return;
        }

        victim->perm_stat[STAT_CHA] = value;
        return;
    }
	
    if (!str_prefix (arg2, "sex"))
    {
        if (value < 0 || value > 2)
        {
            SEND ("Sex range is 0 to 2.\r\n", ch);
            return;
        }
        victim->sex = value;
        if (!IS_NPC (victim))
            victim->pcdata->true_sex = value;
        return;
    }

    if (!str_prefix (arg2, "class"))
    {
        int ch_class;

        if (IS_NPC (victim))
        {
            SEND ("Mobiles have no class.\r\n", ch);
            return;
        }

       ch_class = ch_class_lookup (arg3);
        if (ch_class == -1)
        {
            char buf[MAX_STRING_LENGTH];

            strcpy (buf, "Possible classes are: ");
            for (ch_class = 0;ch_class < MAX_CLASS;ch_class++)
            {
                if (ch_class > 0)
                    strcat (buf, " ");
                strcat (buf,ch_class_table[ch_class].name);
            }
            strcat (buf, ".\r\n");

            SEND (buf, ch);
            return;
        }

        victim->ch_class = ch_class;
        return;
    }

	if (!str_prefix (arg2, "class2"))
    {
        int ch_class;

        if (IS_NPC (victim))
        {
            SEND ("Mobiles have no class.\r\n", ch);
            return;
        }

       ch_class = ch_class_lookup (arg3);
        if (ch_class == -1)
        {
            char buf[MAX_STRING_LENGTH];

            strcpy (buf, "Possible classes are: ");
            for (ch_class = 0;ch_class < MAX_CLASS;ch_class++)
            {
                if (ch_class > 0)
                    strcat (buf, " ");
                strcat (buf,ch_class_table[ch_class].name);
            }
            strcat (buf, ".\r\n");

            SEND (buf, ch);
            return;
        }

        victim->ch_class2 =ch_class;
		victim->mClass = TRUE;
        return;
    }
	
    if (!str_prefix (arg2, "level"))
    {
        if (!IS_NPC (victim))
        {
            SEND ("Not on PC's.\r\n", ch);
            return;
        }

        if (value < 0 || value > MAX_LEVEL)
        {
            sprintf (buf, "Level range is 0 to %d.\r\n", MAX_LEVEL);
            SEND (buf, ch);
            return;
        }
        victim->level = value;
        return;
    }

	 if (!str_prefix (arg2, "level"))
    {
        if (!IS_NPC (victim))
        {
            SEND ("Not on PC's.\r\n", ch);
            return;
        }

        if (value < 0 || value > MCLASS_TWO)
        {
            sprintf (buf, "Level range is 0 to %d.\r\n", MCLASS_TWO);
            SEND (buf, ch);
            return;
        }
		if (!IS_MCLASSED(victim))
		{
			sprintf(buf, "%s is not multi classed.\r\n", victim->name);
			SEND(buf, ch);
			return;
		}
		
        victim->level2 = value;
        return;
    }
	
	if (!str_prefix (arg2, "guildpoints"))
    {					
		if (value < 0 || value > 10000)
        {
            SEND ("Level range is 0 to 10000.\r\n", ch);            
            return;
        }
		
		if (value > 1000 && total_levels(ch) < MAX_LEVEL)
		{
			SEND ("You don't have the power to do that.\r\n",ch);
			return;
		}
	
        victim->guildpoints = value;
        return;
    }
	
    if (!str_prefix (arg2, "gold"))
    {
        victim->gold = value;
        return;
    }

    if (!str_prefix (arg2, "silver"))
    {
        victim->silver = value;
        return;
    }

    if (!str_prefix (arg2, "hp"))
    {
        if (value < -10 || value > 30000)
        {
            SEND ("Hp range is -10 to 30,000 hit points.\r\n", ch);
            return;
        }
        victim->max_hit = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_hit = value;
        return;
    }
	
	if (!str_prefix (arg2, "pp"))
    {
        if (value < 0 || value > 999)
        {
            SEND ("Hp range is 0 to 999 psionic points.\r\n", ch);
            return;
        }
        victim->max_pp = value;
        if (!IS_NPC (victim))
		{
            victim->max_pp = value;
			victim->pp = victim->max_pp;
		}
		else
			SEND ("Not on NPCs.\r\n",ch);
        return;
    }

    if (!str_prefix (arg2, "mana"))
    {
        if (value < 0 || value > 30000)
        {
            SEND ("Mana range is 0 to 30,000 mana points.\r\n", ch);
            return;
        }
        victim->max_mana = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_mana = value;
        return;
    }

    if (!str_prefix (arg2, "move"))
    {
        if (value < 0 || value > 30000)
        {
            SEND ("Move range is 0 to 30,000 move points.\r\n", ch);
            return;
        }
        victim->max_move = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_move = value;
        return;
    }

    if (!str_prefix (arg2, "practice"))
    {
        if (value < 0 || value > 250)
        {
            SEND ("Practice range is 0 to 250 sessions.\r\n", ch);
            return;
        }
        victim->practice = value;
        return;
    }

    if (!str_prefix (arg2, "train"))
    {
        if (value < 0 || value > 50)
        {
            SEND ("Training session range is 0 to 50 sessions.\r\n",
                          ch);
            return;
        }
        victim->train = value;
        return;
    }

    if (!str_prefix (arg2, "align"))
    {
        if (value < -1000 || value > 1000)
		{			
            SEND ("Alignment range is -1000 to 1000.\r\n", ch);
            return;
        }
        victim->alignment = value;
        return;
    }

    if (!str_prefix (arg2, "thirst"))
    {
        if (IS_NPC (victim))
        {
            SEND ("Not on NPC's.\r\n", ch);
            return;
        }

        if (value < -1 || value > 100)
        {
            SEND ("Thirst range is -1 to 100.\r\n", ch);
            return;
        }

        victim->pcdata->condition[COND_THIRST] = value;
        return;
    }

    if (!str_prefix (arg2, "drunk"))
    {
        if (IS_NPC (victim))
        {
            SEND ("Not on NPC's.\r\n", ch);
            return;
        }

        if (value < -1 || value > 100)
        {
            SEND ("Drunk range is -1 to 100.\r\n", ch);
            return;
        }

        victim->pcdata->condition[COND_DRUNK] = value;
        return;
    }

    if (!str_prefix (arg2, "full"))
    {
        if (IS_NPC (victim))
        {
            SEND ("Not on NPC's.\r\n", ch);
            return;
        }

        if (value < -1 || value > 100)
        {
            SEND ("Full range is -1 to 100.\r\n", ch);
            return;
        }

        victim->pcdata->condition[COND_FULL] = value;
        return;
    }

    if (!str_prefix (arg2, "hunger"))
    {
        if (IS_NPC (victim))
        {
            SEND ("Not on NPC's.\r\n", ch);
            return;
        }

        if (value < -1 || value > 100)
        {
            SEND ("Full range is -1 to 100.\r\n", ch);
            return;
        }

        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if (!str_prefix (arg2, "race"))
    {
        int race;

        race = race_lookup (arg3);

        if (race == 0)
        {
            SEND ("That is not a valid race.\r\n", ch);
            return;
        }

        if (!IS_NPC (victim) && !race_table[race].pc_race)
        {
            SEND ("That is not a valid player race.\r\n", ch);
            return;
        }

        victim->race = race;
        return;
    }

    if (!str_prefix (arg2, "group"))
    {
        if (!IS_NPC (victim))
        {
            SEND ("Only on NPCs.\r\n", ch);
            return;
        }
        victim->group = value;
        return;
    }

	if (!str_prefix (arg2, "hours"))
	{
		if (IS_NPC (victim))
		{
			SEND ("Not on NPC's.\r\n", ch);
			return;
		}

		if (!is_number (arg3))
		{
			SEND ("Value must be numeric.\r\n", ch);
			return;
		}

		value = atoi (arg3);

		if (value < 0 || value > 999)
		{
			SEND ("Value must be between 0 and 999.\r\n", ch);
			return;
		}

		victim->played = ( value * 3600 );
		printf_to_char(ch, "%s's hours set to %d.", victim->name, value);

		return;
	}
	
    /*
     * Generate usage message.
     */
    do_function (ch, &do_mset, "");
    return;
}

void do_string (CHAR_DATA * ch, char *argument)
{
    char type[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde (argument);
    argument = one_argument (argument, type);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0'
        || arg3[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  string char <name> <field> <string>\r\n", ch);
        SEND ("    fields: name short long desc title spec\r\n", ch);
        SEND ("  string obj  <name> <field> <string>\r\n", ch);
        SEND ("    fields: name short long extended\r\n", ch);
        return;
    }

    if (!str_prefix (type, "character") || !str_prefix (type, "mobile"))
    {
        if ((victim = get_char_world (ch, arg1)) == NULL)
        {
            SEND ("They aren't here.\r\n", ch);
            return;
        }

        /* clear zone for mobs */
        victim->zone = NULL;

        /* string something */

        if (!str_prefix (arg2, "name"))
        {
            if (!IS_NPC (victim))
            {
                SEND ("Not on PC's.\r\n", ch);
                return;
            }
            free_string (victim->name);
            victim->name = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "description"))
        {
            free_string (victim->description);
            victim->description = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "short"))
        {
            free_string (victim->short_descr);
            victim->short_descr = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "long"))
        {
            free_string (victim->long_descr);
            strcat (arg3, "\r\n");
            victim->long_descr = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "title"))
        {
            if (IS_NPC (victim))
            {
                SEND ("Not on NPC's.\r\n", ch);
                return;
            }

            set_title (victim, arg3);
            return;
        }

        if (!str_prefix (arg2, "spec"))
        {
            if (!IS_NPC (victim))
            {
                SEND ("Not on PC's.\r\n", ch);
                return;
            }

            if ((victim->spec_fun = spec_lookup (arg3)) == 0)
            {
                SEND ("No such spec fun.\r\n", ch);
                return;
            }

            return;
        }
    }

    if (!str_prefix (type, "object"))
    {
        /* string an obj */

        if ((obj = get_obj_world (ch, arg1)) == NULL)
        {
            SEND ("Nothing like that in heaven or earth.\r\n", ch);
            return;
        }

        if (!str_prefix (arg2, "name"))
        {
            free_string (obj->name);
            obj->name = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "short"))
        {
            free_string (obj->short_descr);
            obj->short_descr = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "long"))
        {
            free_string (obj->description);
            obj->description = str_dup (arg3);
            return;
        }

        if (!str_prefix (arg2, "ed") || !str_prefix (arg2, "extended"))
        {
            EXTRA_DESCR_DATA *ed;

            argument = one_argument (argument, arg3);
            if (argument == NULL)
            {
                SEND
                    ("Syntax: oset <object> ed <keyword> <string>\r\n", ch);
                return;
            }

            strcat (argument, "\r\n");

            ed = new_extra_descr ();

            ed->keyword = str_dup (arg3);
            ed->description = str_dup (argument);
            ed->next = obj->extra_descr;
            obj->extra_descr = ed;
            return;
        }
    }


    /* echo bad use message */
    do_function (ch, &do_string, "");
}



void do_oset (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde (argument);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  set obj <object> <field> <value>\r\n", ch);
        SEND ("  Field being one of:\r\n", ch);
        SEND ("    value0 value1 value2 value3 value4 (v1-v4)\r\n",
                      ch);
        SEND ("    extra wear level weight cost timer\r\n", ch);
        return;
    }

    if ((obj = get_obj_world (ch, arg1)) == NULL)
    {
        SEND ("Nothing like that in heaven or earth.\r\n", ch);
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi (arg3);

    /*
     * Set something.
     */
    if (!str_cmp (arg2, "value0") || !str_cmp (arg2, "v0"))
    {
        obj->value[0] = UMIN (50, value);
        return;
    }

    if (!str_cmp (arg2, "value1") || !str_cmp (arg2, "v1"))
    {
        obj->value[1] = value;
        return;
    }

    if (!str_cmp (arg2, "value2") || !str_cmp (arg2, "v2"))
    {
        obj->value[2] = value;
        return;
    }

    if (!str_cmp (arg2, "value3") || !str_cmp (arg2, "v3"))
    {
        obj->value[3] = value;
        return;
    }

    if (!str_cmp (arg2, "value4") || !str_cmp (arg2, "v4"))
    {
        obj->value[4] = value;
        return;
    }

    if (!str_prefix (arg2, "extra"))
    {
        obj->extra_flags = value;
        return;
    }

    if (!str_prefix (arg2, "wear"))
    {
        obj->wear_flags = value;
        return;
    }

    if (!str_prefix (arg2, "level"))
    {
        obj->level = value;
        return;
    }

    if (!str_prefix (arg2, "weight"))
    {
        obj->weight = value;
        return;
    }

    if (!str_prefix (arg2, "cost"))
    {
        obj->cost = value;
        return;
    }

    if (!str_prefix (arg2, "timer"))
    {
        obj->timer = value;
        return;
    }

    /*
     * Generate usage message.
     */
    do_function (ch, &do_oset, "");
    return;
}



void do_rset (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde (argument);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  set room <location> <field> <value>\r\n", ch);
        SEND ("  Field being one of:\r\n", ch);
        SEND ("    flags sector\r\n", ch);
        return;
    }

    if ((location = find_location (ch, arg1)) == NULL)
    {
        SEND ("No such location.\r\n", ch);
        return;
    }

    if (!is_room_owner (ch, location) && ch->in_room != location
        && room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        SEND ("That room is private right now.\r\n", ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if (!is_number (arg3))
    {
        SEND ("Value must be numeric.\r\n", ch);
        return;
    }
    value = atoi (arg3);

    /*
     * Set something.
     */
    if (!str_prefix (arg2, "flags"))
    {
        location->room_flags = value;
        return;
    }

    if (!str_prefix (arg2, "sector"))
    {
        location->sector_type = value;
        return;
    }

    /*
     * Generate usage message.
     */
    do_function (ch, &do_rset, "");
    return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        SEND ("Force whom to do what?\r\n", ch);
        return;
    }

    one_argument (argument, arg2);

    if (!str_cmp (arg2, "delete") || !str_prefix (arg2, "mob"))
    {
        SEND ("That will NOT be done.\r\n", ch);
        return;
    }

    sprintf (buf, "$n forces you to '%s'.", argument);

	/* Replaced original block with code by Edwin to keep from
	 * corrupting pfiles in certain pet-infested situations.
	 * JR -- 10/15/00
	 */
	if ( !str_cmp( arg, "all" ) )
	{
    	DESCRIPTOR_DATA *desc,*desc_next;

    	if (get_trust(ch) < MAX_LEVEL - 3)
    	{
			SEND("Not at your level!\r\n",ch);
			return;
    	}

    	for ( desc = descriptor_list; desc != NULL; desc = desc_next )
    	{
			desc_next = desc->next;

			if (desc->connected==CON_PLAYING &&
	    		get_trust( desc->character ) < get_trust( ch ) )
	    	{
	    		act( buf, ch, NULL, desc->character, TO_VICT );
	    		interpret( desc->character, argument );
			}
    	}
	}
    else if (!str_cmp (arg, "players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (get_trust (ch) < MAX_LEVEL - 2)
        {
            SEND ("Not at your level!\r\n", ch);
            return;
        }

        for (vch = char_list; vch != NULL; vch = vch_next)
        {
            vch_next = vch->next;

            if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch)
                && vch->level < LEVEL_HERO)
            {
                act (buf, ch, NULL, vch, TO_VICT);
                interpret (vch, argument);
            }
        }
    }
    else if (!str_cmp (arg, "gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (get_trust (ch) < MAX_LEVEL - 2)
        {
            SEND ("Not at your level!\r\n", ch);
            return;
        }

        for (vch = char_list; vch != NULL; vch = vch_next)
        {
            vch_next = vch->next;

            if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch)
                && vch->level >= LEVEL_HERO)
            {
                act (buf, ch, NULL, vch, TO_VICT);
                interpret (vch, argument);
            }
        }
    }
    else
    {
        CHAR_DATA *victim;

        if ((victim = get_char_world (ch, arg)) == NULL)
        {
            SEND ("They aren't here.\r\n", ch);
            return;
        }

        if (victim == ch)
        {
            SEND ("Aye aye, right away!\r\n", ch);
            return;
        }

        if (!is_room_owner (ch, victim->in_room)
            && ch->in_room != victim->in_room
            && room_is_private (victim->in_room)
            && !IS_TRUSTED (ch, IMPLEMENTOR))
        {
            SEND ("That character is in a private room.\r\n", ch);
            return;
        }

        if (get_trust (victim) >= get_trust (ch))
        {
            SEND ("Do it yourself!\r\n", ch);
            return;
        }

        if (!IS_NPC (victim) && get_trust (ch) < MAX_LEVEL - 3)
        {
            SEND ("Not at your level!\r\n", ch);
            return;
        }

        act (buf, ch, NULL, victim, TO_VICT);
        interpret (victim, argument);
    }

    SEND ("Ok.\r\n", ch);
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis (CHAR_DATA * ch, char *argument)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument (argument, arg);

    if (arg[0] == '\0')
        /* take the default path */

        if (ch->invis_level)
        {
            ch->invis_level = 0;
            act ("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
            SEND ("You slowly fade back into existence.\r\n", ch);
        }
        else
        {
            ch->invis_level = get_trust (ch);
            act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
            SEND ("You slowly vanish into thin air.\r\n", ch);
        }
    else
        /* do the level thing */
    {
        level = atoi (arg);
        if (level < 2 || level > get_trust (ch))
        {
            SEND ("Invis level must be between 2 and your level.\r\n",
                          ch);
            return;
        }
        else
        {
            ch->reply = NULL;
            ch->invis_level = level;
            act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
            SEND ("You slowly vanish into thin air.\r\n", ch);
        }
    }

    return;
}


void do_incognito (CHAR_DATA * ch, char *argument)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument (argument, arg);

    if (arg[0] == '\0')
        /* take the default path */

        if (ch->incog_level)
        {
            ch->incog_level = 0;
            act ("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
            SEND ("You are no longer cloaked.\r\n", ch);
        }
        else
        {
            ch->incog_level = get_trust (ch);
            act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
            SEND ("You cloak your presence.\r\n", ch);
        }
    else
        /* do the level thing */
    {
        level = atoi (arg);
        if (level < 2 || level > get_trust (ch))
        {
            SEND ("Incog level must be between 2 and your level.\r\n",
                          ch);
            return;
        }
        else
        {
            ch->reply = NULL;
            ch->incog_level = level;
            act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
            SEND ("You cloak your presence.\r\n", ch);
        }
    }

    return;
}



void do_holylight (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_HOLYLIGHT))
    {
        REMOVE_BIT (ch->act, PLR_HOLYLIGHT);
        SEND ("Holy light mode off.\r\n", ch);
    }
    else
    {
        SET_BIT (ch->act, PLR_HOLYLIGHT);
        SEND ("Holy light mode on.\r\n", ch);
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA * ch, char *argument)
{
    SEND ("You cannot abbreviate the prefix command.\r\n", ch);
    return;
}

void do_prefix (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
        if (ch->prefix[0] == '\0')
        {
            SEND ("You have no prefix to clear.\r\n", ch);
            return;
        }

        SEND ("Prefix removed.\r\n", ch);
        free_string (ch->prefix);
        ch->prefix = str_dup ("");
        return;
    }

    if (ch->prefix[0] != '\0')
    {
        sprintf (buf, "Prefix changed to %s.\r\n", argument);
        free_string (ch->prefix);
    }
    else
    {
        sprintf (buf, "Prefix set to %s.\r\n", argument);
    }

    ch->prefix = str_dup (argument);
}

/* commenting in order to fix gcc errors on fedora core 2
 * 9/21/04

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)
*/

/* This file holds the copyover data */
#define COPYOVER_FILE "copyover.data"

/* This is the executable file */
#define EXE_FILE      "../src/rom"


/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */

 /*
 void do_copyover (CHAR_DATA * ch, char *argument)
{
    FILE *fp;
    DESCRIPTOR_DATA *d, *d_next;
    char buf[100], buf2[100], buf3[100];
    extern int port, control;    // db.c 
	//int most_today = 0;

    fp = fopen (COPYOVER_FILE, "w");

    if (!fp)
    {
        SEND ("Copyover file not writeable, aborted.\r\n", ch);
        log_f ("Could not write to copyover file: %s", COPYOVER_FILE);
        perror ("do_copyover:fopen");
        return;
    }

    // Consider changing all saved areas here, if you use OLC 

    // do_asave (NULL, ""); - autosave changed areas 


    sprintf (buf, "\r\n *** COPYOVER by %s - please remain seated!\r\n",
             ch->name);

    // For each playing descriptor, save its state 
    for (d = descriptor_list; d; d = d_next)
    {
        CHAR_DATA *och = CH (d);
        d_next = d->next;        // We delete from the list , so need to save this 

        if (!d->character || d->connected > CON_PLAYING || d->connected < CON_PLAYING)
        {                        // drop those logging on 
            write_to_descriptor (d->descriptor,
                                 "\r\nSorry, we are rebooting. Come back in a few minutes.\r\n",
                                 0);
            close_socket (d);    // throw'em out 
        }
        else
        {
            fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
            save_char_obj (och);

            write_to_descriptor (d->descriptor, buf, 0);
        }
    }

    fprintf (fp, "-1\n");
    fclose (fp);

     //Close reserve and other always-open files and release other resources 

    fclose (fpReserve);

     //exec - descriptors are inherited 

    sprintf (buf, "%d", port);
    sprintf (buf2, "%d", control);
    strncpy( buf3, "-1", 100 );
    execl (EXE_FILE, "rom", buf, "copyover", buf2, buf3, (char *) NULL);

    // Failed - sucessful exec will not return 

    perror ("do_copyover: execl");
    SEND ("Copyover FAILED!\r\n", ch);

    // Here you might want to reopen fpReserve 
    fpReserve = fopen (NULL_FILE, "r");
}
*/

/* Recover from a copyover - load players */

/*
void copyover_recover ()
{
    DESCRIPTOR_DATA *d;
    FILE *fp;
    char name[100];
    char host[MSL];
    int desc;
    bool fOld;

    log_f ("Copyover recovery initiated");

    fp = fopen (COPYOVER_FILE, "r");

    if (!fp)
    {                            // there are some descriptors open which will hang forever then ? 
        perror ("copyover_recover:fopen");
        log_f ("Copyover file not found. Exitting.\r\n");
        exit (1);
    }

    unlink (COPYOVER_FILE);        // In case something crashes - doesn't prevent reading  

    for (;;)
    {
		//Compiler doesn't like this line. Upro
        fscanf (fp, "%d %s %s\n", &desc, name, host);
        if (desc == -1)
            break;

         //Write something, and check if it goes error-free 
        if (!write_to_descriptor
            (desc, "\r\nRestoring from copyover...\r\n", 0))
        {
            close (desc);        // nope 
            continue;
        }

        d = new_descriptor ();
        d->descriptor = desc;

        d->host = str_dup (host);
        d->next = descriptor_list;
        descriptor_list = d;
        d->connected = CON_COPYOVER_RECOVER;    // -15, so close_socket frees the char 


        // Now, find the pfile 

        fOld = load_char_obj (d, name);

        if (!fOld)
        {                        // Player file not found?! 
            write_to_descriptor (desc,
                                 "\r\nSomehow, your character was lost in the copyover. Sorry.\r\n",
                                 0);
            close_socket (d);
        }
        else
        {                        // ok! 

            write_to_descriptor (desc, "\r\nCopyover recovery complete.\r\n",
                                 0);

            // Just In Case 
            if (!d->character->in_room)
                d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

            // Insert in the char_list 
            d->character->next = char_list;
            char_list = d->character;

            char_to_room (d->character, d->character->in_room);
            do_look (d->character, "auto");
            act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
            d->connected = CON_PLAYING;

            if (d->character->pet != NULL)
            {
                char_to_room (d->character->pet, d->character->in_room);
                act ("$n materializes!.", d->character->pet, NULL, NULL,
                     TO_ROOM);
            }
        }

    }
    fclose (fp);
}
*/

/* This _should_ encompass all the QuickMUD config commands */
/* -- JR 11/24/00                                           */

void do_qmconfig (CHAR_DATA * ch, char * argument)
{
	extern int mud_ansiprompt;
	extern int mud_ansicolor;
	extern int mud_telnetga;
	extern char *mud_ipaddress;
	char arg1[MSL];
	char arg2[MSL];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		printf_to_char(ch, "Valid qmconfig options are:\r\n");
		printf_to_char(ch, "    show       (shows current status of toggles)\r\n");
		printf_to_char(ch, "    ansiprompt [on|off]\r\n");
		printf_to_char(ch, "    ansicolor  [on|off]\r\n");
		printf_to_char(ch, "    telnetga   [on|off]\r\n");
		printf_to_char(ch, "    read\r\n");
		return;
	}
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (!str_prefix(arg1, "read")) {
		qmconfig_read();
		return;
	}
		
	if (!str_prefix(arg1, "show"))
	{
		printf_to_char(ch, "ANSI prompt: %s", mud_ansiprompt ? "{GON{x\r\n" : "{ROFF{x\r\n");
		printf_to_char(ch, "ANSI color : %s", mud_ansicolor ? "{GON{x\r\n" : "{ROFF{x\r\n");
		printf_to_char(ch, "IP Address : %s\r\n", mud_ipaddress);
		printf_to_char(ch, "Telnet GA  : %s", mud_telnetga ? "{GON{x\r\n" : "{ROFF{x\r\n");
		return;
	}

	if (!str_prefix(arg1, "ansiprompt"))
	{
		if (!str_prefix(arg2, "on"))
		{
			mud_ansiprompt=TRUE;
			printf_to_char(ch, "New logins will now get an ANSI color prompt.\r\n");
			return;
		}

		else if(!str_prefix(arg2, "off"))
		{
			mud_ansiprompt=FALSE;
			printf_to_char(ch, "New logins will not get an ANSI color prompt.\r\n");
			return;
		}

		printf_to_char(ch, "Valid arguments are \"on\" and \"off\".\r\n");
		return;
	}

	if (!str_prefix(arg1, "ansicolor"))
	{
		if (!str_prefix(arg2, "on"))
		{
			mud_ansicolor=TRUE;
			printf_to_char(ch, "New players will have color enabled.\r\n");
			return;
		}
		
		else if (!str_prefix(arg2, "off"))
		{
			mud_ansicolor=FALSE;
			printf_to_char(ch, "New players will not have color enabled.\r\n");
			return;
		}

		printf_to_char(ch, "Valid arguments are \"on\" and \"off\".\r\n");
		return;
	}
	
	if (!str_prefix(arg1, "telnetga"))
	{
		if (!str_prefix(arg2, "on"))
		{
			mud_telnetga=TRUE;
			printf_to_char(ch, "Telnet GA will be enabled for new players.\r\n");
			return;
		}
		
		else if (!str_prefix(arg2, "off"))
		{
			mud_telnetga=FALSE;
			printf_to_char(ch, "Telnet GA will be disabled for new players.\r\n");
			return;
		}

		printf_to_char(ch, "Valid arguments are \"on\" and \"off\".\r\n");
		return;
	}

	printf_to_char(ch, "I have no clue what you are trying to do...\r\n");
	return;
}

void qmconfig_read (void) {
	FILE *fp;
	bool fMatch;
	char *word;
	extern int mud_ansiprompt, mud_ansicolor, mud_telnetga;

	log_f("Loading configuration settings from ../area/qmconfig.rc.");

	fp = fopen("../area/qmconfig.rc","r");
	if (!fp) {
		log_f("qmconfig.rc not found. Using compiled-in defaults.");
		return;
	}

	for(;;) {
		word = feof (fp) ? "END" : fread_word(fp);

		fMatch = FALSE;

		switch (UPPER(word[0])) {
			case '#':
				/* This is a comment line! */
				fMatch = TRUE;
				fread_to_eol (fp);
				break;
			case '*':
				fMatch = TRUE;
				fread_to_eol (fp);
				break;

			case 'A':
				KEY ("Ansicolor", mud_ansicolor, fread_number(fp));
				KEY ("Ansiprompt", mud_ansiprompt, fread_number(fp));
				break;
			case 'E':
				if (!str_cmp(word, "END"))
					return;
				break;
			case 'T':
				KEY ("Telnetga", mud_telnetga, fread_number(fp));
				break;
		}
		if (!fMatch) {
			log_f("qmconfig_read: no match for %s!", word);
			fread_to_eol(fp);
		}
	}	
	log_f("Settings have been read from ../area/qmconfig.rc");
	exit(0);

}


void do_addguildpoints ( CHAR_DATA *ch, char *argument)
{

    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
    {
	SEND("huh?\r\n", ch);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	SEND( "Syntax: addgp <char> <guildpoints>.\r\n", ch );
	return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      SEND("That player is not here.\r\n", ch);
      return;
    }

    if ( IS_NPC( victim ) )
    {
	SEND( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( get_trust( ch ) < get_trust( victim ))
    {
	SEND( "That person is above your level. Nice try.\r\n", ch );
	return;
    }

    value = atoi( arg2 );

    if ( value < -200 || value > 200 )
    {
	SEND( "Value range is -200 to 200.\r\n", ch );
	return;
    }

    if ( value == 0 )
    {
	SEND( "The value must not be equal to 0.\r\n", ch );
	return;
    }
    
	victim->guildpoints += value;
	
    sprintf( buf,"You have given %s %d guild points.\r\n",
    		victim->name, value);
    		SEND(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"You gain %d {bguild points{x!\r\n", value );
      SEND( buf, victim );
    }
    else
    {
      sprintf( buf,"You lose %d {bguild points{x :(.\r\n", abs(value) );
      SEND( buf, victim );
    }

    return;
}



void do_addhours( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
    {
	SEND("huh?\r\n", ch);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	SEND( "Syntax: addhours <char> <hours>.\r\n", ch );
	return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      SEND("That player is not here.\r\n", ch);
      return;
    }

    if ( IS_NPC( victim ) )
    {
	SEND( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( get_trust( ch ) < get_trust( victim ))
    {
	SEND( "That person is above your level. Nice try.\r\n", ch );
	return;
    }

    value = atoi( arg2 );

    if ( value < -200 || value > 200 )
    {
	SEND( "Value range is -200 to 200.\r\n", ch );
	return;
    }

    if ( value == 0 )
    {
	SEND( "The value must not be equal to 0.\r\n", ch );
	return;
    }

    victim->played += value * 60 * 60;

    sprintf( buf,"You have given %s %d playing hours.\r\n",
    		victim->name, value);
    		SEND(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"You gain %d {bhours{x!\r\n", value );
      SEND( buf, victim );
    }
    else
    {
      sprintf( buf,"You lose %d {bhours{x :(.\r\n", abs(value) );
      SEND( buf, victim );
    }

    return;
}

void do_addmin( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
    {
	SEND("huh?\r\n", ch);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	SEND( "Syntax: addmin <char> <minutes>.\r\n", ch );
	return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      SEND("That player is not here.\r\n", ch);
      return;
    }

    if ( IS_NPC( victim ) )
    {
	SEND( "Not on NPC's.\r\n", ch );
	return;
    }

    if ( get_trust( ch ) < get_trust( victim ))
    {
	SEND( "That person is above your level. Nice try.\r\n", ch );
	return;
    }

    value = atoi( arg2 );

    if ( value < -60 || value > 60 )
    {
	SEND( "Value range is -60 to 60.\r\n", ch );
	return;
    }

    if ( value == 0 )
    {
	SEND( "The value must not be equal to 0.\r\n", ch );
	return;
    }

    victim->played += value * 60;

    sprintf( buf,"You have given %s %d playing minutes.\r\n",
    		victim->name, value);
    		SEND(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"You gain %d {bminutes{x!\r\n", value );
      SEND( buf, victim );
    }
    else
    {
      sprintf( buf,"You lose %d {bminutes{x :(.\r\n", abs(value) );
      SEND( buf, victim );
    }

    return;
}

void do_addrep (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char buf [MSL];
	char arg1 [MIL];
	char arg2 [MIL];
	char arg3 [MIL];
	int value;	
	int i = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
	

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || !is_number( arg3 ) )
    {
		SEND( "Syntax: addrep <char> <faction> <amount>.\r\n", ch );
		return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
		SEND("That player is not here.\r\n", ch);
		return;
    }

    if ( IS_NPC( victim ) )
    {
		SEND( "Not on NPC's.\r\n", ch );
		return;
    }

    /*if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
    {
		SEND("You can't bonus immortals, silly!\r\n", ch);
		return;
    }*/

	bool goodfac = FALSE;
	
	for (i = 0; i <= MAX_FACTION; i++)
	{
		if (!str_cmp(arg2, faction_table[i].name))
		{
			goodfac = TRUE;
			break;
		}
	}
	
	if (goodfac)
	{
		value = atoi( arg3 );

		if(get_trust(ch) < 48 && ( value < -200 || value > 200 ))
		{
			SEND( "Value range is -200 to 200.\r\n", ch );
			return;
		}

		if ( value == 0 )
		{
			SEND( "The value must not be equal to 0.\r\n", ch );
			return;
		}
		
		victim->faction_rep[i] += value;	
		
		sprintf( buf,"You have given %s %d {bfaction{x points.\r\n",
				victim->name, value);
		SEND(buf, ch);
		sprintf( buf,"You gain %d {bfaction{x points!\r\n", value );
		SEND( buf, victim );
		return;
	}
	else
	{
		SEND("That's not a valid faction.\r\n",ch);
		return;
	}
	return;
}

void do_addxp( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
	char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
    {
	SEND("huh?\r\n", ch);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
		SEND( "Syntax: addxp <char> <exp>.\r\n", ch );
		return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
		SEND("That player is not here.\r\n", ch);
		return;
    }

    if ( IS_NPC( victim ) )
    {
		SEND( "Not on NPC's.\r\n", ch );
		return;
    }

    if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
    {
		SEND("You can't bonus immortals, silly!\r\n", ch);
		return;
    }

    value = atoi( arg2 );

    if(ch->level < 49 && ( value < -10000 || value > 10000 ))
    {
		SEND( "Value range is -10000 to 10000.\r\n", ch );
		return;
    }

    if ( value == 0 )
    {
		SEND( "The value must not be equal to 0.\r\n", ch );
		return;
    }
    
    if (!(IS_MCLASSED(victim)))
	{
		gain_exp(victim, value, FALSE);
		return;
	}
	if (IS_MCLASSED(victim) && victim->level < MCLASS_ONE)
	{
		if (victim->level2 < MCLASS_TWO)
		{
			gain_exp(victim, value / 2, FALSE);
			gain_exp(victim, value / 2, TRUE);
		}
		else
			gain_exp(victim, value, FALSE);
	}
	else if (IS_MCLASSED(victim) && victim->level >= MCLASS_ONE && victim->level2 < MCLASS_TWO)
		gain_exp(victim, value, TRUE);
	else
	{
		SEND ("They cannot possibly receive more experience points.\r\n",ch);		
		return;
	}
	
	sprintf( buf,"You have given %s %d {bexperience{x points.\r\n",
    		victim->name, value);
    SEND(buf, ch);
    sprintf( buf,"You gain %d {bexperience{x!\r\n", value );
    SEND( buf, victim );
    return;
}



void do_pretitle( CHAR_DATA *ch, char *argument )
{
    char strVictim [MAX_INPUT_LENGTH];
    char strPretitle [MAX_INPUT_LENGTH];

    CHAR_DATA *victim;
    unsigned int i = 0;
    int count = 0;
    
    smash_tilde( argument );
    argument = one_argument( argument, strVictim );
    strcpy( strPretitle, argument );

    if ( IS_NPC( ch ) )
    {
	return;
    }

    if ( !IS_IMMORTAL( ch ) && !is_clan( ch ) )
    {
	SEND( "You are not currently clanned; therefore, you cannot assign pretitles.\r\n", ch );
	return;
    }

    if ( !IS_IMMORTAL( ch ) && clan_table[ch->clan].independent )
    {
	SEND( "You are in an independent clan; therefore, you cannot assign pretitles.\r\n", ch );
	return;
    }

    //if ( !IS_IMMORTAL( ch ) && ch->pcdata->rank < 5 )
	if ( !IS_IMMORTAL(ch))
    {
	SEND( "You are not the leader of your clan; therefore, you cannot assign pretitles.\r\n", ch );
	return;
    }

    if ( strVictim[0] == '\0' || strPretitle[0] == '\0' )
    {
        SEND("Syntax:  pretitle <name> <pretitle>\r\n",ch);
        SEND("         pretitle <name> null\r\n",ch);
        return;
    }
    
    if ( ( victim = get_char_world( ch, strVictim ) ) == NULL )
    {
        SEND( "They aren't here.\r\n", ch );
        return;
    }
    
    if ( IS_NPC( victim ) )
    {
        SEND( "That person is not a player character!  Nice try.\r\n", ch );
        return;
    }

    if ( !IS_IMMORTAL( ch ) && victim->clan != ch->clan )
    {
        SEND( "That person is not even in the same clan as you!  Nice try.\r\n", ch );
        return;
    }

    //if ( !IS_IMMORTAL( ch ) && victim->pcdata->rank < 3 )
	if ( !IS_IMMORTAL ( ch ))
    {
        SEND( "You cannot assign a pretitle to people lower rank than 3.\r\n", ch );
        return;
    }

    if ( IS_IMMORTAL( victim ) )
    {
	if ( !IS_IMMORTAL( ch ) )
	{
            SEND( "Funny.  Now you're in trouble.\r\n", ch );
            return;
	}        

        if ( get_trust(victim) > get_trust(ch))
        {
            SEND("Don't do it.\r\n",ch);
            return;
        }
    }
   
    if ( !str_cmp( strPretitle, "null" ) )
    {
    	act( "You have erased $N's pretitle.", ch, NULL, victim, TO_CHAR );
    	act( "$n has erased your pretitle.  Nothing to see here, move along.", ch, NULL, victim, TO_VICT );

    	free_string( victim->pcdata->pretitle );
    	victim->pcdata->pretitle = str_dup( "" );
	return;
    }

    if ( colorstrlen( strPretitle ) < 4 || colorstrlen( strPretitle ) > 12 )
    {
        SEND( "Pretitles should be 4 - 12 characters long, not including color characters.\r\n", ch );
        return;
    }
  
    for ( i = 0; i < strlen( strPretitle ); i++ )
    {
	if ( strPretitle[ i ] == '{' )
	{
	    count++;
	}

	if ( count > 1 )
	{
	    break;
	}
    }

    if ( count != 1 )
    {
        SEND( "Pretitles are required a single color character, no more, no less.\r\n", ch );
        return;	
    }

    if ( strPretitle[ 0 ] != '{' )
    {
        SEND( "Pretitles are required to start with a color character.\r\n", ch );
        return;	
    }

    if ( strPretitle[ 1 ] != 'r' && strPretitle[ 1 ] != 'g' && strPretitle[ 1 ] != 'b' && strPretitle[ 1 ] != 'm' 
	&& strPretitle[ 1 ] != 'c' && strPretitle[ 1 ] != 'y' && strPretitle[ 1 ] != 'W' && strPretitle[ 1 ] != 'D' && 
        strPretitle[ 1 ] != 'x')
    {
        SEND( "Pretitles cannot use a bright color such as {{W, {{Y, {{G, {{B.  {{D, {{b, {{g, etc. are acceptable.\r\n", ch );
        return;	
    }

    act( "You have assigned a new pretitle to $N.", ch, NULL, victim, TO_CHAR );
    act( "$n has assigned you a new pretitle, check it out!", ch, NULL, victim, TO_VICT );

    free_string( victim->pcdata->pretitle );
    if(colorstrlen( strPretitle ) != 0)
       strcat( strPretitle, "{x " );
    victim->pcdata->pretitle = str_dup( strPretitle );
}


void do_sockets( CHAR_DATA *ch, char *arg )
{
    DESCRIPTOR_DATA      *d;
    CHAR_DATA            *wch;
    char                  buf[21000];
    int                   count= 0;

    buf[0]      = '\0';

    /* Show all descriptors, even if character not assigned yet. */
    /* --------------------------------------------------------- */

    for ( d= descriptor_list; d; d= d->next )
    {
        wch = d->original ? d->original : d->character;

        if ( ( arg[0] == '\0' && ( wch == NULL || can_see( ch, wch ) ) )
        ||   ( arg[0] != '\0' &&  wch && is_name( arg, wch->name ) ) )
        {
	    count++;
            sprintf( buf + strlen(buf), "{r[{R%1s{r]{x %12s {r[{x%3d %16s %6ld{r] [{x%36s{r]{x\r\n",                
				d->character->timer > 4 ? "X" : "", 
				wch  ? wch->name : "(none)",
				d->descriptor,
                GET_CONNECTED_STRING(d),
				(d->character->in_room != NULL ? d->character->in_room->vnum : -1),
                d->host);				
        }
    }
	SEND("\r\n",ch);
    /* Status messages */
    /* --------------- */

    if ( count == 0 )
    {
        sprintf( buf, "You didn't find any user with the name %s.\r\n", arg );
        SEND( buf, ch );
    }
    else
    {        
		sprintf( buf + strlen(buf), "\r\nThere %s currently {g%d{x connection%s.\r\n", count > 1 ? "are" : "is",count, count > 1 ? "s" : "" );
        page_to_char( buf, ch );
    }
}

void do_repstat(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH]; 
    char buf[MSL];	
    CHAR_DATA *victim;
	int i;

    if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg);
	
    if ( arg[0] == '\0' )
    {
        SEND( "List reputation for whom?\r\n", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        SEND( "They aren't here.\r\n", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        SEND( "Use this for factions/reputation on players.\r\n", ch );
        return;
    }

	// Show faction rep, rank, and society status/faction loyalty.
	sprintf(buf, "{g%-14s      {g%-5s    {g%-8s\r\n","Faction{x:","Rep{x:","Rank{x:");
	SEND (buf,ch);
	SEND ("{D----------------------------------------{x\r\n",ch);	
	for (i = 0; i < MAX_FACTION; i++)
	{
		sprintf(buf, "%-14s      %-5d    %-8s\r\n", faction_table[i].name, victim->faction_rep[i], get_rank(victim->faction_rank[i]));
		SEND(buf, ch);
	}
	return;
	
}

void do_skillstat(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg);
	
    if ( arg[0] == '\0' )
    {
        SEND( "List skills for whom?\r\n", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        SEND( "They aren't here.\r\n", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        SEND( "Use this for skills on players.\r\n", ch );
        return;
    }

    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
        break;

        if ((level = skill_table[sn].skill_level[victim->ch_class]) < LEVEL_HERO
+ 1
        &&  level >= min_lev && level <= max_lev
        &&  (skill_table[sn].spell_fun == spell_null)
        &&  victim->pcdata->learned[sn] > 0)
        {
            found = TRUE;
            level = skill_table[sn].skill_level[victim->ch_class];
            if (victim->level < level)
                sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
            else
                sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name,
                victim->pcdata->learned[sn]);

            if (skill_list[level][0] == '\0')
                sprintf(skill_list[level],"\r\nLevel %2d: %s",level,buf);
            else /* append */
            {
                if ( ++skill_columns[level] % 2 == 0)
                    strcat(skill_list[level],"\r\n          ");
                    strcat(skill_list[level],buf);
            }
        }
    }

    /* return results */

    if (!found)
    {
        SEND("No skills found.\r\n",ch);
        return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
        if (skill_list[level][0] != '\0')
            add_buf(buffer,skill_list[level]);
            add_buf(buffer,"\r\n");
            page_to_char(buf_string(buffer),ch);
            free_buf(buffer);
}

void do_spellstat(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char buff[100];
    char arg[MAX_INPUT_LENGTH];
    char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO + 1];
    int sn, gn, col, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

	argument = one_argument(argument, arg);
	
    if (IS_NPC(ch))
    return;

    if ( arg[0] == '\0' )
    {
        SEND( "List spells for whom?\r\n", ch );
        return;
    }	
	
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        SEND( "They aren't here.\r\n", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        SEND( "Use this for skills on players.\r\n", ch );
        return;
    }

    /* groups */

    col = 0;

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
        break;
        if (victim->pcdata->group_known[gn])
        {
            sprintf(buff,"%-20s ",group_table[gn].name);
            SEND(buff,ch);
            if (++col % 3 == 0)
                SEND("\r\n",ch);
        }
    }
    if ( col % 3 != 0 )
    {
        SEND( "\r\n", ch );
        sprintf(buff,"Creation points: %d\r\n",victim->pcdata->points);
        SEND(buff,ch);
    }
    
    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        spell_columns[level] = 0;
        spell_list[level][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL )
        break;

        if ((level = skill_table[sn].skill_level[victim->ch_class]) < LEVEL_HERO
+ 1
        &&  level >= min_lev && level <= max_lev
        &&  skill_table[sn].spell_fun != spell_null
        &&  victim->pcdata->learned[sn] > 0)
        {
            found = TRUE;
            level = skill_table[sn].skill_level[victim->ch_class];

            if (victim->level < level)
                sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
            else
            {
                mana = UMAX(skill_table[sn].min_mana,
                100/(2 + victim->level - level));
                sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);
            }

            if (spell_list[level][0] == '\0')
                sprintf(spell_list[level],"\r\nLevel %2d: %s",level,buf);
            else /* append */
            {
                if ( ++spell_columns[level] % 2 == 0)
                strcat(spell_list[level],"\r\n          ");
                strcat(spell_list[level],buf);
            }
        }
    }

    /* return results */

    if (!found)
    {
        SEND("No spells found.\r\n",ch);
        return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
        if (spell_list[level][0] != '\0')
            add_buf(buffer,spell_list[level]);
            add_buf(buffer,"\r\n");
            page_to_char(buf_string(buffer),ch);
            free_buf(buffer);
}


void do_checkmobs (CHAR_DATA *ch, char* argument)
{
	char arg1[MIL], arg2[MIL],
	buf[MIL], name[MIL];
    BUFFER *buffer;
    CHAR_DATA *vch;
    bool found;
    int level = 0, number = 0;
	
	found = FALSE;
    number = 0;
    name[0] = '\0';

    buffer = new_buf();

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( arg1[0] == '\0' )
    {
	SEND("Syntax: checkmob <level or name> <level or name>\r\n", ch);
	return;
    }

    if ( is_number(arg1) )
	level = atoi(arg1);
    else sprintf(name,"%s",arg1);

    if ( arg2[0] != '\0' )
    {
	if ( is_number(arg2) )
	{
	    if ( level ) // User entered 2 level values
	    {
		SEND("Please enter only 1 level value.\r\n", ch);
		return;
	    }
	    else level = atoi(arg2);
	}
	else if ( name[0] != '\0' ) // User entered 2 name values
	{
	    SEND("Please enter only 1 name value.\r\n", ch);
	    return;
	}
	else sprintf(name,"%s",arg2);
    }

    // Now just execute 1 loop
    for ( vch = char_list; vch != NULL; vch = vch->next )
    {
		if (IS_NPC(vch) && (name[0] == '\0' || is_name(name,vch->name))
		&& (!level || vch->level == level))
		{
			number++;
			sprintf(buf," %2d) {r[{x HP: %5d {r] [{x Race: %14s {r] [{x Lvl: %3d {r] [{x VNUM: %5ld {r]{x\r\n",number,vch->hit, race_table[vch->race].name, vch->level, vch->pIndexData->vnum);
			add_buf(buffer,buf);
		}
    }

    if ( buf_string(buffer)[0] == '\0' )
	SEND("No matching mobs found.\r\n", ch);
    else page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
	return;
}

void do_checkweaps(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
buf[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    bool found;
    int level = 0, number = 0;

    found = FALSE;
    number = 0;
    name[0] = '\0';

    buffer = new_buf();

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( arg1[0] == '\0' )
    {
	SEND("Syntax: checkwpn <level or name> <level or name>\r\n", ch);
	return;
    }

    if ( is_number(arg1) )
	level = atoi(arg1);
    else sprintf(name,"%s",arg1);

    if ( arg2[0] != '\0' )
    {
	if ( is_number(arg2) )
	{
	    if ( level ) // User entered 2 level values
	    {
		SEND("Please enter only 1 level value.\r\n",
ch);
		return;
	    }
	    else level = atoi(arg2);
	}
	else if ( name[0] != '\0' ) // User entered 2 name values
	{
	    SEND("Please enter only 1 name value.\r\n", ch);
	    return;
	}
	else sprintf(name,"%s",arg2);
    }

    // Now just execute 1 loop
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if(obj->item_type == ITEM_WEAPON
	//if(obj->wear_loc == WEAR_WIELD && obj->item_type == ITEM_WEAPON
	&& (name[0] == '\0' || is_name(name,obj->name))
	&& (!level || obj->level == level))
	{
	    number++;
	    sprintf(buf," %2d) {r[{x AVG: %3d {r] [{x Type: %10s {r] [{x Lvl: %3d {r] [{x VNUM: %5ld {r]{x   %s\r\n",number,((1 + obj->value[2]) * obj->value[1] / 2), flag_string (weapon_class, obj->value[0]), obj->level,obj->pIndexData->vnum,obj->short_descr);
	    add_buf(buffer,buf);
	}
    }

    if ( buf_string(buffer)[0] == '\0' )
	SEND("No matching weapons found.\r\n", ch);
    else page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
    return;
}


/*
void do_checkarmor(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
buf[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    bool found;
    int level = 0, number = 0;

    found = FALSE;
    number = 0;
    name[0] = '\0';

    buffer = new_buf();

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( arg1[0] == '\0' )
    {
	SEND("Syntax: checkarmor <level or name> <level or name>\r\n", ch);
	return;
    }

    if ( is_number(arg1) )
	level = atoi(arg1);
    else sprintf(name,"%s",arg1);

    if ( arg2[0] != '\0' )
    {
	if ( is_number(arg2) )
	{
	    if ( level ) // User entered 2 level values
	    {
		SEND("Please enter only 1 level value.\r\n",
ch);
		return;
	    }
	    else level = atoi(arg2);
	}
	else if ( name[0] != '\0' ) // User entered 2 name values
	{
	    SEND("Please enter only 1 name value.\r\n", ch);
	    return;
	}
	else sprintf(name,"%s",arg2);
    }

    // Now just execute 1 loop
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if(obj->item_type == ITEM_WEAPON
	//if(obj->wear_loc == WEAR_WIELD && obj->item_type == ITEM_WEAPON
	&& (name[0] == '\0' || is_name(name,obj->name))
	&& (!level || obj->level == level))
	{
	    number++;
	    sprintf(buf," %d ) [ AVG DAM: %3d ] [ LEVEL: %3d ] [ VNUM: %3d ] %s\r\n",number,((1 + obj->value[2]) * obj->value[1] / 2),obj->level,obj->pIndexData->vnum,obj->short_descr);
	    add_buf(buffer,buf);
	}
    }

    if ( buf_string(buffer)[0] == '\0' )
	SEND("No matching weapons found.\r\n", ch);
    else page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
    return;
}
*/
void do_email_pw(CHAR_DATA *ch, char *argument)
{
	//char *pw;
	
	/*char buf[MAX_STRING_LENGTH];	
	if (IS_NPC(ch))
		return;
	
	FILE *ofp;
	ofp = fopen("pwemail","w");
	fprintf(ofp, "Your password for %s is: %s\n", ch->name, ch->pcdata->pwd);
	fclose(ofp);
	
	if (ch->pcdata->email != NULL)	
	{
		sprintf(buf, "mail -v -s \"Password Retrieval - MUD\" %s < pwemail", ch->pcdata->email);	
		system(buf);	
		unlink("pwdemail");
		SEND("Your password has been sent the the email we have on file.\r\n", ch);
		return;
	}	
	else
	{
		SEND("You need to set your email via the finger command first.\r\n",ch);
		return;
	}*/
}


//Check how many open files there are:
void do_fileio(CHAR_DATA *ch, char *argument)
{
	BUFFER *output = new_buf();
	FILE_DATA *filedata;
	char buf[MSL];
	int count = 0;
 
	if(file_list)
	{
		add_buf(output, "Filename       Mode        Opened         Function       Line\r\n");
	}
 
	for(filedata = file_list; filedata; filedata = filedata->next)
	{
		sprintf(buf, "%16s %16s %16s %16s %15d\r\n", filedata->filename, filedata->mode, filedata->file, filedata->function, filedata->line);
		add_buf(output, buf);
	}
 
 
	// *Add to the evil* //
	add_buf(output, "\r\n");
 
	// *Make sure the count is right.* //
	FilesOpen = count;
 
	// *Send it out, and Enjoy* //
	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void do_slist (CHAR_DATA * ch, char * argument)
{
	char arg1[MIL];	
	char arg2[MIL];
	char buf[MSL];
	char buf2[MSL];
	int i = 0, a = 0, col = 0;
	
	argument = one_argument(argument, arg1); 
	argument = one_argument(argument, arg2); 	
	
	if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
		SEND("Syntax: slist <spell/skill> <class name>\r\n", ch);
		return;
    }
	
	if ( !str_cmp(arg1, "spell") )
	{
		int spell_count = 0;
		for (i = 0; i < MAX_CLASS; i++)
		{
			if ( !str_cmp(arg2,ch_class_table[i].name) )
			{
				for (a = 0; a < MAX_SKILL; a++)
				{
					if (skill_table[a].spell_fun != spell_null && skill_table[a].rating[i] > 0)
					{
						spell_count += 1;
						sprintf(buf, "%-28s ", skill_table[a].name);
						SEND( buf, ch);
						if (++col % 3 == 0)
							SEND ("\r\n", ch);
					}
				}
			}
		}
		sprintf(buf2, "\r\nTotal Spells: %d\r\n", spell_count);
		SEND(buf2, ch);
	}
	else if ( !str_cmp(arg1, "skill") )
	{
		int skill_count = 0;
		for (i = 0; i < MAX_CLASS; i++)
		{
			if ( !str_cmp(arg2,ch_class_table[i].name) )
			{
				for (a = 0; a < MAX_SKILL; a++)
				{
					if (skill_table[a].spell_fun == spell_null && skill_table[a].rating[i] > 0)
					{
						skill_count += 1;
						sprintf(buf, "%-28s ", skill_table[a].name);
						SEND( buf, ch);
						if (++col % 3 == 0)
							SEND ("\r\n", ch);
					}
				}
			}
		}
		sprintf(buf2, "\r\nTotal Skills: %d\r\n", skill_count);
		SEND(buf2, ch);
	}
	else
	{
		SEND("You must choose to show either skills or spells.\r\n",ch);
		return;
	}
	return;
}

void do_rename (CHAR_DATA* ch, char* argument)
{
	char old_name[MAX_INPUT_LENGTH],
	     new_name[MAX_INPUT_LENGTH],
	     strsave [MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *obj2, *obj_next, *obj2_next;
	CHAR_DATA* victim;
	FILE* file;

	if(!ch->desc)
	{
		SEND("That person has no link. Must be active for a rename.\r\n",ch);
		return;
	}
	
	argument = one_argument(argument, old_name); /* find new/old name */
	one_argument (argument, new_name);

	/* Trivial checks */
	if (!old_name[0])
	{
		SEND ("Rename who?\r\n",ch);
		return;
	}

	victim = get_char_world (ch, old_name);

	if (!victim)
	{
		SEND ("There is no such a person online.\r\n",ch);
		return;
	}

	if ( victim == ch )
	{
	    SEND( "You can't rename yourself.\r\n",ch);
	    return;
	}

	if (IS_NPC(victim))
	{
		SEND ("You cannot use Rename on NPCs.\r\n",ch);
		return;
	}

	if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
	{
		SEND ("You failed.\r\n",ch);
		return;
	}

	if (!new_name[0])
	{
		SEND ("Rename to what new name?\r\n",ch);
		return;
	}

	if (!check_parse_name(new_name))
	{
		SEND ("The new name is illegal.\r\n",ch);
		return;
	}

	interpret(victim,"save");

	/* First, check if there is a player named that off-line */

	fclose (fpReserve); /* close the reserve file */
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );
    if ( ( file = fopen( strsave, "r" ) ) != NULL )
	{
		SEND ("A player with that name already exists!\r\n",ch);
		fclose (file);
   		fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */
		return;
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	/* Check .gz file ! */
    sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)
	{
		SEND ("A player with that name already exists in a compressed file!\r\n",ch);
		fclose (file);
    		fpReserve = fopen( NULL_FILE, "r" );
		return;
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	if (get_char_world(ch,new_name))
	{
		SEND ("A player with the name you specified already exists!\r\n",ch);
		return;
	}

	/* Save the filename of the old name */

        sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );

	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

            if(IS_NULLSTR(obj->owner))
				continue;

	    if ( str_cmp( obj->owner, victim->name ))
			continue;

	    if ( obj->contains )
	    {
	    	for ( obj2 = obj->contains; obj2 != NULL; obj2 = obj2_next )
    		{
	    	    obj2_next = obj2->next_content;

                    if(IS_NULLSTR(obj2->owner))
                        continue;

                    if ( str_cmp( obj2->owner, victim->name ))
						continue;

    		    free_string(obj2->owner);
	    	    obj2->owner = str_dup (capitalize(new_name));
		    }
	    }

        free_string(obj->owner);
        obj->owner = str_dup (capitalize(new_name));
	}

	/* Rename the character and save him to a new file */

	//Auto-deny old file
	SET_BIT(victim->act, PLR_DENY);
	save_char_obj (victim);

	REMOVE_BIT(victim->act, PLR_DENY);
	free_string(victim->name);
	victim->name = str_dup (capitalize(new_name));
	
	save_char_obj (victim);

	/* That's it! */

	SEND ("Character renamed.\r\n",ch);

	victim->position = POS_STANDING; /* I am laaazy */
	act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);

}


BAN_DATA *ban_list;

void save_bans (void)
{
    BAN_DATA *pban;
    FILE *fp;
    bool found = FALSE;

    fclose (fpReserve);
    if ((fp = fopen (BAN_FILE, "w")) == NULL)
    {
        perror (BAN_FILE);
    }

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
        if (IS_SET (pban->ban_flags, BAN_PERMANENT))
        {
            found = TRUE;
            fprintf (fp, "%-20s %-2d %s\n", pban->name, pban->level,
                     print_flags (pban->ban_flags));
        }
    }

    fclose (fp);
    fpReserve = fopen (NULL_FILE, "r");
    if (!found)
        unlink (BAN_FILE);
}

void load_bans (void)
{
    FILE *fp;
    BAN_DATA *ban_last;

    if ((fp = fopen (BAN_FILE, "r")) == NULL)
        return;

    ban_last = NULL;
    for (;;)
    {
        BAN_DATA *pban;
        if (feof (fp))
        {
            fclose (fp);
            return;
        }

        pban = new_ban ();

        pban->name = str_dup (fread_word (fp));
        pban->level = fread_number (fp);
        pban->ban_flags = fread_flag (fp);
        fread_to_eol (fp);

        if (ban_list == NULL)
            ban_list = pban;
        else
            ban_last->next = pban;
        ban_last = pban;
    }
}

bool check_ban (char *site, int type)
{
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];

    strcpy (host, capitalize (site));
    host[0] = LOWER (host[0]);

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
        if (!IS_SET (pban->ban_flags, type))
            continue;

        if (IS_SET (pban->ban_flags, BAN_PREFIX)
            && IS_SET (pban->ban_flags, BAN_SUFFIX)
            && strstr (pban->name, host) != NULL)
            return TRUE;

        if (IS_SET (pban->ban_flags, BAN_PREFIX)
            && !str_suffix (pban->name, host))
            return TRUE;

        if (IS_SET (pban->ban_flags, BAN_SUFFIX)
            && !str_prefix (pban->name, host))
            return TRUE;
    }

    return FALSE;
}


void ban_site (CHAR_DATA * ch, char *argument, bool fPerm)
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BUFFER *buffer;
    BAN_DATA *pban, *prev;
    bool prefix = FALSE, suffix = FALSE;
    int type;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0')
    {
        if (ban_list == NULL)
        {
            SEND ("No sites banned at this time.\r\n", ch);
            return;
        }
        buffer = new_buf ();

        add_buf (buffer, "Banned sites  level  type     status\r\n");
        for (pban = ban_list; pban != NULL; pban = pban->next)
        {
            sprintf (buf2, "%s%s%s",
                     IS_SET (pban->ban_flags, BAN_PREFIX) ? "*" : "",
                     pban->name,
                     IS_SET (pban->ban_flags, BAN_SUFFIX) ? "*" : "");
            sprintf (buf, "%-12s    %-3d  %-7s  %s\r\n",
                     buf2, pban->level,
                     IS_SET (pban->ban_flags, BAN_NEWBIES) ? "newbies" :
                     IS_SET (pban->ban_flags, BAN_PERMIT) ? "permit" :
                     IS_SET (pban->ban_flags, BAN_ALL) ? "all" : "",
                     IS_SET (pban->ban_flags,
                             BAN_PERMANENT) ? "perm" : "temp");
            add_buf (buffer, buf);
        }

        page_to_char (buf_string (buffer), ch);
        free_buf (buffer);
        return;
    }

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix (arg2, "all"))
        type = BAN_ALL;
    else if (!str_prefix (arg2, "newbies"))
        type = BAN_NEWBIES;
    else if (!str_prefix (arg2, "permit"))
        type = BAN_PERMIT;
    else
    {
        SEND
            ("Acceptable ban types are all, newbies, and permit.\r\n", ch);
        return;
    }

    name = arg1;

    if (name[0] == '*')
    {
        prefix = TRUE;
        name++;
    }

    if (name[strlen (name) - 1] == '*')
    {
        suffix = TRUE;
        name[strlen (name) - 1] = '\0';
    }

    if (strlen (name) == 0)
    {
        SEND ("You have to ban SOMETHING.\r\n", ch);
        return;
    }

    prev = NULL;
    for (pban = ban_list; pban != NULL; prev = pban, pban = pban->next)
    {
        if (!str_cmp (name, pban->name))
        {
            if (pban->level > get_trust (ch))
            {
                SEND ("That ban was set by a higher power.\r\n", ch);
                return;
            }
            else
            {
                if (prev == NULL)
                    ban_list = pban->next;
                else
                    prev->next = pban->next;
                free_ban (pban);
            }
        }
    }

    pban = new_ban ();
    pban->name = str_dup (name);
    pban->level = get_trust (ch);

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
        SET_BIT (pban->ban_flags, BAN_PREFIX);
    if (suffix)
        SET_BIT (pban->ban_flags, BAN_SUFFIX);
    if (fPerm)
        SET_BIT (pban->ban_flags, BAN_PERMANENT);

    pban->next = ban_list;
    ban_list = pban;
    save_bans ();
    sprintf (buf, "%s has been banned.\r\n", pban->name);
    SEND (buf, ch);
    return;
}

void do_ban (CHAR_DATA * ch, char *argument)
{
    ban_site (ch, argument, FALSE);
}

void do_permban (CHAR_DATA * ch, char *argument)
{
    ban_site (ch, argument, TRUE);
}

void do_allow (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Remove which site from the ban list?\r\n", ch);
        return;
    }

    prev = NULL;
    for (curr = ban_list; curr != NULL; prev = curr, curr = curr->next)
    {
        if (!str_cmp (arg, curr->name))
        {
            if (curr->level > get_trust (ch))
            {
                SEND
                    ("You are not powerful enough to lift that ban.\r\n", ch);
                return;
            }
            if (prev == NULL)
                ban_list = ban_list->next;
            else
                prev->next = curr->next;

            free_ban (curr);
            sprintf (buf, "Ban on %s lifted.\r\n", arg);
            SEND (buf, ch);
            save_bans ();
            return;
        }
    }

    SEND ("Site is not banned.\r\n", ch);
    return;
}


void do_flag (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
        arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    const struct flag_type *flag_table;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument (argument, word);

    if (arg1[0] == '\0')
    {
        SEND ("Syntax:\r\n", ch);
        SEND ("  flag mob  <name> <field> <flags>\r\n", ch);
        SEND ("  flag char <name> <field> <flags>\r\n", ch);
        SEND ("  mob  flags: act,aff,off,imm,res,vuln,form,part\r\n",
                      ch);
        SEND ("  char flags: plr,comm,aff,imm,res,vuln,\r\n", ch);
        SEND ("  +: add flag, -: remove flag, = set equal to\r\n",
                      ch);
        SEND ("  otherwise flag toggles the flags listed.\r\n", ch);
        return;
    }

    if (arg2[0] == '\0')
    {
        SEND ("What do you wish to set flags on?\r\n", ch);
        return;
    }

    if (arg3[0] == '\0')
    {
        SEND ("You need to specify a flag to set.\r\n", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        SEND ("Which flags do you wish to change?\r\n", ch);
        return;
    }

    if (!str_prefix (arg1, "mob") || !str_prefix (arg1, "char"))
    {
        victim = get_char_world (ch, arg2);
        if (victim == NULL)
        {
            SEND ("You can't find them.\r\n", ch);
            return;
        }

        /* select a flag to set */
        if (!str_prefix (arg3, "act"))
        {
            if (!IS_NPC (victim))
            {
                SEND ("Use plr for PCs.\r\n", ch);
                return;
            }

            flag = &victim->act;
            flag_table = act_flags;
        }

        else if (!str_prefix (arg3, "plr"))
        {
            if (IS_NPC (victim))
            {
                SEND ("Use act for NPCs.\r\n", ch);
                return;
            }

            flag = &victim->act;
            flag_table = plr_flags;
        }

        else if (!str_prefix (arg3, "aff"))
        {
            flag = &victim->affected_by;
            flag_table = affect_flags;
        }

        else if (!str_prefix (arg3, "immunity"))
        {
            flag = &victim->imm_flags;
            flag_table = imm_flags;
        }

        else if (!str_prefix (arg3, "resist"))
        {
            flag = &victim->res_flags;
            flag_table = imm_flags;
        }

        else if (!str_prefix (arg3, "vuln"))
        {
            flag = &victim->vuln_flags;
            flag_table = imm_flags;
        }

        else if (!str_prefix (arg3, "form"))
        {
            if (!IS_NPC (victim))
            {
                SEND ("Form can't be set on PCs.\r\n", ch);
                return;
            }

            flag = &victim->form;
            flag_table = form_flags;
        }

        else if (!str_prefix (arg3, "parts"))
        {
            if (!IS_NPC (victim))
            {
                SEND ("Parts can't be set on PCs.\r\n", ch);
                return;
            }

            flag = &victim->parts;
            flag_table = part_flags;
        }

        else if (!str_prefix (arg3, "comm"))
        {
            if (IS_NPC (victim))
            {
                SEND ("Comm can't be set on NPCs.\r\n", ch);
                return;
            }

            flag = &victim->comm;
            flag_table = comm_flags;
        }

        else
        {
            SEND ("That's not an acceptable flag.\r\n", ch);
            return;
        }

        old = *flag;
        victim->zone = NULL;

        if (type != '=')
            new = old;

        /* mark the words */
        for (;;)
        {
            argument = one_argument (argument, word);

            if (word[0] == '\0')
                break;

            pos = flag_lookup (word, flag_table);

            if (pos == NO_FLAG)
            {
                SEND ("That flag doesn't exist!\r\n", ch);
                return;
            }
            else
                SET_BIT (marked, pos);
        }

        for (pos = 0; flag_table[pos].name != NULL; pos++)
        {
            if (!flag_table[pos].settable
                && IS_SET (old, flag_table[pos].bit))
            {
                SET_BIT (new, flag_table[pos].bit);
                continue;
            }

            if (IS_SET (marked, flag_table[pos].bit))
            {
                switch (type)
                {
                    case '=':
                    case '+':
                        SET_BIT (new, flag_table[pos].bit);
                        break;
                    case '-':
                        REMOVE_BIT (new, flag_table[pos].bit);
                        break;
                    default:
                        if (IS_SET (new, flag_table[pos].bit))
                            REMOVE_BIT (new, flag_table[pos].bit);
                        else
                            SET_BIT (new, flag_table[pos].bit);
                }
            }
        }
        *flag = new;
        return;
    }
}



void do_checktype(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    bool found;
    int level = 0, number = 0, wearloc;
	int itemtype = 0;
 
    found = FALSE;
    number = 0;
    name[0] = '\0';
 
    buffer = new_buf();
 
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
 
    if ( arg1[0] == '\0' )
    {
        SEND("Syntax: checktype <level or type> <level or type>\r\n", ch);
        SEND("        Type can be one of the following:\r\n",ch);
        SEND("              Light       Finger\r\n",ch);
        SEND("              Neck        Body\r\n",ch);
        SEND("              Head        Legs\r\n",ch);
        SEND("              Feet        Hands\r\n",ch);
        SEND("              Arms        Shield\r\n",ch);
        SEND("              About       Waist\r\n",ch);
        SEND("              Wrist       Hold\r\n",ch);
        SEND("              Floating    Wield\r\n",ch);
		SEND("              Wand        Scroll\r\n", ch);
		SEND("              Potion      Alchemy_Recipe\r\n", ch);
		SEND("              Food        Cooking_Recipe\r\n", ch);
        return;
    }
 
    if ( is_number(arg1) )
        level = atoi(arg1);
    else sprintf(name,"%s",arg1);
 
    if ( arg2[0] != '\0' )
    {
        if ( is_number(arg2) )
        {
            if ( level ) // User entered 2 level values
            {
                SEND("Please enter only 1 level value.\r\n",ch);
                return;
            }
            else level = atoi(arg2);
        }
        else if ( name[0] != '\0' ) // User entered 2 name values
        {
            SEND("Please enter only 1 name value.\r\n", ch);
            return;
        }
        else sprintf(name,"%s",arg2);
    }
        if ( !str_cmp( name, "light" ))
                wearloc = WEAR_LIGHT;
        else if ( !str_cmp( name, "finger" ))
                wearloc = WEAR_FINGER_L;
        else if ( !str_cmp( name, "neck" ))
                wearloc = WEAR_NECK_1;
        else if ( !str_cmp( name, "body" ))
                wearloc = WEAR_BODY;
        else if ( !str_cmp( name, "head" ))
                wearloc = WEAR_HEAD;
        else if ( !str_cmp( name, "legs" ))
                wearloc = WEAR_LEGS;
        else if ( !str_cmp( name, "feet" ))
                wearloc = WEAR_FEET;
        else if ( !str_cmp( name, "hands" ))
                wearloc = WEAR_HANDS;
        else if ( !str_cmp( name, "arms" ))
                wearloc = WEAR_ARMS;
        else if ( !str_cmp( name, "arms" ))
                wearloc = WEAR_ARMS;
        else if ( !str_cmp( name, "shield" ))
                wearloc = WEAR_SHIELD;
        else if ( !str_cmp( name, "about" ))
                wearloc = WEAR_ABOUT;
        else if ( !str_cmp( name, "waist" ))
                wearloc = WEAR_WAIST;
        else if ( !str_cmp( name, "wrist" ))
                wearloc = WEAR_WRIST_L;
        else if ( !str_cmp( name, "hold" ))
                wearloc = WEAR_HOLD;
        else if ( !str_cmp( name, "float" ))
                wearloc = WEAR_FLOAT;
        else if ( !str_cmp( name, "wield" ))
                wearloc = WEAR_WIELD;
		else if ( !str_cmp( name, "wand" ))
		{
				wearloc = -1;
				itemtype = ITEM_WAND;
		}
		else if ( !str_cmp( name, "scroll" ))
		{
				wearloc = -1;
				itemtype = ITEM_SCROLL;
		}
		else if ( !str_cmp( name, "potion" ))
		{
				wearloc = -1;
				itemtype = ITEM_POTION;
		}
		else if ( !str_cmp( name, "alchemy_recipe"))
		{
				wearloc = -1;
				itemtype = ITEM_ALCHEMY_RECIPE;
		}
		else if ( !str_cmp( name, "cooking_recipe" ))
		{
				wearloc = -1;
				itemtype = ITEM_COOKING_RECIPE;
		}
		else if ( !str_cmp( name, "food" ))
		{
			wearloc = -1;
			itemtype = ITEM_FOOD;
		}
        else
        {
                SEND("Syntax: checktype <level or type> <level or type>\r\n", ch);
                SEND("        Type can be one of the following:\r\n",ch);
                SEND("              Light       Finger\r\n",ch);
                SEND("              Neck        Body\r\n",ch);
                SEND("              Head        Legs\r\n",ch);
                SEND("              Feet        Hands\r\n",ch);
                SEND("              Arms        Shield\r\n",ch);
                SEND("              About       Waist\r\n",ch);
                SEND("              Arms        Shield\r\n",ch);
                SEND("              About       Waist\r\n",ch);
                SEND("              Wrist       Hold\r\n",ch);
                SEND("              Floating    Wield\r\n",ch);
				SEND("              Wand        Scroll\r\n", ch);
				SEND("              Potion      Alchemy_Recipe\r\n", ch);
				SEND("              Food        Cooking_Recipe\r\n", ch);
                return;
        }
    // Now just execute 1 loop
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
				if (wearloc > -1)
				{
					if(obj->wear_loc == wearloc     && (!level || obj->level == level))
					{
						number++;
						sprintf(buf," %d ) [ LEVEL: %3d ] %s (%ld)\r\n",number,obj->level,obj->short_descr,obj->pIndexData->vnum);
						add_buf(buffer,buf);
					}
				}
				else
				{
					if (obj->item_type == itemtype)
					{
						number++;
						sprintf(buf," %d ) [ LEVEL: %3d ] %s (%ld)\r\n",number,obj->level,obj->short_descr,obj->pIndexData->vnum);
						add_buf(buffer,buf);
					}
				}
				
    }
 
    if ( buf_string(buffer)[0] == '\0' )
        SEND("Nothing matching the given criteria found.\r\n", ch);
    else page_to_char(buf_string(buffer),ch);
 
    free_buf(buffer);
    return;
}

void do_doublexp( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char arg1[MIL];
	char buf[MSL];
    int amount;
 
    argument = one_argument( argument, arg );
    one_argument( argument, arg1 ); 
 
    if ( arg[0] == '\0' )
    {
        SEND("Syntax: double <on|off> ticks.\r\n",ch);
        return;
    }
 
    if (!str_cmp(arg, "on"))
    {
        if ( arg1[0] == '\0' || !is_number( arg1 ) )
        {
            SEND("You need to apply the number of ticks.\r\n", ch );
            return;
        }
 
        if (double_exp)
        {
            SEND("Double exp is already in affect!\r\n",ch);
            return;
        }
 
        amount = atoi( arg1 );
 
        if ( amount > 500 )
        {
			if (amount < 0 && total_levels(ch) < MAX_LEVEL)
			{
				SEND( "Please choose an amount between 0 and 500.\r\n", ch );
				return;
			}
        }
 
        if (amount < 0)
			global_exp = 99999;
		else
			global_exp = amount;
        double_exp = TRUE;
        sprintf(buf, "{r[{xINFO{r]{x:  %s has declared %d ticks of double experience for everyone.\r\n", ch->name, amount < 0 ? 99999 : amount);
		do_function(ch, &do_echo, buf);
        SEND("Double exp is now in affect!\r\n",ch);
        return;
    }                
 
    if (!str_cmp(arg, "off"))
    {
        if (!double_exp)
        {
            SEND("Double exp is not on please turn it on first!\r\n",ch);
            return;
        }
        double_exp = FALSE;
        global_exp = 0;
		sprintf(buf, "{r[{xINFO{r]{x:  %s has removed the double experience bonus.\r\n", ch->name);
		do_function(ch, &do_echo, buf);        
        SEND( "You have turned off double exp!\r\n", ch );
        return;             
    }
}


void do_double_gold( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char arg1[MIL];
	char buf[MSL];
    int amount;
 
    argument = one_argument( argument, arg );
    one_argument( argument, arg1 ); 
 
    if ( arg[0] == '\0' )
    {
        SEND("Syntax: double <on|off> ticks.\r\n",ch);
        return;
    }
 
    if (!str_cmp(arg, "on"))
    {
        if ( arg1[0] == '\0' || !is_number( arg1 ) )
        {
            SEND("You need to apply the number of ticks.\r\n", ch );
            return;
        }
 
        if (double_gold)
        {
            SEND("Double gold is already in affect!\r\n",ch);
            return;
        }
 
        amount = atoi( arg1 );
 
        if ( amount > 500 )
        {
			if (amount < 0 && total_levels(ch) < MAX_LEVEL)
			{
				SEND( "Please choose an amount between 0 and 500.\r\n", ch );
				return;
			}
        }
 
        if (amount < 0)
			global_gold = 999;
		else if (amount > 999)
			global_gold = 999;
		else
			global_gold = amount;
        double_gold = TRUE;
        sprintf(buf, "{r[{xINFO{r]{x:  %s has declared %d ticks of double {Ygold{x for everyone.\r\n", ch->name, amount < 0 ? 999 : amount);
		do_function(ch, &do_echo, buf);
        SEND("Double gold is now in affect!\r\n",ch);
        return;
    }                
 
    if (!str_cmp(arg, "off"))
    {
        if (!double_gold)
        {
            SEND("Double gold is not on please turn it on first!\r\n",ch);
            return;
        }
        double_gold = FALSE;
        global_gold = 0;
		sprintf(buf, "{r[{xINFO{r]{x:  %s has removed the double {Ygold{x bonus.\r\n", ch->name);
		do_function(ch, &do_echo, buf);        
        SEND( "You have turned off double gold!\r\n", ch );
        return;             
    }
}

void do_double_skill( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char arg1[MIL];
	char buf[MSL];
    int amount;
 
    argument = one_argument( argument, arg );
    one_argument( argument, arg1 ); 
 
    if ( arg[0] == '\0' )
    {
        SEND("Syntax: double <on|off> ticks.\r\n",ch);
        return;
    }
 
    if (!str_cmp(arg, "on"))
    {
        if ( arg1[0] == '\0' || !is_number( arg1 ) )
        {
            SEND("You need to apply the number of ticks.\r\n", ch );
            return;
        }
 
        if (double_skill)
        {
            SEND("Double skill is already in affect!\r\n",ch);
            return;
        }
 
        amount = atoi( arg1 );
 
        if ( amount > 500 )
        {
			if (amount < 0 && total_levels(ch) < MAX_LEVEL)
			{
				SEND( "Please choose an amount between 0 and 500.\r\n", ch );
				return;
			}
        }
 
        if (amount < 0 || amount > 999)
			global_skill = 999;
		else
			global_skill = amount;
        double_skill = TRUE;
        sprintf(buf, "{r[{xINFO{r]{x:  %s has declared %d ticks of double skill improvement for everyone.\r\n", ch->name, amount < 0 ? 999 : amount);
		do_function(ch, &do_echo, buf);
        SEND("Double skill is now in affect!\r\n",ch);
        return;
    }                
 
    if (!str_cmp(arg, "off"))
    {
        if (!double_skill)
        {
            SEND("Double skill is not on please turn it on first!\r\n",ch);
            return;
        }
        double_skill = FALSE;
        global_skill = 0;
		sprintf(buf, "{r[{xINFO{r]{x:  %s has removed the double skill improvement bonus.\r\n", ch->name);
		do_function(ch, &do_echo, buf);        
        SEND( "You have turned off double skill!\r\n", ch );
        return;             
    }
}


void do_autodig(CHAR_DATA *ch, char *argument)
{
	if(IS_SET(ch->act, PLR_AUTODIG))
	{
		SEND("Autodig is now disabled.\r\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTODIG);
		return;
	}
 
	SEND("Autodig is now enabled.\r\n", ch);
	SET_BIT(ch->act, PLR_AUTODIG);
	return;
}




void do_rat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    int Start, End, vnum;

    //set_char_color( AT_IMMORT, ch );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
		SEND( "Syntax: rat <start> <end> <command>\r\n", ch );
		return;
    }

    Start = atoi( arg1 );	End = atoi( arg2 );
    if ( Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUM )
    {
		SEND( "Invalid range.\r\n", ch );
		return;
    }
    if ( !str_cmp( argument, "quit" ) || !str_cmp( argument, "delete" ) )
    {
		SEND( "Seriously? What an ass.\r\n", ch );
		return;
    }

    original = ch->in_room;
    for ( vnum = Start; vnum <= End; vnum++ )
    {
		if ( (location = get_room_index(vnum)) == NULL )
		  continue;
		char_from_room( ch );
		char_to_room( ch, location );
		interpret( ch, argument );
    }

    char_from_room( ch );
    char_to_room( ch, original );
    SEND( "Done.\r\n", ch );
    return;
}



void do_auto_shutdown()
{

/*This allows for a shutdown without somebody in-game actually calling it.
		-Ferric*/
    //FILE *fp;
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    merc_down = TRUE;

    /* This is to write to the file. */
    fclose(fpReserve);
    //if((fp = fopen(LAST_COMMAND,"a")) == NULL)
    //  bug("Error in do_auto_save opening last_command.txt",0);
   
    //  fprintf(fp,"%s\n",
    //        last_command);

    //fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    for ( d = descriptor_list; d != NULL; d = d_next)
      {
      if(d->character)
	  do_save (d->character, "");
      d_next = d->next;
      close_socket(d);
      }
    return;
}

void do_gweather(CHAR_DATA *ch, char *argument)
{
	AREA_DATA * pArea;	
	char buf[MSL];
	char windbuf[MSL];

    static char *const clouds[4] = 
	{
        "cloudless",
        "partly cloudy",
        "cloudy",
        "heavily cloudy"
    };

	static char *const precip[4] = 
	{		
		"raining",
		"sleeting",
		"snowing",
		"hailing"
	};
	
	static char *const amount[5] = 
	{
		"very lightly",
		"lightly",
		"moderately",
		"heavily",
		"very heavily"
	};
	
	static char *const wind[6] =
	{
		"There seems to be no wind blowing whatsoever",
		"There is a slight breeze",
		"There is a moderate wind",
		"The wind is blowing steadily",		
		"There are heavy winds passing through",
		"The wind is blowing incredibly hard"
	};
	
		static char *const temp[9] =
	{
		"feels very hot",
		"is fairly hot",
		"is rather warm",		
		"is slightly chilly",
		"is quite chilly",
		"is cold",
		"feels very cold",
		"is freezing",
		"is DANGEROUSLY cold"		
	};   
	
	bool linked = FALSE;
	
	if (!str_cmp(argument, "linked"))
		linked = TRUE;
	
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		if (!IS_SET(pArea->area_flags, AREA_LINKED) && linked)
			continue;
			
		sprintf (buf, "---{B%s{x---\r\n", pArea->name);
		SEND(buf, ch);
		sprintf (buf, "The sky is %s. ", clouds[pArea->curr_cloud]);
		SEND(buf, ch);
		if (pArea->curr_precip_amount > -1)
		{
			sprintf(buf, "It is currently %s %s.\r\n", amount[pArea->curr_precip_amount], precip[pArea->curr_precip]);
		}
		else
			sprintf(buf, "There appears to be no current precipitation.\r\n");
		SEND (buf, ch);
		
		if (pArea->curr_wind == 0)
			sprintf(windbuf, "%s and the temperature ", wind[0]);
		else if (pArea->curr_wind <= 10 && pArea->curr_wind > 0)
			sprintf(windbuf, "%s and the temperature ", wind[1]);
		else if (pArea->curr_wind > 10 && pArea->curr_wind <= 20)
			sprintf(windbuf, "%s and the temperature ", wind[2]);
		else if (pArea->curr_wind > 20 && pArea->curr_wind <= 30)
			sprintf(windbuf, "%s and the temperature ", wind[3]);
		else if (pArea->curr_wind > 30 && pArea->curr_wind <= 50)
			sprintf(windbuf, "%s and the temperature ", wind[4]);
		else
			sprintf(windbuf, "%s and the temperature ", wind[5]);
		
		SEND(windbuf,ch);
		
		if (pArea->curr_temp >= 95)
			sprintf(buf, "%s.\r\n", temp[0]);
		else if (pArea->curr_temp < 95 && pArea->curr_temp >= 85)
			sprintf(buf, "%s.\r\n", temp[1]);
		else if (pArea->curr_temp < 85 && pArea->curr_temp >= 70)
			sprintf(buf, "%s.\r\n", temp[2]);
		else if (pArea->curr_temp < 70 && pArea->curr_temp >= 60)
			sprintf(buf, "%s.\r\n", temp[3]);
		else if (pArea->curr_temp < 60 && pArea->curr_temp >= 45)
			sprintf(buf, "%s.\r\n", temp[4]);
		else if (pArea->curr_temp < 45 && pArea->curr_temp >= 35)
			sprintf(buf, "%s.\r\n", temp[5]);
		else if (pArea->curr_temp < 35 && pArea->curr_temp >= 25)
			sprintf(buf, "%s.\r\n", temp[6]);
		else if (pArea->curr_temp < 25 && pArea->curr_temp >= 10)
			sprintf(buf, "%s.\r\n", temp[7]);
		else
			sprintf(buf, "%s.\r\n", temp[8]);
		
		SEND(buf, ch);
		SEND("\r\n",ch);
		
		buf[0] = '\0';
		windbuf[0] = '\0';
		free_string(buf);
		free_string(windbuf);
	}
	return;
}


void do_enchant_list(CHAR_DATA *ch, char *argument)
{
	int i = 0;
	char buf[MSL];
	for (i = 0; i <= MAX_ENCHANTS; i++)
	{
		//sprintf(buf, "%-20s %-12s %-3d %-8s\r\n", enchant_table[i].name, apply_flags[enchant_table[i].apply].name, enchant_table[i].amt, where_name2[enchant_table[i].slot]);		
		sprintf(buf, "%-35s %-12s %-3d\r\n", enchant_table[i].name, apply_flags[enchant_table[i].apply].name, enchant_table[i].amt);		
		SEND(buf, ch);
	}
	
}