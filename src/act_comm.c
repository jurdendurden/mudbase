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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_comm.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include <ctype.h> /* for isalpha() and isspace() -- JR */

/* RT code to delete yourself */

void do_delet (CHAR_DATA * ch, char *argument)
{
    SEND ("You must type the full command to delete yourself.\r\n",
                  ch);
}

void do_delete (CHAR_DATA * ch, char *argument)
{
    char strsave[MAX_INPUT_LENGTH];
	int i;

    if (IS_NPC (ch))
        return;

    if (ch->pcdata->confirm_delete)
    {
        if (argument[0] != '\0')
        {
            SEND ("Delete status removed.\r\n", ch);
            ch->pcdata->confirm_delete = FALSE;
            return;
        }
        else
        {
            sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->name));
            wiznet (str_dup("$N turns $Mself into line noise."), ch, NULL, 0, 0, 0);
            stop_fighting (ch, TRUE);
			
			ACCOUNT_DATA * acc = ch->desc->account;
			acc->numb = getCharNumb( acc, ch->name );		
				
			free_string(acc->char_list[acc->numb].char_name);
			acc->char_list[acc->numb].char_name = str_dup("");
			
				
			save_account_obj(acc);
            
			do_function (ch, &do_quit, str_dup(""));
            unlink (strsave);
            return;
        }
    }

    if (argument[0] != '\0')
    {
        SEND ("Just type delete. No argument.\r\n", ch);
        return;
    }

    SEND ("Type delete again to confirm this command.\r\n", ch);
    SEND ("WARNING: this command is irreversible.\r\n", ch);
    SEND
        ("Typing delete with an argument will undo delete status.\r\n", ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet (str_dup("$N is contemplating deletion."), ch, NULL, 0, 0, get_trust (ch));
}


/* RT code to display channel status */

void do_channels (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */
    SEND ("   channel     status\r\n", ch);
    SEND ("---------------------\r\n", ch);

    SEND ("{Bgossip{x         ", ch);
    if (!IS_SET (ch->comm, COMM_NOGOSSIP))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("{cauction{x        ", ch);
    if (!IS_SET (ch->comm, COMM_NOAUCTION))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("{mmusic{x          ", ch);
    if (!IS_SET (ch->comm, COMM_NOMUSIC))
		SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("{gq{x/{ga{x            ", ch);
    if (!IS_SET (ch->comm, COMM_NOQUESTION))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("{Dquote{x          ", ch);
    if (!IS_SET (ch->comm, COMM_NOQUOTE))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("{Ygrats{x          ", ch);
    if (!IS_SET (ch->comm, COMM_NOGRATS))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("{Ctells{x          ", ch);
    if (!IS_SET (ch->comm, COMM_DEAF))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
	
    SEND("{rOOC{x            ",ch);
    if (!IS_SET(ch->comm,COMM_OOCOFF))
		SEND("{GON{x\r\n",ch);
    else
		SEND("{ROFF{x\r\n",ch);
		
	if (IS_IMMORTAL (ch))
    {
        SEND ("{yimmtalk{x        ", ch);
        if (!IS_SET (ch->comm, COMM_NOWIZ))
            SEND ("{GON{x\r\n", ch);
		else
			SEND ("{ROFF{x\r\n", ch);
    }
	
    SEND ("{tquiet mode{x     ", ch);
    if (IS_SET (ch->comm, COMM_QUIET))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);	
		
	SEND ("{tblind mode{x     ", ch);
    if (ch->blind)
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
	
	SEND ("{tMXP{x            ", ch);
    if (ch->desc->mxp)
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
	
	SEND("\r\n",ch);
	
    if (IS_SET (ch->comm, COMM_AFK))
        SEND ("You are {bAFK{x.\r\n", ch);
		
    if (IS_SET (ch->comm, COMM_SNOOP_PROOF))
        SEND ("You are {Dimmune{x to snooping.\r\n", ch);
	else
		SEND ("You are {Dnot immune{x to snooping.\r\n", ch);
    if (ch->lines != PAGELEN)
    {
        if (ch->lines)
        {
            sprintf (buf, "You display {D%d{x lines of scroll.\r\n",
                     ch->lines + 2);
            SEND (buf, ch);
        }
        else
            SEND ("Scroll buffering is {Roff{x.\r\n", ch);
    }

    if (ch->prompt != NULL)
    {
        sprintf (buf, "Your current prompt is: %s\r\n", ch->prompt);
        SEND (buf, ch);
    }
	else	
		SEND ("Your current prompt is: {Ddefault{x\r\n", ch);        
	
	sprintf (buf, "You are currently %s item conditions.\r\n", ch->pcdata->item_condition == TRUE ? "{Gshowing{x" : "{Rhiding{x");
	SEND (buf, ch);
	
	
    if (IS_SET (ch->comm, COMM_NOSHOUT))
        SEND ("You cannot yell.\r\n", ch);

    if (IS_SET (ch->comm, COMM_NOTELL))
        SEND ("You cannot use tells.\r\n", ch);

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
        SEND ("You cannot use any channels.\r\n", ch);

    if (IS_SET (ch->comm, COMM_NOEMOTE))
        SEND ("You cannot show emotions.\r\n", ch);

}

void do_blind (CHAR_DATA *ch, char *argument)
{
	if (ch->blind)
	{
		SEND ("You turn off blind mode.\r\n",ch);
		ch->blind = FALSE;
	}
	else
	{
		SEND ("You turn on blind mode.\r\n",ch);
		ch->blind = TRUE;
	}
	return;
}

/* RT deaf blocks out all shouts */

void do_deaf (CHAR_DATA * ch, char *argument)
{

    if (IS_SET (ch->comm, COMM_DEAF))
    {
        SEND ("You can now hear tells again.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_DEAF);
    }
    else
    {
        SEND ("From now on, you won't hear tells.\r\n", ch);
        SET_BIT (ch->comm, COMM_DEAF);
    }
	return;
}

/* RT quiet blocks out all communication */

void do_quiet (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_QUIET))
    {
        SEND ("Quiet mode removed.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_QUIET);
    }
    else
    {
        SEND ("From now on, you will only hear says and emotes.\r\n",
                      ch);
        SET_BIT (ch->comm, COMM_QUIET);
    }
}

/* afk command */

void do_afk (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_AFK))
    {
        SEND ("AFK mode removed. Type 'replay' to see tells.\r\n",
                      ch);
        REMOVE_BIT (ch->comm, COMM_AFK);
    }
    else
    {
        SEND ("You are now in AFK mode.\r\n", ch);
        SET_BIT (ch->comm, COMM_AFK);
    }
}

void do_replay (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
    {
        SEND ("You can't replay.\r\n", ch);
        return;
    }

    if (buf_string (ch->pcdata->buffer)[0] == '\0')
    {
        SEND ("You have no tells to replay.\r\n", ch);
        return;
    }

    page_to_char (buf_string (ch->pcdata->buffer), ch);
    clear_buf (ch->pcdata->buffer);
}

/* RT auction rewritten in ROM style */
/*
void do_auction (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOAUCTION))
        {
            SEND ("{aAuction channel is now ON.{x\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOAUCTION);
			return;
        }
        else
        {
            SEND ("{aAuction channel is now OFF.{x\r\n", ch);
            SET_BIT (ch->comm, COMM_NOAUCTION);
			return;
        }
    }
    else
    {                            // auction message sent, turn auction on if it is off 

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOAUCTION);
    }

    sprintf (buf, "{aYou auction '{A%s{a'{x\r\n", argument);
    SEND (buf, ch);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            !IS_SET (victim->comm, COMM_NOAUCTION) &&
            !IS_SET (victim->comm, COMM_QUIET))
        {
            act_new ("{a$n auctions '{A$t{a'{x",
                     ch, argument, d->character, TO_VICT, POS_DEAD);
        }
    }
}
*/


/* RT chat replaced with ROM gossip */
void do_gossip (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOGOSSIP))
        {
            SEND ("Gossip channel is now ON.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOGOSSIP);
			return;
        }
        else
        {
            SEND ("Gossip channel is now OFF.\r\n", ch);
            SET_BIT (ch->comm, COMM_NOGOSSIP);
			return;
        }
    }
    else
    {                            /* gossip message sent, turn gossip on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;

        }

		if (strlen(argument) < 2)
		{
			return;
		}
		
        REMOVE_BIT (ch->comm, COMM_NOGOSSIP);

        sprintf (buf, "You {Bgossip{x '%s'{x\r\n", argument);
        SEND (buf, ch);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOGOSSIP) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("$n {Bgossips{x '$t'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

void do_grats (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOGRATS))
        {
            SEND ("Grats channel is now ON.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOGRATS);
        }
        else
        {
            SEND ("Grats channel is now OFF.\r\n", ch);
            SET_BIT (ch->comm, COMM_NOGRATS);
        }
    }
    else
    {                            /* grats message sent, turn grats on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;

        }

        REMOVE_BIT (ch->comm, COMM_NOGRATS);

        sprintf (buf, "You {Ycongratulate{x '%s'\r\n{x", argument);
        SEND (buf, ch);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOGRATS) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("$n {Ycongratulates{x '$t'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

void do_quote (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOQUOTE))
        {
            SEND ("Quote channel is now ON.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOQUOTE);
        }
        else
        {
            SEND ("Quote channel is now OFF.\r\n", ch);
            SET_BIT (ch->comm, COMM_NOQUOTE);
        }
    }
    else
    {                            /* quote message sent, turn quote on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;

        }

        REMOVE_BIT (ch->comm, COMM_NOQUOTE);

        sprintf (buf, "You {Dquote{x '%s'{x\r\n", argument);
        SEND (buf, ch);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOQUOTE) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("$n {Dquotes{x '$t'{x",
                    ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

/* RT question channel */
void do_question (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOQUESTION))
        {
            SEND ("Q/A channel is now ON.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOQUESTION);
        }
        else
        {
            SEND ("Q/A channel is now OFF.\r\n", ch);
            SET_BIT (ch->comm, COMM_NOQUESTION);
        }
    }
    else
    {                            /* question sent, turn Q/A on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOQUESTION);

        sprintf (buf, "You {gquestion{x '%s'\r\n{x", argument);
        SEND (buf, ch);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOQUESTION) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("$n {gquestions{x '$t'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

/* RT answer channel - uses same line as questions */
void do_answer (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOQUESTION))
        {
            SEND ("Q/A channel is now ON.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOQUESTION);
        }
        else
        {
            SEND ("Q/A channel is now OFF.\r\n", ch);
            SET_BIT (ch->comm, COMM_NOQUESTION);
        }
    }
    else
    {                            /* answer sent, turn Q/A on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOQUESTION);

        sprintf (buf, "You {ganswer{x '%s'\r\n{x", argument);
        SEND (buf, ch);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOQUESTION) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("$n {ganswers{x '$t'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

/* RT music channel */
void do_music (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOMUSIC))
        {
            SEND ("Music channel is now ON.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOMUSIC);
        }
        else
        {
            SEND ("Music channel is now OFF.\r\n", ch);
            SET_BIT (ch->comm, COMM_NOMUSIC);
        }
    }
    else
    {                            /* music sent, turn music on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            SEND ("You must turn off quiet mode first.\r\n", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            SEND
                ("The gods have revoked your channel priviliges.\r\n", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOMUSIC);

        sprintf (buf, "You {msing{x: '%s'\r\n{x", argument);
        SEND (buf, ch);        
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOMUSIC) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("$n {msings{x: '$t'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

/* clan channels */
void do_clantalk (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (!is_clan (ch) || clan_table[ch->clan].independent)
    {
        SEND ("You aren't in a clan.\r\n", ch);
        return;
    }
    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOCLAN))
        {
            SEND ("Clan channel is now ON\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOCLAN);
        }
        else
        {
            SEND ("Clan channel is now OFF\r\n", ch);
            SET_BIT (ch->comm, COMM_NOCLAN);
        }
        return;
    }

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
        SEND ("The gods have revoked your channel priviliges.\r\n",
                      ch);
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOCLAN);

    sprintf (buf, "You %sclan{x '%s'{x\r\n", clan_table[ch->clan].clan_channel, argument);
    SEND (buf, ch);
    sprintf (buf, "%s %sclans{x '%s'{x\r\n", ch->name, clan_table[ch->clan].clan_channel, argument);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            is_same_clan (ch, d->character) &&
            !IS_SET (d->character->comm, COMM_NOCLAN) &&
            !IS_SET (d->character->comm, COMM_QUIET))
        {
            SEND(buf, d->character);
        }
    }

    return;
}

void do_immtalk (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOWIZ))
        {
            SEND ("Immortal channel is now ON\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_NOWIZ);
        }
        else
        {
            SEND ("Immortal channel is now OFF\r\n", ch);
            SET_BIT (ch->comm, COMM_NOWIZ);
        }
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOWIZ);

    act_new ("{r[{I$n{r]{x  '$t'{x", ch, argument, NULL, TO_CHAR, POS_DEAD);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING &&
            IS_IMMORTAL (d->character) &&
            !IS_SET (d->character->comm, COMM_NOWIZ))
        {
            act_new ("{r[{I$n{r]{x  '$t'{x", ch, argument, d->character, TO_VICT,
                     POS_DEAD);
        }
    }

    return;
}



void do_say (CHAR_DATA * ch, char *argument)
{
    if (argument[0] == '\0')
    {
        SEND ("Say what?\r\n", ch);
        return;
    }
	//Drunk code.
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		argument = makedrunk(argument,ch);

	//put intelligence code here for mumbling with low int. Upro
    act ("$n {bsays{x '$T'{x", ch, NULL, argument, TO_ROOM);
    act ("You {bsay{x '$T'{x", ch, NULL, argument, TO_CHAR);

	OBJ_DATA *obj, *obj_next;
	//mprog triggers
    if (!IS_NPC (ch))
    {
        CHAR_DATA *mob, *mob_next;
        for (mob = ch->in_room->people; mob != NULL; mob = mob_next)
        {
            mob_next = mob->next_in_room;
            if (IS_NPC (mob) && HAS_TRIGGER_MOB(mob, TRIG_SPEECH)
                && mob->position == mob->pIndexData->default_pos)
                p_act_trigger(argument, mob, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH);
			for ( obj = mob->carrying; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
				p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
			}
        }
    }
	//oprog triggers.
	for ( obj = ch->in_room->contents; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
		p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
	}
	//rprog triggers
	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_SPEECH ) )
	    p_act_trigger( argument, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_SPEECH );
		
    return;
}

void do_condition ( CHAR_DATA *ch, char *argument )
{
	if (ch->pcdata->item_condition)
	{
		SEND("You will no longer see item conditions.\r\n",ch);
		ch->pcdata->item_condition = FALSE;
	}
	else
	{
		SEND("You will now see item conditions.\r\n",ch);
		ch->pcdata->item_condition = TRUE;
	}
		 
    return;
}

void do_ooc( CHAR_DATA *ch, char *argument )
{

    DESCRIPTOR_DATA *d;
	char buf[MSL]; 
	 
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  SEND("You must turn off quiet mode first.\r\n",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  SEND("The {Rgods{x have revoked your channel priviliges.\r\n",ch);
	  return; 
	} 
     
    if (argument[0] == '\0' )
    {
      	
       if (IS_SET(ch->comm,COMM_OOCOFF))
      	{
            SEND("You can hear {rOOC{x messages again.\r\n",ch);
            REMOVE_BIT(ch->comm,COMM_OOCOFF);
      	}
      	else
      	{
            SEND("You will no longer hear {rOOC{x messages.\r\n",ch);
            SET_BIT(ch->comm,COMM_OOCOFF);
      	}
             
       return;
    }
          
    if ( IS_SET(ch->comm,COMM_OOCOFF))
    {
		SEND("{rOOC{x is currently off. Type {rOOC{x without any arguements to turn it on.",ch);
		return;
    }         
     
    REMOVE_BIT (ch->comm, COMM_OOCOFF);	
	char buf2[MSL];
	sprintf(buf2, "%s", ctime (&current_time));
     
	
	
	if (IS_SET (ch->comm, COMM_TIME_STAMPS))		
		sprintf (buf, "%s You {rOOC{x '%s{x'\r\n", buf2, argument);
	else
		sprintf (buf, "You {rOOC{x '%s{x'\r\n", argument);
    SEND (buf, ch);
    
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;

		if ( d->connected == CON_PLAYING &&
			 d->character != ch &&
			 !IS_SET(victim->comm, COMM_OOCOFF) &&
			 !IS_SET(victim->comm, COMM_QUIET) &&
			 !IS_SET(victim->comm, COMM_NOCHANNELS)
			) 
		{
			if (IS_SET (victim->comm, COMM_TIME_STAMPS))	
			{				
				sprintf(buf, "%s $n {rOOC's{x '$t{x'", buf2);
				act_new (buf,ch, argument, d->character, TO_VICT, POS_SLEEPING);
			}
			else
				act_new ("$n {rOOC's{x '$t{x'",ch, argument, d->character, TO_VICT, POS_SLEEPING);
		}
    }
}






void do_tell (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char buf2[MSL];
    CHAR_DATA *victim;


    if (IS_SET (ch->comm, COMM_NOTELL) || IS_SET (ch->comm, COMM_DEAF))
    {
        SEND ("Your message didn't get through.\r\n", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_QUIET))
    {
        SEND ("You must turn off quiet mode first.\r\n", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_DEAF))
    {
        SEND ("You must turn off deaf mode first.\r\n", ch);
        return;
    }

    argument = one_argument (argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        SEND ("Tell whom what?\r\n", ch);
        return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ((victim = get_char_world (ch, arg)) == NULL
        || (IS_NPC (victim) && victim->in_room != ch->in_room))
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->desc == NULL && !IS_NPC (victim))
    {
        act ("$N seems to have misplaced $S link...try again later.",
             ch, NULL, victim, TO_CHAR);
        sprintf (buf, "%s {Ctells{x you '%s'%s{x\r\n", PERS (ch, victim),
                 argument, victim->pcdata->beeptells ? "{*" : "");
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

    if (!(IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL)
        && !IS_AWAKE (victim))
    {
        act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (
        (IS_SET (victim->comm, COMM_QUIET)
         || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch))
    {
        act ("$E is not receiving tells.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (IS_SET (victim->comm, COMM_AFK))
    {
        if (IS_NPC (victim))
        {
            act ("$E is {bAFK{x, and not receiving tells.", ch, NULL, victim,
                 TO_CHAR);
            return;
        }

        act ("$E is {bAFK{x, but your tell will go through when $E returns.",
             ch, NULL, victim, TO_CHAR);
        sprintf (buf, "%s {Ctells{x you '%s'\r\n", PERS (ch, victim),
                 argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

	/*if (victim->desc->connected >= CON_NOTE_TO && victim->desc->connected <= CON_NOTE_FINISH)
	{
		act ("$E is writing a note, but your tell will go through when $E returns.",
				ch, NULL, victim, TO_CHAR);
		sprintf (buf, "%s {Ctells{x you '%s'{x\r\n", PERS (ch, victim), argument);
		buf[0] = UPPER (buf[0]);
		add_buf (victim->pcdata->buffer, buf);
		return;
	}*/

	if (IS_SET (ch->comm, COMM_TIME_STAMPS))
	{
		sprintf(buf2, "{r[{x%s{r]{x You {Ctell{x $N '$t'{x", ctime (&current_time));
		act (buf2, ch, argument, victim, TO_CHAR);		
	}
	else
		act ("You {Ctell{x $N '$t'{x", ch, argument, victim, TO_CHAR);
		
	if (IS_SET (victim->comm, COMM_TIME_STAMPS))	
	{		
		sprintf(buf2, "{r[{x%s{r]{x $n {Ctells{x you '$t'{x", ctime (&current_time));
		act_new(buf2, ch, argument, victim, TO_VICT, POS_DEAD);
	}
	else
		act_new ("$n {Ctells{x you '$t'{x", ch, argument, victim, TO_VICT, POS_DEAD);
	
    victim->reply = ch;

    if (!IS_NPC (ch) && IS_NPC (victim) && HAS_TRIGGER_MOB(victim, TRIG_SPEECH))
        p_act_trigger(argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH);

    return;
}



void do_reply (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET (ch->comm, COMM_NOTELL))
    {
        SEND ("Your message didn't get through.\r\n", ch);
        return;
    }

    if ((victim = ch->reply) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (victim->desc == NULL && !IS_NPC (victim))
    {
        act ("$N seems to have misplaced $S link...try again later.",
             ch, NULL, victim, TO_CHAR);
        sprintf (buf, "%s {Creplies{x to you '%s'%s{x\r\n", PERS (ch, victim),
                 argument, victim->pcdata->beeptells ? "{*" : "");
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

    if (!IS_IMMORTAL (ch) && !IS_AWAKE (victim))
    {
        act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (
        (IS_SET (victim->comm, COMM_QUIET)
         || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch)
        && !IS_IMMORTAL (victim))
    {
        act_new ("$E is not receiving tells.", ch, 0, victim, TO_CHAR,
                 POS_DEAD);
        return;
    }

    if (!IS_IMMORTAL (victim) && !IS_AWAKE (ch))
    {
        SEND ("In your dreams, or what?\r\n", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_AFK))
    {
        if (IS_NPC (victim))
        {
            act_new ("$E is {bAFK{x, and not receiving tells.",
                     ch, NULL, victim, TO_CHAR, POS_DEAD);
            return;
        }

        act_new ("$E is {bAFK{x, but your tell will go through when $E returns.",
                 ch, NULL, victim, TO_CHAR, POS_DEAD);
        sprintf (buf, "%s {Creplies{x to you '%s'{x\r\n", PERS (ch, victim),
                 argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

    act_new ("You {Creply{x to $N '$t'{x", ch, argument, victim, TO_CHAR,
             POS_DEAD);
    act_new ("$n {Creplies{x to you '$t'{x", ch, argument, victim, TO_VICT,
             POS_DEAD);
    victim->reply = ch;

    return;
}

void do_yell (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (IS_SET (ch->comm, COMM_NOSHOUT))
    {
        SEND ("You can't yell.\r\n", ch);
        return;
    }

    if (argument[0] == '\0')
    {        
		SEND ("Yell what?\r\n", ch);
        return;
    }

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		argument = makedrunk(argument,ch);


    act ("You {Gyell{x '$t'{x", ch, argument, NULL, TO_CHAR);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING
            && d->character != ch
            && d->character->in_room != NULL
            && d->character->in_room->area == ch->in_room->area
            && !IS_SET (d->character->comm, COMM_QUIET))
        {
            act ("$n {Gyells{x '$t'{x", ch, argument, d->character, TO_VICT);
        }
    }

    return;
}


void do_emote (CHAR_DATA * ch, char *argument)
{
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

    /* little hack to fix the ',{' bug posted to rom list
     * around 4/16/01 -- JR
     */
    if (!(isalpha(argument[0])) || (isspace(argument[0])))
    {
	SEND ("Moron!\r\n", ch);
	return;
    }

    MOBtrigger = FALSE;
    act ("$n $T", ch, NULL, argument, TO_ROOM);
    act ("$n $T", ch, NULL, argument, TO_CHAR);
    MOBtrigger = TRUE;
    return;
}


void do_pmote (CHAR_DATA * ch, char *argument)
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

	
    /* little hack to fix the ',{' bug posted to rom list
     * around 4/16/01 -- JR
     */
    if (!(isalpha(argument[0])) || (isspace(argument[0])))
    {
	SEND ("Moron!\r\n", ch);
	return;
    }

    act ("$n $t", ch, argument, NULL, TO_CHAR);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;

        if ((letter = strstr (argument, vch->name)) == NULL)
        {
            MOBtrigger = FALSE;
            act ("$N $t", vch, argument, ch, TO_CHAR);
            MOBtrigger = TRUE;
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

        MOBtrigger = FALSE;
        act ("$N $t", vch, temp, ch, TO_CHAR);
        MOBtrigger = TRUE;
    }

    return;
}


/*
 * All the posing stuff.
 */
struct pose_table_type {
    char *message[2 * MAX_CLASS];
};

/*
const struct pose_table_type pose_table[] = {
    {
     {
      "You sizzle with energy.",
      "$n sizzles with energy.",
      "You feel very holy.",
      "$n looks very holy.",
      "You perform a small card trick.",
      "$n performs a small card trick.",
      "You show your bulging muscles.",
      "$n shows $s bulging muscles."}
     },

    {
     {
      "You turn into a butterfly, then return to your normal shape.",
      "$n turns into a butterfly, then returns to $s normal shape.",
      "You nonchalantly turn wine into water.",
      "$n nonchalantly turns wine into water.",
      "You wiggle your ears alternately.",
      "$n wiggles $s ears alternately.",
      "You crack nuts between your fingers.",
      "$n cracks nuts between $s fingers."}
     },

    {
     {
      "Blue sparks fly from your fingers.",
      "Blue sparks fly from $n's fingers.",
      "A halo appears over your head.",
      "A halo appears over $n's head.",
      "You nimbly tie yourself into a knot.",
      "$n nimbly ties $mself into a knot.",
      "You grizzle your teeth and look mean.",
      "$n grizzles $s teeth and looks mean."}
     },

    {
     {
      "Little red lights dance in your eyes.",
      "Little red lights dance in $n's eyes.",
      "You recite words of wisdom.",
      "$n recites words of wisdom.",
      "You juggle with daggers, apples, and eyeballs.",
      "$n juggles with daggers, apples, and eyeballs.",
      "You hit your head, and your eyes roll.",
      "$n hits $s head, and $s eyes roll."}
     },

    {
     {
      "A slimy green monster appears before you and bows.",
      "A slimy green monster appears before $n and bows.",
      "Deep in prayer, you levitate.",
      "Deep in prayer, $n levitates.",
      "You steal the underwear off every person in the room.",
      "Your underwear is gone!  $n stole it!",
      "Crunch, crunch -- you munch a bottle.",
      "Crunch, crunch -- $n munches a bottle."}
     },

    {
     {
      "You turn everybody into a little pink elephant.",
      "You are turned into a little pink elephant by $n.",
      "An angel consults you.",
      "An angel consults $n.",
      "The dice roll ... and you win again.",
      "The dice roll ... and $n wins again.",
      "... 98, 99, 100 ... you do pushups.",
      "... 98, 99, 100 ... $n does pushups."}
     },

    {
     {
      "A small ball of light dances on your fingertips.",
      "A small ball of light dances on $n's fingertips.",
      "Your body glows with an unearthly light.",
      "$n's body glows with an unearthly light.",
      "You count the money in everyone's pockets.",
      "Check your money, $n is counting it.",
      "Arnold Schwarzenegger admires your physique.",
      "Arnold Schwarzenegger admires $n's physique."}
     },

    {
     {
      "Smoke and fumes leak from your nostrils.",
      "Smoke and fumes leak from $n's nostrils.",
      "A spot light hits you.",
      "A spot light hits $n.",
      "You balance a pocket knife on your tongue.",
      "$n balances a pocket knife on your tongue.",
      "Watch your feet, you are juggling granite boulders.",
      "Watch your feet, $n is juggling granite boulders."}
     },

    {
     {
      "The light flickers as you rap in magical languages.",
      "The light flickers as $n raps in magical languages.",
      "Everyone levitates as you pray.",
      "You levitate as $n prays.",
      "You produce a coin from everyone's ear.",
      "$n produces a coin from your ear.",
      "Oomph!  You squeeze water out of a granite boulder.",
      "Oomph!  $n squeezes water out of a granite boulder."}
     },

    {
     {
      "Your head disappears.",
      "$n's head disappears.",
      "A cool breeze refreshes you.",
      "A cool breeze refreshes $n.",
      "You step behind your shadow.",
      "$n steps behind $s shadow.",
      "You pick your teeth with a spear.",
      "$n picks $s teeth with a spear."}
     },

    {
     {
      "A fire elemental singes your hair.",
      "A fire elemental singes $n's hair.",
      "The sun pierces through the clouds to illuminate you.",
      "The sun pierces through the clouds to illuminate $n.",
      "Your eyes dance with greed.",
      "$n's eyes dance with greed.",
      "Everyone is swept off their foot by your hug.",
      "You are swept off your feet by $n's hug."}
     },

    {
     {
      "The sky changes color to match your eyes.",
      "The sky changes color to match $n's eyes.",
      "The ocean parts before you.",
      "The ocean parts before $n.",
      "You deftly steal everyone's weapon.",
      "$n deftly steals your weapon.",
      "Your karate chop splits a tree.",
      "$n's karate chop splits a tree."}
     },

    {
     {
      "The stones dance to your command.",
      "The stones dance to $n's command.",
      "A thunder cloud kneels to you.",
      "A thunder cloud kneels to $n.",
      "The Grey Mouser buys you a beer.",
      "The Grey Mouser buys $n a beer.",
      "A strap of your armor breaks over your mighty thews.",
      "A strap of $n's armor breaks over $s mighty thews."}
     },

    {
     {
      "The heavens and grass change colour as you smile.",
      "The heavens and grass change colour as $n smiles.",
      "The Burning Man speaks to you.",
      "The Burning Man speaks to $n.",
      "Everyone's pocket explodes with your fireworks.",
      "Your pocket explodes with $n's fireworks.",
      "A boulder cracks at your frown.",
      "A boulder cracks at $n's frown."}
     },

    {
     {
      "Everyone's clothes are transparent, and you are laughing.",
      "Your clothes are transparent, and $n is laughing.",
      "An eye in a pyramid winks at you.",
      "An eye in a pyramid winks at $n.",
      "Everyone discovers your dagger a centimeter from their eye.",
      "You discover $n's dagger a centimeter from your eye.",
      "Mercenaries arrive to do your bidding.",
      "Mercenaries arrive to do $n's bidding."}
     },

    {
     {
      "A black hole swallows you.",
      "A black hole swallows $n.",
      "Valentine Michael Smith offers you a glass of water.",
      "Valentine Michael Smith offers $n a glass of water.",
      "Where did you go?",
      "Where did $n go?",
      "Four matched Percherons bring in your chariot.",
      "Four matched Percherons bring in $n's chariot."}
     },

    {
     {
      "The world shimmers in time with your whistling.",
      "The world shimmers in time with $n's whistling.",
      "The great god Mota gives you a staff.",
      "The great god Mota gives $n a staff.",
      "Click.",
      "Click.",
      "Atlas asks you to relieve him.",
      "Atlas asks $n to relieve him."}
     }
};



void do_pose (CHAR_DATA * ch, char *argument)
{
    int level;
    int pose;

    if (IS_NPC (ch))
        return;

    level =
        UMIN (ch->level, sizeof (pose_table) / sizeof (pose_table[0]) - 1);
    pose = number_range (0, level);

    act (pose_table[pose].message[2 * ch->ch_class + 0], ch, NULL, NULL,
         TO_CHAR);
    act (pose_table[pose].message[2 * ch->ch_class + 1], ch, NULL, NULL,
         TO_ROOM);

    return;
}

*/

void do_bug (CHAR_DATA * ch, char *argument)
{
    append_file (ch, BUG_FILE, argument);
    SEND ("Bug logged.\r\n", ch);
    return;
}

void do_typo (CHAR_DATA * ch, char *argument)
{
    append_file (ch, TYPO_FILE, argument);
    SEND ("Typo logged.\r\n", ch);
    return;
}

void do_rent (CHAR_DATA * ch, char *argument)
{
    SEND ("There is no rent here.  Just save and quit.\r\n", ch);
    return;
}


void do_qui (CHAR_DATA * ch, char *argument)
{
    SEND ("If you want to QUIT, you have to spell it out.\r\n", ch);
    return;
}



void do_quit (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if (IS_NPC (ch))
        return;
	
	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL)
	{
		SEND("Not while charmed!\r\n",ch);
		return;
	}
	
    if (ch->position == POS_FIGHTING)
    {
        SEND ("No way! You are fighting.\r\n", ch);
        return;
    }

    if (ch->position < POS_STUNNED)
    {
        SEND ("You're not DEAD yet.\r\n", ch);
        return;
    }
	
	if (ch->onBoard)
	{
		SEND ("No quitting on ships.\r\n",ch);
		return;
	}
    SEND ("Alas, all good things must come to an end.\r\n", ch);
    act ("$n has left the game.", ch, NULL, NULL, TO_ROOM);
    sprintf (log_buf, "%s has quit. vnum: [%ld]", ch->name, ch->in_room->vnum);
    log_string (log_buf);
	char buf[MAX_STRING_LENGTH];
	sprintf (buf, "%s rejoins the real world. {xvnum: {b[{x%ld{b]{x", ch->name,ch->in_room->vnum);        
    wiznet (buf, NULL, NULL, WIZ_LOGINS, 0, get_trust(ch));   		
	
    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj (ch);

	/* Free note that might be there somehow */
	if (ch->pcdata->in_progress)
		free_note (ch->pcdata->in_progress);

    id = ch->id;
    d = ch->desc;
	
	//Free the account.
	if (d->account)
		free_account(d->account);
	
    extract_char (ch, TRUE);
    if (d != NULL)
        close_socket (d);

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
        CHAR_DATA *tch;

        d_next = d->next;
        tch = d->original ? d->original : d->character;
        if (tch && tch->id == id)
        {
            extract_char (tch, TRUE);
            close_socket (d);
        }
    }

    return;
}



void do_save (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    save_char_obj (ch);
		
	ACCOUNT_DATA * acc = ch->desc->account;	
	int i = getCharNumb(acc, ch->name);
		
	free_string( acc->char_list[i].char_name );
	acc->char_list[i].char_name = str_dup(ch->name);	

	acc->char_list[i].level 	= 	total_levels(ch);
	acc->char_list[i].ch_class 	= 	ch->ch_class;
	acc->char_list[i].race 	= 		ch->race;
	acc->char_list[i].clan 	= 		ch->clan;
	acc->char_list[i].rank 	= 		ch->clan_rank;
	
	save_account_obj(acc);
	//save_account_vault_obj(acc);

    SEND ("Saving character.. . .  .  .   .   .done!\r\n", ch);	
    return;
}



void do_follow (CHAR_DATA * ch, char *argument)
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Follow whom?\r\n", ch);
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL)
    {
        act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        if (ch->master == NULL)
        {
            SEND ("You already follow yourself.\r\n", ch);
            return;
        }
        stop_follower (ch);
        return;
    }

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_PET))
	{
		SEND("You can't follow someone's pet...\r\n",ch);
		return;
	}
	
    if (!IS_NPC (victim) && IS_SET (victim->act, PLR_NOFOLLOW)
        && !IS_IMMORTAL (ch))
    {
        act ("$N doesn't seem to want any followers.\r\n", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    REMOVE_BIT (ch->act, PLR_NOFOLLOW);

    if (ch->master != NULL)
        stop_follower (ch);

    add_follower (ch, victim);
    return;
}


void add_follower (CHAR_DATA * ch, CHAR_DATA * master)
{
    if (ch->master != NULL)
    {
        bug ("Add_follower: non-null master.", 0);
        return;
    }

    ch->master = master;
    ch->leader = NULL;
	ch->grank = G_BACK;
	
    if ((can_see (master, ch) && !is_affected(ch, gsn_sneak)) || IS_IMMORTAL(ch))
        act ("$n now follows you.", ch, NULL, master, TO_VICT);

	if (is_affected(ch, gsn_sneak))
		act ("You sneakily follow $N.", ch, NULL, master, TO_CHAR);
    else
		act ("You now follow $N.", ch, NULL, master, TO_CHAR);

    return;
}



void stop_follower (CHAR_DATA * ch)
{
    if (ch->master == NULL)
    {
        bug ("Stop_follower: null master.", 0);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM))
    {
        REMOVE_BIT (ch->affected_by, AFF_CHARM);
        affect_strip (ch, gsn_charm_person);
    }

    if (can_see (ch->master, ch) && ch->in_room != NULL)
    {
        act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
        act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    }
    if (ch->master->pet == ch)
        ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
	ch->grank = G_FRONT;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets (CHAR_DATA * ch)
{
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
        stop_follower (pet);
        if (pet->in_room != NULL)
            act ("$N slowly fades away.", ch, NULL, pet, TO_NOTVICT);
        extract_char (pet, TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower (CHAR_DATA * ch)
{
    CHAR_DATA *fch;

    if (ch->master != NULL)
    {
        if (ch->master->pet == ch)
            ch->master->pet = NULL;
        stop_follower (ch);
    }

    ch->leader = NULL;

    for (fch = char_list; fch != NULL; fch = fch->next)
    {
        if (fch->master == ch)
            stop_follower (fch);
			
        if (fch->leader == ch)
		{
            fch->leader = fch;			
			fch->grank = G_FRONT;
		}
			
    }

    return;
}



void do_order (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument (argument, arg);
    one_argument (argument, arg2);

    if (!str_cmp (arg2, "delete") || !str_cmp (arg2, "mob") || !str_cmp (arg2, "alias") || !str_cmp (arg2, "mclass") || !str_cmp (arg2, "sacrifice")
	|| !str_cmp (arg2, "embark") || !str_cmp (arg2, "disembark"))
    {
        SEND ("You cannot order them to do that.\r\n", ch);
        return;
    }

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        SEND ("Order whom to do what?\r\n", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM))
    {
        SEND ("You feel like taking, not giving, orders.\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        fAll = TRUE;
        victim = NULL;
    }
    else
    {
        fAll = FALSE;
        if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
        {
            SEND ("They aren't here.\r\n", ch);
            return;
        }

        if (victim == ch)
        {
            SEND ("Aye aye, right away!\r\n", ch);
            return;
        }

        if (!IS_AFFECTED (victim, AFF_CHARM) || victim->master != ch
            || (IS_IMMORTAL (victim) && victim->trust >= ch->trust))
        {
            SEND ("Do it yourself!\r\n", ch);
            return;
        }
    }

    found = FALSE;
    for (och = ch->in_room->people; och != NULL; och = och_next)
    {
        och_next = och->next_in_room;

        if (IS_AFFECTED (och, AFF_CHARM)
            && och->master == ch && (fAll || och == victim))
        {
            found = TRUE;
            sprintf (buf, "$n orders you to '%s'.", argument);
            act (buf, ch, NULL, och, TO_VICT);
            interpret (och, argument);
        }
    }

    if (found)
    {
        WAIT_STATE (ch, PULSE_VIOLENCE);
        SEND ("Ok.\r\n", ch);
    }
    else
        SEND ("You have no followers here.\r\n", ch);
    return;
}



void do_group (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
	char *rank;
    CHAR_DATA *victim;		
	CHAR_DATA * vch;
	CHAR_DATA * vch_next;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        CHAR_DATA *gch;
        CHAR_DATA *leader;

        leader = (ch->leader != NULL) ? ch->leader : ch;
        sprintf (buf, "%s's group:\r\n", PERS (leader, ch));		
        SEND (buf, ch);		
		
        for (gch = char_list; gch != NULL; gch = gch->next)
        {
            if (is_same_group (gch, ch))
            {
				switch (gch->grank)
				{
					default: 		rank = "none"; 	break;
					case G_NONE: 	rank = "none"; 	break;
					case G_FRONT:	rank = "front"; break;
					case G_BACK:	rank = "back";	break;
					case G_FLANK:	rank = "flank";	break;
				}													
				if (!IS_MCLASSED(gch))
				{
					sprintf (buf,
                         "{b[{x%5d   %s{b]%s{x %-16s{x %s%5d{D/{x%5d hp %s%5d{D/{x%5d mana %s%5d{D/{x%5d mv  {b[{x%5s{b]{x\r\n",
                         gch->level,
                         IS_NPC (gch) ? "Mob" :ch_class_table[gch->ch_class].who_name,
						 gch->bleeding < 1 ? "" : gch->bleeding < 5 ? "{r" : "{R",
                         capitalize (PERS (gch, ch)), LOW_HEALTH(gch) ? "{r" : "", gch->hit, gch->max_hit,
                         LOW_MANA(gch) ? "{r" : "", gch->mana, gch->max_mana, LOW_MOVE(gch) ? "{r" : "", gch->move, gch->max_move,
                         rank);
					SEND (buf, ch);
				}
				else
				{
					sprintf (buf,
                         "{b[{x%2d{D/{x%2d %5s{D/{x%5s{b]{x %s%-16s{x %s%5d{D/{x%5d hp %s%5d{D/{x%5d mana %s%5d{D/{x%5d mv  {b[{x%5s{b]{x\r\n",
                         gch->level, gch->level2,
                         ch_class_table[gch->ch_class].who_short,ch_class_table[gch->ch_class2].who_short,
						 gch->bleeding < 1 ? "" : gch->bleeding < 5 ? "{r" : "{R",
                         capitalize (PERS (gch, ch)), LOW_HEALTH(gch) ? "{r" : "", gch->hit, gch->max_hit,
                         LOW_MANA(gch) ? "{r" : "", gch->mana, gch->max_mana, LOW_MOVE(gch) ? "{r" : "", gch->move, gch->max_move, 
						 rank);
					SEND (buf, ch);
				}
            }
        }
        return;
    }

	if (!str_cmp(arg, "flank"))
	{
		if (!ch->leader && group_size(ch) < 1)
		{
			SEND ("You can't switch ranks unless you're in a group.\r\n",ch);
			return;
		}
		
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
	
	if (!str_cmp(arg, "back"))
	{
		if (!ch->leader && group_size(ch) < 1)
		{
			SEND ("You can't switch ranks unless you're in a group.\r\n",ch);
			return;
		}
		
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
	
	if (!str_cmp(arg, "front"))
	{
		if (!ch->leader && group_size(ch) < 1)
		{
			SEND ("You can't switch ranks unless you're in a group.\r\n",ch);
			return;
		}
		
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
	
	
	
    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
        return;
    }

    if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
        SEND ("But you are following someone else!\r\n", ch);
        return;
    }

    if (victim->master != ch && ch != victim)
    {
        act_new ("$N isn't following you.", ch, NULL, victim, TO_CHAR,
                 POS_SLEEPING);
        return;
    }

    if (IS_AFFECTED (victim, AFF_CHARM))
    {
        SEND ("You can't remove charmed mobs from your group.\r\n",ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM))
    {
        act_new ("You like your master too much to leave $m!",
                 ch, NULL, victim, TO_VICT, POS_SLEEPING);
        return;
    }

    if (is_same_group (victim, ch) && ch != victim)
    {
        victim->leader = NULL;
		victim->grank = G_FRONT;
        act_new ("$n removes $N from $s group.",
                 ch, NULL, victim, TO_NOTVICT, POS_RESTING);
        act_new ("$n removes you from $s group.",
                 ch, NULL, victim, TO_VICT, POS_SLEEPING);
        act_new ("You remove $N from your group.",
                 ch, NULL, victim, TO_CHAR, POS_SLEEPING);		
        return;
    }

    victim->leader = ch;
	victim->grank = G_BACK;
    act_new ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT,
             POS_RESTING);
    act_new ("You join $n's group.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
    act_new ("$N joins your group.", ch, NULL, victim, TO_CHAR, POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument (argument, arg1);
    one_argument (argument, arg2);

    if (arg1[0] == '\0')
    {
        SEND ("Split how much?\r\n", ch);
        return;
    }

    amount_silver = atoi (arg1);

    if (arg2[0] != '\0')
        amount_gold = atoi (arg2);

    if (amount_gold < 0 || amount_silver < 0)
    {
        SEND ("Your group wouldn't like that.\r\n", ch);
        return;
    }

    if (amount_gold == 0 && amount_silver == 0)
    {
        SEND ("You hand out zero coins, but no one notices.\r\n", ch);
        return;
    }

    if (ch->gold < amount_gold || ch->silver < amount_silver)
    {
        SEND ("You don't have that much to split.\r\n", ch);
        return;
    }

    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (is_same_group (gch, ch) && !IS_AFFECTED (gch, AFF_CHARM))
            members++;
    }

    if (members < 2)
    {
        SEND ("Just keep it all.\r\n", ch);
        return;
    }

    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold = amount_gold / members;
    extra_gold = amount_gold % members;

    if (share_gold == 0 && share_silver == 0)
    {
        SEND ("Don't even bother, cheapskate.\r\n", ch);
        return;
    }

    ch->silver -= amount_silver;
    ch->silver += share_silver + extra_silver;
    ch->gold -= amount_gold;
    ch->gold += share_gold + extra_gold;

    if (share_silver > 0)
    {
        sprintf (buf,
                 "You split %d silver coins. Your share is %d silver.\r\n",
                 amount_silver, share_silver + extra_silver);
        SEND (buf, ch);
    }

    if (share_gold > 0)
    {
        sprintf (buf,
                 "You split %d gold coins. Your share is %d gold.\r\n",
                 amount_gold, share_gold + extra_gold);
        SEND (buf, ch);
    }

    if (share_gold == 0)
    {
        sprintf (buf, "$n splits %d silver coins. Your share is %d silver.",
                 amount_silver, share_silver);
    }
    else if (share_silver == 0)
    {
        sprintf (buf, "$n splits %d gold coins. Your share is %d gold.",
                 amount_gold, share_gold);
    }
    else
    {
        sprintf (buf,
                 "$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\r\n",
                 amount_silver, amount_gold, share_silver, share_gold);
    }

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (gch != ch && is_same_group (gch, ch)
            && !IS_AFFECTED (gch, AFF_CHARM))
        {
            act (buf, ch, NULL, gch, TO_VICT);
            gch->gold += share_gold;
            gch->silver += share_silver;
        }
    }

    return;
}



void do_gtell (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *gch;
	char buf[MSL];

    if (argument[0] == '\0')
    {
        SEND ("Tell your group what?\r\n", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_NOTELL))
    {
        SEND ("Your message didn't get through!\r\n", ch);
        return;
    }

    for (gch = char_list; gch != NULL; gch = gch->next)
    {
        if (is_same_group (gch, ch))
            act_new ("$n {Dtells{x the group '$t'",
                     ch, argument, gch, TO_VICT, POS_SLEEPING);
					 
    }
	
	sprintf (buf, "You {Dtell{x the group '%s'{x\r\n", argument);
    SEND (buf, ch);
	
    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group (CHAR_DATA * ach, CHAR_DATA * bch)
{
    if (ach == NULL || bch == NULL)
        return FALSE;

    if (ach->leader != NULL)
        ach = ach->leader;
    if (bch->leader != NULL)
        bch = bch->leader;
    return ach == bch;
}

/*
 * ColoUr setting and unsetting, way cool, Ant Oct 94
 *        revised to include config colour, Ant Feb 95
 */
void do_colour (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        SEND_bw ("ColoUr is not ON, Way Moron!\r\n", ch);
        return;
    }

    argument = one_argument (argument, arg);

    if (!*arg)
    {
        if (!IS_SET (ch->act, PLR_COLOUR))
        {
            SET_BIT (ch->act, PLR_COLOUR);
            SEND ("ColoUr is now ON, Way Cool!\r\n"
                          "Further syntax:\r\n   colour {c<{xfield{c> <{xcolour{c>{x\r\n"
                          "   colour {c<{xfield{c>{x {cbeep{x|{cnobeep{x\r\n"
                          "Type help {ccolour{x and {ccolour2{x for details.\r\n"
                          "ColoUr is brought to you by Lope, ant@solace.mh.se.\r\n",
                          ch);
        }
        else
        {
            SEND_bw ("ColoUr is now OFF, <sigh>\r\n", ch);
            REMOVE_BIT (ch->act, PLR_COLOUR);
        }
        return;
    }

    if (!str_cmp (arg, "default"))
    {
        default_colour (ch);
        SEND_bw ("ColoUr setting set to default values.\r\n", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        all_colour (ch, argument);
        return;
    }

    /*
     * Yes, I know this is ugly and unnessessary repetition, but its old
     * and I can't justify the time to make it pretty. -Lope
     */
    if (!str_cmp (arg, "text"))
    {
    ALTER_COLOUR (text)}
    else if (!str_cmp (arg, "auction"))
    {
    ALTER_COLOUR (auction)}
    else if (!str_cmp (arg, "auction_text"))
    {
    ALTER_COLOUR (auction_text)}
    else if (!str_cmp (arg, "gossip"))
    {
    ALTER_COLOUR (gossip)}
    else if (!str_cmp (arg, "gossip_text"))
    {
    ALTER_COLOUR (gossip_text)}
    else if (!str_cmp (arg, "music"))
    {
    ALTER_COLOUR (music)}
    else if (!str_cmp (arg, "music_text"))
    {
    ALTER_COLOUR (music_text)}
    else if (!str_cmp (arg, "question"))
    {
    ALTER_COLOUR (question)}
    else if (!str_cmp (arg, "question_text"))
    {
    ALTER_COLOUR (question_text)}
    else if (!str_cmp (arg, "answer"))
    {
    ALTER_COLOUR (answer)}
    else if (!str_cmp (arg, "answer_text"))
    {
    ALTER_COLOUR (answer_text)}
    else if (!str_cmp (arg, "quote"))
    {
    ALTER_COLOUR (quote)}
    else if (!str_cmp (arg, "quote_text"))
    {
    ALTER_COLOUR (quote_text)}
    else if (!str_cmp (arg, "immtalk_text"))
    {
    ALTER_COLOUR (immtalk_text)}
    else if (!str_cmp (arg, "immtalk_type"))
    {
    ALTER_COLOUR (immtalk_type)}
    else if (!str_cmp (arg, "info"))
    {
    ALTER_COLOUR (info)}
    else if (!str_cmp (arg, "say"))
    {
    ALTER_COLOUR (say)}
    else if (!str_cmp (arg, "say_text"))
    {
    ALTER_COLOUR (say_text)}
    else if (!str_cmp (arg, "tell"))
    {
    ALTER_COLOUR (tell)}
    else if (!str_cmp (arg, "tell_text"))
    {
    ALTER_COLOUR (tell_text)}
    else if (!str_cmp (arg, "reply"))
    {
    ALTER_COLOUR (reply)}
    else if (!str_cmp (arg, "reply_text"))
    {
    ALTER_COLOUR (reply_text)}
    else if (!str_cmp (arg, "gtell_text"))
    {
    ALTER_COLOUR (gtell_text)}
    else if (!str_cmp (arg, "gtell_type"))
    {
    ALTER_COLOUR (gtell_type)}
    else if (!str_cmp (arg, "wiznet"))
    {
    ALTER_COLOUR (wiznet)}
    else if (!str_cmp (arg, "room_title"))
    {
    ALTER_COLOUR (room_title)}
    else if (!str_cmp (arg, "room_text"))
    {
    ALTER_COLOUR (room_text)}
    else if (!str_cmp (arg, "room_exits"))
    {
    ALTER_COLOUR (room_exits)}
    else if (!str_cmp (arg, "room_things"))
    {
    ALTER_COLOUR (room_things)}
    else if (!str_cmp (arg, "prompt"))
    {
    ALTER_COLOUR (prompt)}
    else if (!str_cmp (arg, "fight_death"))
    {
    ALTER_COLOUR (fight_death)}
    else if (!str_cmp (arg, "fight_yhit"))
    {
    ALTER_COLOUR (fight_yhit)}
    else if (!str_cmp (arg, "fight_ohit"))
    {
    ALTER_COLOUR (fight_ohit)}
    else if (!str_cmp (arg, "fight_thit"))
    {
    ALTER_COLOUR (fight_thit)}
    else if (!str_cmp (arg, "fight_skill"))
    {
    ALTER_COLOUR (fight_skill)}
    else
    {
        SEND_bw ("Unrecognised Colour Parameter Not Set.\r\n", ch);
        return;
    }

    SEND_bw ("New Colour Parameter Set.\r\n", ch);
    return;
}

/* How to make a string look drunk... by Apex (robink@htsa.hva.nl) */
/* Modified and enhanced for envy(2) by the Maniac from Mythran */
/* Further mods/upgrades for ROM 2.4 by Kohl Desenee */


char * makedrunk (char *string, CHAR_DATA * ch)
{
// This structure defines all changes for a character 
  struct struckdrunk drunk[] =
  {
    {3, 10,
     {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,
     {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 6,
     {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

  char buf[1024];
  char temp;
  int pos = 0;
  int drunklevel;
  int randomnum;

  // Check how drunk a person is... 
  if (IS_NPC(ch))
        drunklevel = 0;
  else
        drunklevel = ch->pcdata->condition[COND_DRUNK];

  if (drunklevel > 0)
    {
      do
        {
          temp = toupper (*string);
          if ((temp >= 'A') && (temp <= 'Z'))
            {
              if (drunklevel > drunk[temp - 'A'].min_drunk_level)
                {
                  randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
                  strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
                  pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
                }
              else
                buf[pos++] = *string;
            }
          else
            {
              if ((temp >= '0') && (temp <= '9'))
                {
                  temp = '0' + number_range (0, 9);
                  buf[pos++] = temp;
                }
              else
                buf[pos++] = *string;
            }
        }
      while (*string++);
      buf[pos] = '\0';          // Mark end of the string... 
      strcpy(string, buf);
      return(string);
    }
  return (string);
}



void do_noexp(CHAR_DATA *ch, char *argument)
{	

    if (IS_NPC (ch))
    {
        SEND ("Not as an NPC.\r\n", ch);
        return;
    }
	
	if (argument[0] == '\0')
	{
		if (IS_MCLASSED(ch))
		{
			SEND("Which class will you stop/start advancing? Primary or secondary?\r\n",ch);
			return;
		}
		else
		{
			if (ch->no_exp[0] == TRUE)
			{
				SEND("You are gaining experience as normal again.\r\n",ch);
				ch->no_exp[0] = FALSE;
				return;
			}
			else
			{
				SEND("You are no longer gaining experience normally.\r\n",ch);
				ch->no_exp[0] = TRUE;
				return;
			}
		}
	}
	else
	{
		if (!strcmp(argument, "primary"))
		{
			if (ch->no_exp[0] == TRUE)
			{
				SEND("You are gaining experience as normal again.\r\n",ch);
				ch->no_exp[0] = FALSE;
				return;
			}
			else
			{
				SEND("You are no longer gaining experience normally.\r\n",ch);
				ch->no_exp[0] = TRUE;
				return;
			}
		}
		else if (!strcmp(argument, "secondary"))
		{
			if (ch->no_exp[1] == TRUE)
			{
				SEND("You are gaining experience as normal again.\r\n",ch);
				ch->no_exp[1] = FALSE;
				return;
			}
			else
			{
				SEND("You are no longer gaining experience normally.\r\n",ch);
				ch->no_exp[1] = TRUE;
				return;
			}
		}
		else
		{
			SEND("Not a valid argument.\r\n",ch);
			return;
		}
	}
	
	return;
}

void logout(DESCRIPTOR_DATA *dlogout, CHAR_DATA *vch) {

    CHAR_DATA *ch;
    DESCRIPTOR_DATA *d;
    OBJ_DATA *obj, *obj_next;
    ROOM_INDEX_DATA *room, *roomold;
	CHAR_DATA *vch_next;
    int id;
        
    if ( vch && !dlogout )
		ch = vch;
    else if ( !vch && dlogout )
		ch = dlogout->character;
    else
    {
		log_string( "desc + ch in logout." );
		return;
    }

    if (!ch || IS_NPC(ch)) 
        return;
    
    if (ch->desc && !dlogout)
		dlogout = ch->desc;

    if ( dlogout && dlogout->snoop_by )
    {
//        DESCRIPTOR_DATA *d_next;

        write_to_buffer( dlogout->snoop_by, "Your victim has left the game.\n\r", 0 );
         
/*
        for ( d = descriptor_list; d; d = d_next )
        {
            d_next = d->next;

            if (d->snoop_by == dlogout)
                d->snoop_by = NULL;
        }
*/        
    }


    if (total_levels(ch) >= 41)        
        do_snoop(ch, "self"); /* cancel all snoops */

    switch( number_range(0,10))
    {
		case 0 : SEND("Alas, all good things must come to an end.\n\r",ch);
			break;	
		case 1 :  SEND("We'll be waiting for you to return.\n\r",ch);
			break;
		case 2 :  SEND("Real life, humbug.\n\r",ch);
			break;
		case 3 :  SEND("Sunlight, what?  Never.\n\r",ch);
			break;
		case 4 :  SEND("That stuff on your fingers?  It's heroin of course.\n\r",ch);
			break;
		case 5 :  SEND("Close your eyes, it will be all right.\n\r",ch);
			break;
		case 6 :  SEND("Say cheese.  Monterey Jack.\n\r",ch);
			break;
		case 7 :  SEND("Goodbye cruel world.\n\r",ch);
			break;
		case 8 :  SEND("The last spec of existence fades from view.\n\r",ch);
			break;
		case 9 : SEND("Your body disintigrates, you think.\n\r",ch);
			break;
		case 10 : SEND("(insert nothing here)\n\r",ch);
			break;	
    }

    act( "$n is consumed by eerie {Dshadows{x...", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s returns to their real life activities.", ch->name );
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_LOGINS,WIZ_SITES,get_trust(ch));

    if (ch->desc && IS_SET(ch->desc->account->act, ACC_HOSTMASK))
    { 
        sprintf( log_buf, "%s@%s logs out", ch->name, ch->desc->host);
        log_string( log_buf );
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
        sprintf( log_buf, "%s real host is: {C%s{x", ch->name, ch->desc->host);
        log_string( log_buf );
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,UMAX(60,get_trust(ch)));
    }
    else 
    {
		sprintf( log_buf, "%s@%s logs out", ch->name, ( ch->desc ? ch->desc->host : "" ));
			log_string( log_buf );
			wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
    }		
	if (ch->grank == G_FRONT && !IS_NPC(ch))
	{
		for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( is_same_group( ch, vch ) && vch->grank != G_FRONT)
			{
				vch->grank = G_FRONT;
				SEND("Your group leader has {DQUIT{x.\r\n",vch);
				SEND("You have been moved to the front of the group.\r\n",vch);
				break;
			}
		}			
	}
	ch->leader = NULL;	
	ch->grank = G_BACK;
    save_char_obj( ch );    
    d = ch->desc;

    extract( ch, TRUE );
}

bool can_logout( CHAR_DATA *ch )
{
    if ( IS_NPC(ch) )
		return FALSE;
    if(IS_IMMORTAL(ch))
		return true;

    if (IS_AFFECTED(ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_SLEEP))
    {
		SEND("You're not getting out of it that easily!\r\n", ch);
		return FALSE;
    }

/*    if ( IS_MOUNTED( ch ))
    {
        SEND("You cannot quit while mounted!\r\n", ch);
        return FALSE;
    }

    if ( IS_RIDING( ch ));
    {
        SEND("You cannot quit while riding!\r\n", ch);
        return FALSE;
    } */

    if ( ch->position == POS_FIGHTING )
    {
		SEND( "No way! You are fighting.\r\n", ch );
		return FALSE;
    }

    if ( ch->position  < POS_STUNNED  )
    {
		SEND( "You're not DEAD yet.\r\n", ch );
		return FALSE;
    }   
    /*if ( !IS_IMMORTAL(ch) && is_clan( ch ) && ch->in_room != NULL
    && ch->in_room->clan != 0 && ch->in_room->clan != ch->clan )
    {
	SEND( "A mystical force prevents you from doing this.\r\n",ch);
	do_yell( ch, "Wouldn't it be devious if you could quit in someone else's clanhall?" );
	return FALSE;
    }*/
    /*if (ch->pnote != NULL && ch->timer <= 18)
    {
	SEND("You have a note in progress.\r\n", ch);
	return FALSE;
    }*/
    /*if (ch->pktimer > 0)
    {
		SEND("Not so quick...\r\n", ch);
		return FALSE;
    }*/
    return TRUE;
}

void do_logout( CHAR_DATA *ch, char *argument )
{        
    save_char_obj( ch );

    if (can_logout( ch )) {
        DESCRIPTOR_DATA *desc = ch->desc;

        logout( NULL, ch );
        // ch no longer VALID!
            
        if ( desc ) {
	    // resetting some vital info between characters.
	    desc->snoop_by = NULL;
	    desc->pEdit = NULL;
	    desc->pString = NULL;
	    desc->editor = 0;
	    desc->pload = false;

            desc->connected = CON_ACCOUNT;
            account_main_menu( desc );
            return;
        }
    }
    return;
}

void do_eqcondition (CHAR_DATA *ch, char *argument)
{
	if ( ch->pcdata->eqcondition )
    {
		ch->pcdata->eqcondition = 0;
		SEND ("You are no longer seeing equipment conditions.\r\n",ch);
		return;
	}
	else
	{
		ch->pcdata->eqcondition = 1;
		SEND ("You will now see equipment conditions.\r\n",ch);
		return;
	}
}

void do_beeptell (CHAR_DATA *ch, char *argument)
{
	if ( ch->pcdata->beeptells )
    {
		ch->pcdata->beeptells = 0;
		SEND ("You will no longer hear beeps in {Ctells{x.\r\n",ch);
		return;
	}
	else
	{
		ch->pcdata->beeptells = 1;
		SEND ("You will now hear beeps in {Ctells{x.\r\n",ch);
		return;
	}
}

void do_spellup (CHAR_DATA * ch, char * argument)
{
	char buf[MSL];      

	if (!is_affected (ch, skill_lookup ("extension")))
		do_cast (ch, "extension");

	if (!is_affected (ch, skill_lookup ("armor")))
		do_cast (ch, "armor");

	if (!is_affected (ch, skill_lookup ("bless")))
		do_cast (ch, "bless");

	if (!is_affected (ch, skill_lookup ("detect evil")))
	{
		sprintf (buf, "'detect evil' %s", ch->name);
		do_cast (ch, buf);
	}

	if (!is_affected (ch, skill_lookup ("detect good")))
	{
		sprintf (buf, "'detect good' %s", ch->name);
		do_cast (ch, buf);
	}     

	if (!is_affected (ch, skill_lookup ("detect hidden")))
	{
		sprintf (buf, "'detect hidden' %s", ch->name);
		do_cast (ch, buf);
	}  

	if (!is_affected (ch, skill_lookup ("detect invis")))
	{
		sprintf (buf, "'detect invis' %s", ch->name);
		do_cast (ch, buf);
	}  

	if (!is_affected (ch, skill_lookup ("detect magic")))
	{
		sprintf (buf, "'detect magic' %s", ch->name);
		do_cast (ch, buf);
	}  

	if (!is_affected (ch, skill_lookup ("fly")))
		do_cast (ch, "fly");

	if (!is_affected (ch, skill_lookup ("frenzy")))
		do_cast (ch, "frenzy");

	if (!is_affected (ch, skill_lookup ("haste")))
		do_cast (ch, "haste");

	if (!is_affected (ch, skill_lookup ("infravision")))
	{
		sprintf (buf, "'infravision' %s", ch->name);
		do_cast (ch, buf);
	}

	if (!is_affected (ch, skill_lookup ("invis")))
		do_cast (ch, "invis");

	if (!is_affected (ch, skill_lookup ("giant strength")))
	{
		sprintf (buf, "'giant strength' %s", ch->name);
		do_cast (ch, buf);
	}  

	if (!is_affected (ch, skill_lookup ("pass door")))
	{
		sprintf (buf, "'pass door' %s", ch->name);
		do_cast (ch, buf);
	}

	if (!is_affected (ch, skill_lookup ("sanctuary")))
		do_cast (ch, "sanctuary");

	if (!is_affected (ch, skill_lookup ("shield")))
		do_cast (ch, "shield");

	if (!is_affected (ch, skill_lookup ("stone skin")))
	{
		sprintf (buf, "'stone skin' %s", ch->name);
		do_cast (ch, buf);
	}     

	if (!is_affected (ch, skill_lookup ("barkskin")))
	{
		sprintf (buf, "'barkskin' %s", ch->name);
		do_cast (ch, buf);
	}     

	return;
}
