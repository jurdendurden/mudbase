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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/****************************************************************************
 *   This file is just the stock nanny() function ripped from comm.c. It    *
 *   seems to be a popular task for new mud coders, so what the heck?       *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>                /* OLC -- for close read write etc */
#include <stdarg.h>                /* printf_to_char */

#include "merc.h"
#include "olc.h"
#include "interp.h"
#include "recycle.h"
#if    defined(macintosh) || defined(MSDOS)
extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];
#endif

#if    defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];
#endif

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args ((void));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

#if defined(unix)
void game_loop_unix args ((int control));
int init_socket args ((int port));
void init_descriptor args ((int control));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

/*
 *  * Other local functions (OS-independent).
 *   */
bool check_parse_name args ((char *name));
bool check_reconnect args ((DESCRIPTOR_DATA * d, char *name, bool fConn));
bool check_playing args ((DESCRIPTOR_DATA * d, char *name, bool account));

//Elemental Spec Upro
int specLookup args ((const char *name));

//Weapon spec Upro
int wpnSpecLookup args ((const char *name));

//god lookup Upro
int     god_lookup      args( ( const char *name) );

//stat roller
int roll_stat   	args (( CHAR_DATA *ch, int stat ));

/*
 * Global variables.
 */
extern DESCRIPTOR_DATA *descriptor_list;    /* All open descriptors     */
extern DESCRIPTOR_DATA *d_next;        /* Next descriptor in loop  */
extern FILE *fpReserve;                /* Reserved file handle     */
extern bool god;                        /* All new chars are gods!  */
extern bool merc_down;                    /* Shutdown         */
extern bool wizlock;                    /* Game is wizlocked        */
extern bool newlock;                    /* Game is newlocked        */
extern char str_boot_time[MAX_INPUT_LENGTH];
extern time_t current_time;            /* time of this pulse */

//Account globals.
//ACCOUNT_DATA * account;
bool loading = FALSE;



void show_ch_class_options(DESCRIPTOR_DATA*d)
{
	int iClass = 0;
	char buf[MSL];
	
	STD ("\r\n\r\nThe following classes are available:\r\n",d);					
	
	for (iClass = 0; iClass < MAX_CLASS; iClass++)
	{
		if (!qualify_ch_class(d->character, iClass))
			continue;
		if (iClass >= MAX_CLASS)
			break;			
		if ( ch_class_table[iClass].secret == FALSE)
		{
			sprintf (buf, "{r[{x %8s {r]{x\r\n", ch_class_table[iClass].name);
			STD(buf,d);                    						
		}
	}					
	
	return;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny (DESCRIPTOR_DATA * d, char *argument)
{
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;	
    char *pwdnew;
    char *p;
    //int iClass, race, i, weapon;
	int iClass = 0, race = 0, i = 0;
    bool fOld;
    extern int mud_telnetga, mud_ansicolor;
	int spellChance = 0;
	int element = 0, wpn = 0, favored = 0;
	int value = 0, y = 0;	
	int stat;
	int roll;
	int z;

    /* Delete leading spaces UNLESS character is writing a note */
	//if (d->connected != CON_NOTE_TEXT)
	//if (d->character->note_state != NOTE_TEXT)
	//{
		while ( isspace(*argument) )
			argument++;
	//}
    ch = d->character;	
	
    switch (d->connected)
    {

		default:
            bug ("Nanny: bad d->connected %d.", d->connected);
            close_socket (d);
            return;

		case CON_EXTRACTED:
			close_socket( d );
			return;

		case CON_INTRO:

			if(argument[0] =='\0' )
			{
				intro_menu(d);
				return;
			}
			switch(argument[0])
			{
				case 'c':	/* create a new account */
				case 'C':
					if(wizlock || newlock)
					{
						write_to_buffer(d, "The game is locked for development.  Please try again later.\n\r", 0);
						close_socket( d );
						return;
					}

					write_to_buffer(d, "Account Name:  ", 0);
					d->connected = CON_GET_NAME;
					return;
				
				case 'e':	/* exit */
				case 'E':
					write_to_buffer(d, "Goodbye.\n\r", 0);
					close_socket( d );
					return;
				
				case 'f':	/* mail password out */
				case 'F':
					write_to_buffer(d, "This is still being implemented.\n\r", 0);
					return;
				
				case 'l':	/* log into an account */
				case 'L':
					write_to_buffer(d,"Name:  ", 0);
					d->connected = CON_GET_OLD_ACCOUNT;
					return;
				
				case 'h':	/* get account help */
				case 'H':
					account_help( d );
					break;

				case 't':	/* toggle color */
				case 'T':
				{
					if(d->color)
						d->color = 0;
					else
						d->color = 1;
				}
				break;

				default:
					write_to_buffer( d, "Not a valid choice, please choose another.\n\r", 0);
				break;
			}
			
			/* if we get to here, we didn't change d->connected */
			intro_menu(d);
			return;
		break;

		case CON_SEND_PASS:
			d->connected = CON_INTRO;
		break;

		case CON_GET_OLD_ACCOUNT:
			if ( argument[0] == '\0' )
			{
				close_socket( d );
				return;
			}

			argument[0] = UPPER(argument[0]);
			if ( !check_account_name( argument ) )
			{
				write_to_buffer( d, "Illegal name, try another \n\rName:  ", 0);
				return;
			}
			d->account = NULL;
			fOld = load_account_obj( d, argument );

			if(!check_ban(d->host, BAN_PERMIT) 
				&& (check_ban(d->host, BAN_ALL) || check_ban(d->host, BAN_NEWBIES)))
			{
				if (!fOld || (fOld && !ACC_IS_SET(d->account->act, ACC_PERMIT)) )
				{
				write_to_buffer(d,"\n\r\n\rYou are banned from this game.\n\r",0);
				write_to_buffer(d,"\n\r\n\rGood Bye *wave*\n\r",0);
				close_socket(d);
				return;
				}
			}

					if (fOld)   /* Old account */
					{
				if ( IS_NULLSTR(d->account->name))
				{
							sprintf( log_buf, "Account bugged:  %s@%s.",argument, d->host);
							log_string( log_buf );
					write_to_buffer(d, "Your account is bugged/broken, contact an immortal ASAP.\n\r",0);
					close_socket(d);
					return;
				}
						if(ACC_IS_SET(d->account->act, ACC_DENY))
						{
							sprintf( log_buf, "Denying access to account %s@%s.", d->account->name, d->host);
							log_string( log_buf );
							write_to_buffer( d, "Your account has been denied.\n\r", 0);
							close_socket(d);
							return;
						}
				if(wizlock)
				{
				write_to_buffer(d, "{RATTENTION{x:  We are currently {YWIZLOCKED{x.\n\r",0);
				if(!ACC_IS_SET(d->account->act, ACC_IMM))
				{
					close_socket( d );			
					return;
				}
				}

						write_to_buffer( d, "Password: ", 0 );
						write_to_buffer( d, echo_off_str, 0 );
						d->connected = CON_GET_OLD_PASSWORD;
						return;
					}
			else 
			{
				write_to_buffer( d, "No account by that name exists.\n\r",0);
				intro_menu(d);
				d->connected = CON_INTRO;
				return;
			}
		break;

		case CON_GET_NAME:
			if ( argument[0] == '\0' )
			{
				close_socket( d );
				return;
			}

			argument[0] = UPPER(argument[0]);
			if ( !check_account_name( argument ) )
			{
				write_to_buffer( d, "Illegal name, try another.\n\rName:  ", 0 );
				return;
			}
				d->account = NULL;
			fOld = load_account_obj( d, argument );

			if (check_ban(d->host,BAN_PERMIT))
			{
				if (!fOld || (fOld && !ACC_IS_SET(d->account->act, ACC_PERMIT)) )
				{
				write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
				close_socket(d);
				return;
				}
			}

			if (fOld)   /* Old account */
			{
				write_to_buffer(d, "That account name already exists, try another.\n\rName:  ", 0);
				return;
			}

			else	/* New account */
			{
				if (newlock)
				{
				write_to_buffer( d, "The game is newlocked.\n\r", 0 );
				close_socket( d );
				return;
				}
			
			
				if (check_ban(d->host,BAN_NEWBIES))
				{
				write_to_buffer(d, "New players aren't allowed from your site.\n\r",0);
				close_socket(d);
				return;
				}
		
				create_account_menu( d );
				sprintf( buf, "\n\rDid I get that right, %s (Y/N)? ", argument );
				write_to_buffer( d, buf, 0 );
				d->account = new_account();
				free_string( d->account->name);
				d->account->name = str_dup(argument);
				d->connected = CON_CONFIRM_NEW_ACC_NAME;
				return;
			}

		break;

		case CON_GET_OLD_PASSWORD:
			#if defined(unix)
			write_to_buffer( d, "\n\r", 2 );
			#endif

			if ( strcmp( argument, d->account->password ))
			{
				write_to_buffer( d, "Wrong password.\n\r", 0 );
				close_socket( d );
				return;
			}
			write_to_buffer( d, echo_on_str, 0 );

			if(!ACC_IS_SET(d->account->act, VERIFY)) 
			{	/* account needs to be verified */
				STD("This account needs to be verified.\n\r", d);
				STD("Failure to verify your account will limit the number of characters\n\r", d);
				STD("you may have to {YONE{x, as well as reduce their maximum level to\n\r",d);
				STD("{Y15{x.  If you are having problems getting your account verified,\n\r",d);
				STD("speak with one of the immortals.\n\r", d);
				STD("Please enter your verification number:  ", d);
				d->connected = CON_ACC_VERIFY;
			}
			else	/* account has been verified */
			{
				account_main_menu(d);
				d->connected = CON_ACCOUNT;
			}
		break;

		case CON_ACC_VERIFY:
			if ( atoi( argument) != d->account->verify )
			{
				write_to_buffer( d, "Incorrect verification number.\n\r", 0);
				write_to_buffer( d, "Your account has not yet been verified.\n\r", 0);
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}

			write_to_buffer( d, "Account has been verified.\n\r", 0);
			ACC_SET(d->account, VERIFY);
			save_account_obj(d->account);
			account_main_menu(d);
			d->connected = CON_ACCOUNT;
		break;

		case CON_CONFIRM_NEW_ACC_NAME:
			switch ( argument[0] )
			{
			case 'y': 
			case 'Y':
				write_to_buffer( d, "( New account ) Please enter an email address. If\n\r", 0);
				write_to_buffer( d, "you don't have a valid email, you should enter NULL\n\r", 0 );
				write_to_buffer( d, "as your email address, then write a note to powerimms\n\r", 0); 
				write_to_buffer( d, "about it, or talk to Slate, Xerxes or Onorus to get\n\r", 0);
				write_to_buffer( d, "your account approved.\n\r", 0);
				write_to_buffer( d, "\n\rEmail address:  ", 0);

				// set newbie thing for new accounts.
				d->account->act = d->account->act | ACC_NEWBIE; 
				d->connected = CON_GET_EMAIL;
			break;

			case 'n':
			case 'N':
				write_to_buffer( d, "Ok, what IS it, then? :: ", 0 );
				free_string( d->account->name );
				d->account->name = str_dup("");
				d->connected = CON_GET_NAME;
			break;

			default:
				write_to_buffer( d, "Yes or No :: ", 0 );
				break;
			}
		break;

		case CON_GET_EMAIL: // if you change this, need to change it in account.c send_mail as well.
			d->account->verify = create_key();
			if(argument[0] == '\0')
				write_to_buffer( d, "You will need to speak with an immortal to get your account approved\n\r", 0);
			else 
			{
				if(!strcasecmp(argument, "NULL") )
				{
				write_to_buffer( d, "Remember to contact Slate, Xerxes or Onorus to\n\r", 0);
				write_to_buffer( d, "get your account verified.  If you don't, you\n\r", 0);
				write_to_buffer( d, "won't be able to play.\n\r",0);	
				}
				else
				{
				int check1 = check_account(d, argument);
				if(check1==1)
				{
					close_socket(d);
					return;
				}
				else if(check1 == 2)
				{
					free_string( d->account->email);
					d->account->email = str_dup(argument);
					d->connected = CON_VERIFY_EMAIL;
					return;
				}
				else
				{
					free_string( d->account->email);
					d->account->email = str_dup(argument);
					send_mail(d);
					write_to_buffer( d, "A verification number is being sent to your \n\r", 0);
					write_to_buffer( d, "email address.  You will need this number to\n\r", 0);
					write_to_buffer( d, "verify your account next time you log.\n\r\n\r", 0);
				}
				}
			}

			write_to_buffer( d, "What do you want your password to be?\n\r", 0);
			write_to_buffer( d, "Remember to choose something hard to guess,\n\r", 0);
			write_to_buffer( d, "because this allows acces to all your characters.\n\r", 0);
			write_to_buffer( d, "Password:  ", 0);
			write_to_buffer( d, echo_off_str, 0);
			d->connected = CON_GET_PASS;
			return;
		break;

		case CON_VERIFY_EMAIL:
			if ((argument[0] == 'y') || (argument[0] == 'Y'))
			{
					write_to_buffer( d, "\n\rWhat do you want your password to be?\n\r", 0);
					write_to_buffer( d, "Remember to choose something hard to guess,\n\r", 0);
					write_to_buffer( d, "because this allows acces to all your characters.\n\r", 0);
					write_to_buffer( d, "Password:  ", 0);
					write_to_buffer( d, echo_off_str, 0);
			d->connected = CON_GET_PASS;
			return;	
			}
			else
			{
			write_to_buffer( d, "\n\r( New account ) Please enter an email address.\n\r", 0);
			write_to_buffer( d, "If you don't have a valid email, you should enter\n\r", 0 );
			write_to_buffer( d, "NULL as your email address, then write a note to\n\r", 0);
					write_to_buffer( d, "powerimms about it, or talk to Slate, Xerxes or\n\r", 0);
			write_to_buffer( d, "Onorus to get your account approved.\n\r", 0);
			write_to_buffer( d, "\n\rEmail address:  ", 0);
			d->connected = CON_GET_EMAIL;
			return;
			}
		break;

		case CON_GET_PASS:
			#if defined(unix)
			write_to_buffer( d, "\n\r", 2 );
			#endif

			if ( strlen(argument) < 6 )
			{
				write_to_buffer( d, "Password must be at least six characters long.\n\rPassword: ",0 );
				return;
			}
			if(strstr(argument," ") != NULL){
				write_to_buffer(d,"Your password cannot contain spaces.\n\rPassword: ",0);
				return;
			}
			if(strstr(argument,"~") != NULL){
				write_to_buffer(d,"Your password cannot contain tildes.\n\rPassword: ",0);
				return;
			}
			free_string(d->account->password );
			d->account->password = str_dup( argument );
			write_to_buffer( d, "Please retype password: ", 0 );
			d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

		case CON_CONFIRM_NEW_PASSWORD:
			#if defined(unix)
			write_to_buffer( d, "\n\r", 2 );
			#endif

			if ( strcmp( argument, d->account->password ) )
			{
				write_to_buffer( d, "Passwords don't match.\n\rRetype password: ", 0 );
				d->connected = CON_GET_PASS;
				return;
			}

			save_account_obj(d->account);
			write_to_buffer( d, echo_on_str, 0 );
			account_main_menu( d );
			d->connected = CON_ACCOUNT;
		break;

		case CON_ACCOUNT:

			if(d->account != NULL)
				//d->account->cleanOldCharacters();

					if(account_is_playing(d))
						break;
	 
			switch(argument[0])
			{
			case 'c': /* create new character */
			case 'C':

						if(wizlock || newlock)
							{
								write_to_buffer(d, "The game is locked to new characters.\n\r", 0);
					account_main_menu(d);
								return;
							}

				if(account_get_new_char_num(d->account))
				{
			write_to_buffer(d,"\n\r\n\r",0);

			write_to_buffer(d,"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",0);
			write_to_buffer(d,"= You are creating a new character to live, explore, and interact within      =\n\r",0);
			write_to_buffer(d,"= a roleplaying environment with other real live players.  Please put some    =\n\r",0);
			write_to_buffer(d,"= amount of thought into your name.  If you are unfamiliar with our naming    =\n\r",0);
			write_to_buffer(d,"= rules, please consult 'help name'   In general:  video game, current        =\n\r",0);
			write_to_buffer(d,"= event, historical figure, popular characters from books, culture,           =\n\r",0);
			write_to_buffer(d,"= compound words, actual words, or words with a letter changed etc            =\n\r",0);
					write_to_buffer(d,"= are illegal.                                                                =\n\r",0);
			write_to_buffer(d,"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\r",0);
			write_to_buffer(d,"\n\r\n\r",0);
					write_to_buffer(d, "What name do you wish others to know you as?  ",0);
					d->connected = CON_GET_NEW_NAME;
					return;
				}
				else
				{
					write_to_buffer(d, "You have reached your maximum number of characters.\n\r", 0);
					account_main_menu(d);
					return;
				}

			case 'd': /* delete account */
			case 'D':
							write_to_buffer( d, "That's not a valid command.\n\r", 0);
							account_main_menu( d );
							return;
				
				write_to_buffer(d, "Are you sure about this?  Deleting your account is irreversable...  ", 0);
				d->connected = CON_DEL_ACC;
				return;

			case 'l': /* play a character */
			case 'L':
				list_characters(d);
				write_to_buffer(d, "\n\rWhich character do you want to play?  ", 0);
				d->connected = CON_PLAY_CHAR;
				return;

			case 's': /* list characters */
			case 'S':
				list_characters(d);
				account_main_menu( d );
				return;

			case 'e': /* change email */
			case 'E':/*still needs to be done*/
				write_to_buffer(d, "Temporarily disabled.\n\r", 0);
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;

			case 'a': /* change passowrd */
			case 'A':
				write_to_buffer(d, "What do you want to change your password too?  ", 0);
							write_to_buffer( d, echo_off_str, 0 );
				d->connected = CON_CHNG_PASS;
				return;
/*
			case 'r': // register char 
			case 'R':
				if( account_get_new_char_num( d->account) )
				{
					write_to_buffer(d, "What's the name of the character you want to register?  ", 0);
					d->connected = CON_REGISTER_CHAR;
				}
				else
				{
					write_to_buffer(d, "You have no open character slots.\n\r", 0);
					account_main_menu( d );
				}
				return;
*/
			case 'q': /* quit */
			case 'Q':
				write_to_buffer(d, "Good bye.\n\r", 0);
				close_socket( d );
				return;
			case 'w': /* who */
			case 'W': 
	//			write_to_buffer(d, "Not implemented yet.\n\r", 0);
				intro_who( d );
				account_main_menu(d);
				return;

			default:
				write_to_buffer( d, "That's not a valid command.\n\r", 0);
				account_main_menu( d );
				return;
			}
		break;

		case  CON_CHNG_PASS:
	//		write_to_buffer(d, "Not imped yet\n\r",0);
	//		account_main_menu( d );
	//		d->connected = CON_ACCOUNT;
					if ( argument[0] != '\0' )
					{
						if ( strlen(argument) < 6 )
				{
					write_to_buffer( d, "\n\rPassword must be at least six characters long.\n\rPassword: ",0 );
					return;
				}
				if(strstr(argument," ") != NULL){
					write_to_buffer(d,"\n\rYour password cannot contain spaces.\n\rPassword: ",0);
				return;
						}
				if(strstr(argument,"~") != NULL){
				write_to_buffer(d,"\n\rYour password cannot contain tildes.\n\rPassword: ",0);
				return;
				}
						free_string(d->account->old_passwd);
						d->account->old_passwd = str_dup(d->account->password);

						free_string(d->account->password);
						d->account->password = str_dup(argument);

						write_to_buffer(d, "\n\rConfirm your password by typing it again: ", 0);
						d->connected = CON_CNFRM_PASS; 
					}
					else
					{
						write_to_buffer(d, echo_on_str, 0);
						write_to_buffer(d, "\n\rPassword not changed!\n\r", 0);
						account_main_menu( d );
						d->connected = CON_ACCOUNT;
					}
		break;
		
		case CON_CNFRM_PASS:
				#if defined(unix)
			write_to_buffer( d, "\n\r", 2 );
			#endif
		
			if ( !str_cmp( argument, d->account->password ) )
			{
				write_to_buffer(d, "Password changed!\n\r", 0);

				free_string(d->account->old_passwd);
			d->account->old_passwd = str_dup( "" );

			free_string(d->account->password);
			d->account->password = str_dup(argument);

			save_account_obj(d->account);
			} 
			else
			{
				write_to_buffer(d, "Passwords don't match!\n\r", 0);

				free_string(d->account->password);
				d->account->password = str_dup(d->account->old_passwd);

				free_string(d->account->old_passwd);
			d->account->old_passwd = str_dup( "" );
			}

				write_to_buffer( d, echo_on_str, 0 );
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
					
			break;

		case CON_REGISTER_CHAR:
			if(argument[0] != '\0')
			{
				if( account_register_char(d, capitalize(argument)) )
				{   /* found character, read password */
					write_to_buffer(d, "Password:  ", 0);
					d->connected = CON_REGISTER_PASSWORD;
					return;
				}
				else
				{
					write_to_buffer(d, "Can not find character, or that character has been registered\n\r",0);
					account_main_menu( d );
					d->connected = CON_ACCOUNT;
					return;
				}
			}
			else 
			{
				write_to_buffer(d, "Can not find character, or that character has been registered\n\r",0);
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}
				
		
		break;

		case CON_REGISTER_PASSWORD:
			if((!argument) || (IS_NULLSTR(d->account->char_list[d->account->numb].password)))
			{
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}
			if(strcmp(argument,d->account->char_list[d->account->numb].password))
			{ /* passwords don't match */
				write_to_buffer(d, "Incorrect password\n\r", 0);
				
				/* no memory leeks*/
				account_free_char(d->account, d->account->numb);

				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}
			else
			{
				char abuff[1000];				
				/* move the character from UNREGISTER to PLAYER_DIR */
				sprintf(abuff, "mv %s%s %s%s", UNREGISTER, 
				   d->account->char_list[d->account->numb].char_name, 
				   PLAYER_DIR, d->account->char_list[d->account->numb].char_name);
				system(abuff);

				if(d->account->char_list[d->account->numb].level >= 49)
					ACC_SET(d->account, ACC_IMP);
				if(d->account->char_list[d->account->numb].level >= 42)
					ACC_SET(d->account, ACC_IMM);

				write_to_buffer(d, "character registered to your account\n\r", 0);
				save_account_obj(d->account);
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
			}

		break;

		case CON_DEL_ACC:
			switch( argument[0] )
			{
			case 'y':
			case 'Y':
				write_to_buffer(d, "Please enter your password:  ", 0);
				d->connected = CON_CONFIRM_DEL_ACC;
				return;
			default:
				write_to_buffer(d, "Whew... that was close.\n\n\r", 0);
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}
		break;

		case CON_CONFIRM_DEL_ACC:
			if(!(strcmp(argument, d->account->password)))
			{  /*delete account*/
				int i;
				char delbuf[MAX_STRING_LENGTH];
				del_account_name( d->account->email );
				for(i=0;i<MAX_ACCOUNT_CHAR; i++)
				{
					if(!IS_NULLSTR(d->account->char_list[i].char_name))
					{
						sprintf(delbuf, "%s%s", PLAYER_DIR, d->account->char_list[i].char_name);
						unlink(delbuf);
					}
				}
				sprintf(delbuf, "%s%s", ACCOUNT_DIR, d->account->name);
				unlink(delbuf);
				close_socket(d);
				return;
			}
			else
			{
				write_to_buffer(d, "Incorrect password.\n\n\r", 0);
				account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}
		break;

		case CON_PLAY_CHAR:
			
				if( IS_NULLSTR(argument) || !is_number( argument ))
			{
				write_to_buffer(d, "That is not a valid character.\n\r",0);
			account_main_menu( d );
			d->connected = CON_ACCOUNT;
			return;
			}

			i = atoi(argument);

			if ( i < 0 || i >= MAX_ACCOUNT_CHAR )
			{
			write_to_buffer(d, "That is not a valid character.\n\r",0);
			account_main_menu( d );
			d->connected = CON_ACCOUNT;
			return;
			}

					if (IS_NULLSTR(d->account->char_list[i].char_name))
				{
				write_to_buffer( d,"There has been a problem loading your character.\r\n",0);
				write_to_buffer( d,"Please contact the immortal staff.\n\r", 0);
					account_main_menu( d );
				d->connected = CON_ACCOUNT;
				return;
			}

			d->account->numb = i;
			fOld = load_char_obj( d, d->account->char_list[ i ].char_name );
			ch = d->character;		

			if(!fOld)
			{
				write_to_buffer( d,"There has been a problem loading your character.\r\n",0);
				write_to_buffer( d,"Please contact the immortal staff. \n\r", 0);
				account_main_menu( d );
					d->connected = CON_ACCOUNT;
				return;
			}

			if (IS_SET(ch->act, PLR_DENY))
			{
				write_to_buffer( d, "That character has been denied.\n\r",0);
				close_socket(d);
				return;
				}

			if (check_playing(d,ch->name, TRUE))
				return;

			if ( check_reconnect( d, ch->name, TRUE ) )
				return;

			/*JK2*/
			if (IS_SET(d->account->act, ACC_HOSTMASK))
			{ 
						sprintf( log_buf, "%s@%s has connected", ch->name,  d->host);
						wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
						sprintf( log_buf, "%s real host is: {C%s{x", ch->name, d->host );
						wiznet(log_buf,NULL,NULL,WIZ_SITES,0,UMAX(60,get_trust(ch)));
			}
					else 
					{
					sprintf( log_buf, "%s@%s has connected", ch->name, d->host);
						wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
			}					
		
			if ( IS_IMMORTAL(ch) )
			{
				WTB (d, "\r\n", 2);
				do_function (ch, &do_help, "imotd");				
				d->connected = CON_READ_IMOTD;
			}
			else
			{
				WTB (d, "\r\n", 2);
				do_function (ch, &do_help, "motd");
				d->connected = CON_READ_MOTD;
			}
			
			break;

		case CON_GET_NEW_NAME:
			if ( argument[0] == '\0' )
			{
				write_to_buffer( d, "Good bye.\n\r", 0 );
				close_socket( d );
				return;
			}
				for ( i = 0; argument[i] != '\0'; i++ )
				argument[i] = LOWER(argument[i]);
				/*if ( dict_test(argument) )
				{
					write_to_buffer( d, "That name showed up in our english word list.\n\rName: ", 0 );
					return;            
				}*/
			// checking for people in creation status
					for(DESCRIPTOR_DATA *dlist = descriptor_list; dlist; dlist = d_next)
					{
						d_next = dlist->next;
						
						if(!dlist->character || !dlist->character->name)
							continue;
						
						if(!str_cmp(argument,dlist->character->name))
						{
							write_to_buffer(d,"Someone else already has that name in use.\n\rName: ",0);
							return;
						}
					} 
					argument[0] = UPPER(argument[0]);
					if ( !check_parse_name( argument ) )
					{
						write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
						return;
					}

					if (ACC_IS_SET(d->account->act, PLR_DENY))
					{
						sprintf( log_buf, "Denying access to account %s@%s.", d->account->name, d->host);
						log_string( log_buf );
						write_to_buffer( d, "You are denied access.\n\r", 0 );
						account_main_menu( d );
						d->connected = CON_ACCOUNT;
						return;
					}

						if (newlock)
						{
						write_to_buffer( d, "The game is newlocked.\n\r", 0 );
						account_main_menu( d );
						d->connected = CON_ACCOUNT;
					}

					if (check_ban(d->host,BAN_NEWBIES))
					{
						write_to_buffer(d,
						"New players are not allowed from your site.\n\r",0);
						d->connected = CON_ACCOUNT;
						return;
					}

					{
						FILE * ifp;
						char ibuff[1000];

						/* need this next line to malloc the space for pcdata */
						sprintf(ibuff, "%s%s", PLAYER_DIR, capitalize(argument));
						if((ifp = fopen( ibuff, "r")))
						{
							write_to_buffer(d, "This name already exists, please choose another.  Name: ", 0);
							fclose(ifp);
							return;
						}

						sprintf(ibuff, "%s%s", UNREGISTER, capitalize(argument));
						if((ifp = fopen( ibuff, "r")))
						{
							write_to_buffer(d, "This name exists on an unregistered character.\n\r", 0);
							fclose(ifp);
							account_main_menu(d);
							d->connected = CON_ACCOUNT;
							return;
						}
					}
			fOld = load_char_obj(d, argument);
			sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
			free_string(d->character->name);
			d->character->name = str_dup(capitalize(argument));
			write_to_buffer( d, buf, 0 );
			d->connected = CON_CONFIRM_NEW_NAME;			
			break;   

		case CON_CONFIRM_NEW_NAME:
            switch (argument[0])
            {
                case 'y':
                case 'Y':				
                    sprintf (buf, "We have no record of this hero, creating a new file for %s.\r\n\r\n", ch->name);
					
					for(i=0;i<MAX_ACCOUNT_CHAR; i++)
					{
						if(IS_NULLSTR(d->account->char_list[i].char_name))
						{
							/* Save the name in an account slot so we save the data when we save */
							free_string(d->account->char_list[d->account->numb].char_name);
							d->account->char_list[d->account->numb].char_name = str_dup(ch->name);					
							break;
						}
					}
					free_string(d->character->account);
					d->character->account = str_dup(d->account->name);													
					
					
					if (ch->desc->color)
						SET_BIT (ch->act, PLR_COLOUR);

					STD (buf, d);
					//d->connected = CON_GET_NEW_PASSWORD;
					WTB (d, "\r\n", 0);            
					STD ("What is your gender: (M)ale/(F)emale)?\r\n", d);
					d->connected = CON_GET_NEW_SEX;					
                    break;

                case 'n':
                case 'N':
                    STD ("Ok, what IS it, then? ", d);
                    free_char (d->character);
                    d->character = NULL;
                    d->connected = CON_GET_NAME;
                    break;

                default:
					STD ("Is this correct?\r\n",d);
                    STD ("Please type {r({xY{r){xes or {r({xN{r){xo? ", d);
                    break;
            }
            break;

        case CON_GET_NEW_SEX:
            switch (argument[0])
            {
                case 'm':
                case 'M':
                    ch->sex = SEX_MALE;
                    ch->pcdata->true_sex = SEX_MALE;
					STD ("\r\n",d);
//Alert Immortals of new players:					
					sprintf (log_buf, "%s@%s new player.%c", ch->name, d->host, 007);
					log_string (log_buf);
					wiznet ("Newbie alert!  $N sighted.", ch, NULL, WIZ_NEWBIE, 0, 0);
					wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (ch));										
					WTB (d, "\r\n", 0);		
					/*if (d->account)
					{
						if (++d->account->char_count <= MAX_CHAR_ACCOUNT)
						{
							d->account->characters[d->account->char_count - 1] = str_dup(ch->name);
						}
					}*/
					d->connected = CON_DECIDE_STATS;		
                    break;
                case 'f':
                case 'F':
                    ch->sex = SEX_FEMALE;
                    ch->pcdata->true_sex = SEX_FEMALE;
					STD ("\r\n",d);					
					sprintf (log_buf, "%s@%s new player.%c", ch->name, d->host, 007);
					log_string (log_buf);
					wiznet ("Newbie alert!  $N sighted.", ch, NULL, WIZ_NEWBIE, 0, 0);
					wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, get_trust (ch));										
					WTB (d, "\r\n", 0);		
					/*if (d->account)
					{
						if (++d->account->char_count <= MAX_CHAR_ACCOUNT)
						{
							d->account->characters[d->account->char_count - 1] = str_dup(ch->name);
						}	
					}*/
					d->connected = CON_DECIDE_STATS;		
                    break;
                default:
                    STD ("\r\nThat's not a gender.\r\nWhat IS your gender? ",
                                  d);
                    return;
            }
			case CON_DECIDE_STATS:
			{
				switch(argument[0])
				{
					case 'r':
					case 'R':
						
						ch->perm_stat[STAT_STR] = roll_stat(ch,STAT_STR);
						ch->perm_stat[STAT_INT] = roll_stat(ch,STAT_INT);
						ch->perm_stat[STAT_WIS] = roll_stat(ch,STAT_WIS);
						ch->perm_stat[STAT_DEX] = roll_stat(ch,STAT_DEX);
						ch->perm_stat[STAT_CON] = roll_stat(ch,STAT_CON);
						ch->perm_stat[STAT_CHA] = roll_stat(ch,STAT_CHA);
						sprintf( buf, "[ Str: {r({x%d{r){x  Int: {r({x%d{r){x  Wis: {r({x%d{r){x  Dex: {r({x%d{r){x  Con: {r({x%d{r){x  Cha: {r({x%d{r){x ]",
						ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
						ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
						ch->perm_stat[STAT_CON], ch->perm_stat[STAT_CHA] );
						STD( buf, d );						
						STD( "\r\n\r\nKeep? (Y)es or (N)o? ", d );
						d->connected = CON_ROLL_STATS;		
						return;
					
					case 't':
					case 'T':						
						
						for (z = 0; z <= MAX_STATS; z++)
						{			
							roll = number_percent();
							stat = 13;
							if (roll < 20)
								stat++;
							if (roll < 5)
								stat++;
							if (roll == 1)
								stat++;
							if (roll > 60)
								stat--;
							if (roll > 80)
								stat--;
							if (roll > 95)
								stat--;
								
							ch->perm_stat[z] = number_range(stat-1,stat+2);													
						}
						STD ("\r\n\r\nYou choose a template character.\r\n\r\n", d);
						STD ("The following races are available:\r\n", d);
						for (race = 1; race_table[race].name != NULL; race++)
						{
							if (!race_table[race].pc_race)
								break;			
							sprintf (buf, "{r[{x %9s {r]{x %s\r\n", race_table[race].name, pc_race_table[race].short_description);
							STD(buf,d);                    
						}
						WTB (d, "\r\n", 0);
						STD ("What will be your character's race? (type {Bhelp{x <race name> for more information)\r\n", d);
						STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
						d->connected = CON_GET_NEW_RACE;
						return;            									
				}
				STD ("You must choose to (R)oll your stats or take a (T)emplate.\r\n",d);
				d->connected = CON_DECIDE_STATS;
				break;
			}
			case CON_ROLL_STATS:												
				switch (argument[0] )
				{
				case 'y': case 'Y':								
					STD ("\r\n\r\nYou choose to keep these rolls.\r\n\r\n", d);
					STD ("The following races are available:\r\n", d);
					for (race = 1; race_table[race].name != NULL; race++)
					{
						if (!race_table[race].pc_race)
							break;			
						sprintf (buf, "{r[{x %9s {r]{x %s\r\n", race_table[race].name, pc_race_table[race].short_description);
						STD(buf,d);                    
					}
					WTB (d, "\r\n", 0);
					STD ("What will be your character's race? (type {Bhelp{x <race name> for more information)\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);						
					d->connected = CON_GET_NEW_RACE;
					break;            
					
				case 'n': case 'N':					
					WTB (d, "\r\n\r\n", 0);
					ch->perm_stat[STAT_STR] = roll_stat(ch,STAT_STR);
					ch->perm_stat[STAT_INT] = roll_stat(ch,STAT_INT);
					ch->perm_stat[STAT_WIS] = roll_stat(ch,STAT_WIS);
					ch->perm_stat[STAT_DEX] = roll_stat(ch,STAT_DEX);
					ch->perm_stat[STAT_CON] = roll_stat(ch,STAT_CON);
					ch->perm_stat[STAT_CHA] = roll_stat(ch,STAT_CHA);
					sprintf( buf, "[ Str: {r({x%d{r){x  Int: {r({x%d{r){x  Wis: {r({x%d{r){x  Dex: {r({x%d{r){x  Con: {r({x%d{r){x  Cha: {r({x%d{r){x ]",
					ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
					ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
					ch->perm_stat[STAT_CON], ch->perm_stat[STAT_CHA] );
					STD( buf, d );					
					STD( "\r\nKeep? (Y)es or (N)o? ", d );
					return;
				default: 	
					STD( "\r\nKeep? (Y)es or (N)o? ", d );
					return;
				}

				
			
			case CON_GET_NEW_RACE:											
				one_argument(argument,arg);								
				
				if (!strcmp (arg, "help"))
				{					
					argument = one_argument(argument, arg);
					if (argument[0] == '\0')
						do_function (ch, &do_help, "race help");
					else
						do_function (ch, &do_help, argument);
					STD
						("\r\nWhat is your race? ({Bhelp{x <racename> for more information)\r\n\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					break;
				}
				
				if (argument[0] == 'b' || argument[0] == 'B')
				{
					STD ("Returning to previous menu. (press Enter/Return)\r\n", d);
					d->connected = CON_DECIDE_STATS;
					break;
				}
				
				race = race_lookup (argument);

				if ((!race_table[race].pc_race && !ch->race) || race < 1)
				{                
					STD ("\r\nThat is not a valid race.\r\n", d);
					
					STD ("\r\nThe following races are available:\r\n", d);
					for (race = 1; race_table[race].name != NULL; race++)
					{
						if (!race_table[race].pc_race)
							break;			
						sprintf (buf, "{r[{x %9s {r]{x %s\r\n", race_table[race].name, pc_race_table[race].short_description);
						STD(buf,d);                    
					}
					WTB (d, "\r\n", 0);
					STD ("What will be your character's race? (Type {Bhelp{x <race name> for more information)\r\n", d);	
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					
					d->connected = CON_GET_NEW_RACE;
					return;
				}
				else
				{
					if (race && race_table[race].pc_race)
					{
						sprintf(buf, "\r\nYou have chosen to be a %s.\r\n", pc_race_table[race].name);
						ch->race = race;          
						STD(buf,d);					
						//racial stat modifiers:
						if (IS_ELF(ch) || IS_DROW(ch))
						{
							ch->perm_stat[STAT_CON] --;
							ch->perm_stat[STAT_WIS] ++;							
						}
						if (IS_DWARF(ch))
						{
							ch->perm_stat[STAT_CHA] -=2;
							ch->perm_stat[STAT_CON] ++;
							ch->perm_stat[STAT_STR] ++;
						}
						if (IS_HALFLING(ch))
						{
							ch->perm_stat[STAT_STR] --;
							ch->perm_stat[STAT_DEX] +=2;
							ch->perm_stat[STAT_WIS] --;
						}
						if (IS_GNOME(ch))
						{
							ch->perm_stat[STAT_WIS] --;
							ch->perm_stat[STAT_INT] +=2;						
							ch->perm_stat[STAT_DEX] --;
						}
						if (IS_ORC(ch))
						{
							ch->perm_stat[STAT_CHA] --;
							ch->perm_stat[STAT_WIS] --;
							ch->perm_stat[STAT_STR] ++;
							ch->perm_stat[STAT_CON] ++;
						}
						if (IS_HALF_OGRE(ch))
						{
							ch->perm_stat[STAT_STR] +=2;
							ch->perm_stat[STAT_CON] +=2;
							ch->perm_stat[STAT_WIS] --;
							ch->perm_stat[STAT_INT] -=2;
							ch->perm_stat[STAT_CHA] --;
						}
						if (IS_GNOLL(ch))
						{
							ch->perm_stat[STAT_STR] +=2;
							ch->perm_stat[STAT_CON] ++;
							ch->perm_stat[STAT_CHA] --;
							ch->perm_stat[STAT_INT] -=2;
						}
						if (IS_MINOTAUR(ch))
						{
							ch->perm_stat[STAT_STR] +=2;
							ch->perm_stat[STAT_CON] ++;
							ch->perm_stat[STAT_CHA] -=2;
							ch->perm_stat[STAT_INT] --;
						}
						if (IS_BIRDFOLK(ch))
						{
							ch->perm_stat[STAT_DEX] ++;
							ch->perm_stat[STAT_STR] ++;
							ch->perm_stat[STAT_CHA] -=1;							
						}
			
						ch->affected_by = ch->affected_by | race_table[race].aff;
						ch->imm_flags = ch->imm_flags | race_table[race].imm;
						ch->res_flags = ch->res_flags | race_table[race].res;
						ch->vuln_flags = ch->vuln_flags | race_table[race].vuln;
						ch->form = race_table[race].form;
						ch->parts = race_table[race].parts;
				
				
						/* add skills */
						for (i = 0; i < 5; i++)
						{
							if (pc_race_table[race].skills[i] == NULL)
								break;
							group_add (ch, pc_race_table[race].skills[i], FALSE);
						}
						ch->size = pc_race_table[race].size;						

						WTB(d, "\r\nAvailable eye colors:\r\n",0);
						
						for (y = 0; y < MAX_EYE_COLOR; y++)
						{
							sprintf (buf, "{r[{x %8s {r]{x\r\n", eye_color_flags[y].name);
							STD(buf,d);                    						
						}						
						
						WTB(d, "\r\nWhat color eyes will this character have?\r\n", 0);
						STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
						d->connected = CON_GET_EYES;
						break;
					}
					else
					{
						return;
					}
				}				
		
		case CON_GET_EYES:	
			
			one_argument(argument,arg);								
			
			if (!strcmp (arg, "help"))
			{					
				argument = one_argument(argument, arg);
				if (argument[0] == '\0')
					do_function (ch, &do_help, "help");
				else
					do_function (ch, &do_help, argument);
				STD
					("\r\nWhat is your eye color? (help for more information)\r\n\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					
				break;
			}			
			
			if (!str_cmp(argument, "back") || !str_cmp(argument, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				d->connected = CON_GET_NEW_RACE;
				break;
			}
			
			if ((value = flag_value (eye_color_flags, argument)) == NO_FLAG)
			{
				for (y = 0; y < MAX_EYE_COLOR; y++)
				{
					sprintf (buf, "{r[{x %8s {r]{x\r\n", eye_color_flags[y].name);
					STD(buf,d);                    						
				}
				STD
					("\r\nWhat is your eye color? (help for more information)\r\n\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;				
			}	
				
			TOGGLE_BIT (ch->eyes, value);	
			
			SEND("\r\nAvailable hair styles:\r\n",ch);
			
			if (ch->sex == SEX_MALE)
			{
				for (y = 0; y < MAX_MALE_HAIRSTYLES; y++)
				{
					sprintf (buf, "{r[{x %8s {r]{x\r\n", haircut_flags[y].name);
					STD(buf,d);                    						
				}						
			}
			else
			{
				for (y = 6; y < (MAX_FEMALE_HAIRSTYLES + MAX_MALE_HAIRSTYLES - 1); y++)
				{
					sprintf (buf, "{r[{x %8s {r]{x\r\n", haircut_flags[y].name);
					STD(buf,d);                    						
				}
			}
			
			SEND ("\r\nEye color selected, now choose your hair style.\r\n", ch);
			STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
			d->connected = CON_GET_HAIRSTYLE;
			break;
			
		case CON_GET_HAIR:
		
			one_argument(argument,arg);								
			
			if (!strcmp (arg, "help"))
			{					
				argument = one_argument(argument, arg);
				if (argument[0] == '\0')
					do_function (ch, &do_help, "help");
				else
					do_function (ch, &do_help, argument);
				STD
					("\r\nWhat is your hair color? (help for more information)\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n\r\n", d);
				break;
			}
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				d->connected = CON_GET_HAIRSTYLE;
				break;
			}			
			
			if ((value = flag_value (hair_color_flags, argument)) == NO_FLAG)
			{
				for (y = 0; y < MAX_HAIR_COLOR; y++)
				{
					sprintf (buf, "{r[{x %8s {r]{x\r\n", hair_color_flags[y].name);
					STD(buf,d);                    						
				}
				STD
					("\r\nWhat is your hair color? (help for more information)\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n\r\n", d);
				break;				
			}	
				
			TOGGLE_BIT (ch->hair, value);	

			
			
			if (ch->sex == SEX_MALE || IS_DWARF(ch))
			{			
				SEND ("\r\nAvailable facial hair:\r\n",ch);
				for (y = 0; y < MAX_FACIAL_HAIR; y++)
				{
					sprintf (buf, "{r[{x %12s {r]{x\r\n", facial_hair_flags[y].name);
					STD(buf,d);                    						
				}
				SEND ("\r\nHair color selected, now choose your facial hair.\r\n", ch);	
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_GET_FACIAL_HAIR;
				break;	
			}
			else
			{
				show_ch_class_options(d);
				SEND("\r\nHair color selected, now choose your class.\r\n",ch);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_GET_NEW_CLASS;
				break;
			}
			

		case CON_GET_HAIRSTYLE:
			
			one_argument(argument,arg);								
			
			if (!strcmp (arg, "help"))
			{					
				argument = one_argument(argument, arg);
				if (argument[0] == '\0')
					do_function (ch, &do_help, "help");
				else
					do_function (ch, &do_help, argument);
				STD
					("\r\nWhat is your hair style? ({Bhelp{x for more information)\r\n\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				d->connected = CON_GET_EYES;
				break;
			}			
			
			if ((value = flag_value (haircut_flags, argument)) == NO_FLAG)
			{
				if (ch->sex == SEX_MALE)
				{
					for (y = 0; y < MAX_MALE_HAIRSTYLES; y++)
					{
						sprintf (buf, "{r[{x %8s {r]{x\r\n", haircut_flags[y].name);
						STD(buf,d);                    						
					}
					STD
						("\r\nWhat is your hair style? (help for more information)\r\n\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					break;				
				}
				else
				{
					for (y = 8; y < (MAX_FEMALE_HAIRSTYLES + MAX_MALE_HAIRSTYLES - 1); y++)
					{
						sprintf (buf, "{r[{x %8s {r]{x\r\n", haircut_flags[y].name);
						STD(buf,d);                    						
					}
					STD
						("\r\nWhat is your hair style? (help for more information)\r\n\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					break;				
				}
			}	
				
			TOGGLE_BIT (ch->haircut, value);	
			if (!IS_SET(ch->haircut, STYLE_BALD))
			{				
				SEND("\r\nAvailable hair colors:\r\n",ch);
		
				for (y = 0; y < MAX_HAIR_COLOR; y++)
				{
					sprintf (buf, "{r[{x %8s {r]{x\r\n", hair_color_flags[y].name);
					STD(buf,d);                    						
				}
				STD ("\r\nHair style selected, now choose your hair color.\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_GET_HAIR;
				break;
			}
			else
			{
				if (ch->sex == SEX_MALE || IS_DWARF(ch))
				{			
					SEND ("\r\nAvailable facial hair:\r\n",ch);
					for (y = 0; y < MAX_FACIAL_HAIR; y++)
					{
						sprintf (buf, "{r[{x %12s {r]{x\r\n", facial_hair_flags[y].name);
						STD(buf,d);                    						
					}
					SEND ("\r\nHair style selected, now choose your facial hair.\r\n", ch);	
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					d->connected = CON_GET_FACIAL_HAIR;
					break;	
				}
				else
				{
					show_ch_class_options(d);
					SEND("\r\nHair style selected, now choose your class.\r\n",ch);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					d->connected = CON_GET_NEW_CLASS;
					break;
				}
			}			
			
		case CON_GET_FACIAL_HAIR:
			one_argument(argument,arg);								
			
			if (!strcmp (arg, "help"))
			{					
				argument = one_argument(argument, arg);
				if (argument[0] == '\0')
					do_function (ch, &do_help, "help");
				else
					do_function (ch, &do_help, argument);
				STD
					("\r\nWhat type of facial hair will you have? (help for more information)\r\n\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				d->connected = CON_GET_HAIR;
				break;
			}			
			
			if ((value = flag_value (facial_hair_flags, argument)) == NO_FLAG)
			{
				for (y = 0; y < MAX_FACIAL_HAIR; y++)
				{
					sprintf (buf, "{r[{x %12s {r]{x\r\n", facial_hair_flags[y].name);
					STD(buf,d);                    						
				}
				STD
					("\r\nWhat kind of facial hair will you have? (help for more information)\r\n\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;				
			}	
				
			TOGGLE_BIT (ch->facial_hair, value);				
						
			show_ch_class_options(d);
			STD ("\r\nYou have chosen your facial hair, now choose your class.\r\n", d);
			STD ("Type {Bhelp{x <classname> for information on each class.\r\n", d);
			STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
			d->connected = CON_GET_NEW_CLASS;
			break;
		
		case CON_GET_FAVORED_ENEMY:
			
			if (!strcmp (argument, "list"))
			{
				do_function (ch, &do_help, "favored");
				WTB(d, "\r\nWhat will be your favored enemy race? (list for options)\r\n", 0);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				show_ch_class_options(d);
				d->connected = CON_GET_NEW_CLASS;
				break;
			}			
			
			if ((favored = race_lookup (argument)) != 0)
			{
				sprintf(buf, "\r\nYou have chosen your favored enemy to be the {B%s{x.\r\n", race_table[favored].name);
				STD(buf, d);
				ch->favored_enemy = favored;
				WTB (d, "\r\nNow you must pick your alignment.\r\n", 0);
				WTB (d, "\r\n\r\n[Hit enter to continue]\r\n\r\n", 0);
				d->connected = CON_GET_ALIGNMENT;
				break;
			}
			else
			{
				STD ("\r\nNot a valid favored enemy race. ({Blist{x for options)\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}
		
			
		case CON_GET_NEW_CLASS:
            one_argument(argument,arg);								
				
			if (!strcmp (arg, "help"))
			{					
				argument = one_argument(argument, arg);
				if (argument[0] == '\0')
					do_function (ch, &do_help, "class");
				else
					do_function (ch, &do_help, argument);
				STD
					("\r\nWhat is your class? (help for more information)\r\n\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				
				if (ch->sex == SEX_MALE || IS_DWARF(ch))
					d->connected = CON_GET_FACIAL_HAIR;
				else
					d->connected = CON_GET_HAIR;
				break;
			}			
			
			iClass = ch_class_lookup (argument);			
			
            if (iClass == -1)
            {
                //STD ("\r\nThat's not a ch_class.\r\nPlease choose a valid ch_class.\r\n", d);
				STD ("\r\n\r\nThe following classes are available:\r\n",d);					
				for (iClass = 0; iClass < MAX_CLASS; iClass++)
				{
					
					if (!qualify_ch_class(ch, iClass))
						continue;
					if (iClass >= MAX_CLASS)
						break;		
					if ( ch_class_table[iClass].secret == FALSE)
					{
						sprintf (buf, "{r[{x %8s {r]{x\r\n", ch_class_table[iClass].name);
						STD(buf,d);                    						
					}           						
				}					
            					
				STD	("\r\nWhat will be your character's class? (Type {Bhelp{x <class name> for more information)\r\n",d);				
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
                return;
            }
			//Make sure they can BE that class.
			if (!qualify_ch_class(ch, iClass))
			{
				STD ("That's not an available class for you.\r\n",d);
				show_ch_class_options(d);            					
				STD	("\r\nWhat will be your character's class? (Type {Bhelp{x <class name> for more information)\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);				
				return;
			}
			
			//race restrictions.
			if (IS_HALF_OGRE(ch) || IS_DROW(ch) || IS_DUERGAR(ch))
			{
				if (iClass == DRUID) //druid
				{
					STD ("\r\nYour race restricts you from being this class.\r\n",d);
					show_ch_class_options(d);          									
					STD	("\r\nWhat will be your character's class? (Type {Bhelp{x <class name> for more information)\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);					
					return;
				}
			}
            ch-> ch_class = iClass;			
			sprintf(buf, "\r\nYou have chosen to be a %s.\r\n", ch_class_table[iClass].name);
			STD (buf, d);
			STD ("\r\n",d);			
			
			//add class based skills here: - Upro
			group_add (ch, "rom basics", FALSE);
            group_add (ch, ch_class_table[ch-> ch_class].base_group, FALSE);
			
			//check for psionic abilities here - Upro 9/23/2010
			int psionic_chance = 1;
			
			if (GET_CON(ch) >= 17)
				psionic_chance++;
			if (GET_INT(ch) >= 17)
				psionic_chance++;
			if (number_percent() <= psionic_chance )
			{
				ch->has_talent = TRUE;
				ch->max_pp = 10;
				ch->max_pp += wis_app[GET_WIS(ch)].inherent_potential + con_app[GET_CON(ch)].pp_bonus;
				ch->pp = ch->max_pp;
			}

			if (IS_MONK(ch))
				ch->pcdata->learned[gsn_hand_to_hand] = 50;
			
			//Ranger stuff
			if (IS_RANGER(ch))
			{
								
				WTB(d, "\r\nAs a ranger, you must choose your favored enemy.\r\n", 0);
				STD("\r\nType {Blist{x for options or type the race name you choose.\r\n",d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_GET_FAVORED_ENEMY;
				break;
			}
			//give clerics a random utility spell.
			if (IS_CLERIC(ch))
			{
				spellChance = number_range(1,100);
				
				if (spellChance < 30)
					group_add(ch, "create spring", FALSE);
				else if (spellChance > 29 && spellChance < 70)
					group_add(ch, "create food", FALSE);
				else
					group_add(ch, "create water", FALSE);
			}
			
			//wizard stuff
			if (IS_WIZARD(ch))
			{
			
				//combat spell.
				spellChance = number_range(1,100);
				if (spellChance < 25)
				{
					group_add (ch, "chill touch", FALSE);				
				}
				else if (spellChance < 50 && spellChance > 24)
				{
					group_add (ch, "magic missile", FALSE);
				}
				else if (spellChance < 75 && spellChance > 49)
				{
					group_add (ch, "burning hands", FALSE);
				}			
				else
					group_add (ch, "shocking grasp", FALSE);
				
				//utility spell.
				spellChance = number_range(1,100);
				if (spellChance < 25)
				{
					group_add (ch, "copy", FALSE);				
				}
				else if (spellChance < 50 && spellChance > 24)
				{
					group_add (ch, "audible glamor", FALSE);
				}
				else 
				{
					group_add (ch, "knock", FALSE);
				}			
							
				STD("\r\nAs a wizard, you must choose your elemental specialization. (type {Blist{x for options)\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_ELEMENTAL_SPECIALIZATION;
				break;
			}
			//Fighter weapon specialization.
			else if (IS_FIGHTER(ch))
			{
				WTB(d,"\r\n",0);
				STD ("As a fighter, you must choose your weapon specialization. (type {Blist{x for options)\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_WEAPON_SPECIALIZATION;
				break;
			}
			else
			{
				if (IS_PALADIN(ch))
				{
					if (IS_DUERGAR(ch) || IS_DROW(ch) || IS_MINOTAUR(ch))
					{
						STD("\r\nAs a paladin, you must be of lawful evil alignment.\r\n",d);
						ch->alignment = -1000;
					}
					else
					{
						STD("\r\nAs a paladin, you must be of lawful good alignment.\r\n",d);
						ch->alignment = 1000;
					}
					WTB(d,"\r\nThe following boons are available:\r\n",0);
					WTB(d,"(A)mbidexterity -- helps dual wielding, hand parrying, counterstrike, and hand to hand.\r\n",0);
					WTB(d,"Resist (F)ire   -- 25%\r\n",0);
					WTB(d,"Resist (C)old   -- 25%\r\n",0);
					WTB(d,"Resist Shoc(k)  -- 25%\r\n",0);
					WTB(d,"Resist (E)arth  -- 25%\r\n",0);
					WTB(d,"Resist A(i)r    -- 25%\r\n",0);
					WTB(d,"Resist (W)ater  -- 25%\r\n",0);
					WTB(d,"(H)ardiness     -- +1 hp per level.\r\n",0);
					WTB(d,"B(r)illiance    -- +1 mana per level.\r\n",0);
					WTB(d,"(G)unslinger    -- Zap wands twice as fast. Bonus to missile weaponry.\r\n",0);
					WTB(d,"(S)kip          -- Skip picking a boon.\r\n",0);
					
					STD ("Taking a boon will use 3 of your starting 6 trains.\r\n", d);		
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					d->connected = CON_GET_BOON;
					break;
				}
				
				
				
				STD("\r\nNow you must choose your alignment, which you MUST roleplay in game.\r\n\r\n",d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				
				//if (pc_race_table[race].can_good == TRUE && pc_race_table[race].can_neutral == TRUE && pc_race_table[race].can_evil == TRUE)
				
				STD ("You may be good, neutral, or evil.\r\n", d);
				STD ("Which alignment {r({xG{r/{xN{r/{xE{r){x?\r\n", d);
				//WTB(d,"(Press Enter)\r\n",0);
				d->connected = CON_GET_ALIGNMENT;
				break;
			}
			
		case CON_GET_ALIGNMENT:
			switch (argument[0])
			{
				case 'b':
				case 'B':
					STD("Returning to previous menu.\r\n",d);
					show_ch_class_options(d);
					d->connected = CON_GET_NEW_CLASS;
					return;
			
				case 'g':
				case 'G':
					if (/*!pc_race_table[ch->race].can_good || */IS_DROW(ch) || IS_DUERGAR(ch) || IS_MINOTAUR(ch))
					{
						SEND("Your race is inherently not good aligned, choose again.\r\n",ch);
						WTB(d,"(Press Enter)\r\n",0);
						return;
					}
					ch->alignment = 500;
					break;
				case 'n':
				case 'N':
					/*if (!pc_race_table[ch->race].can_neutral)
					{
						SEND("Your race is inherently not neutral, choose again.\r\n",ch);
						WTB(d,"(Press Enter)\r\n",0);
						return;
					}*/
					ch->alignment = 0;
					break;
				case 'e':
				case 'E':
					if (/*!pc_race_table[ch->race].can_evil ||*/ IS_DRUID(ch) || IS_SURFACE_ELF(ch))
					{
						SEND("Your race is inherently not evil aligned, choose again.\r\n",ch);
						WTB(d,"(Press Enter)\r\n",0);
						return;
					}
					ch->alignment = -500;
					break;
				default:
					STD ("You may be good, neutral, or evil.\r\n", d);
					STD ("Which alignment {r({xG{r/{xN{r/{xE{r){x?\r\n", d);
					return;
			}			
			STD("Please choose your ethos (help ethos for more information.)\r\n",d);
			STD("Chaotic, Neutral, or Lawful? {r({xC{r/{xN{r/{xL{r){x\r\n",d);
            //d->connected = CON_GET_GOD;
			d->connected = CON_GET_ETHOS;
			break;
			
		case CON_GET_ETHOS:
			
			switch (argument[0])
			{
				case 'b':
				case 'B':
					STD("Returning to previous menu.\r\n",d);
					STD("\r\nNow you must choose your alignment, which you MUST roleplay in game.\r\n\r\n",d);								
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					STD ("You may be good, neutral, or evil.\r\n", d);
					STD ("Which alignment {r({xG{r/{xN{r/{xE{r){x?\r\n", d);
					d->connected = CON_GET_ALIGNMENT;
					return;
			
				case 'c':
				case 'C':
					if (ch->alignment == 500)
					{
						ch->alignment = ALIGN_CG;	
						break;					
					}
					else if (ch->alignment == -500)
					{
						ch->alignment = ALIGN_CE;
						break;
					}
					else
					{
						ch->alignment = ALIGN_CN;
						break;
					}
					break;
				case 'n':
				case 'N':
					break;
				case 'l':
				case 'L':
					if (ch->alignment == 500)
					{
						ch->alignment = ALIGN_LG;	
						break;					
					}
					else if (ch->alignment == -500)
					{
						ch->alignment = ALIGN_LE;
						break;
					}
					else
					{
						ch->alignment = ALIGN_LN;
						break;
					}
					break;
				default:
					STD ("You may be chaotic, neutral, or lawful.\r\n", d);
					STD ("Which ethos {r({xC{r/{xN{r/{xL{r){x?\r\n", d);
					return;
			}
			
			WTB(d,"\r\nThe following boons are available:\r\n",0);
			WTB(d,"(A)mbidexterity -- helps dual wielding, hand parrying, counterstrike, and hand to hand.\r\n",0);
			WTB(d,"Resist (F)ire   -- 25%\r\n",0);
			WTB(d,"Resist (C)old   -- 25%\r\n",0);
			WTB(d,"Resist Shoc(k)  -- 25%\r\n",0);
			WTB(d,"Resist (E)arth  -- 25%\r\n",0);
			WTB(d,"Resist A(i)r    -- 25%\r\n",0);
			WTB(d,"Resist (W)ater  -- 25%\r\n",0);
			WTB(d,"(H)ardiness     -- +1 hp per level.\r\n",0);
			WTB(d,"B(r)illiance    -- +1 mana per level.\r\n",0);
			WTB(d,"(G)unslinger    -- Zap wands twice as fast. Bonus to missile weaponry.\r\n",0);
			WTB(d,"(S)kip          -- Skip picking a boon.\r\n",0);

			STD ("Taking a boon will use 3 of your starting 6 trains.\r\n", d);		
			STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);			
			d->connected = CON_GET_BOON;
			break;
			
		case CON_GET_BOON:				
			switch (argument[0])
			{
				case '\0':
					d->connected = CON_GET_BOON;
					return;
				case 'b':
				case 'B':
					STD("Returning to previous menu.\r\n",d);
					STD("Please choose your ethos (help ethos for more information.)\r\n",d);
					STD("Chaotic, Neutral, or Lawful? {r({xC{r/{xN{r/{xL{r){x\r\n",d);
					d->connected = CON_GET_ETHOS;
					return;
				
				case 'l':
				case 'L':
					WTB(d,"\r\nThe following boons are available:\r\n",0);
					WTB(d,"(A)mbidexterity -- helps dual wielding, hand parrying, counterstrike, and hand to hand.\r\n",0);
					WTB(d,"Resist (F)ire   -- 25%\r\n",0);
					WTB(d,"Resist (C)old   -- 25%\r\n",0);
					WTB(d,"Resist Shoc(k)  -- 25%\r\n",0);
					WTB(d,"Resist (E)arth  -- 25%\r\n",0);
					WTB(d,"Resist A(i)r    -- 25%\r\n",0);
					WTB(d,"Resist (W)ater  -- 25%\r\n",0);
					WTB(d,"(H)ardiness     -- +1 hp per level.\r\n",0);
					WTB(d,"B(r)illiance    -- +1 mana per level.\r\n",0);
					WTB(d,"(G)unslinger    -- Zap wands twice as fast. Bonus to missile weaponry.\r\n",0);
					WTB(d,"(S)kip          -- Skip picking a boon.\r\n",0);
					
					STD ("Taking a boon will use 3 of your starting 6 trains.\r\n", d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					return;
					
				case 's':
				case 'S':
					break;
				
				case 'a':
				case 'A':
					STD("\r\nYou have chosen ambidexterity for your boon.\r\n\r\n",d);
					ch->boon = BOON_AMBI;
					ch->train -= 3;
					break;
				
				case 'f':
				case 'F':
					STD("\r\nYou have chosen resistance to fire for your boon.\r\n\r\n",d);
					ch->boon = BOON_RES_FIRE;
					ch->train -= 3;
					break;
				
				case 'c':
				case 'C':
					STD("\r\nYou have chosen resistance to cold for your boon.\r\n\r\n",d);
					ch->boon = BOON_RES_COLD;
					ch->train -= 3;
					break;				
				
				case 'k':
				case 'K':
					STD("\r\nYou have chosen resistance to shocking/electrocution for your boon.\r\n\r\n",d);
					ch->boon = BOON_RES_SHOCK;
					ch->train -= 3;
					break;
				
				case 'e':
				case 'E':
					STD("\r\nYou have chosen resistance to earth for your boon.\r\n\r\n",d);
					ch->boon = BOON_RES_EARTH;
					ch->train -= 3;
					break;
				
				case 'i':
				case 'I':
					STD("\r\nYou have chosen resistance to air for your boon.\r\n\r\n",d);
					ch->boon = BOON_RES_AIR;
					ch->train -= 3;
					break;
				
				case 'w':
				case 'W':
					STD("\r\nYou have chosen resistance to water/drowning for your boon.\r\n\r\n",d);
					ch->boon = BOON_RES_DROWN;
					ch->train -= 3;
					break;
				
				case 'h':
				case 'H':
					STD("\r\nYou have chosen hardiness for your boon.\r\n\r\n",d);
					ch->boon = BOON_HARDINESS;
					ch->train -= 3;
					break;
					
				case 'r':
				case 'R':
					STD("\r\nYou have chosen brilliance for your boon.\r\n\r\n",d);
					ch->boon = BOON_BRILLIANCE;
					ch->train -= 3;
					break;
					
				case 'g':
				case 'G':
					STD("\r\nYou have chosen gunslinger for your boon. PEW! PEW!\r\n\r\n",d);
					ch->boon = BOON_GUNSLINGER;
					ch->train -= 3;
					break;
					
				default:
					STD ("\r\nPlease choose a boon or choose to '{Bskip{X' this process.\r\n",d);
					STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
					STD ("Press Enter/Return to continue.\r\n", d);
					d->connected = CON_GET_BOON;
					break;
					
			}
		
			WTB(d,"\r\nThe following gods are available:\r\n",0);
			for ( god = 0; god < MAX_GOD; god++ )
			{
				if ((god_table[god].pc_evil && IS_EVIL(ch))
				|| (god_table[god].pc_neutral && IS_NEUTRAL(ch))
				|| (god_table[god].pc_good && IS_GOOD(ch)))
				{
					if((god_table[god].pc_wizard && ch->ch_class == 0)
					|| (god_table[god].pc_cleric && ch->ch_class == 1)
					|| (god_table[god].pc_thief && ch->ch_class == 2)
					|| (god_table[god].pc_fighter && ch->ch_class == 3)
					|| (god_table[god].pc_ranger && ch->ch_class == 4)
					|| (god_table[god].pc_druid && ch->ch_class == 5)
					|| (god_table[god].pc_paladin && ch->ch_class == 6)
					|| (god_table[god].pc_bard && ch->ch_class == 7)
					|| (god_table[god].pc_monk && ch->ch_class == 8))
					{
						
						sprintf (buf, "{r[{x %8s {r]{x\r\n", god_table[god].name);
						STD(buf,d);                    						
					}
				}
			}
			STD ("Please choose accordingly:\r\n", d);
			d->connected = CON_GET_GOD;
			break;
			
        case CON_GET_GOD:
					
			one_argument(argument,arg);								
			if (!strcmp (arg, ""))
			{
				STD ("Choose a god that exists within this gameverse please.\r\n\r\n", d);
				STD ("The following gods are available:\n\r", d);
				for ( god = 0; god < MAX_GOD; god++ )
				{
				  if ((god_table[god].pc_evil && IS_EVIL(ch))
				|| (god_table[god].pc_neutral && IS_NEUTRAL(ch))
				|| (god_table[god].pc_good && IS_GOOD(ch)))
					 {
						if((god_table[god].pc_wizard && ch->ch_class == 0)
					    || (god_table[god].pc_cleric && ch->ch_class == 1)
					    || (god_table[god].pc_thief && ch->ch_class == 2)
					    || (god_table[god].pc_fighter && ch->ch_class == 3) 
					    || (god_table[god].pc_ranger && ch->ch_class == 4)
						|| (god_table[god].pc_druid && ch->ch_class == 5)
						|| (god_table[god].pc_paladin && ch->ch_class == 6)
						|| (god_table[god].pc_bard && ch->ch_class == 7)
						|| (god_table[god].pc_monk && ch->ch_class == 8))
					    {
							sprintf (buf, "{r[{x %8s {r]{x\r\n", god_table[god].name);
							STD(buf,d);
					    }
					 }
				}
				
				d->connected = CON_GET_GOD;
				break;				
			}
			else if (!strcmp (arg, "help"))
			{					
				argument = one_argument(argument, arg);
				if (argument[0] == '\0')
					do_function (ch, &do_help, "god");
				else
					do_function (ch, &do_help, argument);
				STD ("\r\nWhich god will you choose to worship? (help for more information)\r\n", d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}			
			else if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				d->connected = CON_GET_ALIGNMENT;
				break;
			}	
			else
			{
				god = god_lookup(argument);

			if ( god == -1
				|| (!god_table[god].pc_evil && IS_EVIL(ch))
				|| (!god_table[god].pc_neutral && IS_NEUTRAL(ch))
				|| (!god_table[god].pc_good && IS_GOOD(ch))
				|| (!god_table[god].pc_wizard && ch->ch_class == 0)
				|| (!god_table[god].pc_cleric && ch->ch_class == 1)
				|| (!god_table[god].pc_thief && ch->ch_class == 2)
				|| (!god_table[god].pc_fighter && ch->ch_class == 3)
				|| (!god_table[god].pc_ranger && ch->ch_class == 4)
				|| (!god_table[god].pc_druid && ch->ch_class == 5)
				|| (!god_table[god].pc_paladin && ch->ch_class == 6)
				|| (!god_table[god].pc_bard && ch->ch_class == 7)
				|| (!god_table[god].pc_monk && ch->ch_class == 8)
				)		
			{
				WTB(d,"That is not a valid god.\n\r",0);
				WTB(d,"The following gods are available:\n\r",0);
				for ( god = 0; god < MAX_GOD; god++ )
				{
				  if ((god_table[god].pc_evil && IS_EVIL(ch))
				|| (god_table[god].pc_neutral && IS_NEUTRAL(ch))
				|| (god_table[god].pc_good && IS_GOOD(ch)))
					 {
						if((god_table[god].pc_wizard && ch->ch_class == 0)
					    || (god_table[god].pc_cleric && ch->ch_class == 1)
					    || (god_table[god].pc_thief && ch->ch_class == 2)
					    || (god_table[god].pc_fighter && ch->ch_class == 3) 
					    || (god_table[god].pc_ranger && ch->ch_class == 4)
						|| (god_table[god].pc_druid && ch->ch_class == 5)
						|| (god_table[god].pc_paladin && ch->ch_class == 6)
						|| (god_table[god].pc_bard && ch->ch_class == 7)
						|| (god_table[god].pc_monk && ch->ch_class == 8))
					    {
							sprintf (buf, "{r[{x %8s {r]{x\r\n", god_table[god].name);
							STD(buf,d);
					    }
					 }
				}
				WTB(d,"\r\n",0);
				STD ("Who do you want to worship? (Type {Bhelp{x <god name> for more information)\r\n",d);
				STD ("You can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				d->connected = CON_GET_GOD;
				break;
			 }

			ch->god = god;
			sprintf(buf, "\r\nYou have chosen to follow %s.\r\n\r\n",god_table[ch->god].name);
			SEND (buf, ch);
			WTB(d,"\r\nHit Enter to continue to the game.",0);
			//End char creation:		
			do_function (ch, &do_help, "motd");
			d->connected = CON_READ_MOTD;
			break;				
		}
						
			
		/*case CON_GET_EMAIL:
			
			one_argument(argument,arg);
			
			if (argument[0] == '\0')
			{
				SEND("\r\n\r\nYou have chosen to skip recording your email address.\r\n",ch);
				//End char creation:		
				do_function (ch, &do_help, "motd");
				d->connected = CON_READ_MOTD;
				break;
			}
			else if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				WTB(d,"\r\nThe following gods are available:\r\n",0);
				for ( god = 0; god < MAX_GOD; god++ )
				{
				if ((god_table[god].pc_evil && IS_EVIL(ch))
				|| (god_table[god].pc_neutral && IS_NEUTRAL(ch))
				|| (god_table[god].pc_good && IS_GOOD(ch)))
				{
					if((god_table[god].pc_wizard && ch-> ch_class == 0)
					|| (god_table[god].pc_cleric && ch-> ch_class == 1)
					|| (god_table[god].pc_thief && ch-> ch_class == 2)
					|| (god_table[god].pc_fighter && ch-> ch_class == 3)
					|| (god_table[god].pc_ranger && ch-> ch_class == 4)
					|| (god_table[god].pc_druid && ch-> ch_class == 5)
					|| (god_table[god].pc_paladin && ch-> ch_class == 6)
					|| (god_table[god].pc_bard && ch-> ch_class == 7)
					|| (god_table[god].pc_monk && ch-> ch_class == 8))
					{
						
						sprintf (buf, "{r[{x %8s {r]{x\r\n", god_table[god].name);
						STD(buf,d);                    						
					}
				}
			}
				
				d->connected = CON_GET_GOD;
				break;
			}
			else
			{
				SEND ("\r\n\r\nYour email address has been recorded.\r\n",ch);					
				free_string(d->character->email);
				d->character->email = str_dup(argument);
				//End char creation:		
				do_function (ch, &do_help, "motd");
				d->connected = CON_READ_MOTD;
				break;
			}
			*/
	//Weapon spec.... upro 10/2009
		case CON_WEAPON_SPECIALIZATION:
			WTB(d,"\r\n",1);
			one_argument(argument,arg);
 

			if( argument[0] == '\0' )
			{
				SEND( "You must choose the weapon that you wish to specialize with.\r\n",ch);
				SEND( "Choose again :: ", ch );
				d->connected = CON_WEAPON_SPECIALIZATION;
				break;
			}
			//weapon spec list
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				show_ch_class_options(d);
				d->connected = CON_GET_NEW_CLASS;
				break;
			}			
			
			if (!strcmp(arg,"list") || !strcmp(arg,"li"))
			{
				i = 0;
				while(wpnSpecTable[++i].name != NULL)
					PRINTF(ch,"%15s%s",wpnSpecTable[i].name,i % 3 == 0 ? "\r\n" : "");
				WTB(d,"\r\nWhich weapon will you choose to have specialization with?\r\n",0);
				d->connected = CON_WEAPON_SPECIALIZATION;
				break;
			}


			if (!strcmp(arg,"help"))
			{
				argument = one_argument(argument,arg);
				WTB(d,"\r\nWhich weapon will you choose to have specialization with?\r\n",0);
				d->connected = CON_WEAPON_SPECIALIZATION;
				break;
			}

			wpn = wpnSpecLookup(argument);

			if(wpn == -1 || wpn == 0){
				SEND("That was not a valid choice of a weapon.\r\n",ch);
				SEND("Available choices:\r\n",ch);
				i = 0;
				while(wpnSpecTable[++i].name != NULL)
					PRINTF(ch,"%15s%s",wpnSpecTable[i].name,i % 3 == 0 ? "\r\n" : "");
				break;
			}

			PRINTF(ch,"\r\nYour weapon specialization is now in %ss.\r\n",wpnSpecTable[wpn].name);
			ch->pcdata->weaponSpec = wpn;			
			
			int x;
			int sn;
			for (x = 1; weapon_table[x].name != NULL; x++)
			{
				if (wpnSpecTable[ch->pcdata->weaponSpec].wpnType == weapon_table[x].type)
				{
					sn = *weapon_table[x].gsn;
					ch->pcdata->learned[sn] = 75;
					break;
				}
			}
			
			STD("\r\n\r\nNow you must choose your alignment, which you MUST roleplay in game.\r\n",d);
			
			d->connected = CON_GET_ALIGNMENT;				
			break;
			
	//Elemental specialization (Upro 2007)
	
		case CON_ELEMENTAL_SPECIALIZATION:
			
			WTB(d,"\r\n",1);
			one_argument(argument,arg);
 

			if( argument[0] == '\0' )
			{
				SEND( "You must choose the element that you wish to specialize in.\r\n",ch);
				SEND( "Choose again:\r\n", ch );				
				i = 0;
				while(specTable[++i].name != NULL)
					PRINTF(ch,"{r[{x %15s%s {r]{x",specTable[i].name,i % 3 == 0 ? "\r\n" : "");
				WTB(d,"\r\nWhich element will you choose to have affinity for?\r\n",0);
				d->connected = CON_ELEMENTAL_SPECIALIZATION;
				STD ("\r\nYou can also type {Bback{x at any time to return to the previous menu.\r\n", d);
				break;
			}					
			
			if (!str_cmp(arg, "back") || !str_cmp(arg, "BACK"))
			{
				STD ("Returning to previous menu.\r\n", d);
				show_ch_class_options(d);
				d->connected = CON_GET_NEW_CLASS;
				break;
			}			
			
			if (!strcmp(arg,"list") || !strcmp(arg,"li"))
			{
				i = 0;
				while(specTable[++i].name != NULL)
					PRINTF(ch,"%15s%s",specTable[i].name,i % 3 == 0 ? "\r\n" : "");
				WTB(d,"\r\nWhich element will you choose to have affinity for?\r\n",0);
				d->connected = CON_ELEMENTAL_SPECIALIZATION;
				break;
			}


			if (!strcmp(arg,"help"))
			{
				argument = one_argument(argument,arg);
				WTB(d,"\r\nWhich element will you choose to have affinity for? :: \r\n",0);
				d->connected = CON_ELEMENTAL_SPECIALIZATION;
				break;
			}

			element = specLookup(argument);

			if(element == -1){
				SEND("That was not a valid choice of the elements.\r\n",ch);
				SEND("Available choices:\r\n",ch);
				break;
			}

			PRINTF(ch,"Your elemental affinity is now with %s.\r\n",specTable[element].name);
			ch->pcdata->elementalSpec = element;		   
			
			STD("\r\n\r\nNow you must choose your alignment, which you MUST roleplay in game.\r\n",d);
			//WTB(d,"(Press Enter)\r\n",0);
			d->connected = CON_GET_ALIGNMENT;
			break;

        
			       
        case CON_READ_IMOTD:
            WTB (d, "\r\n", 2);
            do_function (ch, &do_help, "motd");
            d->connected = CON_READ_MOTD;
            break;

		/* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
		/* ch MUST be PC here; have nwrite check for PC status! */
		/*


		
		case CON_NOTE_TO:
			handle_con_note_to (d, argument);
			break;

		case CON_NOTE_SUBJECT:
			handle_con_note_subject (d, argument);
			break;

		case CON_NOTE_EXPIRE:
			handle_con_note_expire (d, argument);
			break;

		case CON_NOTE_TEXT:
			handle_con_note_text (d, argument);			
			break;

		case CON_NOTE_FINISH:
			handle_con_note_finish (d, argument);
			break;

		*/
			
        case CON_READ_MOTD:          

            WTB (d,"Welcome to Aragond : The Chronicles! (based on Rom 2.4/QuickMUD).\r\n",0);
            ch->next = char_list;
            char_list = ch;

            d->connected = CON_PLAYING;			
			reset_char(ch);

			if (ch->pcdata->recall < 1)			
				ch->pcdata->recall = ROOM_VNUM_RECALL;
			
			

			//IF NEW CHARACTER
            if (ch->level == 0)
            {			
				if (d->color)
					SET_BIT (ch->act, PLR_COLOUR);
			
				if(mud_telnetga)
					SET_BIT (ch->comm, COMM_TELNET_GA);
               
				//Set values for new characters.
				
				ch->prompt = str_dup ("(%h/%Hhp %m/%Mm %v/%Vmv) (%gg %ss) (%x/%Xxp) (%t) [ %b ]");
				
                ch->level = 1;
                ch->exp = 0;
				ch->gold = number_range(100,200);
				ch->silver = number_range(100,1000);
                ch->hit = ch->max_hit;
                ch->mana = ch->max_mana;
                ch->move = ch->max_move;
                ch->train += 6;
                ch->practice = 6;
				ch->pkill = 0;
				ch->pdeath = 0;
				ch->mkill = 0;
				ch->pcdata->learned[gsn_first_aid] = 75;
				ch->age = number_fuzzy(pc_race_table[ch->race].base_age);
				
				ch->pcdata->recall = ROOM_VNUM_RECALL;				
				
				OBJ_DATA *potion=NULL;
				OBJ_DATA *bandage = NULL;
				int x=0;
				
				
				//Give 3 health potions and bandages to start.
				for (x = 0; x < 3; x++)
				{
					potion = create_object(get_obj_index(OBJ_VNUM_MINOR_HEALTH_POTION),0);
					obj_to_char(potion,ch);
					
					bandage = create_object(get_obj_index(OBJ_VNUM_ACADEMY_BANDAGE),0);
					obj_to_char(bandage,ch);
				}
				

				SET_BIT (ch->act, PLR_AUTOEXIT);                
				SET_BIT (ch->act, PLR_AUTOASSIST);
				SET_BIT (ch->act, PLR_AUTOGOLD);
				SET_BIT (ch->act, PLR_AUTOMAP);
				//SET_BIT(ch->comm,TICK_SEE);			
				SET_BIT(ch->act,PLR_AUTOLOOT);
                set_title (ch, "the fresh recruit.");

                do_function (ch, &do_outfit, "");               

				save_char_obj(ch);
				save_account_obj(d->account);

                char_to_room (ch, get_room_index (ROOM_VNUM_START));
                SEND ("\r\n", ch);
                do_function (ch, &do_help, "newbie info");
                SEND ("\r\n", ch);
            }
            else if (ch->in_room != NULL)
            {
                char_to_room (ch, ch->in_room);
            }
            else if (IS_IMMORTAL (ch))
            {
                char_to_room (ch, get_room_index (ROOM_VNUM_CHAT));
            }
            else
            {
                char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
            }

            act ("Mystical energy crackles and pops as $n emerges from a w{Da{xv{De{xr{Di{xn{Dg{x portal.", ch, NULL, NULL, TO_ROOM);
            do_function (ch, &do_look, "auto");

            wiznet ("$N has left real life behind. We got another one, cap'n.", ch, NULL,
                    WIZ_LOGINS, WIZ_SITES, get_trust (ch));

            if (ch->pet != NULL)
            {
                char_to_room (ch->pet, ch->in_room);
                act ("Mystical energy crackles and pops as $n emerges from a w{Da{xv{De{xr{Di{xn{Dg{x portal.", ch->pet, NULL, NULL,
                     TO_ROOM);
            }

			SEND("\n", ch);
            do_function (ch, &do_board, "");
            break;
    }

    return;
}