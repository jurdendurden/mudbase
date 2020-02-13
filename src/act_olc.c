/***************************************************************************
 *  File: olc_act.c                                                        *
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

/* Locals */
void wipe_resets( ROOM_INDEX_DATA *pRoom );
void instaroom( ROOM_INDEX_DATA *pRoom );
char *prog_type_to_name(int type);
int get_weapon_type args((OBJ_INDEX_DATA *obj));
bool check_existing_coord args((ROOM_INDEX_DATA *pRoom, int x, int y, int z));

//Note stuff
NOTE_DATA *new_note args(());
int board_number 	args((const BOARD_DATA *board));


extern HELP_AREA *had_list;

#define ALT_FLAGVALUE_SET( _blargh, _table, _arg )        \
    {                            \
        int blah = flag_value( _table, _arg );        \
        _blargh = (blah == NO_FLAG) ? 0 : blah;        \
    }

#define ALT_FLAGVALUE_TOGGLE( _blargh, _table, _arg )        \
    {                            \
        int blah = flag_value( _table, _arg );        \
        _blargh ^= (blah == NO_FLAG) ? 0 : blah;    \
    }

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )
#define HEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )
#define NEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define QEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

struct olc_help_type {
    char *command;
    const void *structure;
    char *desc;
};



bool show_version (CHAR_DATA * ch, char *argument)
{
    SEND (VERSION, ch);
    SEND ("\r\n", ch);
    SEND (AUTHOR, ch);
    SEND ("\r\n", ch);
    SEND (DATE, ch);
    SEND ("\r\n", ch);
    SEND (CREDITS, ch);
    SEND ("\r\n", ch);

    return FALSE;
}

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] = {
    {"area", area_flags, "Area attributes."},
    {"room", room_flags, "Room attributes."},
    {"sector", sector_flags, "Sector types, terrain."},
    {"exit", exit_flags, "Exit types."},
    {"type", type_flags, "Types of objects."},
    {"extra", extra_flags, "Object attributes."},
	{"extra2", extra2_flags, "Object attributes."},
    {"wear", wear_flags, "Where to wear object."},
    {"spec", spec_table, "Available special programs."},
    {"sex", sex_flags, "Sexes."},
    {"act", act_flags, "Mobile attributes."},
	{"act2", act2_flags, "Mobile attributes 2."},
    {"affect", affect_flags, "Mobile affects."},
    {"wear-loc", wear_loc_flags, "Where mobile wears object."},
    {"spells", skill_table, "Names of current spells."},
    {"container", container_flags, "Container status."},	

/* ROM specific bits: */
	{"aflag", area_flags, "Area flags"},
    {"armor", ac_type, "Ac for different attacks."},
    {"apply", apply_flags, "Apply flags"},
    {"form", form_flags, "Mobile body form."},
    {"part", part_flags, "Mobile body parts."},
    {"imm", imm_flags, "Mobile immunity."},
    {"res", res_flags, "Mobile resistance."},
    {"vuln", vuln_flags, "Mobile vulnerability."},
    {"off", off_flags, "Mobile offensive behaviour."},
    {"size", size_flags, "Mobile size."},
    {"position", position_flags, "Mobile positions."},
    {"wclass", weapon_class, "Weapon class."},
    {"wtype", weapon_type2, "Special weapon type."},
    {"portal", portal_flags, "Portal types."},
    {"furniture", furniture_flags, "Furniture types."},
    {"liquid", liq_table, "Liquid types."},
    {"apptype", apply_types, "Apply types."},
    {"weapon", attack_table, "Weapon types."},
    {"mprog", mprog_flags, "MobProgram flags."},
	{	"oprog",	oprog_flags,	 "ObjProgram flags."		 },
    {	"rprog",	rprog_flags,	 "RoomProgram flags."		 },
	{	"qtype",	quest_types,	"Quest Type Flags."			 },
    {NULL, NULL, NULL}
};

char * const mob_difficulty [] =
{  "veasy","easy","normal","hard","vhard" };



/*****************************************************************************
 Name:        show_flag_cmds
 Purpose:    Displays settable flags and stats.
 Called by:    show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds (CHAR_DATA * ch, const struct flag_type *flag_table)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int flag;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if (flag_table[flag].settable)
        {
            sprintf (buf, "%-19.18s", flag_table[flag].name);
            strcat (buf1, buf);
            if (++col % 4 == 0)
                strcat (buf1, "\r\n");
        }
    }

    if (col % 4 != 0)
        strcat (buf1, "\r\n");

    SEND (buf1, ch);
    return;
}



/*****************************************************************************
 Name:        show_skill_cmds
 Purpose:    Displays all skill functions.
         Does remove those damn immortal commands from the list.
         Could be improved by:
         (1) Adding a check for a particular class.
         (2) Adding a check for a level range.
 Called by:    show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds (CHAR_DATA * ch, int tar)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH * 2];
    int sn;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (!skill_table[sn].name)
            break;

        if (!str_cmp (skill_table[sn].name, "reserved")
            || skill_table[sn].spell_fun == spell_null)
            continue;

        if (tar == -1 || skill_table[sn].target == tar)
        {
            sprintf (buf, "%-19.18s", skill_table[sn].name);
            strcat (buf1, buf);
            if (++col % 4 == 0)
                strcat (buf1, "\r\n");
        }
    }

    if (col % 4 != 0)
        strcat (buf1, "\r\n");

    SEND (buf1, ch);
    return;
}




/*****************************************************************************
 Name:        show_spec_cmds
 Purpose:    Displays settable special functions.
 Called by:    show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int spec;
    int col;

    buf1[0] = '\0';
    col = 0;
    SEND ("Preceed special functions with 'spec_'\r\r\n\n", ch);
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
        sprintf (buf, "%-19.18s", &spec_table[spec].name[5]);
        strcat (buf1, buf);
        if (++col % 4 == 0)
            strcat (buf1, "\r\n");
    }

    if (col % 4 != 0)
        strcat (buf1, "\r\n");

    SEND (buf1, ch);
    return;
}



/*****************************************************************************
 Name:        show_help
 Purpose:    Displays help for many tables used in OLC.
 Called by:    olc interpreters.
 ****************************************************************************/
bool show_help (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument (argument, arg);
    one_argument (argument, spell);

    /*
     * Display syntax.
     */
    if (arg[0] == '\0')
    {
        SEND ("Syntax:  ? [command]\r\r\n\n", ch);
        SEND ("[command]  [description]\r\n", ch);
        for (cnt = 0; help_table[cnt].command != NULL; cnt++)
        {
            sprintf (buf, "%-10.10s -%s\r\n",
                     capitalize (help_table[cnt].command),
                     help_table[cnt].desc);
            SEND (buf, ch);
        }
        return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if (arg[0] == help_table[cnt].command[0]
            && !str_prefix (arg, help_table[cnt].command))
        {
            if (help_table[cnt].structure == spec_table)
            {
                show_spec_cmds (ch);
                return FALSE;
            }
            else if (help_table[cnt].structure == liq_table)
            {
                show_liqlist (ch);
                return FALSE;
            }
            else if (help_table[cnt].structure == attack_table)
            {
                show_damlist (ch);
                return FALSE;
            }
			/*else if (help_table[cnt].structure == instrument_type)
			{
				SEND ("Options: LUTE HARP DRUMS PICCOLO HORN\r\n",ch);
				return FALSE;
			}*/
            else if (help_table[cnt].structure == skill_table)
            {

                if (spell[0] == '\0')
                {
                    SEND ("Syntax:  ? spells "
                                  "[ignore/attack/defend/self/object/all]\r\n",
                                  ch);
                    return FALSE;
                }

                if (!str_prefix (spell, "all"))
                    show_skill_cmds (ch, -1);
                else if (!str_prefix (spell, "ignore"))
                    show_skill_cmds (ch, TAR_IGNORE);
                else if (!str_prefix (spell, "attack"))
                    show_skill_cmds (ch, TAR_CHAR_OFFENSIVE);
                else if (!str_prefix (spell, "defend"))
                    show_skill_cmds (ch, TAR_CHAR_DEFENSIVE);
                else if (!str_prefix (spell, "self"))
                    show_skill_cmds (ch, TAR_CHAR_SELF);
                else if (!str_prefix (spell, "object"))
                    show_skill_cmds (ch, TAR_OBJ_INV);
                else
                    SEND ("Syntax:  ? spell "
                                  "[ignore/attack/defend/self/object/all]\r\n",
                                  ch);

                return FALSE;
            }
            else
            {
                show_flag_cmds (ch, help_table[cnt].structure);
                return FALSE;
            }
        }
    }

    show_help (ch, "");
    return FALSE;
}

REDIT (redit_rlist)
{
    ROOM_INDEX_DATA *pRoomIndex;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
	char buf2[MSL];
    BUFFER *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool found;
    long vnum;
    int col = 0;	
	int room_count = 0;
	
    one_argument (argument, arg);

    pArea = ch->in_room->area;
    buf1 = new_buf ();
    found = FALSE;

	int rooms_in_area = pArea->max_vnum - pArea->min_vnum;
	
	if (!str_cmp(arg, "free"))
	{
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pRoomIndex = get_room_index (vnum)))			
				continue;
			else
			{
				found = TRUE;
				sprintf(buf, "{r%6ld{x ", vnum);
				add_buf(buf1, buf);		
				if (++col % 4 == 0)
					add_buf (buf1, "\r\n");				
			}
		}
		
		if (col % 4 != 0)
			add_buf (buf1, "\r\n");
	}
	else
	{
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pRoomIndex = get_room_index (vnum)))
			{
				room_count++;
				found = TRUE;
				sprintf (buf, "[%6ld] %-28s",
						 vnum, capitalize (pRoomIndex->name));
				add_buf (buf1, buf);
				if (++col % 3 == 0)
					add_buf (buf1, "\r\n");
			}
		}
		
		if (!found)
		{
			SEND ("Room(s) not found in this area.\r\n", ch);
			return FALSE;
		}

		if (col % 3 != 0)
			add_buf (buf1, "\r\n");

		sprintf(buf2, "\r\nThere are {g%d{x rooms free of {g%d{x total rooms available.\r\n\r\n", rooms_in_area - room_count, rooms_in_area);
		SEND(buf2, ch);
	}	
	
    page_to_char (buf_string (buf1), ch);
    free_buf (buf1);
	
    return FALSE;
}

REDIT (redit_mlist)
{
    MOB_INDEX_DATA *pMobIndex;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, found;
    long vnum;
    int col = 0;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Syntax:  mlist <all/name>\r\n", ch);
        return FALSE;
    }

    buf1 = new_buf ();
    pArea = ch->in_room->area;
	
	if (!str_cmp(arg, "free"))
	{
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pMobIndex = get_mob_index (vnum)))			
				continue;
			else
			{
				found = TRUE;
				sprintf(buf, "{r%6ld{x ", vnum);
				add_buf(buf1, buf);		
				if (++col % 4 == 0)
					add_buf (buf1, "\r\n");				
			}
		}
		
		if (col % 4 != 0)
			add_buf (buf1, "\r\n");
	}
	else
	{
		fAll = !str_cmp (arg, "all");
		found = FALSE;

		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pMobIndex = get_mob_index (vnum)) != NULL)
			{
				if (fAll || is_name (arg, pMobIndex->player_name))
				{
					found = TRUE;
					sprintf (buf, "[%6ld] %-28s{x",
							 pMobIndex->vnum,
							 capitalize (pMobIndex->short_descr));
					add_buf (buf1, buf);
					if (++col % 3 == 0)
						add_buf (buf1, "\r\n");
				}
			}
		}

		if (!found)
		{
			SEND ("Mobile(s) not found in this area.\r\n", ch);
			return FALSE;
		}

		if (col % 3 != 0)
			add_buf (buf1, "{x\r\n");
	}
	
    page_to_char (buf_string (buf1), ch);
    free_buf (buf1);
    return FALSE;
}



QEDIT (qedit_qlist)
{
	QUEST_INDEX_DATA *pQuest;
	AREA_DATA *pArea;
	int vnum;
	BUFFER *buf1;
	int col = 0;
	char buf[MAX_STRING_LENGTH];
	bool found = FALSE;

	
	EDIT_QUEST(ch, pQuest);
	
	//pArea = pQuest->area;
	pArea = ch->in_room->area;
    buf1 = new_buf ();
	
	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {
        if ((pQuest = get_quest_index (vnum)) != NULL)
        {
			found = TRUE;
			sprintf (buf, "{x[%5ld] %-28s ",
					 pQuest->vnum,
					 capitalize (pQuest->name));
			add_buf (buf1, buf);
			if (++col % 3 == 0)
				add_buf (buf1, "\r\n");
		}
	}	
	
	if (!found)
    {
        SEND ("No quests in this area yet.\r\n", ch);
        return FALSE;
    }

    if (col % 3 != 0)
        add_buf (buf1, "{x\r\n");
	
	page_to_char (buf_string (buf1), ch);
    free_buf (buf1);
    return FALSE;
}

REDIT (redit_olist)
{
    OBJ_INDEX_DATA *pObjIndex;
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buf1;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, found;
    long vnum;
    int col = 0;
	int obj_count = 0;
	char buf2[MSL];

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        SEND ("Syntax:  olist <all/name/item_type>\r\n", ch);
        return FALSE;
    }

    pArea = ch->in_room->area;
	
	
	int objs_in_area = pArea->max_vnum - pArea->min_vnum;
    
	buf1 = new_buf ();
	
	if (!str_cmp(arg, "free"))
	{
		 for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pObjIndex = get_obj_index (vnum)))			
				continue;
			else
			{
				sprintf(buf, "{r%6ld{x ", vnum);
				add_buf(buf1, buf);		
				if (++col % 4 == 0)
					add_buf (buf1, "\r\n");				
			}
		}
	}
	else
	{
	
		fAll = !str_cmp (arg, "all");
		found = FALSE;

		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pObjIndex = get_obj_index (vnum)))
			{
				if (fAll || is_name (arg, pObjIndex->name)
					|| flag_value (type_flags, arg) == pObjIndex->item_type)
				{
					found = TRUE;
					obj_count++;
					sprintf (buf, "{x[%5ld] %26s ",
							 pObjIndex->vnum,
							 capitalize (pObjIndex->short_descr));
					add_buf (buf1, buf);
					if (++col % 3 == 0)
						add_buf (buf1, "\r\n");
				}
			}
		}

		if (!found)
		{
			SEND ("Object(s) not found in this area.\r\n", ch);
			return FALSE;
		}
		
		sprintf(buf2, "\r\nThere are {g%d{x objs free of {g%d{x total objs available.\r\n\r\n", objs_in_area - obj_count, objs_in_area);
		SEND(buf2, ch);
	}
	
	add_buf (buf1, "\r\n");
	
    page_to_char (buf_string (buf1), ch);
    free_buf (buf1);
    return FALSE;
}



REDIT (redit_mshow)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  mshow <vnum>\r\n", ch);
        return FALSE;
    }

    if (!is_number (argument))
    {
        SEND ("REdit: Must be a number.\r\n", ch);
        return FALSE;
    }

    if (is_number (argument))
    {
        value = atoi (argument);
        if (!(pMob = get_mob_index (value)))
        {
            SEND ("REdit:  That mobile does not exist.\r\n", ch);
            return FALSE;
        }

        ch->desc->pEdit = (void *) pMob;
    }

    medit_show (ch, argument);
    ch->desc->pEdit = (void *) ch->in_room;
    return FALSE;
}



REDIT (redit_oshow)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  oshow <vnum>\r\n", ch);
        return FALSE;
    }

    if (!is_number (argument))
    {
        SEND ("REdit: Must be a number.\r\n", ch);
        return FALSE;
    }

    if (is_number (argument))
    {
        value = atoi (argument);
        if (!(pObj = get_obj_index (value)))
        {
            SEND ("REdit:  That object does not exist.\r\n", ch);
            return FALSE;
        }

        ch->desc->pEdit = (void *) pObj;
    }

    oedit_show (ch, argument);
    ch->desc->pEdit = (void *) ch->in_room;
    return FALSE;
}

REDIT (redit_copy)
{
	ROOM_INDEX_DATA *pRoomOrig;
	ROOM_INDEX_DATA *pRoomCopy;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	long vnum;
	char buf[MAX_STRING_LENGTH];
 
	if (argument[0] == '\0')
	{
		sprintf (buf, "{rREDIT: {yCOPY: {xYou must specify a vnum to copy.\r\n"
		              "       Syntax: COPY <vnum> <argument>\r\n"
		              "       {yValid Arguments:{x\r\n"
		              "          all, name, sector, flags, heal, mana,\r\n"
		              "          clan, owner, desc, extra. Use all to clone.\r\n");
		SEND (buf, ch);
		return FALSE;
	}
	bool cAll;
 
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
 
	if (!str_cmp (arg2, "all" ) || arg2[0] == '\0')
		cAll = TRUE;
	else
		cAll = FALSE;
 
 
	if (!is_number(arg1))
	{
		sprintf (buf, "{rREDIT: {yCOPY: {xYou must specify a vnum to copy.\r\n"
					  "       Syntax: COPY <vnum>\r\n"
					  "               COPY <vnum> <argument>\r\n"
					  "       %s is not a number.\r\n", arg1);
		SEND (buf, ch);
		return FALSE;
	}
	else
	{
		vnum = atol(arg1);
		if ((pRoomOrig = get_room_index(vnum)) == NULL)
		{
			sprintf (buf, "{rREDIT: {yCOPY: {xYou must specify an EXISTING vnum to copy.\r\n"
					  "       %ld is not an existing room.\r\n", vnum);
			SEND (buf, ch);
			return FALSE;
		}
 
	// Ok, made it through, let's start and copy the whole thing, excluding
	// any exits (link issue ;p), we're not taking resets either
 
    EDIT_ROOM(ch, pRoomCopy);
 
	if (cAll || !str_prefix (arg2, "name"))
	{
      free_string( pRoomCopy->name );
      pRoomCopy->name = str_dup( pRoomOrig->name );
      sprintf (buf, "{rREDIT: {yCOPY:{x name copied:            {y%s{x\r\n", 
    	pRoomCopy->name);
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "sector"))
	{
      pRoomCopy->sector_type = pRoomOrig->sector_type;
      sprintf (buf, "{rREDIT: {yCOPY:{x sector copied:          {y%s{x\r\n", 
	   flag_string( sector_flags, pRoomCopy->sector_type ) );
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "flags"))
	{
      pRoomCopy->room_flags = pRoomOrig->room_flags;
      sprintf (buf, "{rREDIT: {yCOPY:{x room flags copied:      {y%s{x\r\n", 
	    flag_string( room_flags, pRoomCopy->room_flags ) );
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "heal"))
	{
 
      pRoomCopy->heal_rate = pRoomOrig->heal_rate;
      sprintf (buf, "{rREDIT: {yCOPY:{x heal rate copied:       {y%d{x\r\n",
        pRoomCopy->heal_rate);
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "mana"))
	{
      pRoomCopy->mana_rate = pRoomOrig->mana_rate;
      sprintf (buf, "{rREDIT: {yCOPY:{x mana rate copied:       {y%d{x\r\n",
        pRoomCopy->mana_rate);
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "clan"))
	{
      pRoomCopy->clan = pRoomOrig->clan;
      sprintf( buf, "{rREDIT: {yCOPY:{x clan copied:            {y%s{x\r\n" ,
        ((pRoomCopy->clan > 0) ? clan_table[pRoomCopy->clan].name : "None" ));
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "owner"))
	{
      free_string( pRoomCopy->owner );
      pRoomCopy->owner = str_dup( pRoomOrig->owner );
      sprintf (buf, "{rREDIT: {yCOPY:{x owner copied:           {y%s{x\r\n",
        pRoomCopy->owner[0] == '\0' ? "None" : pRoomCopy->owner );
      SEND(buf,ch);
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "extra"))
	{
      if (pRoomOrig->extra_descr)
      {
    	EXTRA_DESCR_DATA *ed;
    	pRoomCopy->extra_descr = pRoomOrig->extra_descr;
    	sprintf (buf, "{rREDIT: {yCOPY:{x extra descs copied:     {y");
    	SEND (buf,ch);
    	for (ed = pRoomCopy->extra_descr; ed; ed = ed->next)
    	{	
    		SEND (ed->keyword, ch);
    		SEND (" ", ch);
    	}
    	SEND ("{x\r\n", ch);
      }
      if (!cAll)
    	return TRUE;
    }
	if (cAll || !str_prefix (arg2, "description"))
	{
      free_string( pRoomCopy->description );
      pRoomCopy->description = str_dup( pRoomOrig->description );
      sprintf (buf, "{rREDIT: {yCOPY:{x description copied: \r\n%s{x",
        pRoomCopy->description);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll)
    {
      sprintf(buf,  "\r\n{rREDIT: {yCOPY: Room %ld duplicated.{x\r\n", vnum );
	  SEND (buf, ch);
      return TRUE;
    }
 
      sprintf(buf, "{rREDIT: {yCOPY:{x Syntax Error. \"{r%s{x\"\r\n"
      		       "       {yValid Arguments:{x\r\n"
		           "       all, name, sector, flags, heal, mana,\r\n"
		           "       clan, owner, desc, extra. Use all to clone.\r\n", arg2);
      SEND (buf, ch);
      return FALSE;
	}
}



/*****************************************************************************
 Name:        check_range( lower vnum, upper vnum )
 Purpose:    Ensures the range spans only one area.
 Called by:    aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range (int lower, int upper)
{
    AREA_DATA *pArea;
    int cnt = 0;

    for (pArea = area_first; pArea; pArea = pArea->next)
    {
        /*
         * lower < area < upper
         */
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper)
            || (lower <= pArea->max_vnum && pArea->max_vnum <= upper))
            ++cnt;

        if (cnt > 1)
            return FALSE;
    }
    return TRUE;
}



AREA_DATA *get_vnum_area (int vnum)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next)
    {
        if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
            return pArea;
    }

    return 0;
}



char * convert_climate(int climate)
{
	char * str_climate;
	switch (climate)
	{
		default: str_climate = "none specified"; break;
		case CL_TROPICAL:
			str_climate = "tropical"; break;
		case CL_DRY:
			str_climate = "dry"; break;
		case CL_COASTAL:
			str_climate = "coastal"; break;
		case CL_ARCTIC:
			str_climate = "arctic"; break;
		case CL_HIGHLAND:
			str_climate = "highlands"; break;
		case CL_TEMPERATE:
			str_climate = "temperate"; break;
	}
	
	return str_climate;
}

AEDIT (aedit_history)
{
    AREA_DATA *pArea;

    EDIT_AREA (ch, pArea);

    if (argument[0] == '\0')
    {
        string_append (ch, &pArea->history);
        return TRUE;
    }

    SEND ("Syntax:  desc\r\n", ch);
    return FALSE;
}

AEDIT (aedit_faction)
{
	AREA_DATA *pArea;
	char buf[MSL];
	int i = 0;
	
	EDIT_AREA (ch, pArea);
	
	if (argument[0] == '\0')
	{
		SEND("Which faction will this area be part of?\r\n",ch);
		return FALSE;
	}
	
	if (argument[0] == '?')
	{
		SEND("Current Available Factions:\r\n",ch);
		for (i = 0; i < MAX_FACTION; i++)
		{
			sprintf(buf, "%s\r\n", faction_table[i].name);
			SEND(buf,ch);
		}
		return FALSE;
	}
	
	for (i = 0; i <= MAX_FACTION; i++)
	{
		if (!str_cmp(argument, faction_table[i].name))
		{
			free_string (pArea->faction);		
			pArea->faction = str_dup (argument);		
			SEND ("Faction set.\r\n", ch);		
			return TRUE;
		}
	}
	
	SEND("Invalid faction.\r\n",ch);
	return FALSE;	
}


AEDIT (aedit_type)
{
	AREA_DATA *pArea;
	//char buf[MSL];
	int value = 0;
	
	EDIT_AREA (ch, pArea);
	
	if (argument[0] == '\0')
    {
        SEND ("Syntax:  type [string]\r\n", ch);
        return FALSE;
    }
	
	// if (argument[0] == '?')
	// {
		// SEND("Available Materials:\r\n",ch);
		// for (i = 0; i < MAX_MATERIAL; i++)
		// {
			// sprintf(buf, "%s\r\n", mat_table[i].material);
			// SEND(buf,ch);
		// }
		// return FALSE;
	// }
	
	int i = 0;
	for (i = 0; i < SECT_SHIP; i++)
	{
		if (!str_cmp(argument, sector_flags[i].name))
		{
			value = i;
			break;
		}
	}
	
	
	if (value && value < SECT_MAX && value != SECT_SHIP)
	{
		pArea->type = value;
		SEND("Area type set.\r\n",ch);
		return TRUE;
	}
	else
	{
		SEND("Invalid area type.\r\n",ch);
		return FALSE;
	}
	return FALSE;
}

AEDIT (aedit_resource)
{
	AREA_DATA *pArea;
	char buf[MSL];
	int i = 0;
	

    EDIT_AREA (ch, pArea);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  resource [string]\r\n", ch);
        return FALSE;
    }
	
	if (argument[0] == '?')
	{
		SEND("Available Materials:\r\n",ch);
		for (i = 0; i < MAX_MATERIAL; i++)
		{
			sprintf(buf, "%s\r\n", mat_table[i].material);
			SEND(buf,ch);
		}
		return FALSE;
	}
	
	if (valid_material(argument))
	{		
		free_string (pArea->main_resource);		
		pArea->main_resource = str_dup (argument);		
		SEND ("Primary Resource set.\r\n", ch);		
		return TRUE;
	}
	else
	{
		SEND("Invalid material type.\r\n",ch);
		return FALSE;
	}
	return FALSE;
}


/*AEDIT (aedit_qlist)
{
	AREA_DATA *pArea;
    //char buf[MAX_STRING_LENGTH];

    EDIT_AREA (ch, pArea);

	return FALSE;
}*/

AEDIT (aedit_allsect)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoomIndex;    
	int vnum;
	int value = 0;

	EDIT_ROOM (ch, pRoomIndex);
	
	pArea = pRoomIndex->area;

	if ((value = flag_value (sector_flags, argument)) == NO_FLAG)
	{
		SEND("Invalid sector type.\r\n",ch);
		return FALSE;
	}
	else
	{
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pRoomIndex = get_room_index (vnum)) != NULL)
			{
				pRoomIndex->sector_type = value;
			}
		}
		SEND("All sectors set for this area.\r\n",ch);
		return TRUE;
	}
	
	return FALSE;
}

AEDIT (aedit_allheal)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoomIndex;
	int value = 0;
	int vnum = 0;
	

	EDIT_ROOM (ch, pRoomIndex);
	
	pArea = pRoomIndex->area;
	
	if (argument[0] == '\0')
	{
		SEND("Need to specify an amount.\r\n",ch);
		return FALSE;
	}
	
	if (is_number(argument))
	{
		value = atoi(argument);
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pRoomIndex = get_room_index (vnum)) != NULL)
			{
				pRoomIndex->heal_rate = value;
			}
		}
		SEND("All heal rates set for this area.\r\n",ch);
		return TRUE;
	}
	else
	{
		SEND("Need to specify an amount.\r\n",ch);
		return FALSE;
	}
	
	return FALSE;
}

AEDIT (aedit_allmana)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoomIndex;
	int value = 0;
	int vnum = 0;
	

    EDIT_ROOM (ch, pRoomIndex);
	
	pArea = pRoomIndex->area;
	
	if (argument[0] == '\0')
	{
		SEND("Need to specify an amount.\r\n",ch);
		return FALSE;
	}
	
	if (is_number(argument))
	{
		value = atoi(argument);
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pRoomIndex = get_room_index (vnum)) != NULL)
			{
				pRoomIndex->mana_rate = value;
			}
		}
		SEND("All mana rates set for this area.\r\n",ch);
		return TRUE;
	}
	else
	{
		SEND("Need to specify an amount.\r\n",ch);
		return FALSE;
	}
	
	return FALSE;
}

/*
 * Area Editor Functions.
 */
AEDIT (aedit_show)
{
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA (ch, pArea);

    sprintf (buf, "Name:     {g[{x%3d{g]{x %s\r\n", pArea->vnum, pArea->name);
    SEND (buf, ch);

#if 0                            /* ROM OLC */
    sprintf (buf, "Recall:   {g[{x%5d{g]{x %s\r\n", pArea->recall,
             get_room_index (pArea->recall)
             ? get_room_index (pArea->recall)->name : "none");
    SEND (buf, ch);
#endif /* ROM */

    sprintf (buf, "File:     %s\r\n", pArea->file_name);
    SEND (buf, ch);

    sprintf (buf, "Vnums:    {g[{x%ld{g-{x%ld{g]{x\r\n", pArea->min_vnum, pArea->max_vnum);
    SEND (buf, ch);
	
	sprintf (buf, "Level:    {g[{x%d{g-{x%d{g]{x\r\n", pArea->low_range, pArea->high_range);
    SEND (buf, ch);
	
    sprintf (buf, "Age:      {g[{x%d{g]{x\r\n", pArea->age);
    SEND (buf, ch);

	sprintf (buf, "Climate:  {g[{x%s{g]{x\r\n", convert_climate(pArea->climate));
	SEND (buf, ch);	
	
	sprintf (buf, "Faction:  {g[{x%s{g]{x\r\n", pArea->faction);
	SEND (buf, ch);
	
    sprintf (buf, "Players:  {g[{x%d{g]{x\r\n", pArea->nplayer);
    SEND (buf, ch);

    sprintf (buf, "Security: {g[{x%d{g]{x\r\n", pArea->security);
    SEND (buf, ch);

    sprintf (buf, "Builders: {g[{x%s{g]{x\r\n", pArea->builders);
    SEND (buf, ch);

    sprintf (buf, "Credits:  {g[{x%s{g]{x\r\n", pArea->credits);
    SEND (buf, ch);
	
//	sprintf (buf, "Type:     {g[{x%s{g]{x\r\n", sector_flags[pArea->type].name);
//	SEND (buf, ch);
	
	sprintf (buf, "Plane:    {g[{x%s{g]{x\r\n", plane_table[pArea->plane].name);
	SEND (buf, ch);
	
	sprintf (buf, "Pri Resource:  {g[{x%s{g]{x\r\n", pArea->main_resource);
	SEND (buf, ch);
	
    sprintf (buf, "Flags:    {g[{x%s{g]{x\r\n",
             flag_string (area_flags, pArea->area_flags));
    SEND (buf, ch);

	if (IS_SET(pArea->area_flags, AREA_SHIP))
	{
		sprintf (buf, "Ship Loc: {g[{x%d{g]{x\r\n", pArea->ship_vnum);
		SEND (buf, ch);
	}
	
	sprintf (buf, "History:  \r\n{g[{x%s{g]{x\r\n", pArea->history);
	SEND (buf, ch);
	
    return FALSE;
}


AEDIT (aedit_shiploc)
{
	AREA_DATA *pArea;
	int value = 0;
	
	EDIT_AREA (ch, pArea);
	
	if (IS_SET(pArea->area_flags, AREA_SHIP))
	{
	
		if (argument[0] == '\0')
		{
			SEND("Which vnum will the ship start off at?\r\n",ch);
			return FALSE;
		}
		
		if (is_number(argument))
		{
			value = atoi(argument);
			pArea->ship_vnum = value;
			return TRUE;
		}
		else
		{
			SEND("Needs to be a number.\r\n",ch);
			return FALSE;
		}
	}
	return FALSE;
}

AEDIT (aedit_reset)
{
    AREA_DATA *pArea;

    EDIT_AREA (ch, pArea);

    reset_area (pArea);
    SEND ("Area reset.\r\n", ch);

    return FALSE;
}


//Define what plane of existance this area is in.
AEDIT (aedit_plane)
{
	AREA_DATA *pArea;
	char buf [MSL];
	int value = 0;
	
	EDIT_AREA (ch, pArea);
	
	if (argument[0] == '\0')
	{
		SEND("Please input a plane.\r\n",ch);
		return FALSE;
	}
		
	if (is_number(argument))
	{
		value = atoi(argument);
		if (value < MAX_PLANE && value > 0)
		{
			pArea->plane = value;
			sprintf(buf, "Area plane changed to: %s\r\n", plane_table[value].name);
			SEND(buf, ch);
			return TRUE;
		}
		else
		{
			SEND("Not a valid plane of existance.\r\n",ch);
			return FALSE;
		}
	}
	else
	{
		SEND("Needs to be a number.\r\n",ch);
		return FALSE;
	}
	
	return FALSE;
	
}


AEDIT (aedit_climate)
{
	AREA_DATA *pArea;
	int value = 0;
	
	EDIT_AREA (ch, pArea);
	
	if (argument[0] == '\0')
	{
		SEND("Syntax:     climate [climate #]\r\n",ch);
		SEND("Valid climates are: 0 (Tropical), 1 (Dry), 2 (Coastal), 3 (Arctic), 4 (Highlands), 5 (Temperate).\r\n",ch);
		return FALSE;
	}
		
	if (is_number(argument))
	{
		value = atoi(argument);
		if (value <= MAX_CLIMATE && value > -1)
		{
			pArea->climate = value;
			return TRUE;
		}
		else
		{
			SEND("Not a valid climate type.\r\n",ch);
			return FALSE;
		}
	}
	else
	{
		SEND("Needs to be a number.\r\n",ch);
		return FALSE;
	}
	
	return FALSE;
}


AEDIT (aedit_create)
{
    AREA_DATA *pArea;

    pArea = new_area ();
    area_last->next = pArea;
    area_last = pArea;            /* Thanks, Walker. */
    ch->desc->pEdit = (void *) pArea;

    SET_BIT (pArea->area_flags, AREA_ADDED);
    SEND ("Area Created.\r\n", ch);
    return FALSE;
}



AEDIT (aedit_name)
{
    AREA_DATA *pArea;

    EDIT_AREA (ch, pArea);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:   name [$name]\r\n", ch);
        return FALSE;
    }

    free_string (pArea->name);
    pArea->name = str_dup (argument);

    SEND ("Name set.\r\n", ch);
    return TRUE;
}

AEDIT (aedit_credits)
{
    AREA_DATA *pArea;

    EDIT_AREA (ch, pArea);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:   credits [$credits]\r\n", ch);
        return FALSE;
    }

    free_string (pArea->credits);
    pArea->credits = str_dup (argument);

    SEND ("Credits set.\r\n", ch);
    return TRUE;
}


AEDIT (aedit_file)
{
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA (ch, pArea);

    one_argument (argument, file);    /* Forces Lowercase */

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  filename [$file]\r\n", ch);
        return FALSE;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen (argument);
    if (length > 12)
    {
        SEND ("No more than twelve characters allowed.\r\n", ch);
        return FALSE;
    }

    /*
     * Allow only letters and numbers.
     */
    for (i = 0; i < length; i++)
    {
        if (!isalnum (file[i]) && file[i] != '_')
        {
            SEND ("Only letters, numbers, and underscores are valid.\r\n", ch);
            return FALSE;
        }
    }

    free_string (pArea->file_name);
    strcat (file, ".are");
    pArea->file_name = str_dup (file);

    SEND ("Filename set.\r\n", ch);
    return TRUE;
}



AEDIT (aedit_age)
{
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA (ch, pArea);

    one_argument (argument, age);

    if (!is_number (age) || age[0] == '\0')
    {
        SEND ("Syntax:  age [#xage]\r\n", ch);
        return FALSE;
    }

    pArea->age = atoi (age);

    SEND ("Age set.\r\n", ch);
    return TRUE;
}


#if 0                            /* ROM OLC */
AEDIT (aedit_recall)
{
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA (ch, pArea);

    one_argument (argument, room);

    if (!is_number (argument) || argument[0] == '\0')
    {
        SEND ("Syntax:  recall [#xrvnum]\r\n", ch);
        return FALSE;
    }

    value = atoi (room);

    if (!get_room_index (value))
    {
        SEND ("AEdit:  Room vnum does not exist.\r\n", ch);
        return FALSE;
    }

    pArea->recall = value;

    SEND ("Recall set.\r\n", ch);
    return TRUE;
}
#endif /* ROM OLC */


AEDIT (aedit_security)
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA (ch, pArea);

    one_argument (argument, sec);

    if (!is_number (sec) || sec[0] == '\0')
    {
        SEND ("Syntax:  security [#xlevel]\r\n", ch);
        return FALSE;
    }

    value = atoi (sec);

    if (value > ch->pcdata->security || value < 0)
    {
        if (ch->pcdata->security != 0)
        {
            sprintf (buf, "Security is 0-%d.\r\n", ch->pcdata->security);
            SEND (buf, ch);
        }
        else
            SEND ("Security is 0 only.\r\n", ch);
        return FALSE;
    }

    pArea->security = value;

    SEND ("Security set.\r\n", ch);
    return TRUE;
}



AEDIT (aedit_builder)
{
    AREA_DATA *pArea;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA (ch, pArea);

    one_argument (argument, name);

    if (name[0] == '\0')
    {
        SEND ("Syntax:  builder [$name]  -toggles builder\r\n", ch);
        SEND ("Syntax:  builder All      -allows everyone\r\n", ch);
        return FALSE;
    }

    name[0] = UPPER (name[0]);

    if (strstr (pArea->builders, name) != '\0')
    {
        pArea->builders = string_replace (pArea->builders, name, "\0");
        pArea->builders = string_unpad (pArea->builders);

        if (pArea->builders[0] == '\0')
        {
            free_string (pArea->builders);
            pArea->builders = str_dup ("None");
        }
        SEND ("Builder removed.\r\n", ch);
        return TRUE;
    }
    else
    {
        buf[0] = '\0';
        if (strstr (pArea->builders, "None") != '\0')
        {
            pArea->builders = string_replace (pArea->builders, "None", "\0");
            pArea->builders = string_unpad (pArea->builders);
        }

        if (pArea->builders[0] != '\0')
        {
            strcat (buf, pArea->builders);
            strcat (buf, " ");
        }
        strcat (buf, name);
        free_string (pArea->builders);
        pArea->builders = string_proper (str_dup (buf));

        SEND ("Builder added.\r\n", ch);
        SEND (pArea->builders, ch);
        return TRUE;
    }

    return FALSE;
}



AEDIT (aedit_vnum)
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    long ilower;
    long iupper;

    EDIT_AREA (ch, pArea);

    argument = one_argument (argument, lower);
    one_argument (argument, upper);

    if (!is_number (lower) || lower[0] == '\0'
        || !is_number (upper) || upper[0] == '\0')
    {
        SEND ("Syntax:  vnum [#xlower] [#xupper]\r\n", ch);
        return FALSE;
    }

    if ((ilower = atol (lower)) > (iupper = atol (upper)))
    {
        SEND ("AEdit:  Upper must be larger then lower.\r\n", ch);
        return FALSE;
    }

    if (!check_range (atol (lower), atol (upper)))
    {
        SEND ("AEdit:  Range must include only this area.\r\n", ch);
        return FALSE;
    }

    if (get_vnum_area (ilower) && get_vnum_area (ilower) != pArea)
    {
        SEND ("AEdit:  Lower vnum already assigned.\r\n", ch);
        return FALSE;
    }

    pArea->min_vnum = ilower;
    SEND ("Lower vnum set.\r\n", ch);

    if (get_vnum_area (iupper) && get_vnum_area (iupper) != pArea)
    {
        SEND ("AEdit:  Upper vnum already assigned.\r\n", ch);
        return TRUE;            /* The lower value has been set. */
    }

    pArea->max_vnum = iupper;
    SEND ("Upper vnum set.\r\n", ch);

    return TRUE;
}

AEDIT (aedit_flags)
{
	AREA_DATA *pArea;
	int value;
	
	EDIT_AREA (ch, pArea);
		
    if ((value = flag_value (area_flags, argument)) == NO_FLAG)
    {
        SEND ("Syntax: flag [flags]\r\n", ch);
        return FALSE;
    }	
		
	TOGGLE_BIT (pArea->area_flags, value);	
    SEND ("Area flags toggled.\r\n", ch);
	if (value == AREA_RANDOM)
	{
		if (pArea->low_range > 0 && pArea->high_range > 0 && pArea->climate > 0 && pArea->main_resource > 0 && pArea->age > 0 && pArea->min_vnum > 0 && pArea->max_vnum > 0)
		{
			if (create_random_area(pArea))		
				SEND ("Area successfully randomly generated.\r\n",ch);							
			else
				SEND ("Area generation unsuccessful.\r\n",ch);
		}
		else
		{
			SEND ("Finish setting the details of the area before generating it!\r\n",ch);
			TOGGLE_BIT (pArea->area_flags, value);
			return FALSE;
		}
	}
    return TRUE;
}


AEDIT (aedit_level)
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, pArea);

    argument = one_argument (argument, lower);
    one_argument (argument, upper);

    if (!is_number (lower) || lower[0] == '\0'
        || !is_number (upper) || upper[0] == '\0')
    {
        SEND ("Syntax:  level [#xlower] [#xupper]\r\n", ch);
        return FALSE;
    }

    if ((ilower = atoi (lower)) > (iupper = atoi (upper)))
    {
        SEND ("AEdit:  Upper must be larger then lower.\r\n", ch);
        return FALSE;
    }    

	if (ilower < 1)
	{
		SEND("Must be at least level 1.\r\n",ch);
		return FALSE;
	}
	
	if (iupper > 50)
	{
		SEND("Cannot be higher than 50.\r\n",ch);
		return FALSE;
	}
	
    pArea->low_range = ilower;
    SEND ("Lower level set.\r\n", ch);    

    pArea->high_range = iupper;
    SEND ("Upper level set.\r\n", ch);

    return TRUE;
}


AEDIT (aedit_lvnum)
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, pArea);

    one_argument (argument, lower);

    if (!is_number (lower) || lower[0] == '\0')
    {
        SEND ("Syntax:  min_vnum [#xlower]\r\n", ch);
        return FALSE;
    }

    if ((ilower = atoi (lower)) > (iupper = pArea->max_vnum))
    {
        SEND ("AEdit:  Value must be less than the max_vnum.\r\n",
                      ch);
        return FALSE;
    }

    if (!check_range (ilower, iupper))
    {
        SEND ("AEdit:  Range must include only this area.\r\n", ch);
        return FALSE;
    }

    if (get_vnum_area (ilower) && get_vnum_area (ilower) != pArea)
    {
        SEND ("AEdit:  Lower vnum already assigned.\r\n", ch);
        return FALSE;
    }

    pArea->min_vnum = ilower;
    SEND ("Lower vnum set.\r\n", ch);
    return TRUE;
}



AEDIT (aedit_uvnum)
{
    AREA_DATA *pArea;
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, pArea);

    one_argument (argument, upper);

    if (!is_number (upper) || upper[0] == '\0')
    {
        SEND ("Syntax:  max_vnum [#xupper]\r\n", ch);
        return FALSE;
    }

    if ((ilower = pArea->min_vnum) > (iupper = atoi (upper)))
    {
        SEND ("AEdit:  Upper must be larger then lower.\r\n", ch);
        return FALSE;
    }

    if (!check_range (ilower, iupper))
    {
        SEND ("AEdit:  Range must include only this area.\r\n", ch);
        return FALSE;
    }

    if (get_vnum_area (iupper) && get_vnum_area (iupper) != pArea)
    {
        SEND ("AEdit:  Upper vnum already assigned.\r\n", ch);
        return FALSE;
    }

    pArea->max_vnum = iupper;
    SEND ("Upper vnum set.\r\n", ch);

    return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT (redit_show)
{
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    char buf1[2 * MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *rch;
	PROG_LIST		*list;
    int door;
    bool fcnt;

    EDIT_ROOM (ch, pRoom);

    buf1[0] = '\0';

    sprintf (buf, "Description:\r\n%s", pRoom->description);
    strcat (buf1, buf);

    sprintf (buf, "Name:       {g[{x%s{g]{x\r\nArea:       {g[{x%5d{g]{x %s\r\n",
             pRoom->name, pRoom->area->vnum, pRoom->area->name);
    strcat (buf1, buf);

    sprintf (buf, "Vnum:       {g[{x%5ld{g]{x\r\nSector:     {g[{x%s{g]{x\r\n",
             pRoom->vnum, flag_string (sector_flags, pRoom->sector_type));
    strcat (buf1, buf);

	
	sprintf (buf, "Coords:     {g[{xX %3d{g]{x  {g[{xY %3d{g]{x  {g[{xZ %3d{g]{x\r\n",
		pRoom->x, pRoom->y, pRoom->z);
	strcat (buf1, buf);
	
	
    sprintf (buf, "Room flags: {g[{x%s{g]{x\r\n",
             flag_string (room_flags, pRoom->room_flags));
    strcat (buf1, buf);

	sprintf (buf, "Health rec: {g[{x%d{g]{x\r\nMana rec  : {g[{x%d{g]{x\r\n",
			 pRoom->heal_rate, pRoom->mana_rate);
	strcat (buf1, buf);

    if (pRoom->clan > 0)
    {
        sprintf (buf, "Clan      : {g[{x%d{g]{x %s\r\n",
                 pRoom->clan, clan_table[pRoom->clan].name);
        strcat (buf1, buf);
    }

    if (!IS_NULLSTR (pRoom->owner))
    {
        sprintf (buf, "Owner     : {g[{x%s{g]{x\r\n", pRoom->owner);
        strcat (buf1, buf);
    }

    if (pRoom->extra_descr)
    {
        EXTRA_DESCR_DATA *ed;

        strcat (buf1, "Desc Kwds:  {g[{x");
        for (ed = pRoom->extra_descr; ed; ed = ed->next)
        {
            strcat (buf1, ed->keyword);
            if (ed->next)
                strcat (buf1, " ");
        }
        strcat (buf1, "{g]{x\r\n");
    }

    strcat (buf1, "Characters: [");
    fcnt = FALSE;
    for (rch = pRoom->people; rch; rch = rch->next_in_room)
    {
        one_argument (rch->name, buf);
        strcat (buf1, buf);
        strcat (buf1, " ");
        fcnt = TRUE;
    }

    if (fcnt)
    {
        int end;

        end = strlen (buf1) - 1;
        buf1[end] = ']';
        strcat (buf1, "\r\n");
    }
    else
        strcat (buf1, "none]\r\n");

    strcat (buf1, "Objects:    [");
    fcnt = FALSE;
    for (obj = pRoom->contents; obj; obj = obj->next_content)
    {
        one_argument (obj->name, buf);
        strcat (buf1, buf);
        strcat (buf1, " ");
        fcnt = TRUE;
    }

    if (fcnt)
    {
        int end;

        end = strlen (buf1) - 1;
        buf1[end] = ']';
        strcat (buf1, "\r\n");
    }
    else
        strcat (buf1, "none]\r\n");

    for (door = 0; door < MAX_DIR; door++)
    {
        EXIT_DATA *pexit;

        if ((pexit = pRoom->exit[door]))
        {
            char word[MAX_INPUT_LENGTH];
            char reset_state[MAX_STRING_LENGTH];
            char *state;
            int i, length;

            sprintf (buf, "-%-5s to {g[{x%5ld{g]{x Key: {g[{x%5d{g]{x ",
                     capitalize (dir_name[door]),
                     pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,    /* ROM OLC */
                     pexit->key);
            strcat (buf1, buf);

            /*
             * Format up the exit info.
             * Capitalize all flags that are not part of the reset info.
             */
            strcpy (reset_state, flag_string (exit_flags, pexit->rs_flags));
            state = flag_string (exit_flags, pexit->exit_info);
            strcat (buf1, " Exit flags: [");
            for (;;)
            {
                state = one_argument (state, word);

                if (word[0] == '\0')
                {
                    int end;

                    end = strlen (buf1) - 1;
                    buf1[end] = ']';
                    strcat (buf1, "\r\n");
                    break;
                }

                if (str_infix (word, reset_state))
                {
                    length = strlen (word);
                    for (i = 0; i < length; i++)
                        word[i] = UPPER (word[i]);
                }
                strcat (buf1, word);
                strcat (buf1, " ");
            }

            if (pexit->keyword && pexit->keyword[0] != '\0')
            {
                sprintf (buf, "Kwds: {g[{x%s{g]{x\r\n", pexit->keyword);
                strcat (buf1, buf);
            }
            if (pexit->description && pexit->description[0] != '\0')
            {
                sprintf (buf, "%s", pexit->description);
                strcat (buf1, buf);
            }
        }
    }

    SEND (buf1, ch);
	if ( pRoom->rprogs )
    {
		int cnt;
	 
		sprintf(buf, "\r\nROOMPrograms for {g[{x%6ld{g]{x:\r\n", pRoom->vnum);
		SEND( buf, ch );
	 
		for (cnt=0, list=pRoom->rprogs; list; list=list->next)
		{
			if (cnt ==0)
			{
				SEND ( " Number Vnum  Trigger Phrase\r\n", ch );
				SEND ( " ------ ----- ------- ------\r\n", ch );
			}
	 
			sprintf(buf, "{g[{x%5d{g]{X %5ld %7s %s\r\n", cnt,
				list->vnum,prog_type_to_name(list->trig_type),
				list->trig_phrase);
			SEND( buf, ch );
			cnt++;
		}
    }
    return FALSE;
}




/* Local function. */
bool change_exit (CHAR_DATA * ch, char *argument, int door)
{
    ROOM_INDEX_DATA *pRoom;
    ROOM_INDEX_DATA *pRoomIndex;
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char digroom[MAX_STRING_LENGTH];
    int value, foundroom;
    int newvnum;
 
    newvnum = 0;
    foundroom = 0;
    pArea = ch->in_room->area;
 
    EDIT_ROOM (ch, pRoom);
 
    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */
    if ((value = flag_value (exit_flags, argument)) != NO_FLAG)
    {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev;                /* ROM OLC */
 
        if (!pRoom->exit[door])
        {
            SEND ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }
 
        /*
         * This room.
         */
        TOGGLE_BIT (pRoom->exit[door]->rs_flags, value);
        /* Don't toggle exit_info because it can be changed by players. */
        pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;
 
        /*
         * Connected room.
         */
        pToRoom = pRoom->exit[door]->u1.to_room;    /* ROM OLC */
        rev = rev_dir[door];
 
        if (pToRoom->exit[rev] != NULL)
        {
            pToRoom->exit[rev]->rs_flags = pRoom->exit[door]->rs_flags;
            pToRoom->exit[rev]->exit_info = pRoom->exit[door]->exit_info;
        }
 
        SEND ("Exit flag toggled.\n\r", ch);
        return TRUE;
    }
 
    /*
     * Now parse the arguments.
     */
    argument = one_argument (argument, command);
    one_argument (argument, arg);
 
    if (command[0] == '\0' && argument[0] == '\0')
    {                            /* Move command. */
 
    if (IS_SET(ch->act, PLR_AUTODIG))
    {
        char buf[MAX_STRING_LENGTH];
 
                newvnum = pArea->min_vnum;
 
                while (foundroom != 1)
                {
                        newvnum++;
                        if ((pRoomIndex = get_room_index (newvnum)))
                                foundroom = 0;
                        else
                          foundroom = 1;
                }
 
                if (newvnum > pArea->max_vnum)
                {
                        SEND ("Dig Error: No more free vnums in area.\n\r", ch);
                        return FALSE;
                }
 
                sprintf (digroom, "%d", newvnum);
                redit_create (ch, digroom);
                sprintf (buf, "link %s", digroom);
                change_exit (ch, buf, door);
                move_char (ch, door, TRUE);    /* ROM OLC */
                return TRUE;
    }
    else
    {
        move_char (ch, door, TRUE);    /* ROM OLC */
        return FALSE;
    }
    }
 
    if (command[0] == '?')
    {
        do_help (ch, "EXIT");
        return FALSE;
    }
 
    if (!str_cmp (command, "delete"))
    {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev;                /* ROM OLC */
 
        if (!pRoom->exit[door])
        {
            SEND ("REdit:  Cannot delete a null exit.\n\r", ch);
            return FALSE;
        }
 
        /*
         * Remove ToRoom Exit.
         */
        rev = rev_dir[door];
        pToRoom = pRoom->exit[door]->u1.to_room;    /* ROM OLC */
 
        if (pToRoom->exit[rev])
        {
            free_exit (pToRoom->exit[rev]);
            pToRoom->exit[rev] = NULL;
        }
 
        /*
         * Remove this exit.
         */
        free_exit (pRoom->exit[door]);
        pRoom->exit[door] = NULL;
 
        SEND ("Exit unlinked.\n\r", ch);
        return TRUE;
    }
 
 
    if (!str_cmp (command, "link"))
    {
        EXIT_DATA *pExit;
        ROOM_INDEX_DATA *toRoom;
 
        if (arg[0] == '\0' || !is_number (arg))
        {
            SEND ("Syntax:  [direction] link [vnum]\n\r", ch);
            return FALSE;
        }
 
        value = atoi (arg);
 
        if (!(toRoom = get_room_index (value)))
        {
            SEND ("REdit:  Cannot link to non-existant room.\n\r",
                          ch);
            return FALSE;
        }
 
        if (!IS_BUILDER (ch, toRoom->area))
        {
            SEND ("REdit:  Cannot link to that area.\n\r", ch);
            return FALSE;
        }
 
        if (toRoom->exit[rev_dir[door]])
        {
            SEND ("REdit:  Remote side's exit already exists.\n\r",
                          ch);
            return FALSE;
        }
 
        if (!pRoom->exit[door])
            pRoom->exit[door] = new_exit ();
 
        pRoom->exit[door]->u1.to_room = toRoom;
        pRoom->exit[door]->orig_door = door;
 
        door = rev_dir[door];
        pExit = new_exit ();
        pExit->u1.to_room = pRoom;
        pExit->orig_door = door;
        toRoom->exit[door] = pExit;
 
        SEND ("Two-way link established.\n\r", ch);
        return TRUE;
    }
 
    if (!str_cmp (command, "dig"))
    {
        char buf[MAX_STRING_LENGTH];
 
        if (arg[0] == '\0')
        {
                newvnum = pArea->min_vnum;
 
                while (foundroom != 1)
                {
                        newvnum++;
                        if ((pRoomIndex = get_room_index (newvnum)))
                                foundroom = 0;
                        else
                          foundroom = 1;
                }
 
                if (newvnum > pArea->max_vnum)
                {
                        SEND ("Dig Error: No more free vnums in area.\n\r", ch);
                        return FALSE;
                }
 
                sprintf (digroom, "%d", newvnum);
                redit_create (ch, digroom);
                sprintf (buf, "link %s", digroom);
                change_exit (ch, buf, door);
                return TRUE;
 
        }
        else
        {
                if (!is_number (arg))
                {
                        SEND ("Error: Argument must be numerical\n\r", ch);
                        return FALSE;
                }
                else
                {
                        redit_create (ch, arg);
                        sprintf (buf, "link %s", arg);
                        change_exit (ch, buf, door);
                        return TRUE;
                }
        }
    }
 
    if (!str_cmp (command, "room"))
    {
        ROOM_INDEX_DATA *toRoom;
 
        if (arg[0] == '\0' || !is_number (arg))
        {
            SEND ("Syntax:  [direction] room [vnum]\n\r", ch);
            return FALSE;
        }
 
        value = atoi (arg);
 
        if (!(toRoom = get_room_index (value)))
        {
            SEND ("REdit:  Cannot link to non-existant room.\n\r",
                          ch);
            return FALSE;
        }
 
        if (!pRoom->exit[door])
            pRoom->exit[door] = new_exit ();
 
        pRoom->exit[door]->u1.to_room = toRoom;    /* ROM OLC */
        pRoom->exit[door]->orig_door = door;
 
        SEND ("One-way link established.\n\r", ch);
        return TRUE;
    }
 
    if (!str_cmp (command, "key"))
    {
        OBJ_INDEX_DATA *key;
 
        if (arg[0] == '\0' || !is_number (arg))
        {
            SEND ("Syntax:  [direction] key [vnum]\n\r", ch);
            return FALSE;
        }
 
        if (!pRoom->exit[door])
        {
            SEND ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }
 
        value = atoi (arg);
 
        if (!(key = get_obj_index (value)))
        {
            SEND ("REdit:  Key doesn't exist.\n\r", ch);
            return FALSE;
        }
 
        if (key->item_type != ITEM_KEY)
        {
            SEND ("REdit:  Object is not a key.\n\r", ch);
            return FALSE;
        }
 
        pRoom->exit[door]->key = value;
 
        SEND ("Exit key set.\n\r", ch);
        return TRUE;
    }
 
    if (!str_cmp (command, "name"))
    {
        if (arg[0] == '\0')
        {
            SEND ("Syntax:  [direction] name [string]\n\r", ch);
            SEND ("         [direction] name none\n\r", ch);
            return FALSE;
        }
 
        if (!pRoom->exit[door])
        {
            SEND ("Exit doesn't exist.\n\r", ch);
            return FALSE;
        }
 
        free_string (pRoom->exit[door]->keyword);
 
        if (str_cmp (arg, "none"))
            pRoom->exit[door]->keyword = str_dup (arg);
        else
            pRoom->exit[door]->keyword = str_dup ("");
 
        SEND ("Exit name set.\n\r", ch);
        return TRUE;
    }
 
    if (!str_prefix (command, "description"))
    {
        if (arg[0] == '\0')
        {
            if (!pRoom->exit[door])
            {
                SEND ("Exit doesn't exist.\n\r", ch);
                return FALSE;
            }
 
            string_append (ch, &pRoom->exit[door]->description);
            return TRUE;
        }
 
        SEND ("Syntax:  [direction] desc\n\r", ch);
        return FALSE;
    }
 
    return FALSE;
}



REDIT (redit_north)
{
    if (change_exit (ch, argument, DIR_NORTH))
        return TRUE;

    return FALSE;
}



REDIT (redit_south)
{
    if (change_exit (ch, argument, DIR_SOUTH))
        return TRUE;

    return FALSE;
}



REDIT (redit_east)
{
    if (change_exit (ch, argument, DIR_EAST))
        return TRUE;

    return FALSE;
}



REDIT (redit_west)
{
    if (change_exit (ch, argument, DIR_WEST))
        return TRUE;

    return FALSE;
}



REDIT (redit_up)
{
    if (change_exit (ch, argument, DIR_UP))
        return TRUE;

    return FALSE;
}



REDIT (redit_down)
{
    if (change_exit (ch, argument, DIR_DOWN))
        return TRUE;

    return FALSE;
}


REDIT (redit_ed)
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM (ch, pRoom);

    argument = one_argument (argument, command);
    one_argument (argument, keyword);

    if (command[0] == '\0' || keyword[0] == '\0')
    {
        SEND ("Syntax:  ed add [keyword]\r\n", ch);
        SEND ("         ed edit [keyword]\r\n", ch);
        SEND ("         ed delete [keyword]\r\n", ch);
        SEND ("         ed format [keyword]\r\n", ch);
        return FALSE;
    }

    if (!str_cmp (command, "add"))
    {
        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed add [keyword]\r\n", ch);
            return FALSE;
        }

        ed = new_extra_descr ();
        ed->keyword = str_dup (keyword);
        ed->description = str_dup ("");
        ed->next = pRoom->extra_descr;
        pRoom->extra_descr = ed;

        string_append (ch, &ed->description);

        return TRUE;
    }


    if (!str_cmp (command, "edit"))
    {
        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed edit [keyword]\r\n", ch);
            return FALSE;
        }

        for (ed = pRoom->extra_descr; ed; ed = ed->next)
        {
            if (is_name (keyword, ed->keyword))
                break;
        }

        if (!ed)
        {
            SEND ("REdit:  Extra description keyword not found.\r\n",
                          ch);
            return FALSE;
        }

        string_append (ch, &ed->description);

        return TRUE;
    }


    if (!str_cmp (command, "delete"))
    {
        EXTRA_DESCR_DATA *ped = NULL;

        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed delete [keyword]\r\n", ch);
            return FALSE;
        }

        for (ed = pRoom->extra_descr; ed; ed = ed->next)
        {
            if (is_name (keyword, ed->keyword))
                break;
            ped = ed;
        }

        if (!ed)
        {
            SEND ("REdit:  Extra description keyword not found.\r\n",
                          ch);
            return FALSE;
        }

        if (!ped)
            pRoom->extra_descr = ed->next;
        else
            ped->next = ed->next;

        free_extra_descr (ed);

        SEND ("Extra description deleted.\r\n", ch);
        return TRUE;
    }


    if (!str_cmp (command, "format"))
    {
        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed format [keyword]\r\n", ch);
            return FALSE;
        }

        for (ed = pRoom->extra_descr; ed; ed = ed->next)
        {
            if (is_name (keyword, ed->keyword))
                break;
        }

        if (!ed)
        {
            SEND ("REdit:  Extra description keyword not found.\r\n",
                          ch);
            return FALSE;
        }

        ed->description = format_string (ed->description);

        SEND ("Extra description formatted.\r\n", ch);
        return TRUE;
    }

    redit_ed (ch, "");
    return FALSE;
}



REDIT (redit_create)
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;

    EDIT_ROOM (ch, pRoom);

    value = atoi (argument);

    if (argument[0] == '\0' || value <= 0)
    {
        SEND ("Syntax:  create [vnum > 0]\r\n", ch);
        return FALSE;
    }

    pArea = get_vnum_area (value);
    if (!pArea)
    {
        SEND ("REdit:  That vnum is not assigned an area.\r\n", ch);
        return FALSE;
    }

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("REdit:  Vnum in an area you cannot build in.\r\n", ch);
        return FALSE;
    }

    if (get_room_index (value))
    {
        SEND ("REdit:  Room vnum already exists.\r\n", ch);
        return FALSE;
    }

    pRoom = new_room_index ();
    pRoom->area = pArea;
    pRoom->vnum = value;
	pRoom->heal_rate = 25;
	pRoom->mana_rate = 25;
    if (value > top_vnum_room)
        top_vnum_room = value;

    iHash = value % MAX_KEY_HASH;
    pRoom->next = room_index_hash[iHash];
    room_index_hash[iHash] = pRoom;
    ch->desc->pEdit = (void *) pRoom;

    SEND ("Room created.\r\n", ch);
    return TRUE;
}



REDIT (redit_name)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  name [name]\r\n", ch);
        return FALSE;
    }

    free_string (pRoom->name);
    pRoom->name = str_dup (argument);

    SEND ("Name set.\r\n", ch);
    return TRUE;
}

bool check_existing_coord(ROOM_INDEX_DATA *pRoom, int x, int y, int z)
{
	ROOM_INDEX_DATA *pRoomIndex;
	long vnum = 0;
	AREA_DATA * pArea;
		
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pRoomIndex = get_room_index (vnum)) != NULL)
			{
				if (pRoomIndex->x == x && pRoomIndex->y == y && pRoomIndex->z == z && pRoomIndex->vnum != pRoom->vnum)
					return TRUE;
			}
		}
	}		
	return FALSE;
}


REDIT (redit_x)
{
	char buf[MSL];
	ROOM_INDEX_DATA *pRoom;
	
	EDIT_ROOM (ch, pRoom);
	
	if (argument[0] == '\0' || !is_number(argument))
	{
		SEND ("Syntax:  x <number>\r\n", ch);
		return FALSE;
	}
	else
	{
		if (check_existing_coord(pRoom, atoi(argument), pRoom->y, pRoom->z))
		{
			SEND ("Coordinates already in use.\r\n",ch);
			return FALSE;
		}
		pRoom->x = atoi(argument);
		sprintf(buf, "Room X coordinate set to %s.\r\n", argument);
		SEND(buf, ch);
		return TRUE;
	}
	return FALSE;
}

REDIT (redit_y)
{
	char buf[MSL];
	ROOM_INDEX_DATA *pRoom;
	
	EDIT_ROOM (ch, pRoom);
	
	if (argument[0] == '\0' || !is_number(argument))
	{
		SEND ("Syntax:  y <number>\r\n", ch);
		return FALSE;
	}
	else
	{
		if (check_existing_coord(pRoom, pRoom->x, atoi(argument), pRoom->z))
		{
			SEND ("Coordinates already in use.\r\n",ch);
			return FALSE;
		}
		pRoom->y = atoi(argument);
		sprintf(buf, "Room Y coordinate set to %s.\r\n", argument);
		SEND(buf, ch);
		return TRUE;
	}
	return FALSE;
}

REDIT (redit_z)
{
	char buf[MSL];
	ROOM_INDEX_DATA *pRoom;
	
	EDIT_ROOM (ch, pRoom);
	
	if (argument[0] == '\0' || !is_number(argument))
	{
		SEND ("Syntax:  z <number>\r\n", ch);
		return FALSE;
	}
	else
	{
		if (check_existing_coord(pRoom, pRoom->x, pRoom->y, atoi(argument)))
		{
			SEND ("Coordinates already in use.\r\n",ch);
			return FALSE;
		}
		pRoom->z = atoi(argument);
		sprintf(buf, "Room Z coordinate set to %s.\r\n", argument);
		SEND(buf, ch);
		return TRUE;
	}
	return FALSE;
}

REDIT (redit_desc)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    if (argument[0] == '\0')
    {
        string_append (ch, &pRoom->description);
        return TRUE;
    }

    SEND ("Syntax:  desc\r\n", ch);
    return FALSE;
}


REDIT (redit_heal)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    if (is_number (argument))
    {
        pRoom->heal_rate = atoi (argument);
        SEND ("Heal rate set.\r\n", ch);
        return TRUE;
    }

    SEND ("Syntax: heal <#xnumber>\r\n", ch);
    return FALSE;
}

REDIT (redit_mana)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    if (is_number (argument))
    {
        pRoom->mana_rate = atoi (argument);
        SEND ("Mana rate set.\r\n", ch);
        return TRUE;
    }

    SEND ("Syntax: mana <#xnumber>\r\n", ch);
    return FALSE;
}

REDIT (redit_clan)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    pRoom->clan = clan_lookup (argument);

    SEND ("Clan set.\r\n", ch);
    return TRUE;
}

REDIT (redit_format)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    pRoom->description = format_string (pRoom->description);

    SEND ("String formatted.\r\n", ch);
    return TRUE;
}



REDIT (redit_mreset)
{
    ROOM_INDEX_DATA *pRoom;
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *newmob;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    RESET_DATA *pReset;
    char output[MAX_STRING_LENGTH];

    EDIT_ROOM (ch, pRoom);

    argument = one_argument (argument, arg);
    argument = one_argument (argument, arg2);

    if (arg[0] == '\0' || !is_number (arg))
    {
        SEND ("Syntax:  mreset <vnum> <max #x> <mix #x>\r\n", ch);
        return FALSE;
    }

    if (!(pMobIndex = get_mob_index (atoi (arg))))
    {
        SEND ("REdit: No mobile has that vnum.\r\n", ch);
        return FALSE;
    }

    if (pMobIndex->area != pRoom->area)
    {
        SEND ("REdit: No such mobile in this area.\r\n", ch);
        return FALSE;
    }

    /*
     * Create the mobile reset.
     */
    pReset = new_reset_data ();
    pReset->command = 'M';
    pReset->arg1 = pMobIndex->vnum;
    pReset->arg2 = is_number (arg2) ? atoi (arg2) : MAX_MOB;
    pReset->arg3 = pRoom->vnum;
    pReset->arg4 = is_number (argument) ? atoi (argument) : 1;
    add_reset (pRoom, pReset, 0 /* Last slot */ );

    /*
     * Create the mobile.
     */
    newmob = create_mobile (pMobIndex);
    char_to_room (newmob, pRoom);

    sprintf (output, "%s (%ld) has been loaded and added to resets.\r\n"
             "There will be a maximum of %d loaded to this room.\r\n",
             capitalize (pMobIndex->short_descr),
             pMobIndex->vnum, pReset->arg2);
    SEND (output, ch);
    act ("$n has created $N!", ch, NULL, newmob, TO_ROOM);
    return TRUE;
}



struct wear_type {
    int wear_loc;
    int wear_bit;
};



const struct wear_type wear_table[] = {
    {WEAR_NONE, ITEM_TAKE},
    {WEAR_LIGHT, ITEM_LIGHT},
    {WEAR_FINGER_L, ITEM_WEAR_FINGER},
    {WEAR_FINGER_R, ITEM_WEAR_FINGER},
    {WEAR_NECK_1, ITEM_WEAR_NECK},
    {WEAR_NECK_2, ITEM_WEAR_NECK},
    {WEAR_BODY, ITEM_WEAR_BODY},
    {WEAR_HEAD, ITEM_WEAR_HEAD},
    {WEAR_LEGS, ITEM_WEAR_LEGS},
    {WEAR_FEET, ITEM_WEAR_FEET},
    {WEAR_HANDS, ITEM_WEAR_HANDS},
    {WEAR_ARMS, ITEM_WEAR_ARMS},
    {WEAR_SHIELD, ITEM_WEAR_SHIELD},
    {WEAR_ABOUT, ITEM_WEAR_ABOUT},
    {WEAR_WAIST, ITEM_WEAR_WAIST},
    {WEAR_WRIST_L, ITEM_WEAR_WRIST},
    {WEAR_WRIST_R, ITEM_WEAR_WRIST},
    {WEAR_WIELD, ITEM_WIELD},
    {WEAR_HOLD, ITEM_HOLD},
    {NO_FLAG, NO_FLAG}
};



/*****************************************************************************
 Name:        wear_loc
 Purpose:    Returns the location of the bit that matches the count.
         1 = first match, 2 = second match etc.
 Called by:    oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc (int bits, int count)
{
    int flag;

    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
        if (IS_SET (bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }

    return NO_FLAG;
}



/*****************************************************************************
 Name:        wear_bit
 Purpose:    Converts a wear_loc into a bit.
 Called by:    redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit (int loc)
{
    int flag;

    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if (loc == wear_table[flag].wear_loc)
            return wear_table[flag].wear_bit;
    }

    return 0;
}



REDIT (redit_oreset)
{
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *newobj;
    OBJ_DATA *to_obj;
    CHAR_DATA *to_mob;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int olevel = 0;

    RESET_DATA *pReset;
    char output[MAX_STRING_LENGTH];

    EDIT_ROOM (ch, pRoom);

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || !is_number (arg1))
    {
        SEND ("Syntax:  oreset <vnum> <args>\r\n", ch);
        SEND ("        -no_args               = into room\r\n", ch);
        SEND ("        -<obj_name>            = into obj\r\n", ch);
        SEND ("        -<mob_name> <wear_loc> = into mob\r\n", ch);
        return FALSE;
    }

    if (!(pObjIndex = get_obj_index (atoi (arg1))))
    {
        SEND ("REdit: No object has that vnum.\r\n", ch);
        return FALSE;
    }

    if (pObjIndex->area != pRoom->area)
    {
        SEND ("REdit: No such object in this area.\r\n", ch);
        return FALSE;
    }

    /*
     * Load into room.
     */
    if (arg2[0] == '\0')
    {
        pReset = new_reset_data ();
        pReset->command = 'O';
        pReset->arg1 = pObjIndex->vnum;
        pReset->arg2 = 0;
        pReset->arg3 = pRoom->vnum;
        pReset->arg4 = 0;
        add_reset (pRoom, pReset, 0 /* Last slot */ );

        newobj = create_object (pObjIndex, number_fuzzy (olevel));
        obj_to_room (newobj, pRoom);

        sprintf (output, "%s (%ld) has been loaded and added to resets.\r\n",
                 capitalize (pObjIndex->short_descr), pObjIndex->vnum);
        SEND (output, ch);
    }
    else
        /*
         * Load into object's inventory.
         */
        if (argument[0] == '\0'
            && ((to_obj = get_obj_list (ch, arg2, pRoom->contents)) != NULL))
    {
        pReset = new_reset_data ();
        pReset->command = 'P';
        pReset->arg1 = pObjIndex->vnum;
        pReset->arg2 = 0;
        pReset->arg3 = to_obj->pIndexData->vnum;
        pReset->arg4 = 1;
        add_reset (pRoom, pReset, 0 /* Last slot */ );

        newobj = create_object (pObjIndex, number_fuzzy (olevel));
        newobj->cost = 0;
        obj_to_obj (newobj, to_obj);

        sprintf (output, "%s (%ld) has been loaded into "
                 "%s (%ld) and added to resets.\r\n",
                 capitalize (newobj->short_descr),
                 newobj->pIndexData->vnum,
                 to_obj->short_descr, to_obj->pIndexData->vnum);
        SEND (output, ch);
    }
    else
        /*
         * Load into mobile's inventory.
         */
    if ((to_mob = get_char_room ( ch, NULL, arg2)) != NULL)
    {
        int wear_loc;

        /*
         * Make sure the location on mobile is valid.
         */
        if ((wear_loc = flag_value (wear_loc_flags, argument)) == NO_FLAG)
        {
            SEND ("REdit: Invalid wear_loc.  '? wear-loc'\r\n", ch);
            return FALSE;
        }

        /*
         * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
         */
        if (!IS_SET (pObjIndex->wear_flags, wear_bit (wear_loc)))
        {
            sprintf (output,
                     "%s (%ld) has wear flags: [%s]\r\n",
                     capitalize (pObjIndex->short_descr),
                     pObjIndex->vnum,
                     flag_string (wear_flags, pObjIndex->wear_flags));
            SEND (output, ch);
            return FALSE;
        }

        /*
         * Can't load into same position.
         */
        if (get_eq_char (to_mob, wear_loc))
        {
            SEND ("REdit:  Object already equipped.\r\n", ch);
            return FALSE;
        }

        pReset = new_reset_data ();
        pReset->arg1 = pObjIndex->vnum;
        pReset->arg2 = wear_loc;
        if (pReset->arg2 == WEAR_NONE)
            pReset->command = 'G';
        else
            pReset->command = 'E';
        pReset->arg3 = wear_loc;

        add_reset (pRoom, pReset, 0 /* Last slot */ );

        olevel = URANGE (0, to_mob->level - 2, LEVEL_HERO);
        newobj = create_object (pObjIndex, number_fuzzy (olevel));

        if (to_mob->pIndexData->pShop)
        {                        /* Shop-keeper? */
            switch (pObjIndex->item_type)
            {
                default:
                    olevel = 0;
                    break;
                case ITEM_PILL:
                    olevel = number_range (0, 10);
                    break;
                case ITEM_POTION:
                    olevel = number_range (0, 10);
                    break;
                case ITEM_SCROLL:
                    olevel = number_range (5, 15);
                    break;
                case ITEM_WAND:
                    olevel = number_range (10, 20);
                    break;
                case ITEM_STAFF:
                    olevel = number_range (15, 25);
                    break;
                case ITEM_ARMOR:
                    olevel = number_range (5, 15);
                    break;
                case ITEM_WEAPON:
                    if (pReset->command == 'G')
                        olevel = number_range (5, 15);
                    else
                        olevel = number_fuzzy (olevel);
                    break;
            }

            newobj = create_object (pObjIndex, olevel);
            if (pReset->arg2 == WEAR_NONE)
                SET_BIT (newobj->extra_flags, ITEM_INVENTORY);
        }
        else
            newobj = create_object (pObjIndex, number_fuzzy (olevel));

        obj_to_char (newobj, to_mob);
        if (pReset->command == 'E')
            equip_char (to_mob, newobj, pReset->arg3);

        sprintf (output, "%s (%ld) has been loaded "
                 "%s of %s (%ld) and added to resets.\r\n",
                 capitalize (pObjIndex->short_descr),
                 pObjIndex->vnum,
                 flag_string (wear_loc_strings, pReset->arg3),
                 to_mob->short_descr, to_mob->pIndexData->vnum);
        SEND (output, ch);
    }
    else
    {                            /* Display Syntax */

        SEND ("REdit:  That mobile isn't here.\r\n", ch);
        return FALSE;
    }

    act ("$n has created $p!", ch, newobj, NULL, TO_ROOM);
    return TRUE;
}

char * get_tree_age(int value)
{
	switch (value)
	{
		default:
			return "sapling";
		case 0:
			return "sapling";
		case 1:
			return "young";
		case 2:
			return "adult";
		case 3:
			return "aged";
		case 4:
			return "ancient";
	}
}


/*
 * Object Editor Functions.
 */
void show_obj_values (CHAR_DATA * ch, OBJ_INDEX_DATA * obj)
{
	OBJ_INDEX_DATA *ing;
	OBJ_INDEX_DATA *result;
    char buf[MAX_STRING_LENGTH];

    switch (obj->item_type)
    {
        default:                /* No values. */
            break;
		
		case ITEM_COOKING_RECIPE:			
			ing = get_obj_index(obj->value[0]);
			sprintf (buf,"[v0] Main Ingredient:			[%s (%ld)]\r\n", (ing != NULL ? ing->short_descr : ""), (ing != NULL ? ing->vnum : -1));
			SEND(buf, ch);
			//
			ing = get_obj_index(obj->value[1]);
			sprintf (buf,"[v1] Secondary Ingredient:		[%s (%ld)]\r\n", (ing != NULL ? ing->short_descr : ""), (ing != NULL ? ing->vnum : -1));
			SEND(buf, ch);
			//
			ing = get_obj_index(obj->value[2]);
			sprintf (buf,"[v2] Tertiary Ingredient:		[%s (%ld)]\r\n", (ing != NULL ? ing->short_descr : ""), (ing != NULL ? ing->vnum : -1));
			SEND(buf, ch);
			//
			sprintf (buf,"[v3] Resulting Food:			[%s]\r\n", food_table[obj->value[3]].name);
			SEND(buf, ch);
			break;
		case ITEM_ALCHEMY_RECIPE:
			sprintf (buf,"[v0] Main Ingredient:         [%s]\r\n", herb_table[obj->value[0]].name);
			SEND(buf, ch);
			sprintf (buf,"[v1] Secondary Ingredient:    [%s]\r\n", herb_table[obj->value[1]].name);
			SEND(buf, ch);
			sprintf (buf,"[v2] Tertiary Ingredient:     [%s]\r\n", herb_table[obj->value[2]].name);
			SEND(buf, ch);
			sprintf (buf,"[v3] Resulting Brew:          [%s]\r\n", skill_table[obj->value[3]].name);
			SEND(buf, ch);
			break;		
		case ITEM_BLACKSMITH_PLANS:
			sprintf (buf,"[v0] Primary Ore:			[%s]\r\n", mat_table[get_ore(ALL_RARITY)].material);
			SEND(buf, ch);
			sprintf (buf,"[v1] Secondary Ore:		[%s]\r\n", mat_table[get_ore(ALL_RARITY)].material);
			SEND(buf, ch);
			sprintf (buf,"[v2] Extra Parts:		[%d]\r\n", obj->value[2]);
			SEND(buf, ch);
			//sprintf (buf,"[v3] Resulting Piece:			[%d]\r\n", obj->value[3]);
			//SEND(buf, ch);
			break;
		case ITEM_TAILORING_PLANS:
			sprintf (buf,"[v0] Main Fabric:             [%s]\r\n", obj->value[0] > -1 ? mat_table[get_cloth(obj->value[0])].material : mat_table[get_cloth(ALL_RARITY)].material);
			SEND(buf, ch);
			sprintf (buf,"[v1] Secondary Fabric:        [%s]\r\n", obj->value[1] > -1 ? mat_table[get_cloth(obj->value[1])].material : mat_table[get_cloth(ALL_RARITY)].material);
			SEND(buf, ch);
			sprintf (buf,"[v2] Dye:                     [%s]\r\n", mat_table[get_cloth(ALL_RARITY)].material);
			SEND(buf, ch);
			
			result = get_obj_index(obj->value[3]);
			sprintf (buf,"[v3] Resulting Piece:         [%s (%ld)]\r\n", (result != NULL ? result->short_descr : ""), (result != NULL ? result->vnum : -1));
			SEND(buf, ch);
			break;		
		
		case ITEM_FISH_POLE:
			sprintf (buf,"[v0] Fishing Bonus:			[%d]\r\n", obj->value[0]);
			SEND(buf,ch);
			sprintf (buf,"[v1] Strength:    			[%d]\r\n", obj->value[1]);
			SEND(buf,ch);
			break;
		
		case ITEM_MINING_TOOL:		
		case ITEM_LOOM:
		case ITEM_ANVIL:
		case ITEM_ALCHEMY_LAB:
		case ITEM_BLACKSMITH_HAMMER:
			sprintf (buf,"[v0] Crafting Bonus:			[%d]\r\n", obj->value[0]);
			SEND(buf,ch);
			break;

		case ITEM_TREE:
			sprintf (buf,"[v0] Height (in feet):		[%d]\r\n", obj->value[0]);
			SEND(buf,ch);
			sprintf (buf,"[v1] Age:						[%s]\r\n", get_tree_age(obj->value[1]));
			SEND(buf,ch);
			break;
	
		case ITEM_BANDAGE:
			sprintf (buf, "[v0] Bonus:                  [%d]\r\n", obj->value[0]);
			SEND(buf, ch);
			sprintf (buf, "[v1] Heals:                  [%d]\r\n", obj->value[1]);
			SEND(buf, ch);
			break;
	
		case ITEM_MISSILE:
			sprintf (buf,"[v0] Missile Type:		[%d]\r\n", obj->value[0]);
			SEND(buf,ch);
			sprintf (buf,"[v1] Damage:      		[%d]\r\n", obj->value[1]);
			SEND(buf,ch);
			sprintf (buf,"[v2] Damage Type:   		[%d]\r\n", obj->value[2]);
			SEND(buf,ch);
			break;
			
		case ITEM_FIRE:
			sprintf (buf,"[v0] Magical:	        	[%s]\r\n", obj->value[0] == 1 ? "yes" : "no");
			SEND(buf,ch);
			break;
			
		case ITEM_BUILDING:
			sprintf (buf,"[v0] Climbable:       	[%s]\r\n", obj->value[0] == 1 ? "yes" : "no");
			SEND(buf,ch);
			break;
			
		case ITEM_SHOVEL:
			sprintf (buf,"[v0] Digging Bonus:   	[%3d]\r\n", obj->value[0]);
			SEND(buf,ch);
			break;
		
        case ITEM_LIGHT:
			if (obj->value[1] == 0)
				SEND ("[v1] Enclosed:    no\r\n",ch);
			if (obj->value[1] == 1)
				SEND ("[v1] Enclosed:    yes\r\n",ch);
            if (obj->value[2] == -1 || obj->value[2] == 999)    /* ROM OLC */
                sprintf (buf, "[v2] Light:       Infinite [-1]\r\n");
            else
                sprintf (buf, "[v2] Light:       [%d]\r\n", obj->value[2]);
            SEND (buf, ch);
            break;		
			
        case ITEM_WAND:
        case ITEM_STAFF:
		case ITEM_FIGURINE:
            sprintf (buf,
                     "[v0] Level:          [%d]\r\n"
                     "[v1] Charges Total:  [%d]\r\n"
                     "[v2] Charges Left:   [%d]\r\n"
                     "[v3] Spell:          %s\r\n",
                     obj->value[0],
                     obj->value[1],
                     obj->value[2],
                     obj->value[3] != -1 ? skill_table[obj->value[3]].name
                     : "none");
            SEND (buf, ch);
            break;

        case ITEM_PORTAL:
            sprintf (buf,
                     "[v0] Charges:        [%d]\r\n"
                     "[v1] Exit Flags:     %s\r\n"
                     "[v2] Portal Flags:   %s\r\n"
                     "[v3] Goes to (vnum): [%d]\r\n",
                     obj->value[0],
                     flag_string (exit_flags, obj->value[1]),
                     flag_string (portal_flags, obj->value[2]),
                     obj->value[3]);
            SEND (buf, ch);
            break;

        case ITEM_FURNITURE:
            sprintf (buf,
                     "[v0] Max people:      [%d]\r\n"
                     "[v1] Max weight:      [%d]\r\n"
                     "[v2] Furniture Flags: %s\r\n"
                     "[v3] Heal bonus:      [%d]\r\n"
                     "[v4] Mana bonus:      [%d]\r\n",
                     obj->value[0],
                     obj->value[1],
                     flag_string (furniture_flags, obj->value[2]),
                     obj->value[3], obj->value[4]);
            SEND (buf, ch);
            break;

		case ITEM_HERB:
			sprintf (buf, 
					"[v0] Herb Type:	     [%s]\r\n",
					herb_table[obj->value[0]].name);
			SEND(buf, ch);
			
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf (buf,
                     "[v0] Level:  [%d]\r\n"
                     "[v1] Spell:  %s\r\n"
                     "[v2] Spell:  %s\r\n"
                     "[v3] Spell:  %s\r\n"
                     "[v4] Spell:  %s\r\n",
                     obj->value[0],
                     obj->value[1] != -1 ? skill_table[obj->value[1]].name
                     : "none",
                     obj->value[2] != -1 ? skill_table[obj->value[2]].name
                     : "none",
                     obj->value[3] != -1 ? skill_table[obj->value[3]].name
                     : "none",
                     obj->value[4] != -1 ? skill_table[obj->value[4]].name
                     : "none");
            SEND (buf, ch);
			break;
		
		case ITEM_THIEVES_TOOLS:
			sprintf (buf, 
					"[v0] Pick Lock Bonus:	[%d]\r\n"
					"[v1] Trap Bonus:		[%d]\r\n",
					obj->value[0], obj->value[1]);
			SEND(buf, ch);
			break;		
			
		case ITEM_INSTRUMENT:
			sprintf( buf, "[v0] Instrument class:   %s\r\n",
			flag_string( instrument_type_flags, obj->value[0] ) );
			send_to_char( buf, ch );
			sprintf( buf, "[v1] Quality:	         %s\r\n",
			flag_string( instrument_quality_flags, obj->value[1] ) );
			send_to_char( buf, ch );
			/*sprintf( buf, "[v2] null	    %d\n\r",
			flag_string( 0, obj->value[2] ) );
			send_to_char( buf, ch );
			sprintf( buf, "[v3] null	    %d\n\r",
			flag_string( 0, obj->value[3] ) );
			send_to_char( buf, ch );
			sprintf( buf, "[v4] null	    %d\n\r",
			flag_string( 0, obj->value[4] ) );
			send_to_char( buf, ch );
			*/
			break;

/* ARMOR for ROM */

        case ITEM_ARMOR:
            sprintf (buf,
                     "[v0] Ac pierce       [%d]\r\n"
                     "[v1] Ac bash         [%d]\r\n"
                     "[v2] Ac slash        [%d]\r\n"
                     "[v3] Ac exotic       [%d]\r\n"
					 "[v4] Bulk			   [%d]\r\n",
                     obj->value[0], obj->value[1], obj->value[2],
                     obj->value[3], obj->value[4]);
            SEND (buf, ch);
            break;
		
		case ITEM_KEY:
			sprintf (buf, 
					"[v0] Single use		[%s]\r\n",
					obj->value[0] < 1 ? "no" : "yes");
			SEND (buf, ch);
			break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
        case ITEM_WEAPON:
            sprintf (buf, "[v0] Weapon class:   %s\r\n",
                     flag_string (weapon_class, obj->value[0]));
            SEND (buf, ch);
            sprintf (buf, "[v1] Number of dice: [%d]\r\n", obj->value[1]);
            SEND (buf, ch);
            sprintf (buf, "[v2] Type of dice:   [%d]\r\n", obj->value[2]);
            SEND (buf, ch);
            sprintf (buf, "[v3] Type:           %s\r\n",
                     attack_table[obj->value[3]].name);
            SEND (buf, ch);
            sprintf (buf, "[v4] Special type:   %s\r\n",
                     flag_string (weapon_type2, obj->value[4]));
            SEND (buf, ch);
            break;

        case ITEM_CONTAINER:
            sprintf (buf,
                     "[v0] Weight:     [%d lbs]\r\n"
                     "[v1] Flags:      [%s]\r\n"
                     "[v2] Key:     %s [%d]\r\n"
                     "[v3] Capacity    [%d]\r\n"
                     "[v4] Weight Mult [%d]\r\n",
                     obj->value[0],
                     flag_string (container_flags, obj->value[1]),
                     get_obj_index (obj->value[2])
                     ? get_obj_index (obj->value[2])->short_descr
                     : "none", obj->value[2], obj->value[3], obj->value[4]);
            SEND (buf, ch);
            break;

        case ITEM_DRINK_CON:
		case ITEM_FLASK:
            sprintf (buf,
                     "[v0] Liquid Total: [%d]\r\n"
                     "[v1] Liquid Left:  [%d]\r\n"
                     "[v2] Liquid:       %s\r\n"
                     "[v3] Poisoned:     %s\r\n",
                     obj->value[0],
                     obj->value[1],
                     liq_table[obj->value[2]].liq_name,
                     obj->value[3] != 0 ? "Yes" : "No");
            SEND (buf, ch);
            break;

        case ITEM_FOUNTAIN:
            sprintf (buf,
                     "[v0] Liquid Total: [%d]\r\n"
                     "[v1] Liquid Left:  [%d]\r\n"
                     "[v2] Liquid:        %s\r\n",
                     obj->value[0],
                     obj->value[1], liq_table[obj->value[2]].liq_name);
            SEND (buf, ch);
            break;

        case ITEM_FOOD:
            sprintf (buf,
                     "[v0] Food hours: [%d]\r\n"
                     "[v1] Full hours: [%d]\r\n"
                     "[v3] Poisoned:   %s\r\n",
                     obj->value[0],
                     obj->value[1], obj->value[3] != 0 ? "Yes" : "No");
            SEND (buf, ch);
            break;

        case ITEM_MONEY:
            sprintf (buf, "[v0] Gold:   [%d]\r\n", obj->value[0]);
            SEND (buf, ch);
            break;
    }

    return;
}



bool set_obj_values (CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, int value_num,
                     char *argument)					
{
	int vnum = 0;
	
    switch (pObj->item_type)
    {
        default:
            break;

		case ITEM_THIEVES_TOOLS:
			switch (value_num)
			{
				default:
					do_help (ch, "ITEM_THIEVES_TOOLS");
					return FALSE;
				case 0:
					SEND ("Pick lock bonus/negative set.\r\r\n\n", ch);
					pObj->value[0] = atoi (argument);
					break;
				case 1:
					SEND ("Trap removing/laying bonus/negative set.\r\r\n\n", ch);
					pObj->value[1] = atoi (argument);
					break;
			}
			break;
				
		case ITEM_TAILORING_PLANS:
			switch (value_num)
			{
				int vnum = 0, cloth = 0;
				OBJ_INDEX_DATA *result = NULL;
				//OBJ_INDEX_DATA *dye = NULL;
				
				default:
					do_help (ch, "ITEM_TAILORING_PLANS");
					return FALSE;
				case 0:
				case 1:														
					if (mat_table[get_material(argument)].is_cloth != TRUE || !valid_material(argument))					
					{
						SEND("That isn't a valid cloth!\r\n",ch);
						return FALSE;
					}					
					cloth = get_material(argument);
					pObj->value[value_num] = cloth;
					SEND ("Cloth set!\r\n\r\n",ch);
					break;
				case 2:
					break;
				case 3:
					vnum = atoi(argument);
					if ((result = get_obj_index(vnum)) == NULL)
					{
						SEND("That item doesn't exist!\r\n",ch);
						return FALSE;
					}
					if (!IS_CLOTH(result->material))
					{
						SEND("That's not a valid result.\r\n",ch);
						return FALSE;
					}
					pObj->value[value_num] = vnum;
					SEND ("Result set!\r\n",ch);
					break;
			}
			break;
		case ITEM_COOKING_RECIPE:
			switch (value_num)
			{				
				OBJ_INDEX_DATA *ing = NULL;
				
				default:
					do_help (ch, "ITEM_COOKING_RECIPE");
					return FALSE;
				case 0:
				case 1:
				case 2:
					vnum = atoi(argument);
					if ((ing = get_obj_index(vnum)) == NULL)
					{
						SEND("That ingredient doesn't exist!\r\n",ch);
						return FALSE;
					}
					if (ing->item_type != ITEM_INGREDIENT)
					{
						SEND("That's not a valid ingredient.\r\n",ch);
						return FALSE;
					}
					pObj->value[value_num] = vnum;
					SEND ("Ingredient set!\r\n",ch);
					break;
				case 3:
					if (food_table[food_lookup(argument)].name)
					{
						SEND ("Food result set.\r\n", ch);
						pObj->value[3] = food_lookup (argument);
						break;
					}
					else
					{
						SEND ("Invalid food result.\r\n",ch);
						return FALSE;
					}                    
					break;
			}
			break;
		case ITEM_INSTRUMENT:
            switch ( value_num )
            {
                default:
                    do_help( ch, "ITEM_INSTRUMENT" );
                    return FALSE;
                case 0:
                    send_to_char( "INSTRUMENT TYPE SET.\n\r\n\r", ch );
                    pObj->value[0] = flag_value( instrument_type_flags, argument );
                    break;
                case 1:
                    send_to_char( "INSTRUMENT QUALITY SET.\n\r\n\r", ch );
                    pObj->value[1] = flag_value( instrument_quality_flags, argument );
					//pObj->value[1] = instrument_quality(argument );
					//pObj->value[1] = ( instrument_quality, argument );
                    break;
            }
            break;
		case ITEM_ALCHEMY_RECIPE:
			switch (value_num)
			{
				default:
					do_help (ch, "ITEM_ALCHEMY_RECIPE");
					return FALSE;
				case 0:
					if (!str_cmp(argument, "none"))
					{
						SEND ("Main ingredient set.\r\n",ch);
						pObj->value[0] = -1;
						return TRUE;
					}
					if (valid_herb(argument))
					{
						int herb;
						herb = get_herb(argument);
						SEND ("Main ingredient set.\r\n",ch);						
						pObj->value[0] = herb;
						return TRUE;
					}
					else
					{
						SEND ("Not a valid herb.\r\n",ch);
						return FALSE;
					}
				case 1:
					if (!str_cmp(argument, "none"))
					{
						SEND ("Secondary ingredient set.\r\n",ch);
						pObj->value[1] = -1;
						return TRUE;
					}
					if (valid_herb(argument))
					{
						int herb;
						herb = get_herb(argument);
						SEND ("Secondary ingredient set.\r\n",ch);						
						pObj->value[1] = herb;
						return TRUE;
					}
					else
					{
						SEND ("Not a valid herb.\r\n",ch);
						return FALSE;
					}
				case 2:
					if (!str_cmp(argument, "none"))
					{
						SEND ("Tertiary ingredient set.\r\n",ch);
						pObj->value[2] = -1;
						return TRUE;
					}
					if (valid_herb(argument))
					{
						int herb;
						herb = get_herb(argument);
						SEND ("Tertiary ingredient set.\r\n",ch);						
						pObj->value[2] = herb;
						return TRUE;
					}
					else
					{
						SEND ("Not a valid herb.\r\n",ch);
						return FALSE;
					}
				case 3:					                    
					if (skill_table[skill_lookup(argument)].spell_level > -1 && skill_table[skill_lookup(argument)].can_scribe)
					{
						SEND ("Spell result set.\r\n", ch);
						pObj->value[3] = skill_lookup (argument);
						break;
					}
					else
					{
						SEND ("Invalid spell result.\r\n",ch);
						return FALSE;
					}                    
			}
			break;
		case ITEM_FISH_POLE:
			switch (value_num)
			{
				default:
					do_help (ch, "ITEM_CRAFT");
					return FALSE;
				case 0:
					SEND ("Fishing bonus set.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;									
				case 1:
					SEND ("Pole strength set.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;							
			}
			break;
			
		case ITEM_MINING_TOOL:		
		case ITEM_LOOM:
		case ITEM_ANVIL:
		case ITEM_ALCHEMY_LAB:
		case ITEM_BLACKSMITH_HAMMER:
			switch (value_num)
			{
				default:
					do_help (ch, "ITEM_CRAFT");
					return FALSE;
				case 0:
					SEND ("Crafting bonus set.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;									
			}
			break;
		case ITEM_BANDAGE:
			switch (value_num)
			{
				default:
					do_help (ch, "ITEM_BANDAGE");
					return FALSE;
				case 0:
					SEND ("Bandage bonus set.\r\r\n\n", ch);
					pObj->value[0] = atoi (argument);
					break;
				case 1:
					SEND ("Bandage healing amount set.\r\r\n\n", ch);
					pObj->value[1] = atoi (argument);
					break;
			}
			break;
		case ITEM_TREE:
			switch (value_num)
			{	
				default:
					do_help (ch, "ITEM_TREE");
					return FALSE;
				case 0:
					SEND ("Height set.\r\r\n\n", ch);
					pObj->value[0] = atoi (argument);
					break;
				case 1:
					if (!str_cmp(argument, "sapling"))
					{
						SEND ("Tree age set.\r\r\n\n",ch);
						pObj->value[1] = 0;
						break;
					}				
					else if (!str_cmp(argument, "young"))
					{
						SEND ("Tree age set.\r\r\n\n",ch);
						pObj->value[1] = 1;
						break;
					}				
					else if (!str_cmp(argument, "adult"))
					{
						SEND ("Tree age set.\r\r\n\n",ch);
						pObj->value[1] = 2;
						break;
					}			
					else if (!str_cmp(argument, "aged"))
					{
						SEND ("Tree age set.\r\r\n\n",ch);
						pObj->value[1] = 3;
						break;
					}		
					else if (!str_cmp(argument, "ancient"))
					{
						SEND ("Tree age set.\r\r\n\n",ch);
						pObj->value[1] = 4;
						break;
					}
					else
					{
						SEND ("Valid ages are: sapling, young, adult, aged, ancient.\r\n",ch);
						return FALSE;
					}
					
			}
			break;
		
		case ITEM_KEY:
			switch (value_num)
			{
				default:	
					do_help (ch, "ITEM_KEY");
					return FALSE;
				case 0:
					if (!str_cmp(argument, "on") || !str_cmp(argument, "yes"))
					{
						SEND ("SINGLE USE SET.\r\r\n\n",ch);
						pObj->value[0] = 1;						
					}
					if (!str_cmp(argument, "off") || !str_cmp(argument, "no"))
					{
						SEND ("SINGLE USE REMOVED.\r\r\n\n",ch);
						pObj->value[0] = 0;						
					}
					break;		
			}
			break;
        case ITEM_LIGHT:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_LIGHT");
                    return FALSE;
				case 1:
					if (!str_cmp(argument, "on") || !str_cmp(argument, "yes"))
					{
						SEND ("ENCLOSED SET.\r\r\n\n",ch);
						pObj->value[1] = 1;						
					}
					if (!str_cmp(argument, "off") || !str_cmp(argument, "no"))
					{
						SEND ("ENCLOSED REMOVED.\r\r\n\n",ch);
						pObj->value[1] = 0;						
					}
					break;
                case 2:
                    SEND ("HOURS OF LIGHT SET.\r\r\n\n", ch);
                    pObj->value[2] = atoi (argument);
                    break;
            }
            break;		
		case ITEM_MISSILE:
			switch (value_num)
			{
				default:
					do_help (ch, "ITEM_MISSILE");
					return FALSE;
				case 0:
					if (!str_cmp(argument, "arrow"))
					{
						SEND ("Missile Type:   arrow\r\r\n\n",ch);
						pObj->value[0] = MISSILE_ARROW;
						break;
					}
					if (!str_cmp(argument, "bolt"))
					{
						SEND ("Missile Type:   bolt\r\r\n\n",ch);
						pObj->value[0] = MISSILE_BOLT;
						break;
					}
					break;
				case 1: //damage
					break;
				case 2: //dam type
					break;
			}
			break;
			
		case ITEM_FIRE:	
		
			switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_FIRE");
                    return FALSE;
				case 1:
					if (!str_cmp(argument, "on") || !str_cmp(argument, "yes"))
					{
						SEND ("MAGICAL SET.\r\r\n\n",ch);
						pObj->value[1] = 1;						
					}
					if (!str_cmp(argument, "off") || !str_cmp(argument, "no"))
					{
						SEND ("MAGICAL REMOVED.\r\r\n\n",ch);
						pObj->value[1] = 0;						
					}
					break;                
            }
            break;		
		
        case ITEM_SHOVEL:
		    
			
			switch (value_num)
            {				
                default:
                    do_help (ch, "ITEM_SHOVEL");    
                    return FALSE;
                case 0:
                    if (atoi(argument) > 30 || atoi(argument) < -10)
					{
						do_help (ch, "ITEM_SHOVEL");                    
						break;
					}
					else
					{
						char buf[MSL];
						sprintf(buf, "DIGGING BONUS SET (%s)\r\r\n\n", argument);
						SEND (buf, ch);
						pObj->value[0] = atoi(argument);                
						break;
					}                              
            }
			break;
			
		case ITEM_FIGURINE:		            
		case ITEM_WAND:
        case ITEM_STAFF:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_STAFF_WAND");
                    return FALSE;
                case 0:
                    SEND ("SPELL LEVEL SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("TOTAL NUMBER OF CHARGES SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    SEND ("CURRENT NUMBER OF CHARGES SET.\r\r\n\n",
                                  ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    SEND ("SPELL TYPE SET.\r\n", ch);
                    pObj->value[3] = skill_lookup (argument);
                    break;
            }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_SCROLL_POTION_PILL");
                    return FALSE;
                case 0:
                    SEND ("SPELL LEVEL SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("SPELL TYPE 1 SET.\r\r\n\n", ch);
                    pObj->value[1] = skill_lookup (argument);
                    break;
                case 2:
                    SEND ("SPELL TYPE 2 SET.\r\r\n\n", ch);
                    pObj->value[2] = skill_lookup (argument);
                    break;
                case 3:
                    SEND ("SPELL TYPE 3 SET.\r\r\n\n", ch);
                    pObj->value[3] = skill_lookup (argument);
                    break;
                case 4:
                    SEND ("SPELL TYPE 4 SET.\r\r\n\n", ch);
                    pObj->value[4] = skill_lookup (argument);
                    break;
            }
            break;

/* ARMOR for ROM: */

        case ITEM_ARMOR:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_ARMOR");
                    return FALSE;
                case 0:
                    SEND ("AC PIERCE SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("AC BASH SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    SEND ("AC SLASH SET.\r\r\n\n", ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    SEND ("AC EXOTIC SET.\r\r\n\n", ch);
                    pObj->value[3] = atoi (argument);
                    break;
				case 4:
					SEND ("Bulk set.\r\r\n\n",ch);
					pObj->value[4] = atoi (argument);
					break;
            }
            break;

/* WEAPONS changed in ROM */

        case ITEM_WEAPON:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_WEAPON");
                    return FALSE;
                case 0:
                    SEND ("WEAPON CLASS SET.\r\r\n\n", ch);
                    ALT_FLAGVALUE_SET (pObj->value[0], weapon_class,
                                       argument);
                    break;
                case 1:
                    SEND ("NUMBER OF DICE SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    SEND ("TYPE OF DICE SET.\r\r\n\n", ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    SEND ("WEAPON TYPE SET.\r\r\n\n", ch);
                    pObj->value[3] = attack_lookup (argument);
                    break;
                case 4:
                    SEND ("SPECIAL WEAPON TYPE TOGGLED.\r\r\n\n", ch);
                    ALT_FLAGVALUE_TOGGLE (pObj->value[4], weapon_type2,
                                          argument);
                    break;
            }
            break;

        case ITEM_PORTAL:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_PORTAL");
                    return FALSE;

                case 0:
                    SEND ("CHARGES SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("EXIT FLAGS SET.\r\r\n\n", ch);
                    ALT_FLAGVALUE_SET (pObj->value[1], exit_flags, argument);
                    break;
                case 2:
                    SEND ("PORTAL FLAGS SET.\r\r\n\n", ch);
                    ALT_FLAGVALUE_SET (pObj->value[2], portal_flags,
                                       argument);
                    break;
                case 3:
                    SEND ("EXIT VNUM SET.\r\r\n\n", ch);
                    pObj->value[3] = atoi (argument);
                    break;
            }
            break;

        case ITEM_FURNITURE:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_FURNITURE");
                    return FALSE;

                case 0:
                    SEND ("NUMBER OF PEOPLE SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("MAX WEIGHT SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    SEND ("FURNITURE FLAGS TOGGLED.\r\r\n\n", ch);
                    ALT_FLAGVALUE_TOGGLE (pObj->value[2], furniture_flags,
                                          argument);
                    break;
                case 3:
                    SEND ("HEAL BONUS SET.\r\r\n\n", ch);
                    pObj->value[3] = atoi (argument);
                    break;
                case 4:
                    SEND ("MANA BONUS SET.\r\r\n\n", ch);
                    pObj->value[4] = atoi (argument);
                    break;
            }
            break;

        case ITEM_CONTAINER:
            switch (value_num)
            {
                    int value;

                default:
                    do_help (ch, "ITEM_CONTAINER");
                    return FALSE;
                case 0:
                    SEND ("WEIGHT CAPACITY SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    if ((value = flag_value (container_flags, argument)) !=
                        NO_FLAG)
                        TOGGLE_BIT (pObj->value[1], value);
                    else
                    {
                        do_help (ch, "ITEM_CONTAINER");
                        return FALSE;
                    }
                    SEND ("CONTAINER TYPE SET.\r\r\n\n", ch);
                    break;
                case 2:
                    if (atoi (argument) != 0)
                    {
                        if (!get_obj_index (atoi (argument)))
                        {
                            SEND ("THERE IS NO SUCH ITEM.\r\r\n\n",
                                          ch);
                            return FALSE;
                        }

                        if (get_obj_index (atoi (argument))->item_type !=
                            ITEM_KEY)
                        {
                            SEND ("THAT ITEM IS NOT A KEY.\r\r\n\n",
                                          ch);
                            return FALSE;
                        }
                    }
                    SEND ("CONTAINER KEY SET.\r\r\n\n", ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    SEND ("CONTAINER MAX WEIGHT SET.\r\n", ch);
                    pObj->value[3] = atoi (argument);
                    break;
                case 4:
                    SEND ("WEIGHT MULTIPLIER SET.\r\r\n\n", ch);
                    pObj->value[4] = atoi (argument);
                    break;
            }
            break;

        case ITEM_DRINK_CON:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_DRINK");
/* OLC            do_help( ch, "liquids" );    */
                    return FALSE;
                case 0:
                    SEND
                        ("MAXIMUM AMOUT OF LIQUID HOURS SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND
                        ("CURRENT AMOUNT OF LIQUID HOURS SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    SEND ("LIQUID TYPE SET.\r\r\n\n", ch);
                    pObj->value[2] = (liq_lookup (argument) != -1 ?
                                      liq_lookup (argument) : 0);
                    break;
                case 3:
                    SEND ("POISON VALUE TOGGLED.\r\r\n\n", ch);
                    pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
                    break;
            }
            break;

        case ITEM_FOUNTAIN:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_FOUNTAIN");
/* OLC            do_help( ch, "liquids" );    */
                    return FALSE;
                case 0:
                    SEND
                        ("MAXIMUM AMOUT OF LIQUID HOURS SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND
                        ("CURRENT AMOUNT OF LIQUID HOURS SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    SEND ("LIQUID TYPE SET.\r\r\n\n", ch);
                    pObj->value[2] = (liq_lookup (argument) != -1 ?
                                      liq_lookup (argument) : 0);
                    break;
            }
            break;

        case ITEM_FOOD:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_FOOD");
                    return FALSE;
                case 0:
                    SEND ("HOURS OF FOOD SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("HOURS OF FULL SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 3:
                    SEND ("POISON VALUE TOGGLED.\r\r\n\n", ch);
                    pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
                    break;
            }
            break;

        case ITEM_MONEY:
            switch (value_num)
            {
                default:
                    do_help (ch, "ITEM_MONEY");
                    return FALSE;
                case 0:
                    SEND ("GOLD AMOUNT SET.\r\r\n\n", ch);
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    SEND ("SILVER AMOUNT SET.\r\r\n\n", ch);
                    pObj->value[1] = atoi (argument);
                    break;
            }
            break;
    }

    show_obj_values (ch, pObj);

    return TRUE;
}



OEDIT (oedit_show)
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
	PROG_LIST			*list;
    int cnt;

    EDIT_OBJ (ch, pObj);

    sprintf (buf, "Name:         {g[{x%s{g]{x\r\nArea:         {g[{x%5d{g]{x %s\r\n",
             pObj->name,
             !pObj->area ? -1 : pObj->area->vnum,
             !pObj->area ? "No Area" : pObj->area->name);
    SEND (buf, ch);


    sprintf (buf, "Vnum:         {g[{x%5ld{g]{x\r\nType:         {g[{x%s{g]{x\r\n",
             pObj->vnum, flag_string (type_flags, pObj->item_type));
    SEND (buf, ch);

    sprintf (buf, "Level:        {g[{x%d{g]{x\r\n", pObj->level);
    SEND (buf, ch);

    sprintf (buf, "Wear flags:   {g[{x%s{g]{x\r\n",
             flag_string (wear_flags, pObj->wear_flags));
    SEND (buf, ch);

    sprintf (buf, "Extra flags:  {g[{x%s{g]{x\r\n",
             flag_string (extra_flags, pObj->extra_flags));
    SEND (buf, ch);
	
	sprintf (buf, "Extra2 flags: {g[{x%s{g]{x\r\n",
             flag_string (extra2_flags, pObj->extra2_flags));
    SEND (buf, ch);

    sprintf (buf, "Material:     {g[{x%s{g]{x\r\n",    /* ROM */ // Expanded by Upro
             pObj->material);
    SEND (buf, ch);

	if (IS_SET(pObj->extra_flags, ITEM_HAD_TIMER))
	{
		sprintf (buf, "Timer:        {g[{x%d{g]{x\r\n",    /* ROM */
			pObj->timer);
		SEND (buf, ch);
	}
		
    sprintf (buf, "Condition:    {g[{x%d{g]{x\r\n",    /* ROM */
             pObj->condition);
    SEND (buf, ch);

    // sprintf (buf, "Quality:		 [%d]\r\n",  
			// pObj->quality);
    // SEND (buf, ch);
	
    sprintf (buf, "Weight:       {g[{x%d{g]{x\r\nCost:         {g[{x%d{g]{x\r\n",
             pObj->weight, pObj->cost);
    SEND (buf, ch);

    if (pObj->extra_descr)
    {
        EXTRA_DESCR_DATA *ed;

        SEND ("Ex desc kwd: ", ch);

        for (ed = pObj->extra_descr; ed; ed = ed->next)
        {
            SEND ("{g[{x", ch);
            SEND (ed->keyword, ch);
            SEND ("{g]{x", ch);
        }

        SEND ("\r\n", ch);
    }

    sprintf (buf, "Short desc:  %s\r\nLong desc:\r\n     %s\r\n",
             pObj->short_descr, pObj->description);
    SEND (buf, ch);

    for (cnt = 0, paf = pObj->affected; paf; paf = paf->next)
    {
        if (cnt == 0)
        {
            SEND ("Number Modifier Affects\r\n", ch);
            SEND ("------ -------- -------\r\n", ch);
        }
        sprintf (buf, "{g[{x%4d{g]{x %-8d %s\r\n", cnt,
                 paf->modifier, flag_string (apply_flags, paf->location));
        SEND (buf, ch);
        cnt++;
    }

    show_obj_values (ch, pObj);
	if ( pObj->oprogs )
		{
		int cnt;
	 
		sprintf(buf, "\r\nOBJPrograms for {g[{x%5ld{g]{x:\r\n", pObj->vnum);
		SEND( buf, ch );
	 
		for (cnt=0, list=pObj->oprogs; list; list=list->next)
		{
			if (cnt ==0)
			{
				SEND ( " Number Vnum  Trigger Phrase\r\n", ch );
				SEND ( " ------ ----- ------- ------\r\n", ch );
			}
	 
			sprintf(buf, "{g[{x%5d{g]{x %5ld %7s %s\r\n", cnt,
				list->vnum,prog_type_to_name(list->trig_type),
				list->trig_phrase);
			SEND( buf, ch );
			cnt++;
		}
    }
    return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT (oedit_addaffect)
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ (ch, pObj);

    argument = one_argument (argument, loc);
    one_argument (argument, mod);

    if (loc[0] == '\0' || mod[0] == '\0' || !is_number (mod))
    {
        SEND ("Syntax:  addaffect [location] [#xmod]\r\n", ch);
        return FALSE;
    }

    if ((value = flag_value (apply_flags, loc)) == NO_FLAG)
    {                            /* Hugin */
        SEND ("Valid affects are:\r\n", ch);
        show_help (ch, "apply");
        return FALSE;
    }

    pAf = new_affect ();
    pAf->location = value;
    pAf->modifier = atoi (mod);
    pAf->where = TO_OBJECT;
    pAf->type = -1;
    pAf->duration = -1;
    pAf->bitvector = 0;
    pAf->level = pObj->level;
    pAf->next = pObj->affected;
    pObj->affected = pAf;

    SEND ("Affect added.\r\n", ch);
    return TRUE;
}

OEDIT (oedit_addapply)
{
    int value, bv, typ;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char type[MAX_STRING_LENGTH];
    char bvector[MAX_STRING_LENGTH];

    EDIT_OBJ (ch, pObj);

    argument = one_argument (argument, type);
    argument = one_argument (argument, loc);
    argument = one_argument (argument, mod);
    one_argument (argument, bvector);

    if (type[0] == '\0' || (typ = flag_value (apply_types, type)) == NO_FLAG)
    {
        SEND ("Invalid apply type. Valid apply types are:\r\n", ch);
        show_help (ch, "apptype");
        return FALSE;
    }

    if (loc[0] == '\0' || (value = flag_value (apply_flags, loc)) == NO_FLAG)
    {
        SEND ("Valid applys are:\r\n", ch);
        show_help (ch, "apply");
        return FALSE;
    }

    if (bvector[0] == '\0'
        || (bv = flag_value (bitvector_type[typ].table, bvector)) == NO_FLAG)
    {
        SEND ("Invalid bitvector type.\r\n", ch);
        SEND ("Valid bitvector types are:\r\n", ch);
        show_help (ch, bitvector_type[typ].help);
        return FALSE;
    }

    if (mod[0] == '\0' || !is_number (mod))
    {
        SEND
            ("Syntax:  addapply [type] [location] [#xmod] [bitvector]\r\n",
             ch);
        return FALSE;
    }

    pAf = new_affect ();
    pAf->location = value;
    pAf->modifier = atoi (mod);
    pAf->where = apply_types[typ].bit;
    pAf->type = -1;
    pAf->duration = -1;
    pAf->bitvector = bv;
    pAf->level = pObj->level;
    pAf->next = pObj->affected;
    pObj->affected = pAf;

    SEND ("Apply added.\r\n", ch);
    return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT (oedit_delaffect)
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_OBJ (ch, pObj);

    one_argument (argument, affect);

    if (!is_number (affect) || affect[0] == '\0')
    {
        SEND ("Syntax:  delaffect [#xaffect]\r\n", ch);
        return FALSE;
    }

    value = atoi (affect);

    if (value < 0)
    {
        SEND ("Only non-negative affect-numbers allowed.\r\n", ch);
        return FALSE;
    }

    if (!(pAf = pObj->affected))
    {
        SEND ("OEdit:  Non-existant affect.\r\n", ch);
        return FALSE;
    }

    if (value == 0)
    {                            /* First case: Remove first affect */
        pAf = pObj->affected;
        pObj->affected = pAf->next;
        free_affect (pAf);
    }
    else
    {                            /* Affect to remove is not the first */

        while ((pAf_next = pAf->next) && (++cnt < value))
            pAf = pAf_next;

        if (pAf_next)
        {                        /* See if it's the next affect */
            pAf->next = pAf_next->next;
            free_affect (pAf_next);
        }
        else
        {                        /* Doesn't exist */

            SEND ("No such affect.\r\n", ch);
            return FALSE;
        }
    }

    SEND ("Affect removed.\r\n", ch);
    return TRUE;
}



OEDIT (oedit_name)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  name [string]\r\n", ch);
        return FALSE;
    }

    free_string (pObj->name);
    pObj->name = str_dup (argument);

    SEND ("Name set.\r\n", ch);
    return TRUE;
}



OEDIT (oedit_short)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  short [string]\r\n", ch);
        return FALSE;
    }

    free_string (pObj->short_descr);
    pObj->short_descr = str_dup (argument);
    pObj->short_descr[0] = LOWER (pObj->short_descr[0]);

    SEND ("Short description set.\r\n", ch);
    return TRUE;
}



OEDIT (oedit_long)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  long [string]\r\n", ch);
        return FALSE;
    }

    free_string (pObj->description);
    pObj->description = str_dup (argument);
    pObj->description[0] = UPPER (pObj->description[0]);

    SEND ("Long description set.\r\n", ch);
    return TRUE;
}



bool set_value (CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, char *argument,
                int value)
{
    if (argument[0] == '\0')
    {
        set_obj_values (ch, pObj, -1, "");    /* '\0' changed to "" -- Hugin */
        return FALSE;
    }

    if (set_obj_values (ch, pObj, value, argument))
        return TRUE;

    return FALSE;
}



/*****************************************************************************
 Name:        oedit_values
 Purpose:    Finds the object and sets its value.
 Called by:    The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values (CHAR_DATA * ch, char *argument, int value)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (set_value (ch, pObj, argument, value))
        return TRUE;

    return FALSE;
}


OEDIT (oedit_value0)
{
    if (oedit_values (ch, argument, 0))
        return TRUE;

    return FALSE;
}



OEDIT (oedit_value1)
{
    if (oedit_values (ch, argument, 1))
        return TRUE;

    return FALSE;
}



OEDIT (oedit_value2)
{
    if (oedit_values (ch, argument, 2))
        return TRUE;

    return FALSE;
}



OEDIT (oedit_value3)
{
    if (oedit_values (ch, argument, 3))
        return TRUE;

    return FALSE;
}



OEDIT (oedit_value4)
{
    if (oedit_values (ch, argument, 4))
        return TRUE;

    return FALSE;
}



OEDIT (oedit_weight)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  weight [number]\r\n", ch);
        return FALSE;
    }

    pObj->weight = atoi (argument);

    SEND ("Weight set.\r\n", ch);
    return TRUE;
}

OEDIT (oedit_cost)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  cost [number]\r\n", ch);
        return FALSE;
    }

    pObj->cost = atoi (argument);

    SEND ("Cost set.\r\n", ch);
    return TRUE;
}



OEDIT (oedit_create)
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int value;
    int iHash;
	char buf[MSL];

    value = atoi (argument);
    if (argument[0] == '\0' || value == 0)
    {
        SEND ("Syntax:  oedit create [vnum]\r\n", ch);
        return FALSE;
    }

    pArea = get_vnum_area (value);
    if (!pArea)
    {
        SEND ("OEdit:  That vnum is not assigned an area.\r\n", ch);
        return FALSE;
    }

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("OEdit:  Vnum is in an area you cannot build in.\r\n", ch);
        return FALSE;
    }

    if (get_obj_index (value))
    {
        SEND ("OEdit:  Object vnum already exists.\r\n", ch);
        return FALSE;
    }

    pObj = new_obj_index ();
    pObj->vnum = value;
    pObj->area = pArea;

    if (value > top_vnum_obj)
        top_vnum_obj = value;

    iHash = value % MAX_KEY_HASH;
    pObj->next = obj_index_hash[iHash];
    obj_index_hash[iHash] = pObj;
    ch->desc->pEdit = (void *) pObj;

	
	sprintf(buf, "{r[{x Object Vnum {y%ld{x Created. {r]{x\r\n", pObj->vnum);	
    SEND (buf, ch);
	SEND ("Entering Object Editor.\r\n",ch);
    return TRUE;
}



OEDIT (oedit_ed)
{
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ (ch, pObj);

    argument = one_argument (argument, command);
    one_argument (argument, keyword);

    if (command[0] == '\0')
    {
        SEND ("Syntax:  ed add [keyword]\r\n", ch);
        SEND ("         ed delete [keyword]\r\n", ch);
        SEND ("         ed edit [keyword]\r\n", ch);
        SEND ("         ed format [keyword]\r\n", ch);
        return FALSE;
    }

    if (!str_cmp (command, "add"))
    {
        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed add [keyword]\r\n", ch);
            return FALSE;
        }

        ed = new_extra_descr ();
        ed->keyword = str_dup (keyword);
        ed->next = pObj->extra_descr;
        pObj->extra_descr = ed;

        string_append (ch, &ed->description);

        return TRUE;
    }

    if (!str_cmp (command, "edit"))
    {
        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed edit [keyword]\r\n", ch);
            return FALSE;
        }

        for (ed = pObj->extra_descr; ed; ed = ed->next)
        {
            if (is_name (keyword, ed->keyword))
                break;
        }

        if (!ed)
        {
            SEND ("OEdit:  Extra description keyword not found.\r\n",
                          ch);
            return FALSE;
        }

        string_append (ch, &ed->description);

        return TRUE;
    }

    if (!str_cmp (command, "delete"))
    {
        EXTRA_DESCR_DATA *ped = NULL;

        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed delete [keyword]\r\n", ch);
            return FALSE;
        }

        for (ed = pObj->extra_descr; ed; ed = ed->next)
        {
            if (is_name (keyword, ed->keyword))
                break;
            ped = ed;
        }

        if (!ed)
        {
            SEND ("OEdit:  Extra description keyword not found.\r\n",
                          ch);
            return FALSE;
        }

        if (!ped)
            pObj->extra_descr = ed->next;
        else
            ped->next = ed->next;

        free_extra_descr (ed);

        SEND ("Extra description deleted.\r\n", ch);
        return TRUE;
    }


    if (!str_cmp (command, "format"))
    {
        EXTRA_DESCR_DATA *ped = NULL;

        if (keyword[0] == '\0')
        {
            SEND ("Syntax:  ed format [keyword]\r\n", ch);
            return FALSE;
        }

        for (ed = pObj->extra_descr; ed; ed = ed->next)
        {
            if (is_name (keyword, ed->keyword))
                break;
            ped = ed;
        }

        if (!ed)
        {
            SEND ("OEdit:  Extra description keyword not found.\r\n",
                          ch);
            return FALSE;
        }

        ed->description = format_string (ed->description);

        SEND ("Extra description formatted.\r\n", ch);
        return TRUE;
    }

    oedit_ed (ch, "");
    return FALSE;
}





/* ROM object functions : */

OEDIT (oedit_extra)
{                                /* Moved out of oedit() due to naming conflicts -- Hugin */
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_OBJ (ch, pObj);

        if ((value = flag_value (extra_flags, argument)) != NO_FLAG)
        {
            TOGGLE_BIT (pObj->extra_flags, value);

            SEND ("Extra flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax:  extra [flag]\r\n"
                  "Type '? extra' for a list of flags.\r\n", ch);
    return FALSE;
}

OEDIT (oedit_extra2)
{                                /* Moved out of oedit() due to naming conflicts -- Hugin */
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_OBJ (ch, pObj);

        if ((value = flag_value (extra2_flags, argument)) != NO_FLAG)
        {
            TOGGLE_BIT (pObj->extra2_flags, value);

            SEND ("Extra2 flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax:  extra2 [flag]\r\n"
                  "Type '? extra2' for a list of flags.\r\n", ch);
    return FALSE;
}

OEDIT (oedit_wear)
{                                /* Moved out of oedit() due to naming conflicts -- Hugin */
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_OBJ (ch, pObj);

        if ((value = flag_value (wear_flags, argument)) != NO_FLAG)
        {
            TOGGLE_BIT (pObj->wear_flags, value);

            SEND ("Wear flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax:  wear [flag]\r\n"
                  "Type '? wear' for a list of flags.\r\n", ch);
    return FALSE;
}


OEDIT (oedit_type)
{                                /* Moved out of oedit() due to naming conflicts -- Hugin */
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_OBJ (ch, pObj);

        if ((value = flag_value (type_flags, argument)) != NO_FLAG)
        {
            pObj->item_type = value;
			if (pObj->item_type == ITEM_WEAPON)
			{
				SET_BIT(pObj->wear_flags,ITEM_WIELD);
				SET_BIT(pObj->wear_flags,ITEM_HOLD);
			}	
            SEND ("Type set.\r\n", ch);

            /*
             * Clear the values.
             */
            pObj->value[0] = 0;
            pObj->value[1] = 0;
            pObj->value[2] = 0;
            pObj->value[3] = 0;
            pObj->value[4] = 0;    /* ROM */

            return TRUE;
        }
    }

    SEND ("Syntax:  type [flag]\r\n"
                  "Type '? type' for a list of flags.\r\n", ch);
    return FALSE;
}

bool valid_herb (char *argument)
{
	int i = 0;
	
	for (i = 0; i < MAX_HERB; i++)
	{
		if (!str_cmp(herb_table[i].name, argument))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

int get_herb(char *argument)
{
	int i = 0;

	for (i = 0; i < MAX_HERB; i++)
	{
		if (!str_cmp(herb_table[i].name, argument))
		{
			return i;
		}
	}
	return -1;
}

bool valid_material(char *argument)
{
	int i = 0;
	
	for (i = 0; i <= MAX_MATERIAL; i++)
	{
		if (!IS_NULLSTR(mat_table[i].material))
		{
			if (!str_cmp(mat_table[i].material, argument))
				return TRUE;
		}
	}
	return FALSE;
}

int get_material(char *argument)
{
	int i = 0;

	for (i = 0; i < MAX_MATERIAL; i++)
	{
		if (!IS_NULLSTR(mat_table[i].material))
		{
			if (!str_cmp(mat_table[i].material, argument))
				return i;
		}
	}
	return -1;
}

OEDIT (oedit_material)
{
    OBJ_INDEX_DATA *pObj;
	char buf[MSL];
	int i = 0;
	

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  material [string]\r\n", ch);
        return FALSE;
    }
	
	if (argument[0] == '?')
	{
		SEND("Available Materials:\r\n",ch);
		for (i = 0; i < MAX_MATERIAL; i++)
		{
			sprintf(buf, "%s\r\n", mat_table[i].material);
			SEND(buf,ch);
		}
		return FALSE;
	}
	
	if (valid_material(argument))
	{
		if (pObj->item_type == ITEM_WEAPON && get_weapon_type(pObj) != gsn_whip)
		{
			if (mat_table[get_material(argument)].is_cloth == TRUE)
			{
				sprintf(buf, "You cannot make weapons from %s. (Unless it's a whip.)\r\n", argument);
				SEND(buf,ch);
				return FALSE;
			}
		}	
		if ( pObj->item_type == ITEM_THIEVES_TOOLS )
		{
			if ( mat_table[get_material(argument)].is_metal == FALSE )
			{
				SEND ( "This item must be made of metal.\r\n", ch );
				return FALSE;
			}
		}
		free_string (pObj->material);		
		pObj->material = str_dup (argument);		
		SEND ("Material set.\r\n", ch);
		if (pObj->item_type == ITEM_ARMOR)
			pObj->value[4] = mat_table[get_material(pObj->material)].bulk;		
		return TRUE;
	}
	else
	{
		SEND("Invalid material type.\r\n",ch);
		return FALSE;
	}
	return FALSE;
}

OEDIT (oedit_level)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  level [number]\r\n", ch);
        return FALSE;
    }

    pObj->level = atoi (argument);

    SEND ("Level set.\r\n", ch);
	if (pObj->item_type == ITEM_WEAPON)
	{
		if (pObj->level <= 5)
		{
			pObj->value[1] = 1;		
			pObj->value[2] = number_range(4,6);
		}
		else if (pObj->level > 5 && pObj->level <= 10)
		{
			pObj->value[1] = number_range(1,2);
			pObj->value[2] = number_range(4,6);
		}
		else if (pObj->level > 10 && pObj->level <= 20)
		{
			pObj->value[1] = 2;
			pObj->value[2] = number_range(5,9);
		}
		else if (pObj->level > 20 && pObj->level <= 30)
		{
			pObj->value[1] = number_range(2,3);
			pObj->value[2] = number_range(6,9);
		}
		else
		{
			pObj->value[1] = number_range(3,4);
			pObj->value[2] = number_range(6,9);
		}
		SEND ("Default damage dice values set for weapon.\r\n",ch);
	}
	if (pObj->item_type == ITEM_ARMOR)
	{	//Set default ac values:
		pObj->value[0] = number_range((pObj->level * 0.13),(pObj->level * 0.23));
		pObj->value[1] = number_range((pObj->level * 0.13),(pObj->level * 0.23));
		pObj->value[2] = number_range((pObj->level * 0.13),(pObj->level * 0.23));
		pObj->value[3] = number_range((pObj->level * 0.03),(pObj->level * 0.13));
		
		switch (pObj->value[4])
		{
			default:
				break;
			case 0:
			case 1:
				break;
			case 2:
				pObj->value[0] += 1;
				pObj->value[1] += 1;
				pObj->value[2] += 1;
				pObj->value[3] += 1;
				break;
			case 3:
				pObj->value[0] += 2;
				pObj->value[1] += 2;
				pObj->value[2] += 2;
				pObj->value[3] += 2;
				break;
			case 4:
				pObj->value[0] += 3;
				pObj->value[1] += 3;
				pObj->value[2] += 3;
				pObj->value[3] += 3;
				break;
		}
		SEND ("Default AC values set for armor.\r\n",ch);
	}
    return TRUE;
}

OEDIT (oedit_timer)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  timer [number]\r\n", ch);
        return FALSE;
    }

    pObj->timer = atoi (argument);

    SEND ("Timer set.\r\n", ch);
    return TRUE;
}



OEDIT (oedit_condition)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0'
        && (value = atoi (argument)) >= 0 && (value <= 100))
    {
        EDIT_OBJ (ch, pObj);

        pObj->condition = value;
        SEND ("Condition set.\r\n", ch);

        return TRUE;
    }

    SEND ("Syntax:  condition [number]\r\n"
                  "Where number can range from 0 (ruined) to 100 (perfect).\r\n",
                  ch);
    return FALSE;
}

OEDIT (oedit_quality)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0'
        && (value = atoi (argument)) >= 0 && (value <= 6))
    {
        EDIT_OBJ (ch, pObj);

        pObj->quality = value;        
		SEND ("Quality set.\r\n", ch);		

        return TRUE;
    }

    SEND ("Syntax:  quality [number]\r\n"
                  "Where number can range from 0 (Poor) to 6 (Legendary).\r\n",
                  ch);
    return FALSE;
}

OEDIT (oedit_copy)
{
	OBJ_INDEX_DATA *pObjOrig;
	OBJ_INDEX_DATA *pObjCopy;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int vnum, i;
	char buf[MAX_STRING_LENGTH];
 
	if (argument[0] == '\0')
	{
		sprintf (buf, "{rOEDIT: {yCOPY: {xYou must specify a vnum to copy.\r\n"
		              "       Syntax: COPY <vnum> <argument>\r\n"
		              "       {yValid Arguments:{x\r\n"
		              "          level, name, long, short, extended, type, wear,\r\n"
		              "          material, extra, affects, values\r\n"
		              "          condition, weight, cost, timer. Use \'all\' to clone.\r\n");
		SEND (buf, ch);
		return FALSE;
	}
	bool cAll;
 
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
 
	if (!str_cmp (arg2, "all" ) || arg2[0] == '\0')
		cAll = TRUE;
	else
		cAll = FALSE;
 
 
	if (!is_number(arg1))
	{
		sprintf (buf, "{rOEDIT: {yCOPY: {xYou must specify a vnum to copy.\r\n"
					  "       Syntax: COPY <vnum>\r\n"
					  "               COPY <vnum> <argument>\r\n"
					  "       %s is not a number.\r\n", arg1);
		SEND (buf, ch);
		return FALSE;
	}
	else
	{
		vnum = atoi(arg1);
		if ((pObjOrig = get_obj_index(vnum)) == NULL)
		{
			sprintf (buf, "{rOEDIT: {yCOPY: {xYou must specify an EXISTING object to copy.\r\n"
					  "       %d is not an existing object.\r\n", vnum);
			SEND (buf, ch);
			return FALSE;
		}
 
 
    EDIT_OBJ (ch, pObjCopy);
 
	if (cAll || !str_prefix (arg2, "level"))
	{
      pObjCopy->level = pObjOrig->level;
      sprintf (buf, "{rOEDIT: {yCOPY:{x level copied:            {y%d{x\r\n",
        pObjCopy->level);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "name"))
	{
      free_string( pObjCopy->name );
      pObjCopy->name = str_dup( pObjOrig->name );
      sprintf (buf, "{rOEDIT: {yCOPY:{x name copied:             {y%s{x\r\n",
        pObjCopy->name);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "long") || !str_prefix (arg2, "description"))
	{
      free_string( pObjCopy->description );
      pObjCopy->description = str_dup( pObjOrig->description );
      sprintf (buf, "{rOEDIT: {yCOPY:{x description copied:      {y%s{x\r\n",
        pObjCopy->description);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "short"))
	{
      free_string( pObjCopy->short_descr );
      pObjCopy->short_descr = str_dup( pObjOrig->short_descr );
      sprintf (buf, "{rOEDIT: {yCOPY:{x Short desc copied:       {y%s{x\r\n",
        pObjCopy->short_descr);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "extended") || !str_prefix (arg2, "ed"))
	{
      EXTRA_DESCR_DATA *ed;
      EXTRA_DESCR_DATA *edCLONE;
 
      for (ed = pObjOrig->extra_descr; ed != NULL; ed = ed->next)
	  {
	    edCLONE                  =   new_extra_descr();
	    edCLONE->keyword         =   str_dup( ed->keyword );
	    edCLONE->description     =   str_dup( ed->description);
	    edCLONE->next            =   pObjCopy->extra_descr;
	    pObjCopy->extra_descr    =   edCLONE;
	  }
 
      sprintf (buf, "{rOEDIT: {yCOPY:{x extra desc copied:      {y");
      SEND(buf,ch);
	  for ( ed = pObjCopy->extra_descr; ed; ed = ed->next )
	  {
	    SEND( ed->keyword, ch );
	    SEND( " ", ch );
	  }
	  SEND( "{x\r\n", ch );      
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "type"))
	{
      pObjCopy->item_type = pObjOrig->item_type;
      sprintf (buf, "{rOEDIT: {yCOPY:{x item type copied:        {y%s{x\r\n",
        flag_string( type_flags, pObjCopy->item_type));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "wear"))
	{
      pObjCopy->wear_flags = pObjOrig->wear_flags;
      sprintf (buf, "{rOEDIT: {yCOPY:{x wear flags copied:       {y%s{x\r\n",
        flag_string ( wear_flags, pObjCopy->wear_flags));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "material"))
	{
      pObjCopy->material = pObjOrig->material;	        
      SEND("{rOEDIT: {yCOPY:{x material copied.        \r\n",ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "extra"))
	{
      pObjCopy->extra_flags = pObjOrig->extra_flags;
      sprintf (buf, "{rOEDIT: {yCOPY:{x extra flags copied:      {y%s{x\r\n",
        flag_string ( extra_flags, pObjCopy->extra_flags));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "condition"))
	{
      pObjCopy->condition = pObjOrig->condition;
      sprintf (buf, "{rOEDIT: {yCOPY:{x condition copied:        {y%d{x\r\n",
        pObjCopy->condition);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "weight"))
	{
      pObjCopy->weight = pObjOrig->weight;
      sprintf (buf, "{rOEDIT: {yCOPY:{x weight copied:           {y%d{x (10th lbs)\r\n",
        pObjCopy->weight);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "cost"))
	{
      pObjCopy->cost = pObjOrig->cost;
      sprintf (buf, "{rOEDIT: {yCOPY:{x cost copied:             {y%d{x\r\n",
        pObjCopy->cost);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}/*
	if (cAll || !str_prefix (arg2, "timer"))
	{
      pObjCopy->timer = pObjOrig->timer;
      sprintf (buf, "{rOEDIT: {yCOPY:{x timer copied:            {y%d{x\r\n",
        pObjCopy->timer);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}*/
	/*
	if (cAll || !str_prefix (arg2, "affects"))
	{
	  AFFECT_DATA *paf;
	  int cnt;
 
	// it's just an int, zap it over then show what we did
      pObjCopy->affected = pObjOrig->affected;
      SEND ("{rOEDIT: {yCOPY:{x affects copied: \r\n", ch);
    for ( cnt = 0, paf = pObjCopy->affected; paf; paf = paf->next )
    {
	  if ( cnt == 0 )
	  {
	      SEND( "Number Modifier Affects\r\n", ch );
	      SEND( "------ -------- -------\r\n", ch );
	  }
	  if (paf->where != TO_RESISTANCE_AFFECTS && paf->where != TO_RESISTANCE_AFFECTS2)
	  {
	      sprintf( buf, "[%4d] %-8d %s\r\n", cnt,
	        paf->modifier,
	        flag_string( apply_flags, paf->location ) );
	      SEND( buf, ch );
	  }
	  else
	  {
	      sprintf (buf, "[%4d] %-8d Resistance %s\r\n", cnt, paf->modifier,
	        res_table[paf->location].cname);
	      SEND (buf, ch);
	  }
	  cnt++;
    }
      if (!cAll)
      	return TRUE;
	}*/
	if (cAll || !str_prefix (arg2, "values"))
	{
	  for (i=0;i<5;i++)
      pObjCopy->value[i] = pObjOrig->value[i];
      SEND ("{rOEDIT: {yCOPY:{x values copied:{x\r\n",ch);
      show_obj_values (ch, pObjCopy);
      if (!cAll)
      	return TRUE;
	}
 
	if (cAll)
    {
      sprintf(buf,  "\r\n{rOEDIT: {yCOPY: Object %d duplicated.{x\r\n", vnum );
	  SEND (buf, ch);
      return TRUE;
    }
 
      sprintf(buf, 
		"{rOEDIT: {yCOPY:{x Syntax Error. \"{r%s{x\"\r\n"
		"       {yValid Arguments:{x\r\n"
		"          level, name, long, short, extended, type, wear,\r\n"
		"          material, extra, values\r\n"
		"          condition, weight, cost, timer. Use \'all\' to clone.\r\n", arg2);
      SEND (buf, ch);
      return FALSE;
	}
}



/*
 * Mobile Editor Functions.
 */
 
MEDIT (medit_entrance)
{ 
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  walk/entrance [string]\r\n", ch);
        return FALSE;
    }

    free_string (pMob->walk_desc);
	if (strlen(argument) < 7)
	{
		SEND ("Must be at least 7 characters in length.\r\n",ch);
		return FALSE;
	}
    pMob->walk_desc = str_dup (argument);

    SEND ("Walk description set.\r\n", ch);
    return TRUE;
}
 
MEDIT (medit_show)
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    PROG_LIST *list;

    EDIT_MOB (ch, pMob);

    sprintf (buf, "Name:        {g[{x%s{g]{x\r\nArea:        {g[{x%5d{g]{x %s\r\n",
             pMob->player_name,
             !pMob->area ? -1 : pMob->area->vnum,
             !pMob->area ? "No Area" : pMob->area->name);
    SEND (buf, ch);

    sprintf (buf, "Act:         {g[{x%s{g]{x\r\n",
             flag_string (act_flags, pMob->act));
	SEND (buf, ch);
	sprintf (buf, "Act2:        {g[{x%s{g]{x\r\n",
             flag_string (act2_flags, pMob->act2));
    SEND (buf, ch);

    sprintf (buf, "Vnum:        {g[{x%5ld{g]{x Sex:   {g[{x%s{g]{x   Race: {g[{x%s{g]{x\r\n",
             pMob->vnum,
             pMob->sex == SEX_MALE ? "male   " :
             pMob->sex == SEX_FEMALE ? "female " :
             pMob->sex == 3 ? "random " : "neutral",
             race_table[pMob->race].name);
    SEND (buf, ch);

    sprintf (buf,
             "Level:       {g[{x%2d{g]{x    Align: {g[{x%s{g]{x      Hitroll: {g[{x%2d{g]{x Dam Type:    {g[{x%s{g]{x\r\n",
             pMob->level, GET_ALIGN_STRING(pMob),
             pMob->hitroll, attack_table[pMob->dam_type].name);
    SEND (buf, ch);

    if (pMob->group)
    {
        sprintf (buf, "Group:       {g[{x%5d{g]{x\r\n", pMob->group);
        SEND (buf, ch);
    }

    sprintf (buf, "Hit dice:    {g[{x%2dd%-3d+%4d{g]{x ",
             pMob->hit[DICE_NUMBER],
             pMob->hit[DICE_TYPE], pMob->hit[DICE_BONUS]);
    SEND (buf, ch);

    sprintf (buf, "Damage dice: {g[{x%2dd%-3d+%4d{g]{x ",
             pMob->damage[DICE_NUMBER],
             pMob->damage[DICE_TYPE], pMob->damage[DICE_BONUS]);
    SEND (buf, ch);

    sprintf (buf, "Mana dice:   {g[{x%2dd%-3d+%4d{g]{x\r\n",
             pMob->mana[DICE_NUMBER],
             pMob->mana[DICE_TYPE], pMob->mana[DICE_BONUS]);
    SEND (buf, ch);

/* ROM values end */

    sprintf (buf, "Affected by: {g[{x%s{g]{x\r\n",
             flag_string (affect_flags, pMob->affected_by));
    SEND (buf, ch);

/* ROM values: */

    sprintf (buf,
             "Armor:       {g[{xpierce: %d  bash: %d  slash: %d  magic: %d{g]{x\r\n",
             pMob->ac[AC_PIERCE], pMob->ac[AC_BASH], pMob->ac[AC_SLASH],
             pMob->ac[AC_EXOTIC]);
    SEND (buf, ch);

    sprintf (buf, "Form:        {g[{x%s{g]{x\r\n",
             flag_string (form_flags, pMob->form));
    SEND (buf, ch);

    sprintf (buf, "Parts:       {g[{x%s{g]{x\r\n",
             flag_string (part_flags, pMob->parts));
    SEND (buf, ch);

    sprintf (buf, "Imm:         {g[{x%s{g]{x\r\n",
             flag_string (imm_flags, pMob->imm_flags));
    SEND (buf, ch);

    sprintf (buf, "Res:         {g[{x%s{g]{x\r\n",
             flag_string (res_flags, pMob->res_flags));
    SEND (buf, ch);

    sprintf (buf, "Vuln:        {g[{x%s{g]{x\r\n",
             flag_string (vuln_flags, pMob->vuln_flags));
    SEND (buf, ch);

    sprintf (buf, "Off:         {g[{x%s{g]{x\r\n",
             flag_string (off_flags, pMob->off_flags));
    SEND (buf, ch);

    sprintf (buf, "Size:        {g[{x%s{g]{x\r\n",
             flag_string (size_flags, pMob->size));
    SEND (buf, ch);

    sprintf (buf, "Material:    {g[{x%s{g]{x\r\n", pMob->material);
    SEND (buf, ch);

    sprintf (buf, "Start pos.   {g[{x%s{g]{x\r\n",
             flag_string (position_flags, pMob->start_pos));
    SEND (buf, ch);

    sprintf (buf, "Default pos  {g[{x%s{g]{x\r\n",
             flag_string (position_flags, pMob->default_pos));
    SEND (buf, ch);

    sprintf (buf, "Wealth:      {g[{x%5ld{g]{x\r\n", pMob->wealth);
    SEND (buf, ch);

/* ROM values end */

    if (pMob->spec_fun)
    {
        sprintf (buf, "Spec fun:    {g[{x%s{g]{x\r\n", spec_name (pMob->spec_fun));
        SEND (buf, ch);
    }

    sprintf (buf, "Short descr: %s\r\nLong descr:\r\n%s",
             pMob->short_descr, pMob->long_descr);
    SEND (buf, ch);

	sprintf (buf, "Walk Description: %s\r\n",
             pMob->walk_desc);
    SEND (buf, ch);
	
    sprintf (buf, "Description:\r\n%s", pMob->description);
    SEND (buf, ch);

	
    if (pMob->pShop)
    {
        SHOP_DATA *pShop;
        int iTrade;

        pShop = pMob->pShop;

        sprintf (buf,
                 "Shop data for {g[{x%5ld{g]{x:\r\n"
                 "  Markup for purchaser: %d%%\r\n"
                 "  Markdown for seller:  %d%%\r\n",
                 pShop->keeper, pShop->profit_buy, pShop->profit_sell);
        SEND (buf, ch);
        sprintf (buf, "  Hours: %d to %d.\r\n",
                 pShop->open_hour, pShop->close_hour);
        SEND (buf, ch);
		sprintf (buf, "  Random Gear: %s.\r\n",
				 (pShop->random_gear ? "yes" : "no"));
		SEND (buf, ch);

        for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
        {
            if (pShop->buy_type[iTrade] != 0)
            {
                if (iTrade == 0)
                {
                    SEND ("  Number Trades Type\r\n", ch);
                    SEND ("  ------ -----------\r\n", ch);
                }
                sprintf (buf, "  {g[{x%4d{g]{x %s\r\n", iTrade,
                         flag_string (type_flags, pShop->buy_type[iTrade]));
                SEND (buf, ch);
            }
        }
    }

    if (pMob->mprogs)
    {
        int cnt;

        sprintf (buf, "\r\nMOBPrograms for {g[{x%6ld{g]{x:\r\n", pMob->vnum);
        SEND (buf, ch);

        for (cnt = 0, list = pMob->mprogs; list; list = list->next)
        {
            if (cnt == 0)
            {
                SEND (" Number Vnum  Trigger Phrase\r\n", ch);
                SEND (" ------ ----- ------- ------\r\n", ch);
            }

            sprintf (buf, "{g[{x%5d{g]{x %5ld %7s %s\r\n", cnt,
                     list->vnum, prog_type_to_name(list->trig_type),
                     list->trig_phrase);
            SEND (buf, ch);
            cnt++;
        }
    }

    return FALSE;
}



MEDIT (medit_create)
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
	char buf[MSL];
    int value;
    int iHash;

    value = atoi (argument);
    if (argument[0] == '\0' || value == 0)
    {
        SEND ("Syntax:  medit create [vnum]\r\n", ch);
        return FALSE;
    }

    pArea = get_vnum_area (value);

    if (!pArea)
    {
        SEND ("MEdit:  That vnum is not assigned an area.\r\n", ch);
        return FALSE;
    }

    if (!IS_BUILDER (ch, pArea))
    {
        SEND ("MEdit:  Vnum is in an area you cannot build in.\r\n", ch);
        return FALSE;
    }

    if (get_mob_index (value))
    {
        SEND ("MEdit:  Mobile vnum already exists.\r\n", ch);
        return FALSE;
    }

    pMob = new_mob_index ();
    pMob->vnum = value;
    pMob->area = pArea;

    if (value > top_vnum_mob)
        top_vnum_mob = value;

	pMob->race = RACE_HUMAN;	
    pMob->act |= race_table[pMob->race].act;
	pMob->affected_by |= race_table[pMob->race].aff;
	pMob->off_flags |= race_table[pMob->race].off;
	pMob->imm_flags |= race_table[pMob->race].imm;
	pMob->res_flags |= race_table[pMob->race].res;
	pMob->vuln_flags |= race_table[pMob->race].vuln;
	sprintf(buf, "a %s", race_table[ch->race].name);
	pMob->walk_desc = str_dup("");
	//pMob->form |= race_table[pMob->race].form;
	//pMob->parts |= race_table[pMob->race].parts;
		
    pMob->act = ACT_IS_NPC;
	pMob->act += ACT_STAY_AREA;
    iHash = value % MAX_KEY_HASH;
    pMob->next = mob_index_hash[iHash];
    mob_index_hash[iHash] = pMob;
    ch->desc->pEdit = (void *) pMob;
	
	sprintf(buf, "{r[{x Mobile Vnum {y%ld{x Created. {r]{x\r\n", pMob->vnum);
	SEND (buf, ch);
	SEND ("Entering Mobile Editor.\r\r\n\n",ch);    
	medit_show (ch, argument);
    return TRUE;
}



MEDIT (medit_spec)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  spec [special function]\r\n", ch);
        return FALSE;
    }


    if (!str_cmp (argument, "none"))
    {
        pMob->spec_fun = NULL;

        SEND ("Spec removed.\r\n", ch);
        return TRUE;
    }

    if (spec_lookup (argument))
    {
        pMob->spec_fun = spec_lookup (argument);
        SEND ("Spec set.\r\n", ch);
        return TRUE;
    }

    SEND ("MEdit: No such special function.\r\n", ch);
    return FALSE;
}

MEDIT (medit_damtype)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  damtype [damage message]\r\n", ch);
        SEND
            ("For a list of damage types, type '? weapon'.\r\n",
             ch);
        return FALSE;
    }

    pMob->dam_type = attack_lookup (argument);
    SEND ("Damage type set.\r\n", ch);
    return TRUE;
}


MEDIT (medit_align)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);
	
    //if (argument[0] == '\0' || !is_number (argument))
    // {
        // SEND ("Syntax:  alignment [LG,CG,NG,LN,TN,CN,NE,CE,LE]\r\n", ch);
        // return FALSE;
    // }
	
	if (!str_cmp(argument, "lawful good") || !str_cmp(argument, "lg"))
		pMob->alignment = ALIGN_LG;	
	else if (!str_cmp(argument, "chaotic good") || !str_cmp(argument, "cg"))
		pMob->alignment = ALIGN_CG;	
	else if (!str_cmp(argument, "neutral good") || !str_cmp(argument, "ng"))
		pMob->alignment = ALIGN_NG;	
	else if (!str_cmp(argument, "lawful neutral") || !str_cmp(argument, "ln"))
		pMob->alignment = ALIGN_LN;	
	else if (!str_cmp(argument, "true neutral") || !str_cmp(argument, "tn"))
		pMob->alignment = ALIGN_TN;	
	else if (!str_cmp(argument, "chaotic neutral") || !str_cmp(argument, "cn"))
		pMob->alignment = ALIGN_CN;
	else if (!str_cmp(argument, "lawful evil") || !str_cmp(argument, "le"))
		pMob->alignment = ALIGN_LE;
	else if (!str_cmp(argument, "chaotic evil") || !str_cmp(argument, "ce"))
		pMob->alignment = ALIGN_CE;		
	else if (!str_cmp(argument, "neutral evil") || !str_cmp(argument, "ne"))
		pMob->alignment = ALIGN_NE;		
	else
	{
		SEND ("Syntax:  alignment [LG,CG,NG,LN,TN,CN,NE,CE,LE]\r\n", ch);
        return FALSE;
	}
	// if (atoi (argument) < -1000 || atoi (argument) > 1000)
	// {
		// SEND("Invalid alignment setting. (-1000 to 1000 only, thanks.)\r\n",ch);
		// return FALSE;
	// }
    //pMob->alignment = atoi (argument);
    SEND ("Alignment set.\r\n", ch);
    return TRUE;
}



MEDIT (medit_level)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  level [number]\r\n", ch);
        return FALSE;
    }

    pMob->level = atoi (argument);

    SEND ("Level set.\r\n", ch);
	if (!pMob->hit[DICE_NUMBER])
	{
		pMob->hit[DICE_NUMBER] = atoi (argument);
		pMob->hit[DICE_TYPE] = 8;
		pMob->hit[DICE_BONUS] = 2 * atoi(argument);
		SEND ("Auto Hitdice set.\r\n", ch);
	}    
	if (!pMob->mana[DICE_NUMBER])
	{
		pMob->mana[DICE_NUMBER] = atoi (argument);
		pMob->mana[DICE_TYPE] = 8;
		pMob->mana[DICE_BONUS] = 100;
		SEND ("Auto Manadice set.\r\n", ch);
	}
    return TRUE;
}



MEDIT (medit_desc)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        string_append (ch, &pMob->description);
        return TRUE;
    }

    SEND ("Syntax:  desc    - line edit\r\n", ch);
    return FALSE;
}




MEDIT (medit_long)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  long [string]\r\n", ch);
        return FALSE;
    }

    free_string (pMob->long_descr);
    strcat (argument, "\r\n");
    pMob->long_descr = str_dup (argument);
    pMob->long_descr[0] = UPPER (pMob->long_descr[0]);

    SEND ("Long description set.\r\n", ch);
    return TRUE;
}



MEDIT (medit_short)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  short [string]\r\n", ch);
        return FALSE;
    }

    free_string (pMob->short_descr);
    pMob->short_descr = str_dup (argument);

    SEND ("Short description set.\r\n", ch);
    return TRUE;
}



MEDIT (medit_name)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  name [string]\r\n", ch);
        return FALSE;
    }

    free_string (pMob->player_name);
    pMob->player_name = str_dup (argument);

    SEND ("Name set.\r\n", ch);
    return TRUE;
}

MEDIT (medit_shop)
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    argument = one_argument (argument, arg1);

    EDIT_MOB (ch, pMob);

    if (command[0] == '\0')
    {
        SEND ("Syntax:  shop hours [#xopening] [#xclosing] (military hours)\r\n", ch);
        SEND ("         shop profit [#xbuying%] [#xselling%]\r\n",
                      ch);
        SEND ("         shop type [#x0-4] [item type]\r\n", ch);
        SEND ("         shop assign\r\n", ch);
        SEND ("         shop remove\r\n", ch);
		SEND ("			shop random\r\n", ch);		
        return FALSE;
    }


	if (!str_cmp (command, "random"))
	{
		if (!pMob->pShop)
        {
            SEND
                ("MEdit:  You must create the shop first (shop assign).\r\n",
                 ch);
            return FALSE;
        }
		
		if (pMob->pShop->random_gear == FALSE)
			pMob->pShop->random_gear = TRUE;
		else
			pMob->pShop->random_gear = FALSE;
			
		SEND ("Shop randomness set.\r\n", ch);
        return TRUE;
	}
	
    if (!str_cmp (command, "hours"))
    {
        if (arg1[0] == '\0' || !is_number (arg1)
            || argument[0] == '\0' || !is_number (argument))
        {
            SEND ("Syntax:  shop hours [#xopening] [#xclosing]\r\n",
                          ch);
            return FALSE;
        }

        if (!pMob->pShop)
        {
            SEND
                ("MEdit:  You must create the shop first (shop assign).\r\n",
                 ch);
            return FALSE;
        }

        pMob->pShop->open_hour = atoi (arg1);
        pMob->pShop->close_hour = atoi (argument);

        SEND ("Shop hours set.\r\n", ch);
        return TRUE;
    }


    if (!str_cmp (command, "profit"))
    {
        if (arg1[0] == '\0' || !is_number (arg1)
            || argument[0] == '\0' || !is_number (argument))
        {
            SEND ("Syntax:  shop profit [#xbuying%] [#xselling%]\r\n",
                          ch);
            return FALSE;
        }

        if (!pMob->pShop)
        {
            SEND
                ("MEdit:  You must create the shop first (shop assign).\r\n",
                 ch);
            return FALSE;
        }

        pMob->pShop->profit_buy = atoi (arg1);
        pMob->pShop->profit_sell = atoi (argument);

        SEND ("Shop profit set.\r\n", ch);
        return TRUE;
    }


    if (!str_cmp (command, "type"))
    {
        char buf[MAX_INPUT_LENGTH];
        int value;

        if (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0')
        {
            SEND ("Syntax:  shop type [#x0-4] [item type]\r\n", ch);
            return FALSE;
        }

        if (atoi (arg1) >= MAX_TRADE)
        {
            sprintf (buf, "MEdit:  May sell %d items max.\r\n", MAX_TRADE);
            SEND (buf, ch);
            return FALSE;
        }

        if (!pMob->pShop)
        {
            SEND
                ("MEdit:  You must create the shop first (shop assign).\r\n",
                 ch);
            return FALSE;
        }

        if ((value = flag_value (type_flags, argument)) == NO_FLAG)
        {
            SEND ("MEdit:  That type of item is not known.\r\n", ch);
            return FALSE;
        }		
		
        pMob->pShop->buy_type[atoi (arg1)] = value;

        SEND ("Shop type set.\r\n", ch);
        return TRUE;
    }

    /* shop assign && shop delete by Phoenix */

    if (!str_prefix (command, "assign"))
    {
        if (pMob->pShop)
        {
            SEND ("Mob already has a shop assigned to it.\r\n", ch);
            return FALSE;
        }

        pMob->pShop = new_shop ();
        if (!shop_first)
            shop_first = pMob->pShop;
        if (shop_last)
            shop_last->next = pMob->pShop;
        shop_last = pMob->pShop;

        pMob->pShop->keeper = pMob->vnum;

        SEND ("New shop assigned to mobile.\r\n", ch);
        return TRUE;
    }

    if (!str_prefix (command, "remove"))
    {
        SHOP_DATA *pShop;

        pShop = pMob->pShop;
        pMob->pShop = NULL;

        if (pShop == shop_first)
        {
            if (!pShop->next)
            {
                shop_first = NULL;
                shop_last = NULL;
            }
            else
                shop_first = pShop->next;
        }
        else
        {
            SHOP_DATA *ipShop;

            for (ipShop = shop_first; ipShop; ipShop = ipShop->next)
            {
                if (ipShop->next == pShop)
                {
                    if (!pShop->next)
                    {
                        shop_last = ipShop;
                        shop_last->next = NULL;
                    }
                    else
                        ipShop->next = pShop->next;
                }
            }
        }

        free_shop (pShop);

        SEND ("Mobile is no longer a shopkeeper.\r\n", ch);
        return TRUE;
    }

    medit_shop (ch, "");
    return FALSE;
}


/* ROM medit functions: */


MEDIT (medit_sex)
{                                /* Moved out of medit() due to naming conflicts -- Hugin */
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (sex_flags, argument)) != NO_FLAG)
        {
            pMob->sex = value;

            SEND ("Sex set.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: sex [sex]\r\n"
                  "Type '? sex' for a list of flags.\r\n", ch);
    return FALSE;
}


MEDIT (medit_act)
{                                /* Moved out of medit() due to naming conflicts -- Hugin */
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (act_flags, argument)) != NO_FLAG)
        {
            pMob->act ^= value;
            SET_BIT (pMob->act, ACT_IS_NPC);

            SEND ("Act flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: act [flag]\r\n"
                  "Type '? act' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_act2)
{                                
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (act2_flags, argument)) != NO_FLAG)
        {
            pMob->act2 ^= value;
            SET_BIT (pMob->act, ACT_IS_NPC);

            SEND ("Act flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: act2 [flag]\r\n"
                  "Type '? act2' for a list of flags.\r\n", ch);
    return FALSE;
}


MEDIT (medit_affect)
{                                /* Moved out of medit() due to naming conflicts -- Hugin */
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (affect_flags, argument)) != NO_FLAG)
        {
            pMob->affected_by ^= value;

            SEND ("Affect flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: affect [flag]\r\n"
                  "Type '? affect' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_autoac)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea = NULL;	
	int pierce, bash, slash, exotic;
	int vnum = 0;
	
	EDIT_MOB (ch, pMob);
	
	if (argument[0] == '\0')
    {        	
		pierce = 	150 - (pMob->level * 10);
		bash = 		150 - (pMob->level * 10);
		slash = 	150 - (pMob->level * 10);
		exotic = 	200 - (pMob->level * 10);	
	
		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH] = bash;
		pMob->ac[AC_SLASH] = slash;
		pMob->ac[AC_EXOTIC] = exotic;

		SEND ("Ac autoset.\r\n", ch);
		return TRUE;
	}
	else if (!str_cmp(argument, "all"))
	{
		pArea = ch->in_room->area;
		
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		{
			if ((pMob = get_mob_index (vnum)) != NULL)
			{
				pierce = 	150 - (pMob->level * 10);
				bash = 		150 - (pMob->level * 10);
				slash = 	150 - (pMob->level * 10);
				exotic = 	200 - (pMob->level * 10);	
				
				pMob->ac[AC_PIERCE] = pierce;
				pMob->ac[AC_BASH] = bash;
				pMob->ac[AC_SLASH] = slash;
				pMob->ac[AC_EXOTIC] = exotic;
			}
		}
		
		SEND ("Ac autoset for entire area.\r\n", ch);
		return TRUE;
	}
	else
	{
		SEND("The only valid argument for autoac is 'all'.\r\n",ch);
		return FALSE;
	}
}

MEDIT (medit_ac)
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do
    {                            /* So that I can use break and send the syntax in one place */
        if (argument[0] == '\0')
            break;

        EDIT_MOB (ch, pMob);
        argument = one_argument (argument, arg);

        if (!is_number (arg))
            break;
        pierce = atoi (arg);
        argument = one_argument (argument, arg);

        if (arg[0] != '\0')
        {
            if (!is_number (arg))
                break;
            bash = atoi (arg);
            argument = one_argument (argument, arg);
        }
        else
            bash = pMob->ac[AC_BASH];

        if (arg[0] != '\0')
        {
            if (!is_number (arg))
                break;
            slash = atoi (arg);
            argument = one_argument (argument, arg);
        }
        else
            slash = pMob->ac[AC_SLASH];

        if (arg[0] != '\0')
        {
            if (!is_number (arg))
                break;
            exotic = atoi (arg);
        }
        else
            exotic = pMob->ac[AC_EXOTIC];

        pMob->ac[AC_PIERCE] = pierce;
        pMob->ac[AC_BASH] = bash;
        pMob->ac[AC_SLASH] = slash;
        pMob->ac[AC_EXOTIC] = exotic;

        SEND ("Ac set.\r\n", ch);
        return TRUE;
    }
    while (FALSE);                /* Just do it once.. */

    SEND
        ("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\r\n"
         "help MOB_AC  gives a list of reasonable ac-values.\r\n", ch);
    return FALSE;
}

MEDIT (medit_form)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (form_flags, argument)) != NO_FLAG)
        {
            pMob->form ^= value;
            SEND ("Form toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: form [flags]\r\n"
                  "Type '? form' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_part)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (part_flags, argument)) != NO_FLAG)
        {
            pMob->parts ^= value;
            SEND ("Parts toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: part [flags]\r\n"
                  "Type '? part' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_imm)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (imm_flags, argument)) != NO_FLAG)
        {
            pMob->imm_flags ^= value;
            SEND ("Immunity toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: imm [flags]\r\n"
                  "Type '? imm' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_res)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (res_flags, argument)) != NO_FLAG)
        {
            pMob->res_flags ^= value;
            SEND ("Resistance toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: res [flags]\r\n"
                  "Type '? res' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_vuln)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (vuln_flags, argument)) != NO_FLAG)
        {
            pMob->vuln_flags ^= value;
            SEND ("Vulnerability toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: vuln [flags]\r\n"
                  "Type '? vuln' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_material)
{
    MOB_INDEX_DATA *pMob;
	char buf[MSL];
	int i = 0;
    
	EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  material [string]\r\n", ch);
        return FALSE;
    }
	
	if (argument[0] == '?')
	{
		SEND("Available Materials:\r\n",ch);
		for (i = 0; i < MAX_MATERIAL; i++)
		{
			sprintf(buf, "%s\r\n", mat_table[i].material);
			SEND(buf,ch);
		}
		return FALSE;
	}

	
	if (valid_material(argument))
	{		
		free_string (pMob->material);
		pMob->material = str_dup (argument);
		SEND ("Material set.\r\n", ch);
		return TRUE;
	}
	else
	{
		SEND("Invalid material type.\r\n",ch);
		return FALSE;
	}
	return FALSE;
}

MEDIT (medit_off)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (off_flags, argument)) != NO_FLAG)
        {
            pMob->off_flags ^= value;
            SEND ("Offensive behaviour toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: off [flags]\r\n"
                  "Type '? off' for a list of flags.\r\n", ch);
    return FALSE;
}

MEDIT (medit_size)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_MOB (ch, pMob);

        if ((value = flag_value (size_flags, argument)) != NO_FLAG)
        {
            pMob->size = value;
            SEND ("Size set.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: size [size]\r\n"
                  "Type '? size' for a list of sizes.\r\n", ch);
    return FALSE;
}

MEDIT (medit_hitdice)
{
    static char syntax[] = "Syntax:  hitdice <number> d <type(1-8)> + <bonus>\r\n";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND (syntax, ch);
        return FALSE;
    }

    num = cp = argument;

    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    type = cp;

    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    bonus = cp;

    while (isdigit (*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    if ((!is_number (num) || atoi (num) < 1)
        || !is_number (type) || atoi (type) < 1 || atoi(type) > 8
        || (!is_number (bonus) || atoi (bonus) < 0))
    {
        SEND (syntax, ch);
        return FALSE;
    }

    pMob->hit[DICE_NUMBER] = atoi (num);
    pMob->hit[DICE_TYPE] = atoi (type);
    pMob->hit[DICE_BONUS] = atoi (bonus);

    SEND ("Hitdice set.\r\n", ch);
    return TRUE;
}

MEDIT (medit_manadice)
{
    static char syntax[] =
        "Syntax:  manadice <number> d <type> + <bonus>\r\n";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND (syntax, ch);
        return FALSE;
    }

    num = cp = argument;

    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    type = cp;

    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    bonus = cp;

    while (isdigit (*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    if (!(is_number (num) && is_number (type) && is_number (bonus)))
    {
        SEND (syntax, ch);
        return FALSE;
    }

    if ((!is_number (num) || atoi (num) < 1)
        || (!is_number (type) || atoi (type) < 1)
        || (!is_number (bonus) || atoi (bonus) < 0))
    {
        SEND (syntax, ch);
        return FALSE;
    }

    pMob->mana[DICE_NUMBER] = atoi (num);
    pMob->mana[DICE_TYPE] = atoi (type);
    pMob->mana[DICE_BONUS] = atoi (bonus);

    SEND ("Manadice set.\r\n", ch);
    return TRUE;
}

MEDIT (medit_damdice)
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\r\n";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND (syntax, ch);
        return FALSE;
    }

    num = cp = argument;

    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    type = cp;

    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    bonus = cp;

    while (isdigit (*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    if (!(is_number (num) && is_number (type) && is_number (bonus)))
    {
        SEND (syntax, ch);
        return FALSE;
    }

    if ((!is_number (num) || atoi (num) < 1)
        || (!is_number (type) || atoi (type) < 1)
        || (!is_number (bonus) || atoi (bonus) < 0))
    {
        SEND (syntax, ch);
        return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi (num);
    pMob->damage[DICE_TYPE] = atoi (type);
    pMob->damage[DICE_BONUS] = atoi (bonus);

    SEND ("Damdice set.\r\n", ch);
    return TRUE;
}


MEDIT (medit_race)
{
    MOB_INDEX_DATA *pMob;
    int race;

    if (argument[0] != '\0' && (race = race_lookup (argument)) != 0)
    {
        EDIT_MOB (ch, pMob);

        pMob->race = race;
        pMob->act |= race_table[race].act;
        pMob->affected_by |= race_table[race].aff;
        pMob->off_flags |= race_table[race].off;
        pMob->imm_flags |= race_table[race].imm;
        pMob->res_flags |= race_table[race].res;
        pMob->vuln_flags |= race_table[race].vuln;
        pMob->form |= race_table[race].form;
        pMob->parts |= race_table[race].parts;

        SEND ("Race set.\r\n", ch);
        return TRUE;
    }

    if (argument[0] == '?')
    {
        char buf[MAX_STRING_LENGTH];

        SEND ("Available races are:", ch);

        for (race = 0; race_table[race].name != NULL; race++)
        {
            if ((race % 3) == 0)
                SEND ("\r\n", ch);
            sprintf (buf, " %-15s", race_table[race].name);
            SEND (buf, ch);
        }

        SEND ("\r\n", ch);
        return FALSE;
    }

    SEND ("Syntax:  race [race]\r\n"
                  "Type 'race ?' for a list of races.\r\n", ch);
    return FALSE;
}


MEDIT (medit_position)
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument (argument, arg);

    switch (arg[0])
    {
        default:
            break;

        case 'S':
        case 's':
            if (str_prefix (arg, "start"))
                break;

            if ((value = flag_value (position_flags, argument)) == NO_FLAG)
                break;

            EDIT_MOB (ch, pMob);

            pMob->start_pos = value;
            SEND ("Start position set.\r\n", ch);
            return TRUE;

        case 'D':
        case 'd':
            if (str_prefix (arg, "default"))
                break;

            if ((value = flag_value (position_flags, argument)) == NO_FLAG)
                break;

            EDIT_MOB (ch, pMob);

            pMob->default_pos = value;
            SEND ("Default position set.\r\n", ch);
            return TRUE;
    }

    SEND ("Syntax:  position [start/default] [position]\r\n"
                  "Type '? position' for a list of positions.\r\n", ch);
    return FALSE;
}


MEDIT (medit_gold)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  wealth [number]\r\n", ch);
        return FALSE;
    }

    pMob->wealth = atoi (argument);

    SEND ("Wealth set.\r\n", ch);
    return TRUE;
}

MEDIT (medit_hitroll)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  hitroll [number]\r\n", ch);
        return FALSE;
    }

    pMob->hitroll = atoi (argument);

    SEND ("Hitroll set.\r\n", ch);
    return TRUE;
}

void show_liqlist (CHAR_DATA * ch)
{
    int liq;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = new_buf ();

    for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
        if ((liq % 21) == 0)
            add_buf (buffer,
                     "Name                 Color          Proof Full Thirst Food Ssize\r\n");

        sprintf (buf, "%-20s %-14s %5d %4d %6d %4d %5d\r\n",
                 liq_table[liq].liq_name, liq_table[liq].liq_color,
                 liq_table[liq].liq_affect[0], liq_table[liq].liq_affect[1],
                 liq_table[liq].liq_affect[2], liq_table[liq].liq_affect[3],
                 liq_table[liq].liq_affect[4]);
        add_buf (buffer, buf);
    }

    page_to_char (buf_string (buffer), ch);
    free_buf (buffer);

    return;
}

void show_damlist (CHAR_DATA * ch)
{
    int att;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = new_buf ();

    for (att = 0; attack_table[att].name != NULL; att++)
    {
        if ((att % 21) == 0)
            add_buf (buffer, "Name                 Noun\r\n");

        sprintf (buf, "%-20s %-20s\r\n",
                 attack_table[att].name, attack_table[att].noun);
        add_buf (buffer, buf);
    }

    page_to_char (buf_string (buffer), ch);
    free_buf (buffer);

    return;
}

MEDIT (medit_group)
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER *buffer;
    bool found = FALSE;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0')
    {
        SEND ("Syntax: group [number]\r\n", ch);
        SEND ("        group show [number]\r\n", ch);
        return FALSE;
    }

    if (is_number (argument))
    {
        pMob->group = atoi (argument);
        SEND ("Group set.\r\n", ch);
        return TRUE;
    }

    argument = one_argument (argument, arg);

    if (!strcmp (arg, "show") && is_number (argument))
    {
        if (atoi (argument) == 0)
        {
            SEND ("Are you crazy?\r\n", ch);
            return FALSE;
        }

        buffer = new_buf ();

        for (temp = 0; temp < 65536; temp++)
        {
            pMTemp = get_mob_index (temp);
            if (pMTemp && (pMTemp->group == atoi (argument)))
            {
                found = TRUE;
                sprintf (buf, "[%5ld] %s\r\n", pMTemp->vnum,
                         pMTemp->player_name);
                add_buf (buffer, buf);
            }
        }

        if (found)
            page_to_char (buf_string (buffer), ch);
        else
            SEND ("No mobs in that group.\r\n", ch);

        free_buf (buffer);
        return FALSE;
    }

    return FALSE;
}

MEDIT (medit_copy)
{
	MOB_INDEX_DATA *pMobOrig;
	MOB_INDEX_DATA *pMobCopy;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int vnum;
	char buf[MAX_STRING_LENGTH];
 
	if (argument[0] == '\0')
	{
		sprintf (buf, "{rMEDIT: {yCOPY: {xYou must specify a vnum to copy.\r\n"
		              "       Syntax: COPY <vnum> <argument>\r\n"
		              "       {yValid Arguments:{x\r\n"
        "          name, act, act2, sex, group, race, level, alignment,\r\n"
        "          hitroll, dam_type (damage), hit dice, damage dice, \r\n"
        "          mana, aff, armor/ac, form, parts, imm, off, shop,\r\n"
        "          size, start, default, wealth, special, short, long, desc, walk/entrance.\r\n"
		"          Use \'all\' to clone the mob.\r\n");
		SEND (buf, ch);
		return FALSE;
	}
	bool cAll;
 
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
 
	if (!str_cmp (arg2, "all" ) || arg2[0] == '\0')
		cAll = TRUE;
	else
		cAll = FALSE;
 
 
	if (!is_number(arg1))
	{
		sprintf (buf, "{rMEDIT: {yCOPY: {xYou must specify a vnum to copy.\r\n" \
					  "       Syntax: COPY <vnum>\r\n" \
					  "               COPY <vnum> <argument>\r\n" \
					  "       %s is not a number.\r\n", arg1);
		SEND (buf, ch);
		return FALSE;
	}
	else
	{
		vnum = atoi(arg1);
		if ((pMobOrig = get_mob_index(vnum)) == NULL)
		{
			sprintf (buf, "{rMEDIT: {yCOPY: {xYou must specify an EXISTING mob to copy.\r\n"
					  "       %d is not an existing mob.\r\n", vnum);
			SEND (buf, ch);
			return FALSE;
		}
 
 
    EDIT_MOB (ch, pMobCopy);
	if (cAll || !str_prefix (arg2, "name"))
	{
      free_string( pMobCopy->player_name );
      pMobCopy->player_name = str_dup( pMobOrig->player_name );
      sprintf (buf, "{rMEDIT: {yCOPY:{x Name copied:             {y%s{x\r\n",
        pMobCopy->player_name);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_cmp (arg2, "act"))
	{
      pMobCopy->act = pMobOrig->act;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Act copied:              {y%s{x\r\n",
        flag_string(act_flags, pMobCopy->act));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}	
	if (cAll || !str_cmp (arg2, "act2"))
	{
      pMobCopy->act2 = pMobOrig->act2;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Act2 copied:             {y%s{x\r\n",
        flag_string(act2_flags, pMobCopy->act2));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "sex"))
	{
      pMobCopy->sex = pMobOrig->sex;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Sex copied:              {y%s{x\r\n",
      pMobCopy->sex == SEX_MALE    ? "male"   :
	  pMobCopy->sex == SEX_FEMALE  ? "female" : 
	  pMobCopy->sex == 3           ? "random" : "neutral");
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	/* If you have created mob PERM stats and coded it exactly the same
	way that I happened to do so? Uncomment this heh. 
	if (cAll || is_name (arg2, "permstat"))
	{
      sprintf (buf, "{rMEDIT: {yCOPY:{x Perm stats copied:       {y");
      SEND (buf, ch);
      int str, inte, wis, dex, con;
      str = pMobOrig->perm_stat[STAT_STR];
      inte = pMobOrig->perm_stat[STAT_INT];
      wis = pMobOrig->perm_stat[STAT_WIS];
      dex = pMobOrig->perm_stat[STAT_DEX];
      con = pMobOrig->perm_stat[STAT_CON];
      sprintf (buf, "STR:{%s%2d{y INT:{%s%2d{y WIS:{%s%2d{y DEX:{%s%2d{y CON:{%s%2d{y\r\n", str == 0 ? "y":"r", str, inte == 0 ? "y":"r", inte, wis == 0 ? "y":"r", wis, dex == 0 ? "y":"r", dex, con == 0 ? "y":"r", con);
      if (str + inte + wis + dex + con > 0)
        SEND (buf, ch);
      else
        SEND ("None modified.\r\n", ch);
	  if (!cAll)
	    return TRUE;
	} */
	/* Resistances on Ansalon are an array of ints, yours aren't, ignore 
	if (cAll || is_name (arg2, "resistance") || is_name (arg2, "resmod"))
	{
	char reses[MAX_STRING_LENGTH];
	int i, col;
	bool resshow = FALSE;
    sprintf (reses, "{rMEDIT: {yCOPY:{x Resistance Mods copied:  ");
    for (i = 1, col = 0; res_table[i].cname != NULL; i++)
      {
        if (pMobOrig->res_mod[i] != 0)
        {
          sprintf (buf, "%-10s {%s%4d{x ", res_table[i].cname, pMobOrig->res_mod[i] > 0 ? "y" : "r", pMobOrig->res_mod[i]);
          strcat (reses, buf);
          pMobCopy->res_mod[i] = pMobOrig->res_mod[i];
          if (++col % 3 == 0)
            strcat (reses, "\r\n                                      ");
          resshow = TRUE;
        }
      }
      if (resshow)
      {
        SEND (reses, ch);
        if (col % 4 != 0)
          SEND ("\r\n", ch);
      }
 
	  if (!cAll)
	    return TRUE;
	}*/
	if (cAll || !str_prefix (arg2, "group"))
	{
      pMobCopy->group = pMobOrig->group;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Group copied:            {y%d{x\r\n",
        pMobCopy->group);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "race"))
	{
      pMobCopy->race = pMobOrig->race;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Race copied:             {y%s{x\r\n",
        race_table[pMobCopy->race].name);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "level"))
	{
      pMobCopy->level = pMobOrig->level;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Level copied:            {y%d{x\r\n",
        pMobCopy->level);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "alignment"))
	{
      pMobCopy->alignment = pMobOrig->alignment;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Alignment copied:        {y%d{x\r\n",
        pMobCopy->alignment);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_cmp (arg2, "hitroll"))
	{
      pMobCopy->hitroll = pMobOrig->hitroll;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Hitroll copied:          {y%d{x\r\n",
        pMobCopy->hitroll);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_cmp (arg2, "dam_type") || !str_cmp (arg2, "damage"))
	{
      pMobCopy->dam_type = pMobOrig->dam_type;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Damage type copied:      {y%s{x\r\n",
        attack_table[pMobCopy->dam_type].name);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_cmp (arg2, "hit") || !str_cmp (arg2, "hit dice"))
	{
      pMobCopy->hit[DICE_NUMBER] = pMobOrig->hit[DICE_NUMBER];
      pMobCopy->hit[DICE_TYPE]   = pMobOrig->hit[DICE_TYPE];
      pMobCopy->hit[DICE_BONUS]  = pMobOrig->hit[DICE_BONUS];
 
      sprintf (buf, "{rMEDIT: {yCOPY:{x Hit dice copied:         {y%2dd%-3d+%4d{x\r\n",
        pMobCopy->hit[DICE_NUMBER], pMobCopy->hit[DICE_TYPE], pMobCopy->hit[DICE_BONUS]);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "damdice") || !str_cmp (arg2, "damage dice"))
	{
      pMobCopy->damage[DICE_NUMBER] = pMobOrig->damage[DICE_NUMBER];
      pMobCopy->damage[DICE_TYPE]   = pMobOrig->damage[DICE_TYPE];
      pMobCopy->damage[DICE_BONUS]  = pMobOrig->damage[DICE_BONUS];
 
      sprintf (buf, "{rMEDIT: {yCOPY:{x Damage dice copied:      {y%2dd%-3d+%4d{x\r\n",
        pMobCopy->damage[DICE_NUMBER], pMobCopy->damage[DICE_TYPE], pMobCopy->damage[DICE_BONUS]);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "mana") || !str_cmp (arg2, "mana dice"))
	{
      pMobCopy->mana[DICE_NUMBER] = pMobOrig->mana[DICE_NUMBER];
      pMobCopy->mana[DICE_TYPE]   = pMobOrig->mana[DICE_TYPE];
      pMobCopy->mana[DICE_BONUS]  = pMobOrig->mana[DICE_BONUS];
 
      sprintf (buf, "{rMEDIT: {yCOPY:{x Mana dice copied:        {y%2dd%-3d+%4d{x\r\n",
        pMobCopy->mana[DICE_NUMBER], pMobCopy->mana[DICE_TYPE], pMobCopy->mana[DICE_BONUS]);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_cmp (arg2, "aff"))
	{
      pMobCopy->affected_by = pMobOrig->affected_by;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Aff copied:              {y%s{x\r\n",
        flag_string(affect_flags, pMobCopy->affected_by));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	/* If you've installed 'affect2' code, de-comment this:
	if (cAll || !str_cmp (arg2, "aff2"))
	{
      pMobCopy->affected_by2 = pMobOrig->affected_by2;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Aff2 copied:             {y%s{x\r\n",
        flag_string(affect_flags2, pMobCopy->affected_by2));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}*/
	if (cAll || !str_cmp (arg2, "armor") || !str_cmp (arg2, "ac"))
	{
      pMobCopy->ac[AC_PIERCE] = pMobOrig->ac[AC_PIERCE];
      pMobCopy->ac[AC_BASH]   = pMobOrig->ac[AC_BASH];
      pMobCopy->ac[AC_SLASH]  = pMobOrig->ac[AC_SLASH];
      pMobCopy->ac[AC_EXOTIC] = pMobOrig->ac[AC_EXOTIC];
      sprintf (buf, "{rMEDIT: {yCOPY:{x Armor copied:            pierce: {y%d{x  bash: {y%d{x  slash: {y%d{x  magic: {y%d{x\r\n",
        pMobCopy->ac[AC_PIERCE], pMobCopy->ac[AC_BASH],
		pMobCopy->ac[AC_SLASH], pMobCopy->ac[AC_EXOTIC]);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "form"))
	{
      pMobCopy->form = pMobOrig->form;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Form copied:             {y%s{x\r\n",
        flag_string(form_flags, pMobCopy->form));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "parts"))
	{
      pMobCopy->parts = pMobOrig->parts;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Parts copied:            {y%s{x\r\n",
        flag_string(part_flags, pMobCopy->parts));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "immunities") 
	|| !str_prefix (arg2, "immune"))
	{
      pMobCopy->imm_flags = pMobOrig->imm_flags;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Immunities copied:       {y%s{x\r\n",
        flag_string(imm_flags, pMobCopy->imm_flags));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "offense"))
	{
      pMobCopy->off_flags = pMobOrig->off_flags;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Offensive copied:        {y%s{x\r\n",
        flag_string(off_flags, pMobCopy->off_flags));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "size"))
	{
      pMobCopy->size = pMobOrig->size;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Size copied:             {y%s{x\r\n",
        flag_string(size_flags, pMobCopy->size));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "start") || !str_cmp (arg2, "position start"))
	{
      pMobCopy->start_pos = pMobOrig->start_pos;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Start Position copied:   {y%s{x\r\n",
        flag_string(position_flags, pMobCopy->start_pos));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "default") || !str_cmp (arg2, "position default"))
	{
      pMobCopy->default_pos = pMobOrig->default_pos;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Default Position copied: {y%s{x\r\n",
        flag_string(position_flags, pMobCopy->default_pos));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "wealth"))
	{
      pMobCopy->wealth = pMobOrig->wealth;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Wealth copied:           {y%ld{x\r\n",
        pMobCopy->wealth);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "spec_fun") || !str_prefix (arg2, "special"))
	{
      pMobCopy->spec_fun = pMobOrig->spec_fun;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Spec copied:             {y%s{x\r\n",
        spec_name (pMobCopy->spec_fun));
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "short"))
	{
      free_string( pMobCopy->short_descr );
      pMobCopy->short_descr = str_dup( pMobOrig->short_descr );
      sprintf (buf, "{rMEDIT: {yCOPY:{x Short Description copied:{y%s{x\r\n",
        pMobCopy->short_descr);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "long"))
	{
      free_string( pMobCopy->long_descr );
      pMobCopy->long_descr = str_dup( pMobOrig->long_descr );
      sprintf (buf, "{rMEDIT: {yCOPY:{x Long Description copied: {y%s{x",
        pMobCopy->long_descr);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_prefix (arg2, "walk") || !str_prefix (arg2, "entrance"))
	{
		free_string( pMobCopy->walk_desc );
		pMobCopy->walk_desc = str_dup( pMobOrig->walk_desc );
		sprintf (buf, "{rOEDIT: {yCOPY:{x walk description copied:      {y%s{x\r\n", pMobCopy->walk_desc);
		SEND(buf,ch);
		if (!cAll)
			return TRUE;
	}
	if (cAll || !str_prefix (arg2, "description"))
	{
      free_string( pMobCopy->description );
      pMobCopy->description = str_dup( pMobOrig->description );
      sprintf (buf, "{rMEDIT: {yCOPY:{x Description copied:\r\n%s{x",
        pMobCopy->description);
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
	}
	if (cAll || !str_cmp (arg2, "shop"))
	{
      if (pMobOrig->pShop) // Only do this if the original has a shop.
      {
      	if (!pMobCopy->pShop) // ONLY assign shop if there isn't one.
      	{
      		pMobCopy->pShop = new_shop();
      		if (!shop_first)
      			shop_first = pMobCopy->pShop;
      		if (shop_last)
      			shop_last->next = pMobCopy->pShop;
      		shop_last = pMobCopy->pShop;
	        pMobCopy->pShop->keeper	= pMobCopy->vnum;
      		sprintf (buf, "{rMEDIT: {yCOPY:{x Shop assigned:           {y%s{x\r\n",
              pMobCopy->pShop ? "yes":"no");
        	SEND(buf,ch);
      	}
	// Shop open copy
      	pMobCopy->pShop->open_hour = pMobOrig->pShop->open_hour;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Shop open copied:        {y%d{x\r\n",
        pMobCopy->pShop->open_hour);
        SEND(buf,ch);
	// Shop close copy
      	pMobCopy->pShop->close_hour = pMobOrig->pShop->close_hour;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Shop close copied:       {y%d{x\r\n",
        pMobCopy->pShop->close_hour);
        SEND(buf,ch);
	// Shop profit copy
      	pMobCopy->pShop->profit_buy = pMobOrig->pShop->profit_buy;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Shop profit buy copied:  {y%d{x\r\n",
        pMobCopy->pShop->profit_buy);
        SEND(buf,ch);
      	pMobCopy->pShop->profit_sell = pMobOrig->pShop->profit_sell;
      sprintf (buf, "{rMEDIT: {yCOPY:{x Shop profit sell copied: {y%d{x\r\n",
        pMobCopy->pShop->profit_sell);
        SEND(buf,ch);
    // Shop types
      int iTrade;
      for (iTrade=0;iTrade<MAX_TRADE;iTrade++)
    	{
    	  pMobCopy->pShop->buy_type[iTrade] = pMobOrig->pShop->buy_type[iTrade];
    	  sprintf (buf, "{rMEDIT: {yCOPY:{x Shop type copied:        {y[%d] %s{x\r\n",
            iTrade, flag_string (type_flags, pMobCopy->pShop->buy_type[iTrade]));
          SEND (buf, ch);
    	}
 
      sprintf (buf, "{rMEDIT: {yCOPY:{x Shop copied.{x\r\n");
      SEND(buf,ch);
      if (!cAll)
      	return TRUE;
      }// End of 'has pShop' to copy
      else
	  {
         sprintf (buf, "{rMEDIT: {yCOPY:{x No shop to copy.{x\r\n");
         SEND(buf,ch);
      }
	}
	if (!pMobOrig->pShop && pMobCopy->pShop)
	{
	SHOP_DATA *pShop;
 
	pShop		= pMobCopy->pShop;
	pMobCopy->pShop	= NULL;
 
	if ( pShop == shop_first )
	{
		if ( !pShop->next )
		{
			shop_first = NULL;
			shop_last = NULL;
		}
		else
			shop_first = pShop->next;
	}
	else
	{
		SHOP_DATA *ipShop;
 
		for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
		{
			if ( ipShop->next == pShop )
			{
				if ( !pShop->next )
				{
					shop_last = ipShop;
					shop_last->next = NULL;
				}
				else
					ipShop->next = pShop->next;
			}
		}
	}
 
	free_shop(pShop);
 
	SEND("{rMEDIT: {yCOPY:{x Shop removed.{x\r\n", ch);
    if (!cAll)
      return TRUE;
 
	}
 
// Straight copy, just let them know it all was.
	if (cAll)
    {
      sprintf(buf,  "\r\n{rMEDIT: {yCOPY: %s (Mob %d) duplicated.{x\r\n", pMobOrig->short_descr, vnum );
	  SEND (buf, ch);
      return TRUE;
    }
 
// Not a valid command/part, let them know, and yes. I'm a goof, Syntax error heh.
      sprintf(buf, 
		"{rMEDIT: {yCOPY:{x Syntax Error. \"{r%s{x\"\r\n"
		"       {yValid Arguments:{x\r\n"
        "          name, act, act2, sex, group, race, level, alignment,\r\n"
        "          hitroll, dam_type (damage), hit dice, damage dice, \r\n"
        "          mana, aff, armor/ac, form, parts, imm, off, shop,\r\n"
        "          size, start, default, wealth, special, short, long, desc.\r\n"
		"          Use \'all\' to clone the mob.\r\n", arg2);
      SEND (buf, ch);
      return FALSE;
	}
}


REDIT (redit_owner)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM (ch, pRoom);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  owner [owner]\r\n", ch);
        SEND ("         owner none\r\n", ch);
        return FALSE;
    }

    free_string (pRoom->owner);
    if (!str_cmp (argument, "none"))
        pRoom->owner = str_dup ("");
    else
        pRoom->owner = str_dup (argument);

    SEND ("Owner set.\r\n", ch);
    return TRUE;
}

MEDIT (medit_addmprog)
{
    int value;
    MOB_INDEX_DATA *pMob;
    PROG_LIST *list;
    PROG_CODE *code;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_MOB (ch, pMob);
    argument = one_argument (argument, num);
    argument = one_argument (argument, trigger);
    argument = one_argument (argument, phrase);

    if (!is_number (num) || trigger[0] == '\0' || phrase[0] == '\0')
    {
        SEND ("Syntax:   addmprog [vnum] [trigger] [phrase]\r\n", ch);
        return FALSE;
    }

    if ((value = flag_value (mprog_flags, trigger)) == NO_FLAG)
    {
        SEND ("Valid flags are:\r\n", ch);
        show_help (ch, "mprog");
        return FALSE;
    }

    if ((code = get_prog_index(atoi (num), PRG_MPROG)) == NULL)
    {
        SEND ("No such MOBProgram.\r\n", ch);
        return FALSE;
    }

    list = 				new_mprog ();
    list->vnum = 		atoi (num);
    list->trig_type = 	value;
    list->trig_phrase = str_dup (phrase);
    list->code = 		code->code;
    SET_BIT (pMob->mprog_flags, value);
    list->next =		pMob->mprogs;
    pMob->mprogs = 		list;

    SEND ("Mprog Added.\r\n", ch);
    return TRUE;
}

MEDIT (medit_delmprog)
{
    MOB_INDEX_DATA *pMob;
    PROG_LIST *list;
    PROG_LIST *list_next;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_MOB (ch, pMob);

    one_argument (argument, mprog);
    if (!is_number (mprog) || mprog[0] == '\0')
    {
        SEND ("Syntax:  delmprog [#mprog]\r\n", ch);
        return FALSE;
    }

    value = atoi (mprog);

    if (value < 0)
    {
        SEND ("Only non-negative mprog-numbers allowed.\r\n", ch);
        return FALSE;
    }

    if (!(list = pMob->mprogs))
    {
        SEND ("MEdit:  Non existant mprog.\r\n", ch);
        return FALSE;
    }

    if (value == 0)
    {
        REMOVE_BIT (pMob->mprog_flags, pMob->mprogs->trig_type);
        list = pMob->mprogs;
        pMob->mprogs = list->next;
        free_mprog (list);
    }
    else
    {
        while ((list_next = list->next) && (++cnt < value))
            list = list_next;

        if (list_next)
        {
            REMOVE_BIT (pMob->mprog_flags, list_next->trig_type);
            list->next = list_next->next;
            free_mprog (list_next);
        }
        else
        {
            SEND ("No such mprog.\r\n", ch);
            return FALSE;
        }
    }

    SEND ("Mprog removed.\r\n", ch);
    return TRUE;
}

REDIT (redit_room)
{
    ROOM_INDEX_DATA *room;
    int value;

    EDIT_ROOM (ch, room);

    if ((value = flag_value (room_flags, argument)) == NO_FLAG)
    {
        SEND ("Syntax: room [flags]\r\n", ch);
        return FALSE;
    }

    TOGGLE_BIT (room->room_flags, value);
    SEND ("Room flags toggled.\r\n", ch);
    return TRUE;
}

REDIT (redit_sector)
{
    ROOM_INDEX_DATA *room;
    int value;

    EDIT_ROOM (ch, room);

    if ((value = flag_value (sector_flags, argument)) == NO_FLAG)
    {
        SEND ("Syntax: sector [type]\r\n", ch);
        return FALSE;
    }

    room->sector_type = value;
    SEND ("Sector type set.\r\n", ch);

    return TRUE;
}


OEDIT ( oedit_addoprog )
{
  int value;
  OBJ_INDEX_DATA *pObj;
  PROG_LIST *list;
  PROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];
 
  EDIT_OBJ(ch, pObj);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);
 
  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
        SEND("Syntax:   addoprog [vnum] [trigger] [phrase]\r\n",ch);
        return FALSE;
  }
 
  if ( (value = flag_value (oprog_flags, trigger) ) == NO_FLAG)
  {
        SEND("Valid flags are:\r\n",ch);
        show_help( ch, "oprog");
        return FALSE;
  }
 
  if ( ( code =get_prog_index (atoi(num), PRG_OPROG ) ) == NULL)
  {
        SEND("No such OBJProgram.\r\n",ch);
        return FALSE;
  }
 
  list                  = new_oprog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  list->code            = code->code;
  SET_BIT(pObj->oprog_flags,value);
  list->next            = pObj->oprogs;
  pObj->oprogs          = list;
 
  SEND( "Oprog Added.\r\n",ch);
  return TRUE;
}
 
OEDIT ( oedit_deloprog )
{
    OBJ_INDEX_DATA *pObj;
    PROG_LIST *list;
    PROG_LIST *list_next;
    char oprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;
 
    EDIT_OBJ(ch, pObj);
 
    one_argument( argument, oprog );
    if (!is_number( oprog ) || oprog[0] == '\0' )
    {
       SEND("Syntax:  deloprog [#oprog]\r\n",ch);
       return FALSE;
    }
 
    value = atoi ( oprog );
 
    if ( value < 0 )
    {
        SEND("Only non-negative oprog-numbers allowed.\r\n",ch);
        return FALSE;
    }
 
    if ( !(list= pObj->oprogs) )
    {
        SEND("OEdit:  Non existant oprog.\r\n",ch);
        return FALSE;
    }
 
    if ( value == 0 )
    {
	REMOVE_BIT(pObj->oprog_flags, pObj->oprogs->trig_type);
        list = pObj->oprogs;
        pObj->oprogs = list->next;
        free_oprog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;
 
        if ( list_next )
        {
		REMOVE_BIT(pObj->oprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_oprog(list_next);
        }
        else
        {
                SEND("No such oprog.\r\n",ch);
                return FALSE;
        }
    }
 
    SEND("Oprog removed.\r\n", ch);
    return TRUE;
}
 
REDIT ( redit_addrprog )
{
  int value;
  ROOM_INDEX_DATA *pRoom;
  PROG_LIST *list;
  PROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];
 
  EDIT_ROOM(ch, pRoom);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);
 
  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
        SEND("Syntax:   addrprog [vnum] [trigger] [phrase]\r\n",ch);
        return FALSE;
  }
 
  if ( (value = flag_value (rprog_flags, trigger) ) == NO_FLAG)
  {
        SEND("Valid flags are:\r\n",ch);
        show_help( ch, "rprog");
        return FALSE;
  }
 
  if ( ( code =get_prog_index (atoi(num), PRG_RPROG ) ) == NULL)
  {
        SEND("No such ROOMProgram.\r\n",ch);
        return FALSE;
  }
 
  list                  = new_rprog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  list->code            = code->code;
  SET_BIT(pRoom->rprog_flags,value);
  list->next            = pRoom->rprogs;
  pRoom->rprogs          = list;
 
  SEND( "Rprog Added.\r\n",ch);
  return TRUE;
}
 
REDIT ( redit_delrprog )
{
    ROOM_INDEX_DATA *pRoom;
    PROG_LIST *list;
    PROG_LIST *list_next;
    char rprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;
 
    EDIT_ROOM(ch, pRoom);
 
    one_argument( argument, rprog );
    if (!is_number( rprog ) || rprog[0] == '\0' )
    {
       SEND("Syntax:  delrprog [#rprog]\r\n",ch);
       return FALSE;
    }
 
    value = atoi ( rprog );
 
    if ( value < 0 )
    {
        SEND("Only non-negative rprog-numbers allowed.\r\n",ch);
        return FALSE;
    }
 
    if ( !(list= pRoom->rprogs) )
    {
        SEND("REdit:  Non existant rprog.\r\n",ch);
        return FALSE;
    }
 
    if ( value == 0 )
    {
	REMOVE_BIT(pRoom->rprog_flags, pRoom->rprogs->trig_type);
        list = pRoom->rprogs;
        pRoom->rprogs = list->next;
        free_rprog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;
 
        if ( list_next )
        {
		REMOVE_BIT(pRoom->rprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_rprog(list_next);
        }
        else
        {
                SEND("No such rprog.\r\n",ch);
                return FALSE;
        }
    }
 
    SEND("Rprog removed.\r\n", ch);
    return TRUE;
}


/* called from do_instaroom further below */
void reset_instaroom( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA     *pRoom;
  CHAR_DATA           *mob;
  OBJ_DATA            *obj, *inobj;

  pRoom = ch->in_room;

  /* Containers can NOT be closed when doing an 'instaroom'. Thus a builder will have to close
   * the container after the reset is installed
   */

  /* lets go through a mob first */
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  {
     if ( IS_NPC( mob ) )
     {
         /* only mobs with this areas vnums are allowed */
        if ( mob->pIndexData->area != pRoom->area )
        {
           SEND( "There is a mob in this room that is not part of your area, resets not set.\r\n", ch );
           return;
        }
    
        for ( obj = mob->carrying; obj; obj = obj->next_content )
        {
           if( obj->pIndexData->area != pRoom->area )
           {
               SEND( "There is an object in a MOB that is not part of your area, resets not set.\r\n", ch );
              return;
           }
      
           if ( IS_SET( obj->value[1], CONT_CLOSED ) )
           {
              SEND( "There is a container in a MOB I can't see inside of. Get the container from the mob, open\r\n"
                            "it up, give it _back_ to your mob, then do an instaroom.\r\n" 
                            "AFTER you have set the container to load into the mob,\r\n"
                            "you can then get the container from him again, close/lock it, and return it....In other words\r\n"
                            "A container must be open first, the reset installed, after that, you can close/lock it.\r\n",ch);
              return;
           }

           if ( obj->contains )
           {
               for (inobj = obj->contains; inobj; inobj = obj->next_content )
               {
                   if ( inobj->pIndexData->area != pRoom->area )
                   {
                      SEND("There is an object in a container which a MOB in this room has, which is not\r\n"
                                   "a vnum for this area. RESETS NOT SET.\r\n",ch);
                      return;
                   }
               }
           }
        }
     }
  }/* done looking at mobs and their eq/inv */


  /* lets take a look at objects in the room and their contents */ 
  for ( obj = pRoom->contents; obj != NULL; obj = obj->next_content )
  {
      if( obj->pIndexData->area != pRoom->area )
      {
          SEND( "There is an object in this room that is not a vnum of your area, resets not set.\r\n", ch );
          return;
      }
  
      if ( IS_SET( obj->value[1], CONT_CLOSED ) )
      {
          SEND( "There is a container in this room I can't see inside of. Open it up first, do the\r\n"
                        "instaroom command, THEN you can close/lock the container.\r\n",ch);
          return;
      }

      if ( obj->contains )
      {
         for (inobj = obj->contains; inobj; inobj = obj->next_content )
         {
            if ( inobj->pIndexData->area != pRoom->area )
            {
               SEND("There is an object in a container that does not a vnum in this area. No resets set.\r\n",ch);
               return;
            }
         }
      }
  }

  if ( pRoom->reset_first )
    wipe_resets(pRoom);  
  instaroom( pRoom );

  SEND( "Room resets installed.\r\n", ch );
  return;
}

/* Separate function for recursive purposes */
void delete_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset, int insert_loc, bool wipe_all)
{

  if( !pRoom->reset_first )
  {
     return;
  }

  if ( insert_loc-1 <= 0 )
  {
       pReset = pRoom->reset_first;
       pRoom->reset_first = pRoom->reset_first->next;
       if ( !pRoom->reset_first )
           pRoom->reset_last = NULL;
   }
   else
   {
       int iReset = 0;
       RESET_DATA *prev = NULL;

       for ( pReset = pRoom->reset_first;
             pReset;
             pReset = pReset->next )
       {
             if ( ++iReset == insert_loc )
                    break;
             prev = pReset;
       }

       if ( !pReset )
       {
           return;
       }

        if ( prev )
            prev->next = prev->next->next;
        else
            pRoom->reset_first = pRoom->reset_first->next;

        for ( pRoom->reset_last = pRoom->reset_first;
              pRoom->reset_last->next;
              pRoom->reset_last = pRoom->reset_last->next );
   }/*else*/

   if ( pReset == pRoom->last_mob_reset )
    pRoom->last_mob_reset = NULL;
  if ( pReset == pRoom->last_obj_reset )
    pRoom->last_obj_reset = NULL;

   free_reset_data( pReset );

   /* TRUE/FALSE call on delete_reset here, so we can delete all resets or just one */
   if( wipe_all )
   {
     if( ( pReset = pRoom->reset_first)  != '\0')
       delete_reset( pRoom, pReset, 1, TRUE );
   }
}

void wipe_resets( ROOM_INDEX_DATA *pRoom )
{
  RESET_DATA *pReset;
  int i = 1;
  
  for ( pReset = pRoom->reset_first; pReset != NULL; )
  {
    if (pReset)
    {
      delete_reset(pRoom, pReset, i, TRUE);
    }
    i++;
    pReset = pReset->next;
  }
  return;
}

/* make_reset called by add_new_reset in order to create a blank
 * reset to load data into the reset_list 
 */
RESET_DATA *make_reset( char letter, int arg1, int arg2, int arg3, int arg4 )
{
        RESET_DATA *pReset;

        pReset          = new_reset_data();
        pReset->command = letter;
        pReset->arg1    = arg1;
        pReset->arg2    = arg2;
        pReset->arg3    = arg3;
        pReset->arg4    = arg4;
        return pReset;
}


/* add_new_reset called from several places below, it adds in the actual pReset
 * arguments to the reset_list..tricky footwork here 
 */
RESET_DATA *add_new_reset( ROOM_INDEX_DATA *pRoom, char letter, int arg1, int arg2, int arg3, int arg4 )
{
    RESET_DATA *pReset;

    if ( !pRoom )
    {
        bug( "add_reset: NULL area!", 0 );
        return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, arg1, arg2, arg3, arg4 );
    switch( letter )
    {
        case 'M':  pRoom->last_mob_reset = pReset;      break;
        case 'E':  case 'G':  case 'P':
        case 'O':  pRoom->last_obj_reset = pReset;      break;
            break;
    }

    /* LINK code from the SMAUG server */
    LINK( pReset, pRoom->reset_first, pRoom->reset_last, next, prev );
    return pReset;
}


/* Called from instaroom */
/* The below function debugged by Chalc on the Romlist, thanx again Chalc */
void add_obj_reset( ROOM_INDEX_DATA *pRoom, char cm, OBJ_DATA *obj, int v2, int v3, int v4 )
{
  add_new_reset( pRoom, cm, obj->pIndexData->vnum, v2, v3, v4 );

  if(obj->contains)
  {
    OBJ_INDEX_DATA **objList;
    OBJ_DATA *inObj;
    int *objCount;
    int count;
    int itemCount;

    for(inObj = obj->contains, itemCount = 0; inObj; inObj = inObj->next_content) itemCount++;

    /* Now have count objects in obj, allocate space for lists */
    objList = alloc_mem(itemCount * sizeof(OBJ_INDEX_DATA *));
    objCount = alloc_mem(itemCount * sizeof(int));
    
    /* Initialize Memory */
    memset(objList, 0, itemCount * sizeof(OBJ_INDEX_DATA *));
    memset(objCount, 0, itemCount * sizeof(int));

    /* Figure out how many of each obj is in the container */
    for(inObj = obj->contains; inObj; inObj = inObj->next_content)
    {
      for(count = 0; objList[count] && objList[count] != inObj->pIndexData; count++);
      if(!objList[count]) objList[count] = inObj->pIndexData;
      objCount[count]++;
    }    

    /* Create the resets */
    for(count = 0; objList[count]; count++)
      add_new_reset(pRoom, 'P', objList[count]->vnum, objCount[count], obj->pIndexData->vnum, objCount[count]);

    /* Free the memory */
    free_mem(objList, itemCount * sizeof(OBJ_INDEX_DATA *));
    free_mem(objCount, itemCount * sizeof(int));
  }
  /* And Done */
  return;
}

void instaroom( ROOM_INDEX_DATA *pRoom )
{
  CHAR_DATA *rch;
  OBJ_DATA *obj;
  
  for ( rch = pRoom->people; rch; rch = rch->next_in_room )
  {
    if ( !IS_NPC(rch) )
      continue;
    add_new_reset( pRoom, 'M',rch->pIndexData->vnum, rch->pIndexData->count, pRoom->vnum, 3 );
    for ( obj = rch->carrying; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE )
        add_obj_reset( pRoom, 'G', obj, 1, 0, 3 );
      else
        add_obj_reset( pRoom, 'E', obj, 1, obj->wear_loc, 3 );
    }
  }
  for ( obj = pRoom->contents; obj; obj = obj->next_content )
  {
    add_obj_reset( pRoom, 'O', obj, 1, pRoom->vnum, 1 );
  }
  return;
}

void do_ireset( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom = ch->in_room;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument, arg);

  if ( pRoom->reset_first )
    wipe_resets(pRoom);  
  instaroom( pRoom );

  SEND( "Room resets installed.\r\n", ch );
}









HELP_AREA *get_help_area (HELP_DATA * help)
{
    HELP_AREA *temp;
    HELP_DATA *thelp;

    for (temp = had_list; temp; temp = temp->next)
        for (thelp = temp->first; thelp; thelp = thelp->next_area)
            if (thelp == help)
                return temp;

    return NULL;
}

HEDIT (hedit_make)
{
	
    HELP_DATA *pHelp;
     
    if (argument[0] == '\0')
    {
        SEND("Syntax: hedit create [keyword(s)]\r\n",ch);
         return FALSE;
    }
 
    pHelp                        	= new_help();
    pHelp->keyword 					= str_dup(argument);
    pHelp->next                 	= help_first;
    help_first                    	= pHelp;
    ch->desc->pEdit               	= (void *)pHelp;
	pHelp->last_editted_by 			= str_dup(ch->name);
 
    SEND("New Help Entry Created.\r\n",ch);
    return TRUE;
}

HEDIT( hedit_show)
{
    HELP_DATA *pHelp;
    char buf[MSL];

    EDIT_HELP(ch,pHelp);

    if(pHelp->to_delete)  {
        SEND("\n\nTHIS HELP IS MARKED FOR DELETION!\r\n",ch);
        return FALSE;
    }
    
    sprintf(buf, "Level:    {g[{x%d{g]{x\r\n"
                 "Keywords: %s\r\n"
                "\r\n%s\r\n",
            pHelp->level, pHelp->keyword, pHelp->text);
    SEND(buf,ch);
    
    return FALSE;
}

HEDIT( hedit_desc)
{
    HELP_DATA *pHelp;
    EDIT_HELP(ch, pHelp);

    if (argument[0] =='\0')
    {
       string_append(ch, &pHelp->text);
	   pHelp->last_editted_by = str_dup(ch->name);
       return TRUE;
    }

    SEND(" Syntax: desc\r\n",ch);
    return FALSE;
}

HEDIT( hedit_keywords)
{
    HELP_DATA *pHelp;
    EDIT_HELP(ch, pHelp);

    if(argument[0] == '\0')
    {
        SEND(" Syntax: keywords [keywords]\r\n",ch);
        return FALSE;
    }

    pHelp->keyword = str_dup(argument);
	pHelp->last_editted_by = str_dup(ch->name);
    SEND( "Keyword(s) Set.\r\n", ch);
    return TRUE;
}

HEDIT(hedit_level)
{
    HELP_DATA *pHelp;

    EDIT_HELP(ch, pHelp);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	SEND( "Syntax:  level [number]\r\n", ch );
	return FALSE;
    }

    pHelp->level = atoi( argument );

	if (pHelp->level < 0)
		pHelp->level = 0;
	if (pHelp->level > MAX_LEVEL)
		pHelp->level = MAX_LEVEL;
	
	pHelp->last_editted_by = str_dup(ch->name);
    SEND( "Level set.\r\n", ch);
    return TRUE;
}

HEDIT( hedit_delete)
{
    HELP_DATA *pHelp;

    EDIT_HELP(ch,pHelp);

    if(!pHelp->to_delete) {
        pHelp->to_delete = TRUE;
        SEND("YOU HAVE MARKED THIS HELP DELETION!\r\n",ch);	
        return TRUE;
    }

    pHelp->to_delete = FALSE;
    SEND("YOU HAVE UNMARKED THIS HELP FOR DELETION!\r\n",ch);
    return TRUE;
}


void hedit( CHAR_DATA *ch, char *argument)
{
    char command[MIL];
    char arg[MIL];
    int cmd;
    
    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);

    if (ch->pcdata->security < 6)
    {
       SEND("HEdit: Insufficient security to modify helpfiles.\r\n",ch);
       edit_done(ch);
	   return;
    }

    if ( !str_cmp(command, "done") )
    {
		edit_done( ch );
		return;
    }

    if ( command[0] == '\0' )
    {
        hedit_show( ch, argument );
		return;
    }

    for ( cmd = 0; hedit_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, hedit_table[cmd].name ) )
	{
            (*hedit_table[cmd].olc_fun) ( ch, argument );
            return;
	}
    }

    interpret( ch, arg );
    return;    
}

void do_hedit( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char arg1[MIL];
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    bool found = FALSE;

    strcpy(arg1,argument);
    
    if(argument[0] != '\0')
    {
        /* Taken from do_help */
        argall[0] = '\0';
        while (argument[0] != '\0' )
        {
            argument = one_argument(argument,argone);
            if (argall[0] != '\0')
                strcat(argall," ");
            strcat(argall,argone);
        }

        for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
        {
            if ( is_name( argall, pHelp->keyword ) )
            {
                ch->desc->pEdit=(void *)pHelp;
                ch->desc->editor= ED_HELP;
				SEND ("Editting helpfile.\r\n",ch);
                found = TRUE;
                return;
            }
        }
    }

    if(!found)
    {
        argument = one_argument(arg1, arg1);
        
        if(!str_cmp(arg1,"create"))
        {
            if (argument[0] == '\0')
            {
                SEND("Syntax: create [topic]\r\n",ch);
                return;
            }
            if (hedit_make(ch, argument) )
                ch->desc->editor = ED_HELP;
            return;
        }
    }

    SEND( "HEdit:  There is no default help to edit.\r\n", ch );
    return;
}

HEDIT (hedit_list)
{
    char buf[MIL];
    int cnt = 0;
    HELP_DATA *pHelp;
    BUFFER *buffer;

    EDIT_HELP (ch, pHelp);

    if (!str_cmp (argument, "all"))
    {
        buffer = new_buf ();

        for (pHelp = help_first; pHelp; pHelp = pHelp->next)
        {
            sprintf (buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
                     cnt % 4 == 3 ? "\r\n" : " ");
            add_buf (buffer, buf);
            cnt++;
        }

        if (cnt % 4)
            add_buf (buffer, "\r\n");

        page_to_char (buf_string (buffer), ch);
        return FALSE;
    }

    if (!str_cmp (argument, "area"))
    {
        if (ch->in_room->area->helps == NULL)
        {
            SEND ("No helps in this area.\r\n", ch);
            return FALSE;
        }

        buffer = new_buf ();

        for (pHelp = ch->in_room->area->helps->first; pHelp;
             pHelp = pHelp->next_area)
        {
            sprintf (buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
                     cnt % 4 == 3 ? "\r\n" : " ");
            add_buf (buffer, buf);
            cnt++;
        }

        if (cnt % 4)
            add_buf (buffer, "\r\n");

        page_to_char (buf_string (buffer), ch);
        return FALSE;
    }

    if (IS_NULLSTR (argument))
    {
        SEND ("Syntax: list all\r\n", ch);
        SEND ("        list area\r\n", ch);
        return FALSE;
    }

    return FALSE;
}




MEDIT( medit_autoset )
{
    MOB_INDEX_DATA *pMob;
    char arg[100];
    int diff = -1, i = 0, stat1 = 0, stat2 = 0;
    int base = 0, level = 0;
         
    EDIT_MOB(ch, pMob);
    
    if ( IS_NULLSTR(argument))
    {
        SEND("Please choose veasy/easy/normal/hard/vhard to autoset.\n\r",ch);
        return FALSE;
    }
    
    one_argument(argument,arg);

    for (i=0;i<5;i++)
    {
        if ( !str_prefix(arg,mob_difficulty[i]))
        {
            diff = i;
            break;
        }
    }
    if ( diff < 0 || diff >= 5 )
    {
        SEND("Please choose veasy/easy/normal/hard/vhard to autoset.\n\r",ch);
        return FALSE;
    }
// subtract two, no need to figure things too much
    diff -= 2;

// figure hp right here
    level                       = pMob->level;
        
    base                        = mob_stat_table[level].hp;
    base += level * diff * (level < 20 ? 1 : level < 35 ? 2 : level < 50 ? 4 : 8 );
    base = number_range((4 * base) / 5, (5 * base) / 4);
         
    stat1                       = base / 4;
    stat2                       = number_range(1,level);
    pMob->hit[DICE_NUMBER]     	= UMAX(1,stat2);
    pMob->hit[DICE_TYPE]       	= UMAX(1,stat1 / stat2);
    pMob->hit[DICE_BONUS]       = UMAX(1,((3 * base) / 4) + (stat1 % stat2));
     
// hitroll here
    base                        = mob_stat_table[level].hit;
    base += ((level/9) + (level < 20 ? 1 : level < 50 ? 3 : 5 )) * diff;
    base                        = number_range(base - (level / 8), base + (level / 8));
    base                        = number_range(base - 1, base + 1);
    pMob->hitroll               = base;
    
// mana here, use hit point formula
    base                        = mob_stat_table[level].mana;
    base += level * diff * (level < 20 ? 1 : level < 35 ? 2 : level < 50 ? 4 : 8 );
    base = number_range((4 * base) / 5, (5 * base) / 4);
    
    stat1                       = base / 4;
    stat2                       = number_range(1,level);
    pMob->mana[DICE_NUMBER]     = UMAX(1,stat2);
    pMob->mana[DICE_TYPE]       = UMAX(1,stat1 / stat2);
    pMob->mana[DICE_BONUS]      = ((3 * base) / 4) + (stat1 % stat2);
    
// armor classes
    base                        = mob_stat_table[level].ac;
    base -= level * diff * (level < 40 ? 1 : level < 50 ? 2 : 3 ); 
    base                        = number_range(base - (level / 2), base + (level / 2));
    
    for ( i = 0; i < 3; i++)
        pMob->ac[i]             = base;
    pMob->ac[3]                 = (3 * base)/4;
    
// damage rolls
    base                        = mob_stat_table[level].dam;
    base += ((level/5) + (level < 20 ? 2 : level < 50 ? 5 : 10 )) * diff;
    base = number_range((4 * base) / 5, (5 * base) / 4);
    
    stat1                       = base / 2;
    stat2                       = number_range(1,level/2);
    pMob->damage[DICE_NUMBER]   = UMAX(1,stat2);
    pMob->damage[DICE_TYPE]     = UMAX(1,stat1 / stat2);   
    pMob->damage[DICE_BONUS]    = ( base / 2) + (stat1 % stat2);
    
    pMob->wealth 		= number_range( level / 2, level * 4 );
    pMob->form   		= race_table[pMob->race].form;
    pMob->parts  		= race_table[pMob->race].parts;
    pMob->dam_type 		= number_range(1,MAX_DAMAGE_MESSAGE);

    sprintf(arg, "Mob: %ld autoset for level %d\n\r",pMob->vnum, pMob->level );
    SEND(arg,ch);
    return TRUE;
}


NEDIT ( nedit_new )
{
	char *strtime;
	//char buf[200];
	
	if (IS_NPC(ch)) /* NPC cannot post notes */
		return FALSE;
		
	if (get_trust(ch) < ch->pcdata->board->write_level)
	{
		SEND ("You cannot post notes on this board.\r\n",ch);
		return FALSE;
	}
	
	/* continue previous note, if any text was written*/ 
	if (ch->pcdata->in_progress && (!ch->pcdata->in_progress->text))
	{
		SEND ("Note in progress cancelled because you did not manage to write any text \r\n"
		              "before losing link.\r\r\n\n",ch);
		free_note (ch->pcdata->in_progress);		              
		ch->pcdata->in_progress = NULL;
	}
	
	
	if (!ch->pcdata->in_progress)
	{
		ch->pcdata->in_progress = new_note();
		ch->pcdata->in_progress->sender = str_dup (ch->name);

		/* convert to ascii. ctime returns a string which last character is \n, so remove that */	
		strtime = ctime (&current_time);
		strtime[strlen(strtime)-1] = '\0';
	
		ch->pcdata->in_progress->date = str_dup (strtime);
	}
	else
	{
		SEND("You already have a note in progress!\r\n",ch);
		return FALSE;
	}

		act ("{G$n starts writing a note.{x", ch, NULL, NULL, TO_ROOM);
	
		/* Begin writing the note ! */
		printf_to_char (ch, "You are now %s a new note on the {W%s{x board.\r\n",
					   ch->pcdata->in_progress->text ? "continuing" : "posting",
					   ch->pcdata->board->short_name);
				
	return TRUE;
}

NEDIT ( nedit_to )
{
	char buf[MSL];

	if (!argument || argument[0] == '\0')
	{
		SEND("You must specify a recipient!\r\n",ch);
		return FALSE;
	}
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}
	/*
	if (!ch->pcdata->in_progress)
	{
		if (ch->desc->editor != ED_NOTE)
		{
			if (nedit_new(ch, argument))
			{
				ch->desc->editor = ED_NOTE;  
				smash_tilde (argument); // change ~ to - as we save this field as a string later 
				ch->pcdata->in_progress->to_list = str_dup (argument);
				SEND ("Recipient set.\r\n",ch);
				return TRUE;
			}
			else
			{
				SEND("Something went wrong starting the note.\r\n",ch);
				return FALSE;
			}
			//SEND("Start a new note first!\r\n",ch);	
			return 
		}
		else
		{
			SEND("You're already writing a note!\r\n",ch);
			return FALSE;
		}				
	}
	else*/
	
		smash_tilde (argument); /* change ~ to - as we save this field as a string later */
		sprintf(buf, "To: %s\r\n", argument);
		SEND(buf,ch);
		free_string(ch->pcdata->in_progress->to_list);
		ch->pcdata->in_progress->to_list = str_dup (argument);
		SEND ("Recipient set.\r\n",ch);
		return TRUE;	
	
}

NEDIT ( nedit_show )
{
	NOTE_DATA * note;
    char buf[MSL];    	    
	
    EDIT_NOTE (ch, note);

	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}
	
    sprintf (buf, "To:            [ %s ]\r\nSubject:       [ %s ]\r\n",
             ch->pcdata->in_progress->to_list, ch->pcdata->in_progress->subject);
    SEND (buf, ch);
	
	sprintf (buf, "Date:          [ %s ]\r\nType:          [ %s ]\r\n\r\n",
             ch->pcdata->in_progress->date, ch->pcdata->board->short_name);
    SEND (buf, ch);
	
	sprintf (buf, "Text (body): \r\n%s\r\n\r\n", ch->pcdata->in_progress->text);
    SEND (buf, ch);
	
	
	return TRUE;
}



NEDIT ( nedit_subject )
{
	char buf [MIL];

	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}	

	strcpy (buf, argument);
	smash_tilde (buf); /* change ~ to - as we save this field as a string later */
	
	/* Do not allow empty subjects */
	
	if (/*buf[0] == '\0' ||*/ strlen(buf) < 3)		
	{
		SEND ("Please find a meaningful subject!\r\n",ch);		
		return FALSE;
	}
	else if (strlen(buf)>60)
	{
		SEND ("No, no. This is just the Subject. You're note writing the note yet.\r\n",ch);
		return FALSE;
	}
	else
	/* advance to next stage */
	{
		ch->pcdata->in_progress->subject = str_dup(buf);
		ch->pcdata->in_progress->expire = current_time + ch->pcdata->board->purge_days * 24L * 3600L;
		SEND("Subject set.\r\n",ch);		
		return TRUE;
	}
	
	return FALSE;
}

NEDIT ( nedit_expire )
{
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}
	
	return FALSE;
}

NEDIT ( nedit_text )
{
	NOTE_DATA * note;

    EDIT_NOTE (ch, note);
	
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}	

    if (argument[0] == '\0')
    {
        string_append (ch, &ch->pcdata->in_progress->text);
        return TRUE;
    }

    SEND ("Syntax:  desc\r\n", ch);    
	return FALSE;
}

NEDIT ( nedit_send )
{
	DESCRIPTOR_DATA *d;
	char buf[MSL];
	
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}

	if (!ch->pcdata->in_progress->subject)
	{
		SEND("You need a subject first!\r\n",ch);
		return FALSE;
	}
	
	if (!ch->pcdata->in_progress->to_list)
	{
		SEND("You need a recipient first!\r\n",ch);
		return FALSE;
	}
	
	if (!ch->pcdata->in_progress->text)
	{
		SEND("You need a body first!\r\n",ch);
		return FALSE;
	}
	
	finish_note (ch->pcdata->board, ch->pcdata->in_progress);
	SEND ("Note posted.\r\n",ch);	
	ch->desc->editor = ED_NONE;
	ch->pcdata->in_progress = NULL;
	act ("{G$n finishes $s note.{x", ch, NULL, NULL, TO_ROOM);	
	
	for ( d = descriptor_list; d; d = d->next )
    {
		if ( d->connected == CON_PLAYING && d && d->character != ch )
		{
		
			sprintf(buf, "A new note has been added to the board: {G%s{x.\n\r", ch->pcdata->board->short_name );
			SEND( buf, d->character );
			//}
		}
    }
	return TRUE;	
}

NEDIT ( nedit_clear )
{
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}
	
	SEND ("Note cancelled!\r\n",ch);
	free_note (ch->pcdata->in_progress);
	ch->pcdata->in_progress = NULL;
	ch->desc->editor = ED_NONE;	
	
	return TRUE;
}


NEDIT ( nedit_forward )
{
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}

	return FALSE;
}

NEDIT ( nedit_list )
{
	int count= 0, show = 0, num = 0, has_shown = 0;
	time_t last_note;
	NOTE_DATA *p;
	char buf[MAX_STRING_LENGTH];
	
	
	if (is_number(argument))	 /* first, count the number of notes */
	{
		show = atoi(argument);
		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (is_note_to(ch,p))
			{
				count++;
				num++;
			}			
	}
	

	SEND ("{rNum: Author:      Subject:{x\r\n",ch);
	              
	last_note = ch->pcdata->last_note[board_number (ch->pcdata->board)];
	
	if (!str_cmp(argument, "new"))
	{
		for (p = ch->pcdata->board->note_first; p; p = p->next)
		{
			if (last_note <= p->date_stamp)
			{		
				num++;
				if (is_note_to(ch,p))
				{
					sprintf (buf, "%3d{r){B%c{x%-13s{x{y%s{x\r\n",
				               num, 
				               last_note < p->date_stamp ? '*' : ' ',
				               p->sender, p->subject);
					SEND (buf,ch);
				}
			}
			
			if (num < 1)
			SEND("You have no new notes on this board.\r\n",ch);
				              
		}		
		
		
		return TRUE;
	}
	
	for (p = ch->pcdata->board->note_first; p; p = p->next)
	{
		num++;
		if (is_note_to(ch,p))
		{
			has_shown++; /* note that we want to see X VISIBLE note, not just last X */
			if (show < 1 || ((count - show) < has_shown))
			{
				sprintf (buf, "%3d{r){B%c{x%-13s{x{y%s{x\r\n",
				               num, 
				               last_note < p->date_stamp ? '*' : ' ',
				               p->sender, p->subject);
				SEND (buf,ch);
			}
		}
		if (num < 1)
			SEND("You have no new notes on this board.\r\n",ch);
				              
	}
	
	return TRUE;
}

NEDIT ( nedit_type )
{
	if (!ch->pcdata->in_progress)
	{
		SEND("Start a new note first!\r\n",ch);
		return FALSE;
	}

	if (argument[0] == '\0')
	{
		SEND("Which type of note would you like to send?\r\n",ch);
		return FALSE;
	}
	
	if (!str_cmp (argument, "general") || !str_cmp (argument, "gen"))
	{
		ch->pcdata->in_progress->type = 0;		
		SEND ("Note type set.\r\n",ch);
		return TRUE;
	}
	
	 if (!str_cmp (argument, "idea") || !str_cmp (argument, "ideas"))
	{
		ch->pcdata->in_progress->type = 1;		
		SEND ("Note type set.\r\n",ch);
		return TRUE;
	}
	
	 if (!str_cmp (argument, "announce") || !str_cmp (argument, "ann"))
	{
		ch->pcdata->in_progress->type = 2;	
		SEND ("Note type set.\r\n",ch);
		return TRUE;		
	}
	
	 if (!str_cmp (argument, "bug") || !str_cmp (argument, "bugs"))
	{
		ch->pcdata->in_progress->type = 3;		
		SEND ("Note type set.\r\n",ch);
		return TRUE;
	}
	
	 if (!str_cmp (argument, "personal") || !str_cmp (argument, "pers"))
	{
		ch->pcdata->in_progress->type = 4;		
		SEND ("Note type set.\r\n",ch);
		return TRUE;
	}
	
	SEND ("Choose a proper note type to send!\r\n",ch);
	return FALSE;
}




QEDIT ( qedit_create )
{

	QUEST_INDEX_DATA *pQuest;
	AREA_DATA *pArea;
	int value;
	int iHash;

	
	value = atoi (argument);
    if (argument[0] == '\0' || value == 0)
    {
        SEND ("Syntax:  qedit create [vnum]\r\n", ch);
        return FALSE;
    }
	
	pArea = get_vnum_area (value);
	
	if (!pArea)
    {
        SEND ("QEdit:  That vnum is not assigned an area.\r\n", ch);
        return FALSE;
    }
	
	if (!IS_BUILDER (ch, pArea))
    {
        SEND ("QEdit:  Vnum is in an area you cannot build in.\r\n", ch);
        return FALSE;
    }

    if (get_quest_index (value))
    {
        SEND ("QEdit:  Quest vnum already exists.\r\n", ch);
        return FALSE;
    }
	
	pQuest = new_quest ();
	pQuest->area = pArea;
	pQuest->vnum = value;
	
    if (value > top_vnum_quest)
		top_vnum_quest = value;
    
	iHash = value % MAX_KEY_HASH;
    pQuest->next = quest_index_hash[iHash];
	quest_index_hash[iHash] = pQuest;
	
    ch->desc->pEdit = (void *) pQuest;	
    SEND ("Quest Created.\r\n", ch);    
	return TRUE;
}

QEDIT (qedit_show)
{
    QUEST_INDEX_DATA *pQuest;
    char buf[MSL];
   

    EDIT_QUEST (ch, pQuest);

    sprintf (buf, "Name:        [%s]\r\nArea:        [%5d] %s\r\n",
             pQuest->name,
             !pQuest->area ? -1 : pQuest->area->vnum,
             !pQuest->area ? "No Area" : pQuest->area->name);
    SEND (buf, ch);

    sprintf (buf, "Vnum:        [%5ld]\r\n",
             pQuest->vnum);
    SEND (buf, ch);

	sprintf (buf, "Type:        [%s]\r\n",
             flag_string (quest_types, pQuest->type));
	SEND (buf, ch);
	
	if (IS_SET(pQuest->type, B))
	{
		sprintf (buf, "Object Target:	[%ld]\r\n", pQuest->target_obj_vnum);
		SEND(buf, ch);
		sprintf (buf, "Object Multiples:[%d]\r\n", pQuest->obj_mult);
		SEND(buf, ch);		
	}
	if (IS_SET(pQuest->type, A))
	{
		sprintf (buf, "Mobile Target:	[%ld]\r\n", pQuest->target_mob_vnum);
		SEND(buf, ch);
		sprintf (buf, "Mobile Multiples:[%d]\r\n", pQuest->mob_mult);
		SEND(buf, ch);		
	}
	if (IS_SET(pQuest->type, E))
	{
		sprintf (buf, "Material Target:	[%d]\r\n", pQuest->target_mat_index);
		SEND(buf, ch);
		sprintf (buf, "Material Multiples:[%d]\r\n", pQuest->mat_mult);
		SEND(buf, ch);		
	}
	
	if (IS_SET(pQuest->type, D))
	{
		
		sprintf (buf, "Helpfile target 1:	[%s]\r\n", pQuest->help1);
		SEND(buf, ch);
		
		if (pQuest->help1)
		{
			sprintf (buf, "Helpfile target 2:	[%s]\r\n", pQuest->help2);
			SEND(buf, ch);
		}
		
		if (pQuest->help2)
		{
			sprintf (buf, "Helpfile target 3:	[%s]\r\n", pQuest->help3);
			SEND(buf, ch);
		}
	}
	//AREA EXPLORE
	
	sprintf (buf, "Class:       [%s]\r\n",	
             pQuest->ch_class_type == -1 ? "none" : ch_class_table[pQuest->ch_class_type].name);
	SEND (buf, ch);
	//-1 for guildmaster below.
    sprintf (buf,
             "Min Level:       [%2d]    Max Level:		[%2d]\nStart Mob: [%5ld]      	End Mob: [%5ld]\r\n",
             pQuest->min_level, pQuest->max_level, pQuest->accept_vnum,
             pQuest->return_vnum);
    SEND (buf, ch);
	
	SEND ("\r\n--Rewards --\r\n",ch);
	sprintf (buf,
			"Guild Points: [%4d]	Gold:	[%4d]		Experience:	[%5d]\r\n",
			pQuest->gp_reward, pQuest->gold_reward, pQuest->xp_reward);
	SEND (buf, ch);

	SEND ("\r\n--Chained From --\r\n",ch);
	int i;
	for (i = 0; i < 4; i++)
	{
		//sprintf (buf,"Quest vnum: [%6ld]\r\n", pQuest->quest_requisite[i]); SEND (buf, ch);
	}
	
	
	
	
	
	sprintf (buf, "\r\nDescription:\r\n%s", pQuest->desc);
    SEND (buf, ch);
	
	
/*
	char 	* desc;		//short description of quest.
	sh_int	rank;		//which guild rank do you get this quest at.
	int 	gp_reward; 	//guildpoint reward
	int		xp_reward; 	//xp reward
	int		gold_reward;//gold reward
	int		obj_reward;	//obj_reward vnum
	long 	type;		//what kind of quest is this? (flags)
	long	vnum;		//vnum of item/mob to kill, if applicable.	(Or can be the material index needed)
	sh_int  multiples;	//how many items to collect/mobs to kill/material pieces to turn in?	
	char 	* help1;	//first helpfile to read.
	char 	* help2;	//second helpfile to read.
	char 	* help3;	//third helpfile to read.
	
*/
    return FALSE;
}

QEDIT ( qedit_class )
{
	QUEST_INDEX_DATA *pQuest;
	int i = 0;
	
	EDIT_QUEST (ch, pQuest);	

	if ( !str_cmp(argument, "none" ) )
	{
		pQuest->ch_class_type = -1;
		SEND ("Quest class requirement set.\r\n", ch);
		return TRUE;
	}
	else	
	{
		for (i = 0; i < MAX_CLASS; i++)
		{
			if ( !str_cmp(argument,ch_class_table[i].name) )
			{
				pQuest->ch_class_type = i;
				SEND ("Quest class requirement set.\r\n", ch);
				break;
			}
		}
		return TRUE;	
	}

	SEND ("Syntax:class [class]\r\n"
                  "Classes being fighter, wizard, thief, cleric, etc.. (or none, free to all)\r\n"
						"It means that this quest can only be taken by that class.\r\n", ch);    
	return FALSE;
}

QEDIT (qedit_desc)
{
    QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0')
    {
        string_append (ch, &pQuest->desc);
        return TRUE;
    }

    SEND ("Syntax:  desc\r\n", ch);
    return FALSE;
}

QEDIT ( qedit_type )
{
	QUEST_INDEX_DATA *pQuest;
    int value;

    if (argument[0] != '\0')
    {
        EDIT_QUEST (ch, pQuest);		
		
        if ((value = flag_value (quest_types, argument)) != NO_FLAG)
        {
            pQuest->type ^= value;            

            SEND ("Quest type flag toggled.\r\n", ch);
            return TRUE;
        }
    }

    SEND ("Syntax: type [flag]\r\n"
                  "Valid quest flags are:\r\n", ch);
	show_help (ch, "qtype");
    return TRUE;
}

QEDIT ( qedit_name )
{
    QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0')
    {
        SEND ("Syntax:  name [string]\r\n", ch);
        return FALSE;
    }

    free_string (pQuest->name);
    pQuest->name = str_dup (argument);

    SEND ("Quest name set.\r\n", ch);
    return TRUE;
}

QEDIT ( qedit_start )
{
	
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  start [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 9999)
		pQuest->accept_vnum = 9999;	
	else if (atoi (argument) < 0)
		pQuest->accept_vnum = -1;
	else
		pQuest->accept_vnum = atoi(argument);
	SEND("Quest accept vnum set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_end )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  start [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 9999)
		pQuest->return_vnum = 9999;	
	else if (atoi (argument) < 0)
		pQuest->return_vnum = -1;
	else
		pQuest->return_vnum = atoi(argument);
	SEND("Quest return vnum set.\r\n",ch);
	return TRUE;
}



QEDIT ( qedit_gold_reward )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  gold [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 9999)
		pQuest->gold_reward = 9999;	
	else if (atoi (argument) < 0)
		pQuest->gold_reward = 0;
	else
		pQuest->gold_reward = atoi(argument);
	SEND("Quest gold reward set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_xp_reward )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  xp [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 9999)
		pQuest->xp_reward = 9999;	
	else if (atoi (argument) < 0)
		pQuest->xp_reward = 0;
	else
		pQuest->xp_reward = atoi(argument);
	SEND("Quest experience point reward set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_obj_multiples )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

	if (!IS_SET(pQuest->type, B)) //item retrieve
	{
		SEND("Make it an object retrieval quest first.\r\n",ch);
		return FALSE;
	}
	
    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  objmult [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 99)
		pQuest->obj_mult = 99;	
	else if (atoi (argument) < 1)
		pQuest->obj_mult = 1;
	else
		pQuest->obj_mult = atoi(argument);
	SEND("Quest object multiples set.\r\n",ch);
	return TRUE;	
}

QEDIT ( qedit_mob_multiples )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

	if (!IS_SET(pQuest->type, A)) //item retrieve
	{
		SEND("Make it a mob kill quest first.\r\n",ch);
		return FALSE;
	}
	
    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  mobmult [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 99)
		pQuest->mob_mult = 99;	
	else if (atoi (argument) < 1)
		pQuest->mob_mult = 1;
	else
		pQuest->mob_mult = atoi(argument);
	SEND("Quest mobile multiples set.\r\n",ch);
	return TRUE;	
}

QEDIT ( qedit_mat_multiples )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

	if (!IS_SET(pQuest->type, E)) //mat retrieve
	{
		SEND("Make it an material retrieval quest first.\r\n",ch);
		return FALSE;
	}
	
    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  matmult [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 99)
		pQuest->mat_mult = 99;	
	else if (atoi (argument) < 1)
		pQuest->mat_mult = 1;
	else
		pQuest->mat_mult = atoi(argument);
	SEND("Quest material multiples set.\r\n",ch);
	return TRUE;	
}

QEDIT ( qedit_herb_multiples )
{
/*	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

	if (!IS_SET(pQuest->type, B)) //item retrieve
	{
		SEND("Make it an object retrieval quest first.\r\n",ch);
		return FALSE;
	}
	
    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  objmult [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 99)
		pQuest->obj_mult = 99;	
	else if (atoi (argument) < 1)
		pQuest->obj_mult = 1;
	else
		pQuest->obj_mult = atoi(argument);
	SEND("Quest object multiples set.\r\n",ch);*/
	return TRUE;	
}



QEDIT ( qedit_gp_reward )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  gp [number]\r\n", ch);
        return FALSE;
    }
	if (atoi (argument) > 9999)
		pQuest->gp_reward = 9999;	
	else if (atoi (argument) < 0)
		pQuest->gp_reward = 0;
	else
		pQuest->gp_reward = atoi(argument);
	SEND("Quest guild point reward set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_objtarget )
{
	QUEST_INDEX_DATA *pQuest;
	OBJ_INDEX_DATA *pObj;
	long value;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  objtarget <vnum>\r\n", ch);
        return FALSE;
    }
    
	value = atol (argument);
	if (!(pObj = get_obj_index (value)))
	{
		SEND ("QEdit:  That object does not exist.\r\n", ch);
		return FALSE;
	}	
	
    pQuest->target_obj_vnum = atoi (argument);
	SEND("Quest obj target set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_mobtarget )
{
	QUEST_INDEX_DATA *pQuest;
	MOB_INDEX_DATA *pMob;
	long value;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  mobtarget [number]\r\n", ch);
        return FALSE;
    }
    
	value = atol (argument);
	if (!(pMob = get_mob_index (value)))
	{
		SEND ("QEdit:  That mobile does not exist.\r\n", ch);
		return FALSE;
	}	
	
    pQuest->target_mob_vnum = atoi (argument);
	SEND("Quest mob target set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_help1 )
{
	QUEST_INDEX_DATA *pQuest;
    EDIT_QUEST (ch, pQuest);
	HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];

	if (argument[0] == '\0')
	{
		SEND("Which helpfile will you tie it to?\r\n",ch);
        return FALSE;
	}
	
    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

	if (!IS_SET(pQuest->type, D))
	{
		SEND("Set it as a helpfile quest first.\r\n",ch);
		return FALSE;
	}
	
	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
		if (pHelp->level > 40)
			continue;
					
		if (is_name (argall, pHelp->keyword))
        {
			SEND("Helpfile Target set.\r\n",ch);
			pQuest->help1 = str_dup(argall);
			return TRUE;
		}
	}
	return FALSE;
}

QEDIT ( qedit_help2 )
{
	QUEST_INDEX_DATA *pQuest;
    EDIT_QUEST (ch, pQuest);
	HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
	
	if (argument[0] == '\0')
	{
		SEND("Which helpfile will you tie it to?\r\n",ch);
        return FALSE;
	}
	
    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

	if (!IS_SET(pQuest->type, D))
	{
		SEND("Set it as a helpfile quest first.\r\n",ch);
		return FALSE;
	}
	
	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
		if (pHelp->level > 40)
			continue;
					
		if (is_name (argall, pHelp->keyword))
        {
			SEND("Helpfile Target set.\r\n",ch);
			pQuest->help2 = str_dup(argall);
			return TRUE;
		}
	}
	return FALSE;
}

QEDIT ( qedit_help3 )
{
	QUEST_INDEX_DATA *pQuest;
    EDIT_QUEST (ch, pQuest);
	HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    
	
	if (argument[0] == '\0')
	{
		SEND("Which helpfile will you tie it to?\r\n",ch);
        return FALSE;
	}
	
    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

	if (!IS_SET(pQuest->type, D))
	{
		SEND("Set it as a helpfile quest first.\r\n",ch);
		return FALSE;
	}
	
	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
		if (pHelp->level > 40)
			continue;
					
		if (is_name (argall, pHelp->keyword))
        {
			SEND("Helpfile Target set.\r\n",ch);
			pQuest->help3 = str_dup(argall);
			return TRUE;
		}
	}
	return FALSE;
}


QEDIT ( qedit_mattarget )
{
	QUEST_INDEX_DATA *pQuest;
	char buf[MSL];
	int i = 0;
	
    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  mattarget [number]\r\n", ch);
        return FALSE;
    }

	if (argument[0] == '?')
	{
		SEND("Available Materials:\r\n",ch);
		for (i = 0; i < MAX_MATERIAL; i++)
		{
			sprintf(buf, "%s\r\n", mat_table[i].material);
			SEND(buf,ch);
		}
		return FALSE;
	}
	
	if (valid_material(argument))
	{
		pQuest->target_mat_index = atoi (argument);
		SEND("Quest material target set.\r\n",ch);
		return TRUE;	
	}
	return FALSE;
}

QEDIT ( qedit_min_level )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  minlevel [number]\r\n", ch);
        return FALSE;
    }

    pQuest->min_level = atoi (argument);
	SEND("Quest min level set.\r\n",ch);
	return TRUE;
}

QEDIT ( qedit_max_level )
{
	QUEST_INDEX_DATA *pQuest;

    EDIT_QUEST (ch, pQuest);

    if (argument[0] == '\0' || !is_number (argument))
    {
        SEND ("Syntax:  maxlevel [number]\r\n", ch);
        return FALSE;
    }

    pQuest->max_level = atoi (argument);
	SEND("Quest max level set.\r\n",ch);
	return TRUE;
}
