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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"


bool check_social args ((CHAR_DATA * ch, char *command, char *argument));

/*
 * Command logging types.
 */
#define LOG_NORMAL  0
#define LOG_ALWAYS  1
#define LOG_NEVER   2

char    last_command[MAX_STRING_LENGTH];

/*
 * Log-all switch.
 */
bool fLogAll = FALSE;



/*
 * Command table.
 */
const struct cmd_type cmd_table[] = {

	{"get",       do_get,       POS_RESTING,  0,  LOG_NORMAL, 1},
	
    /*
     * Common movement commands.
     */
    {"north", do_north, POS_STANDING, 0, LOG_NEVER, 0, MOVE},
    {"east",  do_east,  POS_STANDING, 0, LOG_NEVER, 0, MOVE},
    {"south", do_south, POS_STANDING, 0, LOG_NEVER, 0, MOVE},
    {"west",  do_west,  POS_STANDING, 0, LOG_NEVER, 0, MOVE},
    {"up",    do_up,    POS_STANDING, 0, LOG_NEVER, 0, MOVE},
    {"down",  do_down,  POS_STANDING, 0, LOG_NEVER, 0, MOVE},
	{"go", 	  do_enter, POS_STANDING, 0, LOG_NORMAL, 0, MOVE},	
	
    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
	{"a",		  do_affects,	POS_DEAD,	  0, LOG_NEVER, 1, INFO}, 
    {"at",        do_at,        POS_DEAD,     L4, LOG_ALWAYS, 1, COMM},
	{"atall",	  do_at_all,	POS_DEAD,	  L3, LOG_ALWAYS, 1, COMM},
	{"rat",		  do_rat,		POS_DEAD,	  L2, LOG_ALWAYS, 1, COMM},
    {"cast",      do_cast,      POS_FIGHTING, 0,  LOG_NORMAL, 1, COMBAT},    
    {"buy",       do_buy,       POS_RESTING,  0,  LOG_NORMAL, 1, OBJ},
    {"channels",  do_channels,  POS_DEAD,     0,  LOG_NORMAL, 1, COMM},
    {"exits",     do_exits,     POS_RESTING,  0,  LOG_NORMAL, 1, INFO},    
    {"goto",      do_goto,      POS_DEAD,     IM, LOG_ALWAYS, 1, MOVE},
    {"group",     do_group,     POS_SLEEPING, 0,  LOG_NORMAL, 1, INFO},
    {"guild",     do_guild,     POS_DEAD,     0, LOG_ALWAYS, 1, COMM},
    {"hit",       do_kill,      POS_FIGHTING, 0,  LOG_NORMAL, 0, COMBAT},
    {"inventory", do_inventory, POS_DEAD,     0,  LOG_NORMAL, 1, INFO},
    {"kill",      do_kill,      POS_FIGHTING, 0,  LOG_NORMAL, 1, COMBAT},
    {"look",      do_look,      POS_RESTING,  0,  LOG_NORMAL, 1, INFO},
	{"land",  	  do_land,  POS_STANDING, 0, LOG_NORMAL, 0, MOVE}, 	//moved here to not
																	//interfere with look
    {"clan",      do_clantalk,  POS_SLEEPING, 0,  LOG_NORMAL, 1, COMM},
    {"music",     do_music,     POS_SLEEPING, 0,  LOG_NORMAL, 1, COMM},
    {"order",     do_order,     POS_RESTING,  0,  LOG_NORMAL, 1, COMBAT},
    {"practice",  do_practice,  POS_SLEEPING, 0,  LOG_NORMAL, 1, INFO},
    {"rest",      do_rest,      POS_SLEEPING, 0,  LOG_NORMAL, 1, MOVE},
    {"sit",       do_sit,       POS_SLEEPING, 0,  LOG_NORMAL, 1, MOVE},    
    {"stand",     do_stand,     POS_SLEEPING, 0,  LOG_NORMAL, 1, MOVE},
    {"tell",      do_tell,      POS_RESTING,  0,  LOG_NORMAL, 1, COMM},
    {"unlock",    do_unlock,    POS_RESTING,  0,  LOG_NORMAL, 1, MOVE},
    {"wield",     do_wear,      POS_RESTING,  0,  LOG_NORMAL, 1, OBJ},
    {"wizhelp",   do_wizhelp,   POS_DEAD,     IM, LOG_NORMAL, 1, INFO},
	{"mclass", 	  do_mclass,	POS_DEAD,	  0,  LOG_NORMAL, 1, CONFIG},
	{"engage",	  do_engage,	POS_FIGHTING,	0, 	LOG_NORMAL, 1, COMBAT},
	{"palmstrike",do_palm_strike, POS_SLEEPING, 0, LOG_NORMAL, 1, COMBAT},
	{"entrance",  do_entrance,	POS_SLEEPING,	0, LOG_NORMAL, 1, CONFIG},
	{"fly",		  do_fly,			POS_STANDING,	0, LOG_NORMAL, 1, COMMON},
	{"peek",	  do_peek,		POS_STANDING,	0, LOG_NORMAL, 1, INFO},
	{"mercy",	  do_mercy,		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMBAT},
	{"logout",	  do_logout,		POS_SLEEPING, 0, LOG_NORMAL, 1, COMM},
    /*
	* Informational commands.
     */	
    {"affects",   do_affects,   POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"areas",     do_areas,     POS_DEAD,     0, LOG_NORMAL, 1, INFO},
	{"bank", 	  do_bank,  	POS_STANDING,	0, LOG_NORMAL, 1, INFO},
	{"vault",	  do_vault,		POS_STANDING,	0, LOG_NORMAL, 1, INFO},
    {"board",     do_board,     POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},
	{"bounty",	  do_bounty,    POS_STANDING,	0, LOG_ALWAYS, 1, INFO},	
	{"census",    do_census,    POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},	
    {"commands",  do_commands,  POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"compare",   do_compare,   POS_RESTING,  0, LOG_NORMAL, 1, INFO},
    {"consider",  do_consider,  POS_RESTING,  0, LOG_NORMAL, 1, INFO},
    {"count",     do_count,     POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},
    {"credits",   do_credits,   POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"equipment", do_equipment, POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"examine",   do_examine,   POS_RESTING,  0, LOG_NORMAL, 1, INFO},
	{"explored",  do_explored,		POS_RESTING,	 0,  LOG_NORMAL, 1, INFO },	
	{"faction",	  do_faction,		POS_DEAD,		0,	LOG_NORMAL, 1, INFO },
	{"finger", 	  do_finger,	POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},
    {"help",      do_help,      POS_DEAD,     0, LOG_NORMAL, 1, INFO},    
	{"info",      do_help,      POS_DEAD,     0, LOG_NORMAL, 1, INFO},    
	{"history",	  do_history,	POS_DEAD,	  0, LOG_NORMAL, 1, INFO},
    {"motd",      do_motd,      POS_DEAD,     0, LOG_NORMAL, 1, INFO},
	{"people",	  do_last,		POS_DEAD,		0,	LOG_NORMAL,	1, INFO },	
	{"profile",   do_finger,	POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},    
	{"read",      do_read,      POS_RESTING,  0, LOG_NORMAL, 1, INFO},
    {"report",    do_report,    POS_RESTING,  0, LOG_NORMAL, 1, INFO},
    {"rules",     do_rules,     POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"score",     do_score,     POS_DEAD,     0, LOG_NORMAL, 1, INFO},    
	{"score2",    do_score2,    POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"skills",    do_skills,    POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"socials",   do_socials,   POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"show",      do_show,      POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"spells",    do_new_spells,POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"story",     do_story,     POS_DEAD,     0, LOG_NORMAL, 1, INFO},
	{"vuln",      do_vuln,		POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},    
	{"resist",    do_vuln,		POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},    
	{"time",      do_time,      POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"typo",      do_typo,      POS_DEAD,     0, LOG_NORMAL, 1, INFO},
	{"unread",	  do_board,		POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},    
	{"weather",   do_weather,   POS_RESTING,  0, LOG_NORMAL, 1, INFO},
    {"who",       do_who,       POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"whois",     do_whois,     POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"wizlist",   do_wizlist,   POS_DEAD,     0, LOG_NORMAL, 1, INFO},
    {"worth",     do_worth,     POS_SLEEPING, 0, LOG_NORMAL, 1, INFO},    
	{"scan",	  do_scan,	    POS_STANDING,	0, LOG_NORMAL, 1, INFO},	
	
    /*
     * Configuration commands.
     */
    {"alia",		do_alia, 		POS_DEAD, 		0, LOG_NORMAL, 0, CONFIG},
    {"alias", 		do_alias, 		POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
    {"autolist", 	do_autolist, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
	{"config",		do_autolist,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"autoall", 	do_autoall, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
    {"autoassist", 	do_autoassist, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
	{"autotrack",	do_autotrack,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"autoexit", 	do_autoexit, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
    {"autogold", 	do_autogold, 	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"autoloot", 	do_autoloot, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
    {"autosac", 	do_autosac, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
    {"autosplit", 	do_autosplit, 	POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
	{"automap",		do_automap,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{"map",			do_automap,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{"autoscroll",	do_autoscroll,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{"autodrink",	do_autodrink,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{"autoeat",		do_autoeat,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{"autosheathe",	do_autosheathe,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},	
    {"brief", 		do_brief, 		POS_DEAD, 		0, LOG_NORMAL, 1, CONFIG},
    {"colour",		do_colour,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"color",		do_colour,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"combine",		do_combine,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"compact",		do_compact,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"description",	do_description,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"delet",		do_delet,		POS_DEAD,		0, LOG_ALWAYS, 0, CONFIG},
    {"delete",		do_delete,		POS_STANDING,	0, LOG_ALWAYS, 1, CONFIG},
    {"nofollow",	do_nofollow,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"noloot",		do_noloot,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"nosummon",	do_nosummon,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"outfit",		do_outfit,		POS_RESTING,	0, LOG_NORMAL, 1, CONFIG},    
    {"prompt",		do_prompt,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"scroll",		do_scroll,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"telnetga",	do_telnetga,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"title",		do_title,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"unalias",		do_unalias,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
    {"wimpy",		do_wimpy,		POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{ "quest",		do_quest,		POS_STANDING,	0, LOG_NORMAL, 1, CONFIG},
	{"lastname",	do_last_name,	POS_DEAD,		0, LOG_NORMAL, 1, CONFIG},
	{ "material",	do_material,	POS_SLEEPING,	0, LOG_NORMAL, 1, CONFIG},
	//{ "mxp",		do_mxp,			POS_RESTING,	0,	LOG_NORMAL, 1, CONFIG},	
	{"noexp",		do_noexp,		POS_SLEEPING,	1, LOG_NORMAL, 1, CONFIG},
	{"condition",	do_condition,	POS_SLEEPING,	1, LOG_NORMAL, 1, CONFIG},
	{"eqcondition", do_eqcondition, POS_SLEEPING,   1, LOG_NORMAL, 1, CONFIG},
	{"timestamp", 	do_timestamp, 	POS_SLEEPING,   1, LOG_NORMAL, 1, CONFIG},
	
    /*
     * Communication commands.
     */
    {"afk", 		do_afk, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {"answer", 		do_answer, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
	{"beeptells",   do_beeptell,	POS_SLEEPING,	0, LOG_NORMAL, 1, COMM}, 
    {"deaf", 		do_deaf, 		POS_DEAD, 		0, LOG_NORMAL, 1, COMM},
    {"emote", 		do_emote, 		POS_RESTING, 	0, LOG_NORMAL, 1, COMM},
    {"pmote", 		do_pmote, 		POS_RESTING, 	0, LOG_NORMAL, 1, COMM},
    {".", 			do_gossip, 		POS_SLEEPING, 	0, LOG_NORMAL, 0, COMM},
    {"gossip", 		do_gossip, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {",", 			do_emote, 		POS_RESTING, 	0, LOG_NORMAL, 0, COMM},
    {"grats", 		do_grats, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {"gtell", 		do_gtell, 		POS_DEAD, 		0, LOG_NORMAL, 1, COMM},
    {";", 			do_gtell, 		POS_DEAD, 		0, LOG_NORMAL, 0, COMM},
    {"note", 		do_nedit, 		POS_DEAD, 		1, LOG_NORMAL, 1, COMM},
    //{"pose", 		do_pose, 		POS_RESTING, 	0, LOG_NORMAL, 1, MOVE},
    {"question", 	do_question, 	POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
	{"?", 			do_question, 	POS_SLEEPING, 	0, LOG_NORMAL, 0, COMM},
    {"quote", 		do_quote, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {"quiet", 		do_quiet, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {"reply", 		do_reply, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {"replay", 		do_replay, 		POS_SLEEPING, 	0, LOG_NORMAL, 1, COMM},
    {"say", 		do_say, 		POS_RESTING, 	0, LOG_NORMAL, 1, COMM},
    {"'", 			do_say, 		POS_RESTING, 	0, LOG_NORMAL, 0, COMM},
    {"shout", 		do_yell, 		POS_RESTING, 	3, LOG_NORMAL, 1, COMM},
    {"yell", 		do_yell, 		POS_RESTING, 	0, LOG_NORMAL, 1, COMM},
	{"ooc",         do_ooc,     	POS_SLEEPING,   1, LOG_NORMAL, 1, COMM},
	{"nudge",		do_nudge,		POS_STANDING,	1, LOG_NORMAL, 1, COMM},	
	//{"tweet",		do_addtwitter,	POS_SLEEPING,   L3,LOG_NORMAL, 1, COMM}, 
	//{"twitter",		do_twitters, 	POS_SLEEPING,	L3,LOG_NORMAL, 1, COMM},
	
	/*
     * Languages.
     */
    { "speak",          do_speak,       POS_RESTING,     0,  LOG_NORMAL },
    { "common",         do_common,      POS_RESTING,     0,  LOG_NORMAL },
    { "dwarvish",       do_dwarvish,    POS_RESTING,     0,  LOG_NORMAL },
    { "elvish",         do_elvish,      POS_RESTING,     0,  LOG_NORMAL },
    { "human",          do_human,       POS_RESTING,     0,  LOG_NORMAL },
    { "gnomish",        do_gnomish,     POS_RESTING,     0,  LOG_NORMAL },
    { "orcish",         do_orcish,      POS_RESTING,     0,  LOG_NORMAL },
    { "goblin",         do_goblin,      POS_RESTING,     0,  LOG_NORMAL },
    { "ogre",           do_ogre,        POS_RESTING,     0,  LOG_NORMAL },
    { "trollish",       do_trollish,    POS_RESTING,     0,  LOG_NORMAL },
    { "kobold",         do_kobold,      POS_RESTING,     0,  LOG_NORMAL },
    { "drow",           do_drow,        POS_RESTING,     0,  LOG_NORMAL },
    { "hobbit",         do_hobbit,      POS_RESTING,     0,  LOG_NORMAL },
    { "langlearn",      do_learnLang,   POS_RESTING,    0,  LOG_ALWAYS },
    { "lset",           do_lset,        POS_DEAD,        L7,  LOG_ALWAYS },
    { "lstat",       	do_lstat,       POS_DEAD,        L7,  LOG_NORMAL },

	
    /*
     * Object manipulation commands.
     */
    {"brandish", 		do_brandish, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"close", 			do_close, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"drink", 			do_drink, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"drop", 			do_drop, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"dump", 			do_obj_dump, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"eat", 			do_eat, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"envenom", 		do_envenom, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"fill", 			do_fill, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"give", 			do_give, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"heal", 			do_heal, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"hold", 			do_wear, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"list", 			do_list, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
	{"wares", 			do_list, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"lock", 			do_lock, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"open", 			do_open, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"pick", 			do_pick, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"pour", 			do_pour, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
	{"imbue",			do_imbue, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
	{"brew",			do_brew,	POS_STANDING, 0, LOG_NORMAL, 1, OBJ},
    {"put", 			do_put, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"quaff", 			do_quaff, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"recite", 			do_recite, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"remove", 			do_remove, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"sell", 			do_sell, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"take", 			do_get, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"sacrifice", 		do_sacrifice, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"junk", 			do_sacrifice, POS_RESTING, 0, LOG_NORMAL, 0, OBJ},  
    {"value", 			do_value, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"wear", 			do_wear, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
    {"zap", 			do_zap, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
	{"forage", 			do_forage, POS_STANDING, 0, LOG_NORMAL, 1, OBJ},
	{"mine", 			do_mine, POS_STANDING, 0, LOG_NORMAL, 1, OBJ},
	{"cook",			do_cook,	POS_STANDING,	0,	LOG_NORMAL,	OBJ},
	{"dig",				do_dig, POS_STANDING, 0, LOG_NORMAL, OBJ},
	{"bury",				do_bury, POS_STANDING, 0, LOG_NORMAL, OBJ},
	
	{"prospect", do_prospect, POS_RESTING, 0, LOG_NORMAL, 1, OBJ},
	{"plant", do_plant, POS_STANDING, 0, LOG_NORMAL, 1, OBJ},	
	{"sharpen",	do_sharpen, POS_STANDING, 0, LOG_NORMAL, 1, OBJ},

	
	//Upro
	{ "learn",		do_learn,	POS_RESTING,	0, LOG_NORMAL, 1, OBJ},
	{ "dual", 		do_dual,	POS_FIGHTING,	0, LOG_NORMAL, 1, OBJ},
	{ "scribe",		do_scribe,	POS_RESTING, 	0, LOG_NORMAL, 1, OBJ},
	{ "butcher",	do_butcher,	POS_STANDING,	0, LOG_NORMAL, 1, OBJ},
	{ "skin",		do_skin,	POS_STANDING,	0, LOG_NORMAL, 1, OBJ},	
	{ "tailor",		do_tailor,  POS_STANDING,	0, LOG_NORMAL, 1, OBJ},
	{ "blacksmith",	do_blacksmith,  POS_STANDING,	0, LOG_NORMAL, 1, OBJ},
	{ "jewelcraft",	do_jewelcraft,  POS_STANDING,	0, LOG_NORMAL, 1, OBJ},
	{ "arena",		do_arena,		POS_STANDING,	0,	LOG_NORMAL, 1, OBJ},
		{"gore",    do_gore,  		POS_FIGHTING,   0, LOG_NORMAL, 1, COMBAT},
    /*
     * Combat commands.
     */
    {"backstab", 	do_backstab, 		POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"bash", 		do_bash, 			POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"sbash", 		do_shield_bash, 	POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"bs", 			do_backstab, 		POS_FIGHTING, 0, LOG_NORMAL, 0, COMBAT},
    {"berserk",		do_berserk, 		POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
	{"bloodlust",	do_bloodlust, 		POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
	{"battlecry",	do_battlecry, 		POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
	{"caltrops",	do_caltrops, 		POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
	{"throw",		do_throw, 			POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},

	
    {"dirt", 		do_dirt, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"disarm", 		do_disarm, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"flee", 		do_flee, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"kick", 		do_kick, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"murde", 		do_murde, POS_FIGHTING, 0, LOG_NORMAL, 0, COMBAT},
    {"murder", 		do_murder, POS_FIGHTING, 5, LOG_ALWAYS, 1, COMBAT},
    {"rescue", 		do_rescue, POS_FIGHTING, 0, LOG_NORMAL, 0, COMBAT},
    {"surrender", 	do_surrender, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
    {"trip", 		do_trip, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
	{"stunfist",	do_stun_fist, POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},
	{"energy",		do_energy,	POS_FIGHTING, 0, LOG_NORMAL, 1, COMBAT},

	{"earclap",		do_earclap, 	POS_FIGHTING,	0,	LOG_NORMAL,	1,	COMBAT},
	{"gouge",		do_gouge,	 	POS_FIGHTING,	0,	LOG_NORMAL,	1,	COMBAT},
	{"kidney",		do_kidney_punch,POS_FIGHTING,	0,	LOG_NORMAL,	1,	COMBAT},	
	
	//Item Manipulation - Upro 8/27/2010
	{"push",	do_push,	POS_STANDING,	0,	LOG_NORMAL,	1, OBJ},
	{"pull",	do_pull,	POS_STANDING,	0,	LOG_NORMAL,	1, OBJ},
	{"pry",		do_pry,		POS_STANDING,	0,	LOG_NORMAL,	1, OBJ},
	//{"press",	do_press,	POS_STANDING,	0,	LOG_NORMAL,	1, OBJ},
	
    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    {"mob", do_mob, POS_DEAD, 0, LOG_NEVER, 0, IMM},

    /*
     * Miscellaneous commands.
     */
    {"enter", do_enter, POS_STANDING, 0, LOG_NORMAL, 1, MOVE},
    {"follow", do_follow, POS_RESTING, 0, LOG_NORMAL, 1, MOVE},
    {"gain", do_gain, POS_STANDING, 0, LOG_NORMAL, 1, COMMON},
    {"hide", do_hide, POS_RESTING, 0, LOG_NORMAL, 1, MOVE},
    {"play", do_play, POS_RESTING, 0, LOG_NORMAL, 1, COMMON},
    {"qui", do_qui, POS_DEAD, 0, LOG_NORMAL, 0, COMMON},
    {"quit", do_quit, POS_DEAD, 0, LOG_NORMAL, 1, COMMON},
    {"recall", do_recall, POS_FIGHTING, 0, LOG_NORMAL, 1, MOVE},
    {"/", do_recall, POS_FIGHTING, 0, LOG_NORMAL, 0, MOVE},    
    {"save", do_save, POS_DEAD, 0, LOG_NORMAL, 1, COMMON},
    {"sleep", do_sleep, POS_SLEEPING, 0, LOG_NORMAL, 1, MOVE},
    {"sneak", do_sneak, POS_STANDING, 0, LOG_NORMAL, 1, MOVE},
    {"split", do_split, POS_RESTING, 0, LOG_NORMAL, 1, INFO},
    {"steal", do_steal, POS_STANDING, 0, LOG_NORMAL, 1, COMBAT},
    {"train", do_train, POS_RESTING, 0, LOG_NORMAL, 1, COMMON},
	{"expertise", do_expertise, POS_RESTING, 0, LOG_NORMAL, 1, COMMON},
    {"visible", do_visible, POS_SLEEPING, 0, LOG_NORMAL, 1, MOVE},
    {"wake", do_wake, POS_SLEEPING, 0, LOG_NORMAL, 1, MOVE},
    {"where", do_where, POS_RESTING, 0, LOG_NORMAL, 1, INFO},
	{"identify", do_identify, POS_STANDING, 0, LOG_NORMAL, 1, INFO},
	{"lore", do_lore, POS_STANDING, 0, LOG_NORMAL, 1, INFO},
	{"knock", do_knock, POS_STANDING, 0, LOG_NORMAL, 1, MOVE},	
	{"shipstat", do_shipstat, POS_STANDING, 0, LOG_NORMAL, 1, INFO},
	{"climb", do_climb, POS_STANDING, 0, LOG_NORMAL, 1, MOVE},
	{"capture",	do_capture, POS_STANDING, 0, LOG_NORMAL, 1, MOVE},
	{"peek", do_capture, POS_STANDING, 0, LOG_NORMAL, 1, INFO},
	
    /*
     * Immortal commands.
     */
    {"advance",		do_advance,	POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
    //{"copyover",	do_copyover,POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
    {"dump",		do_dump,	POS_DEAD, ML, LOG_ALWAYS, 0, IMM},
    {"trust",		do_trust,	POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
    {"violate",		do_violate,	POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
	{"doublexp",    do_doublexp,POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
	{"doubleskill", do_double_skill,POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
	{"doublegold",  do_double_gold,POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
    {"allow",		do_allow,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"ban",			do_ban,		POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"deny",		do_deny,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"disconnect",	do_disconnect,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"flag",		do_flag,	POS_DEAD, L6, LOG_ALWAYS, 1, IMM},
    {"freeze",		do_freeze,	POS_DEAD, L7, LOG_ALWAYS, 1, IMM},
    {"permban",		do_permban,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"protect",		do_protect,	POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
    {"reboo",		do_reboo,	POS_DEAD, L4, LOG_ALWAYS, 0, IMM},
    {"reboot",		do_reboot,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"set",			do_set,		POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"shutdow",		do_shutdow,	POS_DEAD, L3, LOG_ALWAYS, 0, IMM},
    {"shutdown",	do_shutdown,POS_DEAD, L3, LOG_ALWAYS, 1, IMM},
    {"wizlock",		do_wizlock,	POS_DEAD, ML, LOG_ALWAYS, 1, IMM},

    {"force",		do_force,	POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
    {"oload",		do_oload,	POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
	{"load",		do_load,	POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
    {"newlock",		do_newlock,	POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
    {"nochannels",	do_nochannels,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"noemote",		do_noemote,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"noshout",		do_noshout,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"notell",		do_notell,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"pecho",		do_pecho,	POS_DEAD, L6, LOG_ALWAYS, 1, IMM},
    {"pardon",		do_pardon,	POS_DEAD, L3, LOG_ALWAYS, 1, IMM},
    {"purge",		do_purge,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"qmconfig",	do_qmconfig,POS_DEAD, ML, LOG_ALWAYS, 1, IMM},
    {"restore",		do_restore,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    //{"sla",			do_sla,		POS_DEAD, L4, LOG_ALWAYS, 0, IMM},
    {"slay",		do_slay,	POS_DEAD, L6, LOG_ALWAYS, 1, IMM},
    {"teleport",	do_transfer,POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"transfer",	do_transfer,POS_DEAD, L5, LOG_ALWAYS, 1, IMM},

    {"poofin",		do_bamfin,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    {"poofout",		do_bamfout,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    {"gecho",		do_echo,	POS_DEAD, L5, LOG_ALWAYS, 1, IMM},
    {"holylight",	do_holylight,POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    {"incognito",	do_incognito,POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"invis",		do_invis,	POS_DEAD, IM, LOG_ALWAYS, 0, IMM},
    {"log",			do_log,		POS_DEAD, L3, LOG_ALWAYS, 1, IMM},
    {"memory",		do_memory,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    {"mwhere",		do_mwhere,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"owhere",		do_owhere,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
	{"rwhere",      do_rwhere,  POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"peace",		do_peace,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    {"echo",		do_recho,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"return",		do_return,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"snoop",		do_snoop,	POS_DEAD, L3, LOG_ALWAYS, 1, IMM},
    {"stat",		do_stat,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    //Placed here to not confuse with the immortal stat command.
	{"stats",       do_score,   POS_DEAD, 0,  LOG_NORMAL, 1, INFO},
	{"string",		do_string,	POS_DEAD, L7, LOG_ALWAYS, 1, IMM},
    {"switch",		do_switch,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"wizinvis",	do_invis,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"vnum",		do_vnum,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},
    {"zecho",		do_zecho,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},

    {"clone",		do_clone,	POS_DEAD, L4, LOG_ALWAYS, 1, IMM},

    {"wiznet",		do_wiznet,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
	{"im",			do_immtalk, POS_DEAD, IM, LOG_ALWAYS, 1, IMM},
    {"immtalk",		do_immtalk,	POS_DEAD, IM, LOG_ALWAYS, 1, IMM},	
    {"imotd",		do_imotd,	POS_DEAD, L3, LOG_NORMAL, 1, IMM},
    {":",			do_immtalk,	POS_DEAD, IM, LOG_NORMAL, 0, IMM},
    {"smote",		do_smote,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    {"prefi",		do_prefi,	POS_DEAD, IM, LOG_NORMAL, 0, IMM},
    {"prefix",		do_prefix,	POS_DEAD, IM, LOG_NORMAL, 1, IMM},
    { "mpdump",		do_mpdump,	POS_DEAD,   L6,  LOG_ALWAYS, 1, IMM},
    { "mpstat",		do_mpstat,	POS_DEAD,   L6,  LOG_ALWAYS, 1, IMM},
	{ "opedit",     do_opedit,  POS_DEAD,   L6,  LOG_ALWAYS, 1 , IMM},
    { "rpedit",     do_rpedit,  POS_DEAD,   L6,  LOG_ALWAYS, 1 , IMM},
    { "opdump",		do_opdump,	POS_DEAD,	L6,  LOG_ALWAYS,  1 , IMM},
    { "opstat",		do_opstat,	POS_DEAD,	L6,  LOG_ALWAYS,  1 , IMM},
    { "rpdump",		do_rpdump,	POS_DEAD,	L6,  LOG_ALWAYS,  1 , IMM},
    { "rpstat",		do_rpstat,	POS_DEAD,	L6,  LOG_ALWAYS,  1 , IMM},
	{ "ireset",		do_ireset,	POS_DEAD,	L6,  LOG_ALWAYS, 1 , IMM},
	{ "autodig", 	do_autodig, POS_DEAD,   L6,  LOG_ALWAYS, 1 , IMM},
	//upro imm stuff
	
	{"addapply", 	do_addapply,	POS_DEAD,	L2, LOG_ALWAYS, 1, IMM},
	{"addhours", 	do_addhours,	POS_DEAD, 	L2, LOG_ALWAYS, 1, IMM},
	{"addxp", 		do_addxp,		POS_DEAD,	L4, LOG_ALWAYS, 1, IMM},
	{"addmin", 		do_addmin,		POS_DEAD, 	L2, LOG_ALWAYS, 1, IMM},	
	{"addgp",		do_addguildpoints, POS_DEAD,L4, LOG_ALWAYS, 1, IMM},
	{"addrep",		do_addrep,		POS_DEAD,	L3,	LOG_ALWAYS,	1, IMM},
	{"pretitle",	do_pretitle,	POS_DEAD, 	L5, LOG_ALWAYS, 1, IMM},
	{"sockets",		do_sockets,		POS_DEAD,	L3,	LOG_ALWAYS, 1, IMM},
	{"users",		do_sockets,		POS_DEAD,	L3,	LOG_ALWAYS, 1, IMM},
	{"skillstat",	do_skillstat,	POS_DEAD,	L3, LOG_ALWAYS, 1, IMM},
	{"spellstat",	do_spellstat,	POS_DEAD,	L3, LOG_ALWAYS, 1, IMM},
	{"repstat",		do_repstat,		POS_DEAD,	L3, LOG_ALWAYS, 1, IMM},
	{"checkwpn",    do_checkweaps,  POS_DEAD,	IM, LOG_ALWAYS, 1, IMM},
	{"checktype",	do_checktype,	POS_DEAD,	IM, LOG_ALWAYS, 1, IMM},
	{"checkmob",    do_checkmobs, 	POS_DEAD,   IM, LOG_ALWAYS, 1, IMM},
	{"emailpw",     do_email_pw,	POS_DEAD,	IM, LOG_ALWAYS, 1, IMM},
	{"rename",      do_rename,		POS_DEAD,   L3, LOG_ALWAYS, 1, IMM},
	{"slist",       do_slist,		POS_DEAD,   L3, LOG_ALWAYS, 1, IMM},
	{"ftick",		do_ftick,		POS_DEAD,	L2,	LOG_ALWAYS,  1, IMM},	
	{"enchantlist",	do_enchant_list,POS_DEAD,	L3, LOG_ALWAYS, 1, IMM},
	{"create",		do_create		,POS_DEAD,	L3, LOG_ALWAYS, 1, IMM},
	{"gweather",	do_gweather		,POS_DEAD,	IM, LOG_ALWAYS, 1, IMM},
	
  /*
	Ship shit.
  */


    { "embark",	    do_embark,		POS_STANDING,	0,  	LOG_NORMAL, 1, MOVE },
	{ "disembark",	do_disembark,	POS_STANDING,	0,		LOG_NORMAL, 1, MOVE },    
    { "cannon",		do_cannon,		POS_STANDING,	0,  	LOG_NORMAL, 1, COMBAT },
    { "sail",		do_sail,		POS_STANDING,	0,  	LOG_NORMAL, 1, MOVE },
	{ "anchor",		do_anchor,		POS_STANDING,	0,  	LOG_NORMAL, 1, MOVE },
    { "raise",		do_raise,		POS_STANDING,	0,  	LOG_NORMAL, 1, MOVE },
    { "dock",		do_dock,		POS_STANDING,	0,  	LOG_NORMAL, 1, MOVE }, 


	{ "trap",			do_traps,		POS_STANDING,		0,	LOG_NORMAL, 1, OBJ },	
	{ "repair",			do_repair,		POS_STANDING,		0,	LOG_NORMAL, 1, OBJ },
	{ "recharge",		do_recharge,	POS_STANDING,		0,	LOG_NORMAL, 1, OBJ },
	{ "bandage",		do_bandage,		POS_STANDING,		0,	LOG_NORMAL, 1, OBJ },
	//{ "grank",			do_grank,		POS_STANDING,		0,	LOG_NORMAL,	1, CONFIG },
	{ "gather",			do_gather, 		POS_STANDING,		1,	LOG_NORMAL, 1, OBJ },
	{ "blind",			do_blind,		POS_DEAD,			1,  LOG_ALWAYS,	1, CONFIG },
	{ "gameinfo",       do_gameinfo,    POS_DEAD,     		0,  LOG_NORMAL, 1, INFO},
	{ "experience",     do_experience,  POS_DEAD,     		0,  LOG_NORMAL, 1, INFO},
	{ "level",		    do_experience,  POS_DEAD,     		0,  LOG_NORMAL, 1, INFO},
	{ "chop",			do_woodcut,		POS_STANDING,		0,	LOG_NORMAL,	1, OBJ },	
	{ "achievements",	do_achievements,POS_SLEEPING,		0,	LOG_NORMAL, 1, INFO },
	{ "spellup",		do_spellup,		POS_STANDING,		0,	LOG_NORMAL, 1, COMBAT},

    /*
     * OLC
     */
    {"edit", 	do_olc,		POS_DEAD, L7, 	LOG_NORMAL, 	1, IMM},
    {"asave", 	do_asave, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"alist", 	do_alist, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"resets", 	do_resets, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
	{"areset", 	do_aresets, POS_DEAD, L8, 	LOG_NORMAL,		1, IMM},
    {"redit", 	do_redit, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"medit", 	do_medit, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"aedit", 	do_aedit, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"oedit", 	do_oedit, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"mpedit", 	do_mpedit, 	POS_DEAD, L8, 	LOG_NORMAL, 	1, IMM},
    {"hedit", 	do_hedit, 	POS_DEAD, L6, 	LOG_ALWAYS, 	1, IMM},
	//{"nedit",	do_nedit, 	POS_DEAD, 1,	LOG_NORMAL,		1, COMM},
	{"qedit",	do_qedit, 	POS_DEAD, ML,	LOG_NORMAL,		1, IMM},
	
	//Clan stuff Upro 1/19/2020
	{"rank", 	do_rank,		POS_SLEEPING, 0, 	LOG_ALWAYS, 	1, COMM},

    /*
     * End of list.
     */
    {"", 0, POS_DEAD, 0, LOG_NORMAL, 0, -1}
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret (CHAR_DATA * ch, char *argument)
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
	//char buf [MSL];
    int cmd;
    int trust;
    bool found;

    /*
     * Strip leading spaces.
     */
    while (isspace (*argument))
        argument++;
    if (argument[0] == '\0')
        return;

    /*
     * No hiding.
     */
    REMOVE_BIT (ch->affected_by, AFF_HIDE);

    /*
     * Implement freeze command.
     */
    if (!IS_NPC (ch) && IS_SET (ch->act, PLR_FREEZE))
    {
        SEND ("You're totally frozen!\r\n", ch);
        return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     * also no spaces needed after punctuation.
     */
    strcpy (logline, argument);
	/*Lets see who is doing what? -Ferric*/
    //strcpy( buf, argument);
    //sprintf(last_command,"%s in room [%d] using this command: %s.",ch->name,
    //        ch->in_room->vnum,
    //        buf);
    if (!isalpha (argument[0]) && !isdigit (argument[0]))
    {
        command[0] = argument[0];
        command[1] = '\0';
        argument++;
        while (isspace (*argument))
            argument++;
    }
    else
    {
        argument = one_argument (argument, command);
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust (ch);
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == cmd_table[cmd].name[0]
            && !str_prefix (command, cmd_table[cmd].name)
            && cmd_table[cmd].level <= trust)
        {
            found = TRUE;
            break;
        }
    }

    /*
     * Log and snoop.
     */
    smash_dollar(logline);
    
    if (cmd_table[cmd].log == LOG_NEVER)
        strcpy (logline, "");

	/* Replaced original block of code with fix from Edwin
	 * to prevent crashes due to dollar signs in logstrings.
	 * I threw in the above call to smash_dollar() just for
	 * the sake of overkill :) JR -- 10/15/00
	 */
    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
		||   fLogAll
		||   cmd_table[cmd].log == LOG_ALWAYS )
	{
    	char    s[2*MAX_INPUT_LENGTH],*ps;
    	int     i;

    	ps=s; 
    	sprintf( log_buf, "Log %s: %s", ch->name, logline );
    	/* Make sure that was is displayed is what is typed */
    	for (i=0;log_buf[i];i++)
    	{ 
			*ps++=log_buf[i];  
			if (log_buf[i]=='$')
	    		*ps++='$';
			if (log_buf[i]=='{')
	    		*ps++='{';
    	}
    	*ps=0;
    	wiznet(s,ch,NULL,WIZ_SECURE,0,get_trust(ch));
    	log_string( log_buf );
	}

    if (ch->desc != NULL && ch->desc->snoop_by != NULL)
    {
        write_to_buffer (ch->desc->snoop_by, "% ", 2);
        write_to_buffer (ch->desc->snoop_by, logline, 0);
        write_to_buffer (ch->desc->snoop_by, "\r\n", 2);
    }

    if (!found)
    {
        /*
         * Look for command in socials table.
         */
        if (!check_social (ch, command, argument))
            SEND ("That's not a valid command. (type {Dcommands{x for a list)\r\n", ch);
        return;
    }

    /*
     * Character not in position for command?
     */
    if (ch->position < cmd_table[cmd].position)
    {
        switch (ch->position)
        {
            case POS_DEAD:
                SEND ("Lie still; you are DEAD.\r\n", ch);
                break;

            case POS_MORTAL:
            case POS_INCAP:
                SEND ("You are hurt far too bad for that.\r\n", ch);
                break;

            case POS_STUNNED:
                SEND ("You are too stunned to do that.\r\n", ch);
                break;

            case POS_SLEEPING:
                SEND ("Not while you are sleeping.\r\n", ch);
                break;

            case POS_RESTING:
                SEND ("You must stand up first.\r\n", ch);
                break;

            case POS_SITTING:
                SEND ("you must stand up first.\r\n", ch);
                break;

            case POS_FIGHTING:
                SEND ("No way!  You are still fighting!\r\n", ch);
                break;

        }
        return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) (ch, argument);

    tail_chain ();
    return;
}

/* function to keep argument safe in all commands -- no static strings */
void do_function (CHAR_DATA * ch, DO_FUN * do_fun, char *argument)
{
    char *command_string;

    /* copy the string */
    command_string = str_dup (argument);

    /* dispatch the command */
    (*do_fun) (ch, command_string);

    /* free the string */
    free_string (command_string);
}

bool check_social (CHAR_DATA * ch, char *command, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == social_table[cmd].name[0]
            && !str_prefix (command, social_table[cmd].name))
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
        return FALSE;

    if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
        SEND ("You are anti-social!\r\n", ch);
        return TRUE;
    }

    switch (ch->position)
    {
        case POS_DEAD:
            SEND ("Lie still; you are DEAD.\r\n", ch);
            return TRUE;

        case POS_INCAP:
        case POS_MORTAL:
            SEND ("You are hurt far too bad for that.\r\n", ch);
            return TRUE;

        case POS_STUNNED:
            SEND ("You are too stunned to do that.\r\n", ch);
            return TRUE;

        case POS_SLEEPING:
            /*
             * I just know this is the path to a 12" 'if' statement.  :(
             * But two players asked for it already!  -- Furey
             */
            if (!str_cmp (social_table[cmd].name, "snore"))
                break;
            SEND ("In your dreams, or what?\r\n", ch);
            return TRUE;

    }

    one_argument (argument, arg);
    victim = NULL;
    if (arg[0] == '\0')
    {
        act (social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM);
        act (social_table[cmd].char_no_arg, ch, NULL, victim, TO_CHAR);
    }
    else if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They aren't here.\r\n", ch);
    }
    else if (victim == ch)
    {
        act (social_table[cmd].others_auto, ch, NULL, victim, TO_ROOM);
        act (social_table[cmd].char_auto, ch, NULL, victim, TO_CHAR);
    }
    else
    {
        act (social_table[cmd].others_found, ch, NULL, victim, TO_NOTVICT);
        act (social_table[cmd].char_found, ch, NULL, victim, TO_CHAR);
        act (social_table[cmd].vict_found, ch, NULL, victim, TO_VICT);

        if (!IS_NPC (ch) && IS_NPC (victim)
            && !IS_AFFECTED (victim, AFF_CHARM)
            && IS_AWAKE (victim) && victim->desc == NULL)
        {
            switch (number_bits (4))
            {
                case 0:

                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                    act (social_table[cmd].others_found,
                         victim, NULL, ch, TO_NOTVICT);
                    act (social_table[cmd].char_found, victim, NULL, ch,
                         TO_CHAR);
                    act (social_table[cmd].vict_found, victim, NULL, ch,
                         TO_VICT);
                    break;

                case 9:
                case 10:
                case 11:
                case 12:
                    act ("$n slaps $N.", victim, NULL, ch, TO_NOTVICT);
                    act ("You slap $N.", victim, NULL, ch, TO_CHAR);
                    act ("$n slaps you.", victim, NULL, ch, TO_VICT);
                    break;
            }
        }
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number (char *arg)
{

    if (*arg == '\0')
        return FALSE;

    if (*arg == '+' || *arg == '-')
        arg++;

    for (; *arg != '\0'; arg++)
    {
        if (!isdigit (*arg))
            return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument (char *argument, char *arg)
{
    char *pdot;
    int number;

    for (pdot = argument; *pdot != '\0'; pdot++)
    {
        if (*pdot == '.')
        {
            *pdot = '\0';
            number = atoi (argument);
            *pdot = '.';
            strcpy (arg, pdot + 1);
            return number;
        }
    }

    strcpy (arg, argument);
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument (char *argument, char *arg)
{
    char *pdot;
    int number;

    for (pdot = argument; *pdot != '\0'; pdot++)
    {
        if (*pdot == '*')
        {
            *pdot = '\0';
            number = atoi (argument);
            *pdot = '*';
            strcpy (arg, pdot + 1);
            return number;
        }
    }

    strcpy (arg, argument);
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument (char *argument, char *arg_first)
{
    char cEnd;

    while (isspace (*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0')
    {
        if (*argument == cEnd)
        {
            argument++;
            break;
        }
        *arg_first = LOWER (*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while (isspace (*argument))
        argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
 
void do_commands( CHAR_DATA *ch, char *argument )
{
	
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
    int category;
	int count = 0;

 
    col = 0;
 
 // I had to categorize the immortal commands, but not print them to mortals, 
 // so immortal commands are category 0, and I started the loop with category
 // 1 commands. There's probably a better way to do this, but hey, it works!
 // I also have the commands that I felt were most important in the higher 
 // categories, which print out last. The more useless commands are in lower 
 // categories so they might scroll off the screen.
 //
    for( category = 1; category < 8; category++ )
    {
      switch(category)
      {
      case CONFIG:
	      SEND("{d-=-=-{x Configuration Commands {r-=-=-=-=-=-=-=-{x\r\n", ch);      
	      break;							
      case COMMON: 							
	      SEND("{r-=-=-=-=-=-=-=-{x Common Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch); 		 
	      break;								 
      case COMM:
	      SEND("{r-=-=-=-=-=-=-=-{x Communication Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch);
	      break;
      case INFO:
	      SEND("{r-=-=-=-=-=-=-=-{x Informational Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch);
	      break;
      case OBJ:
	      SEND("{r-=-=-=-=-=-=-=-{x Object Manipulation Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch);
	      break;
      case MOVE:
	      SEND("{r-=-=-=-=-=-=-=-{x Movement Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch);
	      break;
      case COMBAT:
	      SEND("{r-=-=-=-=-=-=-=-{x Combat Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch);
	      break;
	  case IMM:
		if (IS_IMMORTAL(ch))
		{
			SEND("{r-=-=-=-=-=-=-=-{x Immortal Commands {r-=-=-=-=-=-=-=-{x\r\n\r\n", ch);
			break;
		}
		break;
      }
       for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
       {
		   if ( cmd_table[cmd].level < LEVEL_HERO && cmd_table[cmd].level <= get_trust( ch ) && cmd_table[cmd].show && cmd_table[cmd].cat == category )
		   {
			   if ( cmd_table[cmd].cat != IMM || IS_IMMORTAL(ch) )
			   {
				   sprintf( buf, "   %-12s", cmd_table[cmd].name );
				   count++;
				   SEND( buf, ch );
				   if ( ++col % 5 == 0 )
						SEND( "\r\n", ch );
				}
		   }
       }
       if (col % 5 != 0 )
       {
		   SEND( "\r\n\r\n", ch );
		   col = 0;
       }
    }
 
    if ( col % 5 != 0 )
		SEND( "\r\n\r\n", ch );
		
	sprintf(buf, "Total commands available to you: %d\r\n\r\n", count);
	SEND (buf,ch);
	
    return;
	
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
    int clevel;
    col = 0;
 
    for( clevel = LEVEL_HERO + 1; clevel < MAX_LEVEL + 1; clevel++ ) 
    {
        for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{ 
            if ( cmd_table[cmd].level >= LEVEL_HERO
            &&   cmd_table[cmd].level <= get_trust( ch ) 
            &&   cmd_table[cmd].show
	    &&   cmd_table[cmd].level == clevel)
	    {
	        sprintf( buf, "{r[{x%-2d{r] {x%-12s{x", cmd_table[cmd].level, cmd_table[cmd].name );
	        SEND( buf, ch );
	        if ( ++col % 5 == 0 )
		    SEND( "\r\n", ch );
	    }
	}
    }
 
    if ( col % 5 != 0 )
	SEND( "\r\n", ch );
    return;
}
