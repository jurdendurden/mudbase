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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"


void do_gain (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
	char buf2[MSL];	
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int sn = 0;
	
    if (IS_NPC (ch))
        return;

    /* find a trainer */
    for (trainer = ch->in_room->people;
         trainer != NULL; trainer = trainer->next_in_room)
        if (IS_NPC (trainer) && IS_SET (trainer->act, ACT_GAIN))
            break;

    if (trainer == NULL || !can_see (ch, trainer))
    {
        SEND ("You can't do that here.\r\n", ch);
        return;
    }

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND("Syntax: gain <list/skill name/convert>\r\n",ch);
        return;
    }

    if (!str_prefix (arg, "list"))
    {
        int col;
		char * color;
		char * rank;

        col = 0;

        sprintf (buf, "%-22s %-8s %-9s %-14s %-22s %-8s %-9s %-14s\n",
                 "{Bskill{x", "{Bcost{x", "{Blevel{x", "{Brank{x", "{Bskill{x", "{Bcost{x", "{Blevel{x", "{Brank{x");
        SEND (buf, ch);

        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
			
				switch (skill_table[sn].guildrank)
				{
				case 0:
					rank = "initiate";
					break;
				case 1:
					rank = "apprentice";
					break;
				case 2:
					rank = "member";
					break;
				case 3:
					rank = "officer";
					break;
				case 4:
					rank = "master";
					break;
				default:
					rank = "";
					break;
				}
			if (!IS_MCLASSED(ch))
			{
				if (!ch->pcdata->learned[sn]
					&& skill_table[sn].rating[ch-> ch_class] > 0
					&& skill_table[sn].spell_fun == spell_null)					
				{
					if (skill_table[sn].guildrank > ch->guildrank)
						color = "{D";
					else
						color = "{x";
					sprintf (buf, "%s%-18s{x %-4d %-5d %-11s",
							 color,
							 skill_table[sn].name, 
							 skill_table[sn].rating[ch-> ch_class], skill_table[sn].skill_level[ch-> ch_class], rank);
					SEND (buf, ch);
					if (++col % 2 == 0)
						SEND ("\r\n", ch);
				}
			}
			else 
			{
				if (!ch->pcdata->learned[sn]
					&& (skill_table[sn].rating[ch-> ch_class] > 0)
					&& skill_table[sn].spell_fun == spell_null
					&& skill_table[sn].guildrank <= ch->guildrank && skill_table[sn].skill_level[ch-> ch_class] <= MCLASS_ONE)
				{
					if (skill_table[sn].guildrank > ch->guildrank)
						color = "{D";
					else
						color = "{x";
					sprintf (buf, "%s%-18s{x %-4d %-5d %-11s",
							 color,
							 skill_table[sn].name, 
							 skill_table[sn].rating[ch-> ch_class], skill_table[sn].skill_level[ch-> ch_class], rank);
					SEND (buf, ch);
					if (++col % 2 == 0)
						SEND ("\r\n", ch);
				}
				else if (!ch->pcdata->learned[sn]
					&& (skill_table[sn].rating[ch-> ch_class] < 1 && skill_table[sn].rating[ch-> ch_class2] > 0)
					&& skill_table[sn].spell_fun == spell_null
					&& skill_table[sn].guildrank <= ch->guildrank && skill_table[sn].skill_level[ch-> ch_class2] <= MCLASS_TWO)
				{
					if (skill_table[sn].guildrank > ch->guildrank)
						color = "{D";
					else
						color = "{x";
					sprintf (buf, "%s%-18s{x %-4d %-5d %-11s",
							 color,
							 skill_table[sn].name, 
							 skill_table[sn].rating[ch-> ch_class2], skill_table[sn].skill_level[ch-> ch_class2], rank);
					SEND (buf, ch);
					if (++col % 2 == 0)
						SEND ("\r\n", ch);
				}
				else
				{
					continue;
				}
			}			
				
        }
        if (col % 3 != 0)
            SEND ("\r\n", ch);
		
		///////////////////////////////////////////////////////////////////
		///																///
		///				Now do the spells								///
		///																///
		///////////////////////////////////////////////////////////////////
		
		col = 0;
		SEND ("\r\n",ch);
        sprintf (buf, "%-26s %-8s %-9s %-14s %-26s %-8s %-9s %-14s\n",
                 "{Bspell{x", "{Bcost{x", "{Blevel{x", "{Brank{x", "{Bspell{x", "{Bcost{x", "{Blevel{x", "{Brank{x");
        SEND (buf, ch);

        for (sn = 0; sn < MAX_SKILL; sn++)
        {
			switch (skill_table[sn].guildrank)
			{
			case 0:
				rank = "initiate";
				break;
			case 1:
				rank = "apprentice";
				break;
			case 2:
				rank = "member";
				break;
			case 3:
				rank = "officer";
				break;
			case 4:
				rank = "master";
				break;
			default:
				rank = "";
				break;
			}
		
            if (skill_table[sn].name == NULL)
                break;
			if (!IS_MCLASSED(ch))
			{
				if (!ch->pcdata->learned[sn]
					&& can_cast_spell(ch, sn)						//they can master it.
					&& skill_table[sn].rating[ch-> ch_class] > 0		//they're ch_class can do it.
					&& skill_table[sn].spell_fun != spell_null		//it's actually a spell.
					&& skill_table[sn].spell_level > -1)			//valid spell level.
				{
						
					if (skill_table[sn].guildrank > ch->guildrank)	//need more time in the guild.
						color = "{D";
					else
						color = "{x";
					sprintf (buf, "%s%-22s{x %-4d %-5d %-11s",
							 color,
							 skill_table[sn].name,
							 skill_table[sn].rating[ch-> ch_class], 
							 skill_table[sn].spell_level, rank);
					SEND (buf, ch);
					if (++col % 2 == 0)
						SEND ("\r\n", ch);
				}
			}
			else 	//Player is m ch_classed.
			{									
				if (!ch->pcdata->learned[sn]
					&& can_cast_spell(ch, sn)						//they can master it.
					&& (skill_table[sn].rating[ch-> ch_class] > 0 || skill_table[sn].rating[ch-> ch_class2] > 0)
					&& skill_table[sn].spell_fun != spell_null
					&& skill_table[sn].spell_level > -1)			//valid spell level.
				{
					if (skill_table[sn].guildrank > ch->guildrank)	//need more time in the guild.
						color = "{D";
					else
						color = "{x";
				
					sprintf (buf, "%s%-22s{x %-4d %-5d %-11s",
							color,
							skill_table[sn].name,							 
							IS_SECONDARY_SKILL(ch,sn) ? skill_table[sn].rating[ch-> ch_class2] : skill_table[sn].rating[ch-> ch_class],							 
							skill_table[sn].spell_level,
							rank);
					SEND (buf, ch);
					if (++col % 2 == 0)
						SEND ("\r\n", ch);
				}				
				else
				{
					continue;
				}
			}
        }
        if (col % 2 != 0)
            SEND ("\r\n\r\n", ch);
		sprintf ( buf2, "\r\nYou currently have %d train(s).\r\n", ch->train);
		SEND(buf2,ch);
        return;
    }

    if (!str_prefix (arg, "convert"))
    {
        if (ch->practice < 10)
        {
            act ("$N {Ctells{x you, 'You are not yet ready.'",
                 ch, NULL, trainer, TO_CHAR);
            return;
        }

        act ("$N helps you apply your practice to training.",
             ch, NULL, trainer, TO_CHAR);
        ch->practice -= 10;
		ch->guildpoints += number_range(10,20);
        ch->train += 1;
        return;
    }
	
	if (!str_prefix (arg, "revert"))
    {
        if (ch->train < 1)
        {
            act ("$N {Ctells{x you, 'You are not yet ready.'",
                 ch, NULL, trainer, TO_CHAR);
            return;
        }

        act ("$N helps you apply your training to practice.",
             ch, NULL, trainer, TO_CHAR);
        ch->practice += 10;
        ch->train -= 1;
        return;
    }


    sn = skill_lookup (argument);
    if (sn > -1)
    {        
        if (ch->pcdata->learned[sn])
        {
            act ("$N {Ctells{x you, 'You already know that skill!'",
                 ch, NULL, trainer, TO_CHAR);
            return;
        }

		if (!IS_MCLASSED(ch))
		{
			if (skill_table[sn].rating[ch-> ch_class] <= 0)
			{
				act ("$N {Ctells{x you, 'That skill is beyond your powers.'",
					 ch, NULL, trainer, TO_CHAR);
				return;
			}
			if (skill_table[sn].guildrank > ch->guildrank)
			{
				act ("$N {Ctells{x you, 'You must advance in guild rank first.'", ch, NULL, trainer, TO_CHAR);
				return;
			}
			if (ch->train < skill_table[sn].rating[ch-> ch_class])
			{
				act ("$N {Ctells{x you, 'You are not yet ready for that skill.'",
					 ch, NULL, trainer, TO_CHAR);
				return;
			}
		}
		else
		{
			if (skill_table[sn].rating[ch-> ch_class] <= 0 && skill_table[sn].rating[ch-> ch_class2] <= 0 )
			{
				act ("$N {Ctells{x you, 'That skill is beyond your powers.'",
					 ch, NULL, trainer, TO_CHAR);
				return;
			}
			if (skill_table[sn].guildrank > ch->guildrank)
			{
				act ("$N {Ctells{x you, 'You must advance in guild rank first.'", ch, NULL, trainer, TO_CHAR);
				return;
			}			
			bool ready = FALSE;
			
			if (IS_PRIMARY_SKILL(ch, sn) && ch->train < skill_table[sn].rating[ch-> ch_class])
				ready = FALSE;
			else if (IS_PRIMARY_SKILL(ch, sn) && ch->train >= skill_table[sn].rating[ch-> ch_class])
				ready = TRUE;
			else if (IS_SECONDARY_SKILL(ch, sn) && ch->train < skill_table[sn].rating[ch-> ch_class2])
				ready = FALSE;
			else if (IS_SECONDARY_SKILL(ch, sn) && ch->train >= skill_table[sn].rating[ch-> ch_class2])
				ready = TRUE;
			else if (IS_CROSS_SKILL(ch, sn))
			{
				if (skill_table[sn].rating[ch-> ch_class] > skill_table[sn].rating[ch-> ch_class2])
				{
					if (ch->train < skill_table[sn].rating[ch-> ch_class2])
						ready = FALSE;
					else
						ready = TRUE;
				}
				else
				{
					if (ch->train < skill_table[sn].rating[ch-> ch_class])
						ready = FALSE;
					else
						ready = TRUE;
				}
			}
			else
			{
				ready = FALSE;
			}
			
			if (!ready)
			{
				act ("$N {Ctells{x you, 'You are not yet ready for that skill.'",
					 ch, NULL, trainer, TO_CHAR);
				return;
			}
		}
		
        
		
		if (skill_table[sn].requisite != NULL)
		{
			if (!ch->pcdata->learned[*skill_table[sn].requisite])
			{
				act ("$N {Ctells{x you, 'You must learn the pre-requisite skill first!'",
					ch, NULL, trainer, TO_CHAR);
				return;
			}
			if (ch->pcdata->learned[*skill_table[sn].requisite] < 90)
			{
				act ("$N {Ctells{x you, 'You must master the pre-requisite skill first!'",
					ch, NULL, trainer, TO_CHAR);
				return;
			}
		}
		
        /* add the skill */
        ch->pcdata->learned[sn] = 1;
		ch->pcdata->seen_improve[sn] = TRUE;
		ch->guildpoints += number_range(8,12);
        act ("$N trains you in the art of $t.",
             ch, skill_table[sn].name, trainer, TO_CHAR);
        if (IS_MCLASSED(ch) && skill_table[sn].rating[ch-> ch_class] <= 0)
			ch->train -= skill_table[sn].rating[ch-> ch_class2];
		else
			ch->train -= skill_table[sn].rating[ch-> ch_class];
        return;
    }

    act ("$N {Ctells{x you, 'I do not understand...'", ch, NULL, trainer, TO_CHAR);
}


bool can_use_skill (CHAR_DATA *ch, int sn) //checks level of char against skill level needed
{


	if (!IS_MCLASSED(ch))
	{
		if (ch->level >= skill_table[sn].skill_level[ch->ch_class])
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		if (IS_PRIMARY_SKILL(ch,sn))
		{
			if (ch->level >= skill_table[sn].skill_level[ch->ch_class])
				return TRUE;
			else
				return FALSE;
		}
		else if (IS_SECONDARY_SKILL(ch,sn))
		{
			if (ch->level2 >= skill_table[sn].skill_level[ch->ch_class2])
				return TRUE;
			else
				return FALSE;
		}
		else if (IS_CROSS_SKILL(ch,sn))
		{
			if (total_levels(ch) >= skill_table[sn].skill_level[ch->ch_class])
				return TRUE;
			else
				return FALSE;
		}
		
	}
		
	return FALSE;
}

void do_skills (CHAR_DATA * ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
    bool fAll = FALSE, found = FALSE;
    char buf[MAX_STRING_LENGTH];
	
    if (IS_NPC (ch))
        return;

    if (argument[0] != '\0')
    {
        fAll = TRUE;

        if (str_prefix (argument, "all"))
        {
            argument = one_argument (argument, arg);
            if (!is_number (arg))
            {
                SEND ("Arguments must be numerical or all.\r\n", ch);
                return;
            }
            max_lev = atoi (arg);

            if (max_lev < 1 || max_lev > LEVEL_HERO)
            {
                sprintf (buf, "Levels must be between 1 and %d.\r\n",
                         LEVEL_HERO);
                SEND (buf, ch);
                return;
            }

            if (argument[0] != '\0')
            {
                argument = one_argument (argument, arg);
                if (!is_number (arg))
                {
                    SEND ("Arguments must be numerical or all.\r\n",
                                  ch);
                    return;
                }
                min_lev = max_lev;
                max_lev = atoi (arg);

                if (max_lev < 1 || max_lev > LEVEL_HERO)
                {
                    sprintf (buf,
                             "Levels must be between 1 and %d.\r\n",
                             LEVEL_HERO);
                    SEND (buf, ch);
                    return;
                }

                if (min_lev > max_lev)
                {
                    SEND ("That would be silly.\r\n", ch);
                    return;
                }
            }
        }
    }


    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;		
			
        if (((level = skill_table[sn].skill_level[ch->ch_class]) < (LEVEL_HERO + 1))			
            && (fAll || can_use_skill(ch,sn))
            && level >= min_lev && level <= max_lev
            && skill_table[sn].spell_fun == spell_null
            && ch->pcdata->learned[sn] > 0
			&& sn != gsn_recall)
        {
            found = TRUE;
			if (!IS_MCLASSED(ch) || IS_PRIMARY_SKILL(ch,sn))
			{
				level = skill_table[sn].skill_level[ch->ch_class];
				
				if (ch->level < level)
				{
					sprintf (buf, "%16s n/a ",skill_table[sn].name);					
				}
				else
				{
					sprintf (buf, "%16s %s%3d{x%% ", skill_table[sn].name, ch->pcdata->seen_improve[sn] ? "" : "{G",ch->pcdata->learned[sn]);	
					ch->pcdata->seen_improve[sn] = TRUE;					
				}
			}
            else if (IS_SECONDARY_SKILL(ch,sn))
			{
				level = skill_table[sn].skill_level[ch->ch_class2];
				
				if (ch->level2 < level)
				{
					sprintf (buf, "%16s n/a ", skill_table[sn].name);		
				}
				else
				{
					sprintf (buf, "%16s %s%3d{x%% ", skill_table[sn].name, ch->pcdata->seen_improve[sn] ? "" : "{G",ch->pcdata->learned[sn]);																										
					ch->pcdata->seen_improve[sn] = TRUE;				
				}
				
			}
			else if (IS_CROSS_SKILL(ch,sn))
			{
				//bool lower; //which class was lower... primary or secondary?
				
				level = 
					(skill_table[sn].skill_level[ch->ch_class] < skill_table[sn].skill_level[ch->ch_class2] ? 
					 skill_table[sn].skill_level[ch->ch_class] : skill_table[sn].skill_level[ch->ch_class2]);
				
				//skill_table[sn].skill_level[ch->ch_class] < skill_table[sn].skill_level[ch->ch_class2] ? lower = 1 : lower = 2;
				
				if (total_levels(ch) < level)
				{
					sprintf (buf, "%16s n/a ",skill_table[sn].name);	
				}
				else
				{
					sprintf (buf, "%16s %s%3d{x%% ", skill_table[sn].name, ch->pcdata->seen_improve[sn] ? "" : "{G",ch->pcdata->learned[sn]);																										
					ch->pcdata->seen_improve[sn] = TRUE;					
				}
			}
			else
			{
				level = skill_table[sn].skill_level[ch-> ch_class2];
            
				if (ch->level2 < level)
				{
					sprintf (buf, "%16s n/a ", skill_table[sn].name);					
				}
				else
				{
					sprintf (buf, "%16s %s%3d{x%% ", skill_table[sn].name, ch->pcdata->seen_improve[sn] ? "" : "{G",ch->pcdata->learned[sn]);																										
					ch->pcdata->seen_improve[sn] = TRUE;	
				}
			}
            if (skill_list[level][0] == '\0')
                sprintf (skill_list[level], "\r\nLevel %2d: %s", level, buf);
            else
            {                    /* append */

                if (++skill_columns[level] % 2 == 0)
                    strcat (skill_list[level], "\r\n          ");
                strcat (skill_list[level], buf);
            }
        }
    }

    /* return results */

    if (!found)
    {
        SEND ("No skills found.\r\n", ch);
        return;
    }

    buffer = new_buf ();
    for (level = 0; level < LEVEL_HERO + 1; level++)
        if (skill_list[level][0] != '\0')
            add_buf (buffer, skill_list[level]);
    add_buf (buffer, "\r\n");
    page_to_char (buf_string (buffer), ch);
    free_buf (buffer);	
}



void do_new_spells (CHAR_DATA *ch, char *argument)
{	
	int sn = 0;
	int col = 0;
	int curr_sp_lvl = 0;
	char buf[MSL];	
	
	sprintf(buf, "{r-=-=-=-=-=-={x {ySpellbook for %s{x {r=-=-=-=-=-=-{x\r\n\r\n", ch->name);
	SEND(buf, ch);
	
	while (curr_sp_lvl < 10)
	{			
		switch (curr_sp_lvl)
		{
		default:
		case 0:		
		case 1:
			break;
		case 2:
			if (total_levels(ch) < 4)
				return;
			break;
		case 3:
			if (total_levels(ch) < 7)
				return;
			break;
		case 4:
			if (total_levels(ch) < 11)
				return;
			break;
		case 5:
			if (total_levels(ch) < 15)
				return;
			break;
		case 6:
			if (total_levels(ch) < 19)
				return;
			break;
		case 7:
			if (total_levels(ch) < 23)
				return;
			break;
		case 8:
			if (total_levels(ch) < 27)
				return;
			break;
		case 9:
			if (total_levels(ch) < 31)
				return;
			break;
		}	
		//Level header.
		sprintf (buf, "\r\n{r-=-=-={x Level %d {r=-=-=-{x\r\n", curr_sp_lvl);
		SEND(buf, ch);
		col = 0;
		
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			//If it's a skill or some other crap keep going
			if (skill_table[sn].spell_level < 0 || skill_table[sn].spell_fun == spell_null)
				continue;
			
			//If it's not the same spell level as what we're on... keep going.
			if (skill_table[sn].spell_level != curr_sp_lvl)
				continue;
				
			//If they know it AND are powerful enough to cast it.
			if (can_cast_spell(ch, sn) && KNOWS(ch, sn))
			{				
				if (++col % 3 == 0)
				{
					sprintf (buf, "\r\n");
					SEND(buf, ch);
				}
				sprintf (buf, "%25s %s%3d{x%%  ", skill_table[sn].name, ch->pcdata->seen_improve[sn] ? "" : "{C", ch->pcdata->learned[sn]);	
				ch->pcdata->seen_improve[sn] = TRUE;				
				SEND(buf, ch);								
				
			}
		}
		curr_sp_lvl++;
		SEND("\r\n",ch);
	}
}

//For blind gamers. Just here as a placeholder until I spruce it up and add it
//to blind mode.
void do_blind_spells (CHAR_DATA *ch, char *argument)
{	
	int sn = 0;
	int col = 0;
	int curr_sp_lvl = 0;
	char buf[MSL];	
	
	SEND("Spellbook.\r\n\r\n",ch);
	
	while (curr_sp_lvl < 10)
	{			
		//Level header.
		sprintf (buf, "\r\n{r-=-=-={x Level %d {r=-=-=-{x\r\n", curr_sp_lvl);
		SEND(buf, ch);
		col = 0;
		
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			//If it's a skill or some other crap keep going
			if (skill_table[sn].spell_level < 0 || skill_table[sn].spell_fun == spell_null)
				continue;
			
			//If it's not the same spell level as what we're on... keep going.
			if (skill_table[sn].spell_level != curr_sp_lvl)
				continue;
				
			//If they know it AND are powerful enough to cast it.
			if (can_cast_spell(ch, sn) && KNOWS(ch, sn))
			{				
				if (++col % 3 == 0)
				{
					sprintf (buf, "\r\n");
					SEND(buf, ch);
				}
				sprintf (buf, "%25s %3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn]);				
				
				SEND(buf, ch);								
			}
		}
		curr_sp_lvl++;
		SEND("\r\n",ch);
	}
}


void do_spells (CHAR_DATA * ch, char *argument)

{
	BUFFER *buffer;
	char arg[MAX_INPUT_LENGTH];
	char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
	char spell_columns[LEVEL_HERO + 1];
	int sn, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
	bool fAll = TRUE, found = FALSE;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
		return;

	

	if (str_prefix (argument, "all"))
	{

		argument = one_argument (argument, arg);

		if (!is_number (arg))
		{
			SEND ("Arguments must be numerical or all.\n\r", ch);
			return;
		}

		max_lev = atoi (arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
			sprintf (buf, "Levels must be between 1 and %d.\n\r",LEVEL_HERO);
			SEND (buf, ch);
			return;
		}

 

if (argument[0] != '\0')

{

argument = one_argument (argument, arg);

if (!is_number (arg))

{

SEND ("Arguments must be numerical or all.\n\r",

ch);

return;

}

min_lev = max_lev;

max_lev = atoi (arg);

 

if (max_lev < 1 || max_lev > LEVEL_HERO)

{

sprintf (buf,

"Levels must be between 1 and %d.\n\r",

LEVEL_HERO);

SEND (buf, ch);

return;

}

 

if (min_lev > max_lev)

{

SEND ("That would be silly.\n\r", ch);

return;

}

}

}


 

 

/* initialize data */

for (level = 0; level < LEVEL_HERO + 1; level++)

{

spell_columns[level] = 0;

spell_list[level][0] = '\0';

}

 

for (sn = 0; sn < MAX_SKILL; sn++)

{

if (skill_table[sn].name == NULL)

break;

 

if (((level = skill_table[sn].skill_level[ch-> ch_class]) < LEVEL_HERO + 1
	|| IS_SECONDARY_SKILL(ch,sn))
	&& (fAll || level <= ch->level)
	&& level >= min_lev && level <= max_lev
	&& skill_table[sn].spell_fun != spell_null
	&& ch->pcdata->learned[sn] > 0)

{

	found = TRUE;
	level = skill_table[sn].skill_level[ch-> ch_class];
	if (ch->level < level || (IS_SECONDARY_SKILL(ch,sn) && ch->level2 < level))
		sprintf (buf, "%-18s  n/a            ", skill_table[sn].name);
	else
	{
		mana = UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level - level));
		sprintf (buf, "%-18s %3d mana (%3d%%) ", skill_table[sn].name, mana,ch->pcdata->learned[sn]);
	}

	if (spell_list[level][0] == '\0')
		sprintf (spell_list[level], "\n\rLevel %2d: %s", level, buf);
	else
	{ /* append */
		if (++spell_columns[level] % 2 == 0)
			strcat (spell_list[level], "\n\r          ");
		strcat (spell_list[level], buf);
	}

}

}

 

/* return results */

 

if (!found)

{

SEND ("No spells found.\n\r", ch);

return;

}

 

buffer = new_buf ();
for (level = 0; level < LEVEL_HERO + 1; level++)
if (spell_list[level][0] != '\0')
add_buf (buffer, spell_list[level]);
add_buf (buffer, "\n\r");
page_to_char (buf_string (buffer), ch);
free_buf (buffer);
}


/* shows skills, groups and costs (only if not bought) */
void list_group_costs (CHAR_DATA * ch)
{
    char buf[100];
    int gn, sn, col;

    if (IS_NPC (ch))
        return;

    col = 0;

    sprintf (buf, "%-18s %-5s %-18s %-5s %-18s %-5s\r\n", "group", "cp",
             "group", "cp", "group", "cp");
    SEND (buf, ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;

        if (!ch->gen_data->group_chosen[gn]
            && !ch->pcdata->group_known[gn]
            && group_table[gn].rating[ch-> ch_class] > 0)
        {
            sprintf (buf, "%-18s %-5d ", group_table[gn].name,
                     group_table[gn].rating[ch-> ch_class]);
            SEND (buf, ch);
            if (++col % 3 == 0)
                SEND ("\r\n", ch);
        }
    }
    if (col % 3 != 0)
        SEND ("\r\n", ch);
    SEND ("\r\n", ch);

    col = 0;

    sprintf (buf, "%-18s %-5s %-18s %-5s %-18s %-5s\r\n", "skill", "cp",
             "skill", "cp", "skill", "cp");
    SEND (buf, ch);

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

        if (!ch->gen_data->skill_chosen[sn]
            && ch->pcdata->learned[sn] == 0
            && skill_table[sn].spell_fun == spell_null
            && skill_table[sn].rating[ch-> ch_class] > 0)
        {
            sprintf (buf, "%-18s %-5d ", skill_table[sn].name,
                     skill_table[sn].rating[ch-> ch_class]);
            SEND (buf, ch);
            if (++col % 3 == 0)
                SEND ("\r\n", ch);
        }
    }
    if (col % 3 != 0)
        SEND ("\r\n", ch);
    SEND ("\r\n", ch);

    sprintf (buf, "Creation points: %d\r\n", ch->pcdata->points);
    SEND (buf, ch);
    sprintf (buf, "Experience per level: %ld\r\n",
             exp_per_level (ch, FALSE));
    SEND (buf, ch);
    return;
}


void list_group_chosen (CHAR_DATA * ch)
{
    char buf[100];
    int gn, sn, col;

    if (IS_NPC (ch))
        return;

    col = 0;

    sprintf (buf, "%-18s %-5s %-18s %-5s %-18s %-5s", "group", "cp", "group",
             "cp", "group", "cp\r\n");
    SEND (buf, ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;

        if (ch->gen_data->group_chosen[gn]
            && group_table[gn].rating[ch-> ch_class] > 0)
        {
            sprintf (buf, "%-18s %-5d ", group_table[gn].name,
                     group_table[gn].rating[ch-> ch_class]);
            SEND (buf, ch);
            if (++col % 3 == 0)
                SEND ("\r\n", ch);
        }
    }
    if (col % 3 != 0)
        SEND ("\r\n", ch);
    SEND ("\r\n", ch);

    col = 0;

    sprintf (buf, "%-18s %-5s %-18s %-5s %-18s %-5s", "skill", "cp", "skill",
             "cp", "skill", "cp\r\n");
    SEND (buf, ch);

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

        if (ch->gen_data->skill_chosen[sn]
            && skill_table[sn].rating[ch-> ch_class] > 0)
        {
            sprintf (buf, "%-18s %-5d ", skill_table[sn].name,
                     skill_table[sn].rating[ch-> ch_class]);
            SEND (buf, ch);
            if (++col % 3 == 0)
                SEND ("\r\n", ch);
        }
    }
    if (col % 3 != 0)
        SEND ("\r\n", ch);
    SEND ("\r\n", ch);

    sprintf (buf, "Creation points: %d\r\n", ch->gen_data->points_chosen);
    SEND (buf, ch);
    sprintf (buf, "Experience per level: %ld\r\n",
             exp_per_level (ch, FALSE));
    SEND (buf, ch);
    return;
}

long exp_per_level (CHAR_DATA * ch, bool secondary)
{    

	int expl;	
	int i = 0; 		
	
    if (IS_NPC (ch))
	{
		if (total_levels(ch) < 20)
		    return (5000 * ch->level);	
		else
			return (25000 * ch->level);	
	}
			
	if (secondary == FALSE)			
	{				
		expl = ( ch_class_table[ch->ch_class].baseXp ); 
		if (ch->level > 1)
		{
			for (i = 1; i < ch->level; i++)
			{			
					if (i >= 9)
						continue;				
						
					expl *= 2;				
			}
		
			if (ch->level > 9)
			{
				for (i = 9; i < ch->level; i++)
				{	
					expl += (125000 + (ch->level * 12500));
				}
			}
			if (ch->level >= 25)
			{
				for (i = 25; i < ch->level; i++)
				{	
					expl += 200000;
				}
			}
		}
	}
	else		
	{
		expl = ( ch_class_table[ch->ch_class2].baseXp );	
		if (ch->level2 > 1)
		{
			for (i = 1; i < ch->level2; i++)
			{			
					if (i >= 9)
						continue;				
						
					expl *= 2;				
			}
		
			if (ch->level2 > 9)
			{
				for (i = 9; i < ch->level2; i++)
				{	
					expl += (125000 + (ch->level2 * 12500));
				}
			}
			//Leave in just in case I end up doing dynamic m ch_class levels.
			/*if (ch->level2 >= 25)
			{
				for (i = 25; i < ch->level2; i++)
				{	
					expl += 50000;
				}
			}*/
		}
	}
	
	return expl;
	
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int gn, sn, i;

    if (argument[0] == '\0')
        return FALSE;

    argument = one_argument (argument, arg);

    if (!str_prefix (arg, "help"))
    {
        if (argument[0] == '\0')
        {
            do_function (ch, &do_help, "group help");
            return TRUE;
        }

        do_function (ch, &do_help, argument);
        return TRUE;
    }

    if (!str_prefix (arg, "add"))
    {
        if (argument[0] == '\0')
        {
            SEND ("You must provide a skill name.\r\n", ch);
            return TRUE;
        }

        gn = group_lookup (argument);
        if (gn != -1)
        {
            if (ch->gen_data->group_chosen[gn] || ch->pcdata->group_known[gn])
            {
                SEND ("You already know that group!\r\n", ch);
                return TRUE;
            }

            if (group_table[gn].rating[ch-> ch_class] < 1 )
            {
                SEND ("That group is not available.\r\n", ch);
                return TRUE;
            }

            /* Close security hole */
            if (ch->gen_data->points_chosen +
                group_table[gn].rating[ch-> ch_class] > 300)
            {
                SEND
                    ("You cannot take more than 300 creation points.\r\n",
                     ch);
                return TRUE;
            }

            sprintf (buf, "%s group added\r\n", group_table[gn].name);
            SEND (buf, ch);
            ch->gen_data->group_chosen[gn] = TRUE;
            ch->gen_data->points_chosen += group_table[gn].rating[ch-> ch_class];
            gn_add (ch, gn);
            ch->pcdata->points += group_table[gn].rating[ch-> ch_class];
            return TRUE;
        }

        sn = skill_lookup (argument);
        if (sn != -1)
        {
            if (ch->gen_data->skill_chosen[sn] || ch->pcdata->learned[sn] > 0)
            {
                SEND ("You already know that skill!\r\n", ch);
                return TRUE;
            }

            if (skill_table[sn].rating[ch-> ch_class] < 1
                || skill_table[sn].spell_fun != spell_null)
            {
                SEND ("That skill is not available.\r\n", ch);
                return TRUE;
            }

            /* Close security hole */
            if (ch->gen_data->points_chosen +
                skill_table[sn].rating[ch-> ch_class] > 300)
            {
                SEND
                    ("You cannot take more than 300 creation points.\r\n",
                     ch);
                return TRUE;
            }
            sprintf (buf, "%s skill added\r\n", skill_table[sn].name);
            SEND (buf, ch);
            ch->gen_data->skill_chosen[sn] = TRUE;
            ch->gen_data->points_chosen += skill_table[sn].rating[ch-> ch_class];
            ch->pcdata->learned[sn] = 1;
            ch->pcdata->points += skill_table[sn].rating[ch-> ch_class];
            return TRUE;
        }

        SEND ("No skills or groups by that name...\r\n", ch);
        return TRUE;
    }

    if (!strcmp (arg, "drop"))
    {
        if (argument[0] == '\0')
        {
            SEND ("You must provide a skill to drop.\r\n", ch);
            return TRUE;
        }

        gn = group_lookup (argument);
        if (gn != -1 && ch->gen_data->group_chosen[gn])
        {
            SEND ("Group dropped.\r\n", ch);
            ch->gen_data->group_chosen[gn] = FALSE;
            ch->gen_data->points_chosen -= group_table[gn].rating[ch-> ch_class];
            gn_remove (ch, gn);
            for (i = 0; i < MAX_GROUP; i++)
            {
                if (ch->gen_data->group_chosen[gn])
                    gn_add (ch, gn);
            }
            ch->pcdata->points -= group_table[gn].rating[ch-> ch_class];
            return TRUE;
        }

        sn = skill_lookup (argument);
        if (sn != -1 && ch->gen_data->skill_chosen[sn])
        {
            SEND ("Skill dropped.\r\n", ch);
            ch->gen_data->skill_chosen[sn] = FALSE;
            ch->gen_data->points_chosen -= skill_table[sn].rating[ch-> ch_class];
            ch->pcdata->learned[sn] = 0;
            ch->pcdata->points -= skill_table[sn].rating[ch-> ch_class];
            return TRUE;
        }

        SEND ("You haven't bought any such skill or group.\r\n", ch);
        return TRUE;
    }

    if (!str_prefix (arg, "premise"))
    {
        do_function (ch, &do_help, "premise");
        return TRUE;
    }

    if (!str_prefix (arg, "list"))
    {
        list_group_costs (ch);
        return TRUE;
    }

    if (!str_prefix (arg, "learned"))
    {
        list_group_chosen (ch);
        return TRUE;
    }

    if (!str_prefix (arg, "info"))
    {
        do_function (ch, &do_groups, argument);
        return TRUE;
    }

    return FALSE;
}






/* shows all groups, or the sub-members of a group */
void do_groups (CHAR_DATA * ch, char *argument)
{
    char buf[100];
    int gn, sn, col;

    if (IS_NPC (ch))
        return;

    col = 0;

    if (argument[0] == '\0')
    {                            /* show all groups */

        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name == NULL)
                break;
            if (ch->pcdata->group_known[gn])
            {
                sprintf (buf, "%-20s ", group_table[gn].name);
                SEND (buf, ch);
                if (++col % 3 == 0)
                    SEND ("\r\n", ch);
            }
        }
        if (col % 3 != 0)
            SEND ("\r\n", ch);
        sprintf (buf, "Creation points: %d\r\n", ch->pcdata->points);
        SEND (buf, ch);
        return;
    }

    if (!str_cmp (argument, "all"))
    {                            /* show all groups */
        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name == NULL)
                break;
            sprintf (buf, "%-20s ", group_table[gn].name);
            SEND (buf, ch);
            if (++col % 3 == 0)
                SEND ("\r\n", ch);
        }
        if (col % 3 != 0)
            SEND ("\r\n", ch);
        return;
    }


    /* show the sub-members of a group */
    gn = group_lookup (argument);
    if (gn == -1)
    {
        SEND ("No group of that name exist.\r\n", ch);
        SEND
            ("Type 'groups all' or 'info all' for a full listing.\r\n", ch);
        return;
    }

    for (sn = 0; sn < MAX_IN_GROUP; sn++)
    {
        if (group_table[gn].spells[sn] == NULL)
            break;
        sprintf (buf, "%-20s ", group_table[gn].spells[sn]);
        SEND (buf, ch);
        if (++col % 3 == 0)
            SEND ("\r\n", ch);
    }
    if (col % 3 != 0)
        SEND ("\r\n", ch);
}

/* checks for skill improvement */
void check_improve (CHAR_DATA * ch, int sn, bool success, int multiplier)
{
    int chance;
    char buf[100];

    if (IS_NPC (ch))
        return;
	
	if (ch->pcdata->learned[sn] >= 100)
	{
		ch->pcdata->seen_improve[sn] = TRUE;
		ch->pcdata->learned[sn] = 100;
		return;
	}
	
	if (ch->pcdata->learned[sn] == 0)
		return;	
    	
	//They can't improve since it's not a class skill.
	if (!IS_MCLASSED(ch) && !KNOWS(ch,sn))
	{
		if (ch->level < skill_table[sn].skill_level[ch->ch_class] || skill_table[sn].rating[ch-> ch_class] < 1)
			return;
	}
	else //They ARE multiclassed. Updated Upro 2/1/2020
	{
		if (!KNOWS(ch,sn))
		{
			//Neither of their classes can learn.
			if ((skill_table[sn].rating[ch->ch_class] == 0 && skill_table[sn].rating[ch->ch_class2] == 0))
				return;
			//They just aren't high enough level yet.
			if (ch->level < skill_table[sn].skill_level[ch->ch_class] && ch->level2 < skill_table[sn].skill_level[ch->ch_class2])
				return;
		}
	}
	

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[GET_INT(ch)].learn;    //250 max
    
	if (IS_MCLASSED(ch))
	{
		chance += ch->level;						 //40 max
		chance += ch->level2;
	}
	else
		chance += ch->level;						 //40 max
	
	chance *= multiplier;
	
	if (double_skill == TRUE)
		chance *= 2;
	
	//Upgraded from 1000 to 5000, this give a max 5.8% chance to have a chance to improve. Should slow things down sufficiently.
	//Upro 1/14/2020
    if (number_range (1, 5000) > chance)
        return;

    /* now that the character has a CHANCE to learn, see if they really have */

    if (success)
    {
        chance = URANGE (5, 100 - ch->pcdata->learned[sn], 95);
        if (number_percent () < chance)
        {
			ch->pcdata->learned[sn] += number_range(1,3);
            sprintf (buf, "You have become better at {g%s{x! {r[{x%d%%{r]{x\r\n",
                     skill_table[sn].name, ch->pcdata->learned[sn]);
            SEND (buf, ch);            
			ch->pcdata->seen_improve[sn] = FALSE;
			if (!IS_SECONDARY_SKILL(ch,sn))
				gain_exp (ch, 20 * skill_table[sn].rating[ch-> ch_class], FALSE);
			else
				gain_exp (ch, 20 * skill_table[sn].rating[ch-> ch_class2], FALSE);
        }
    }

    else
    {
        chance = URANGE (5, ch->pcdata->learned[sn] / 2, 30);
        if (number_percent () < chance)
        {
            sprintf (buf,
                     "You learn from your mistakes, and your {g%s{x skill improves.\r\n",
                     skill_table[sn].name);
            SEND (buf, ch);
			ch->pcdata->seen_improve[sn] = FALSE;
            ch->pcdata->learned[sn]++;
            ch->pcdata->learned[sn] = UMIN (ch->pcdata->learned[sn], 100);
			if (!IS_SECONDARY_SKILL(ch,sn))
				gain_exp (ch, 2 * skill_table[sn].rating[ch-> ch_class], FALSE);
			else
				gain_exp (ch, 2 * skill_table[sn].rating[ch-> ch_class2], TRUE);
        }
    }
}

/* returns a group index number given the name */
int group_lookup (const char *name)
{
    int gn;

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;
        if (LOWER (name[0]) == LOWER (group_table[gn].name[0])
            && !str_prefix (name, group_table[gn].name))
            return gn;
    }

    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add (CHAR_DATA * ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = TRUE;
    for (i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
//            break;
        group_add (ch, group_table[gn].spells[i], FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove (CHAR_DATA * ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for (i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_remove (ch, group_table[gn].spells[i]);
    }
}

/* use for processing a skill or group for addition  */
void skill_add ( CHAR_DATA *ch, const char *name )
{
	int sn;
	
	if (IS_NPC (ch))            /* NPCs do not have skills */
        return;

    sn = skill_lookup (name);

    if (sn != -1)
    {
        if (ch->pcdata->learned[sn] == 0)
        {                        /* i.e. not known */
            ch->pcdata->learned[sn] = 1;     
        }
        return;
    }
	
}

void group_add (CHAR_DATA * ch, const char *name, bool deduct)
{
    int sn, gn;

    if (IS_NPC (ch))            /* NPCs do not have skills */
        return;

    sn = skill_lookup (name);

    if (sn != -1)
    {
        if (ch->pcdata->learned[sn] == 0)
        {                        /* i.e. not known */
            ch->pcdata->learned[sn] = 1;
            if (deduct)
                ch->pcdata->points += skill_table[sn].rating[ch-> ch_class];
        }
        return;
    }

    /* now check groups */

    gn = group_lookup (name);

    if (gn != -1)
    {
        if (ch->pcdata->group_known[gn] == FALSE)
        {
            ch->pcdata->group_known[gn] = TRUE;
            if (deduct)
                ch->pcdata->points += group_table[gn].rating[ch-> ch_class];
        }
        gn_add (ch, gn);        // make sure all skills in the group are known 
    }

}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove (CHAR_DATA * ch, const char *name)
{
    int sn, gn;

    sn = skill_lookup (name);

    if (sn != -1)
    {
        ch->pcdata->learned[sn] = 0;
        return;
    }

    /* now check groups */

    gn = group_lookup (name);

    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
        ch->pcdata->group_known[gn] = FALSE;
        gn_remove (ch, gn);        /* be sure to call gn_add on all remaining groups */
    }
}



void do_train (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = -1;
    char *pOutput = NULL;
    int cost;

    if (IS_NPC (ch))
        return;

    /*
     * Check for trainer.
     */
    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
        if (IS_NPC (mob) && IS_SET (mob->act, ACT_TRAIN))
            break;
    }

    if (mob == NULL)
    {
        SEND ("You can't do that here.\r\n", ch);
        return;
    }
	
    if (argument[0] == '\0')
    {
        sprintf (buf, "You have %d training sessions.\r\n", ch->train);
        SEND (buf, ch);
        argument = "foo";
    }

    cost = 1;

    if (!str_cmp (argument, "str"))
    {
        if ( ch_class_table[ch-> ch_class].attr_prime == STAT_STR)
            cost = 1;
        stat = STAT_STR;
        pOutput = "strength";
    }

    else if (!str_cmp (argument, "int"))
    {
        if ( ch_class_table[ch-> ch_class].attr_prime == STAT_INT)
            cost = 1;
        stat = STAT_INT;
        pOutput = "intelligence";
    }

    else if (!str_cmp (argument, "wis"))
    {
        if ( ch_class_table[ch-> ch_class].attr_prime == STAT_WIS)
            cost = 1;
        stat = STAT_WIS;
        pOutput = "wisdom";
    }

    else if (!str_cmp (argument, "dex"))
    {
        if ( ch_class_table[ch-> ch_class].attr_prime == STAT_DEX)
            cost = 1;
        stat = STAT_DEX;
        pOutput = "dexterity";
    }

    else if (!str_cmp (argument, "con"))
    {
        if ( ch_class_table[ch-> ch_class].attr_prime == STAT_CON)
            cost = 1;
        stat = STAT_CON;
        pOutput = "constitution";
    }
	
	else if (!str_cmp (argument, "cha"))
    {
		if (!IS_BARD(ch))
		{
			SEND("You can't train that.\r\n",ch);
			return;
		}
		
        if ( ch_class_table[ch->ch_class].attr_prime == STAT_CHA)
            cost = 1;
        stat = STAT_CHA;
        pOutput = "charisma";
    }

    //else if (!str_cmp (argument, "hp"))
    //    cost = 1;

	// if (!str_cmp (argument, "mana"))
    //    cost = 1;

    else
    {
        strcpy (buf, "You can train:");
        if (ch->perm_stat[STAT_STR] < get_max_train (ch, STAT_STR))
            strcat (buf, " str");
        if (ch->perm_stat[STAT_INT] < get_max_train (ch, STAT_INT))
            strcat (buf, " int");
        if (ch->perm_stat[STAT_WIS] < get_max_train (ch, STAT_WIS))
            strcat (buf, " wis");
        if (ch->perm_stat[STAT_DEX] < get_max_train (ch, STAT_DEX))
            strcat (buf, " dex");
        if (ch->perm_stat[STAT_CON] < get_max_train (ch, STAT_CON))
            strcat (buf, " con");
		if (ch->perm_stat[STAT_CHA] < get_max_train (ch, STAT_CHA) && IS_BARD(ch))
		    strcat (buf, " cha");

        //strcat (buf, " hp mana");

        if (buf[strlen (buf) - 1] != ':')
        {
            strcat (buf, ".\r\n");
            SEND (buf, ch);
			SEND ("\r\nType help train to see what else you can do with trains.\r\n",ch);
        }
        else
        {
            /*
             * This message dedicated to Jordan ... you big stud!
             */
            act ("You have nothing left to train, you $T!",
                 ch, NULL,
                 ch->sex == SEX_MALE ? "big stud" :
                 ch->sex == SEX_FEMALE ? "hot babe" : "wild thing", TO_CHAR);
        }

        return;
    }

    /*if (!str_cmp ("hp", argument))
    {
        if (cost > ch->train)
        {
            SEND ("You don't have enough training sessions.\r\n", ch);
            return;
        }

        ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit += 10;
        act ("Your durability increases!", ch, NULL, NULL, TO_CHAR);
        act ("$n's durability increases!", ch, NULL, NULL, TO_ROOM);
        return;
    }

    if (!str_cmp ("mana", argument))
    {
        if (cost > ch->train)
        {
            SEND ("You don't have enough training sessions.\r\n", ch);
            return;
        }

        ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act ("Your power increases!", ch, NULL, NULL, TO_CHAR);
        act ("$n's power increases!", ch, NULL, NULL, TO_ROOM);
        return;
    }*/

    if (ch->perm_stat[stat] >= get_max_train (ch, stat))
    {
        act ("Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR);
        return;
    }

    if (cost > ch->train)
    {
        SEND ("You don't have enough training sessions.\r\n", ch);
        return;
    }

    ch->train -= cost;
	ch->guildpoints += 10;
    ch->perm_stat[stat] += 1;
    act ("Your $T increases!", ch, NULL, pOutput, TO_CHAR);
    act ("$n's $T increases!", ch, NULL, pOutput, TO_ROOM);
    return;
}

