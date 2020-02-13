/***************************************************************************
 *  File: olc.c                                                            *
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
#include "olc.h"
#include "recycle.h"



/*
 * Local functions.
 */
AREA_DATA *get_area_data args ((int vnum));

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor (DESCRIPTOR_DATA * d)
{
    switch (d->editor)
    {
        case ED_AREA:
            aedit (d->character, d->incomm);
            break;
        case ED_ROOM:
            redit (d->character, d->incomm);
            break;
        case ED_OBJECT:
            oedit (d->character, d->incomm);
            break;
        case ED_MOBILE:
            medit (d->character, d->incomm);
            break;
        case ED_MPCODE:
            mpedit (d->character, d->incomm);
            break;
		case ED_OPCODE:
			opedit( d->character, d->incomm );
			break;
		case ED_RPCODE:
			rpedit( d->character, d->incomm );
			break;
        case ED_HELP:
            hedit (d->character, d->incomm);
            break;
		case ED_NOTE:
			nedit (d->character, d->incomm);
			break;
		case ED_QUEST:
			qedit (d->character, d->incomm);
			break;
        default:
            return FALSE;
    }
    return TRUE;
}



char *olc_ed_name (CHAR_DATA * ch)
{
    static char buf[10];

    buf[0] = '\0';
    switch (ch->desc->editor)
    {
        case ED_AREA:
            sprintf (buf, "AEdit");
            break;
        case ED_ROOM:
            sprintf (buf, "REdit");
            break;
        case ED_OBJECT:
            sprintf (buf, "OEdit");
            break;
        case ED_MOBILE:
            sprintf (buf, "MEdit");
            break;
        case ED_MPCODE:
            sprintf (buf, "MPEdit");
            break;
		case ED_OPCODE:
			sprintf( buf, "OPEdit" );
			break;
		case ED_RPCODE:
			sprintf( buf, "RPEdit" );
			break;
        case ED_HELP:
            sprintf (buf, "HEdit");
            break;
		case ED_NOTE:
			sprintf (buf, "NEdit");
			break;
		case ED_QUEST:
			sprintf (buf, "QuestEdit");
			break;
        default:
            sprintf (buf, " ");
            break;
    }
    return buf;
}



char *olc_ed_vnum (CHAR_DATA * ch)
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    PROG_CODE *pMprog;
	PROG_CODE *pOprog;
	PROG_CODE *pRprog;
    HELP_DATA *pHelp;
	NOTE_DATA *pNote; 
	QUEST_INDEX_DATA *pQuest;
    static char buf[MIL];

    buf[0] = '\0';
    switch (ch->desc->editor)
    {
        case ED_AREA:
            pArea = (AREA_DATA *) ch->desc->pEdit;
            sprintf (buf, "%d", pArea ? pArea->vnum : 0);
            break;
        case ED_ROOM:
            pRoom = ch->in_room;
            sprintf (buf, "%ld", pRoom ? pRoom->vnum : 0);
            break;
        case ED_OBJECT:
            pObj = (OBJ_INDEX_DATA *) ch->desc->pEdit;
            sprintf (buf, "%ld", pObj ? pObj->vnum : 0);
            break;
        case ED_MOBILE:
            pMob = (MOB_INDEX_DATA *) ch->desc->pEdit;
            sprintf (buf, "%ld", pMob ? pMob->vnum : 0);
            break;
        case ED_MPCODE:
            pMprog = (PROG_CODE *) ch->desc->pEdit;
            sprintf (buf, "%ld", pMprog ? pMprog->vnum : 0);
            break;
		case ED_OPCODE:
			pOprog = (PROG_CODE *)ch->desc->pEdit;
			sprintf( buf, "%ld", pOprog ? pOprog->vnum : 0 );
			break;
		case ED_RPCODE:
			pRprog = (PROG_CODE *)ch->desc->pEdit;
			sprintf( buf, "%ld", pRprog ? pRprog->vnum : 0 );
			break;
        case ED_HELP:
            pHelp = (HELP_DATA *) ch->desc->pEdit;
            sprintf (buf, "%s", pHelp ? pHelp->keyword : "");
            break;
		case ED_NOTE:
			pNote = (NOTE_DATA *) ch->desc->pEdit;
			sprintf (buf, "%s", pNote ? pNote->subject : "");
			break;
		case ED_QUEST:
			pQuest = (QUEST_INDEX_DATA *) ch->desc->pEdit;
			sprintf (buf, "%s", pQuest ? pQuest->name : "");
			break;
        default:
            sprintf (buf, " ");
            break;
    }

    return buf;
}



/*****************************************************************************
 Name:        show_olc_cmds
 Purpose:    Format up the commands from given table.
 Called by:    show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds (CHAR_DATA * ch, const struct olc_cmd_type *olc_table)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int cmd;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (cmd = 0; olc_table[cmd].name != NULL; cmd++)
    {
        sprintf (buf, "%-15.15s", olc_table[cmd].name);
        strcat (buf1, buf);
        if (++col % 5 == 0)
            strcat (buf1, "\r\n");
    }

    if (col % 5 != 0)
        strcat (buf1, "\r\n");

    SEND (buf1, ch);
    return;
}



/*****************************************************************************
 Name:        show_commands
 Purpose:    Display all olc commands.
 Called by:    olc interpreters.
 ****************************************************************************/
bool show_commands (CHAR_DATA * ch, char *argument)
{
    switch (ch->desc->editor)
    {
        case ED_AREA:
            show_olc_cmds (ch, aedit_table);
            break;
        case ED_ROOM:
            show_olc_cmds (ch, redit_table);
            break;
        case ED_OBJECT:
            show_olc_cmds (ch, oedit_table);
            break;
        case ED_MOBILE:
            show_olc_cmds (ch, medit_table);
            break;
        case ED_MPCODE:
            show_olc_cmds (ch, mpedit_table);
            break;
		case ED_OPCODE:
				show_olc_cmds( ch, opedit_table );
				break;
		case ED_RPCODE:
				show_olc_cmds( ch, rpedit_table );
				break;
        case ED_HELP:
            show_olc_cmds (ch, hedit_table);
            break;
		case ED_NOTE:
			show_olc_cmds (ch, nedit_table);
			break;
		case ED_QUEST:
			show_olc_cmds (ch, qedit_table);
			break;
    }

    return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] = {
/*  {   command        function    }, */

    {"age", 		aedit_age},
    {"builder", 	aedit_builder},    /* s removed -- Hugin */
    {"commands",	show_commands},
    {"create", 		aedit_create},
    {"filename", 	aedit_file},
    {"name", 		aedit_name},
/*  {   "recall",    aedit_recall    },   ROM OLC */
    {"reset", 		aedit_reset},
    {"security", 	aedit_security},
    {"show", 		aedit_show},
    {"vnum", 		aedit_vnum},
    {"lvnum", 		aedit_lvnum},
    {"uvnum", 		aedit_uvnum},
	{"level",   	aedit_level},
    {"credits", 	aedit_credits},
	{"rlist", 		redit_rlist},
    {"olist", 		redit_olist},
	{"mlist", 		redit_mlist},
    {"?", 			show_help},
    {"version", 	show_version},
	{"autoac",		medit_autoac},
	{"flag", 		aedit_flags},
	{"climate",		aedit_climate},
	{"shiploc",		aedit_shiploc},
	{"qlist",		qedit_qlist},
	{"allsect",		aedit_allsect},
	{"allheal",		aedit_allheal},
	{"allmana",		aedit_allmana},
	{"resource",	aedit_resource},
	{"primary"	,	aedit_resource},
	{"faction",		aedit_faction},
	{"history",		aedit_history},
	{"type",		aedit_type},
	{"plane",	    aedit_plane},
	
    {NULL, 0,}
};



const struct olc_cmd_type redit_table[] = {
/*  {   command        function    }, */

    {"commands", show_commands},
    {"create", redit_create},
    {"desc", redit_desc},
    {"ed", redit_ed},
    {"format", redit_format},
    {"name", redit_name},
    {"show", redit_show},
    {"heal", redit_heal},
    {"mana", redit_mana},
    {"clan", redit_clan},

    {"north", redit_north},
    {"south", redit_south},
    {"east", redit_east},
    {"west", redit_west},
    {"up", redit_up},
    {"down", redit_down},

    /* New reset commands. */
    {"mreset", redit_mreset},
    {"oreset", redit_oreset},
    {"mlist", redit_mlist},
    {"rlist", redit_rlist},
    {"olist", redit_olist},
	{"qlist", qedit_qlist},
    {"mshow", redit_mshow},
    {"oshow", redit_oshow},
    {"owner", redit_owner},
    {"room", redit_room},
    {"sector", redit_sector},
    {"copy", redit_copy},
	{	"addrprog",	redit_addrprog	},
    {	"delrprog",	redit_delrprog	},
	
    {"?", show_help},
    {"version", show_version},
	
	{"qlist",		qedit_qlist},
	{"allsect",		aedit_allsect},
	{"allheal",		aedit_allheal},
	{"allmana",		aedit_allmana},
	{"x",			redit_x},
	{"y",			redit_y},
	{"z",			redit_z},
    {NULL, 0,}
};



const struct olc_cmd_type oedit_table[] = {
/*  {   command        function    }, */

    {"addaffect", oedit_addaffect},
    {"addapply", oedit_addapply},
    {"commands", show_commands},
    {"cost", oedit_cost},
    {"create", oedit_create},
    {"delaffect", oedit_delaffect},
    {"ed", oedit_ed},
    {"long", oedit_long},
    {"name", oedit_name},
    {"short", oedit_short},
    {"show", oedit_show},
    {"v0", oedit_value0},
    {"v1", oedit_value1},
    {"v2", oedit_value2},
    {"v3", oedit_value3},
    {"v4", oedit_value4},        /* ROM */
    {"weight", oedit_weight},

    {"extra", oedit_extra},        /* ROM */
	{"extra2", oedit_extra2},
    {"wear", oedit_wear},        /* ROM */
    {"type", oedit_type},        /* ROM */
    {"material", oedit_material},    /* ROM */
    {"level", oedit_level},        /* ROM */
    {"condition", oedit_condition},    /* ROM */
	{"quality", oedit_quality},
	{"olist", 	redit_olist},
	{"mlist",	redit_mlist},
	{"copy",	oedit_copy},
	{	"addoprog",	oedit_addoprog	},
    {	"deloprog",	oedit_deloprog	},
	
    {"?", show_help},
    {"version", show_version},
	
	{"qlist",		qedit_qlist},
	{"timer",		oedit_timer},
	

    {NULL, 0,}
};



const struct olc_cmd_type medit_table[] = {
/*  {   command        function    }, */

    {"alignment", medit_align},
    {"commands", show_commands},
    {"create", medit_create},
    {"desc", medit_desc},
    {"level", medit_level},
    {"long", medit_long},
    {"name", medit_name},
    {"shop", medit_shop},
    {"short", medit_short},
    {"show", medit_show},
    {"spec", medit_spec},

    {"sex", medit_sex},            /* ROM */
    {"act", medit_act},            /* ROM */
	{"act2",medit_act2},
    {"affect", medit_affect},    /* ROM */
    {"armor", medit_ac},        /* ROM */
    {"form", medit_form},        /* ROM */
    {"part", medit_part},        /* ROM */
    {"imm", medit_imm},            /* ROM */
    {"res", medit_res},            /* ROM */
    {"vuln", medit_vuln},        /* ROM */
    {"material", medit_material},    /* ROM */
    {"off", medit_off},            /* ROM */
    {"size", medit_size},        /* ROM */
    {"hitdice", medit_hitdice},    /* ROM */
    {"manadice", medit_manadice},    /* ROM */
    {"damdice", medit_damdice},    /* ROM */
    {"race", medit_race},        /* ROM */
    {"position", medit_position},    /* ROM */
    {"wealth", medit_gold},        /* ROM */
    {"hitroll", medit_hitroll},    /* ROM */
    {"damtype", medit_damtype},    /* ROM */
    {"group", medit_group},        /* ROM */
    {"addmprog", medit_addmprog},    /* ROM */
    {"delmprog", medit_delmprog},    /* ROM */
	{"mlist", 	redit_mlist},
	{"olist",   redit_olist},
	{"autoac",	medit_autoac},
	{"copy", medit_copy},	
    {"?", show_help},
    {"version", show_version},
	{"autoset", medit_autoset},
	{"entrance", medit_entrance},
	{"walk", medit_entrance},
	{"qlist",		qedit_qlist},

    {NULL, 0,}
};


const struct olc_cmd_type hedit_table[] = {
/*    {    command        function    }, */

    {"keyword", 		hedit_keywords},
    {"text", 			hedit_desc},
    {"create", 			hedit_make},
    {"level", 			hedit_level},
    {"commands", 		show_commands},
    {"delete", 			hedit_delete},
    {"list", 			hedit_list},
    {"show", 			hedit_show},
    {"?", 				show_help},

    {NULL, 0}
};

const struct olc_cmd_type nedit_table[] = {
	{"new",				nedit_new},
	{"to",				nedit_to},
	{"subject",			nedit_subject},
	{"expire",			nedit_expire},
	{"text",			nedit_text},
	{"send",			nedit_send},
	{"forward",			nedit_forward},
	{"show",			nedit_show},
	{"clear",			nedit_clear},
	{"type",			nedit_type},
	{"list",			nedit_list},
	
	{NULL,	0}
};


const struct olc_cmd_type qedit_table[] = {
	{"create",			qedit_create},
	{"name",			qedit_name},
	{"start",			qedit_start},
	{"end",				qedit_end},
	{"gold",			qedit_gold_reward},
	{"gp",				qedit_gp_reward},
	{"xp",				qedit_xp_reward},
	{"type",			qedit_type},
	{"show",			qedit_show},
	{"qlist",			qedit_qlist},
	{"minlevel",		qedit_min_level},
	{"maxlevel",		qedit_max_level},
	{"class",			qedit_class},
	{"desc",			qedit_desc},	
	{"objtarget",		qedit_objtarget},
	{"mobtarget",		qedit_mobtarget},
	{"mattarget",		qedit_mattarget},
	{"help1",			qedit_help1},
	{"help2",			qedit_help2},
	{"help3",			qedit_help3},
	{"objmult",			qedit_obj_multiples},
	{"matmult",			qedit_mat_multiples},
	{"mobmult",			qedit_mob_multiples},
	{"herbmult",		qedit_herb_multiples},
	{"?", 				show_help},
	
	{NULL,	0}
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:        get_area_data
 Purpose:    Returns pointer to area with given vnum.
 Called by:    do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data (int vnum)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next)
    {
        if (pArea->vnum == vnum)
            return pArea;
    }

    return 0;
}



/*****************************************************************************
 Name:        edit_done
 Purpose:    Resets builder information on completion.
 Called by:    aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done (CHAR_DATA * ch)
{    
	switch (ch->desc->editor)
	{
		default: 		SEND ("Exitting OLC Editor.\r\n",ch); break;
		case ED_AREA: 	SEND ("Exitting Area Editor.\r\n",ch); break;
		case ED_ROOM:	SEND ("Exitting Room Editor.\r\n",ch); break;
		case ED_MOBILE:	SEND ("Exitting Mobile Editor.\r\n",ch); break;
		case ED_OBJECT:	SEND ("Exitting Object Editor.\r\n",ch); break;
		case ED_HELP:	SEND ("Exitting Help Editor.\r\n",ch); break;		
		case ED_NOTE:	
		{
			SEND ("Exitting Note Editor.\r\n",ch); 
			SEND ("Note cancelled!\r\n",ch);
			free_note (ch->pcdata->in_progress);
			ch->pcdata->in_progress = NULL;			
			break;
		}
		case ED_QUEST: SEND ("Exitting Quest Editor.\r\n",ch); break;
	}
	ch->desc->pEdit = NULL;
    ch->desc->editor = ED_NONE;
    return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;    

    EDIT_AREA (ch, pArea);
    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("AEdit:  Insufficient security to modify area.\r\n",
                      ch);
        edit_done (ch);
        return;
    }

    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }

    if (command[0] == '\0')
    {
        aedit_show (ch, argument);
        return;
    }
	/*
    if ((value = flag_value (area_flags, command)) != NO_FLAG)
    {
        TOGGLE_BIT (pArea->area_flags, value);

        SEND ("Flag toggled.\r\n", ch);
        return;
    }
	*/
    /* Search Table and Dispatch Command. */
    for (cmd = 0; aedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, aedit_table[cmd].name))
        {
            if ((*aedit_table[cmd].olc_fun) (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}

void qedit (CHAR_DATA * ch, char *argument)
{
    QUEST_INDEX_DATA *quest;   
    char arg[MSL];
    char command[MIL];
    int cmd;

    EDIT_QUEST (ch, quest);    

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (ch->level < 1)
    {        
        edit_done (ch);
        return;
    }

    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }

    if (command[0] == '\0')
    {
        qedit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; qedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, qedit_table[cmd].name))
        {            
			if ((*qedit_table[cmd].olc_fun) (ch, argument))
            {
                //SET_BIT (ch->in_room->area_flags, AREA_CHANGED);
                return;
            }			
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}



void nedit (CHAR_DATA * ch, char *argument)
{
    NOTE_DATA * note;    
    char arg[MSL];
    char command[MIL];
    int cmd;

    EDIT_NOTE (ch, note);    

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (ch->level < 1)
    {        
        edit_done (ch);
        return;
    }

    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }

    if (command[0] == '\0')
    {
        nedit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; nedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, nedit_table[cmd].name))
        {            
			return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}



/* Room Interpreter, called by do_redit. */
void redit (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    EDIT_ROOM (ch, pRoom);
    pArea = pRoom->area;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("REdit:  Insufficient security to modify room.\r\n",
                      ch);
        edit_done (ch);
        return;
    }

    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }

    if (command[0] == '\0')
    {
        redit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; redit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, redit_table[cmd].name))
        {
            if ((*redit_table[cmd].olc_fun) (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}



/* Object Interpreter, called by do_oedit. */
void oedit (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_OBJ (ch, pObj);
    pArea = pObj->area;

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("OEdit: Insufficient security to modify area.\r\n", ch);
        edit_done (ch);
        return;
    }

    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }

    if (command[0] == '\0')
    {
        oedit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; oedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, oedit_table[cmd].name))
        {
            if ((*oedit_table[cmd].olc_fun) (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}



/* Mobile Interpreter, called by do_medit. */
void medit (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int cmd;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_MOB (ch, pMob);
    pArea = pMob->area;

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("MEdit: Insufficient security to modify area.\r\n", ch);
        edit_done (ch);
        return;
    }

    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }

    if (command[0] == '\0')
    {
        medit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; medit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, medit_table[cmd].name))
        {
            if ((*medit_table[cmd].olc_fun) (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}




const struct editor_cmd_type editor_table[] = {
/*  {   command        function    }, */

    {"area", 		do_aedit},
    {"room", 		do_redit},
    {"object", 		do_oedit},
    {"mobile", 		do_medit},
    {"mpcode", 		do_mpedit},
	{"opcode",		do_opedit},
    {"rpcode",		do_rpedit},
    {"hedit", 		do_hedit},
	{"nedit",		do_nedit},
	{"qedit",		do_qedit},

    {NULL, 0,}
};


/* Entry point for all editors. */
void do_olc (CHAR_DATA * ch, char *argument)
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, command);

    if (command[0] == '\0')
    {
        do_help (ch, "olc");
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; editor_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, editor_table[cmd].name))
        {
            (*editor_table[cmd].do_fun) (ch, argument);
            return;
        }
    }

    /* Invalid command, send help. */
    do_help (ch, "olc");
    return;
}



/* Entry point for editing area_data. */
void do_aedit (CHAR_DATA * ch, char *argument)
{
    AREA_DATA *pArea;
    int value;
	char buf[MSL];
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    pArea = ch->in_room->area;

    argument = one_argument (argument, arg);

    if (is_number (arg))
    {
        value = atoi (arg);
        for (pArea = area_first; pArea; pArea = pArea->next)
		{
			if (pArea->vnum != value)
				continue;
			else
			{
				if (!IS_BUILDER (ch, pArea))
				{
					SEND ("Insufficient security to edit areas.\r\n", ch);
					return;
				}
				ch->desc->pEdit = (void *) pArea;
				sprintf(buf, "Editting %s.\r\n", pArea->name);
				SEND(buf, ch);
				ch->desc->editor = ED_AREA;
				return;				
			}
		}
		SEND ("That area does not seem to exist yet!\r\n",ch);
		return;
    }
    else if (!str_cmp (arg, "create"))
    {
        if (ch->pcdata->security < 9)
        {
            SEND ("AEdit : Insufficient security to create area.\r\n",
                          ch);
            return;
        }

        aedit_create (ch, "");
        ch->desc->editor = ED_AREA;
        return;
    }
	else if (strlen(arg) > 2 && !is_number(arg))
	{
		for (pArea = area_first; pArea; pArea = pArea->next)
		{		
			if (!str_cmp(pArea->name, arg))
			{
				if (!IS_BUILDER (ch, pArea))
				{
					SEND ("Insufficient security to edit areas.\r\n", ch);
					return;
				}
			
				ch->desc->pEdit = (void *) pArea;
				sprintf(buf, "Editting %s.\r\n", pArea->name);
				SEND(buf, ch);
				ch->desc->editor = ED_AREA;
				return;				
			}						
		}
		SEND ("AEdit: That area doesn't appear to exist.\r\n",ch);
		return;
	}
	else
	{
		if (!IS_BUILDER (ch, pArea))
		{
			SEND ("Insufficient security to edit areas.\r\n", ch);
			return;
		}
		
		ch->desc->pEdit = (void *) pArea;
		sprintf(buf, "Editting %s.\r\n", pArea->name);
		SEND(buf, ch);
		ch->desc->editor = ED_AREA;
	}	       
    return;
}



/* Entry point for editing room_index_data. */
void do_redit (CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *pRoom;
    char arg1[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    pRoom = ch->in_room;

    if (!str_cmp (arg1, "reset"))
    {                            /* redit reset */
        if (!IS_BUILDER (ch, pRoom->area))
        {
            SEND ("Insufficient security to modify room.\r\n", ch);
            return;
        }

        reset_room (pRoom);
        SEND ("Room reset.\r\n", ch);

        return;
    }
    else if (!str_cmp (arg1, "create"))
    {                            /* redit create <vnum> */
        if (argument[0] == '\0' || atoi (argument) == 0)
        {
            SEND ("Syntax:  edit room create [vnum]\r\n", ch);
            return;
        }

        if (redit_create (ch, argument))
        {                        /* pEdit == nuevo cuarto */
            ch->desc->editor = ED_ROOM;
            char_from_room (ch);
            char_to_room (ch, ch->desc->pEdit);
            SET_BIT (((ROOM_INDEX_DATA *) ch->desc->pEdit)->area->area_flags,
                     AREA_CHANGED);
        }

        return;
    }
    else if (!IS_NULLSTR (arg1))
    {                            /* redit <vnum> */
        pRoom = get_room_index (atoi (arg1));

        if (!pRoom)
        {
            SEND ("REdit : Nonexistant room.\r\n", ch);
            return;
        }

        if (!IS_BUILDER (ch, pRoom->area))
        {
            SEND ("REdit : Insufficient security to modify room.\r\n",
                          ch);
            return;
        }

        char_from_room (ch);
        char_to_room (ch, pRoom);
    }

    if (!IS_BUILDER (ch, pRoom->area))
    {
        SEND ("REdit : Insufficient security to modify room.\r\n",
                      ch);
        return;
    }

    ch->desc->pEdit = (void *) pRoom;
    ch->desc->editor = ED_ROOM;

    return;
}

void do_qedit (CHAR_DATA *ch, char *argument)
{
	QUEST_INDEX_DATA *pQuest;
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1))
    {
        value = atoi (arg1);
        if (!(pQuest = get_quest_index (value)))
        {
            SEND ("QEdit:  That quest does not exist.\r\n", ch);
            return;
        }

        if (!IS_BUILDER (ch, pQuest->area))
        {
            SEND ("Insufficient security to modify quests.\r\n", ch);
            return;
        }

        ch->desc->pEdit = (void *) pQuest;
        ch->desc->editor = ED_QUEST;
        return;
    }
    else
    {
        if (!str_cmp (arg1, "create"))
        {
            value = atoi (argument);
            if (argument[0] == '\0' || value == 0)
            {
                SEND ("Syntax:  quest create [vnum]\r\n", ch);
                return;
            }

            pArea = get_vnum_area (value);

            if (!pArea)
            {
                SEND
                    ("QEdit:  That vnum is not assigned an area.\r\n", ch);
                return;
            }

            if (!IS_BUILDER (ch, pArea))
            {
                SEND ("Insufficient security to modify quests.\r\n", ch);
                return;
            }

            if (qedit_create (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                ch->desc->editor = ED_QUEST;
            }
            return;
        }
		if (!str_cmp (arg1, "type"))
		{
			if (qedit_type (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "class"))
		{
			if (qedit_class (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		
		if (!str_cmp (arg1, "show"))
		{
			if (qedit_show (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "name"))
		{
			if (qedit_name (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "minlevel"))
		{
			if (qedit_min_level (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "maxlevel"))
		{
			if (qedit_max_level (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "start"))
		{
			if (qedit_start (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "end"))
		{
			if (qedit_end (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "gold"))
		{
			if (qedit_gold_reward (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "gp") || !str_cmp(arg1, "guild"))
		{
			if (qedit_gp_reward (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "xp") || !str_cmp(arg1, "exp"))
		{
			if (qedit_xp_reward (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "qlist"))
		{
			if (qedit_qlist (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "help1"))
		{
			if (qedit_help1 (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "help2"))
		{
			if (qedit_help2 (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
		if (!str_cmp (arg1, "help3"))
		{
			if (qedit_help3 (ch, argument))
			{
				ch->desc->editor = ED_QUEST;
				return;
			}
			else
				return;
		}
		
    }

    SEND ("QEdit:  There is no default quest to edit.\r\n", ch);
    return;
}


void do_nedit (CHAR_DATA * ch, char *argument)
{
    //NOTE_DATA * note;
    char arg1[MSL];
	char arg2[MSL];
	NOTE_DATA *p;
	int count = 0;
	time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

	if (arg1[0] == '\0')
	{
		char buf[200];
		
		count = 1;
		for (p = ch->pcdata->board->note_first; p ; p = p->next, count++)
		{
			if ((p->date_stamp > *last_note) && is_note_to(ch,p))
			{
				show_note_to_char (ch,p,count);
				/* Advance if new note is newer than the currently newest for that char */
				*last_note =  UMAX (*last_note, p->date_stamp);
				return;
			}
		}
		SEND ("No new notes in this board.\r\n",ch);
		
		if (next_board (ch))
			sprintf (buf, "Changed to next board, %s.\r\n", ch->pcdata->board->short_name);
		else
			sprintf (buf, "There are no more boards.\r\n");			
			
		SEND (buf,ch);
		
		//SEND ("Do what with a note?\r\n",ch);
		return;
	}
    
    if (!str_cmp (arg1, "new") || !str_cmp (arg1, "write"))
    {                           
        if (ch->desc->editor != ED_NOTE)
			if (nedit_new (ch, argument))
			{                       
				ch->desc->editor = ED_NOTE;  
				return;
			}
			else
			{
				SEND("Something went wrong starting the note.\r\n",ch);
				return;
			}		
		else
		{
			SEND("You're already writing a note!\r\n",ch);
			return;
		}
    }
	
	if (!str_cmp (arg1, "catch"))
	{
		/* Find last note */	
		for (p = ch->pcdata->board->note_first; p && p->next; p = p->next);
		
		if (!p)
			SEND ("Alas, there are no notes in that board.\r\n",ch);
		else
		{
			ch->pcdata->last_note[board_number(ch->pcdata->board)] = p->date_stamp;
			SEND ("All notes in this board caught up!\r\n",ch);
		}			
		return;
	}
	
	if (!str_cmp (arg1, "type"))
	{
		if (nedit_type (ch, argument))
		{
			ch->desc->editor = ED_NOTE;
			return;
		}
		else
			return;
	}
	
	if (!str_cmp (arg1, "show"))
	{
		if (nedit_show (ch, argument))
		{
			ch->desc->editor = ED_NOTE;
			return;
		}
		else
			return;
	}
    
	if (!str_cmp (arg1, "subject") || !str_cmp (arg1, "sub"))
	{
		if (nedit_subject (ch, arg2))
		{
			ch->desc->editor = ED_NOTE;
			return;
		}
		else
			return;
	}
	
	if (!str_cmp (arg1, "text") || !str_cmp (arg1, "edit"))
	{
		if (nedit_text (ch, argument))
		{
			ch->desc->editor = ED_NOTE;
			return;
		}
		else
			return;
	}
	
	if (!str_cmp (arg1, "to"))
	{
		if (nedit_to (ch, arg2))
		{
			ch->desc->editor = ED_NOTE;
			return;
		}
		else
			return;
	}
	
	if (!str_cmp (arg1, "clear"))
	{
		if (nedit_clear (ch, argument))
		{
			ch->desc->editor = ED_NONE;
			return;
		}
		else
			return;
	}
	
	if (!str_cmp (arg1, "send") || !str_cmp (arg1, "post"))
	{
		if (nedit_send (ch, argument))
		{
			ch->desc->editor = ED_NONE;
			return;
		}
		else
			return;
	}
	
	if (!str_cmp(arg1, "read")) /* 'note' or 'note read X' */
	{
		do_nread (ch, arg2);
		ch->desc->editor = ED_NONE;
		return;
	}
	
	if (!str_cmp (arg1, "list"))
	{
		if (nedit_list (ch, argument))
		{
			ch->desc->editor = ED_NONE;
			return;
		}
		else
			return;
	}
	
	SEND("Syntax: note <write/text/to/subject/board/post/clear/list/read/show/catch>\r\n",ch);
	SEND ("See "MXPTAG("Help 'note'")"help note"MXPTAG("/Help")" for more information.\r\n",ch);	
    return;
}


/* Entry point for editing obj_index_data. */
void do_oedit (CHAR_DATA * ch, char *argument)
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if (IS_NPC (ch))
        return;

    argument = one_argument (argument, arg1);

    if (is_number (arg1))
    {
        value = atoi (arg1);
        if (!(pObj = get_obj_index (value)))
        {
            SEND ("OEdit:  That vnum does not exist.\r\n", ch);
            return;
        }

        if (!IS_BUILDER (ch, pObj->area))
        {
            SEND ("Insufficient security to modify objects.\r\n", ch);
            return;
        }

        ch->desc->pEdit = (void *) pObj;
        ch->desc->editor = ED_OBJECT;
        return;
    }
    else
    {
        if (!str_cmp (arg1, "create"))
        {
            value = atoi (argument);
            if (argument[0] == '\0' || value == 0)
            {
                SEND ("Syntax:  edit object create [vnum]\r\n", ch);
                return;
            }

            pArea = get_vnum_area (value);

            if (!pArea)
            {
                SEND
                    ("OEdit:  That vnum is not assigned an area.\r\n", ch);
                return;
            }

            if (!IS_BUILDER (ch, pArea))
            {
                SEND ("Insufficient security to modify objects.\r\n",
                              ch);
                return;
            }

            if (oedit_create (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                ch->desc->editor = ED_OBJECT;
            }
            return;
        }
    }

    SEND ("OEdit:  There is no default object to edit.\r\n", ch);
    return;
}



/* Entry point for editing mob_index_data. */
void do_medit (CHAR_DATA * ch, char *argument)
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

    if (IS_NPC (ch))
        return;

    if (is_number (arg1))
    {
        value = atoi (arg1);
        if (!(pMob = get_mob_index (value)))
        {
            SEND ("MEdit:  That vnum does not exist.\r\n", ch);
            return;
        }

        if (!IS_BUILDER (ch, pMob->area))
        {
            SEND ("Insufficient security to modify mobs.\r\n", ch);
            return;
        }

        ch->desc->pEdit = (void *) pMob;
        ch->desc->editor = ED_MOBILE;
        return;
    }
    else
    {
        if (!str_cmp (arg1, "create"))
        {
            value = atoi (argument);
            if (arg1[0] == '\0' || value == 0)
            {
                SEND ("Syntax:  edit mobile create [vnum]\r\n", ch);
                return;
            }

            pArea = get_vnum_area (value);

            if (!pArea)
            {
                SEND
                    ("OEdit:  That vnum is not assigned an area.\r\n", ch);
                return;
            }

            if (!IS_BUILDER (ch, pArea))
            {
                SEND ("Insufficient security to modify mobs.\r\n",
                              ch);
                return;
            }

            if (medit_create (ch, argument))
            {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                ch->desc->editor = ED_MOBILE;
            }
            return;
        }
    }

    SEND ("MEdit:  There is no default mobile to edit.\r\n", ch);
    return;
}

void display_mob_aresets (CHAR_DATA * ch)
{
	RESET_DATA *pReset;
	AREA_DATA * pArea;
    MOB_INDEX_DATA *pMob = NULL;
	ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];    
	char final[MAX_STRING_LENGTH];
    int iReset = 0;
	int vnum = 0;        

	pArea = ch->in_room->area;
	
	SEND
			(" No.  Loads      Description          Location         Vnum    Mx Mn Description"
			 "\r\n"
			 "==== ======== =================== =================== ======== ===== ==========="
			 "\r\n", ch);
	
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {			
		if ((pRoomIndex = get_room_index (vnum)))
        {
			for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next)
			{
				
				MOB_INDEX_DATA *pMobIndex;								
				
				sprintf (final, "{r[{x%3d{r]{x ", ++iReset);
				
				if (!(pMobIndex = get_mob_index (pReset->arg1)))
				{
					sprintf (buf, "Load Mobile - Bad Mob %d\r\n",
							 pReset->arg1);
					strcat (final, buf);
					continue;
				}

				if (!(pRoomIndex = get_room_index (pReset->arg3)))				
				{
					sprintf (buf, "Load Mobile - Bad Room %d\r\n",
							 pReset->arg3);
					strcat (final, buf);
					continue;
				}

				pMob = pMobIndex;
				sprintf (buf,
						 "M{r[{x%5d{r]{x %-19.19s in room             R{r[{x%5d{r]{x %2d-%2d %-19.19s\r\n",
						 pReset->arg1, pMob->short_descr, pReset->arg3,
						 pReset->arg2, pReset->arg4, pRoomIndex->name);
				strcat (final, buf);

				/*
				 * Check for pet shop.
				 * -------------------
				 */
				{
					ROOM_INDEX_DATA *pRoomIndexPrev;

					pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
					if (pRoomIndexPrev
						&& IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
						final[5] = 'P';
				}
				SEND (final, ch);
			}
		}
	}	
	return;
}

void display_obj_aresets (CHAR_DATA * ch)
{
	RESET_DATA *pReset;
	AREA_DATA * pArea;
    //MOB_INDEX_DATA *pMob = NULL;
	ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];    
	char final[MAX_STRING_LENGTH];
    int iReset = 0;
	int vnum = 0;        

	pArea = ch->in_room->area;
	
	SEND
			(" No.  Loads    Description                 Vnum     Room Name "
			 "\r\n"
			 "==== ======== =========================== ======== ==========="
			 "\r\n", ch);
	
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {			
		if ((pRoomIndex = get_room_index (vnum)))
        {
			for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next)
			{
				
				OBJ_INDEX_DATA *pObj;				
				OBJ_INDEX_DATA *pObjIndex;				
				ROOM_INDEX_DATA *pRoomIndex;
				
				sprintf (final, "{r[{x%3d{r]{x ", ++iReset);
				
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Load Object - Bad Object %d\r\n",
							 pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!(pRoomIndex = get_room_index (pReset->arg3)))
				{
					sprintf (buf, "Load Object - Bad Room %d\r\n",
							 pReset->arg3);
					strcat (final, buf);
					continue;
				}

				sprintf (buf, "O{r[{x%5d{r]{x %-24s{x "
						 "{r[{x%5d{r]{x       %-19s{x\r\n",
						 pReset->arg1, pObj->short_descr,
						 pReset->arg3, pRoomIndex->name);
				strcat (final, buf);
				
				SEND(final,ch);
			}
		}
	}
	return;
}

void display_aresets (CHAR_DATA * ch)
{	
    RESET_DATA *pReset;
	AREA_DATA * pArea;
    MOB_INDEX_DATA *pMob = NULL;
	ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    //BUFFER *final;
	char final[MAX_STRING_LENGTH];
    int iReset = 0;
	int vnum = 0;    
    //final = new_buf ();

	pArea = ch->in_room->area;
	
	SEND
			(" No.  Loads      Description          Location         Vnum    Mx Mn Description"
			 "\r\n"
			 "==== ======== =================== =================== ======== ===== ==========="
			 "\r\n", ch);
	
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {			
		if ((pRoomIndex = get_room_index (vnum)))
        {
			for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next)
			{
				OBJ_INDEX_DATA *pObj;
				MOB_INDEX_DATA *pMobIndex;
				OBJ_INDEX_DATA *pObjIndex;
				OBJ_INDEX_DATA *pObjToIndex;				
				
				sprintf (final, "{r[{x%3d{r]{x ", ++iReset);

				switch (pReset->command)
				{
					default:
						sprintf (buf, "Bad reset command: %c.", pReset->command);
						strcat (final, buf);
						break;

					case 'M':
						if (!(pMobIndex = get_mob_index (pReset->arg1)))
						{
							sprintf (buf, "Load Mobile - Bad Mob %d\r\n",
									 pReset->arg1);
							strcat (final, buf);
							continue;
						}

						if (!(pRoomIndex = get_room_index (pReset->arg3)))				
						{
							sprintf (buf, "Load Mobile - Bad Room %d\r\n",
									 pReset->arg3);
							strcat (final, buf);
							continue;
						}

						pMob = pMobIndex;
						sprintf (buf,
								 "M{r[{x%5d{r]{x %-19.19s in room             R{r[{x%5d{r]{x %2d-%2d %-19.19s\r\n",
								 pReset->arg1, pMob->short_descr, pReset->arg3,
								 pReset->arg2, pReset->arg4, pRoomIndex->name);
						strcat (final, buf);

						/*
						 * Check for pet shop.
						 * -------------------
						 */
						{
							ROOM_INDEX_DATA *pRoomIndexPrev;

							pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
							if (pRoomIndexPrev
								&& IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
								final[5] = 'P';
						}

						break;

					case 'O':
						if (!(pObjIndex = get_obj_index (pReset->arg1)))
						{
							sprintf (buf, "Load Object - Bad Object %d\r\n",
									 pReset->arg1);
							strcat (final, buf);
							continue;
						}

						pObj = pObjIndex;

						if (!(pRoomIndex = get_room_index (pReset->arg3)))
						{
							sprintf (buf, "Load Object - Bad Room %d\r\n",
									 pReset->arg3);
							strcat (final, buf);
							continue;
						}

						sprintf (buf, "O{r[{x%5d{r]{x %-19.19s{x              "
								 "R{r[{x%5d{r]{x       %-19.19s{x\r\n",
								 pReset->arg1, pObj->short_descr,
								 pReset->arg3, pRoomIndex->name);
						strcat (final, buf);

						break;

					case 'P':
						if (!(pObjIndex = get_obj_index (pReset->arg1)))
						{
							sprintf (buf, "Put Object - Bad Object %d\r\n",
									 pReset->arg1);
							strcat (final, buf);
							continue;
						}

						pObj = pObjIndex;

						if (!(pObjToIndex = get_obj_index (pReset->arg3)))
						{
							sprintf (buf, "Put Object - Bad To Object %d\r\n",
									 pReset->arg3);
							strcat (final, buf);
							continue;
						}

						sprintf (buf,
								 "O{r[{x%5d{r]{x %-19.19s          inside O{r[{x%5d{r]{x %2d-%2d %-19.19s\r\n",
								 pReset->arg1,
								 pObj->short_descr,
								 pReset->arg3,
								 pReset->arg2, pReset->arg4,
								 pObjToIndex->short_descr);
						strcat (final, buf);

						break;

					case 'G':
					case 'E':
						if (!(pObjIndex = get_obj_index (pReset->arg1)))
						{
							sprintf (buf, "Give/Equip Object - Bad Object %d\r\n",
									 pReset->arg1);
							strcat (final, buf);
							continue;
						}

						pObj = pObjIndex;

						if (!pMob)
						{
							sprintf (buf,
									 "Give/Equip Object - No Previous Mobile\r\n");
							strcat (final, buf);
							break;
						}

						if (pMob->pShop)
						{
							sprintf (buf,
									 "O{r[{x%5d{r]{x %-19.19s         sold by S{r[{x%5ld{r]{x       %-19.19s\r\n",
									 pReset->arg1,
									 pObj->short_descr, pMob->vnum,
									 pMob->short_descr);
						}
						else
							sprintf (buf,
									 "O{r[{x%5d{r]{x %-19.19s %-15s{x M{r[{x%5ld{r]{x       %-19.19s\r\n",
									 pReset->arg1,
									 pObj->short_descr,
									 (pReset->command == 'G') ?
									 flag_string (wear_loc_strings, WEAR_NONE)
									 : flag_string (wear_loc_strings, pReset->arg3),
									 pMob->vnum, pMob->short_descr);
						strcat (final, buf);

						break;

						/*
						 * Doors are set in rs_flags don't need to be displayed.
						 * If you want to display them then uncomment the new_reset
						 * line in the case 'D' in load_resets in db.c and here.
						 */
					case 'D':
						pRoomIndex = get_room_index (pReset->arg1);
						sprintf (buf, "R{r[{x%5d{r]{x %s door of %-19.19s reset to %s\r\n",
								 pReset->arg1,
								 capitalize (dir_name[pReset->arg2]),
								 pRoomIndex->name,
								 flag_string (door_resets, pReset->arg3));
						strcat (final, buf);					

						break;
						/*
						 * End Doors Comment.
						 */
					case 'R':
						if (!(pRoomIndex = get_room_index (pReset->arg1)))
						{
							sprintf (buf, "Randomize Exits - Bad Room %d\r\n",
									 pReset->arg1);
							strcat (final, buf);
							continue;
						}

						sprintf (buf, "R{r[{x%5d{r]{x Exits are randomized in %s\r\n",
								 pReset->arg1, pRoomIndex->name);
						strcat (final, buf);

						break;
				}
				//page_to_char (buf_string (final), ch);
				//free_buf (final);
				SEND (final, ch);
			}
		}
	}
    return;
}

void do_aresets (CHAR_DATA *ch, char *argument)
{
	char arg1[MIL];
	
	argument = one_argument (argument, arg1);
	
	if (arg1[0] == '\0')
	{
		display_aresets(ch);
		return;
	}
	else if (!str_cmp(arg1, "mobs"))
	{
		display_mob_aresets(ch);
		return;
	}
	else if (!str_cmp(arg1, "objs"))
	{
		display_obj_aresets(ch);
		return;
	}
	else
	{
		SEND ("Valid arguments are mobs/objs.\r\n",ch);
		return;
	}
}


void display_resets (CHAR_DATA * ch)
{
    ROOM_INDEX_DATA *pRoom;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH];
    int iReset = 0;

    EDIT_ROOM (ch, pRoom);
    final[0] = '\0';

    SEND
        (" No.  Loads    Description         Location            Vnum     Mx Mn Room Name  "
         "\r\n"
         "===== ======== =================== =================== ======== ===== ==========="
         "\r\n", ch);

    for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
    {
        OBJ_INDEX_DATA *pObj;
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;

        final[0] = '\0';
        sprintf (final, "[%3d] ", ++iReset);

        switch (pReset->command)
        {
            default:
                sprintf (buf, "Bad reset command: %c.", pReset->command);
                strcat (final, buf);
                break;

            case 'M':
                if (!(pMobIndex = get_mob_index (pReset->arg1)))
                {
                    sprintf (buf, "Load Mobile - Bad Mob %d\r\n",
                             pReset->arg1);
                    strcat (final, buf);
                    continue;
                }

                if (!(pRoomIndex = get_room_index (pReset->arg3)))
                {
                    sprintf (buf, "Load Mobile - Bad Room %d\r\n",
                             pReset->arg3);
                    strcat (final, buf);
                    continue;
                }

                pMob = pMobIndex;
                sprintf (buf,
                         "M[%5d] %-19.19s{x in room             R[%5d] %2d-%2d %-15.15s\r\n",
                         pReset->arg1, pMob->short_descr, pReset->arg3,
                         pReset->arg2, pReset->arg4, pRoomIndex->name);
                strcat (final, buf);

                /*
                 * Check for pet shop.
                 * -------------------
                 */
                {
                    ROOM_INDEX_DATA *pRoomIndexPrev;

                    pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
                    if (pRoomIndexPrev
                        && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                        final[5] = 'P';
                }

                break;

            case 'O':
                if (!(pObjIndex = get_obj_index (pReset->arg1)))
                {
                    sprintf (buf, "Load Object - Bad Object %d\r\n",
                             pReset->arg1);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;

                if (!(pRoomIndex = get_room_index (pReset->arg3)))
                {
                    sprintf (buf, "Load Object - Bad Room %d\r\n",
                             pReset->arg3);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf, "O[%5d] %-19.19s{x in room             "
                         "R[%5d]       %-15.15s\r\n",
                         pReset->arg1, pObj->short_descr,
                         pReset->arg3, pRoomIndex->name);
                strcat (final, buf);

                break;

            case 'P':
                if (!(pObjIndex = get_obj_index (pReset->arg1)))
                {
                    sprintf (buf, "Put Object - Bad Object %d\r\n",
                             pReset->arg1);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;

                if (!(pObjToIndex = get_obj_index (pReset->arg3)))
                {
                    sprintf (buf, "Put Object - Bad To Object %d\r\n",
                             pReset->arg3);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf,
                         "O[%5d] %-19.19s{x inside              O[%5d] %2d-%2d %-15.15s\r\n",
                         pReset->arg1,
                         pObj->short_descr,
                         pReset->arg3,
                         pReset->arg2, pReset->arg4,
                         pObjToIndex->short_descr);
                strcat (final, buf);

                break;

            case 'G':
            case 'E':
                if (!(pObjIndex = get_obj_index (pReset->arg1)))
                {
                    sprintf (buf, "Give/Equip Object - Bad Object %d\r\n",
                             pReset->arg1);
                    strcat (final, buf);
                    continue;
                }

                pObj = pObjIndex;

                if (!pMob)
                {
                    sprintf (buf,
                             "Give/Equip Object - No Previous Mobile\r\n");
                    strcat (final, buf);
                    break;
                }

                if (pMob->pShop)
                {
                    sprintf (buf,
                             "O[%5d] %-19.19s{x in the inventory of S[%5ld]       %-15.15s\r\n",
                             pReset->arg1,
                             pObj->short_descr, pMob->vnum,
                             pMob->short_descr);
                }
                else
                    sprintf (buf,
                             "O[%5d] %-19.19s{x %-19.19s M[%5ld]       %-15.15s\r\n",
                             pReset->arg1,
                             pObj->short_descr,
                             (pReset->command == 'G') ?
                             flag_string (wear_loc_strings, WEAR_NONE)
                             : flag_string (wear_loc_strings, pReset->arg3),
                             pMob->vnum, pMob->short_descr);
                strcat (final, buf);

                break;

                /*
                 * Doors are set in rs_flags don't need to be displayed.
                 * If you want to display them then uncomment the new_reset
                 * line in the case 'D' in load_resets in db.c and here.
                 */
            case 'D':
                pRoomIndex = get_room_index (pReset->arg1);
                sprintf (buf, "R[%5d] %s door of %-19.19s reset to %s\r\n",
                         pReset->arg1,
                         capitalize (dir_name[pReset->arg2]),
                         pRoomIndex->name,
                         flag_string (door_resets, pReset->arg3));
                strcat (final, buf);

                break;
                /*
                 * End Doors Comment.
                 */
            case 'R':
                if (!(pRoomIndex = get_room_index (pReset->arg1)))
                {
                    sprintf (buf, "Randomize Exits - Bad Room %d\r\n",
                             pReset->arg1);
                    strcat (final, buf);
                    continue;
                }

                sprintf (buf, "R[%5d] Exits are randomized in %s\r\n",
                         pReset->arg1, pRoomIndex->name);
                strcat (final, buf);

                break;
        }
        SEND (final, ch);
    }

    return;
}



/*****************************************************************************
 Name:        add_reset
 Purpose:    Inserts a new reset in the given index slot.
 Called by:    do_resets(olc.c).
 ****************************************************************************/
void add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index)
{
    RESET_DATA *reset;
    int iReset = 0;

    if (!room->reset_first)
    {
        room->reset_first = pReset;
        room->reset_last = pReset;
        pReset->next = NULL;
        return;
    }

    index--;

    if (index == 0)
    {                            /* First slot (1) selected. */
        pReset->next = room->reset_first;
        room->reset_first = pReset;
        return;
    }

    /*
     * If negative slot( <= 0 selected) then this will find the last.
     */
    for (reset = room->reset_first; reset->next; reset = reset->next)
    {
        if (++iReset == index)
            break;
    }

    pReset->next = reset->next;
    reset->next = pReset;
    if (!pReset->next)
        room->reset_last = pReset;
    return;
}

void do_resets (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);
    argument = one_argument (argument, arg4);
    argument = one_argument (argument, arg5);
    argument = one_argument (argument, arg6);
    argument = one_argument (argument, arg7);

    if (!IS_BUILDER (ch, ch->in_room->area))
    {
        SEND ("Resets: Invalid security for editing this area.\r\n",
                      ch);
        return;
    }

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if (arg1[0] == '\0')
    {
        if (ch->in_room->reset_first)
        {
            SEND ("Resets: M = mobile, R = room, O = object, "
                          "P = pet, S = shopkeeper\r\n", ch);
            display_resets (ch);
        }
        else
            SEND ("No resets in this room.\r\n", ch);
    }


    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if (is_number (arg1))
    {
        ROOM_INDEX_DATA *pRoom = ch->in_room;

        /*
         * Delete a reset.
         * ---------------
         */
        if (!str_cmp (arg2, "delete"))
        {
            int insert_loc = atoi (arg1);

            if (!ch->in_room->reset_first)
            {
                SEND ("No resets in this area.\r\n", ch);
                return;
            }

            if (insert_loc - 1 <= 0)
            {
                pReset = pRoom->reset_first;
                pRoom->reset_first = pRoom->reset_first->next;
                if (!pRoom->reset_first)
                    pRoom->reset_last = NULL;
            }
            else
            {
                int iReset = 0;
                RESET_DATA *prev = NULL;

                for (pReset = pRoom->reset_first;
                     pReset; pReset = pReset->next)
                {
                    if (++iReset == insert_loc)
                        break;
                    prev = pReset;
                }

                if (!pReset)
                {
                    SEND ("Reset not found.\r\n", ch);
                    return;
                }

                if (prev)
                    prev->next = prev->next->next;
                else
                    pRoom->reset_first = pRoom->reset_first->next;

                for (pRoom->reset_last = pRoom->reset_first;
                     pRoom->reset_last->next;
                     pRoom->reset_last = pRoom->reset_last->next);
            }

            free_reset_data (pReset);
            SEND ("Reset deleted.\r\n", ch);
        }
        else
            /*
             * Add a reset.
             * ------------
             */
            if ((!str_cmp (arg2, "mob") && is_number (arg3))
                || (!str_cmp (arg2, "obj") && is_number (arg3)))
        {
            /*
             * Check for Mobile reset.
             * -----------------------
             */
            if (!str_cmp (arg2, "mob"))
            {
                if (get_mob_index (is_number (arg3) ? atoi (arg3) : 1) ==
                    NULL)
                {
                    SEND ("Mob doesn't exist.\r\n", ch);
                    return;
                }
                pReset = new_reset_data ();
                pReset->command = 'M';
                pReset->arg1 = atoi (arg3);
                pReset->arg2 = is_number (arg4) ? atoi (arg4) : 1;    /* Max # */
                pReset->arg3 = ch->in_room->vnum;
                pReset->arg4 = is_number (arg5) ? atoi (arg5) : 1;    /* Min # */
            }
            else
                /*
                 * Check for Object reset.
                 * -----------------------
                 */
            if (!str_cmp (arg2, "obj"))
            {
                pReset = new_reset_data ();
                pReset->arg1 = atoi (arg3);
                /*
                 * Inside another object.
                 * ----------------------
                 */
                if (!str_prefix (arg4, "inside"))
                {
                    OBJ_INDEX_DATA *temp;

                    temp = get_obj_index (is_number (arg5) ? atoi (arg5) : 1);
                    if ((temp->item_type != ITEM_CONTAINER) &&
                        (temp->item_type != ITEM_CORPSE_NPC))
                    {
                        SEND ("Object 2 is not a container.\r\n", ch);
                        return;
                    }
                    pReset->command = 'P';
                    pReset->arg2 = is_number (arg6) ? atoi (arg6) : 1;
                    pReset->arg3 = is_number (arg5) ? atoi (arg5) : 1;
                    pReset->arg4 = is_number (arg7) ? atoi (arg7) : 1;
                }
                else
                    /*
                     * Inside the room.
                     * ----------------
                     */
                if (!str_cmp (arg4, "room"))
                {
                    if (get_obj_index (atoi (arg3)) == NULL)
                    {
                        SEND ("Vnum doesn't exist.\r\n", ch);
                        return;
                    }
                    pReset->command = 'O';
                    pReset->arg2 = 0;
                    pReset->arg3 = ch->in_room->vnum;
                    pReset->arg4 = 0;
                }
                else
                    /*
                     * Into a Mobile's inventory.
                     * --------------------------
                     */
                {
                    if (flag_value (wear_loc_flags, arg4) == NO_FLAG)
                    {
                        SEND ("Resets: '? wear-loc'\r\n", ch);
                        return;
                    }
                    if (get_obj_index (atoi (arg3)) == NULL)
                    {
                        SEND ("Vnum doesn't exist.\r\n", ch);
                        return;
                    }
                    pReset->arg1 = atoi (arg3);
                    pReset->arg3 = flag_value (wear_loc_flags, arg4);
                    if (pReset->arg3 == WEAR_NONE)
                        pReset->command = 'G';
                    else
                        pReset->command = 'E';
                }
            }
            add_reset (ch->in_room, pReset, atoi (arg1));
            SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
            SEND ("Reset added.\r\n", ch);
        }
        else if (!str_cmp (arg2, "random") && is_number (arg3))
        {
            if (atoi (arg3) < 1 || atoi (arg3) > 6)
            {
                SEND ("Invalid argument.\r\n", ch);
                return;
            }
            pReset = new_reset_data ();
            pReset->command = 'R';
            pReset->arg1 = ch->in_room->vnum;
            pReset->arg2 = atoi (arg3);
            add_reset (ch->in_room, pReset, atoi (arg1));
            SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
            SEND ("Random exits reset added.\r\n", ch);
        }
        else
        {
            SEND ("Syntax: RESET <number> OBJ <vnum> <wear_loc>\r\n", ch);
            SEND ("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\r\n", ch);
            SEND ("        RESET <number> OBJ <vnum> room\r\n", ch);
            SEND ("        RESET <number> MOB <vnum> [max #x area] [max #x room]\r\n", ch);
            SEND ("        RESET <number> DELETE\r\n", ch);
            SEND ("        RESET <number> RANDOM [#x exits]\r\n", ch);
        }
    }

    return;
}



/*****************************************************************************
 Name:        do_alist
 Purpose:    Normal command to list areas and display area information.
 Called by:    interpreter(interp.c)
 ****************************************************************************/


 void do_alist (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char result[MAX_STRING_LENGTH * 2];   
    AREA_DATA *pArea;
	int linked = 0;

    if (IS_NPC (ch))
        return;

    sprintf (result, "{r[{x%3s{r][{x%1s{r][{x%1s{r][{x%1s{r][{DArea Name                 {r]({x%-5s-%5s{r) [{x%-17s{r][{x%-16s{r]{x\r\n",
             "{DNum", "{DH", "{DL", "{DS", "{DLower{x", "{DUpper", "{DFilename", "{DBuilders");

	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		sprintf (buf,
				 "{r[{x%3d{r][{x%1s{r][{x%1s{r][{x%1d{r]{x{r[{x%-26.26s{r]({x%-5ld-%5ld{r){x %-16s {r[{x%-14.14s{r]{x\r\n",
				 pArea->vnum, (pArea->history && str_cmp(pArea->history, "(null)")) ? "{GX{x" : " ", 
				 (IS_SET(pArea->area_flags, AREA_LINKED)) ? "{GX{x" : " ",
				 pArea->security, 
				 pArea->name, 
				 pArea->min_vnum, pArea->max_vnum,
				 pArea->file_name,				 
				 pArea->builders);
		strcat (result, buf);
		if (IS_SET(pArea->area_flags, AREA_LINKED))
			linked++;				
	}

	
    SEND (result, ch);
	if (linked > 0)
	{
		sprintf(buf, "\r\nThere are currently {g%d{x linked areas.\r\n", linked);
		SEND(buf, ch);
	}
    return;
}


int get_sector_type_for_area( AREA_DATA * area )
{
	int sect = -1;
	
	switch (area->type)
	{
		default:
			return SECT_FIELD;
			break;
		case SECT_CITY:
			if (number_percent() < 60)			
				return SECT_CITY;
			else
				return SECT_ROAD;
			break;
		case SECT_FOREST:
			if (number_percent() < 75)
				return SECT_FOREST;
			else
				return SECT_ROAD;
	}
	
	return sect;
}


bool create_random_area( AREA_DATA * area )
{
	//CHAR_DATA *mob;
	ROOM_INDEX_DATA *room = NULL;
	long vnum;
	int iHash;
		
	
	//Loop through the area's vnums.
	for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++)
	{
		//creates a room 80% of the time. giving you an average 80 room area
		//within a set of 100 vnums.
		if (number_percent() <= 80) 
		{
			room = new_room_index();
			room->area = area;
			room->vnum = vnum;
			room->name = str_dup("Untamed Wilderness of Aragond");
			room->description = str_dup("You are out in the wilderness. It is indeed very scary out here.\r\n");
			room->sector_type = get_sector_type_for_area(area);			
			room->heal_rate = 25;
			room->mana_rate = 25;
			if (vnum > top_vnum_room)
				top_vnum_room = vnum;

			iHash = vnum % MAX_KEY_HASH;
			room->next = room_index_hash[iHash];
			room_index_hash[iHash] = room;			
						
		}	

		// //creates a mob 55% of the time, giving you around 55 mobs per every 100 vnums.		
		// if (number_percent() <= 55 && (room != NULL))
		// {
			// int vnum = number_range(1, top_mob_index);
			// mob = create_mobile ( get_mob_index(vnum) );
			// if (mob)
				// char_to_room(mob, room);
		// }		
	}
	
	if (!area)
		return FALSE;
	
	
	
	return TRUE;
}