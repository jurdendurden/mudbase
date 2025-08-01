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
*	ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*	ROM has been brought to you by the ROM consortium                      *
*	    Russ Taylor (rtaylor@hypercube.org)                                *
*	    Gabrielle Taylor (gtaylor@hypercube.org)                           *
*	    Brian Moore (zump@rom.org)                                         *
*	By using this code, you have agreed to follow the terms of the         *
*	ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

/* this is a listing of all the commands and command related data */

/* wrapper function for safe command execution */
void do_function args((CHAR_DATA *ch, DO_FUN *do_fun, char *argument));

/* for command types */
#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* creator */
#define L2	MAX_LEVEL - 2	/* supreme being */
#define L3	MAX_LEVEL - 3	/* deity */
#define L4 	MAX_LEVEL - 4	/* god */
#define L5	MAX_LEVEL - 5	/* immortal */
#define L6	MAX_LEVEL - 6	/* demigod */
#define L7	MAX_LEVEL - 7	/* angel */
#define L8	MAX_LEVEL - 8	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* avatar */
#define HE	LEVEL_HERO	/* hero */

#define COM_INGORE	1


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const		name;
    DO_FUN *			do_fun;
    sh_int				position;
    sh_int				level;
    sh_int				log;
    sh_int              show;
	sh_int				cat;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];

//////////////////////////////////
//          MOB PROGS			//
//////////////////////////////////

struct	mob_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};
 
struct obj_cmd_type
{
    char * const	name;
    OBJ_FUN *		obj_fun;
};
 
struct room_cmd_type
{
    char * const	name;
    ROOM_FUN *		room_fun;
};
 
/* the command table itself */
extern	const	struct	mob_cmd_type	mob_cmd_table	[];
 
/*
 * MOBcommand functions.
 * Defined in mob_cmds.c
 */
DECLARE_DO_FUN(	do_mpasound	);
DECLARE_DO_FUN(	do_mpgecho	);
DECLARE_DO_FUN(	do_mpzecho	);
DECLARE_DO_FUN(	do_mpkill	);
DECLARE_DO_FUN(	do_mpassist	);
DECLARE_DO_FUN(	do_mpjunk	);
DECLARE_DO_FUN(	do_mpechoaround	);
DECLARE_DO_FUN(	do_mpecho	);
DECLARE_DO_FUN(	do_mpechoat	);
DECLARE_DO_FUN(	do_mpmload	);
DECLARE_DO_FUN(	do_mpoload	);
DECLARE_DO_FUN(	do_mppurge	);
DECLARE_DO_FUN(	do_mpgoto	);
DECLARE_DO_FUN(	do_mpat		);
DECLARE_DO_FUN(	do_mptransfer	);
DECLARE_DO_FUN(	do_mpgtransfer	);
DECLARE_DO_FUN(	do_mpforce	);
DECLARE_DO_FUN(	do_mpgforce	);
DECLARE_DO_FUN(	do_mpvforce	);
DECLARE_DO_FUN(	do_mpcast	);
DECLARE_DO_FUN(	do_mpdamage	);
DECLARE_DO_FUN(	do_mpremember	);
DECLARE_DO_FUN(	do_mpforget	);
DECLARE_DO_FUN(	do_mpdelay	);
DECLARE_DO_FUN(	do_mpcancel	);
DECLARE_DO_FUN(	do_mpcall	);
DECLARE_DO_FUN(	do_mpflee	);
DECLARE_DO_FUN(	do_mpotransfer	);
DECLARE_DO_FUN(	do_mpremove	);
DECLARE_DO_FUN( do_mpaddgp );
DECLARE_DO_FUN( do_mpaddxp );
DECLARE_DO_FUN( do_mpaddfaction );
 
/*
 * OBJcommand functions
 * Defined in mob_cmds.c
 */
DECLARE_OBJ_FUN( do_opgecho	 );
DECLARE_OBJ_FUN( do_opzecho	 );
DECLARE_OBJ_FUN( do_opecho	 );
DECLARE_OBJ_FUN( do_opechoaround );
DECLARE_OBJ_FUN( do_opechoat	 );
DECLARE_OBJ_FUN( do_opmload	 );
DECLARE_OBJ_FUN( do_opoload	 );
DECLARE_OBJ_FUN( do_oppurge	 );
DECLARE_OBJ_FUN( do_opgoto	 );
DECLARE_OBJ_FUN( do_optransfer	 );
DECLARE_OBJ_FUN( do_opgtransfer	 );
DECLARE_OBJ_FUN( do_opotransfer	 );
DECLARE_OBJ_FUN( do_opforce	 );
DECLARE_OBJ_FUN( do_opgforce	 );
DECLARE_OBJ_FUN( do_opvforce	 );
DECLARE_OBJ_FUN( do_opdamage	 );
DECLARE_OBJ_FUN( do_opremember	 );
DECLARE_OBJ_FUN( do_opforget	 );
DECLARE_OBJ_FUN( do_opdelay	 );
DECLARE_OBJ_FUN( do_opcancel	 );
DECLARE_OBJ_FUN( do_opcall	 );
DECLARE_OBJ_FUN( do_opremove	 );
DECLARE_OBJ_FUN( do_opchange_align);
 
/*
 * ROOMcommand functions
 * Defined in mob_cmds.c
 */
DECLARE_ROOM_FUN( do_rpasound	 );
DECLARE_ROOM_FUN( do_rpgecho	 );
DECLARE_ROOM_FUN( do_rpzecho	 );
DECLARE_ROOM_FUN( do_rpecho	 );
DECLARE_ROOM_FUN( do_rpechoaround);
DECLARE_ROOM_FUN( do_rpechoat	 );
DECLARE_ROOM_FUN( do_rpmload	 );
DECLARE_ROOM_FUN( do_rpoload	 );
DECLARE_ROOM_FUN( do_rppurge	 );
DECLARE_ROOM_FUN( do_rptransfer	 );
DECLARE_ROOM_FUN( do_rpgtransfer );
DECLARE_ROOM_FUN( do_rpotransfer );
DECLARE_ROOM_FUN( do_rpforce	 );
DECLARE_ROOM_FUN( do_rpgforce	 );
DECLARE_ROOM_FUN( do_rpvforce	 );
DECLARE_ROOM_FUN( do_rpdamage	 );
DECLARE_ROOM_FUN( do_rpremember	 );
DECLARE_ROOM_FUN( do_rpforget	 );
DECLARE_ROOM_FUN( do_rpdelay	 );
DECLARE_ROOM_FUN( do_rpcancel	 );
DECLARE_ROOM_FUN( do_rpcall	 );
DECLARE_ROOM_FUN( do_rpremove	 );


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN(	do_advance		);
DECLARE_DO_FUN( do_affects		);
DECLARE_DO_FUN( do_afk			);
DECLARE_DO_FUN( do_alia			);
DECLARE_DO_FUN( do_alias		);
DECLARE_DO_FUN(	do_allow		);
DECLARE_DO_FUN( do_answer		);
DECLARE_DO_FUN(	do_areas		);
DECLARE_DO_FUN(	do_at			);
DECLARE_DO_FUN(	do_auction		);
DECLARE_DO_FUN( do_autoall		);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autotrack	);
DECLARE_DO_FUN( do_autoexit		);
DECLARE_DO_FUN( do_autogold		);
DECLARE_DO_FUN( do_autolist		);
DECLARE_DO_FUN( do_autoloot		);
DECLARE_DO_FUN( do_autosac		);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN( do_automap		);
DECLARE_DO_FUN(	do_backstab		);
DECLARE_DO_FUN(	do_bamfin		);
DECLARE_DO_FUN(	do_bamfout		);
DECLARE_DO_FUN(	do_ban			);
DECLARE_DO_FUN( do_bash			);
DECLARE_DO_FUN( do_shield_bash	);
DECLARE_DO_FUN( do_caltrops		);
DECLARE_DO_FUN( do_throw		);
DECLARE_DO_FUN( do_berserk		);
DECLARE_DO_FUN( do_bloodlust	);
DECLARE_DO_FUN( do_battlecry	);
DECLARE_DO_FUN( do_board		);
DECLARE_DO_FUN(	do_brandish		);
DECLARE_DO_FUN( do_brief		);
DECLARE_DO_FUN(	do_bug			);
DECLARE_DO_FUN(	do_buy			);
DECLARE_DO_FUN(	do_cast			);
DECLARE_DO_FUN( do_channels		);
DECLARE_DO_FUN( do_clone		);
DECLARE_DO_FUN(	do_close		);
DECLARE_DO_FUN(	do_colour		);
DECLARE_DO_FUN(	do_commands		);
DECLARE_DO_FUN( do_combine		);
DECLARE_DO_FUN( do_compact		);
DECLARE_DO_FUN(	do_compare		);
DECLARE_DO_FUN(	do_consider		);
DECLARE_DO_FUN(	do_copyover		);
DECLARE_DO_FUN( do_count		);
DECLARE_DO_FUN(	do_credits		);
DECLARE_DO_FUN( do_deaf			);
DECLARE_DO_FUN( do_delet		);
DECLARE_DO_FUN( do_delete		);
DECLARE_DO_FUN(	do_deny			);
DECLARE_DO_FUN(	do_description		);
DECLARE_DO_FUN( do_dirt			);
DECLARE_DO_FUN(	do_disarm		);
DECLARE_DO_FUN(	do_disconnect		);
DECLARE_DO_FUN(	do_down			);
DECLARE_DO_FUN(	do_drink		);
DECLARE_DO_FUN(	do_drop			);
DECLARE_DO_FUN(	do_obj_dump			);
DECLARE_DO_FUN( do_dump			);
DECLARE_DO_FUN(	do_east			);
DECLARE_DO_FUN(	do_eat			);
DECLARE_DO_FUN(	do_echo			);
DECLARE_DO_FUN(	do_emote		);
DECLARE_DO_FUN( do_enter		);
DECLARE_DO_FUN( do_envenom		);
DECLARE_DO_FUN(	do_equipment		);
DECLARE_DO_FUN(	do_examine		);
DECLARE_DO_FUN(	do_exits		);
DECLARE_DO_FUN(	do_fill			);
DECLARE_DO_FUN( do_flag			);
DECLARE_DO_FUN(	do_flee			);
DECLARE_DO_FUN(	do_follow		);
DECLARE_DO_FUN(	do_force		);
DECLARE_DO_FUN(	do_freeze		);
DECLARE_DO_FUN( do_gain			);
DECLARE_DO_FUN(	do_get			);
DECLARE_DO_FUN(	do_give			);
DECLARE_DO_FUN( do_gossip		);
DECLARE_DO_FUN(	do_goto			);
DECLARE_DO_FUN( do_grats		);
DECLARE_DO_FUN(	do_group		);
DECLARE_DO_FUN( do_groups		);
DECLARE_DO_FUN(	do_gtell		);
DECLARE_DO_FUN( do_guild	    	);
DECLARE_DO_FUN( do_heal			);
DECLARE_DO_FUN(	do_help			);
DECLARE_DO_FUN(	do_hide			);
DECLARE_DO_FUN(	do_holylight		);
DECLARE_DO_FUN(	do_immtalk		);
DECLARE_DO_FUN( do_incognito		);
DECLARE_DO_FUN( do_clantalk		);
DECLARE_DO_FUN( do_imotd		);
DECLARE_DO_FUN(	do_inventory		);
DECLARE_DO_FUN(	do_invis		);
DECLARE_DO_FUN(	do_kick			);
DECLARE_DO_FUN(	do_kill			);
DECLARE_DO_FUN(	do_list			);
DECLARE_DO_FUN( do_load			);
DECLARE_DO_FUN(	do_lock			);
DECLARE_DO_FUN(	do_log			);
DECLARE_DO_FUN(	do_look			);
DECLARE_DO_FUN(	do_memory		);
DECLARE_DO_FUN(	do_mfind		);
DECLARE_DO_FUN(	do_mload		);
DECLARE_DO_FUN(	do_mset			);
DECLARE_DO_FUN(	do_mstat		);
DECLARE_DO_FUN(	do_mwhere		);
DECLARE_DO_FUN( do_mob			);
DECLARE_DO_FUN( do_motd			);
DECLARE_DO_FUN( do_mpstat		);
DECLARE_DO_FUN( do_mpdump		);
DECLARE_DO_FUN(	do_murde		);
DECLARE_DO_FUN(	do_murder		);
DECLARE_DO_FUN( do_music		);
DECLARE_DO_FUN( do_newlock		);
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN(	do_noemote		);
DECLARE_DO_FUN( do_nofollow		);
DECLARE_DO_FUN( do_noloot		);
DECLARE_DO_FUN(	do_north		);
DECLARE_DO_FUN(	do_noshout		);
DECLARE_DO_FUN( do_nosummon		);
DECLARE_DO_FUN(	do_note 		);
DECLARE_DO_FUN(	do_notell		);
DECLARE_DO_FUN( do_ooc			);
DECLARE_DO_FUN( do_condition	);
DECLARE_DO_FUN(	do_ofind		);
DECLARE_DO_FUN(	do_oload		);
DECLARE_DO_FUN(	do_open			);
DECLARE_DO_FUN(	do_order		);
DECLARE_DO_FUN(	do_oset			);
DECLARE_DO_FUN(	do_ostat		);
DECLARE_DO_FUN( do_outfit		);
DECLARE_DO_FUN( do_owhere		);
DECLARE_DO_FUN(	do_pardon		);
DECLARE_DO_FUN(	do_password		);
DECLARE_DO_FUN(	do_peace		);
DECLARE_DO_FUN( do_pecho		);
DECLARE_DO_FUN( do_permban		);
DECLARE_DO_FUN(	do_pick			);
DECLARE_DO_FUN( do_play			);
DECLARE_DO_FUN( do_pmote		);
DECLARE_DO_FUN(	do_pose			);
DECLARE_DO_FUN( do_pour			);
DECLARE_DO_FUN(	do_practice		);
DECLARE_DO_FUN( do_prefi		);
DECLARE_DO_FUN( do_prefix		);
DECLARE_DO_FUN( do_prompt		);
DECLARE_DO_FUN( do_protect		);
DECLARE_DO_FUN(	do_purge		);
DECLARE_DO_FUN(	do_put			);
DECLARE_DO_FUN(	do_qmconfig		);
DECLARE_DO_FUN(	do_quaff		);
DECLARE_DO_FUN( do_question		);
DECLARE_DO_FUN(	do_qui			);
DECLARE_DO_FUN( do_quiet		);
DECLARE_DO_FUN(	do_quit			);
DECLARE_DO_FUN( do_quote		);
DECLARE_DO_FUN( do_read			);
DECLARE_DO_FUN(	do_reboo		);
DECLARE_DO_FUN(	do_reboot		);
DECLARE_DO_FUN(	do_recall		);
DECLARE_DO_FUN(	do_recho		);
DECLARE_DO_FUN(	do_recite		);
DECLARE_DO_FUN(	do_remove		);
DECLARE_DO_FUN(	do_rent			);
DECLARE_DO_FUN( do_replay		);
DECLARE_DO_FUN(	do_reply		);
DECLARE_DO_FUN(	do_report		);
DECLARE_DO_FUN(	do_rescue		);
DECLARE_DO_FUN(	do_rest			);
DECLARE_DO_FUN(	do_restore		);
DECLARE_DO_FUN(	do_return		);
DECLARE_DO_FUN(	do_rset			);
DECLARE_DO_FUN(	do_rstat		);
DECLARE_DO_FUN( do_rules		);
DECLARE_DO_FUN(	do_sacrifice		);
DECLARE_DO_FUN(	do_save			);
DECLARE_DO_FUN(	do_say			);
DECLARE_DO_FUN(	do_scan			);
DECLARE_DO_FUN(	do_score		);
DECLARE_DO_FUN(	do_score2		);
DECLARE_DO_FUN( do_scroll		);
DECLARE_DO_FUN(	do_sell			);
DECLARE_DO_FUN( do_set			);
DECLARE_DO_FUN(	do_shout		);
DECLARE_DO_FUN( do_show			);
DECLARE_DO_FUN(	do_shutdow		);
DECLARE_DO_FUN(	do_shutdown		);
DECLARE_DO_FUN( do_sit			);
DECLARE_DO_FUN( do_skills		);
DECLARE_DO_FUN(	do_sla			);
DECLARE_DO_FUN(	do_slay			);
DECLARE_DO_FUN(	do_sleep		);
DECLARE_DO_FUN(	do_slookup		);
DECLARE_DO_FUN( do_smote		);
DECLARE_DO_FUN(	do_sneak		);
DECLARE_DO_FUN(	do_snoop		);
DECLARE_DO_FUN( do_socials		);
DECLARE_DO_FUN(	do_south		);
DECLARE_DO_FUN( do_sockets		);
DECLARE_DO_FUN( do_spells		);
DECLARE_DO_FUN(	do_split		);
DECLARE_DO_FUN(	do_sset			);
DECLARE_DO_FUN(	do_stand		);
DECLARE_DO_FUN( do_stat			);
DECLARE_DO_FUN(	do_steal		);
DECLARE_DO_FUN( do_story		);
DECLARE_DO_FUN( do_string		);
DECLARE_DO_FUN(	do_surrender		);
DECLARE_DO_FUN(	do_switch		);
DECLARE_DO_FUN(	do_tell			);
DECLARE_DO_FUN(	do_telnetga		);
DECLARE_DO_FUN(	do_time			);
DECLARE_DO_FUN(	do_title		);
DECLARE_DO_FUN(	do_train		);
DECLARE_DO_FUN(	do_transfer		);
DECLARE_DO_FUN( do_trip			);
DECLARE_DO_FUN(	do_trust		);
DECLARE_DO_FUN(	do_typo			);
DECLARE_DO_FUN( do_unalias		);
DECLARE_DO_FUN(	do_unlock		);
DECLARE_DO_FUN(	do_up			);
DECLARE_DO_FUN(	do_value		);
DECLARE_DO_FUN(	do_visible		);
DECLARE_DO_FUN( do_violate		);
DECLARE_DO_FUN( do_vnum			);
DECLARE_DO_FUN( do_wares		);
DECLARE_DO_FUN(	do_wake			);
DECLARE_DO_FUN(	do_wear			);
DECLARE_DO_FUN(	do_weather		);
DECLARE_DO_FUN(	do_west			);
DECLARE_DO_FUN(	do_where		);
DECLARE_DO_FUN(	do_who			);
DECLARE_DO_FUN( do_whois		);
DECLARE_DO_FUN(	do_wimpy		);
DECLARE_DO_FUN(	do_wizhelp		);
DECLARE_DO_FUN(	do_wizlock		);
DECLARE_DO_FUN( do_wizlist		);
DECLARE_DO_FUN( do_wiznet		);
DECLARE_DO_FUN( do_worth		);
DECLARE_DO_FUN(	do_yell			);
DECLARE_DO_FUN(	do_zap			);
DECLARE_DO_FUN( do_zecho		);
DECLARE_DO_FUN( do_olc			);
DECLARE_DO_FUN( do_asave		);
DECLARE_DO_FUN( do_alist		);
DECLARE_DO_FUN( do_resets		);
DECLARE_DO_FUN( do_aresets		);
DECLARE_DO_FUN( do_redit		);
DECLARE_DO_FUN( do_aedit		);
DECLARE_DO_FUN( do_medit		);
DECLARE_DO_FUN( do_oedit		);
DECLARE_DO_FUN( do_mpedit		);
DECLARE_DO_FUN( do_opedit );
DECLARE_DO_FUN( do_rpedit );
DECLARE_DO_FUN( do_opdump );
DECLARE_DO_FUN( do_opstat );
DECLARE_DO_FUN( do_rpdump );
DECLARE_DO_FUN( do_rpstat );
DECLARE_DO_FUN( do_hedit		);
DECLARE_DO_FUN( do_qmread		);
DECLARE_DO_FUN( do_nedit		);
DECLARE_DO_FUN( do_qedit		);

// Upro
DECLARE_DO_FUN( do_learn		);
DECLARE_DO_FUN( do_knock		);
//DECLARE_DO_FUN( do_quest		);
DECLARE_DO_FUN( do_scan			);
DECLARE_DO_FUN( do_scribe		);
DECLARE_DO_FUN( do_bank			);
DECLARE_DO_FUN( do_mclass		);
DECLARE_DO_FUN( do_bounty		);
DECLARE_DO_FUN( do_butcher		);
DECLARE_DO_FUN( do_skin			);
DECLARE_DO_FUN( do_finger		);
DECLARE_DO_FUN( do_forage		);
DECLARE_DO_FUN( do_mine			);
DECLARE_DO_FUN( do_cook			);
DECLARE_DO_FUN( do_dig			);
DECLARE_DO_FUN( do_bury			);
DECLARE_DO_FUN( do_prospect		);
DECLARE_DO_FUN( do_plant		);
DECLARE_DO_FUN( do_palm_strike	);
//DECLARE_DO_FUN( do_grank		);
DECLARE_DO_FUN( do_craft		);
DECLARE_DO_FUN( do_stun_fist	);
DECLARE_DO_FUN(	do_nudge		);
DECLARE_DO_FUN( do_noexp		);
DECLARE_DO_FUN( do_shipstat		);
DECLARE_DO_FUN( do_climb		);
DECLARE_DO_FUN(	do_imbue		);
DECLARE_DO_FUN( do_energy		);
DECLARE_DO_FUN( do_tailor		);	
DECLARE_DO_FUN( do_blacksmith	);
DECLARE_DO_FUN( do_jewelcraft	);
DECLARE_DO_FUN( do_blind		);
DECLARE_DO_FUN( do_faction		);
DECLARE_DO_FUN( do_brew			);

DECLARE_DO_FUN( do_earclap		);
DECLARE_DO_FUN( do_kidney_punch	);
DECLARE_DO_FUN( do_gouge		);

//Extended immortal functions - Upro.
DECLARE_DO_FUN( do_skillstat	);
DECLARE_DO_FUN( do_spellstat	);
DECLARE_DO_FUN( do_addhours		);
DECLARE_DO_FUN( do_addxp		);
DECLARE_DO_FUN( do_addrep		);
DECLARE_DO_FUN( do_addmin		);
DECLARE_DO_FUN( do_addguildpoints);
DECLARE_DO_FUN( do_pretitle		);
DECLARE_DO_FUN( do_users		);
DECLARE_DO_FUN( do_checkweaps	);
DECLARE_DO_FUN( do_checkmobs	);
DECLARE_DO_FUN( do_email_pw		);
DECLARE_DO_FUN( do_ireset		);
DECLARE_DO_FUN( do_rename		);
DECLARE_DO_FUN( do_slist		);
DECLARE_DO_FUN( do_at_all		);
DECLARE_DO_FUN( do_ftick		);
DECLARE_DO_FUN( do_checktype	);
DECLARE_DO_FUN( do_autodig		);
DECLARE_DO_FUN( do_rat			);


/* Ship functions */
DECLARE_DO_FUN( do_cannon		); //fire a cannon!
DECLARE_DO_FUN( do_embark		); //enter ship.
DECLARE_DO_FUN( do_disembark	);

//DECLARE_DO_FUN( do_pluck		); //pick someone up. (aerial creatures only)
DECLARE_DO_FUN( do_anchor		); //lower the anchor.
DECLARE_DO_FUN( do_raise		); //raise the anchor.
DECLARE_DO_FUN( do_dock 		); //dock the ship.
DECLARE_DO_FUN( do_shipstat		); //show status/locale/health/etc of ship.
//DECLARE_DO_FUN( do_buyship  	);
DECLARE_DO_FUN(	do_sail			);

DECLARE_DO_FUN( do_traps		);
DECLARE_DO_FUN( do_repair		);
DECLARE_DO_FUN( do_recharge		);
DECLARE_DO_FUN( do_engage		);
DECLARE_DO_FUN( do_bandage		);
DECLARE_DO_FUN( do_identify		);
DECLARE_DO_FUN( do_lore			);
DECLARE_DO_FUN( do_material		);
DECLARE_DO_FUN( do_quest		);
DECLARE_DO_FUN( do_morgue		);
DECLARE_DO_FUN( do_gather		);
DECLARE_DO_FUN( do_addapply		);
DECLARE_DO_FUN( do_doublexp		);
DECLARE_DO_FUN( do_double_skill	);
DECLARE_DO_FUN( do_double_gold	);
DECLARE_DO_FUN( do_gameinfo		);
DECLARE_DO_FUN( do_experience	);
DECLARE_DO_FUN( do_explored		);
DECLARE_DO_FUN( do_entrance		);
DECLARE_DO_FUN( do_push			);
DECLARE_DO_FUN( do_pull			);
DECLARE_DO_FUN( do_pry			);
DECLARE_DO_FUN( do_press		);	
DECLARE_DO_FUN( do_map			);

//language crap - Upro
DECLARE_DO_FUN( do_common       );
DECLARE_DO_FUN( do_human        );
DECLARE_DO_FUN( do_dwarvish     );
DECLARE_DO_FUN( do_elvish       );
DECLARE_DO_FUN( do_gnomish      );
DECLARE_DO_FUN( do_goblin       );
DECLARE_DO_FUN( do_orcish       );
DECLARE_DO_FUN( do_ogre         );
DECLARE_DO_FUN( do_drow         );
DECLARE_DO_FUN( do_kobold       );
DECLARE_DO_FUN( do_trollish     );
DECLARE_DO_FUN( do_hobbit       );
DECLARE_DO_FUN( do_lstat        );      /* language stat */
DECLARE_DO_FUN( do_lset         );      /* Language set */
DECLARE_DO_FUN( do_learnLang    );      /* learn a language */
DECLARE_DO_FUN( do_speak        );      /* Select a language to speak */
DECLARE_DO_FUN( do_arena		);
DECLARE_DO_FUN( do_bank  		);
DECLARE_DO_FUN( do_woodcut		);
DECLARE_DO_FUN( do_history		);

DECLARE_DO_FUN( do_mxp );
DECLARE_DO_FUN( do_addchange    );
DECLARE_DO_FUN( do_changes      );
DECLARE_DO_FUN( do_enchant_list	);
DECLARE_DO_FUN( do_gweather		);
DECLARE_DO_FUN( do_create		);
DECLARE_DO_FUN (do_note		);
DECLARE_DO_FUN (do_board	);
DECLARE_DO_FUN (do_last_name);
DECLARE_DO_FUN (do_land);
DECLARE_DO_FUN (do_capture);
DECLARE_DO_FUN (do_new_spells);
DECLARE_DO_FUN (do_census);
DECLARE_DO_FUN (do_repstat);



DECLARE_DO_FUN (do_last);
DECLARE_DO_FUN (do_expertise);
DECLARE_DO_FUN (do_sharpen);
DECLARE_DO_FUN (do_achievements);
DECLARE_DO_FUN (do_rwhere);

DECLARE_DO_FUN (do_autoscroll);
DECLARE_DO_FUN (do_autodrink);
DECLARE_DO_FUN (do_autoeat);
DECLARE_DO_FUN (do_autosheathe);

DECLARE_DO_FUN (do_addtwitter);
DECLARE_DO_FUN (do_twitters);


DECLARE_DO_FUN (do_oload);



DECLARE_DO_FUN (do_peek);
DECLARE_DO_FUN (do_fly);
DECLARE_DO_FUN (do_mercy);
DECLARE_DO_FUN (do_vuln);
DECLARE_DO_FUN (do_rank);
DECLARE_DO_FUN (do_logout);

DECLARE_DO_FUN (do_eqcondition);
DECLARE_DO_FUN (do_beeptell);
DECLARE_DO_FUN (do_spellup);
DECLARE_DO_FUN (do_gore);
DECLARE_DO_FUN (do_vault);
DECLARE_DO_FUN (do_info);
DECLARE_DO_FUN (do_timestamp);