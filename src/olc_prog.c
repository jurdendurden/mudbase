/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
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
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/
 
/***************************************************************************
 *                                                                         *
 *  MOBprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
 *  Based on MERC 2.2 MOBprograms concept by N'Atas-ha.                    *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *  This code may be copied and distributed as per the ROM license.        *
 *                                                                         *
 ***************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include "merc.h"
#include "olc.h"
#include "recycle.h"
#include "interp.h"
 
 
SLEEP_DATA *first_sleep = NULL;

extern int flag_lookup( const char *word, const struct flag_type *flag_table );
 
/*
 * These defines correspond to the entries in fn_keyword[] table.
 * If you add a new if_check, you must also add a #define here.
 */
#define CHK_RAND   	(0)
#define CHK_MOBHERE     (1)
#define CHK_OBJHERE     (2)
#define CHK_MOBEXISTS   (3)
#define CHK_OBJEXISTS   (4)
#define CHK_PEOPLE      (5)
#define CHK_PLAYERS     (6)
#define CHK_MOBS        (7)
#define CHK_CLONES      (8)
#define CHK_ORDER       (9)
#define CHK_HOUR        (10)
#define CHK_ISPC        (11)
#define CHK_ISNPC       (12)
#define CHK_ISGOOD      (13)
#define CHK_ISEVIL      (14)
#define CHK_ISNEUTRAL   (15)
#define CHK_ISIMMORT    (16)
#define CHK_ISCHARM     (17)
#define CHK_ISFOLLOW    (18)
#define CHK_ISACTIVE    (19)
#define CHK_ISDELAY     (20)
#define CHK_ISVISIBLE   (21)
#define CHK_HASTARGET   (22)
#define CHK_ISTARGET    (23)
#define CHK_EXISTS      (24)
#define CHK_AFFECTED    (25)
#define CHK_ACT         (26)
#define CHK_OFF         (27)
#define CHK_IMM         (28)
#define CHK_CARRIES     (29)
#define CHK_WEARS       (30)
#define CHK_HAS         (31)
#define CHK_USES        (32)
#define CHK_NAME        (33)
#define CHK_POS         (34)
#define CHK_CLAN        (35)
#define CHK_RACE        (36)
#define CHK_CLASS       (37)
#define CHK_OBJTYPE     (38)
#define CHK_VNUM        (39)
#define CHK_HPCNT       (40)
#define CHK_ROOM        (41)
#define CHK_SEX         (42)
#define CHK_LEVEL       (43)
#define CHK_ALIGN       (44)
#define CHK_MONEY       (45)
#define CHK_OBJVAL0     (46)
#define CHK_OBJVAL1     (47)
#define CHK_OBJVAL2     (48)
#define CHK_OBJVAL3     (49)
#define CHK_OBJVAL4     (50)
#define CHK_GRPSIZE     (51)
#define CHK_GOD			(52)
#define CHK_ON_QUEST	(53)
#define CHK_COMPLETED_QUEST (54)
#define CHK_SOCIETY_RANK (55)
 
/*
 * These defines correspond to the entries in fn_evals[] table.
 */
#define EVAL_EQ            0	// ==
#define EVAL_GE            1	// >=
#define EVAL_LE            2	// <=
#define EVAL_GT            3	// >
#define EVAL_LT            4	// <
#define EVAL_NE            5	// !=



#define MPEDIT( fun )           bool fun(CHAR_DATA *ch, char*argument)
#define OPEDIT( fun )		bool fun(CHAR_DATA *ch, char*argument)
#define RPEDIT( fun )		bool fun(CHAR_DATA *ch, char*argument)
 
const struct olc_cmd_type mpedit_table[] =
{
/*	{	command		function	}, */
 
	{	"commands",	show_commands	},
	{	"create",	mpedit_create	},
	{	"code",		mpedit_code	},
	{	"show",		mpedit_show	},
	{	"list",		mpedit_list	},
	{	"?",		show_help	},
 
	{	NULL,		0		}
};
 
const struct olc_cmd_type opedit_table[] =
{
/*	{	command		function	}, */
 
	{	"commands",	show_commands	},
	{	"create",	opedit_create	},
	{	"code",		opedit_code	},
	{	"show",		opedit_show	},
	{	"list",		opedit_list	},
	{	"?",		show_help	},
 
	{	NULL,		0		}
};
 
const struct olc_cmd_type rpedit_table[] =
{
/*	{	command		function	}, */
 
	{	"commands",	show_commands	},
	{	"create",	rpedit_create	},
	{	"code",		rpedit_code	},
	{	"show",		rpedit_show	},
	{	"list",		rpedit_list	},
	{	"?",		show_help	},
 
	{	NULL,		0		}
};
 
 
DECLARE_DO_FUN( do_look 	);
extern ROOM_INDEX_DATA *find_location( CHAR_DATA *, char * );
 
/*
 * MOBcommand table.
 */
const	struct	mob_cmd_type	mob_cmd_table	[] =
{
    {	"asound", 	do_mpasound	},
    {	"gecho",	do_mpgecho	},
    {	"zecho",	do_mpzecho	},
    {	"kill",		do_mpkill	},
    {	"assist",	do_mpassist	},
    {	"junk",		do_mpjunk	},
    {	"echo",		do_mpecho	},
    {	"echoaround",	do_mpechoaround	},
    {	"echoat",	do_mpechoat	},
    {	"mload",	do_mpmload	},
    {	"oload",	do_mpoload	},
    {	"purge",	do_mppurge	},
    {	"goto",		do_mpgoto	},
    {	"at",		do_mpat		},
    {	"transfer",	do_mptransfer	},
    {	"gtransfer",	do_mpgtransfer	},
    {	"otransfer",	do_mpotransfer	},
    {	"force",	do_mpforce	},
    {	"gforce",	do_mpgforce	},
    {	"vforce",	do_mpvforce	},
    {	"cast",		do_mpcast	},
    {	"damage",	do_mpdamage	},
    {	"remember",	do_mpremember	},
    {	"forget",	do_mpforget	},
    {	"delay",	do_mpdelay	},
    {	"cancel",	do_mpcancel	},
    {	"call",		do_mpcall	},
    {	"flee",		do_mpflee	},
    {	"remove",	do_mpremove	},
	{	"addgp",	do_mpaddgp 	},
	{	"addxp",	do_mpaddxp	},	
	{	"addfaction",	do_mpaddfaction	},
    {	"",		0		}
};
 
 
/*
 * OBJcommand table.
 */
const	struct	obj_cmd_type	obj_cmd_table	[] =
{
    {	"gecho",       	do_opgecho	},
    {	"zecho",	do_opzecho	},
    {	"echo",		do_opecho	},
    {	"echoaround",	do_opechoaround	},
    {	"echoat",	do_opechoat	},
    {	"mload",	do_opmload	},
    {	"oload",	do_opoload	},
    {	"purge",	do_oppurge	},
    {	"goto",		do_opgoto	},
    {	"transfer",	do_optransfer	},
    {	"gtransfer",	do_opgtransfer	},
    {	"otransfer",	do_opotransfer	},
    {	"force",	do_opforce	},
    {	"gforce",	do_opgforce	},
    {	"vforce",	do_opvforce	},
    {	"damage",	do_opdamage	},
    {	"remember",	do_opremember	},
    {	"forget",	do_opforget	},
    {	"delay",	do_opdelay	},
    {	"cancel",	do_opcancel	},
    {	"call",		do_opcall	},
    {	"remove",	do_opremove	},
	{	"changealign", do_opchange_align },
    {	"",		0		}
};
 
/*
 * ROOMcommand table.
 */
const	struct  room_cmd_type	room_cmd_table	[] =
{
    {	"asound",	do_rpasound	},
    {	"gecho",	do_rpgecho	},
    {	"zecho",	do_rpzecho	},
    {	"echo",		do_rpecho 	},
    {	"echoaround",	do_rpechoaround },
    {	"echoat",	do_rpechoat 	},
    {	"mload",	do_rpmload 	},
    {	"oload",	do_rpoload 	},
    {	"purge",	do_rppurge 	},
    {	"transfer",	do_rptransfer 	},
    {	"gtransfer",	do_rpgtransfer 	},
    {	"otransfer",	do_rpotransfer 	},
    {	"force",	do_rpforce 	},
    {	"gforce",	do_rpgforce 	},
    {	"vforce",	do_rpvforce 	},
    {	"damage",       do_rpdamage 	},
    {	"remember",	do_rpremember 	},
    {	"forget",	do_rpforget 	},
    {	"delay",	do_rpdelay 	},
    {	"cancel",	do_rpcancel 	},
    {	"call",		do_rpcall 	},
    {	"remove",	do_rpremove 	},
    {	"",		0 		},
};
 
char *prog_type_to_name( int type )
{
    switch ( type )
    {
    case TRIG_ACT:             	return "ACT";
    case TRIG_SPEECH:          	return "SPEECH";
    case TRIG_RANDOM:          	return "RANDOM";
    case TRIG_FIGHT:           	return "FIGHT";
    case TRIG_HPCNT:           	return "HPCNT";
    case TRIG_DEATH:           	return "DEATH";
    case TRIG_ENTRY:           	return "ENTRY";
    case TRIG_GREET:           	return "GREET";
    case TRIG_GRALL:        	return "GRALL";
    case TRIG_GIVE:            	return "GIVE";
    case TRIG_BRIBE:           	return "BRIBE";
    case TRIG_KILL:	      	return "KILL";
    case TRIG_DELAY:           	return "DELAY";
    case TRIG_SURR:	      	return "SURRENDER";
    case TRIG_EXIT:	      	return "EXIT";
    case TRIG_EXALL:	      	return "EXALL";
    case TRIG_GET:		return "GET";
    case TRIG_DROP:		return "DROP";
    case TRIG_SIT:		return "SIT";
    default:                  	return "ERROR";
    }
}
 
 
void do_mpaddfaction ( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	arg1 [ MIL ];
	char	arg2 [ MIL ];
	char	arg3 [ MIL ];
	char	buf  [ MSL ];
	int value, i;
	
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );    
	
	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')	
		return;
    
	value = atoi(arg3);
	i = atoi(arg2);
	
	if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {    
		return;
    }
	
    if ( IS_NPC( victim ) )
    {
		return;
    }        
	
	if ( value > 2000 )
		value = 2000;
	
	if ( value < -2000 )
		value = -2000;
	
    if ( value == 0 )
    {	
		return;
    }
	
	//for (i = 0; i <= MAX_FACTION; i++)
	//{
	//	if (!str_cmp(faction_table[i].name, arg2))
	//	{
			ch->faction_rep[i] += value;
			sprintf( buf,"You gain %d {gfaction rep{x with %s!\r\n", value, faction_table[i].name );
			SEND( buf, victim );
			//break;
		//}
		//else
//			return;
	//}

    return;
	
}	

 
void do_mpaddxp ( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;    
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {	
		return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {    
      return;
    }

    if ( IS_NPC( victim ) )
    {
		return;
    }    

    value = atoi( arg2 );
	
	if ( value > 2000 )
		value = 2000;
	
	if ( value < -2000 )
		value = -2000;
	
    if ( value == 0 )
    {	
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
		return;
	}	    

    return;
}
 
 
 
 
 
void do_mpaddgp ( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {	
		return;
    }

    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {    
      return;
    }

    if ( IS_NPC( victim ) )
    {
		return;
    }    

    value = atoi( arg2 );
	
	if ( value > 100 )
		value = 100;
	
	if ( value < -100 )
		value = -100;
	
    if ( value == 0 )
    {	
		return;
    }
    
	victim->guildpoints += value;	
	    
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
 
 
/*
 * MOBprog section
 */
 
void do_mob( CHAR_DATA *ch, char *argument )
{
    /*
     * Security check!
     */
    if ( ch->desc != NULL && get_trust(ch) < MAX_LEVEL )
	return;
    mob_interpret( ch, argument );
}
/*
 * Mob command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret()
 */
void mob_interpret( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH], command[MAX_INPUT_LENGTH];
    int cmd;
 
    argument = one_argument( argument, command );
 
    /*
     * Look for command in command table.
     */
    for ( cmd = 0; mob_cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == mob_cmd_table[cmd].name[0]
	&&   !str_prefix( command, mob_cmd_table[cmd].name ) )
	{
	    (*mob_cmd_table[cmd].do_fun) ( ch, argument );
	    tail_chain( );
	    return;
	}
    }
    sprintf( buf, "Mob_interpret: invalid cmd from mob %ld: '%s'",
	IS_NPC(ch) ? ch->pIndexData->vnum : 0, command );
    bug( buf, 0 );
}
 
/* 
 * Displays MOBprogram triggers of a mobile
 *
 * Syntax: mpstat [name]
 */
void do_mpstat( CHAR_DATA *ch, char *argument )
{
    char        arg[ MAX_STRING_LENGTH  ];
    PROG_LIST  *mprg;
    CHAR_DATA   *victim;
    int i;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	SEND( "Mpstat whom?\n\r", ch );
	return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	SEND( "No such creature.\n\r", ch );
	return;
    }
 
    if ( !IS_NPC( victim ) )
    {
	SEND( "That is not a mobile.\n\r", ch);
	return;
    }
 
    sprintf( arg, "Mobile #%-6ld [%s]\n\r",
	victim->pIndexData->vnum, victim->short_descr );
    SEND( arg, ch );
 
    sprintf( arg, "Delay   %-6d [%s]\n\r",
	victim->mprog_delay,
	victim->mprog_target == NULL 
		? "No target" : victim->mprog_target->name );
    SEND( arg, ch );
 
    if ( !victim->pIndexData->mprog_flags )
    {
	SEND( "[No programs set]\n\r", ch);
	return;
    }
 
    for ( i = 0, mprg = victim->pIndexData->mprogs; mprg != NULL;
	 mprg = mprg->next )
 
    {
	sprintf( arg, "[%2d] Trigger [%-8s] Program [%5ld] Phrase [%s]\n\r",
	      ++i,
	      prog_type_to_name( mprg->trig_type ),
	      mprg->vnum,
	      mprg->trig_phrase );
	SEND( arg, ch );
    }
 
    return;
 
}
 
/*
 * Displays the source code of a given MOBprogram
 *
 * Syntax: mpdump [vnum]
 */
void do_mpdump( CHAR_DATA *ch, char *argument )
{
   char buf[ MAX_INPUT_LENGTH ];
   PROG_CODE *mprg;
 
   argument = one_argument( argument, buf );
   if ( ( mprg = get_prog_index( atoi(buf), PRG_MPROG ) ) == NULL )
   {
	SEND( "No such MOBprogram.\n\r", ch );
	return;
   }
   page_to_char( mprg->code, ch );
}
 
/*
 * Prints the argument to all active players in the game
 *
 * Syntax: mob gecho [string]
 */
void do_mpgecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
 
    if ( argument[0] == '\0' )
    {
	bug( "MpGEcho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
 	{
	    if ( IS_IMMORTAL(d->character) )
		SEND( "Mob echo> ", d->character );
	    SEND( argument, d->character );
	    SEND( "\n\r", d->character );
	}
    }
}
 
/*
 * Prints the argument to all players in the same area as the mob
 *
 * Syntax: mob zecho [string]
 */
void do_mpzecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
 
    if ( argument[0] == '\0' )
    {
	bug( "MpZEcho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( ch->in_room == NULL )
	return;
 
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
	&&   d->character->in_room->area == ch->in_room->area )
 	{
	    if ( IS_IMMORTAL(d->character) )
		SEND( "Mob echo> ", d->character );
	    SEND( argument, d->character );
	    SEND( "\n\r", d->character );
	}
    }
}
 
/*
 * Prints the argument to all the rooms aroud the mobile
 *
 * Syntax: mob asound [string]
 */
void do_mpasound( CHAR_DATA *ch, char *argument )
{
 
    ROOM_INDEX_DATA *was_in_room;
    int              door;
 
    if ( argument[0] == '\0' )
	return;
 
    was_in_room = ch->in_room;
    for ( door = 0; door < 6; door++ )
    {
    	EXIT_DATA       *pexit;
 
      	if ( ( pexit = was_in_room->exit[door] ) != NULL
	  &&   pexit->u1.to_room != NULL
	  &&   pexit->u1.to_room != was_in_room )
      	{
	    ch->in_room = pexit->u1.to_room;
	    MOBtrigger  = FALSE;
	    act( argument, ch, NULL, NULL, TO_ROOM );
	    MOBtrigger  = TRUE;
	}
    }
    ch->in_room = was_in_room;
    return;
 
}
 
/*
 * Lets the mobile kill any player or mobile without murder
 *
 * Syntax: mob kill [victim]
 */
void do_mpkill( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	return;
 
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	return;
 
    if ( victim == ch || IS_NPC(victim) || ch->position == POS_FIGHTING )
	return;
 
    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	bug( "MpKill - Charmed mob attacking master from vnum %d.",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}
 
/*
 * Lets the mobile assist another mob or player
 *
 * Syntax: mob assist [character]
 */
void do_mpassist( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	return;
 
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	return;
 
    if ( victim == ch || ch->fighting != NULL || victim->fighting == NULL )
	return;
 
    multi_hit( ch, victim->fighting, TYPE_UNDEFINED );
    return;
}
 
 
/*
 * Lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy 
 * items using all.xxxxx or just plain all of them 
 *
 * Syntax: mob junk [item]
 */
 
void do_mpjunk( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0')
	return;
 
    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
    	if ( ( obj = get_obj_wear( ch, arg, TRUE ) ) != NULL )
      	{
      	    unequip_char( ch, obj );
	    extract_obj( obj );
    	    return;
      	}
      	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	    return; 
	extract_obj( obj );
    }
    else
      	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
      	{
            obj_next = obj->next_content;
	    if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            {
          	if ( obj->wear_loc != WEAR_NONE)
	    	unequip_char( ch, obj );
          	extract_obj( obj );
            } 
      	}
 
    return;
 
}
 
/*
 * Prints the message to everyone in the room other than the mob and victim
 *
 * Syntax: mob echoaround [victim] [string]
 */
 
void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	return;
 
    if ( ( victim=get_char_room( ch, NULL, arg ) ) == NULL )
	return;
 
    act( argument, ch, NULL, victim, TO_NOTVICT );
}
 
/*
 * Prints the message to only the victim
 *
 * Syntax: mob echoat [victim] [string]
 */
void do_mpechoat( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
	return;
 
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	return;
 
    act( argument, ch, NULL, victim, TO_VICT );
}
 
/*
 * Prints the message to the room at large
 *
 * Syntax: mpecho [string]
 */
void do_mpecho( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
	return;
    act( argument, ch, NULL, NULL, TO_ROOM );
}
 
/*
 * Lets the mobile load another mobile.
 *
 * Syntax: mob mload [vnum]
 */
void do_mpmload( CHAR_DATA *ch, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;
    int vnum;
 
    argument = one_argument( argument, arg );
 
    if ( ch->in_room == NULL || arg[0] == '\0' || !is_number(arg) )
	return;
 
    vnum = atoi(arg);
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
	sprintf( arg, "Mpmload: bad mob index (%d) from mob %ld",
	    vnum, IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	bug( arg, 0 );
	return;
    }
    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    return;
}
 
/*
 * Lets the mobile load an object
 *
 * Syntax: mob oload [vnum] [level] {R}
 */
void do_mpoload( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;
    bool            fToroom = FALSE, fWear = FALSE;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
               one_argument( argument, arg3 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        bug( "Mpoload - Bad syntax from vnum %d.",
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    bug( "Mpoload - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	    return;
        }
	level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
	{
	    bug( "Mpoload - Bad level from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	    return;
	}
    }
 
    /*
     * Added 3rd argument
     * omitted - load to mobile's inventory
     * 'R'     - load to room
     * 'W'     - load to mobile and force wear
     */
    if ( arg3[0] == 'R' || arg3[0] == 'r' )
	fToroom = TRUE;
    else if ( arg3[0] == 'W' || arg3[0] == 'w' )
	fWear = TRUE;
 
    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	bug( "Mpoload - Bad vnum arg from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    obj = create_object( pObjIndex, level );
    if ( (fWear || !fToroom) && CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_char( obj, ch );
	if ( fWear )
	    wear_obj( ch, obj, TRUE );
    }
    else
    {
	obj_to_room( obj, ch->in_room );
    }
 
    return;
}
 
/*
 * Lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room. The mobile cannot
 * purge itself for safety reasons.
 *
 * syntax mob purge {target}
 */
void do_mppurge( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;
 
	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC( victim ) && victim != ch 
	    &&   !IS_SET(victim->act, ACT_NOPURGE) )
		extract_char( victim, TRUE );
	}
 
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( !IS_SET(obj->extra_flags, ITEM_NOPURGE) )
		extract_obj( obj );
	}
 
	return;
    }
 
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	if ( ( obj = get_obj_here( ch, NULL, arg ) ) )
	{
	    extract_obj( obj );
	}
	else
	{
	    bug( "Mppurge - Bad argument from vnum %d.",
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	}
	return;
    }
 
    if ( !IS_NPC( victim ) )
    {
	bug( "Mppurge - Purging a PC from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    extract_char( victim, TRUE );
    return;
}
 
 
/*
 * Lets the mobile goto any location it wishes that is not private.
 *
 * Syntax: mob goto [location]
 */
void do_mpgoto( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "Mpgoto - No argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpgoto - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );
 
    char_from_room( ch );
    char_to_room( ch, location );
 
    return;
}
 
/* 
 * Lets the mobile do a command at another location.
 *
 * Syntax: mob at [location] [commands]
 */
void do_mpat( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA       *wch;
    OBJ_DATA 	    *on;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpat - Bad argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	bug( "Mpat - No such location from vnum %d.",
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );
 
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
 
/*
 * Lets the mobile transfer people.  The 'all' argument transfers
 *  everyone in the current room to the specified location
 *
 * Syntax: mob transfer [target|'all'] [location]
 */
void do_mptransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	bug( "Mptransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( !str_cmp( arg1, "all" ) )
    {
	CHAR_DATA *victim_next;
 
	for ( victim = ch->in_room->people; victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( !IS_NPC(victim) )
	    {
		sprintf( buf, "%s %s", victim->name, arg2 );
		do_mptransfer( ch, buf );
	    }
	}
	return;
    }
 
    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    bug( "Mptransfer - No such location from vnum %d.",
	        IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	    return;
	}
 
	if ( room_is_private( location ) )
	    return;
    }
 
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	return;
 
    if ( victim->in_room == NULL )
	return;
 
    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    char_from_room( victim );
    char_to_room( victim, location );
    do_look( victim, "auto" );
 
    return;
}
 
/*
 * Lets the mobile transfer all chars in same group as the victim.
 *
 * Syntax: mob gtransfer [victim] [location]
 */
void do_mpgtransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    CHAR_DATA       *who, *victim, *victim_next;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	bug( "Mpgtransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( (who = get_char_room( ch, NULL, arg1 )) == NULL )
	return;
 
    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
    	victim_next = victim->next_in_room;
    	if( is_same_group( who,victim ) )
    	{
	    sprintf( buf, "%s %s", victim->name, arg2 );
	    do_mptransfer( ch, buf );
    	}
    }
    return;
}
 
/*
 * Lets the mobile force someone to do something. Must be mortal level
 * and the all argument only affects those in the room with the mobile.
 *
 * Syntax: mob force [victim] [commands]
 */
void do_mpforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Mpforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;
 
	    if ( vch->in_room == ch->in_room
		&& get_trust( vch ) < get_trust( ch ) 
		&& can_see( ch, vch ) )
	    {
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;
 
	if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	    return;
 
	if ( victim == ch )
	    return;
 
	interpret( victim, argument );
    }
 
    return;
}
 
/*
 * Lets the mobile force a group something. Must be mortal level.
 *
 * Syntax: mob gforce [victim] [commands]
 */
void do_mpgforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch, *vch_next;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "MpGforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	return;
 
    if ( victim == ch )
	return;
 
    for ( vch = victim->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
 
	if ( is_same_group(victim,vch) )
        {
	    interpret( vch, argument );
	}
    }
    return;
}
 
/*
 * Forces all mobiles of certain vnum to do something (except ch)
 *
 * Syntax: mob vforce [vnum] [commands]
 */
void do_mpvforce( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim, *victim_next;
    char arg[ MAX_INPUT_LENGTH ];
    int vnum;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "MpVforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( !is_number( arg ) )
    {
	bug( "MpVforce - Non-number argument vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    vnum = atoi( arg );
 
    for ( victim = char_list; victim; victim = victim_next )
    {
	victim_next = victim->next;
	if ( IS_NPC(victim) && victim->pIndexData->vnum == vnum
	&&   ch != victim && victim->fighting == NULL )
	    interpret( victim, argument );
    }
    return;
}
 
 
/*
 * Lets the mobile cast spells --
 * Beware: this does only crude checking on the target validity
 * and does not account for mana etc., so you should do all the
 * necessary checking in your mob program before issuing this cmd!
 *
 * Syntax: mob cast [spell] {target}
 */
 
void do_mpcast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    void *victim = NULL;
    char spell[ MAX_INPUT_LENGTH ],
	 target[ MAX_INPUT_LENGTH ];
    int sn;
 
    argument = one_argument( argument, spell );
               one_argument( argument, target );
 
    if ( spell[0] == '\0' )
    {
	bug( "MpCast - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
 
    if ( ( sn = skill_lookup( spell ) ) < 0 )
    {
	bug( "MpCast - No such spell from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    vch = get_char_room( ch, NULL, target );
    obj = get_obj_here( ch, NULL, target );
    switch ( skill_table[sn].target )
    {
	default: return;
	case TAR_IGNORE: 
	    break;
	case TAR_CHAR_OFFENSIVE: 
	    if ( vch == NULL || vch == ch )
		return;
	    victim = ( void * ) vch;
	    break;
	case TAR_CHAR_DEFENSIVE:
	    victim = vch == NULL ? ( void *) ch : (void *) vch; break;
	case TAR_CHAR_SELF:
	    victim = ( void *) ch; break;
	case TAR_OBJ_CHAR_DEF:
	case TAR_OBJ_CHAR_OFF:
	case TAR_OBJ_INV:
	    if ( obj == NULL )
		return;
	    victim = ( void * ) obj;
    }
    (*skill_table[sn].spell_fun)( sn, ch->level, ch, victim,
	skill_table[sn].target );
    return;
}
 
/*
 * Lets mob cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 *
 * Syntax: mob damage [victim] [min] [max] {kill}
 */
void do_mpdamage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
	 min[ MAX_INPUT_LENGTH ],
	 max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;
 
    argument = one_argument( argument, target );
    argument = one_argument( argument, min );
    argument = one_argument( argument, max );
 
    if ( target[0] == '\0' )
    {
	bug( "MpDamage - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if( !str_cmp( target, "all" ) )
	fAll = TRUE;
    else if( ( victim = get_char_room( ch, NULL, target ) ) == NULL )
	return;
 
    if ( is_number( min ) )
	low = atoi( min );
    else
    {
	bug( "MpDamage - Bad damage min vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( is_number( max ) )
	high = atoi( max );
    else
    {
	bug( "MpDamage - Bad damage max vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    argument = one_argument( argument, target );
 
    /*
     * If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.
     */
 
    if ( target[0] != '\0' )
	fKill = TRUE;
    if ( fAll )
    {
	for( victim = ch->in_room->people; victim; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( victim != ch )
    		damage( victim, victim, 
		    fKill ? 
		    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
	        TYPE_UNDEFINED, DAM_NONE, FALSE );
	}
    }
    else
    	damage( victim, victim, 
	    fKill ? 
	    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
        TYPE_UNDEFINED, DAM_NONE, FALSE );
    return;
}
 
/*
 * Lets the mobile to remember a target. The target can be referred to
 * with $q and $Q codes in MOBprograms. See also "mob forget".
 *
 * Syntax: mob remember [victim]
 */
void do_mpremember( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
	ch->mprog_target = get_char_world( ch, arg );
    else
	bug( "MpRemember: missing argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
}
 
/*
 * Reverse of "mob remember".
 *
 * Syntax: mob forget
 */
void do_mpforget( CHAR_DATA *ch, char *argument )
{
    ch->mprog_target = NULL;
}
 
/*
 * Sets a delay for MOBprogram execution. When the delay time expires,
 * the mobile is checked for a MObprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_MOBILE
 *
 * Syntax: mob delay [pulses]
 */
void do_mpdelay( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
    if ( !is_number( arg ) )
    {
	bug( "MpDelay: invalid arg from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    ch->mprog_delay = atoi( arg );
}
 
/*
 * Reverse of "mob delay", deactivates the timer.
 *
 * Syntax: mob cancel
 */
void do_mpcancel( CHAR_DATA *ch, char *argument )
{
   ch->mprog_delay = -1;
}
/*
 * Lets the mobile to call another MOBprogram withing a MOBprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the same room with the
 * mobile.
 *
 * Syntax: mob call [vnum] [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
void do_mpcall( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *vch;
    OBJ_DATA *obj1, *obj2;
    PROG_CODE *prg;
	SLEEP_DATA *test;
    extern void program_flow( long, char *, CHAR_DATA *, OBJ_DATA *, ROOM_INDEX_DATA *, CHAR_DATA *, const void *, const void *, int );
	
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "MpCall: missing arguments from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( ( prg = get_prog_index( atoi(arg), PRG_MPROG ) ) == NULL )
    {
	bug( "MpCall: invalid prog from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        vch = get_char_room( ch, NULL, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj1 = get_obj_here( ch, NULL, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj2 = get_obj_here( ch, NULL, arg );
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
    program_flow( prg->vnum, prg->code, ch, NULL, NULL, vch, (void *)obj1, (void *)obj2, 1 );
}
 
/*
 * Forces the mobile to flee.
 *
 * Syntax: mob flee
 *
 */
void do_mpflee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    EXIT_DATA *pexit;
    int door, attempt;
 
    if ( ch->fighting != NULL )
	return;
 
    if ( (was_in = ch->in_room) == NULL )
	return;
 
    for ( attempt = 0; attempt < 6; attempt++ )
    {
        door = number_door( );
        if ( ( pexit = was_in->exit[door] ) == 0
        ||   pexit->u1.to_room == NULL
        ||   IS_SET(pexit->exit_info, EX_CLOSED)
        || ( IS_NPC(ch)
        &&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
            continue;
 
        move_char( ch, door, FALSE );
        if ( ch->in_room != was_in )
	    return;
    }
}
 
/*
 * Lets the mobile to transfer an object. The object must be in the same
 * room with the mobile.
 *
 * Syntax: mob otransfer [item name] [location]
 */
void do_mpotransfer( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "MpOTransfer - Missing argument from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    argument = one_argument( argument, buf );
    if ( ( location = find_location( ch, buf ) ) == NULL )
    {
	bug( "MpOTransfer - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    if ( (obj = get_obj_here( ch, NULL, arg )) == NULL )
	return;
    if ( obj->carried_by == NULL )
	obj_from_room( obj );
    else
    {
	if ( obj->wear_loc != WEAR_NONE )
	    unequip_char( ch, obj );
	obj_from_char( obj );
    }
    obj_to_room( obj, location );
}
 
/*
 * Lets the mobile to strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: mob remove [victim] [object vnum|'all']
 */
void do_mpremove( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    sh_int vnum = 0;
    bool fAll = FALSE;
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	return;
 
    argument = one_argument( argument, arg );
    if ( !str_cmp( arg, "all" ) )
	fAll = TRUE;
    else if ( !is_number( arg ) )
    {
	bug ( "MpRemove: Invalid object from vnum %d.", 
		IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	return;
    }
    else
	vnum = atoi( arg );
 
    for ( obj = victim->carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( fAll || obj->pIndexData->vnum == vnum )
	{
	     unequip_char( victim, obj );
	     obj_from_char( obj );
	     extract_obj( obj );
	}
    }
}
 
/*
 * OBJprog section
 */
void do_obj( OBJ_DATA *obj, char *argument )
{
    /*
     * Security check!
     */
    if ( obj->level < MAX_LEVEL )
	return;
    obj_interpret( obj, argument );
}
/*
 * Obj command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret()
 */
void obj_interpret( OBJ_DATA *obj, char *argument )
{
    char buf[MAX_STRING_LENGTH], command[MAX_INPUT_LENGTH];
    int cmd;
 
    argument = one_argument( argument, command );
 
    /*
     * Look for command in command table.
     */
    for ( cmd = 0; obj_cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == obj_cmd_table[cmd].name[0]
	&&   !str_prefix( command, obj_cmd_table[cmd].name ) )
	{
	    (*obj_cmd_table[cmd].obj_fun) ( obj, argument );
	    tail_chain( );
	    return;
	}
    }
    sprintf( buf, "Obj_interpret: invalid cmd from obj %ld: '%s'",
	obj->pIndexData->vnum, command );
    bug( buf, 0 );
}
 
/* 
 * Displays OBJprogram triggers of an object
 *
 * Syntax: opstat [name]
 */
void do_opstat( CHAR_DATA *ch, char *argument )
{
    char        arg[ MAX_STRING_LENGTH  ];
    PROG_LIST  *oprg;
    OBJ_DATA   *obj;
    int i;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	SEND( "Opstat what?\n\r", ch );
	return;
    }
 
    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
	SEND( "No such object.\n\r", ch );
	return;
    }
 
    sprintf( arg, "Object #%-6ld [%s]\n\r",
	obj->pIndexData->vnum, obj->short_descr );
    SEND( arg, ch );
 
    sprintf( arg, "Delay   %-6d [%s]\n\r",
	obj->oprog_delay,
	obj->oprog_target == NULL 
		? "No target" : obj->oprog_target->name );
    SEND( arg, ch );
 
    if ( !obj->pIndexData->oprog_flags )
    {
	SEND( "[No programs set]\n\r", ch);
	return;
    }
 
    for ( i = 0, oprg = obj->pIndexData->oprogs; oprg != NULL;
	 oprg = oprg->next )
 
    {
	sprintf( arg, "[%2d] Trigger [%-8s] Program [%4ld] Phrase [%s]\n\r",
	      ++i,
	      prog_type_to_name( oprg->trig_type ),
	      oprg->vnum,
	      oprg->trig_phrase );
	SEND( arg, ch );
    }
 
    return;
 
}
 
/*
 * Displays the source code of a given OBJprogram
 *
 * Syntax: opdump [vnum]
 */
void do_opdump( CHAR_DATA *ch, char *argument )
{
   char buf[ MAX_INPUT_LENGTH ];
   PROG_CODE *oprg;
 
   argument = one_argument( argument, buf );
   if ( ( oprg = get_prog_index( atoi(buf), PRG_OPROG ) ) == NULL )
   {
	SEND( "No such OBJprogram.\n\r", ch );
	return;
   }
   page_to_char( oprg->code, ch );
}
 
/*
 * Prints the argument to all active players in the game
 *
 * Syntax: obj gecho [string]
 */
void do_opgecho( OBJ_DATA *obj, char *argument )
{
    DESCRIPTOR_DATA *d;
 
    if ( argument[0] == '\0' )
    {
	bug( "OpGEcho: missing argument from vnum %d",
	    obj->pIndexData->vnum );
	return;
    }
 
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
 	{
	    if ( IS_IMMORTAL(d->character) )
		SEND( "Obj echo> ", d->character );
	    SEND( argument, d->character );
	    SEND( "\n\r", d->character );
	}
    }
}
 
/*
 * Prints the argument to all players in the same area as the mob
 *
 * Syntax: obj zecho [string]
 */
void do_opzecho( OBJ_DATA *obj, char *argument )
{
    DESCRIPTOR_DATA *d;
 
    if ( argument[0] == '\0' )
    {
	bug( "OpZEcho: missing argument from vnum %d",
	    obj->pIndexData->vnum );
	return;
    }
 
    if ( obj->in_room == NULL && (obj->carried_by == NULL || obj->carried_by->in_room == NULL) )
	return;
 
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
	&&   ( (obj->in_room && d->character->in_room->area == obj->in_room->area)
	|| (obj->carried_by && d->character->in_room->area == obj->carried_by->in_room->area) ) )
 	{
	    if ( IS_IMMORTAL(d->character) )
		SEND( "Obj echo> ", d->character );
	    SEND( argument, d->character );
	    SEND( "\n\r", d->character );
	}
    }
}
 
/*
 * Prints the message to everyone in the room other than the mob and victim
 *
 * Syntax: obj echoaround [victim] [string]
 */
 
void do_opechoaround( OBJ_DATA *obj, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	return;
 
    if ( ( victim=get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg ) ) == NULL )
	return;
 
    if ( obj->in_room && obj->in_room->people )
	vch = obj->in_room->people;
    else if ( obj->carried_by && obj->carried_by->in_room )
	vch = obj->carried_by->in_room->people;
    else
	vch = NULL;
 
    for ( ; vch; vch = vch->next_in_room )
    {
	if ( vch == victim )
	    continue;
	SEND( argument, vch );
    }
}
 
/*
 * Prints the message to only the victim
 *
 * Syntax: obj echoat [victim] [string]
 */
void do_opechoat( OBJ_DATA *obj, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
	return;
 
    if ( ( victim = get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg ) ) == NULL )
	return;
 
    act( argument, obj->carried_by?obj->carried_by:obj->in_room->people, obj, victim, TO_VICT );
}
 
/*
 * Prints the message to the room at large
 *
 * Syntax: obj echo [string]
 */
void do_opecho( OBJ_DATA *obj, char *argument )
{
    if ( argument[0] == '\0' )
	return;
 
    if ( !obj->carried_by && !obj->in_room->people )
	return;
 
    act( argument, obj->carried_by?obj->carried_by:obj->in_room->people, NULL, NULL, TO_ROOM );
    act( argument, obj->carried_by?obj->carried_by:obj->in_room->people, NULL, NULL, TO_CHAR );
}
 
/*
 * Lets the object load a mobile.
 *
 * Syntax: obj mload [vnum]
 */
void do_opmload( OBJ_DATA *obj, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;
    int vnum;
 
    argument = one_argument( argument, arg );
 
    if ( (obj->in_room == NULL
      && (obj->carried_by == NULL || obj->carried_by->in_room == NULL))
      || arg[0] == '\0' || !is_number(arg) )
	return;
 
    vnum = atoi(arg);
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
	sprintf( arg, "Opmload: bad mob index (%d) from obj %ld",
	    vnum, obj->pIndexData->vnum );
	bug( arg, 0 );
	return;
    }
    victim = create_mobile( pMobIndex );
    char_to_room( victim, obj->in_room?obj->in_room:obj->carried_by->in_room );
    return;
}
 
/*
 * Lets the object load another object
 *
 * Syntax: obj oload [vnum] [level]
 */
void do_opoload( OBJ_DATA *obj, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *nobj;
    int             level;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        bug( "Opoload - Bad syntax from vnum %d.",
	    obj->pIndexData->vnum );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = obj->level;
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    bug( "Opoload - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	    return;
        }
	level = atoi( arg2 );
	if ( level < 0 || level > obj->level )
	{
	    bug( "Opoload - Bad level from vnum %d.", 
		obj->pIndexData->vnum );
	    return;
	}
    }
 
    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	bug( "Opoload - Bad vnum arg from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    nobj = create_object( pObjIndex, level );
    obj_to_room( nobj, obj->in_room?obj->in_room:obj->carried_by->in_room );
 
    return;
}
 
/*
 * Lets the object purge all other objects and npcs in the room,
 * or purge a specified object or mob in the room. The object cannot
 * purge itself for safety reasons.
 *
 * syntax obj purge {target}
 */
void do_oppurge( OBJ_DATA *obj, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *vobj;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;
 
	if ( obj->in_room && obj->in_room->people )
	    victim = obj->in_room->people;
	else if ( obj->carried_by && obj->carried_by->in_room )
	    victim = obj->carried_by->in_room->people;
	else
	    victim = NULL;
 
	for ( ; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC( victim )
	    &&   !IS_SET(victim->act, ACT_NOPURGE) )
		extract_char( victim, TRUE );
	}
 
	if ( obj->in_room )
	    vobj = obj->in_room->contents;
	else
	    vobj = obj->carried_by->in_room->contents;
 
	for ( ; vobj != NULL; vobj = obj_next )
	{
	    obj_next = vobj->next_content;
	    if ( !IS_SET(vobj->extra_flags, ITEM_NOPURGE) && vobj != obj )
		extract_obj( vobj );
	}
 
	return;
    }
 
    if ( ( victim = get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg ) ) == NULL )
    {
	if ( ( vobj = get_obj_here( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg ) ) )
	{
	    extract_obj( vobj );
	}
	else if ( obj->carried_by && (vobj = get_obj_carry( obj->carried_by, arg, NULL )) != NULL)
	{
	    extract_obj( vobj );
	}
	else if ( obj->carried_by && (vobj = get_obj_wear( obj->carried_by, arg, FALSE )) != NULL )
	{
	    unequip_char( vobj->carried_by, vobj );
	    extract_obj( vobj );
	}
	else
	{
	    bug( "Oppurge - Bad argument from vnum %d.",
		obj->pIndexData->vnum );
	}
	return;
    }
 
    if ( !IS_NPC( victim ) )
    {
	bug( "Oppurge - Purging a PC from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    extract_char( victim, TRUE );
    return;
}
 
 
/*
 * Lets the object goto any location it wishes that is not private.
 *
 * Syntax: obj goto [location]
 */
void do_opgoto( OBJ_DATA *obj, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;
    OBJ_DATA *dobj;
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "Opgoto - No argument from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( is_number(arg) )
	location = get_room_index( atoi( arg ) );
    else if ( (victim = get_char_world( NULL, arg )) != NULL )
	location = victim->in_room;
    else if ( ( dobj = get_obj_world( NULL, arg )) != NULL )
	location = dobj->in_room;
    else
    {
	bug( "Opgoto - No such location from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    obj_to_room( obj, location );
 
    return;
}
 
/*
 * Lets the object transfer people.  The 'all' argument transfers
 *  everyone in the current room to the specified location
 *
 * Syntax: obj transfer [target|'all'] [location]
 */
void do_optransfer( OBJ_DATA *obj, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;
    OBJ_DATA	    *dobj;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	bug( "Optransfer - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( !str_cmp( arg1, "all" ) )
    {
	CHAR_DATA *victim_next;
 
	if ( obj->in_room && obj->in_room->people )
	    victim = obj->in_room->people;
	else if ( obj->carried_by )
	    victim = obj->carried_by->in_room->people;
	else
	    victim = NULL;
	for ( ; victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( !IS_NPC(victim) )
	    {
		sprintf( buf, "%s %s", victim->name, arg2 );
		do_optransfer( obj, buf );
	    }
	}
	return;
    }
 
    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = obj->in_room?obj->in_room:obj->carried_by->in_room;
    }
    else
    {
	if ( is_number(arg2))
	    location = get_room_index( atoi(arg2) );
	else if ( (victim = get_char_world( NULL, arg2 )) != NULL )
	    location = victim->in_room;
	else if ( ( dobj = get_obj_world( NULL, arg2 )) != NULL )
	    location = dobj->in_room;
	else
	{
	    bug( "Optransfer - No such location from vnum %d.",
	        obj->pIndexData->vnum );
	    return;
	}
 
	if ( room_is_private( location ) )
	    return;
    }
 
    if ( ( victim = get_char_world( NULL, arg1 ) ) == NULL )
	return;
 
    if ( victim->in_room == NULL )
	return;
 
    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    char_from_room( victim );
    char_to_room( victim, location );
    do_look( victim, "auto" );
 
    return;
}
 
/*
 * Lets the object transfer all chars in same group as the victim.
 *
 * Syntax: obj gtransfer [victim] [location]
 */
void do_opgtransfer( OBJ_DATA *obj, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    CHAR_DATA       *who, *victim, *victim_next;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	bug( "Opgtransfer - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( (who = get_char_room( NULL, (obj->in_room)?obj->in_room:obj->carried_by->in_room, arg1 )) == NULL )
	return;
 
    if ( obj->in_room && obj->in_room->people )
	victim = obj->in_room->people;
    else if ( obj->carried_by && obj->carried_by->in_room)
	victim = obj->carried_by->in_room->people;
    else
	victim = NULL;
 
    for ( ; victim; victim = victim_next )
    {
    	victim_next = victim->next_in_room;
    	if( is_same_group( who,victim ) )
    	{
	    sprintf( buf, "%s %s", victim->name, arg2 );
	    do_optransfer( obj, buf );
    	}
    }
    return;
}
 
/*
 * Lets the object force someone to do something. Must be mortal level
 * and the all argument only affects those in the room with the object.
 *
 * Syntax: obj force [victim] [commands]
 */
void do_opforce( OBJ_DATA *obj, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Opforce - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( !obj->in_room && !obj->carried_by )
	return;
    if ( obj->in_room && !obj->in_room->people )
	return;
    if ( obj->carried_by && !obj->carried_by->in_room )
	return;
 
    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;
 
	    if ( ((obj->in_room && vch->in_room == obj->in_room)
		|| (obj->carried_by && vch->in_room == obj->carried_by->in_room))
		&& get_trust( vch ) < obj->level )
	    {
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;
 
	if ( ( victim = get_char_room( NULL, (obj->in_room)?obj->in_room:obj->carried_by->in_room, arg ) ) == NULL )
	    return;
 
	interpret( victim, argument );
    }
 
    return;
}
 
/*
 * Lets the object force a group something. Must be mortal level.
 *
 * Syntax: obj gforce [victim] [commands]
 */
void do_opgforce( OBJ_DATA *obj, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch, *vch_next;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "OpGforce - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( ( victim = get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg ) ) == NULL )
	return;
 
    for ( vch = victim->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
 
	if ( is_same_group(victim,vch) )
        {
	    interpret( vch, argument );
	}
    }
    return;
}
 
/*
 * Forces all mobiles of certain vnum to do something
 *
 * Syntax: obj vforce [vnum] [commands]
 */
void do_opvforce( OBJ_DATA *obj, char *argument )
{
    CHAR_DATA *victim, *victim_next;
    char arg[ MAX_INPUT_LENGTH ];
    int vnum;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "OpVforce - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( !is_number( arg ) )
    {
	bug( "OpVforce - Non-number argument vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    vnum = atoi( arg );
 
    for ( victim = char_list; victim; victim = victim_next )
    {
	victim_next = victim->next;
	if ( IS_NPC(victim) && victim->pIndexData->vnum == vnum
	&& victim->fighting == NULL )
	    interpret( victim, argument );
    }
    return;
}
 
/*
 * Lets obj cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 *
 * Syntax: obj damage [victim] [min] [max] {kill}
 */
void do_opdamage( OBJ_DATA *obj, char *argument )
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
	 min[ MAX_INPUT_LENGTH ],
	 max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;
 
    argument = one_argument( argument, target );
    argument = one_argument( argument, min );
    argument = one_argument( argument, max );
 
    if ( target[0] == '\0' )
    {
	bug( "OpDamage - Bad syntax from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    if( !str_cmp( target, "all" ) )
	fAll = TRUE;
    else if( ( victim = get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, target ) ) == NULL )
	return;
 
    if ( is_number( min ) )
	low = atoi( min );
    else
    {
	bug( "OpDamage - Bad damage min vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    if ( is_number( max ) )
	high = atoi( max );
    else
    {
	bug( "OpDamage - Bad damage max vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    argument = one_argument( argument, target );
 
    /*
     * If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.
     */
 
    if ( target[0] != '\0' )
	fKill = TRUE;
    if ( fAll )
    {
	if ( obj->in_room && obj->in_room->people )
	    victim = obj->in_room->people;
	else if ( obj->carried_by )
	    victim = obj->carried_by->in_room->people;
	else
	    victim = NULL;
 
	for( ; victim; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( obj->carried_by && victim != obj->carried_by )
    		damage( victim, victim, 
		    fKill ? 
		    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
	        TYPE_UNDEFINED, DAM_NONE, FALSE );
	}
    }
    else
    	damage( victim, victim, 
	    fKill ? 
	    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
        TYPE_UNDEFINED, DAM_NONE, FALSE );
    return;
}
 
 void do_opchange_align ( OBJ_DATA *obj, char *argument )
 {
	char arg1[MIL];
	char arg2[MIL];
	char buf[MSL];
	int value = 0;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	
	value = atoi(arg2);	
	if ( arg1[0] != '\0' )
		obj->oprog_target = get_char_world( NULL, arg1 );
	else
		bug( "OpChange_Align: missing argument from vnum %d.", obj->pIndexData->vnum );
		
	if (obj->oprog_target && !IS_NPC(obj->oprog_target))
	{
		obj->oprog_target->alignment += value;
		sprintf(buf, "You feel different, somehow. [%d]\r\n", value);
		SEND(buf,obj->oprog_target);
	}
 }
 
/*
 * Lets the object to remember a target. The target can be referred to
 * with $q and $Q codes in OBJprograms. See also "obj forget".
 *
 * Syntax: obj remember [victim]
 */
 void do_opremember( OBJ_DATA *obj, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
	obj->oprog_target = get_char_world( NULL, arg );
    else
	bug( "OpRemember: missing argument from vnum %d.", 
		obj->pIndexData->vnum );
}
 
/*
 * Reverse of "obj remember".
 *
 * Syntax: obj forget
 */
void do_opforget( OBJ_DATA *obj, char *argument )
{
    obj->oprog_target = NULL;
}
 
/*
 * Sets a delay for OBJprogram execution. When the delay time expires,
 * the object is checked for a OBJprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_TICK
 *
 * Syntax: obj delay [pulses]
 */
void do_opdelay( OBJ_DATA *obj, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
    if ( !is_number( arg ) )
    {
	bug( "OpDelay: invalid arg from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    obj->oprog_delay = atoi( arg );
}
 
/*
 * Reverse of "obj delay", deactivates the timer.
 *
 * Syntax: obj cancel
 */
void do_opcancel( OBJ_DATA *obj, char *argument )
{
   obj->oprog_delay = -1;
}
/*
 * Lets the object to call another OBJprogram withing a OBJprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the same room with the
 * mobile.
 *
 * Syntax: obj call [vnum] [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
void do_opcall( OBJ_DATA *obj, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *vch;
    OBJ_DATA *obj1, *obj2;
    PROG_CODE *prg;
    extern void program_flow( long, char *, CHAR_DATA *, OBJ_DATA *, ROOM_INDEX_DATA *, CHAR_DATA *, const void *, const void * , int);
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "OpCall: missing arguments from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    if ( ( prg = get_prog_index( atoi(arg), PRG_OPROG ) ) == NULL )
    {
	bug( "OpCall: invalid prog from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        vch = get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj1 = get_obj_here( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj2 = get_obj_here( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg );
    program_flow( prg->vnum, prg->code, NULL, obj, NULL, vch, (void *)obj1, (void *)obj2, 1 );
}
 
/*
 * Lets the object to transfer an object. The object must be in the same
 * room with the object.
 *
 * Syntax: obj otransfer [item name] [location]
 */
void do_opotransfer( OBJ_DATA *obj, char *argument )
{
    OBJ_DATA *obj1, *dobj;
    ROOM_INDEX_DATA *location;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "OpOTransfer - Missing argument from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    argument = one_argument( argument, buf );
    if ( is_number( buf ) )
	location = get_room_index( atoi(buf) );
    else if ( (victim = get_char_world( NULL, buf )) != NULL )
	location = victim->in_room;
    else if ( ( dobj = get_obj_world( NULL, arg )) != NULL )
	location = dobj->in_room;
    else
    {
	bug( "OpOTransfer - No such location from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
 
    if ( (obj1 = get_obj_here( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg )) == NULL )
	return;
    if ( obj1->carried_by == NULL )
	obj_from_room( obj1 );
    else
    {
	if ( obj1->wear_loc != WEAR_NONE )
	    unequip_char( obj1->carried_by, obj1 );
	obj_from_char( obj1 );
    }
    obj_to_room( obj1, location );
}
 
/*
 * Lets the object to strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: obj remove [victim] [object vnum|'all']
 */
void do_opremove( OBJ_DATA *obj, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj1, *obj_next;
    sh_int vnum = 0;
    bool fAll = FALSE;
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( NULL, obj->in_room?obj->in_room:obj->carried_by->in_room, arg ) ) == NULL )
	return;
 
    argument = one_argument( argument, arg );
    if ( !str_cmp( arg, "all" ) )
	fAll = TRUE;
    else if ( !is_number( arg ) )
    {
	bug ( "OpRemove: Invalid object from vnum %d.", 
		obj->pIndexData->vnum );
	return;
    }
    else
	vnum = atoi( arg );
 
    for ( obj1 = victim->carrying; obj1; obj1 = obj_next )
    {
	obj_next = obj->next_content;
	if ( fAll || obj1->pIndexData->vnum == vnum )
	{
	     unequip_char( victim, obj1 );
	     obj_from_char( obj1 );
	     extract_obj( obj1 );
	}
    }
}
 
/*
 * ROOMprog section
 */
 
void do_room( ROOM_INDEX_DATA *room, char *argument )
{
    room_interpret( room, argument );
}
 
/*
 * Room command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret()
 */
void room_interpret( ROOM_INDEX_DATA *room, char *argument )
{
    char buf[MAX_STRING_LENGTH], command[MAX_INPUT_LENGTH];
    int cmd;
 
    argument = one_argument( argument, command );
 
    /*
     * Look for command in command table.
     */
    for ( cmd = 0; room_cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == room_cmd_table[cmd].name[0]
	&&   !str_prefix( command, room_cmd_table[cmd].name ) )
	{
	    (*room_cmd_table[cmd].room_fun) ( room, argument );
	    tail_chain( );
	    return;
	}
    }
    sprintf( buf, "Room_interpret: invalid cmd from room %ld: '%s'",
	room->vnum, command );
    bug( buf, 0 );
}
 
/* 
 * Displays ROOMprogram triggers of a room
 *
 * Syntax: rpstat [name]
 */
void do_rpstat( CHAR_DATA *ch, char *argument )
{
    char        arg[ MAX_STRING_LENGTH  ];
    PROG_LIST  *rprg;
    ROOM_INDEX_DATA   *room;
    int i;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	room = ch->in_room;
    else if ( !is_number( arg ) )
    {
	SEND( "You must provide a number.\n\r", ch );
	return;
    }
    else if ( ( room = get_room_index( atoi(arg) ) ) == NULL )
    {
	SEND( "No such room.\n\r", ch );
	return;
    }
 
    sprintf( arg, "Room #%-6ld [%s]\n\r",
	room->vnum, room->name );
    SEND( arg, ch );
 
    sprintf( arg, "Delay   %-6d [%s]\n\r",
	room->rprog_delay,
	room->rprog_target == NULL 
		? "No target" : room->rprog_target->name );
    SEND( arg, ch );
 
    if ( !room->rprog_flags )
    {
	SEND( "[No programs set]\n\r", ch);
	return;
    }
 
    for ( i = 0, rprg = room->rprogs; rprg != NULL;
	 rprg = rprg->next )
 
    {
	sprintf( arg, "[%2d] Trigger [%-8s] Program [%4ld] Phrase [%s]\n\r",
	      ++i,
	      prog_type_to_name( rprg->trig_type ),
	      rprg->vnum,
	      rprg->trig_phrase );
	SEND( arg, ch );
    }
 
    return;
 
}
 
/*
 * Displays the source code of a given ROOMprogram
 *
 * Syntax: rpdump [vnum]
 */
void do_rpdump( CHAR_DATA *ch, char *argument )
{
   char buf[ MAX_INPUT_LENGTH ];
   PROG_CODE *rprg;
 
   argument = one_argument( argument, buf );
   if ( ( rprg = get_prog_index( atoi(buf), PRG_RPROG ) ) == NULL )
   {
	SEND( "No such ROOMprogram.\n\r", ch );
	return;
   }
   page_to_char( rprg->code, ch );
}
 
/*
 * Prints the argument to all active players in the game
 *
 * Syntax: room gecho [string]
 */
void do_rpgecho( ROOM_INDEX_DATA *room, char *argument )
{
    DESCRIPTOR_DATA *d;
 
    if ( argument[0] == '\0' )
    {
	bug( "RpGEcho: missing argument from vnum %d",
	    room->vnum );
	return;
    }
 
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
 	{
	    if ( IS_IMMORTAL(d->character) )
		SEND( "Room echo> ", d->character );
	    SEND( argument, d->character );
	    SEND( "\n\r", d->character );
	}
    }
}
 
/*
 * Prints the argument to all players in the same area as the room
 *
 * Syntax: room zecho [string]
 */
void do_rpzecho( ROOM_INDEX_DATA *room, char *argument )
{
    DESCRIPTOR_DATA *d;
 
    if ( argument[0] == '\0' )
    {
	bug( "RpZEcho: missing argument from vnum %d",
	    room->vnum );
	return;
    }
 
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
	&&   d->character->in_room->area == room->area )
 	{
	    if ( IS_IMMORTAL(d->character) )
		SEND( "Room echo> ", d->character );
	    SEND( argument, d->character );
	    SEND( "\n\r", d->character );
	}
    }
}
 
/*
 * Prints the argument to all the rooms aroud the room
 *
 * Syntax: room asound [string]
 */
void do_rpasound( ROOM_INDEX_DATA *room, char *argument )
{
    int              door;
 
    if ( argument[0] == '\0' )
	return;
 
    for ( door = 0; door < 6; door++ )
    {
    	EXIT_DATA       *pexit;
 
      	if ( ( pexit = room->exit[door] ) != NULL
	  &&   pexit->u1.to_room != NULL
	  &&   pexit->u1.to_room != room
	  &&   pexit->u1.to_room->people != NULL )
      	{
	    act( argument, pexit->u1.to_room->people, NULL, NULL, TO_ROOM );
	    act( argument, pexit->u1.to_room->people, NULL, NULL, TO_CHAR );
	}
    }
    return;
 
}
 
/*
 * Prints the message to everyone in the room other than the victim
 *
 * Syntax: room echoaround [victim] [string]
 */
 
void do_rpechoaround( ROOM_INDEX_DATA *room, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
	return;
 
    if ( ( victim=get_char_room( NULL, room, arg ) ) == NULL )
	return;
 
    act( argument, victim, NULL, victim, TO_NOTVICT );
}
 
/*
 * Prints the message to only the victim
 *
 * Syntax: room echoat [victim] [string]
 */
void do_rpechoat( ROOM_INDEX_DATA *room, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
	return;
 
    if ( ( victim = get_char_room( NULL, room, arg ) ) == NULL )
	return;
 
    act( argument, victim, NULL, NULL, TO_CHAR );
}
 
/*
 * Prints the message to the room at large
 *
 * Syntax: rpecho [string]
 */
void do_rpecho( ROOM_INDEX_DATA *room, char *argument )
{
    if ( argument[0] == '\0' )
	return;
 
    if ( !room->people )
	return;
 
    act( argument, room->people, NULL, NULL, TO_ROOM );
    act( argument, room->people, NULL, NULL, TO_CHAR );
}
 
/*
 * Lets the room load a mobile.
 *
 * Syntax: room mload [vnum]
 */
void do_rpmload( ROOM_INDEX_DATA *room, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;
    int vnum;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || !is_number(arg) )
	return;
 
    vnum = atoi(arg);
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
	sprintf( arg, "Rpmload: bad mob index (%d) from room %ld",
	    vnum, room->vnum );
	bug( arg, 0 );
	return;
    }
    victim = create_mobile( pMobIndex );
    char_to_room( victim, room );
    return;
}
 
/*
 * Lets the room load an object
 *
 * Syntax: room oload [vnum] [level]
 */
void do_rpoload( ROOM_INDEX_DATA *room, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
               one_argument( argument, arg3 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) || arg2[0] == '\0' 
      || !is_number( arg2 ) )
    {
        bug( "Rpoload - Bad syntax from vnum %d.",
	    room->vnum );
        return;
    }
 
    level = atoi( arg2 );
    if ( level < 0 || level > LEVEL_IMMORTAL )
    {
	bug( "Rpoload - Bad level from vnum %d.", room->vnum );
	return;
    }
 
    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	bug( "Rpoload - Bad vnum arg from vnum %d.", room->vnum );
	return;
    }
 
    obj = create_object( pObjIndex, level );
    obj_to_room( obj, room );
 
    return;
}
 
/*
 * Lets the room purge all objects npcs in the room,
 * or purge a specified object or mob in the room.
 *
 * syntax room purge {target}
 */
void do_rppurge( ROOM_INDEX_DATA *room, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;
 
	for ( victim = room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC( victim ) 
	    &&   !IS_SET(victim->act, ACT_NOPURGE) )
		extract_char( victim, TRUE );
	}
 
	for ( obj = room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( !IS_SET(obj->extra_flags, ITEM_NOPURGE) )
		extract_obj( obj );
	}
 
	return;
    }
 
    if ( ( victim = get_char_room( NULL, room, arg ) ) == NULL )
    {
	if ( ( obj = get_obj_here( NULL, room, arg ) ) )
	{
	    extract_obj( obj );
	}
	else
	{
	    bug( "Rppurge - Bad argument from vnum %d.", room->vnum );
	}
	return;
    }
 
    if ( !IS_NPC( victim ) )
    {
	bug( "Rppurge - Purging a PC from vnum %d.", room->vnum );
	return;
    }
    extract_char( victim, TRUE );
    return;
}
 
/*
 * Lets the room transfer people.  The 'all' argument transfers
 *  everyone in the room to the specified location
 *
 * Syntax: room transfer [target|'all'] [location]
 */
void do_rptransfer( ROOM_INDEX_DATA *room, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;
    OBJ_DATA	    *tobj;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	bug( "Rptransfer - Bad syntax from vnum %d.", room->vnum );
	return;
    }
 
    if ( !str_cmp( arg1, "all" ) )
    {
	CHAR_DATA *victim_next;
 
	for ( victim = room->people; victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
	    if ( !IS_NPC(victim) )
	    {
		sprintf( buf, "%s %s", victim->name, arg2 );
		do_rptransfer( room, buf );
	    }
	}
	return;
    }
 
    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = room;
    }
    else
    {
	if ( is_number(arg2))
	    location = get_room_index( atoi(arg2) );
	else if ( (victim = get_char_world( NULL, arg2 )) != NULL )
	    location = victim->in_room;
	else if ( ( tobj = get_obj_world( NULL, arg2 )) != NULL )
	    location = tobj->in_room;
	else
	{
	    bug( "Rptransfer - No such location from vnum %d.", room->vnum );
	    return;
	}
 
	if ( room_is_private( location ) )
	    return;
    }
 
    if ( ( victim = get_char_world( NULL, arg1 ) ) == NULL )
	return;
 
    if ( victim->in_room == NULL )
	return;
 
    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    char_from_room( victim );
    char_to_room( victim, location );
    do_look( victim, "auto" );
 
    return;
}
 
/*
 * Lets the room transfer all chars in same group as the victim.
 *
 * Syntax: room gtransfer [victim] [location]
 */
void do_rpgtransfer( ROOM_INDEX_DATA *room, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char	     buf[MAX_STRING_LENGTH];
    CHAR_DATA       *who, *victim, *victim_next;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	bug( "Rpgtransfer - Bad syntax from vnum %d.", room->vnum );
	return;
    }
 
    if ( (who = get_char_room( NULL, room, arg1 )) == NULL )
	return;
 
    for ( victim = room->people; victim; victim = victim_next )
    {
    	victim_next = victim->next_in_room;
    	if( is_same_group( who,victim ) )
    	{
	    sprintf( buf, "%s %s", victim->name, arg2 );
	    do_rptransfer( room, buf );
    	}
    }
    return;
}
 
/*
 * Lets the room force someone to do something. Must be mortal level
 * and the all argument only affects those in the room.
 *
 * Syntax: room force [victim] [commands]
 */
void do_rpforce( ROOM_INDEX_DATA *room, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "Rpforce - Bad syntax from vnum %d.", room->vnum );
	return;
    }
 
    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;
 
	    if ( vch->in_room == room && !IS_IMMORTAL( vch ) )
		interpret( vch, argument );
	}
    }
    else
    {
	CHAR_DATA *victim;
 
	if ( ( victim = get_char_room( NULL, room, arg ) ) == NULL )
	    return;
 
	interpret( victim, argument );
    }
 
    return;
}
 
/*
 * Lets the room force a group something. Must be mortal level.
 *
 * Syntax: room gforce [victim] [commands]
 */
void do_rpgforce( ROOM_INDEX_DATA *room, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch, *vch_next;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "RpGforce - Bad syntax from vnum %d.", room->vnum );
	return;
    }
 
    if ( ( victim = get_char_room( NULL, room, arg ) ) == NULL )
	return;
 
    for ( vch = victim->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
 
	if ( is_same_group(victim,vch) )
	    interpret( vch, argument );
    }
    return;
}
 
/*
 * Forces all mobiles of certain vnum to do something
 *
 * Syntax: room vforce [vnum] [commands]
 */
void do_rpvforce( ROOM_INDEX_DATA *room, char *argument )
{
    CHAR_DATA *victim, *victim_next;
    char arg[ MAX_INPUT_LENGTH ];
    int vnum;
 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	bug( "RpVforce - Bad syntax from vnum %d.", room->vnum );
	return;
    }
 
    if ( !is_number( arg ) )
    {
	bug( "RpVforce - Non-number argument vnum %d.", room->vnum );
	return;
    }
 
    vnum = atoi( arg );
 
    for ( victim = char_list; victim; victim = victim_next )
    {
	victim_next = victim->next;
	if ( IS_NPC(victim) && victim->pIndexData->vnum == vnum
	  && victim->fighting == NULL )
	    interpret( victim, argument );
    }
    return;
}
 
/*
 * Lets room cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 *
 * Syntax: room damage [victim] [min] [max] {kill}
 */
void do_rpdamage( ROOM_INDEX_DATA *room, char *argument )
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
	 min[ MAX_INPUT_LENGTH ],
	 max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;
 
    argument = one_argument( argument, target );
    argument = one_argument( argument, min );
    argument = one_argument( argument, max );
 
    if ( target[0] == '\0' )
    {
	bug( "RpDamage - Bad syntax from vnum %d.", room->vnum );
	return;
    }
    if( !str_cmp( target, "all" ) )
	fAll = TRUE;
    else if( ( victim = get_char_room( NULL, room, target ) ) == NULL )
	return;
 
    if ( is_number( min ) )
	low = atoi( min );
    else
    {
	bug( "RpDamage - Bad damage min vnum %d.", room->vnum );
	return;
    }
    if ( is_number( max ) )
	high = atoi( max );
    else
    {
	bug( "RpDamage - Bad damage max vnum %d.", room->vnum );
	return;
    }
    argument = one_argument( argument, target );
 
    /*
     * If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.
     */
 
    if ( target[0] != '\0' )
	fKill = TRUE;
    if ( fAll )
    {
	for( victim = room->people; victim; victim = victim_next )
	{
	    victim_next = victim->next_in_room;
    	    damage( victim, victim, 
	      fKill ? 
	      number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
	      TYPE_UNDEFINED, DAM_NONE, FALSE );
	}
    }
    else
    	damage( victim, victim, 
	  fKill ? 
	  number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
          TYPE_UNDEFINED, DAM_NONE, FALSE );
    return;
}
 
/*
 * Lets the room remember a target. The target can be referred to
 * with $q and $Q codes in ROOMprograms. See also "room forget".
 *
 * Syntax: room remember [victim]
 */
void do_rpremember( ROOM_INDEX_DATA *room, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    one_argument( argument, arg );
    if ( arg[0] != '\0' )
	room->rprog_target = get_char_world( NULL, arg );
    else
	bug( "RpRemember: missing argument from vnum %d.", room->vnum );
}
 
/*
 * Reverse of "room remember".
 *
 * Syntax: room forget
 */
void do_rpforget( ROOM_INDEX_DATA *room, char *argument )
{
    room->rprog_target = NULL;
}
 
/*
 * Sets a delay for ROOMprogram execution. When the delay time expires,
 * the room is checked for a ROOMprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_AREA
 *
 * Syntax: room delay [pulses]
 */
void do_rpdelay( ROOM_INDEX_DATA *room, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
 
    one_argument( argument, arg );
    if ( !is_number( arg ) )
    {
	bug( "RpDelay: invalid arg from vnum %d.", room->vnum );
	return;
    }
    room->rprog_delay = atoi( arg );
}
 
/*
 * Reverse of "room delay", deactivates the timer.
 *
 * Syntax: room cancel
 */
void do_rpcancel( ROOM_INDEX_DATA *room, char *argument )
{
   room->rprog_delay = -1;
}
/*
 * Lets the room call another ROOMprogram within a ROOMprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the room.
 *
 * Syntax: room call [vnum] [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
void do_rpcall( ROOM_INDEX_DATA *room, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *vch;
    OBJ_DATA *obj1, *obj2;
    PROG_CODE *prg;
    extern void program_flow( long, char *, CHAR_DATA *, OBJ_DATA *, ROOM_INDEX_DATA *, CHAR_DATA *, const void *, const void * , int);
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "RpCall: missing arguments from vnum %d.", room->vnum );
	return;
    }
    if ( ( prg = get_prog_index( atoi(arg), PRG_RPROG ) ) == NULL )
    {
	bug( "RpCall: invalid prog from vnum %d.", room->vnum );
	return;
    }
    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
        vch = get_char_room( NULL, room, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj1 = get_obj_here( NULL, room, arg );
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    	obj2 = get_obj_here( NULL, room, arg );	
    program_flow( prg->vnum, prg->code, NULL, NULL, room, vch, (void *)obj1, (void *)obj2, 1 );
}
 
/*
 * Lets the room transfer an object. The object must be in the room.
 *
 * Syntax: room otransfer [item name] [location]
 */
void do_rpotransfer( ROOM_INDEX_DATA *room, char *argument )
{
    OBJ_DATA *obj, *tobj;
    ROOM_INDEX_DATA *location;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
 
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	bug( "RpOTransfer - Missing argument from vnum %d.", room->vnum );
	return;
    }
    argument = one_argument( argument, buf );
 
    if ( is_number( buf ) )
	location = get_room_index( atoi(buf) );
    else if ( (victim = get_char_world( NULL, buf )) != NULL )
	location = victim->in_room;
    else if ( ( tobj = get_obj_world( NULL, arg )) != NULL )
	location = tobj->in_room;
    else
    {
	bug( "RpOTransfer - No such location from vnum %d.", room->vnum );
	return;
    }
 
    if ( (obj = get_obj_here( NULL, room, arg )) == NULL )
	return;
 
    if ( obj->carried_by == NULL )
	obj_from_room( obj );
    else
    {
	if ( obj->wear_loc != WEAR_NONE )
	    unequip_char( obj->carried_by, obj );
	obj_from_char( obj );
    }
    obj_to_room( obj, location );
}
 
/*
 * Lets the room strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: room remove [victim] [object vnum|'all']
 */
void do_rpremove( ROOM_INDEX_DATA *room, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    sh_int vnum = 0;
    bool fAll = FALSE;
    char arg[ MAX_INPUT_LENGTH ];
 
    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( NULL, room, arg ) ) == NULL )
	return;
 
    argument = one_argument( argument, arg );
    if ( !str_cmp( arg, "all" ) )
	fAll = TRUE;
    else if ( !is_number( arg ) )
    {
	bug ( "RpRemove: Invalid object from vnum %d.", room->vnum );
	return;
    }
    else
	vnum = atoi( arg );
 
    for ( obj = victim->carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( fAll || obj->pIndexData->vnum == vnum )
	{
	     unequip_char( victim, obj );
	     obj_from_char( obj );
	     extract_obj( obj );
	}
    }
}

 
void mpedit( CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;
 
    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);
 
    EDIT_MPCODE(ch, pMcode);
 
    if (pMcode)
    {
	ad = get_vnum_area( pMcode->vnum );
 
	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}
 
	if ( !IS_BUILDER(ch, ad) )
	{
		SEND("MPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }
 
    if (command[0] == '\0')
    {
        mpedit_show(ch, argument);
        return;
    }
 
    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }
 
    for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, mpedit_table[cmd].name) )
	{
		if ((*mpedit_table[cmd].olc_fun) (ch, argument) && pMcode)
			if ((ad = get_vnum_area(pMcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }
 
    interpret(ch, arg);
 
    return;
}
 
void opedit( CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pOcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;
 
    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);
 
    EDIT_OPCODE(ch, pOcode);
 
    if (pOcode)
    {
	ad = get_vnum_area( pOcode->vnum );
 
	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}
 
	if ( !IS_BUILDER(ch, ad) )
	{
		SEND("OPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }
 
    if (command[0] == '\0')
    {
        opedit_show(ch, argument);
        return;
    }
 
    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }
 
    for (cmd = 0; opedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, opedit_table[cmd].name) )
	{
		if ((*opedit_table[cmd].olc_fun) (ch, argument) && pOcode)
			if ((ad = get_vnum_area(pOcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }
 
    interpret(ch, arg);
 
    return;
}
 
void rpedit( CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pRcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;
 
    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);
 
    EDIT_RPCODE(ch, pRcode);
 
    if (pRcode)
    {
	ad = get_vnum_area( pRcode->vnum );
 
	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}
 
	if ( !IS_BUILDER(ch, ad) )
	{
		SEND("RPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }
 
    if (command[0] == '\0')
    {
        rpedit_show(ch, argument);
        return;
    }
 
    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }
 
    for (cmd = 0; rpedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, rpedit_table[cmd].name) )
	{
		if ((*rpedit_table[cmd].olc_fun) (ch, argument) && pRcode)
			if ((ad = get_vnum_area(pRcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }
 
    interpret(ch, arg);
 
    return;
}
 
void do_mpedit(CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];
 
    argument = one_argument(argument, command);
 
    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;
 
	if ( (pMcode = get_prog_index(vnum,PRG_MPROG)) == NULL )
	{
		SEND("MPEdit : That vnum does not exist.\n\r",ch);
		return;
	}
 
	ad = get_vnum_area(vnum);
 
	if ( ad == NULL )
	{
		SEND( "MPEdit : Vnum is not assigned an area.\n\r", ch );
		return;
	}
 
	if ( !IS_BUILDER(ch, ad) )
	{
		SEND("MPEdit : Insufficient security to modify area.\n\r", ch );
		return;
	}
 
	ch->desc->pEdit		= (void *)pMcode;
	ch->desc->editor	= ED_MPCODE;
 
	return;
    }
 
    if ( !str_cmp(command, "create") )
    {
	if (argument[0] == '\0')
	{
		SEND( "Syntax : mpedit create [vnum]\n\r", ch );
		return;
	}
 
	mpedit_create(ch, argument);
	return;
    }
 
    SEND( "Syntax : mpedit [vnum]\n\r", ch );
    SEND( "         mpedit create [vnum]\n\r", ch );
 
    return;
}
 
void do_opedit(CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pOcode;
    char command[MAX_INPUT_LENGTH];
 
    argument = one_argument(argument, command);
 
    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;
 
	if ( (pOcode = get_prog_index(vnum,PRG_OPROG)) == NULL )
	{
		SEND("OPEdit : That vnum does not exist.\n\r",ch);
		return;
	}
 
	ad = get_vnum_area(vnum);
 
	if ( ad == NULL )
	{
		SEND( "OPEdit : Vnum is not assigned an area.\n\r", ch );
		return;
	}
 
	if ( !IS_BUILDER(ch, ad) )
	{
		SEND("OPEdit : Insufficient security to modify area.\n\r", ch );
		return;
	}
 
	ch->desc->pEdit		= (void *)pOcode;
	ch->desc->editor	= ED_OPCODE;
 
	return;
    }
 
    if ( !str_cmp(command, "create") )
    {
	if (argument[0] == '\0')
	{
		SEND( "Syntax : opedit create [vnum]\n\r", ch );
		return;
	}
 
	opedit_create(ch, argument);
	return;
    }
 
    SEND( "Syntax : opedit [vnum]\n\r", ch );
    SEND( "         opedit create [vnum]\n\r", ch );
 
    return;
}
 
void do_rpedit(CHAR_DATA *ch, char *argument)
{
    PROG_CODE *pRcode;
    char command[MAX_INPUT_LENGTH];
 
    argument = one_argument(argument, command);
 
    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;
 
	if ( (pRcode = get_prog_index(vnum,PRG_RPROG)) == NULL )
	{
		SEND("RPEdit : That vnum does not exist.\n\r",ch);
		return;
	}
 
	ad = get_vnum_area(vnum);
 
	if ( ad == NULL )
	{
		SEND( "RPEdit : Vnum is not assigned an area.\n\r", ch );
		return;
	}
 
	if ( !IS_BUILDER(ch, ad) )
	{
		SEND("RPEdit : Insufficient security to modify area.\n\r", ch );
		return;
	}
 
	ch->desc->pEdit		= (void *)pRcode;
	ch->desc->editor	= ED_RPCODE;
 
	return;
    }
 
    if ( !str_cmp(command, "create") )
    {
	if (argument[0] == '\0')
	{
		SEND( "Syntax : rpedit create [vnum]\n\r", ch );
		return;
	}
 
	rpedit_create(ch, argument);
	return;
    }
 
    SEND( "Syntax : rpedit [vnum]\n\r", ch );
    SEND( "         rpedit create [vnum]\n\r", ch );
 
    return;
}
 
MPEDIT (mpedit_create)
{
    PROG_CODE *pMcode;
    int value = atoi(argument);
    AREA_DATA *ad;
 
    if (IS_NULLSTR(argument) || value < 1)
    {
	SEND( "Syntax : mpedit create [vnum]\n\r", ch );
	return FALSE;
    }
 
    ad = get_vnum_area(value);
 
    if ( ad == NULL )
    {
    	SEND( "MPEdit : Vnum is not assigned an area.\n\r", ch );
    	return FALSE;
    }
 
    if ( !IS_BUILDER(ch, ad) )
    {
        SEND("MPEdit : Insufficient security to create MobProgs.\n\r", ch);
        return FALSE;
    }
 
    if ( get_prog_index(value,PRG_MPROG) )
    {
	SEND("MPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }
 
    pMcode			= new_mpcode();
    pMcode->vnum		= value;
    pMcode->next		= mprog_list;
    mprog_list			= pMcode;
    ch->desc->pEdit		= (void *)pMcode;
    ch->desc->editor		= ED_MPCODE;
 
    SEND("MobProgram Code Created.\n\r",ch);
 
    return TRUE;
}
 
OPEDIT (opedit_create)
{
    PROG_CODE *pOcode;
    int value = atoi(argument);
    AREA_DATA *ad;
 
    if (IS_NULLSTR(argument) || value < 1)
    {
	SEND( "Syntax : opedit create [vnum]\n\r", ch );
	return FALSE;
    }
 
    ad = get_vnum_area(value);
 
    if ( ad == NULL )
    {
    	SEND( "OPEdit : Vnum is not assigned an area.\n\r", ch );
    	return FALSE;
    }
 
    if ( !IS_BUILDER(ch, ad) )
    {
        SEND("OPEdit : Insufficient security to create ObjProgs.\n\r", ch);
        return FALSE;
    }
 
    if ( get_prog_index(value,PRG_OPROG) )
    {
	SEND("OPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }
 
    pOcode			= new_opcode();
    pOcode->vnum		= value;
    pOcode->next		= oprog_list;
    oprog_list			= pOcode;
    ch->desc->pEdit		= (void *)pOcode;
    ch->desc->editor		= ED_OPCODE;
 
    SEND("ObjProgram Code Created.\n\r",ch);
 
    return TRUE;
}
 
RPEDIT (rpedit_create)
{
    PROG_CODE *pRcode;
    int value = atoi(argument);
    AREA_DATA *ad;
 
    if (IS_NULLSTR(argument) || value < 1)
    {
	SEND( "Syntax : rpedit create [vnum]\n\r", ch );
	return FALSE;
    }
 
    ad = get_vnum_area(value);
 
    if ( ad == NULL )
    {
    	SEND( "RPEdit : Vnum is not assigned an area.\n\r", ch );
    	return FALSE;
    }
 
    if ( !IS_BUILDER(ch, ad) )
    {
        SEND("RPEdit : Insufficient security to create RoomProgs.\n\r", ch);
        return FALSE;
    }
 
    if ( get_prog_index(value,PRG_RPROG) )
    {
	SEND("RPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }
 
    pRcode			= new_rpcode();
    pRcode->vnum		= value;
    pRcode->next		= rprog_list;
    rprog_list			= pRcode;
    ch->desc->pEdit		= (void *)pRcode;
    ch->desc->editor		= ED_RPCODE;
 
    SEND("RoomProgram Code Created.\n\r",ch);
 
    return TRUE;
}
 
MPEDIT(mpedit_show)
{
    PROG_CODE *pMcode;
    char buf[MAX_STRING_LENGTH];
 
    EDIT_MPCODE(ch,pMcode);
 
    sprintf(buf,
           "Vnum:       [%ld]\n\r"
           "Code:\n\r%s\n\r",
           pMcode->vnum, pMcode->code);
    SEND(buf, ch);
 
    return FALSE;
}
 
OPEDIT(opedit_show)
{
    PROG_CODE *pOcode;
    char buf[MAX_STRING_LENGTH];
 
    EDIT_OPCODE(ch,pOcode);
 
    sprintf(buf,
           "Vnum:       [%ld]\n\r"
           "Code:\n\r%s\n\r",
           pOcode->vnum, pOcode->code);
    SEND(buf, ch);
 
    return FALSE;
}
 
RPEDIT(rpedit_show)
{
    PROG_CODE *pRcode;
    char buf[MAX_STRING_LENGTH];
 
    EDIT_RPCODE(ch,pRcode);
 
    sprintf(buf,
           "Vnum:       [%ld]\n\r"
           "Code:\n\r%s\n\r",
           pRcode->vnum, pRcode->code);
    SEND(buf, ch);
 
    return FALSE;
}
 
MPEDIT(mpedit_code)
{
    PROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);
 
    if (argument[0] =='\0')
    {
       string_append(ch, &pMcode->code);
       return TRUE;
    }
 
    SEND("Syntax: code\n\r",ch);
    return FALSE;
}
 
OPEDIT(opedit_code)
{
    PROG_CODE *pOcode;
    EDIT_OPCODE(ch, pOcode);
 
    if (argument[0] =='\0')
    {
       string_append(ch, &pOcode->code);
       return TRUE;
    }
 
    SEND("Syntax: code\n\r",ch);
    return FALSE;
}
 
RPEDIT(rpedit_code)
{
    PROG_CODE *pRcode;
    EDIT_RPCODE(ch, pRcode);
 
    if (argument[0] =='\0')
    {
       string_append(ch, &pRcode->code);
       return TRUE;
    }
 
    SEND("Syntax: code\n\r",ch);
    return FALSE;
}
 
MPEDIT( mpedit_list )
{
    int count = 1;
    PROG_CODE *mprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;
	
    buffer = new_buf();
 
	
    for (mprg = mprog_list; mprg !=NULL; mprg = mprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, mprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(mprg->vnum);
 
		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';
 
		sprintf(buf, "[%3d] (%c) %5ld\n\r", count, blah, mprg->vnum );
		add_buf(buffer, buf);
 
		count++;
	}
 
    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "No existing MobPrograms.\n\r" );
    	else
    		add_buf( buffer, "No existing MobPrograms in this area.\n\r" );
    }
 
    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
 
    return FALSE;
}
 
OPEDIT( opedit_list )
{
    int count = 1;
    PROG_CODE *oprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;
 
    buffer = new_buf();
 
    for (oprg = oprog_list; oprg !=NULL; oprg = oprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, oprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(oprg->vnum);
 
		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';
 
		sprintf(buf, "[%3d] (%c) %5ld\n\r", count, blah, oprg->vnum );
		add_buf(buffer, buf);
 
		count++;
	}
 
    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "No existing ObjPrograms.\n\r" );
    	else
    		add_buf( buffer, "No existing ObjPrograms in this area.\n\r" );
    }
 
    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
 
    return FALSE;
}
 
RPEDIT( rpedit_list )
{
    int count = 1;
    PROG_CODE *rprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;
 
    buffer = new_buf();
 
    for (rprg = rprog_list; rprg !=NULL; rprg = rprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, rprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(rprg->vnum);
 
		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';
 
		sprintf(buf, "[%3d] (%c) %5ld\n\r", count, blah, rprg->vnum );
		add_buf(buffer, buf);
 
		count++;
	}
 
    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "No existing RoomPrograms.\n\r" );
    	else
    		add_buf( buffer, "No existing RoomPrograms in this area.\n\r" );
    }
 
    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
 
    return FALSE;
}


 
/*
 * if-check keywords:
 */
const char * fn_keyword[] =
{
    "rand",		/* if rand 30		- if random number < 30 */
    "mobhere",		/* if mobhere fido	- is there a 'fido' here */
    "objhere",		/* if objhere bottle	- is there a 'bottle' here */
			/* if mobhere 1233	- is there mob vnum 1233 here */
			/* if objhere 1233	- is there obj vnum 1233 here */
    "mobexists",	/* if mobexists fido	- is there a fido somewhere */
    "objexists",	/* if objexists sword	- is there a sword somewhere */
 
    "people",		/* if people > 4	- does room contain > 4 people */
    "players",		/* if players > 1	- does room contain > 1 pcs */
    "mobs",		/* if mobs > 2		- does room contain > 2 mobiles */
    "clones",		/* if clones > 3	- are there > 3 mobs of same vnum here */
    "order",		/* if order == 0	- is mob the first in room */
    "hour",		/* if hour > 11		- is the time > 11 o'clock */
 
 
    "ispc",		/* if ispc $n 		- is $n a pc */
    "isnpc",		/* if isnpc $n 		- is $n a mobile */
    "isgood",		/* if isgood $n 	- is $n good */
    "isevil",		/* if isevil $n 	- is $n evil */
    "isneutral",	/* if isneutral $n 	- is $n neutral */
    "isimmort",		/* if isimmort $n	- is $n immortal */
    "ischarm",		/* if ischarm $n	- is $n charmed */
    "isfollow",		/* if isfollow $n	- is $n following someone */
    "isactive",		/* if isactive $n	- is $n's position > SLEEPING */
    "isdelay",		/* if isdelay $i	- does $i have mobprog pending */
    "isvisible",	/* if isvisible $n	- can mob see $n */
    "hastarget",	/* if hastarget $i	- does $i have a valid target */
    "istarget",		/* if istarget $n	- is $n mob's target */
    "exists",		/* if exists $n		- does $n exist somewhere */
 
    "affected",		/* if affected $n blind - is $n affected by blind */
    "act",		/* if act $i sentinel	- is $i flagged sentinel */
    "off",              /* if off $i berserk	- is $i flagged berserk */
    "imm",              /* if imm $i fire	- is $i immune to fire */
    "carries",		/* if carries $n sword	- does $n have a 'sword' */
			/* if carries $n 1233	- does $n have obj vnum 1233 */
    "wears",		/* if wears $n lantern	- is $n wearing a 'lantern' */
			/* if wears $n 1233	- is $n wearing obj vnum 1233 */
    "has",    		/* if has $n weapon	- does $n have obj of type weapon */
    "uses",		/* if uses $n armor	- is $n wearing obj of type armor */
    "name",		/* if name $n puff	- is $n's name 'puff' */
    "pos",		/* if pos $n standing	- is $n standing */
    "clan",		/* if clan $n 'whatever'- does $n belong to clan 'whatever' */
    "race",		/* if race $n dragon	- is $n of 'dragon' race */
    "class",		/* ifch_class $n mage	- is $n'sch_class 'mage' */
    "objtype",		/* if objtype $p scroll	- is $p a scroll */
 
    "vnum",		/* if vnum $i == 1233  	- virtual number check */
    "hpcnt",		/* if hpcnt $i > 30	- hit point percent check */
    "room",		/* if room $i == 1233	- room virtual number */
    "sex",		/* if sex $i == 0	- sex check */
    "level",		/* if level $n < 5	- level check */
    "align",		/* if align $n < -1000	- alignment check */
    "money",		/* if money $n */
    "objval0",		/* if objval0 > 1000 	- object value[] checks 0..4 */
    "objval1",
    "objval2",
    "objval3",
    "objval4",
    "grpsize",		/* if grpsize $n > 6	- group size check */
	"pc_god",		//check god.
	"on_quest",		//are they on this quest? (by vnum)
	"completed_quest",	//have they completed the quest? (by vnum)
	"society_rank",	//check a society rank
 
    "\n"		/* Table terminator */
};
 
const char *fn_evals[] =
{
    "==",
    ">=",
    "<=",
    ">",
    "<",
    "!=",
    "\n"
};
 
/*
 * Return a valid keyword from a keyword table
 */
int keyword_lookup( const char **table, char *keyword )
{
    register int i;
    for( i = 0; table[i][0] != '\n'; i++ )
        if( !str_cmp( table[i], keyword ) )
            return( i );
    return -1;
}
 
/*
 * Perform numeric evaluation.
 * Called by cmd_eval()
 */
int num_eval( int lval, int oper, int rval )
{
    switch( oper )
    {
        case EVAL_EQ:
             return ( lval == rval );
        case EVAL_GE:
             return ( lval >= rval );
        case EVAL_LE:
             return ( lval <= rval );
        case EVAL_NE:
             return ( lval != rval );
        case EVAL_GT:
             return ( lval > rval );
        case EVAL_LT:
             return ( lval < rval );
        default:
             bug( "mob_prog.c: num_eval: invalid operator.", 0 );
             return 0;
    }
}
 
/*
 * ---------------------------------------------------------------------
 * UTILITY FUNCTIONS USED BY CMD_EVAL()
 * ----------------------------------------------------------------------
 */
 
/*
 * Get a random PC in the room (for $r parameter)
 */
CHAR_DATA *get_random_char( CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room )
{
    CHAR_DATA *vch, *victim = NULL;
    int now = 0, highest = 0;
 
    if ( (mob && obj) || (mob && room) || (obj && room) )
    {
	bug( "get_random_char received multiple prog types",0);
	return NULL;
    }
 
    if ( mob )
	vch = mob->in_room->people;
    else if ( obj )
    {
	if ( obj->in_room )
	    vch = obj->in_room->people;
	else
	    vch = obj->carried_by->in_room->people;
    }
    else
	vch = room->people;
 
    for( ; vch; vch = vch->next_in_room )
    {
        if ( mob
	&&   mob != vch 
        &&   !IS_NPC( vch ) 
        &&   can_see( mob, vch )
        &&   ( now = number_percent() ) > highest )
        {
            victim = vch;
            highest = now;
        }
	else if ( (now = number_percent()) > highest )
 	{
	    victim = vch;
	    highest = now;
	}
    }
    return victim;
}
 
/* 
 * How many other players / mobs are there in the room
 * iFlag: 0: all, 1: players, 2: mobiles 3: mobs w/ same vnum 4: same group
 */
int count_people_room( CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, int iFlag )
{
    CHAR_DATA *vch;
    int count;
 
    if ( (mob && obj) || (mob && room) || (obj && room) )
    {
	bug( "count_people_room received multiple prog types",0);
	return 0;
    }
 
    if ( mob )
	vch = mob->in_room->people;
    else if ( obj )
    {
	if ( obj->in_room )
	    vch = obj->in_room->people;
	else
	    vch = obj->carried_by->in_room->people;
    }
    else
	vch = room->people;
 
    for ( count = 0; vch; vch = vch->next_in_room )
    {
	if ( mob )
	{
	    if ( mob != vch 
	    &&   (iFlag == 0
	    || (iFlag == 1 && !IS_NPC( vch )) 
	    || (iFlag == 2 && IS_NPC( vch ))
	    || (iFlag == 3 && IS_NPC( mob ) && IS_NPC( vch ) 
	    && mob->pIndexData->vnum == vch->pIndexData->vnum )
	    || (iFlag == 4 && is_same_group( mob, vch )) )
	    && can_see( mob, vch ) ) 
	  	count++;
	}
	else if ( obj || room )
	{
	    if ( iFlag == 0
	    || (iFlag == 1 && !IS_NPC( vch ))
	    || (iFlag == 2 && IS_NPC( vch )))
		count++;
	}
    }
 
    return ( count );
}
 
/*
 * Get the order of a mob in the room. Useful when several mobs in
 * a room have the same trigger and you want only the first of them
 * to act 
 */
int get_order( CHAR_DATA *ch, OBJ_DATA *obj )
{
    CHAR_DATA *vch;
    OBJ_DATA *vobj;
    int i;
 
    if ( ch && obj )
    {
	bug( "get_order received multiple prog types",0);
	return 0;
    }
 
    if ( ch && !IS_NPC(ch) )
	return 0;
 
    if ( ch )
    {
	vch = ch->in_room->people;
	vobj = NULL;
    }
    else
    {
	vch = NULL;
	if ( obj->in_room )
	    vobj = obj->in_room->contents;
	else if ( obj->carried_by->in_room->contents )
	    vobj = obj->carried_by->in_room->contents;
	else
	    vobj = NULL;
    }
 
    if ( ch )
	for ( i = 0; vch; vch = vch->next_in_room )
	{
	    if ( vch == ch )
		return i;
 
	    if ( IS_NPC(vch) 
	      &&   vch->pIndexData->vnum == ch->pIndexData->vnum )
		i++;
	}
    else
	for ( i = 0; vobj; vobj = vobj->next_content )
	{
	    if ( vobj == obj )
		return i;
 
	    if ( vobj->pIndexData->vnum == obj->pIndexData->vnum )
		i++;
	}
 
    return 0;
}
 
/*
 * Check if ch has a given item or item type
 * vnum: item vnum or -1
 * item_type: item type or -1
 * fWear: TRUE: item must be worn, FALSE: don't care
 */
bool has_item( CHAR_DATA *ch, sh_int vnum, sh_int item_type, bool fWear )
{
    OBJ_DATA *obj;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
	if ( ( vnum < 0 || obj->pIndexData->vnum == vnum )
	&&   ( item_type < 0 || obj->pIndexData->item_type == item_type )
	&&   ( !fWear || obj->wear_loc != WEAR_NONE ) )
	    return TRUE;
    return FALSE;
}
 
/*
 * Check if there's a mob with given vnum in the room
 */
bool get_mob_vnum_room( CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room, sh_int vnum )
{
    CHAR_DATA *mob;
 
    if ( (ch && obj) || (ch && room) || (obj && room) )
    {
	bug( "get_mob_vnum_room received multiple prog types",0);
	return FALSE;
    }
 
    if ( ch )
	mob = ch->in_room->people;
    else if ( obj )
    {
	if ( obj->in_room )
	    mob = obj->in_room->people;
	else
	    mob = obj->carried_by->in_room->people;
    }
    else
	mob = room->people;
 
    for ( ; mob; mob = mob->next_in_room )
	if ( IS_NPC( mob ) && mob->pIndexData->vnum == vnum )
	    return TRUE;
    return FALSE;
}
 
/*
 * Check if there's an object with given vnum in the room
 */
bool get_obj_vnum_room( CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room, sh_int vnum )
{
    OBJ_DATA *vobj;
 
    if ( (ch && obj) || (ch && room) || (obj && room) )
    {
	bug( "get_obj_vnum_room received multiple prog types",0);
	return FALSE;
    }
 
    if ( ch )
	vobj = ch->in_room->contents;
    else if ( obj )
    {
	if ( obj->in_room )
	    vobj = obj->in_room->contents;
	else
	    vobj = obj->carried_by->in_room->contents;
    }
    else
	vobj = room->contents;
 
    for ( ; vobj; vobj = vobj->next_content )
	if ( vobj->pIndexData->vnum == vnum )
	    return TRUE;
    return FALSE;
}
 
/* ---------------------------------------------------------------------
 * CMD_EVAL
 * This monster evaluates an if/or/and statement
 * There are five kinds of statement:
 * 1) keyword and value (no $-code)	    if random 30
 * 2) keyword, comparison and value	    if people > 2
 * 3) keyword and actor		    	    if isnpc $n
 * 4) keyword, actor and value		    if carries $n sword
 * 5) keyword, actor, comparison and value  if level $n >= 10
 *
 *----------------------------------------------------------------------
 */
int cmd_eval_mob( sh_int vnum, char *line, int check,
	CHAR_DATA *mob, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    CHAR_DATA *lval_char = mob;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    OBJ_DATA  *lval_obj = NULL;
 
    char *original, buf[MAX_INPUT_LENGTH], code;
    int lval = 0, oper = 0, rval = -1;
 
    original = line;
    line = one_argument( line, buf );
    if ( buf[0] == '\0' || mob == NULL )
	return FALSE;
 
    /*
     * If this mobile has no target, let's assume our victim is the one
     */
    if ( mob->mprog_target == NULL )
		mob->mprog_target = ch;
 
    switch ( check )
    {
		/*
		 * Case 1: keyword and value
		 */
		case CHK_RAND:
			return( atoi( buf ) < number_percent() );
		case CHK_MOBHERE:
			if ( is_number( buf ) )
			return( get_mob_vnum_room( mob, NULL, NULL, atoi(buf) ) );
			else
			return( (bool) (get_char_room( mob, NULL, buf) != NULL) );
		case CHK_OBJHERE:
			if ( is_number( buf ) )
			return( get_obj_vnum_room( mob, NULL, NULL, atoi(buf) ) );
			else
			return( (bool) (get_obj_here( mob, NULL, buf) != NULL) );
			case CHK_MOBEXISTS:
			return( (bool) (get_char_world( mob, buf) != NULL) );
		case CHK_OBJEXISTS:
			return( (bool) (get_obj_world( mob, buf) != NULL) );
		/*
		 * Case 2 begins here: We sneakily use rval to indicate need
		 * 		       for numeric eval...
		 */
		case CHK_PEOPLE:
			rval = count_people_room( mob, NULL, NULL, 0 ); break;
		case CHK_PLAYERS:
			rval = count_people_room( mob, NULL, NULL, 1 ); break;
		case CHK_MOBS:
			rval = count_people_room( mob, NULL, NULL, 2 ); break;
		case CHK_CLONES:
			rval = count_people_room( mob, NULL, NULL, 3 ); break;
		case CHK_ORDER:
			rval = get_order( mob, NULL ); break;
		case CHK_HOUR:
			rval = time_info.hour; break;
		default:;
		}
	 
		/*
		 * Case 2 continued: evaluate expression
		 */
		if ( rval >= 0 )
		{
			if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
			{
				sprintf( buf, "Mob_prog.c: Cmd_eval_mob: prog %d syntax error(2) '%s'",
				vnum, original );
				bug( buf, 0 );
				return FALSE;
			}
			line = one_argument( line, buf );
			lval = rval;
			rval = atoi( buf );
			return( num_eval( lval, oper, rval ) );
		}
	 
		/*
		 * Case 3,4,5: Grab actors from $* codes
		 */
		if ( buf[0] != '$' || buf[1] == '\0' )
		{
		sprintf( buf, "Mob_prog.c: Cmd_eval_mob: prog %d syntax error(3) '%s'",
			vnum, original );
		bug( buf, 0 );
			return FALSE;
		}
		else
			code = buf[1];
		switch( code )
		{
			case 'i':
				lval_char = mob; break;
			case 'n':
				lval_char = ch; break;
			case 't':
				lval_char = vch; break;
			case 'r':
				lval_char = rch == NULL ? get_random_char( mob, NULL, NULL ) : rch ; break;
			case 'o':
				lval_obj = obj1; break;
			case 'p':
				lval_obj = obj2; break;
		case 'q':
			lval_char = mob->mprog_target; break;
		default:
			sprintf( buf, "Cmd_eval_mob: prog %d syntax error(4) '%s'",
			vnum, original );
			bug( buf, 0 );
			return FALSE;
		}
		/*
		 * From now on, we need an actor, so if none was found, bail out
		 */
		if ( lval_char == NULL && lval_obj == NULL )
			return FALSE;
	 
		/*
		 * Case 3: Keyword, comparison and value
		 */
		switch( check )
		{
		case CHK_ISPC:
				return( lval_char != NULL && !IS_NPC( lval_char ) );
			case CHK_ISNPC:
				return( lval_char != NULL && IS_NPC( lval_char ) );
			case CHK_ISGOOD:
				return( lval_char != NULL && IS_GOOD( lval_char ) );
			case CHK_ISEVIL:
				return( lval_char != NULL && IS_EVIL( lval_char ) );
			case CHK_ISNEUTRAL:
				return( lval_char != NULL && IS_NEUTRAL( lval_char ) );
			case CHK_ISIMMORT:
				return( lval_char != NULL && IS_IMMORTAL( lval_char ) );
			case CHK_ISCHARM: /* A relic from MERC 2.2 MOBprograms */
				return( lval_char != NULL && IS_AFFECTED( lval_char, AFF_CHARM ) );
			case CHK_ISFOLLOW:
				return( lval_char != NULL && lval_char->master != NULL 
			 && lval_char->master->in_room == lval_char->in_room );
		case CHK_ISACTIVE:
			return( lval_char != NULL && lval_char->position > POS_SLEEPING );
		case CHK_ISDELAY:
			return( lval_char != NULL && lval_char->mprog_delay > 0 );
		case CHK_ISVISIBLE:
				switch( code )
				{
					default :
					case 'i':
					case 'n':
					case 't':
					case 'r':
			case 'q':
					return( lval_char != NULL && can_see( mob, lval_char ) );
			case 'o':
			case 'p':
					return( lval_obj != NULL && can_see_obj( mob, lval_obj ) );
			}
		case CHK_HASTARGET:
			return( lval_char != NULL && lval_char->mprog_target != NULL
			&&  lval_char->in_room == lval_char->mprog_target->in_room );
		case CHK_ISTARGET:
			return( lval_char != NULL && mob->mprog_target == lval_char );
		default:;
		 }
	 
		 /* 
		  * Case 4: Keyword, actor and value
		  */
		 line = one_argument( line, buf );
		switch( check )
		{
		case CHK_AFFECTED:
			return( lval_char != NULL 
			&&  IS_SET(lval_char->affected_by, flag_lookup(buf, affect_flags)) );
		case CHK_ACT:
			return( lval_char != NULL 
			&&  IS_SET(lval_char->act, flag_lookup(buf, act_flags)) );
		case CHK_IMM:
			return( lval_char != NULL 
			&&  IS_SET(lval_char->imm_flags, flag_lookup(buf, imm_flags)) );
		case CHK_OFF:
			return( lval_char != NULL 
			&&  IS_SET(lval_char->off_flags, flag_lookup(buf, off_flags)) );
		case CHK_CARRIES:
			if ( is_number( buf ) )
			return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, FALSE ) );
			else
			return( lval_char != NULL && (get_obj_carry( lval_char, buf, lval_char ) != NULL) );
		case CHK_WEARS:
			if ( is_number( buf ) )
			return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, TRUE ) );
			else
			return( lval_char != NULL && (get_obj_wear( lval_char, buf, TRUE ) != NULL) );
		case CHK_HAS:
			return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), FALSE ) );
		case CHK_USES:
			return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), TRUE ) );
		case CHK_NAME:
				switch( code )
				{
					default :
					case 'i':
					case 'n':
					case 't':
					case 'r':
					case 'q':
						return( lval_char != NULL && is_name( buf, lval_char->name ) );
					case 'o':
					case 'p':
						return( lval_obj != NULL && is_name( buf, lval_obj->name ) );
			}
		case CHK_POS:
			return( lval_char != NULL && lval_char->position == position_lookup( buf ) );
		case CHK_CLAN:
			return( lval_char != NULL && lval_char->clan == clan_lookup( buf ) );
		case CHK_RACE:
			return( lval_char != NULL && lval_char->race == race_lookup( buf ) );
		case CHK_CLASS:
			return( lval_char != NULL && lval_char->ch_class == ch_class_lookup( buf ) );
		case CHK_OBJTYPE:
			return( lval_obj != NULL && lval_obj->item_type == item_lookup( buf ) );
		case CHK_GOD:
			return( lval_char != NULL && lval_char->god == god_lookup( buf ) );
		case CHK_ON_QUEST:
			return( lval_char != NULL && get_quest_index(atol(buf)) && on_quest(lval_char, get_quest_index(atol(buf))));
		case CHK_COMPLETED_QUEST:
			return( lval_char != NULL && get_quest_index(atol(buf)) && has_completed_quest(lval_char, get_quest_index(atol(buf))));
		default:;
		}
	 
		/*
		 * Case 5: Keyword, actor, comparison and value
		 */
		if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
		{
		sprintf( buf, "Cmd_eval_mob: prog %d syntax error(5): '%s'",
			vnum, original );
		bug( buf, 0 );
		return FALSE;
		}
		line = one_argument( line, buf );
		rval = atoi( buf );
	 
		switch( check )
		{
		case CHK_VNUM:
			switch( code )
				{
					default :
					case 'i':
					case 'n':
					case 't':
					case 'r':
			case 'q':
						if( lval_char != NULL && IS_NPC( lval_char ) )
							lval = lval_char->pIndexData->vnum;
						break;
					case 'o':
					case 'p':
						 if ( lval_obj != NULL )
							lval = lval_obj->pIndexData->vnum;
				}
				break;
		case CHK_HPCNT:
			if ( lval_char != NULL ) lval = (lval_char->hit * 100)/(UMAX(1,lval_char->max_hit)); break;
		case CHK_ROOM:
			if ( lval_char != NULL && lval_char->in_room != NULL )
			lval = lval_char->in_room->vnum; break;
		case CHK_SEX:
			if ( lval_char != NULL ) lval = lval_char->sex; break;
		case CHK_LEVEL:
				if ( lval_char != NULL ) lval = lval_char->level; break;
		case CHK_SOCIETY_RANK:
				if ( lval_char != NULL ) lval = lval_char->society_rank; break;				
		case CHK_ALIGN:
				if ( lval_char != NULL ) lval = lval_char->alignment; break;
		case CHK_MONEY:  /* Money is converted to silver... */
			if ( lval_char != NULL ) 
			lval = lval_char->gold + (lval_char->silver * 100); break;
		case CHK_OBJVAL0:
				if ( lval_obj != NULL ) lval = lval_obj->value[0]; break;
			case CHK_OBJVAL1:
				if ( lval_obj != NULL ) lval = lval_obj->value[1]; break;
			case CHK_OBJVAL2: 
				if ( lval_obj != NULL ) lval = lval_obj->value[2]; break;
			case CHK_OBJVAL3:
				if ( lval_obj != NULL ) lval = lval_obj->value[3]; break;
		case CHK_OBJVAL4:
			if ( lval_obj != NULL ) lval = lval_obj->value[4]; break;
		case CHK_GRPSIZE:
			if( lval_char != NULL ) lval = count_people_room( lval_char, NULL, NULL, 4 ); break;
		default:
				return FALSE;
    }
    return( num_eval( lval, oper, rval ) );
}
 
int cmd_eval_obj( sh_int vnum, char *line, int check,
	OBJ_DATA *obj, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    CHAR_DATA *lval_char = NULL;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    OBJ_DATA *lval_obj = obj;
 
    char *original, buf[MAX_INPUT_LENGTH], code;
    int lval = 0, oper = 0, rval = -1;
 
    original = line;
    line = one_argument( line, buf );
    if ( buf[0] == '\0' || obj == NULL )
	return FALSE;
 
    /*
     * If this object has no target, let's assume our victim is the one
     */
    if ( obj->oprog_target == NULL )
	obj->oprog_target = ch;
 
    switch ( check )
    {
	/*
	 * Case 1: keyword and value
	 */
	case CHK_RAND:
	    return( atoi( buf ) < number_percent() );
	case CHK_MOBHERE:
	    if ( is_number( buf ) )
		return( get_mob_vnum_room( NULL, obj, NULL,  atoi(buf) ) );
	    else
		return( (bool) (get_char_room( NULL, (obj->in_room?obj->in_room:obj->carried_by->in_room), buf) != NULL) );
	case CHK_OBJHERE:
	    if ( is_number( buf ) )
		return( get_obj_vnum_room( NULL, obj, NULL,  atoi(buf) ) );
	    else
		return( (bool) (get_obj_here( NULL, (obj->in_room?obj->in_room:obj->carried_by->in_room), buf) != NULL) );
        case CHK_MOBEXISTS:
	    return( (bool) (get_char_world( NULL, buf) != NULL) );
	case CHK_OBJEXISTS:
	    return( (bool) (get_obj_world( NULL, buf) != NULL) );
	/*
	 * Case 2 begins here: We sneakily use rval to indicate need
	 * 		       for numeric eval...
	 */
	case CHK_PEOPLE:
	    rval = count_people_room( NULL, obj, NULL, 0 ); break;
	case CHK_PLAYERS:
	    rval = count_people_room( NULL, obj, NULL, 1 ); break;
	case CHK_MOBS:
	    rval = count_people_room( NULL, obj, NULL, 2 ); break;
	case CHK_CLONES:
	    bug( "cmd_eval_obj: received CHK_CLONES.",0); break;
	case CHK_ORDER:
	    rval = get_order( NULL, obj ); break;
	case CHK_HOUR:
	    rval = time_info.hour; break;
	default:;
    }
 
    /*
     * Case 2 continued: evaluate expression
     */
    if ( rval >= 0 )
    {
	if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
	{
	    sprintf( buf, "Cmd_eval_obj: prog %d syntax error(2) '%s'",
		vnum, original );
	    bug( buf, 0 );
	    return FALSE;
	}
	one_argument( line, buf );
	lval = rval;
	rval = atoi( buf );
	return( num_eval( lval, oper, rval ) );
    }
 
    /*
     * Case 3,4,5: Grab actors from $* codes
     */
    if ( buf[0] != '$' || buf[1] == '\0' )
    {
	sprintf( buf, "Cmd_eval_obj: prog %d syntax error(3) '%s'",
		vnum, original );
	bug( buf, 0 );
        return FALSE;
    }
    else
        code = buf[1];
    switch( code )
    {
    	case 'i':
            lval_obj = obj; break;
        case 'n':
            lval_char = ch; break;
        case 't':
            lval_char = vch; break;
        case 'r':
            lval_char = rch == NULL ? get_random_char( NULL, obj, NULL ) : rch ; break;
        case 'o':
            lval_obj = obj1; break;
        case 'p':
            lval_obj = obj2; break;
	case 'q':
	    lval_char = obj->oprog_target; break;
	default:
	    sprintf( buf, "Cmd_eval_obj: prog %d syntax error(4) '%s'",
		vnum, original );
	    bug( buf, 0 );
	    return FALSE;
    }
    /*
     * From now on, we need an actor, so if none was found, bail out
     */
    if ( lval_char == NULL && lval_obj == NULL )
    	return FALSE;
 
    /*
     * Case 3: Keyword, comparison and value
     */
    switch( check )
    {
	case CHK_ISPC:
            return( lval_char != NULL && !IS_NPC( lval_char ) );
        case CHK_ISNPC:
            return( lval_char != NULL && IS_NPC( lval_char ) );
        case CHK_ISGOOD:
            return( lval_char != NULL && IS_GOOD( lval_char ) );
        case CHK_ISEVIL:
            return( lval_char != NULL && IS_EVIL( lval_char ) );
        case CHK_ISNEUTRAL:
            return( lval_char != NULL && IS_NEUTRAL( lval_char ) );
	case CHK_ISIMMORT:
            return( lval_char != NULL && IS_IMMORTAL( lval_char ) );
        case CHK_ISCHARM: /* A relic from MERC 2.2 MOBprograms */
            return( lval_char != NULL && IS_AFFECTED( lval_char, AFF_CHARM ) );
        case CHK_ISFOLLOW:
            return( lval_char != NULL && lval_char->master != NULL 
		 && lval_char->master->in_room == lval_char->in_room );
	case CHK_ISACTIVE:
	    return( lval_char != NULL && lval_char->position > POS_SLEEPING );
	case CHK_ISDELAY:
	    return( lval_char != NULL && lval_char->mprog_delay > 0 );
	case CHK_HASTARGET:
	    return( lval_char != NULL && lval_char->mprog_target != NULL
		&&  lval_char->in_room == lval_char->mprog_target->in_room );
	case CHK_ISTARGET:
	    return( lval_char != NULL && obj->oprog_target == lval_char );
	default:;
     }
 
     /* 
      * Case 4: Keyword, actor and value
      */
     line = one_argument( line, buf );
     switch( check )
     {
	case CHK_AFFECTED:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->affected_by, flag_lookup(buf, affect_flags)) );
	case CHK_ACT:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->act, flag_lookup(buf, act_flags)) );
	case CHK_IMM:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->imm_flags, flag_lookup(buf, imm_flags)) );
	case CHK_OFF:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->off_flags, flag_lookup(buf, off_flags)) );
	case CHK_CARRIES:
	    if ( is_number( buf ) )
		return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, FALSE ) );
	    else
		return( lval_char != NULL && (get_obj_carry( lval_char, buf, lval_char ) != NULL) );
	case CHK_WEARS:
	    if ( is_number( buf ) )
		return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, TRUE ) );
	    else
		return( lval_char != NULL && (get_obj_wear( lval_char, buf, FALSE ) != NULL) );
	case CHK_HAS:
	    return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), FALSE ) );
	case CHK_USES:
	    return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), TRUE ) );
	case CHK_NAME:
            switch( code )
            {
                default :
                case 'n':
                case 't':
                case 'r':
		case 'q':
		    return( lval_char != NULL && is_name( buf, lval_char->name ) );
		case 'i':
		case 'o':
		case 'p':
		    return( lval_obj != NULL && is_name( buf, lval_obj->name ) );
	    }
	case CHK_POS:
	    return( lval_char != NULL && lval_char->position == position_lookup( buf ) );
	case CHK_CLAN:
	    return( lval_char != NULL && lval_char->clan == clan_lookup( buf ) );
	case CHK_RACE:
	    return( lval_char != NULL && lval_char->race == race_lookup( buf ) );
	case CHK_CLASS:
		return( lval_char != NULL && lval_char->ch_class ==ch_class_lookup( buf ) );
	case CHK_OBJTYPE:
	    return( lval_obj != NULL && lval_obj->item_type == item_lookup( buf ) );
	case CHK_GOD:	
		return( lval_char != NULL && lval_char->god == god_lookup( buf ) );
	default:;
    }
 
    /*
     * Case 5: Keyword, actor, comparison and value
     */
    if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
    {
	sprintf( buf, "Cmd_eval_obj: prog %d syntax error(5): '%s'",
		vnum, original );
	bug( buf, 0 );
	return FALSE;
    }
    one_argument( line, buf );
    rval = atoi( buf );
 
    switch( check )
    {
	case CHK_VNUM:
	    switch( code )
            {
                default :
                case 'n':
                case 't':
                case 'r':
		case 'q':
                    if( lval_char != NULL && IS_NPC( lval_char ) )
                        lval = lval_char->pIndexData->vnum;
                    break;
		case 'i':
                case 'o':
                case 'p':
                     if ( lval_obj != NULL )
                        lval = lval_obj->pIndexData->vnum;
            }
            break;
	case CHK_HPCNT:
	    if ( lval_char != NULL ) lval = (lval_char->hit * 100)/(UMAX(1,lval_char->max_hit)); break;
	case CHK_ROOM:
	    if ( lval_char != NULL && lval_char->in_room != NULL )
		lval = lval_char->in_room->vnum;
	    else if ( lval_obj != NULL && (lval_obj->in_room != NULL || lval_obj->carried_by != NULL ))
		lval = lval_obj->in_room?lval_obj->in_room->vnum:lval_obj->carried_by->in_room->vnum; 
	    break;
    case CHK_SEX:
	    if ( lval_char != NULL ) lval = lval_char->sex; break;
    case CHK_LEVEL:
            if ( lval_char != NULL ) lval = lval_char->level; break;
	case CHK_SOCIETY_RANK:
		    if ( lval_char != NULL ) lval = lval_char->society_rank; break;
	case CHK_ALIGN:
            if ( lval_char != NULL ) lval = lval_char->alignment; break;
	case CHK_MONEY:  /* Money is converted to silver... */
	    if ( lval_char != NULL ) 
		lval = lval_char->gold + (lval_char->silver * 100); break;
	case CHK_OBJVAL0:
            if ( lval_obj != NULL ) lval = lval_obj->value[0]; break;
        case CHK_OBJVAL1:
            if ( lval_obj != NULL ) lval = lval_obj->value[1]; break;
        case CHK_OBJVAL2: 
            if ( lval_obj != NULL ) lval = lval_obj->value[2]; break;
        case CHK_OBJVAL3:
            if ( lval_obj != NULL ) lval = lval_obj->value[3]; break;
	case CHK_OBJVAL4:
	    if ( lval_obj != NULL ) lval = lval_obj->value[4]; break;
	case CHK_GRPSIZE:
	    if( lval_char != NULL ) lval = count_people_room( lval_char, NULL, NULL, 4 ); break;
	default:
            return FALSE;
    }
    return( num_eval( lval, oper, rval ) );
}
 
int cmd_eval_room( sh_int vnum, char *line, int check,
	ROOM_INDEX_DATA *room, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    CHAR_DATA *lval_char = NULL;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    OBJ_DATA *lval_obj = NULL;
 
    char *original, buf[MAX_INPUT_LENGTH], code;
    int lval = 0, oper = 0, rval = -1;
 
    original = line;
    line = one_argument( line, buf );
    if ( buf[0] == '\0' || room == NULL )
	return FALSE;
 
    /*
     * If this room has no target, let's assume our victim is the one
     */
    if ( room->rprog_target == NULL )
	room->rprog_target = ch;
 
    switch ( check )
    {
	/*
	 * Case 1: keyword and value
	 */
	case CHK_RAND:
	    return( atoi( buf ) < number_percent() );
	case CHK_MOBHERE:
	    if ( is_number( buf ) )
		return( get_mob_vnum_room( NULL, NULL, room,  atoi(buf) ) );
	    else
		return( (bool) (get_char_room( NULL, room, buf) != NULL) );
	case CHK_OBJHERE:
	    if ( is_number( buf ) )
		return( get_obj_vnum_room( NULL, NULL, room,  atoi(buf) ) );
	    else
		return( (bool) (get_obj_here( NULL, room, buf) != NULL) );
        case CHK_MOBEXISTS:
	    return( (bool) (get_char_world( NULL, buf) != NULL) );
	case CHK_OBJEXISTS:
	    return( (bool) (get_obj_world( NULL, buf) != NULL) );
	/*
	 * Case 2 begins here: We sneakily use rval to indicate need
	 * 		       for numeric eval...
	 */
	case CHK_PEOPLE:
	    rval = count_people_room( NULL, NULL, room, 0 ); break;
	case CHK_PLAYERS:
	    rval = count_people_room( NULL, NULL, room, 1 ); break;
	case CHK_MOBS:
	    rval = count_people_room( NULL, NULL, room, 2 ); break;
	case CHK_CLONES:
	    bug( "Cmd_eval_room: received CHK_CLONES.",0); break;
	case CHK_ORDER:
	    bug( "Cmd_eval_room: received CHK_ORDER.",0); break;
	case CHK_HOUR:
	    rval = time_info.hour; break;
	default:;
    }
 
    /*
     * Case 2 continued: evaluate expression
     */
    if ( rval >= 0 )
    {
	if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
	{
	    sprintf( buf, "Cmd_eval_room: prog %d syntax error(2) '%s'",
		vnum, original );
	    bug( buf, 0 );
	    return FALSE;
	}
	one_argument( line, buf );
	lval = rval;
	rval = atoi( buf );
	return( num_eval( lval, oper, rval ) );
    }
 
    /*
     * Case 3,4,5: Grab actors from $* codes
     */
    if ( buf[0] != '$' || buf[1] == '\0' )
    {
	sprintf( buf, "Cmd_eval_room: prog %d syntax error(3) '%s'",
		vnum, original );
	bug( buf, 0 );
        return FALSE;
    }
    else
        code = buf[1];
    switch( code )
    {
    	case 'i':
            bug( "Cmd_eval_room: received code case 'i'.",0); break;
        case 'n':
            lval_char = ch; break;
        case 't':
            lval_char = vch; break;
        case 'r':
            lval_char = rch == NULL ? get_random_char( NULL, NULL, room ) : rch ; break;
        case 'o':
            lval_obj = obj1; break;
        case 'p':
            lval_obj = obj2; break;
	case 'q':
	    lval_char = room->rprog_target; break;
	default:
	    sprintf( buf, "Cmd_eval_room: prog %d syntax error(4) '%s'",
		vnum, original );
	    bug( buf, 0 );
	    return FALSE;
    }
    /*
     * From now on, we need an actor, so if none was found, bail out
     */
    if ( lval_char == NULL && lval_obj == NULL )
    	return FALSE;
 
    /*
     * Case 3: Keyword, comparison and value
     */
    switch( check )
    {
	case CHK_ISPC:
            return( lval_char != NULL && !IS_NPC( lval_char ) );
        case CHK_ISNPC:
            return( lval_char != NULL && IS_NPC( lval_char ) );
        case CHK_ISGOOD:
            return( lval_char != NULL && IS_GOOD( lval_char ) );
        case CHK_ISEVIL:
            return( lval_char != NULL && IS_EVIL( lval_char ) );
        case CHK_ISNEUTRAL:
            return( lval_char != NULL && IS_NEUTRAL( lval_char ) );
	case CHK_ISIMMORT:
            return( lval_char != NULL && IS_IMMORTAL( lval_char ) );
        case CHK_ISCHARM: /* A relic from MERC 2.2 MOBprograms */
            return( lval_char != NULL && IS_AFFECTED( lval_char, AFF_CHARM ) );
        case CHK_ISFOLLOW:
            return( lval_char != NULL && lval_char->master != NULL 
		 && lval_char->master->in_room == lval_char->in_room );
	case CHK_ISACTIVE:
	    return( lval_char != NULL && lval_char->position > POS_SLEEPING );
	case CHK_ISDELAY:
	    return( lval_char != NULL && lval_char->mprog_delay > 0 );
	case CHK_HASTARGET:
	    return( lval_char != NULL && lval_char->mprog_target != NULL
		&&  lval_char->in_room == lval_char->mprog_target->in_room );
	case CHK_ISTARGET:
	    return( lval_char != NULL && room->rprog_target == lval_char );
	default:;
     }
 
     /* 
      * Case 4: Keyword, actor and value
      */
     line = one_argument( line, buf );
     switch( check )
     {
	case CHK_AFFECTED:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->affected_by, flag_lookup(buf, affect_flags)) );
	case CHK_ACT:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->act, flag_lookup(buf, act_flags)) );
	case CHK_IMM:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->imm_flags, flag_lookup(buf, imm_flags)) );
	case CHK_OFF:
	    return( lval_char != NULL 
		&&  IS_SET(lval_char->off_flags, flag_lookup(buf, off_flags)) );
	case CHK_CARRIES:
	    if ( is_number( buf ) )
		return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, FALSE ) );
	    else
		return( lval_char != NULL && (get_obj_carry( lval_char, buf, lval_char ) != NULL) );
	case CHK_WEARS:
	    if ( is_number( buf ) )
		return( lval_char != NULL && has_item( lval_char, atoi(buf), -1, TRUE ) );
	    else
		return( lval_char != NULL && (get_obj_wear( lval_char, buf, FALSE ) != NULL) );
	case CHK_HAS:
	    return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), FALSE ) );
	case CHK_USES:
	    return( lval_char != NULL && has_item( lval_char, -1, item_lookup(buf), TRUE ) );
	case CHK_NAME:
            switch( code )
            {
                default :
                case 'n':
                case 't':
                case 'r':
		case 'q':
		    return( lval_char != NULL && is_name( buf, lval_char->name ) );
		case 'i':
			return FALSE;
		case 'o':
		case 'p':
		    return( lval_obj != NULL && is_name( buf, lval_obj->name ) );
	    }
	case CHK_POS:
	    return( lval_char != NULL && lval_char->position == position_lookup( buf ) );
	case CHK_CLAN:
	    return( lval_char != NULL && lval_char->clan == clan_lookup( buf ) );
	case CHK_RACE:
	    return( lval_char != NULL && lval_char->race == race_lookup( buf ) );
	case CHK_CLASS:
		return( lval_char != NULL && lval_char->ch_class ==ch_class_lookup( buf ) );
	case CHK_OBJTYPE:
	    return( lval_obj != NULL && lval_obj->item_type == item_lookup( buf ) );
	case CHK_GOD:
		return( lval_char != NULL && lval_char->god == god_lookup( buf ) );
	default:;
    }
 
    /*
     * Case 5: Keyword, actor, comparison and value
     */
    if ( (oper = keyword_lookup( fn_evals, buf )) < 0 )
    {
	sprintf( buf, "Cmd_eval_room: prog %d syntax error(5): '%s'",
		vnum, original );
	bug( buf, 0 );
	return FALSE;
    }
    one_argument( line, buf );
    rval = atoi( buf );
 
    switch( check )
    {
	case CHK_VNUM:
	    switch( code )
            {
                default :
                case 'n':
                case 't':
                case 'r':
		case 'q':
                    if( lval_char != NULL && IS_NPC( lval_char ) )
                        lval = lval_char->pIndexData->vnum;
                    break;
		case 'i':
			return FALSE;
                case 'o':
                case 'p':
                     if ( lval_obj != NULL )
                        lval = lval_obj->pIndexData->vnum;
            }
            break;
	case CHK_HPCNT:
	    if ( lval_char != NULL ) lval = (lval_char->hit * 100)/(UMAX(1,lval_char->max_hit)); break;
	case CHK_ROOM:
	    if ( lval_char != NULL && lval_char->in_room != NULL )
		lval = lval_char->in_room->vnum;
	    else if ( lval_obj != NULL && (lval_obj->in_room != NULL || lval_obj->carried_by != NULL ) )
		lval = lval_obj->in_room?lval_obj->in_room->vnum:lval_obj->carried_by->in_room->vnum; 
	    break;
    case CHK_SEX:
	    if ( lval_char != NULL ) lval = lval_char->sex; break;
    case CHK_LEVEL:
        if ( lval_char != NULL ) lval = lval_char->level; break;
	case CHK_SOCIETY_RANK:
		if ( lval_char != NULL ) lval = lval_char->society_rank; break;				
	case CHK_ALIGN:
            if ( lval_char != NULL ) lval = lval_char->alignment; break;
	case CHK_MONEY:  /* Money is converted to silver... */
	    if ( lval_char != NULL ) 
		lval = lval_char->gold + (lval_char->silver * 100); break;
	case CHK_OBJVAL0:
            if ( lval_obj != NULL ) lval = lval_obj->value[0]; break;
        case CHK_OBJVAL1:
            if ( lval_obj != NULL ) lval = lval_obj->value[1]; break;
        case CHK_OBJVAL2: 
            if ( lval_obj != NULL ) lval = lval_obj->value[2]; break;
        case CHK_OBJVAL3:
            if ( lval_obj != NULL ) lval = lval_obj->value[3]; break;
	case CHK_OBJVAL4:
	    if ( lval_obj != NULL ) lval = lval_obj->value[4]; break;
	case CHK_GRPSIZE:
	    if( lval_char != NULL ) lval = count_people_room( lval_char, NULL, NULL, 4 ); break;
	default:
            return FALSE;
    }
    return( num_eval( lval, oper, rval ) );
}
 
/*
 * ------------------------------------------------------------------------
 * EXPAND_ARG
 * These are hacks of act() in comm.c. I've added some safety guards,
 * so that missing or invalid $-codes do not crash the server
 * ------------------------------------------------------------------------
 */
void expand_arg_mob( char *buf, 
	const char *format, 
	CHAR_DATA *mob, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
    const char *someone = "someone";
    const char *something = "something";
    const char *someones = "someone's";
 
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;
 
    point   = buf;
    str     = format;
    while ( *str != '\0' )
    {
    	if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }
        ++str;
 
        switch ( *str )
        {
            default:  bug( "Expand_arg_mob: bad code %d.", *str );
                          i = " <@@@> ";                        break;
            case 'i':
		one_argument( mob->name, fname );
		i = fname;                         		break;
            case 'I': i = mob->short_descr;                     break;
            case 'n': 
		i = someone;
		if ( ch != NULL && can_see( mob, ch ) )
		{
            	    one_argument( ch->name, fname );
		    i = capitalize(fname);
		}						break;
            case 'N': 
	    	i = (ch != NULL && can_see( mob, ch ) )
		? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
		: someone;                         		break;
            case 't': 
		i = someone;
		if ( vch != NULL && can_see( mob, vch ) )
		{
            	     one_argument( vch->name, fname );
		     i = capitalize(fname);
		}						break;
            case 'T': 
	    	i = (vch != NULL && can_see( mob, vch ))
		? ( IS_NPC( vch ) ? vch->short_descr : vch->name )
		: someone;                         		break;
            case 'r': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob, NULL, NULL );
		i = someone;
		if( rch != NULL && can_see( mob, rch ) )
		{
                    one_argument( rch->name, fname );
		    i = capitalize(fname);
		} 						break;
            case 'R': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob, NULL, NULL );
		i  = ( rch != NULL && can_see( mob, rch ) )
		? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
		:someone;					break;
	    case 'q':
		i = someone;
		if ( mob->mprog_target != NULL && can_see( mob, mob->mprog_target ) )
	        {
		    one_argument( mob->mprog_target->name, fname );
		    i = capitalize( fname );
		} 						break;
	    case 'Q':
	    	i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
		? ( IS_NPC( mob->mprog_target ) ? mob->mprog_target->short_descr : mob->mprog_target->name )
		: someone;                         		break;
            case 'j': i = he_she  [URANGE(0, mob->sex, 2)];     break;
            case 'e': 
	    	i = (ch != NULL && can_see( mob, ch ))
		? he_she  [URANGE(0, ch->sex, 2)]        
		: someone;					break;
            case 'E': 
	    	i = (vch != NULL && can_see( mob, vch ))
		? he_she  [URANGE(0, vch->sex, 2)]        
		: someone;					break;
            case 'J': 
		i = (rch != NULL && can_see( mob, rch ))
		? he_she  [URANGE(0, rch->sex, 2)]        
		: someone;					break;
	    case 'X':
		i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target))
		? he_she  [URANGE(0, mob->mprog_target->sex, 2)]
		: someone;					break;
            case 'k': i = him_her [URANGE(0, mob->sex, 2)];	break;
            case 'm': 
	    	i = (ch != NULL && can_see( mob, ch ))
		? him_her [URANGE(0, ch  ->sex, 2)]
		: someone;        				break;
            case 'M': 
	    	i = (vch != NULL && can_see( mob, vch ))
		? him_her [URANGE(0, vch ->sex, 2)]        
		: someone;					break;
            case 'K': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob, NULL, NULL );
		i = (rch != NULL && can_see( mob, rch ))
		? him_her [URANGE(0, rch ->sex, 2)]
		: someone;					break;
            case 'Y': 
	    	i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
		? him_her [URANGE(0, mob->mprog_target->sex, 2)]        
		: someone;					break;
            case 'l': i = his_her [URANGE(0, mob ->sex, 2)];    break;
            case 's': 
	    	i = (ch != NULL && can_see( mob, ch ))
		? his_her [URANGE(0, ch ->sex, 2)]
		: someones;					break;
            case 'S': 
	    	i = (vch != NULL && can_see( mob, vch ))
		? his_her [URANGE(0, vch ->sex, 2)]
		: someones;					break;
            case 'L': 
		if ( rch == NULL ) 
		    rch = get_random_char( mob, NULL, NULL );
		i = ( rch != NULL && can_see( mob, rch ) )
		? his_her [URANGE(0, rch ->sex, 2)]
		: someones;					break;
            case 'Z': 
	    	i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
		? his_her [URANGE(0, mob->mprog_target->sex, 2)]
		: someones;					break;
	    case 'o':
		i = something;
		if ( obj1 != NULL && can_see_obj( mob, obj1 ) )
		{
            	    one_argument( obj1->name, fname );
                    i = fname;
		} 						break;
            case 'O':
                i = (obj1 != NULL && can_see_obj( mob, obj1 ))
                ? obj1->short_descr
                : something;					break;
            case 'p':
		i = something;
		if ( obj2 != NULL && can_see_obj( mob, obj2 ) )
		{
            	    one_argument( obj2->name, fname );
            	    i = fname;
		} 						break;
            case 'P':
            	i = (obj2 != NULL && can_see_obj( mob, obj2 ))
                ? obj2->short_descr
                : something;					break;
        }
 
        ++str;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
 
    }
    *point = '\0';
 
    return;
}    
 
void expand_arg_other( char *buf, 
	const char *format, 
	OBJ_DATA *obj, ROOM_INDEX_DATA *room, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
    const char *someone = "someone";
    const char *something = "something";
    const char *someones = "someone's";
 
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
 
    if ( obj && room )
    {
	bug( "expand_arg_other received a obj and a room",0);
	return;
    }
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;
 
    point   = buf;
    str     = format;
    while ( *str != '\0' )
    {
    	if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }
        ++str;
 
        switch ( *str )
        {
            default:  bug( "Expand_arg: bad code %d.", *str );
                          i = " <@@@> ";                        break;
            case 'i':
		if ( obj )
		{
		    one_argument( obj->name, fname );
		    i = fname;
		}
		else
		{
		    bug( "Expand_arg_other: room had an \"i\" case.",0);
		    i = " <@@@> ";
		}						break;
            case 'I':
		if ( obj )
		    i = obj->short_descr;
		else
		{
		    bug( "Expand_arg_other: room had an \"I\" case.",0);
		    i = " <@@@> ";
		}						break;
            case 'n': 
		i = someone;
		if ( ch != NULL )
		{
            	    one_argument( ch->name, fname );
		    i = capitalize(fname);
		}						break;
            case 'N': 
	    	i = (ch != NULL )
		? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
		: someone;                         		break;
            case 't': 
		i = someone;
		if ( vch != NULL )
		{
            	     one_argument( vch->name, fname );
		     i = capitalize(fname);
		}						break;
            case 'T': 
	    	i = (vch != NULL )
		? ( IS_NPC( vch ) ? vch->short_descr : vch->name )
		: someone;                         		break;
            case 'r': 
		if ( rch == NULL && obj ) 
		    rch = get_random_char( NULL, obj, NULL );
		else if ( rch == NULL && room )
		    rch = get_random_char( NULL, NULL, room );
		i = someone;
		if( rch != NULL )
		{
                    one_argument( rch->name, fname );
		    i = capitalize(fname);
		} 						break;
            case 'R': 
		if ( rch == NULL && obj ) 
		    rch = get_random_char( NULL, obj, NULL );
		else if ( rch == NULL && room )
		    rch = get_random_char( NULL, NULL, room );
		i  = ( rch != NULL )
		? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
		:someone;					break;
	    case 'q':
		i = someone;
		if ( obj && obj->oprog_target != NULL )
	        {
		    one_argument( obj->oprog_target->name, fname );
		    i = capitalize( fname );
		}
		else if ( room && room->rprog_target != NULL )
		{
		    one_argument( room->rprog_target->name, fname );
		    i = capitalize( fname );
		} 						break;
	    case 'Q':
	    	i = (obj && obj->oprog_target != NULL)
		? ( IS_NPC( obj->oprog_target ) ? 
		obj->oprog_target->short_descr : obj->oprog_target->name )
		: (room && room->rprog_target != NULL)
		? ( IS_NPC( room->rprog_target ) ?
		room->rprog_target->short_descr : room->rprog_target->name )
		: someone;					break;
            case 'j':
		bug( "Expand_arg_other: Obj/room received case 'j'",0);
		i = " <@@@> ";     break;
            case 'e': 
	    	i = (ch != NULL )
		? he_she  [URANGE(0, ch->sex, 2)]        
		: someone;					break;
            case 'E': 
	    	i = (vch != NULL )
		? he_she  [URANGE(0, vch->sex, 2)]        
		: someone;					break;
            case 'J': 
		i = (rch != NULL )
		? he_she  [URANGE(0, rch->sex, 2)]        
		: someone;					break;
	    case 'X':
		i = (obj && obj->oprog_target != NULL )
		? he_she  [URANGE(0, obj->oprog_target->sex, 2)]
		: (room && room->rprog_target != NULL )
		? he_she  [URANGE(0, room->rprog_target->sex, 2)]
		: someone;					break;
            case 'k':
		bug( "Expand_arg_other: received case 'k'.",0);
		i = " <@@@> ";					break;
            case 'm': 
	    	i = (ch != NULL )
		? him_her [URANGE(0, ch->sex, 2)]
		: someone;        				break;
            case 'M': 
	    	i = (vch != NULL )
		? him_her [URANGE(0, vch->sex, 2)]        
		: someone;					break;
            case 'K': 
		if ( obj && rch == NULL ) 
		    rch = get_random_char( NULL, obj, NULL );
		else if ( room && rch == NULL )
		    rch = get_random_char( NULL, NULL, room );
		i = (rch != NULL)
		? him_her [URANGE(0, rch ->sex, 2)]
		: someone;					break;
            case 'Y': 
	    	i = (obj && obj->oprog_target != NULL)
		? him_her [URANGE(0, obj->oprog_target->sex, 2)]        
		: (room && room->rprog_target != NULL)
		? him_her [URANGE(0, room->rprog_target->sex, 2)]
		: someone;					break;
            case 'l':
		bug( "Expand_arg_other: received case 'l'.",0);
		i = " <@@@> ";					break;
            case 's': 
	    	i = (ch != NULL )
		? his_her [URANGE(0, ch ->sex, 2)]
		: someones;					break;
            case 'S': 
	    	i = (vch != NULL )
		? his_her [URANGE(0, vch ->sex, 2)]
		: someones;					break;
            case 'L': 
		if ( obj && rch == NULL ) 
		    rch = get_random_char( NULL, obj, NULL );
		else if ( room && rch == NULL )
		    rch = get_random_char( NULL, NULL, room );
		i = ( rch != NULL )
		? his_her [URANGE(0, rch ->sex, 2)]
		: someones;					break;
            case 'Z': 
	    	i = (obj && obj->oprog_target != NULL)
		? his_her [URANGE(0, obj->oprog_target->sex, 2)]
		: (room && room->rprog_target != NULL)
		? his_her [URANGE(0, room->rprog_target->sex, 2)]
		: someones;					break;
	    case 'o':
		i = something;
		if ( obj1 != NULL )
		{
            	    one_argument( obj1->name, fname );
                    i = fname;
		} 						break;
            case 'O':
                i = (obj1 != NULL)
                ? obj1->short_descr
                : something;					break;
            case 'p':
		i = something;
		if ( obj2 != NULL )
		{
            	    one_argument( obj2->name, fname );
            	    i = fname;
		} 						break;
            case 'P':
            	i = (obj2 != NULL)
                ? obj2->short_descr
                : something;					break;
        }
 
        ++str;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
 
    }
    *point = '\0';
 
    return;
}
 
/*
 * ------------------------------------------------------------------------
 *  PROGRAM_FLOW
 *  This is the program driver. It parses the mob program code lines
 *  and passes "executable" commands to interpret()
 *  Lines beginning with 'mob' are passed to mob_interpret() to handle
 *  special mob commands (in mob_cmds.c)
 *-------------------------------------------------------------------------
 */
 
#define MAX_NESTED_LEVEL 12 /* Maximum nested if-else-endif's (stack size) */
#define BEGIN_BLOCK       0 /* Flag: Begin of if-else-endif block */
#define IN_BLOCK         -1 /* Flag: Executable statements */
#define END_BLOCK        -2 /* Flag: End of if-else-endif block */
#define MAX_CALL_LEVEL   10 /* Maximum nested calls */
 
void program_flow( 
        long pvnum,  /* For diagnostic purposes */
	char *source,  /* the actual MOBprog code */
	CHAR_DATA *mob,
	OBJ_DATA *obj,
	ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, const void *arg1, const void *arg2, int numlines )
 
{
    CHAR_DATA *rch = NULL;
    char *code, *line;
    char buf[MAX_STRING_LENGTH];
    char control[MAX_INPUT_LENGTH], data[MAX_STRING_LENGTH];
    char bugbuf[MAX_STRING_LENGTH];
 
    static int call_level; /* Keep track of nested "mpcall"s */
 
    int level, eval, check;
    int state[MAX_NESTED_LEVEL], /* Block state (BEGIN,IN,END) */
	cond[MAX_NESTED_LEVEL];  /* Boolean value based on the last if-check */
	
	int count = 0;
	PROG_CODE *prog = get_mprog_by_vnum(pvnum);
 
    long mvnum = 0,ovnum = 0,rvnum = 0;
 
    if ( (mob && obj) || (mob && room) || (obj && room) )
    {
	bug( "PROGs: program_flow received multiple prog types.", 0 );
	return;
    }
 
    if ( mob )
	mvnum = mob->pIndexData->vnum;
    else if ( obj )
	ovnum = obj->pIndexData->vnum;
    else if ( room )
	rvnum = room->vnum;
    else
    {
	bug( "PROGs: program_flow did not receive a prog type.", 0 );
	return;
    }
 
    if( numlines == 1 && ++call_level > MAX_CALL_LEVEL )
    {
	if ( mob )
	    sprintf( bugbuf, "Progs: MAX_CALL_LEVEL exceeded, vnum %ld, mprog vnum %ld",
	      mvnum, pvnum );
	else if ( obj )
	    sprintf( bugbuf, "Progs: MAX_CALL_LEVEL exceeded, vnum %ld oprog vnum %ld.",
	      ovnum, pvnum );
	else
	    sprintf( bugbuf, "Progs: MAX_CALL_LEVEL exceeded, vnum %ld rprog vnum %ld.",
	      rvnum, pvnum );
	bug( bugbuf, 0 );
	call_level--;
	return;
    }
 
    /*
     * Reset "stack"
     */
    for ( level = 0; level < MAX_NESTED_LEVEL; level++ )
    {
    	state[level] = IN_BLOCK;
        cond[level]  = TRUE;
    }
    level = 0;
 
    code = source;
    /*
     * Parse the Prog code
     */
    while ( *code )
    {
	bool first_arg = TRUE;
	char *b = buf, *c = control, *d = data;
	/*
	 * Get a command line. We sneakily get both the control word
	 * (if/and/or) and the rest of the line in one pass.
	 */
	while( isspace( *code ) && *code ) code++;
	while ( *code )
	{
	    if ( *code == '\n' || *code == '\r' )
		break;
	    else if ( isspace(*code) )
	    {
		if ( first_arg )
		    first_arg = FALSE;
		else
		    *d++ = *code;
	    }
	    else
	    {
		if ( first_arg )
		   *c++ = *code;
		else
		   *d++ = *code;
	    }
	    *b++ = *code++;
	}
	*b = *c = *d = '\0';
	
	 if(++count < numlines)
    {
		if ( !str_cmp( control, "if" ) )
		{
			state[level] = BEGIN_BLOCK;
			state[++level] = END_BLOCK;
		}
		else if ( !str_cmp( control, "endif" ) )
		{
			state[level] = IN_BLOCK;
			state[--level] = END_BLOCK;
		}
	
		continue;
   	}
	
 
	if ( buf[0] == '\0' )
	    break;
	if ( buf[0] == '*' ) /* Comment */
	    continue;
 
        line = data;
	   /* 
	 * Match control words
	 */
	if ( !str_cmp( control, "sleep" ) )
	{
	    int timer;
	    SLEEP_DATA *sd;
	    
	    line = one_argument( line, control );
	    if(control[0] == '\0')
	        timer = 4;
	    else if(is_number(control) && atoi(control) > 0)
	        timer = atoi(control);
	    else
	    {
		    sprintf( buf, "Mobprog: argument NaN, using 4 as default. mob %ld prog %ld",
			mvnum, pvnum );
		    bug( buf, 0 );
           timer = 4;
	    }
	    
	    sd = new_sleep_data();
	    sd->ch = ch;
	    sd->mob = mob;
	    sd->line = count + 1;
	    sd->vnum = prog->vnum;
	    sd->timer = timer;
	    sd->prog = prog;
	    
	    sd->next = first_sleep;
	    if(first_sleep)
	        first_sleep->prev = sd;
	    first_sleep = sd;
	    return;
	}
	
	if ( !str_cmp( control, "if" ) )
	{
	    if ( state[level] == BEGIN_BLOCK )
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: misplaced if statement, mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: misplaced if statement, obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: misplaced if statement, room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    state[level] = BEGIN_BLOCK;
            if ( ++level >= MAX_NESTED_LEVEL )
            {
		if ( mob )
		    sprintf( buf, "Mobprog: Max nested level exceeded, mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: Max nested level exceeded, obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: Max nested level exceeded, room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) 
	    {
			cond[level] = FALSE;
			continue;
	    }
	    line = one_argument( line, control );
	    if ( mob && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
			cond[level] = cmd_eval_mob( pvnum, line, check, mob, ch, arg1, arg2, rch );
	    }
	    else if ( obj && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
			cond[level] = cmd_eval_obj( pvnum, line, check, obj, ch, arg1, arg2, rch );
	    }
	    else if ( room && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
			cond[level] = cmd_eval_room( pvnum, line, check, room, ch, arg1, arg2, rch );
	    }
	    else
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: invalid if_check (if), mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: invalid if_check (if), obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: invalid if_check (if), room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    state[level] = END_BLOCK;
    	}
	else if ( !str_cmp( control, "or" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: or without if, mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: or without if, obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: or without if, room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) continue;
	    line = one_argument( line, control );
	    if ( mob && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
			eval = cmd_eval_mob( pvnum, line, check, mob, ch, arg1, arg2, rch );
	    }
	    else if ( obj && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
			eval = cmd_eval_obj( pvnum, line, check, obj, ch, arg1, arg2, rch );
	    }
		else if ( room && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
			eval = cmd_eval_room( pvnum, line, check, room, ch, arg1, arg2, rch );
	    }
	    else
            {
		if ( mob )
		    sprintf( buf, "Mobprog: invalid if_check (or), mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: invalid if_check (or), obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: invalid if_check (or), room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
            }
            cond[level] = (eval == TRUE) ? TRUE : cond[level];
    	}
	else if ( !str_cmp( control, "and" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: and without if, mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: and without if, obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: and without if, room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) continue;
	    line = one_argument( line, control );
	    if ( mob && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
		eval = cmd_eval_mob( pvnum, line, check, mob, ch, arg1, arg2, rch );
	    }
	    else if ( obj && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
		eval = cmd_eval_obj( pvnum, line, check, obj, ch, arg1, arg2, rch );
	    }
	    else if ( room && ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
	    {
		eval = cmd_eval_room( pvnum, line, check, room, ch, arg1, arg2, rch );
	    }
	    else
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: invalid if_check (and), mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: invalid if_check (and), obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: invalid if_check (and), room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    cond[level] = (cond[level] == TRUE) && (eval == TRUE) ? TRUE : FALSE;
    	}
	else if ( !str_cmp( control, "endif" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: endif without if, mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: endif without if, obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: endif without if, room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		call_level--;
		return;
	    }
	    cond[level] = TRUE;
	    state[level] = IN_BLOCK;
            state[--level] = END_BLOCK;
        }
	else if ( !str_cmp( control, "else" ) )
	{
	    if ( !level || state[level-1] != BEGIN_BLOCK )
	    {
		if ( mob )
		    sprintf( buf, "Mobprog: else without if, mob %ld prog %ld",
			mvnum, pvnum );
		else if ( obj )
		    sprintf( buf, "Objprog: else without if, obj %ld prog %ld",
			ovnum, pvnum );
		else
		    sprintf( buf, "Roomprog: else without if, room %ld prog %ld",
			rvnum, pvnum );
		bug( buf, 0 );
		return;
	    }
	    if ( level && cond[level-1] == FALSE ) continue;
            state[level] = IN_BLOCK;
            cond[level] = (cond[level] == TRUE) ? FALSE : TRUE;
        }
    	else if ( cond[level] == TRUE
	&& ( !str_cmp( control, "break" ) || !str_cmp( control, "end" ) ) )
	{
	    call_level--;
            return;
	}
	else if ( (!level || cond[level] == TRUE) && buf[0] != '\0' )
	{
	    state[level] = IN_BLOCK;
 
	    if ( mob )
		expand_arg_mob( data, buf, mob, ch, arg1, arg2, rch );
	    else if ( obj )
		expand_arg_other( data, buf, obj, NULL, ch, arg1, arg2, rch );
	    else
		expand_arg_other( data, buf, NULL, room, ch, arg1, arg2, rch );
 
	    if ( !str_cmp( control, "mob" ) )
	    {
		/* 
		 * Found a mob restricted command, pass it to mob interpreter
		 */
		line = one_argument( data, control );
		if ( !mob )
		    bug( "mob command in non MOBprog",0);
		else
		    mob_interpret( mob, line );
	    }
	    else if ( !str_cmp( control, "obj" ) )
	    {
		/*
		 * Found an obj restricted command, pass it to obj interpreter
		 */
		line = one_argument( data, control );
		if ( !obj )
		    bug( "obj command in non OBJprog",0);
		else
		    obj_interpret( obj, line );
	    }
	    else if ( !str_cmp( control, "room" ) )
	    {
		/*
		 * Found a room restricted command, pass it to room interpreter
		 */
		line = one_argument( data, control );
		if ( !room )
		    bug( "room command in non ROOMprog", 0 );
		else
		    room_interpret( room, line );
	    }
	    else
	    {
		/* 
		 * Found a normal mud command, pass it to interpreter
		 */
		if ( !mob )
		    bug( "Normal MUD command in non-MOBprog, prog vnum %d", pvnum );
		else
		    interpret( mob, data );
	    }
	}
    }
    call_level--;
}
 
/* 
 * ---------------------------------------------------------------------
 * Trigger handlers. These are called from various parts of the code
 * when an event is triggered.
 * ---------------------------------------------------------------------
 */
 
/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
void p_act_trigger( 
	char *argument, CHAR_DATA *mob, OBJ_DATA *obj,
	ROOM_INDEX_DATA *room, CHAR_DATA *ch, 
	const void *arg1, const void *arg2, int type )
{
    PROG_LIST *prg;
	SLEEP_DATA *test;
 
    if ( (mob && obj) || (mob && room) || (obj && room) )
    {
	bug( "Multiple program types in ACT trigger.", 0 );
	return;
    }
 
    if ( mob )
    {
	for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
	{
	    if ( prg->trig_type == type 
	      &&  strstr( argument, prg->trig_phrase ) != NULL )
	    {
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
		program_flow( prg->vnum, prg->code, mob, NULL, NULL, ch, arg1, arg2, 1 );
		break;
	    }
	}
    }
    else if ( obj )
    {
	for ( prg = obj->pIndexData->oprogs; prg != NULL; prg = prg->next )
	{
	    if ( prg->trig_type == type
	      && strstr( argument, prg->trig_phrase ) != NULL )
	    {
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
		program_flow( prg->vnum, prg->code, NULL, obj, NULL, ch, arg1, arg2, 1 );
		break;
	    }
	}
    }
    else if ( room )
    {
	for ( prg = room->rprogs; prg != NULL; prg = prg->next )
	{
	    if ( prg->trig_type == type
	      && strstr( argument, prg->trig_phrase ) != NULL )
	    {
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
		program_flow( prg->vnum, prg->code, NULL, NULL, room, ch, arg1, arg2, 1 );
		break;
	    }
	}
    }
    else
	bug( "ACT trigger with no program type.", 0 );
 
    return;
}
 
/*
 * A general purpose percentage trigger. Checks if a random percentage
 * number is less than trigger phrase
 */
bool p_percent_trigger( CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
	CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
    PROG_LIST *prg;
	SLEEP_DATA *test;
	
    if ( (mob && obj) || (mob && room) || (obj && room) )
    {
	bug( "Multiple program types in PERCENT trigger.", 0 );
	return ( FALSE );
    }
 
    if ( mob )
    {
	for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
	{
	    if ( prg->trig_type == type 
	      &&   number_percent() < atoi( prg->trig_phrase ) )
	    {
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return (TRUE);
		program_flow( prg->vnum, prg->code, mob, NULL, NULL, ch, arg1, arg2, 1 );
		return ( TRUE );
	    }
	}
    }
    else if ( obj )
    {
	for ( prg = obj->pIndexData->oprogs; prg != NULL; prg = prg->next )
	{
	    if ( prg->trig_type == type
	      && number_percent() < atoi( prg->trig_phrase ) )
	    {
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return (TRUE);
		program_flow( prg->vnum, prg->code, NULL, obj, NULL, ch, arg1, arg2, 1 );
		return ( TRUE );
	    }
	}
    }
    else if ( room )
    {
	for ( prg = room->rprogs; prg != NULL; prg = prg->next )
	{
	    if ( prg->trig_type == type
	      && number_percent() < atoi( prg->trig_phrase ) )
	    {
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return (TRUE);
		program_flow( prg->vnum, prg->code, NULL, NULL, room, ch, arg1, arg2, 1 );
		return ( TRUE );
	    }
	}
    }
    else
	bug( "PERCENT trigger missing program type.", 0 );
 
    return ( FALSE );
}
 
void p_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{
    PROG_LIST *prg;
	SLEEP_DATA *test;
 
    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
	if ( prg->trig_type == TRIG_BRIBE
	  &&   amount >= atoi( prg->trig_phrase ) )
	{
	for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
	    program_flow( prg->vnum, prg->code, mob, NULL, NULL, ch, NULL, NULL, 1 );
	    break;
	}
    }
return;
}
 
bool p_exit_trigger( CHAR_DATA *ch, int dir, int type )
{
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
    PROG_LIST   *prg;
	SLEEP_DATA *test;
 
    if ( type == PRG_MPROG )
    {
	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{    
	    if ( IS_NPC( mob )
	      &&   ( HAS_TRIGGER_MOB(mob, TRIG_EXIT) || HAS_TRIGGER_MOB(mob, TRIG_EXALL) ) )
	    {
		for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
		{
		/*
		 * Exit trigger works only if the mobile is not busy
		 * (fighting etc.). If you want to be sure all players
		 * are caught, use ExAll trigger
		 */
		    if ( prg->trig_type == TRIG_EXIT
		      &&  dir == atoi( prg->trig_phrase )
		      &&  mob->position == mob->pIndexData->default_pos
		      &&  can_see( mob, ch ) )
		    {
			program_flow( prg->vnum, prg->code, mob, NULL, NULL, ch, NULL, NULL, 1 );
			return TRUE;
		    }
		    else
		    if ( prg->trig_type == TRIG_EXALL
		      &&   dir == atoi( prg->trig_phrase ) )
		    {
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return TRUE;
			program_flow( prg->vnum, prg->code, mob, NULL, NULL, ch, NULL, NULL, 1 );
			return TRUE;
		    }
		}
	    }
	}
    }
    else if ( type == PRG_OPROG )
    {
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
	{
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_EXALL ) )
	    {
		for ( prg = obj->pIndexData->oprogs; prg; prg = prg->next )
		{
		    if ( prg->trig_type == TRIG_EXALL
		      && dir == atoi( prg->trig_phrase ) )
		    {
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return TRUE;
			program_flow( prg->vnum, prg->code, NULL, obj, NULL, ch, NULL, NULL, 1 );
			return TRUE;
		    }
		}
	    }
	}
 
	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
	    for ( obj = mob->carrying; obj; obj = obj->next_content )
	    {
		if ( HAS_TRIGGER_OBJ( obj, TRIG_EXALL ) )
		{
		    for ( prg = obj->pIndexData->oprogs; prg; prg = prg->next )
		    {
			if ( prg->trig_type == TRIG_EXALL
			  && dir == atoi( prg->trig_phrase ) )
			{
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return TRUE;
			    program_flow( prg->vnum, prg->code, NULL, obj, NULL, ch, NULL, NULL, 1 );
			    return TRUE;
			}
		    }
		}
	    }
	}
    }
    else if ( type == PRG_RPROG )
    {
	room = ch->in_room;
 
	if ( HAS_TRIGGER_ROOM( room, TRIG_EXALL ) )
	{
	    for ( prg = room->rprogs; prg; prg = prg->next )
	    {
		if ( prg->trig_type == TRIG_EXALL
		    && dir == atoi( prg->trig_phrase ) )
		{
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return TRUE;
		    program_flow( prg->vnum, prg->code, NULL, NULL, room, ch, NULL, NULL, 1 );
		    return TRUE;
		}
	    }
	}
    }
 
    return FALSE;
}
 
void p_give_trigger( CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
			CHAR_DATA *ch, OBJ_DATA *dropped, int type )
{
 
    char        buf[MAX_INPUT_LENGTH], *p;
    PROG_LIST  *prg;
	SLEEP_DATA *test;
 
    if ( (mob && obj) || (mob && room) || (obj && room) )
    {
	bug( "Multiple program types in GIVE trigger.", 0 );
	return;
    }
 
    if ( mob )
    {
	for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
	    if ( prg->trig_type == TRIG_GIVE )
	    {
		p = prg->trig_phrase;
		/*
	 	 * Vnum argument
	 	 */
		if ( is_number( p ) )
		{
		    if ( dropped->pIndexData->vnum == atoi(p) )
		    {
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
			program_flow(prg->vnum, prg->code, mob, NULL, NULL, ch, (void *) dropped, NULL, 1);
			return;
		    }
		}
		/*
	 	 * Dropped object name argument, e.g. 'sword'
	 	 */
		else
		{
	    	    while( *p )
	    	    {
			p = one_argument( p, buf );
 
			if ( is_name( buf, dropped->name )
			  ||   !str_cmp( "all", buf ) )
			{
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
					
		    	    program_flow(prg->vnum, prg->code, mob, NULL, NULL, ch, (void *) dropped, NULL, 1);
		    	    return;
			}
		    }
		}
	    }
    }
    else if ( obj )
    {
	for ( prg = obj->pIndexData->oprogs; prg; prg = prg->next )
	    if ( prg->trig_type == type )
	    {
		for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
		program_flow( prg->vnum, prg->code, NULL, obj, NULL, ch, (void *) obj, NULL, 1 );
		return;
	    }
    }
    else if ( room )
    {
	for ( prg = room->rprogs; prg; prg = prg->next )
	    if ( prg->trig_type == type )
	    {
		p = prg->trig_phrase;
		/*
		 * Vnum argument
		 */
		if ( is_number( p ) )
		{
		    if ( dropped->pIndexData->vnum == atoi(p) )
		    {
			for(test = first_sleep; test != NULL; test = test->next)
				if(test->ch == ch && test->vnum == prg->vnum)
					return;
			program_flow( prg->vnum, prg->code, NULL, NULL, room, ch, (void *) dropped, NULL, 1 );
			return;
		    }
		}
		/*
		 * Dropped object name argument, e.g. 'sword'
		 */
		else
		{
		    while( *p )
		    {
			p = one_argument( p, buf );
 
			if ( is_name( buf, dropped->name )
			  || !str_cmp( "all", buf ) )
			{
				for(test = first_sleep; test != NULL; test = test->next)
					if(test->ch == ch && test->vnum == prg->vnum)
						return;
			    program_flow( prg->vnum, prg->code, NULL, NULL, room, ch, (void *) dropped, NULL, 1 );
			    return;
			}
		    }
		}
	    }
    }
}
 
void p_greet_trigger( CHAR_DATA *ch, int type )
{
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
 
    if ( type == PRG_MPROG )
    {
	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{    
	    if ( IS_NPC( mob )
	    && ( HAS_TRIGGER_MOB(mob, TRIG_GREET) || HAS_TRIGGER_MOB(mob,TRIG_GRALL) ) )
	    {
		/*
	 	* Greet trigger works only if the mobile is not busy
	 	* (fighting etc.). If you want to catch all players, use
	 	* GrAll trigger
	 	*/
		if ( HAS_TRIGGER_MOB( mob,TRIG_GREET )
		  &&   mob->position == mob->pIndexData->default_pos
		  &&   can_see( mob, ch ) )
			p_percent_trigger( mob, NULL, NULL, ch, NULL, NULL, TRIG_GREET );
		else                 
		    if ( HAS_TRIGGER_MOB( mob, TRIG_GRALL ) )
			p_percent_trigger( mob, NULL, NULL, ch, NULL, NULL, TRIG_GRALL );
	    }
	}
    }
    else if ( type == PRG_OPROG )
    {
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
	{
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_GRALL ) )
	    {
		p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_GRALL );
		return;
	    }
	}
 
	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
	    for ( obj = mob->carrying; obj; obj = obj->next_content )
	    {
		if ( HAS_TRIGGER_OBJ( obj, TRIG_GRALL ) )
		{
		    p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_GRALL );
		    return;
		}
	    }
	}
    }
    else if ( type == PRG_RPROG )
    {
	room = ch->in_room;
 
	if ( HAS_TRIGGER_ROOM( room, TRIG_GRALL ) )
		p_percent_trigger( NULL, NULL, room, ch, NULL, NULL, TRIG_GRALL );
    }
 
    return;
}

 
void p_hprct_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
    PROG_LIST *prg;
	SLEEP_DATA *test;
 
    for ( prg = mob->pIndexData->mprogs; prg != NULL; prg = prg->next )
	if ( ( prg->trig_type == TRIG_HPCNT )
	&& ( (100 * mob->hit / mob->max_hit) < atoi( prg->trig_phrase ) ) )
	{
		for(test = first_sleep; test != NULL; test = test->next)
			if(test->ch == ch && test->vnum == prg->vnum)
				return;
	    program_flow( prg->vnum, prg->code, mob, NULL, NULL, ch, NULL, NULL, 1 );
	    break;
	}
}



PROG_CODE *get_mprog_by_vnum(long vnum)
{
    PROG_CODE *temp;
    for(temp = mprog_list; temp != NULL; temp = temp->next) 
        if(temp->vnum == vnum)
            return temp;
    return NULL;
}


