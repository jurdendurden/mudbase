/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */


/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\r\n" \
		"     Port a ROM 2.4 v1.8\r\n"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\r\n" \
                "     Modified for use with ROM 2.3\r\n"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\r\n" \
                "     Modificado para uso en ROM 2.4b6\r\n"	\
                "     Por Ivan Toledo (itoledo@ctcreuna.cl)\r\n"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\r\n" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\r\n" \
		"     Version actual : 1.8 - Sep 8, 1998\r\n"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"



/*
 * New typedefs.
 */
typedef	bool OLC_FUN		args( ( CHAR_DATA *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun


/* Command procedures needed ROM OLC */
DECLARE_DO_FUN(    do_help    );
DECLARE_SPELL_FUN( spell_null );



/*
 * Interpreter Prototypes
 */
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );
void    redit           args( ( CHAR_DATA *ch, char *argument ) );
void    medit           args( ( CHAR_DATA *ch, char *argument ) );
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );
void	mpedit			args( ( CHAR_DATA *ch, char *argument ) );
void    hedit          	args( ( CHAR_DATA *ch, char *argument ) );
void    opedit          args( ( CHAR_DATA *ch, char *argument ) );
void    rpedit          args( ( CHAR_DATA *ch, char *argument ) );
void 	nedit			args( ( CHAR_DATA *ch, char *argument ) );
void	qedit			args( ( CHAR_DATA *ch, char *argument ));

/*
 * OLC Constants
 */
#define MAX_MOB	1		/* Default maximum number for resetting mobs */



/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
    char * const	name;
    OLC_FUN *		olc_fun;
};



/*
 * Structure for an OLC editor startup command.
 */
struct	editor_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};



/*
 * Utils.
 */
AREA_DATA *get_vnum_area	args ( ( int vnum ) );
AREA_DATA *get_area_data	args ( ( int vnum ) );
HELP_DATA * help_lookup	    args( (char *) );
HELP_AREA * had_lookup	    args( (char *) );
int flag_value			args ( ( const struct flag_type *flag_table,
				         char *argument) );
char *flag_string		args ( ( const struct flag_type *flag_table,
				         int bits ) );
void add_reset			args ( ( ROOM_INDEX_DATA *room, 
				         RESET_DATA *pReset, int index ) );



/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];
extern const struct olc_cmd_type	mpedit_table[];
extern const struct olc_cmd_type	hedit_table[];
extern const struct olc_cmd_type        opedit_table[];
extern const struct olc_cmd_type        rpedit_table[];
extern const struct olc_cmd_type        nedit_table[];
extern const struct olc_cmd_type		qedit_table[];

/*
 * Editor Commands.
 */
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_mpedit		);
DECLARE_DO_FUN( do_hedit		);
DECLARE_DO_FUN( do_opedit       );
DECLARE_DO_FUN( do_rpedit       ); 
DECLARE_DO_FUN( do_nedit        ); 
DECLARE_DO_FUN( do_qedit		);
/*
 * General Functions
 */
bool show_commands		args ( ( CHAR_DATA *ch, char *argument ) );
bool show_help			args ( ( CHAR_DATA *ch, char *argument ) );
bool edit_done			args ( ( CHAR_DATA *ch ) );
bool show_version		args ( ( CHAR_DATA *ch, char *argument ) );



/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show			);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_name			);
DECLARE_OLC_FUN( aedit_file			);
DECLARE_OLC_FUN( aedit_age			);
/* DECLARE_OLC_FUN( aedit_recall	);       ROM OLC */
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_vnum			);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_uvnum		);
DECLARE_OLC_FUN( aedit_credits		);
DECLARE_OLC_FUN( aedit_level		);
DECLARE_OLC_FUN( aedit_flags		);
DECLARE_OLC_FUN( aedit_climate		);
DECLARE_OLC_FUN( aedit_shiploc		);
//DECLARE_OLC_FUN( aedit_qlist		);
DECLARE_OLC_FUN( aedit_allsect		);
DECLARE_OLC_FUN( aedit_allheal		);
DECLARE_OLC_FUN( aedit_allmana		);
DECLARE_OLC_FUN( aedit_resource		);
DECLARE_OLC_FUN( aedit_faction		);
DECLARE_OLC_FUN( aedit_history		);
DECLARE_OLC_FUN( aedit_type			);
DECLARE_OLC_FUN( aedit_plane		);

/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN( redit_show		);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name		);
DECLARE_OLC_FUN( redit_desc		);
DECLARE_OLC_FUN( redit_ed		);
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_east		);
DECLARE_OLC_FUN( redit_west		);
DECLARE_OLC_FUN( redit_up		);
DECLARE_OLC_FUN( redit_down		);
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_rlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_heal		);
DECLARE_OLC_FUN( redit_mana		);
DECLARE_OLC_FUN( redit_clan		);
DECLARE_OLC_FUN( redit_owner		);
DECLARE_OLC_FUN( redit_room		);
DECLARE_OLC_FUN( redit_sector		);
DECLARE_OLC_FUN( redit_copy		);
DECLARE_OLC_FUN( redit_addrprog		);
DECLARE_OLC_FUN( redit_delrprog		);
DECLARE_OLC_FUN( redit_x			);
DECLARE_OLC_FUN( redit_y			);
DECLARE_OLC_FUN( redit_z			);
/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN( oedit_show				);
DECLARE_OLC_FUN( oedit_create			);
DECLARE_OLC_FUN( oedit_name				);
DECLARE_OLC_FUN( oedit_short			);
DECLARE_OLC_FUN( oedit_long				);
DECLARE_OLC_FUN( oedit_addaffect		);
DECLARE_OLC_FUN( oedit_addapply			);
DECLARE_OLC_FUN( oedit_delaffect		);
DECLARE_OLC_FUN( oedit_value0			);
DECLARE_OLC_FUN( oedit_value1			);
DECLARE_OLC_FUN( oedit_value2			);
DECLARE_OLC_FUN( oedit_value3			);
DECLARE_OLC_FUN( oedit_value4			);  /* ROM */
DECLARE_OLC_FUN( oedit_weight			);
DECLARE_OLC_FUN( oedit_cost				);
DECLARE_OLC_FUN( oedit_ed				);
DECLARE_OLC_FUN( oedit_quality			);
DECLARE_OLC_FUN( oedit_extra            );  /* ROM */
DECLARE_OLC_FUN( oedit_extra2           );  // Upro
DECLARE_OLC_FUN( oedit_wear             );  /* ROM */
DECLARE_OLC_FUN( oedit_type             );  /* ROM */
DECLARE_OLC_FUN( oedit_affect           );  /* ROM */
DECLARE_OLC_FUN( oedit_material			);  /* ROM */
DECLARE_OLC_FUN( oedit_level            );  /* ROM */
DECLARE_OLC_FUN( oedit_condition        );  /* ROM */
DECLARE_OLC_FUN( oedit_copy				);
DECLARE_OLC_FUN( oedit_addoprog			);
DECLARE_OLC_FUN( oedit_deloprog			);
DECLARE_OLC_FUN( oedit_timer			);	
/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN( medit_show			);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_name			);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long			);
DECLARE_OLC_FUN( medit_shop			);
DECLARE_OLC_FUN( medit_desc			);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_spec			);

DECLARE_OLC_FUN( medit_sex			);  /* ROM */
DECLARE_OLC_FUN( medit_act			);  /* ROM */
DECLARE_OLC_FUN( medit_act2			);
DECLARE_OLC_FUN( medit_affect		);  /* ROM */
DECLARE_OLC_FUN( medit_ac			);  /* ROM */
DECLARE_OLC_FUN( medit_form			);  /* ROM */
DECLARE_OLC_FUN( medit_part			);  /* ROM */
DECLARE_OLC_FUN( medit_imm			);  /* ROM */
DECLARE_OLC_FUN( medit_res			);  /* ROM */
DECLARE_OLC_FUN( medit_vuln			);  /* ROM */
DECLARE_OLC_FUN( medit_material		);  /* ROM */
DECLARE_OLC_FUN( medit_off			);  /* ROM */
DECLARE_OLC_FUN( medit_size			);  /* ROM */
DECLARE_OLC_FUN( medit_hitdice		);  /* ROM */
DECLARE_OLC_FUN( medit_manadice		);  /* ROM */
DECLARE_OLC_FUN( medit_damdice		);  /* ROM */
DECLARE_OLC_FUN( medit_race			);  /* ROM */
DECLARE_OLC_FUN( medit_position		);  /* ROM */
DECLARE_OLC_FUN( medit_gold			);  /* ROM */
DECLARE_OLC_FUN( medit_hitroll		);  /* ROM */
DECLARE_OLC_FUN( medit_damtype		);  /* ROM */
DECLARE_OLC_FUN( medit_group		);  /* ROM */
DECLARE_OLC_FUN( medit_addmprog		);  /* ROM */
DECLARE_OLC_FUN( medit_delmprog		);  /* ROM */
DECLARE_OLC_FUN( medit_copy			);
DECLARE_OLC_FUN( medit_autoac		);
DECLARE_OLC_FUN( medit_autoset		);
DECLARE_OLC_FUN( medit_entrance		);



/* Mobprog editor */
DECLARE_OLC_FUN( mpedit_create		);
DECLARE_OLC_FUN( mpedit_code		);
DECLARE_OLC_FUN( mpedit_show		);
DECLARE_OLC_FUN( mpedit_list		);

/* Editor de helps */
DECLARE_OLC_FUN( hedit_keyword		);
DECLARE_OLC_FUN( hedit_keywords		);
DECLARE_OLC_FUN( hedit_text			);
DECLARE_OLC_FUN( hedit_new			);
DECLARE_OLC_FUN( hedit_level		);
DECLARE_OLC_FUN( hedit_delete		);
DECLARE_OLC_FUN( hedit_show			);
DECLARE_OLC_FUN( hedit_list			);
DECLARE_OLC_FUN( hedit_make			);
DECLARE_OLC_FUN( hedit_desc			);

DECLARE_OLC_FUN( 	nedit_new		);
DECLARE_OLC_FUN( 	nedit_to		);
DECLARE_OLC_FUN(	nedit_subject	);
DECLARE_OLC_FUN(	nedit_expire	);
DECLARE_OLC_FUN(	nedit_text		);
DECLARE_OLC_FUN(	nedit_send		);
DECLARE_OLC_FUN(	nedit_forward	);
DECLARE_OLC_FUN(	nedit_show		);
DECLARE_OLC_FUN(	nedit_clear		);
DECLARE_OLC_FUN(	nedit_type		);
DECLARE_OLC_FUN(	nedit_list		);

//Quest editor
DECLARE_OLC_FUN(	qedit_create		);
DECLARE_OLC_FUN(	qedit_name		);
DECLARE_OLC_FUN(	qedit_min_level		);
DECLARE_OLC_FUN(	qedit_max_level		);
DECLARE_OLC_FUN(	qedit_gold_reward	);
DECLARE_OLC_FUN(	qedit_xp_reward	);
DECLARE_OLC_FUN(	qedit_gp_reward	);
DECLARE_OLC_FUN(	qedit_start		);
DECLARE_OLC_FUN(	qedit_end		);
DECLARE_OLC_FUN(	qedit_show		);
DECLARE_OLC_FUN(	qedit_type		);
DECLARE_OLC_FUN(	qedit_qlist		);
DECLARE_OLC_FUN(	qedit_level		);
DECLARE_OLC_FUN(	qedit_class		);
DECLARE_OLC_FUN(	qedit_desc		);
DECLARE_OLC_FUN(	qedit_objtarget	);
DECLARE_OLC_FUN(	qedit_mobtarget	);
DECLARE_OLC_FUN(	qedit_mattarget	);
DECLARE_OLC_FUN(	qedit_help1		);
DECLARE_OLC_FUN(	qedit_help2		);
DECLARE_OLC_FUN(	qedit_help3		);
DECLARE_OLC_FUN(	qedit_obj_multiples);
DECLARE_OLC_FUN(	qedit_mob_multiples);
DECLARE_OLC_FUN(	qedit_mat_multiples);
DECLARE_OLC_FUN(	qedit_herb_multiples);


/*
 * Macros
 */
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_MPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_OPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_RPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_HELP(Ch, Help)     ( Help = (HELP_DATA *)Ch->desc->pEdit )
#define EDIT_NOTE(Ch, Note)		( Note = (NOTE_DATA *)Ch->desc->pEdit )
#define EDIT_QUEST(Ch, Quest)	( Quest = (QUEST_INDEX_DATA *)Ch->desc->pEdit )

/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA	*new_reset_data		args ( ( void ) );
void		free_reset_data		args ( ( RESET_DATA *pReset ) );
AREA_DATA	*new_area		args ( ( void ) );
void		free_area		args ( ( AREA_DATA *pArea ) );
QUEST_INDEX_DATA  *new_quest		args ( ( void ) );
void		free_quest		args ( ( QUEST_INDEX_DATA *pQuest ) ); //fix
EXIT_DATA	*new_exit		args ( ( void ) );
void		free_exit		args ( ( EXIT_DATA *pExit ) );
ED 		*new_extra_descr	args ( ( void ) );
void		free_extra_descr	args ( ( ED *pExtra ) );
ROOM_INDEX_DATA *new_room_index		args ( ( void ) );
void		free_room_index		args ( ( ROOM_INDEX_DATA *pRoom ) );
AFFECT_DATA	*new_affect		args ( ( void ) );
void		free_affect		args ( ( AFFECT_DATA* pAf ) );
SHOP_DATA	*new_shop		args ( ( void ) );
void		free_shop		args ( ( SHOP_DATA *pShop ) );
OBJ_INDEX_DATA	*new_obj_index		args ( ( void ) );
void		free_obj_index		args ( ( OBJ_INDEX_DATA *pObj ) );
MOB_INDEX_DATA	*new_mob_index		args ( ( void ) );
void		free_mob_index		args ( ( MOB_INDEX_DATA *pMob ) );
HELP_DATA *new_help args ( (void) );
void free_help  args( ( HELP_DATA * pHelp));
void save_helps args( (void) );
#undef	ED

void		show_liqlist		args ( ( CHAR_DATA *ch ) );
void		show_damlist		args ( ( CHAR_DATA *ch ) );

char *		prog_type_to_name	args ( ( int type ) );
PROG_LIST      *new_mprog              args ( ( void ) );
void            free_mprog              args ( ( PROG_LIST *mp ) );
PROG_CODE	*new_mpcode		args ( (void) );
void		free_mpcode		args ( ( PROG_CODE *pMcode));
PROG_LIST	*new_oprog		args ( ( void ) );
void		free_oprog		args ( ( PROG_LIST *op ) );
PROG_LIST	*new_rprog		args ( ( void ) );
void		free_rprog		args ( ( PROG_LIST *rp ) );
PROG_CODE	*new_opcode		args ( ( void ) );
void		free_opcode		args ( ( PROG_CODE *pOcode ) );
PROG_CODE	*new_rpcode		args ( ( void ) );
void		free_rpcode		args ( ( PROG_CODE *pRcode ) );


/* Objprog editor */
DECLARE_OLC_FUN( opedit_create		);
DECLARE_OLC_FUN( opedit_code		);
DECLARE_OLC_FUN( opedit_show		);
DECLARE_OLC_FUN( opedit_list		);
 
/* Roomprog editor */
DECLARE_OLC_FUN( rpedit_create		);
DECLARE_OLC_FUN( rpedit_code		);
DECLARE_OLC_FUN( rpedit_show		);
DECLARE_OLC_FUN( rpedit_list		);

