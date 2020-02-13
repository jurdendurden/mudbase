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
#else
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"

#if !defined(macintosh)
extern int _filbuf args ((FILE *));
#endif


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                    \
                if ( !str_cmp( word, literal ) )    \
                {                    \
                    field  = value;            \
                    fMatch = TRUE;            \
                    break;                \
                }

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )                    \
                if ( !str_cmp( word, literal ) )    \
                {                    \
                    free_string(field);            \
                    field  = value;            \
                    fMatch = TRUE;            \
                    break;                \
                }




GAME_DATA              game_data;

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST    100
static OBJ_DATA *rgObjNest[MAX_NEST];



/*
 * Local functions.
 */
void fwrite_char 		args ((CHAR_DATA * ch, FILE * fp));
void fwrite_account		args ((ACCOUNT_DATA * account));
ACCOUNT_DATA * fread_account		args ((char * name, DESCRIPTOR_DATA * d));
void fwrite_obj 		args ((CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest));
void fwrite_pet 		args ((CHAR_DATA * pet, FILE * fp));
void fread_char 		args ((CHAR_DATA * ch, FILE * fp));
void fread_pet 			args ((CHAR_DATA * ch, FILE * fp));
void fread_obj 			args ((CHAR_DATA * ch, FILE * fp, ACCOUNT_DATA *acc));
void fwrite_material	(CHAR_DATA *ch, FILE *fp);
void fwrite_factions	(CHAR_DATA *ch, FILE *fp);
void fread_material		(CHAR_DATA *ch, FILE *fp);
void fread_expertise    (CHAR_DATA *ch, FILE *fp);
void fwrite_expertise   args ((CHAR_DATA * ch, FILE * fp));
void fwrite_ship args 	((CHAR_DATA * ch, FILE * fp));
void fread_ship args 	((CHAR_DATA * ch, FILE * fp));
void fwrite_quests 		args((CHAR_DATA *ch, QUEST_DATA *list, FILE *fp));
void fread_quest 		args((CHAR_DATA *ch, FILE *fp));
void  save_game_data	args((void));
void  load_game_data	args((void));
void  fwrite_game_data	args( ( FILE *fp ) );
void  fread_game_data	args( ( FILE *fp ) );
void obj_to_vault args ((OBJ_DATA *obj, ACCOUNT_DATA * acc));


void fwrite_game_data( FILE *fp )
{
	fprintf( fp, "#GAME\n" );
	fprintf( fp, "MostOn %d\n", game_data.most_players );
	fprintf( fp, "Mkills %ld\n", game_data.mkills );
	fprintf( fp, "Mkiller %s~\n", game_data.mkiller );
	fprintf( fp, "Explored %ld\n", game_data.explored);
	fprintf( fp, "Explorer %s~\n", game_data.explorer);
	fprintf( fp, "MostHours %d\n", game_data.most_hours);
	fprintf( fp, "MostHour %s~\n", game_data.most_hour);
	fprintf( fp, "End\n\n");	
	return;
}

void fread_game_data( FILE * fp)
{
	char buf[MSL];
    const char *word;
    bool fMatch;
       
    sprintf(buf,"Loading Game Data Structure.");
    log_string(buf);
	
	game_data.mkills     = 0;
	game_data.mkiller    = str_dup("");
	
	game_data.most_hours = 0;
	game_data.most_hour  = str_dup("");
	
	game_data.explored   = 0;
	game_data.explorer   = str_dup("");
	
	game_data.most_players = 0;	
	
	for ( ; ; )
    {

		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		case '*':
			fMatch = TRUE;		
			fread_to_eol( fp );
			break;
		case 'E':
			KEY( "Explored", game_data.explored,   fread_number( fp ) );
            KEYS( "Explorer", game_data.explorer,   fread_string( fp ) );
			break;		
		case 'M': 
			KEY ( "MKills",  game_data.mkills,    fread_number( fp ) );
            KEYS( "MKiller", game_data.mkiller,   fread_string( fp ) );		
			
			KEY ("MostOn", game_data.most_players, fread_number (fp));
			
			KEY("MostHours", game_data.most_hours, fread_number(fp));
			KEYS("MostHour", game_data.most_hour,   fread_string( fp ) );				
			break;
		}
	}
	
	if ( !fMatch )
	{
		sprintf(buf,"Fread_game: no match for '%s'.", word);
		bug( buf, 0 );
		fread_to_eol( fp );
	}
}


void save_game_data( )
{
    char strsave[MAX_STRING_LENGTH];    
    FILE *fp;

    fclose( fpReserve );
    sprintf( strsave, "game_stats.txt" );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
		bug( "Save_game_data: fopen", 0 );
		perror( strsave );
    }
    else
    {
		fwrite_game_data( fp );
    }
    fprintf( fp, "#END\n\n" );
    fclose( fp );
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



void load_game_data( void )
{
    FILE *fp;
    bool found;

    found = FALSE;
    fclose( fpReserve );
       
    if ( ( fp = fopen( GAME_STATS_FILE, "r" ) ) != NULL )   
    {
		found = TRUE;
		for ( ; ; )
		{
			char letter;
			char *word;

			letter = fread_letter( fp );
			if ( letter == '*' )
			{
			fread_to_eol( fp );
			continue;
			}

			if ( letter != '#' )
			{
				bug( "Load_game_data: # not found.", 0 );
			break;
			}

				word = fread_word( fp );
				if ( !str_cmp( word, "GAME" ) ) 
					fread_game_data( fp );    
				else if ( !str_cmp( word, "END" ) ) 
					break;
				else
				{
					bug( "Load_game_data: bad section.", 0 );
			break;
			}
		}
		fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}




	
/* int rename(const char *oldfname, const char *newfname); viene en stdio.h */

char *print_flags (int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32; count++)
    {
        if (IS_SET (flag, 1 << count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj (CHAR_DATA * ch)
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if (IS_NPC (ch))
        return;

	/*
	 * Fix by Edwin. JR -- 10/15/00
	 *
	 * Don't save if the character is invalidated.
	 * This might happen during the auto-logoff of players.
	 * (or other places not yet found out)
	 */
	if ( !IS_VALID(ch)) {
    	bug("save_char_obj: Trying to save an invalidated character.\n",0);
    	return;
	}

    if (ch->desc != NULL && ch->desc->original != NULL)
        ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL (ch) || ch->level >= LEVEL_IMMORTAL)
    {
        fclose (fpReserve);
        sprintf (strsave, "%s%s", GOD_DIR, capitalize (ch->name));
        if ((fp = fopen (strsave, "w")) == NULL)
        {
            bug ("Save_char_obj: fopen", 0);
            perror (strsave);
        }

        fprintf (fp, "Lev %2d Trust %2d  %s%s\n",
                 ch->level, get_trust (ch), ch->name, ch->pcdata->title);
        fclose (fp);
        fpReserve = fopen (NULL_FILE, "r");
    }
#endif

    fclose (fpReserve);
    sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->name));
    if ((fp = fopen (TEMP_FILE, "w")) == NULL)
    {
        bug ("Save_char_obj: fopen", 0);
        perror (strsave);
    }
    else
    {
        fwrite_char (ch, fp);
		
		if(!IS_NPC(ch))
			fwrite_material(ch,fp);		
		
		if (!IS_NPC(ch))
			fwrite_factions(ch, fp);
		
		if (!IS_NPC(ch))
			fwrite_expertise(ch, fp);
			
		//if (ch->quests)
		//	fwrite_quests (ch, ch->quests, fp);		
		//if(!IS_NPC(ch) && ch->ship);
		//	fwrite_ship(ch, fp);
        if (ch->carrying != NULL)
            fwrite_obj (ch, ch->carrying, fp, 0);
        /* save the pets */
        if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
            fwrite_pet (ch->pet, fp);
        fprintf (fp, "#END\n");
    }
    fclose (fp);
    rename (TEMP_FILE, strsave);
    fpReserve = fopen (NULL_FILE, "r");
    return;
}




/*
 * Write the char.
 */
void fwrite_char (CHAR_DATA * ch, FILE * fp)
{
    AFFECT_DATA *paf;
    int sn, gn, pos, i;

    fprintf (fp, "#%s\n", IS_NPC (ch) ? "MOB" : "PLAYER");

    fprintf (fp, "Name %s~\n", ch->name);	
    fprintf (fp, "Id   %ld\n", ch->id);
    fprintf (fp, "LogO %ld\n", current_time);
    fprintf (fp, "Vers %d\n", CURR_PC_VERSION);	
	fprintf (fp, "Blind %d\n", ch->blind);	
	fprintf (fp, "Boon %d\n", ch->boon);
    if (ch->short_descr[0] != '\0')
        fprintf (fp, "ShD  %s~\n", ch->short_descr);
    if (ch->long_descr[0] != '\0')
        fprintf (fp, "LnD  %s~\n", ch->long_descr);
	if (!IS_NULLSTR(ch->walk_desc))
        fprintf (fp, "WalkDesc  %s~\n", ch->walk_desc);
    if (ch->description[0] != '\0')
        fprintf (fp, "Desc %s~\n", ch->description);
    if (ch->prompt != NULL || !str_cmp (ch->prompt, "<%hhp %mm %vmv> ")
        || !str_cmp (ch->prompt, "{c<%hhp %mm %vmv>{x "))
        fprintf (fp, "Prom %s~\n", ch->prompt);
    fprintf (fp, "Race %s~\n", pc_race_table[ch->race].name);
    if (ch->clan)
	{
        fprintf (fp, "Clan %s~\n", clan_table[ch->clan].name);
		fprintf (fp, "Clan_Rank %d\n", ch->clan_rank);
		fprintf (fp, "Clan_Points %d\n", ch->clan_points);
	}
    fprintf (fp, "Sex  %d\n", ch->sex);
    fprintf (fp, "Cla  %d\n", ch-> ch_class);
	fprintf (fp, "God	%d\n", ch->god);
	fprintf (fp, "Favored %d\n", ch->favored_enemy);
	fprintf (fp, "GuildRnk  %d\n", ch->guildrank);
	fprintf (fp, "GuildPts  %d\n", ch->guildpoints);
	if (ch->mClass == TRUE)
	{
		fprintf (fp, "mClass %d\n", ch->mClass);
		fprintf (fp, "Class2 %d\n", ch->ch_class2);
		fprintf (fp, "Level2 %d\n", ch->level2);
		fprintf (fp, "Exp2   %ld\n", ch->exp2);		
	}		
	//Upro's elemental specialization code 2/1/07
	if (IS_WIZARD(ch) && ch->pcdata->elementalSpec != 0)
	{
		fprintf( fp, "ElemSpec   %d\n", ch->pcdata->elementalSpec	);
	}
	//
	//Fighter weapon spec. Upro 10/25/2009
	if (IS_FIGHTER(ch) && ch->pcdata->weaponSpec != 0)
	{
		fprintf( fp, "WpnSpec	%d\n", ch->pcdata->weaponSpec	);
	}
	
    fprintf (fp, "Levl %d\n", ch->level);
    if (ch->trust != 0)
        fprintf (fp, "Tru  %d\n", ch->trust);
    fprintf (fp, "Sec  %d\n", ch->pcdata->security);    /* OLC */
    fprintf (fp, "Plyd %d\n", ch->played + (int) (current_time - ch->logon));
	fprintf (fp, "Email %s~\n", ch->email);
	fprintf (fp, "Citizen %s~\n", ch->citizen);
    fprintf (fp, "Scro %d\n", ch->lines);
	if (ch->onBoard)
	{
		ch->in_room = ch->ship->in_room;
	}
    fprintf (fp, "Room %ld\n", (ch->in_room == get_room_index (ROOM_VNUM_LIMBO)
                               && ch->was_in_room != NULL)
             ? ch->was_in_room->vnum
             : ch->in_room == NULL ? 3700 : ch->in_room->vnum);
    fprintf (fp, "HMV  %d %d %d %d %d %d\n",
             ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
             ch->max_move);	
	fprintf (fp, "Bleed %d\n", ch->bleeding);
	fprintf( fp, "Talent %d\n", ch->has_talent);
	fprintf( fp, "PP %d %d\n", ch->pp, ch->max_pp );
    if (ch->gold > 0)
        fprintf (fp, "Gold %ld\n", ch->gold);
    else
        fprintf (fp, "Gold %d\n", 0);
    if (ch->silver > 0)
        fprintf (fp, "Silv %ld\n", ch->silver);
    else
        fprintf (fp, "Silv %d\n", 0);
	fprintf( fp, "BAcct %d\n", ch->bank_act	);
    fprintf( fp, "Bank %ld\n",ch->bank_amt     );
    fprintf (fp, "Exp  %ld\n", ch->exp);	
	fprintf (fp, "NoExp1 %d\n", ch->no_exp[0]);
	fprintf (fp, "NoExp2 %d\n", ch->no_exp[1]);
	//pkills/pdeaths/mobkills:
	fprintf (fp, "Pkills %d\n", ch->pkill );
	fprintf (fp, "Pdeath %d\n", ch->pdeath );
	fprintf (fp, "Mkills %d\n", ch->mkill );
	//characteristics:
	if (ch->eyes != 0)
		fprintf (fp, "Eyes %ld\n", ch->eyes);
	if (ch->hair != 0)
		fprintf (fp, "Hair %ld\n", ch->hair);
	if (ch->haircut != 0)
		fprintf (fp, "Haircut %ld\n", ch->haircut);
	if (ch->facial_hair != 0)
		fprintf (fp, "Facial %ld\n", ch->facial_hair);
	
	if (ch->last_name)
		fprintf (fp, "LastName %s~\n", ch->last_name);
	
	if (ch->age)
		fprintf(fp, "Age %d\n", GET_AGE(ch));
	
	if (ch->faction > -1)
		fprintf (fp, "Faction %d\n", ch->faction);
	
	if (ch->faction_invite > -1)
		fprintf (fp, "Invite %d\n", ch->faction_invite);

	if (ch->society_rank > -1)
		fprintf (fp, "Society %d\n", ch->society_rank);
	
	if ( !IS_NULLSTR(ch->account))
		fprintf( fp, "Account %s~\n",	ch->account );
    else if ( ch->desc && ch->desc->account && !IS_NULLSTR(ch->desc->account->name))
		fprintf( fp, "Account %s~\n",	ch->desc->account->name );

	
	fprintf(fp, "Recall %ld\n", ch->pcdata->recall);
	
    if (ch->act != 0)
        fprintf (fp, "Act  %s\n", print_flags (ch->act));
    if (ch->affected_by != 0)
        fprintf (fp, "AfBy %s\n", print_flags (ch->affected_by));
    fprintf (fp, "Comm %s\n", print_flags (ch->comm));
    if (ch->wiznet)
        fprintf (fp, "Wizn %s\n", print_flags (ch->wiznet));
    if (ch->invis_level)
        fprintf (fp, "Invi %d\n", ch->invis_level);
    if (ch->incog_level)
        fprintf (fp, "Inco %d\n", ch->incog_level);
    fprintf (fp, "Pos  %d\n",
             ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
    if (ch->practice != 0)
        fprintf (fp, "Prac %d\n", ch->practice);
    if (ch->train != 0)
        fprintf (fp, "Trai %d\n", ch->train);
    if (ch->saving_throw != 0)
        fprintf (fp, "Save  %d\n", ch->saving_throw);
    fprintf (fp, "Alig  %d\n", ch->alignment);
    if (ch->hitroll != 0)
        fprintf (fp, "Hit   %d\n", ch->hitroll);
    if (ch->damroll != 0)
        fprintf (fp, "Dam   %d\n", ch->damroll);
    fprintf (fp, "ACs %d %d %d %d\n",
             ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3]);
    if (ch->wimpy != 0)
        fprintf (fp, "Wimp  %d\n", ch->wimpy);
    fprintf (fp, "Attr %d %d %d %d %d %d\n",
             ch->perm_stat[STAT_STR],
             ch->perm_stat[STAT_INT],
             ch->perm_stat[STAT_WIS],
             ch->perm_stat[STAT_DEX], 
			 ch->perm_stat[STAT_CON],
			 ch->perm_stat[STAT_CHA]);

    fprintf (fp, "AMod %d %d %d %d %d %d\n",
             ch->mod_stat[STAT_STR],
             ch->mod_stat[STAT_INT],
             ch->mod_stat[STAT_WIS],
             ch->mod_stat[STAT_DEX], 
			 ch->mod_stat[STAT_CON],
			 ch->mod_stat[STAT_CHA]);

    if (IS_NPC (ch))
    {
        fprintf (fp, "Vnum %ld\n", ch->pIndexData->vnum);
    }
    else
    {
        fprintf (fp, "Pass %s~\n", ch->pcdata->pwd);
        if (ch->pcdata->bamfin[0] != '\0')
            fprintf (fp, "Bin  %s~\n", ch->pcdata->bamfin);
        if (ch->pcdata->bamfout[0] != '\0')
            fprintf (fp, "Bout %s~\n", ch->pcdata->bamfout);
        fprintf (fp, "Titl %s~\n", ch->pcdata->title);
		fprintf (fp, "PreT %s~\n", ch->pcdata->pretitle);
		fprintf (fp, "Cond	%d\n", ch->pcdata->item_condition);
		fprintf (fp, "EqCond %d\n", ch->pcdata->eqcondition);
		fprintf (fp, "Beeptells %d\n", ch->pcdata->beeptells);
        fprintf (fp, "Pnts %d\n", ch->pcdata->points);
        fprintf (fp, "TSex %d\n", ch->pcdata->true_sex);
        fprintf (fp, "LLev %d\n", ch->pcdata->last_level);
        fprintf (fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
                 ch->pcdata->perm_mana, ch->pcdata->perm_move);
        fprintf (fp, "Cnd  %d %d %d %d %d\n",
                 ch->pcdata->condition[0],
                 ch->pcdata->condition[1],
                 ch->pcdata->condition[2], ch->pcdata->condition[3], ch->pcdata->condition[4]);

	
        /*
         * Write Colour Config Information.
         */
        fprintf (fp, "Coloura     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->text[2],
                 ch->pcdata->text[0],
                 ch->pcdata->text[1],
                 ch->pcdata->auction[2],
                 ch->pcdata->auction[0],
                 ch->pcdata->auction[1],
                 ch->pcdata->gossip[2],
                 ch->pcdata->gossip[0],
                 ch->pcdata->gossip[1],
                 ch->pcdata->music[2],
                 ch->pcdata->music[0],
                 ch->pcdata->music[1],
                 ch->pcdata->question[2],
                 ch->pcdata->question[0], ch->pcdata->question[1]);
        fprintf (fp, "Colourb     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->answer[2],
                 ch->pcdata->answer[0],
                 ch->pcdata->answer[1],
                 ch->pcdata->quote[2],
                 ch->pcdata->quote[0],
                 ch->pcdata->quote[1],
                 ch->pcdata->quote_text[2],
                 ch->pcdata->quote_text[0],
                 ch->pcdata->quote_text[1],
                 ch->pcdata->immtalk_text[2],
                 ch->pcdata->immtalk_text[0],
                 ch->pcdata->immtalk_text[1],
                 ch->pcdata->immtalk_type[2],
                 ch->pcdata->immtalk_type[0], ch->pcdata->immtalk_type[1]);
        fprintf (fp, "Colourc     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->info[2],
                 ch->pcdata->info[0],
                 ch->pcdata->info[1],
                 ch->pcdata->tell[2],
                 ch->pcdata->tell[0],
                 ch->pcdata->tell[1],
                 ch->pcdata->reply[2],
                 ch->pcdata->reply[0],
                 ch->pcdata->reply[1],
                 ch->pcdata->gtell_text[2],
                 ch->pcdata->gtell_text[0],
                 ch->pcdata->gtell_text[1],
                 ch->pcdata->gtell_type[2],
                 ch->pcdata->gtell_type[0], ch->pcdata->gtell_type[1]);
        fprintf (fp, "Colourd     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->room_title[2],
                 ch->pcdata->room_title[0],
                 ch->pcdata->room_title[1],
                 ch->pcdata->room_text[2],
                 ch->pcdata->room_text[0],
                 ch->pcdata->room_text[1],
                 ch->pcdata->room_exits[2],
                 ch->pcdata->room_exits[0],
                 ch->pcdata->room_exits[1],
                 ch->pcdata->room_things[2],
                 ch->pcdata->room_things[0],
                 ch->pcdata->room_things[1],
                 ch->pcdata->prompt[2],
                 ch->pcdata->prompt[0], ch->pcdata->prompt[1]);
        fprintf (fp, "Coloure     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->fight_death[2],
                 ch->pcdata->fight_death[0],
                 ch->pcdata->fight_death[1],
                 ch->pcdata->fight_yhit[2],
                 ch->pcdata->fight_yhit[0],
                 ch->pcdata->fight_yhit[1],
                 ch->pcdata->fight_ohit[2],
                 ch->pcdata->fight_ohit[0],
                 ch->pcdata->fight_ohit[1],
                 ch->pcdata->fight_thit[2],
                 ch->pcdata->fight_thit[0],
                 ch->pcdata->fight_thit[1],
                 ch->pcdata->fight_skill[2],
                 ch->pcdata->fight_skill[0], ch->pcdata->fight_skill[1]);
        fprintf (fp, "Colourf     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->wiznet[2],
                 ch->pcdata->wiznet[0],
                 ch->pcdata->wiznet[1],
                 ch->pcdata->say[2],
                 ch->pcdata->say[0],
                 ch->pcdata->say[1],
                 ch->pcdata->say_text[2],
                 ch->pcdata->say_text[0],
                 ch->pcdata->say_text[1],
                 ch->pcdata->tell_text[2],
                 ch->pcdata->tell_text[0],
                 ch->pcdata->tell_text[1],
                 ch->pcdata->reply_text[2],
                 ch->pcdata->reply_text[0], ch->pcdata->reply_text[1]);
        fprintf (fp, "Colourg     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->auction_text[2],
                 ch->pcdata->auction_text[0],
                 ch->pcdata->auction_text[1],
                 ch->pcdata->gossip_text[2],
                 ch->pcdata->gossip_text[0],
                 ch->pcdata->gossip_text[1],
                 ch->pcdata->music_text[2],
                 ch->pcdata->music_text[0],
                 ch->pcdata->music_text[1],
                 ch->pcdata->question_text[2],
                 ch->pcdata->question_text[0],
                 ch->pcdata->question_text[1],
                 ch->pcdata->answer_text[2],
                 ch->pcdata->answer_text[0], ch->pcdata->answer_text[1]);

		if(ch->pcdata->explored->set > 0 )
		{	EXPLORE_HOLDER *pExp;
	 
			fprintf(fp, "Explored %d\n", ch->pcdata->explored->set);
			for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
				fprintf(fp, "%d %d\n", pExp->mask, pExp->bits );
			fprintf(fp, "-1 -1\n" );
		}
				 
        /* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
        {
            if (ch->pcdata->alias[pos] == NULL
                || ch->pcdata->alias_sub[pos] == NULL)
                break;

            fprintf (fp, "Alias %s %s~\n", ch->pcdata->alias[pos],
                     ch->pcdata->alias_sub[pos]);
        }

		/* Save note board status */
		/* Save number of boards in case that number changes */
		fprintf (fp, "Boards       %d ", MAX_BOARD);
		for (i = 0; i < MAX_BOARD; i++) 
			fprintf (fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
		fprintf (fp, "\n");

        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
            {
                fprintf (fp, "Sk %d '%s'\n",
                         ch->pcdata->learned[sn], skill_table[sn].name);
            }
        }

        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name != NULL && ch->pcdata->group_known[gn])
            {
                fprintf (fp, "Gr '%s'\n", group_table[gn].name);
            }
        }
    }

    for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
        if (paf->type < 0 || paf->type >= MAX_SKILL)
            continue;

        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
                 skill_table[paf->type].name,
                 paf->where,
                 paf->level,
                 paf->duration, paf->modifier, paf->location, paf->bitvector);
    }
    fprintf (fp, "End\n\n");
    return;
}

/* write a pet */
void fwrite_pet (CHAR_DATA * pet, FILE * fp)
{
    AFFECT_DATA *paf;

    fprintf (fp, "#PET\n");

    fprintf (fp, "Vnum %ld\n", pet->pIndexData->vnum);

    fprintf (fp, "Name %s~\n", pet->name);
    fprintf (fp, "LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
        fprintf (fp, "ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
        fprintf (fp, "LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
        fprintf (fp, "Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
        fprintf (fp, "Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf (fp, "Clan %s~\n", clan_table[pet->clan].name);
    fprintf (fp, "Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
        fprintf (fp, "Levl %d\n", pet->level);
    fprintf (fp, "HMV  %d %d %d %d %d %d\n",
             pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move,
             pet->max_move);
    if (pet->gold > 0)
        fprintf (fp, "Gold %ld\n", pet->gold);
    if (pet->silver > 0)
        fprintf (fp, "Silv %ld\n", pet->silver);
    if (pet->exp > 0)
        fprintf (fp, "Exp  %ld\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
        fprintf (fp, "Act  %s\n", print_flags (pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
        fprintf (fp, "AfBy %s\n", print_flags (pet->affected_by));
    if (pet->comm != 0)
        fprintf (fp, "Comm %s\n", print_flags (pet->comm));
    fprintf (fp, "Pos  %d\n", pet->position =
             POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
        fprintf (fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
        fprintf (fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
        fprintf (fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
        fprintf (fp, "Dam  %d\n", pet->damroll);
    fprintf (fp, "ACs  %d %d %d %d\n",
             pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);
    fprintf (fp, "Attr %d %d %d %d %d\n",
             pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
             pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
             pet->perm_stat[STAT_CON]);
    fprintf (fp, "AMod %d %d %d %d %d\n",
             pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
             pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
             pet->mod_stat[STAT_CON]);

    for (paf = pet->affected; paf != NULL; paf = paf->next)
    {
        if (paf->type < 0 || paf->type >= MAX_SKILL)
            continue;

        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
                 skill_table[paf->type].name,
                 paf->where, paf->level, paf->duration, paf->modifier,
                 paf->location, paf->bitvector);
    }

    fprintf (fp, "End\n");
    return;
}

void fread_quest(CHAR_DATA *ch, FILE *fp)
{
    QUEST_DATA *quest;
    char *word;
    bool fMatch;
    bool fVnum;
    bool first;
	long vnum = 0;

    fVnum = FALSE;
    quest = NULL;
    first = TRUE;                /* used to counter fp offset */

    word = feof (fp) ? "End" : fread_word (fp);
    if (!str_cmp (word, "Vnum"))
    {
        
        first = FALSE;            /* fp will be in right place */

        vnum = fread_number (fp);
        if (get_quest_index (vnum) == NULL)
        {
            bug ("Fread_quest: bad quest vnum %ld.", vnum);
        }
        else
            quest = create_quest (get_quest_index (vnum));
    }

    if (quest == NULL)
    {                            /* either not found or old style */
        quest = new_pc_quest ();
        quest->name = str_dup ("");
        //quest->short_descr = str_dup ("");
        //quest->description = str_dup ("");
    }

	quest_to_char(quest, ch);
	
    fVnum = TRUE;

    for (;;)
    {
        if (first)
            first = FALSE;
        else
            word = feof (fp) ? "End" : fread_word (fp);
        fMatch = FALSE;

        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;
            case 'C':
				if (!str_cmp (word, "Completed"))
				{
					quest->completed = fread_number(fp);
					break;
				}
                break;
			case 'E':
				if (!str_cmp (word, "End"))
                {
                    if (fVnum && quest->pIndexData == NULL)
                    {
                        bug ("Fread_quest: incomplete quest.", 0);
                        free_pc_quest (quest);
						fclose(fp);
                        return;
                    }
                    else
                    {
                        if (!fVnum)
                        {
                            free_pc_quest (quest);
                            quest = create_quest (get_quest_index (vnum));
                        }
						quest = create_quest (quest->pIndexData);
						quest_to_char (quest, ch);                        
						fclose(fp);
                        return;
                    }
                }
                break;
			case 'H':
				if (!str_cmp (word, "HasRead1"))
				{
					quest->has_read_helps[0] = fread_number(fp);
					break;
				}
				if (!str_cmp (word, "HasRead2"))
				{
					quest->has_read_helps[1] = fread_number(fp);
					break;
				}
				if (!str_cmp (word, "HasRead3"))
				{
					quest->has_read_helps[2] = fread_number(fp);
					break;
				}
				break;
			case 'M':
				if (!str_cmp (word, "MobMult"))
				{
					quest->mob_multiples = fread_number(fp);
					break;
				}
				if (!str_cmp (word, "MatMult"))
				{
					quest->mat_multiples = fread_number(fp);
					break;
				}
				break;
			case 'N':
				if (!str_cmp (word, "Name"))
				{
					quest->name = fread_string(fp);
					break;
				}
				break;
			case 'O':
				if (!str_cmp (word, "ObjMult"))				
				{
					quest->obj_multiples = fread_number(fp);
					break;
				}
				break;
				
		}
		
		if (!fMatch)
		{
			bug ("Fread_obj: no match.", 0);
			fread_to_eol (fp);
		}
	}

	fclose(fp);
	return;
}


void fwrite_quests (CHAR_DATA * ch, QUEST_DATA *list, FILE * fp)
{	
	QUEST_DATA *quest;
	//int i = 0;
	
	fprintf(fp,"#QUEST\n");
	
	for (quest = list; quest != NULL; quest = list->next_quest)
	{					
		if (quest->pIndexData != NULL)
		{
			fprintf (fp, "Vnum %ld\n", quest->pIndexData->vnum);
			fprintf (fp, "Name %s~\n", quest->pIndexData->name);		
			fprintf (fp, "ObjMult %d\n", quest->obj_multiples);
			fprintf (fp, "MobMult %d\n", quest->mob_multiples);
			fprintf (fp, "MatMult %d\n", quest->mat_multiples);	
			fprintf (fp, "HerbMult %d\n", quest->herb_multiples);
			fprintf (fp, "Completed %d\n", quest->completed);
			
			
			
			fprintf (fp, "HasRead1 %d\n", quest->has_read_helps[0]);
			fprintf (fp, "HasRead2 %d\n", quest->has_read_helps[1]);
			fprintf (fp, "HasRead3 %d\n", quest->has_read_helps[2]);		
		}
	}
	
	fprintf (fp, "\nEnd\n\n");	
	return;
}



void fwrite_vault_obj (ACCOUNT_DATA *acc, OBJ_DATA * obj, FILE * fp)
{
	AFFECT_DATA *paf;
	EXTRA_DESCR_DATA *ed;
	
	if (obj->next_content != NULL)
        fwrite_vault_obj (acc, obj->next_content, fp);
	
	//No containers, this can lead to basically an endless storage for them.
	if (obj->item_type == ITEM_CONTAINER)
		return;
	
	fprintf (fp, "#O\n");
    fprintf (fp, "Vnum %ld\n", obj->pIndexData->vnum);
    if (!obj->pIndexData->new_format)
        fprintf (fp, "Oldstyle\n");
    if (obj->enchanted)
        fprintf (fp, "Enchanted\n");
    //fprintf (fp, "Nest %d\n", iNest);

    /* these data are only used if they do not match the defaults */

    if (obj->name != obj->pIndexData->name)
        fprintf (fp, "Name %s~\n", obj->name);
    if (obj->short_descr != obj->pIndexData->short_descr)
        fprintf (fp, "ShD  %s~\n", obj->short_descr);
    if (obj->description != obj->pIndexData->description)
        fprintf (fp, "Desc %s~\n", obj->description);
    if (obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf (fp, "ExtF %d\n", obj->extra_flags);
	if (obj->extra2_flags != obj->pIndexData->extra2_flags)
        fprintf (fp, "Extra2Flags %d\n", obj->extra2_flags);
    if (obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf (fp, "WeaF %d\n", obj->wear_flags);
    if (obj->item_type != obj->pIndexData->item_type)
        fprintf (fp, "Ityp %d\n", obj->item_type);
    if (obj->weight != obj->pIndexData->weight)
        fprintf (fp, "Wt   %d\n", obj->weight);
    if (obj->condition != obj->pIndexData->condition)
        fprintf (fp, "Cond %d\n", obj->condition);
	if (obj->material != obj->pIndexData->material)
		fprintf (fp, "Material %s~\n", obj->material);

    /* variable data */

    fprintf (fp, "Wear %d\n", obj->wear_loc);
    if (obj->level != obj->pIndexData->level)
        fprintf (fp, "Lev  %d\n", obj->level);
    if (obj->timer != 0)
        fprintf (fp, "Time %d\n", obj->timer);
    fprintf (fp, "Cost %d\n", obj->cost);
    if (obj->value[0] != obj->pIndexData->value[0]
        || obj->value[1] != obj->pIndexData->value[1]
        || obj->value[2] != obj->pIndexData->value[2]
        || obj->value[3] != obj->pIndexData->value[3]
        || obj->value[4] != obj->pIndexData->value[4])
        fprintf (fp, "Val  %d %d %d %d %d\n",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                 obj->value[4]);

    switch (obj->item_type)
    {
        case ITEM_POTION:
        case ITEM_SCROLL:
        case ITEM_PILL:
            if (obj->value[1] > 0)
            {
                fprintf (fp, "Spell 1 '%s'\n",
                         skill_table[obj->value[1]].name);
            }

            if (obj->value[2] > 0)
            {
                fprintf (fp, "Spell 2 '%s'\n",
                         skill_table[obj->value[2]].name);
            }

            if (obj->value[3] > 0)
            {
                fprintf (fp, "Spell 3 '%s'\n",
                         skill_table[obj->value[3]].name);
            }

            break;

        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->value[3] > 0)
            {
                fprintf (fp, "Spell 3 '%s'\n",
                         skill_table[obj->value[3]].name);
            }

            break;
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        if (paf->type < 0 || paf->type >= MAX_SKILL)
            continue;
        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
                 skill_table[paf->type].name,
                 paf->where,
                 paf->level,
                 paf->duration, paf->modifier, paf->location, paf->bitvector);
    }

    for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
        fprintf (fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
    }

    fprintf (fp, "End\n\n");
	
	return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if (obj->next_content != NULL)
        fwrite_obj (ch, obj->next_content, fp, iNest);

    /*
     * Castrate storage characters.
     */
    if ((total_levels(ch) < obj->level - 2 && obj->item_type != ITEM_CONTAINER && obj->item_type != ITEM_SCROLL)
        || obj->item_type == ITEM_KEY
        || (obj->item_type == ITEM_MAP && !obj->value[0]))
        return;

    fprintf (fp, "#O\n");
    fprintf (fp, "Vnum %ld\n", obj->pIndexData->vnum);
    if (!obj->pIndexData->new_format)
        fprintf (fp, "Oldstyle\n");
    if (obj->enchanted)
        fprintf (fp, "Enchanted\n");
    fprintf (fp, "Nest %d\n", iNest);

    /* these data are only used if they do not match the defaults */

    if (obj->name != obj->pIndexData->name)
        fprintf (fp, "Name %s~\n", obj->name);
    if (obj->short_descr != obj->pIndexData->short_descr)
        fprintf (fp, "ShD  %s~\n", obj->short_descr);
    if (obj->description != obj->pIndexData->description)
        fprintf (fp, "Desc %s~\n", obj->description);
    if (obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf (fp, "ExtF %d\n", obj->extra_flags);
	if (obj->extra2_flags != obj->pIndexData->extra2_flags)
        fprintf (fp, "Extra2Flags %d\n", obj->extra2_flags);
    if (obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf (fp, "WeaF %d\n", obj->wear_flags);
    if (obj->item_type != obj->pIndexData->item_type)
        fprintf (fp, "Ityp %d\n", obj->item_type);
    if (obj->weight != obj->pIndexData->weight)
        fprintf (fp, "Wt   %d\n", obj->weight);
    if (obj->condition != obj->pIndexData->condition)
        fprintf (fp, "Cond %d\n", obj->condition);
	if (obj->material != obj->pIndexData->material)
		fprintf (fp, "Material %s~\n", obj->material);

    /* variable data */

    fprintf (fp, "Wear %d\n", obj->wear_loc);
    if (obj->level != obj->pIndexData->level)
        fprintf (fp, "Lev  %d\n", obj->level);
    if (obj->timer != 0)
        fprintf (fp, "Time %d\n", obj->timer);
    fprintf (fp, "Cost %d\n", obj->cost);
    if (obj->value[0] != obj->pIndexData->value[0]
        || obj->value[1] != obj->pIndexData->value[1]
        || obj->value[2] != obj->pIndexData->value[2]
        || obj->value[3] != obj->pIndexData->value[3]
        || obj->value[4] != obj->pIndexData->value[4])
        fprintf (fp, "Val  %d %d %d %d %d\n",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                 obj->value[4]);

    switch (obj->item_type)
    {
        case ITEM_POTION:
        case ITEM_SCROLL:
        case ITEM_PILL:
            if (obj->value[1] > 0)
            {
                fprintf (fp, "Spell 1 '%s'\n",
                         skill_table[obj->value[1]].name);
            }

            if (obj->value[2] > 0)
            {
                fprintf (fp, "Spell 2 '%s'\n",
                         skill_table[obj->value[2]].name);
            }

            if (obj->value[3] > 0)
            {
                fprintf (fp, "Spell 3 '%s'\n",
                         skill_table[obj->value[3]].name);
            }

            break;

        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->value[3] > 0)
            {
                fprintf (fp, "Spell 3 '%s'\n",
                         skill_table[obj->value[3]].name);
            }

            break;
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
        if (paf->type < 0 || paf->type >= MAX_SKILL)
            continue;
        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
                 skill_table[paf->type].name,
                 paf->where,
                 paf->level,
                 paf->duration, paf->modifier, paf->location, paf->bitvector);
    }

    for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
        fprintf (fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
    }

    fprintf (fp, "End\n\n");

    if (obj->contains != NULL)
        fwrite_obj (ch, obj->contains, fp, iNest + 1);

    return;
}


void fwrite_factions (CHAR_DATA *ch, FILE *fp)
{
	int i;
	
	fprintf(fp, "#FACTIONS\n");
	for ( i = 0; i < MAX_FACTION; i++ )
	{
		fprintf (fp, "%d ", ch->faction_rep[i]);
		fprintf (fp, "%d\n", ch->faction_rank[i]);
	}
	fprintf (fp, "End\n\n");
	return;
}

void fread_factions (CHAR_DATA *ch, FILE*fp)
{	
    char buf[100];
	int i;

    for(i = 0; i < MAX_FACTION; i++ )
	{
		ch->faction_rep[i] =			fread_number(fp);
		ch->faction_rank[i] = 			fread_number(fp);		
    }
    const char * word = feof( fp ) ? "End" : fread_word( fp );
    if(str_cmp(word,"End"))
	{
		sprintf(buf,"No end to faction table found on character %s",ch->name);
		bug(buf,0);
		fread_to_eol(fp);
    }
    return;
}

// write all the materials out for a character in decent fashion
void fwrite_material(CHAR_DATA *ch, FILE *fp)
{
	int i;
	
    fprintf(fp,"#MATERIAL\n");
    for( i = 0 ; i < MAX_MATERIAL; i++ )
		fprintf(fp,"%d %s",ch->mats[i],(i != 0 && i % 10 == 0 ? "\n" : ""));
    fprintf(fp,"\nEnd\n\n");
// do not close file, still need it
    return;
}

void fwrite_expertise(CHAR_DATA *ch, FILE *fp)
{
	int i;
	
	fprintf (fp, "#EXPERTISE\n");
	for (i = 0; i < MAX_WEAPON; i++ )
		fprintf(fp,"%d %s",ch->expertise[i],(i != 0 && i % 5 == 0 ? "\n" : ""));
	fprintf(fp,"\nEnd\n\n");
// do not close file, still need it
    return;	
}


void fread_expertise(CHAR_DATA *ch, FILE *fp)
{
	int num = 0;
    char buf[100];
	int i;

	
    for(i = 0; i < MAX_WEAPON; i++ )
	{
		num = fread_number(fp);
		ch->expertise[i] = num;
    }
    const char * word = feof( fp ) ? "End" : fread_word( fp );
    if(str_cmp(word,"End"))
	{
		sprintf(buf,"No end to expertises found on character %s",ch->name);
		bug(buf,0);
		fread_to_eol(fp);
    }
    return;
}

// read all materials for a character's pfile
void fread_material(CHAR_DATA *ch, FILE *fp){
    int num = 0;
    char buf[100];
	int i;

	int max = 0;
	
	if (ch->version < 9)
		max = 54;
	else
		max = MAX_MATERIAL;
	
    for(i = 0; i < max; i++ )
	{
		num = fread_number(fp);
		ch->mats[i] += num;
    }
    const char * word = feof( fp ) ? "End" : fread_word( fp );
    if(str_cmp(word,"End"))
	{
		sprintf(buf,"No end to material table found on character %s",ch->name);
		bug(buf,0);
		fread_to_eol(fp);
    }
    return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj (DESCRIPTOR_DATA * d, char *name)
{
    char strsave[MAX_INPUT_LENGTH];
	char bug_buffer[MSL];
//    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;
	int sn;

    ch = 				new_char ();	
    ch->pcdata = 		new_pcdata ();
	ch->ship = 			new_shipdata ();
	ch->society_rank = 	0;	
	//ch->ship->in_room->vnum = 100;
    d->character = 		ch;	
	ch->desc = 			d;
	ch->invis_level = 	0;
	ch->incog_level = 	0;
	//ch->pc_data->fight_pos = 0;	
	ch->god = 			0;		
	ch->pk_timer = 		0;
	ch->altitude = 0;
	ch->spell_crit = 0;
	ch->melee_crit = 0;
	ch->spell_dam = 0;
	ch->chopping = NULL;
	ch->no_exp[0] = FALSE;
	ch->no_exp[1] = FALSE;	
	ch->grank = G_FRONT;
	ch->onBoard = FALSE;
	ch->event = EVENT_NONE;
	ch->name = str_dup (name);
    ch->id = get_pc_id ();
    ch->race = race_lookup ("human");
    ch->act = PLR_NOSUMMON;
    ch->comm = COMM_COMBINE | COMM_PROMPT;
    ch->prompt = str_dup ("<%hhp %mm %vmv> ");
    ch->pcdata->confirm_delete = FALSE;
	ch->pcdata->board = &boards[DEFAULT_BOARD];
    ch->pcdata->pwd = str_dup ("");
    ch->pcdata->bamfin = str_dup ("");
    ch->pcdata->bamfout = str_dup ("");
    ch->pcdata->title = str_dup ("");
	ch->guildrank = 0;
	ch->guildpoints = 0;
	ch->pcdata->pretitle = str_dup ("");	
	ch->pcdata->item_condition = TRUE;
    for (stat = 0; stat < MAX_STATS; stat++)
        ch->perm_stat[stat] = 13;
    ch->pcdata->condition[COND_THIRST] = 60;
    ch->pcdata->condition[COND_FULL] = 60;
    ch->pcdata->condition[COND_HUNGER] = 60;
	ch->pcdata->condition[COND_DRUNK] = 0;
	ch->pcdata->condition[COND_EUPHORIC] = 0;
	
    ch->pcdata->security = 0;    /* OLC */	
	
	/*int i = 0;
	for (i = 0; i < MAX_QUEST; i++)
	{		
		ch->complete[i] = 0;
		ch->quest_vnum[i] = 0;
		ch->qread1[i] = 0;
		ch->qread2[i] = 0;
		ch->qread3[i] = 0;
		ch->multiples[i] = 0;
	}*/
	
	int x = 0;
	
	//Reset Factions:
	for (x = 0; x <= MAX_FACTION; x++)
	{
		ch->faction_rep[x] = 0;
		ch->faction_rank[x] = 0;
	}
	ch->faction = -1;
	ch->faction_invite = -1;
	ch->society_rank = -1;

	for (x = 0; x < MAX_WEAPON; x++)
	{
		ch->expertise[x] = 0;		
	}
	
	//Reset Cooldowns	
	for (x = 1; x <= MAX_SKILL; x++)	
		ch->cooldowns[x] = 0;		
	
	
    ch->pcdata->text[0] = (NORMAL);
    ch->pcdata->text[1] = (WHITE);
    ch->pcdata->text[2] = 0;
    ch->pcdata->auction[0] = (BRIGHT);
    ch->pcdata->auction[1] = (YELLOW);
    ch->pcdata->auction[2] = 0;
    ch->pcdata->auction_text[0] = (BRIGHT);
    ch->pcdata->auction_text[1] = (WHITE);
    ch->pcdata->auction_text[2] = 0;
    ch->pcdata->gossip[0] = (NORMAL);
    ch->pcdata->gossip[1] = (MAGENTA);
    ch->pcdata->gossip[2] = 0;
    ch->pcdata->gossip_text[0] = (BRIGHT);
    ch->pcdata->gossip_text[1] = (MAGENTA);
    ch->pcdata->gossip_text[2] = 0;
    ch->pcdata->music[0] = (NORMAL);
    ch->pcdata->music[1] = (RED);
    ch->pcdata->music[2] = 0;
    ch->pcdata->music_text[0] = (BRIGHT);
    ch->pcdata->music_text[1] = (RED);
    ch->pcdata->music_text[2] = 0;
    ch->pcdata->question[0] = (BRIGHT);
    ch->pcdata->question[1] = (YELLOW);
    ch->pcdata->question[2] = 0;
    ch->pcdata->question_text[0] = (BRIGHT);
    ch->pcdata->question_text[1] = (WHITE);
    ch->pcdata->question_text[2] = 0;
    ch->pcdata->answer[0] = (BRIGHT);
    ch->pcdata->answer[1] = (YELLOW);
    ch->pcdata->answer[2] = 0;
    ch->pcdata->answer_text[0] = (BRIGHT);
    ch->pcdata->answer_text[1] = (WHITE);
    ch->pcdata->answer_text[2] = 0;
    ch->pcdata->quote[0] = (NORMAL);
    ch->pcdata->quote[1] = (YELLOW);
    ch->pcdata->quote[2] = 0;
    ch->pcdata->quote_text[0] = (NORMAL);
    ch->pcdata->quote_text[1] = (GREEN);
    ch->pcdata->quote_text[2] = 0;
    ch->pcdata->immtalk_text[0] = (NORMAL);
    ch->pcdata->immtalk_text[1] = (CYAN);
    ch->pcdata->immtalk_text[2] = 0;
    ch->pcdata->immtalk_type[0] = (NORMAL);
    ch->pcdata->immtalk_type[1] = (YELLOW);
    ch->pcdata->immtalk_type[2] = 0;
    ch->pcdata->info[0] = (BRIGHT);
    ch->pcdata->info[1] = (YELLOW);
    ch->pcdata->info[2] = 1;
    ch->pcdata->say[0] = (NORMAL);
    ch->pcdata->say[1] = (GREEN);
    ch->pcdata->say[2] = 0;
    ch->pcdata->say_text[0] = (BRIGHT);
    ch->pcdata->say_text[1] = (GREEN);
    ch->pcdata->say_text[2] = 0;
    ch->pcdata->tell[0] = (NORMAL);
    ch->pcdata->tell[1] = (GREEN);
    ch->pcdata->tell[2] = 0;
    ch->pcdata->tell_text[0] = (BRIGHT);
    ch->pcdata->tell_text[1] = (GREEN);
    ch->pcdata->tell_text[2] = 0;
    ch->pcdata->reply[0] = (NORMAL);
    ch->pcdata->reply[1] = (GREEN);
    ch->pcdata->reply[2] = 0;
    ch->pcdata->reply_text[0] = (BRIGHT);
    ch->pcdata->reply_text[1] = (GREEN);
    ch->pcdata->reply_text[2] = 0;
    ch->pcdata->gtell_text[0] = (NORMAL);
    ch->pcdata->gtell_text[1] = (GREEN);
    ch->pcdata->gtell_text[2] = 0;
    ch->pcdata->gtell_type[0] = (NORMAL);
    ch->pcdata->gtell_type[1] = (RED);
    ch->pcdata->gtell_type[2] = 0;
    ch->pcdata->wiznet[0] = (NORMAL);
    ch->pcdata->wiznet[1] = (GREEN);
    ch->pcdata->wiznet[2] = 0;
    ch->pcdata->room_title[0] = (NORMAL);
    ch->pcdata->room_title[1] = (CYAN);
    ch->pcdata->room_title[2] = 0;
    ch->pcdata->room_text[0] = (NORMAL);
    ch->pcdata->room_text[1] = (WHITE);
    ch->pcdata->room_text[2] = 0;
    ch->pcdata->room_exits[0] = (NORMAL);
    ch->pcdata->room_exits[1] = (GREEN);
    ch->pcdata->room_exits[2] = 0;
    ch->pcdata->room_things[0] = (NORMAL);
    ch->pcdata->room_things[1] = (CYAN);
    ch->pcdata->room_things[2] = 0;
    ch->pcdata->prompt[0] = (NORMAL);
    ch->pcdata->prompt[1] = (CYAN);
    ch->pcdata->prompt[2] = 0;
    ch->pcdata->fight_death[0] = (BRIGHT);
    ch->pcdata->fight_death[1] = (RED);
    ch->pcdata->fight_death[2] = 0;
    ch->pcdata->fight_yhit[0] = (NORMAL);
    ch->pcdata->fight_yhit[1] = (GREEN);
    ch->pcdata->fight_yhit[2] = 0;
    ch->pcdata->fight_ohit[0] = (NORMAL);
    ch->pcdata->fight_ohit[1] = (YELLOW);
    ch->pcdata->fight_ohit[2] = 0;
    ch->pcdata->fight_thit[0] = (NORMAL);
    ch->pcdata->fight_thit[1] = (RED);
    ch->pcdata->fight_thit[2] = 0;
    ch->pcdata->fight_skill[0] = (BRIGHT);
    ch->pcdata->fight_skill[1] = (WHITE);
    ch->pcdata->fight_skill[2] = 0;	
	ch->pcdata->recall = ROOM_VNUM_RECALL;
    found = FALSE;
    fclose (fpReserve);

// #if defined(unix)
    // /* decompress if .gz file exists */
    // sprintf (strsave, "%s%s%s", PLAYER_DIR, capitalize (name), ".gz");
    // if ((fp = fopen (strsave, "r")) != NULL)
    // {
        // fclose (fp);
        // sprintf (buf, "gzip -dfq %s", strsave);
        // system (buf);
    // }
// #endif

    sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (name));
    if ((fp = fopen (strsave, "r")) != NULL)
    {
        int iNest;

        for (iNest = 0; iNest < MAX_NEST; iNest++)
            rgObjNest[iNest] = NULL;

        found = TRUE;
        for (;;)
        {
            char letter;
            char *word;

            letter = fread_letter (fp);
            if (letter == '*')
            {
                fread_to_eol (fp);
                continue;
            }

            if (letter != '#')
            {
                bug ("Load_char_obj: # not found.", 0);
                break;
            }

            word = fread_word (fp);
            if (!str_cmp (word, "PLAYER"))
                fread_char (ch, fp);
            else if (!str_cmp (word, "OBJECT"))
                fread_obj (ch, fp, NULL);
            else if (!str_cmp (word, "O"))
                fread_obj (ch, fp, NULL);
            else if (!str_cmp (word, "PET"))
                fread_pet (ch, fp);
			//else if (!str_cmp (word, "QUEST"))
			//			fread_quest (ch, fp);				
			else if ( !str_cmp( word, "MATERIAL")) 
				fread_material( ch, fp ); 
			else if ( !str_cmp( word, "FACTIONS"))
				fread_factions( ch, fp);
			else if ( !str_cmp( word, "EXPERTISE"))
				fread_expertise( ch, fp);
            else if (!str_cmp (word, "END"))
                break;
            else
            {
				sprintf(bug_buffer, "Load_char_obj: bad section. (%s)", word);
                bug (bug_buffer, 0);
                break;
            }
        }
        fclose (fp);
    }

    fpReserve = fopen (NULL_FILE, "r");


    /* initialize race */
    if (found)
    {
        int i;

        if (ch->race == 0)
            ch->race = race_lookup ("human");

        ch->size = pc_race_table[ch->race].size;
        ch->dam_type = 17;        /*punch */

        for (i = 0; i < 5; i++)
        {
            if (pc_race_table[ch->race].skills[i] == NULL)
                break;
            group_add (ch, pc_race_table[ch->race].skills[i], FALSE);
        }
        ch->affected_by = ch->affected_by | race_table[ch->race].aff;
        ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
        ch->res_flags = ch->res_flags | race_table[ch->race].res;
        ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
        ch->form = race_table[ch->race].form;
        ch->parts = race_table[ch->race].parts;
    }


    /* RT initialize skills */

    if (found && ch->version < 2)
    {                            /* need to add the new skills */
        group_add (ch, "rom basics", FALSE);
        group_add (ch, ch_class_table[ch-> ch_class].base_group, FALSE);
        //group_add (ch, ch_class_table[ch-> ch_class].default_group, TRUE);
        ch->pcdata->learned[gsn_recall] = 50;
    }
	
    return found;
}

/*
 * Read in a char.
 */


void fread_char (CHAR_DATA * ch, FILE * fp)
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int count = 0;
    int lastlogoff = current_time;
    int percent;

    sprintf (buf, "Loading %s.", ch->name);
    log_string (buf);

    for (;;)
    {
        word = feof (fp) ? "End" : fread_word (fp);
        fMatch = FALSE;

        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
				KEY ("Account", ch->account, fread_string(fp));
                KEY ("Act", ch->act, fread_flag (fp));
                KEY ("AffectedBy", ch->affected_by, fread_flag (fp));
                KEY ("AfBy", ch->affected_by, fread_flag (fp));
				KEY ("Age", ch->age, fread_number(fp));
                KEY ("Alignment", ch->alignment, fread_number (fp));
                KEY ("Alig", ch->alignment, fread_number (fp));

                if (!str_cmp (word, "Alia"))
                {
                    if (count >= MAX_ALIAS)
                    {
                        fread_to_eol (fp);
                        fMatch = TRUE;
                        break;
                    }

                    ch->pcdata->alias[count] = str_dup (fread_word (fp));
                    ch->pcdata->alias_sub[count] = str_dup (fread_word (fp));
                    count++;
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Alias"))
                {
                    if (count >= MAX_ALIAS)
                    {
                        fread_to_eol (fp);
                        fMatch = TRUE;
                        break;
                    }

                    ch->pcdata->alias[count] = str_dup (fread_word (fp));
                    ch->pcdata->alias_sub[count] = fread_string (fp);
                    count++;
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "AC") || !str_cmp (word, "Armor"))
                {
                    fread_to_eol (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "ACs"))
                {
                    int i;

                    for (i = 0; i < 4; i++)
                        ch->armor[i] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "AffD"))
                {
                    AFFECT_DATA *paf;
                    int sn;

                    paf = new_affect ();

                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("Fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    paf->next = ch->affected;
                    ch->affected = paf;
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Affc"))
                {
                    AFFECT_DATA *paf;
                    int sn;

                    paf = new_affect ();

                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("Fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->where = fread_number (fp);
                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    paf->next = ch->affected;
                    ch->affected = paf;
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "AttrMod") || !str_cmp (word, "AMod"))
                {
                    int stat;
                    for (stat = 0; stat < MAX_STATS; stat++)
                        ch->mod_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "AttrPerm") || !str_cmp (word, "Attr"))
                {
                    int stat;

                    for (stat = 0; stat < MAX_STATS; stat++)
                        ch->perm_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'B':
				KEY( "BAcct",       ch->bank_act,		fread_number( fp ) );
				KEY( "Bank",	ch->bank_amt,		fread_number( fp ) );
                KEY ("Bamfin", ch->pcdata->bamfin, fread_string (fp));
                KEY ("Bamfout", ch->pcdata->bamfout, fread_string (fp));
				KEY ("Beeptells", ch->pcdata->beeptells, fread_number(fp));
                KEY ("Bin", ch->pcdata->bamfin, fread_string (fp));
				KEY ("Blind", ch->blind,	fread_number( fp ) );
				KEY ("Bleed", ch->bleeding, fread_number(fp));
				KEY ("Boon", ch->boon,		fread_number(fp));
                KEY ("Bout", ch->pcdata->bamfout, fread_string (fp));			

				/* Read in board status */
				if (!str_cmp(word, "Boards" ))
				{
					int i,num = fread_number (fp); /* number of boards saved */
					char *boardname;

					for (; num ; num-- ) /* for each of the board saved */
					{
						boardname = fread_word (fp);
						i = board_lookup (boardname); /* find board number */

						if (i == BOARD_NOTFOUND) /* Does board still exist ? */
						{
							sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.",
							ch->name, boardname);
							log_string (buf);
							fread_number (fp); /* read last_note and skip info */
						}
						else /* Save it */
							ch->pcdata->last_note[i] = fread_number (fp);
					} /* for */

					fMatch = TRUE;
				} /* Boards */
                break;

            case 'C':
                KEY ("Class", ch->ch_class, fread_number (fp));
				KEY ("Class2", ch->ch_class2, fread_number (fp));
                KEY ("Cla", ch->ch_class, fread_number (fp));
                KEY ("Clan", ch->clan, clan_lookup (fread_string (fp)));
				KEY ("Clan_Rank", ch->clan_rank, fread_number (fp));
				KEY ("Citizen", ch->citizen, fread_string (fp));
				KEY ("Clan_Points", ch->clan_points, fread_number(fp));
				//KEY ("Completed", ch->completed, fread_number (fp));
                KEY ("Comm", ch->comm, fread_flag (fp));
				KEY ("Cond", ch->pcdata->item_condition, fread_number(fp));

                if (!str_cmp (word, "Condition") || !str_cmp (word, "Cond"))
                {
                    ch->pcdata->condition[0] = fread_number (fp);
                    ch->pcdata->condition[1] = fread_number (fp);
                    ch->pcdata->condition[2] = fread_number (fp);
					if (ch->version >= VER_CONDITION_UPDATE)
					{
						ch->pcdata->condition[3] = fread_number (fp);
						ch->pcdata->condition[4] = fread_number (fp);
					}
                    fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Cnd"))
                {
                    ch->pcdata->condition[0] = fread_number (fp);
                    ch->pcdata->condition[1] = fread_number (fp);
                    ch->pcdata->condition[2] = fread_number (fp);
					if (ch->version >= VER_CONDITION_UPDATE)
					{
						ch->pcdata->condition[3] = fread_number (fp);
						ch->pcdata->condition[4] = fread_number (fp);
					}
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Coloura"))
                {
                    LOAD_COLOUR (text)
                        LOAD_COLOUR (auction)
                        LOAD_COLOUR (gossip)
                        LOAD_COLOUR (music)
                        LOAD_COLOUR (question) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourb"))
                {
                    LOAD_COLOUR (answer)
                        LOAD_COLOUR (quote)
                        LOAD_COLOUR (quote_text)
                        LOAD_COLOUR (immtalk_text)
                        LOAD_COLOUR (immtalk_type) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourc"))
                {
                    LOAD_COLOUR (info)
                        LOAD_COLOUR (tell)
                        LOAD_COLOUR (reply)
                        LOAD_COLOUR (gtell_text)
                        LOAD_COLOUR (gtell_type) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourd"))
                {
                    LOAD_COLOUR (room_title)
                        LOAD_COLOUR (room_text)
                        LOAD_COLOUR (room_exits)
                        LOAD_COLOUR (room_things)
                        LOAD_COLOUR (prompt) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Coloure"))
                {
                    LOAD_COLOUR (fight_death)
                        LOAD_COLOUR (fight_yhit)
                        LOAD_COLOUR (fight_ohit)
                        LOAD_COLOUR (fight_thit)
                        LOAD_COLOUR (fight_skill) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourf"))
                {
                    LOAD_COLOUR (wiznet)
                        LOAD_COLOUR (say)
                        LOAD_COLOUR (say_text)
                        LOAD_COLOUR (tell_text)
                        LOAD_COLOUR (reply_text) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourg"))
                {
                    LOAD_COLOUR (auction_text)
                        LOAD_COLOUR (gossip_text)
						LOAD_COLOUR (music_text)
                        LOAD_COLOUR (question_text)
                        LOAD_COLOUR (answer_text) fMatch = TRUE;
                    break;
                }

                break;

            case 'D':
                KEY ("Damroll", ch->damroll, fread_number (fp));
                KEY ("Dam", ch->damroll, fread_number (fp));
                KEY ("Description", ch->description, fread_string (fp));
                KEY ("Desc", ch->description, fread_string (fp));
                break;

            case 'E':
				KEY ("EqCond", ch->pcdata->eqcondition, fread_number(fp));
				KEY ("Eyes", ch->eyes, fread_flag (fp));
				
				if(!str_cmp(word, "Explored") )
				{	int mask, bit;
					EXPLORE_HOLDER *pExp;
					ch->pcdata->explored->set = fread_number(fp);
					while(1)
					{	mask = fread_number(fp);
						bit = fread_number(fp);
						if(mask == -1)
							break;
						for(pExp = ch->pcdata->explored->bits ; pExp ; pExp = pExp->next )
							if(pExp->mask == mask)
								break;
						if(!pExp)
						{	pExp = (EXPLORE_HOLDER *)calloc(1, sizeof(*pExp) );
							pExp->next = ch->pcdata->explored->bits;
							ch->pcdata->explored->bits = pExp;
							pExp->mask = mask;
						}
						pExp->bits = bit;
					}
					fMatch = TRUE;
				}
				
				KEY ("Email", ch->email, fread_string (fp));
				
				KEY ( "ElemSpec", ch->pcdata->elementalSpec, fread_number( fp ) );
                if (!str_cmp (word, "End"))
                {
                    /* adjust hp mana move up  -- here for speed's sake */
                    percent =
                        (current_time - lastlogoff) * 25 / (2 * 60 * 60);

                    percent = UMIN (percent, 100);

                    if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
                        && !IS_AFFECTED (ch, AFF_PLAGUE))
                    {
                        ch->hit += (ch->max_hit - ch->hit) * percent / 100;
                        ch->mana += (ch->max_mana - ch->mana) * percent / 100;
                        ch->move += (ch->max_move - ch->move) * percent / 100;
                    }
                    return;
                }
                KEY ("Exp", ch->exp, fread_number (fp));
				KEY ("Exp2", ch->exp2, fread_number (fp));
                break;
			case 'F':
				KEY ("Facial", ch->facial_hair, fread_flag (fp));
				KEY ("Faction", ch->faction, fread_number(fp));
				KEY ("Favored", ch->favored_enemy, fread_number(fp));
				break;
            case 'G':
				KEY ("God",	ch->god, fread_number(fp));
				KEY ("GuildRnk", ch->guildrank, fread_number(fp));
				KEY ("GuildPts", ch->guildpoints, fread_number(fp));				
                KEY ("Gold", ch->gold, fread_number (fp));
                if (!str_cmp (word, "Group") || !str_cmp (word, "Gr"))
                {
                    int gn;
                    char *temp;

                    temp = fread_word (fp);
                    gn = group_lookup (temp);
                    /* gn    = group_lookup( fread_word( fp ) ); */
                    if (gn < 0)
                    {
                        fprintf (stderr, "%s", temp);
                        bug ("Fread_char: unknown group. ", 0);
                    }
                    else
                        gn_add (ch, gn);
                    fMatch = TRUE;
                }
                break;

            case 'H':
				KEY ("Hair", ch->hair, fread_flag (fp));
				KEY ("HairCut", ch->haircut, fread_flag (fp));
                KEY ("Hitroll", ch->hitroll, fread_number (fp));
                KEY ("Hit", ch->hitroll, fread_number (fp));

                if (!str_cmp (word, "HpManaMove") || !str_cmp (word, "HMV"))
                {
                    ch->hit = fread_number (fp);
                    ch->max_hit = fread_number (fp);
                    ch->mana = fread_number (fp);
                    ch->max_mana = fread_number (fp);
                    ch->move = fread_number (fp);
                    ch->max_move = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "HpManaMovePerm")
                    || !str_cmp (word, "HMVP"))
                {
                    ch->pcdata->perm_hit = fread_number (fp);
                    ch->pcdata->perm_mana = fread_number (fp);
                    ch->pcdata->perm_move = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'I':
                KEY ("Id", ch->id, fread_number (fp));
                KEY ("InvisLevel", ch->invis_level, fread_number (fp));
                KEY ("Inco", ch->incog_level, fread_number (fp));
				KEY ("Invite", ch->faction_invite, fread_number (fp));
                KEY ("Invi", ch->invis_level, fread_number (fp));
                break;

            case 'L':
                KEY ("LastLevel", ch->pcdata->last_level, fread_number (fp));
				KEY ("LastName", ch->last_name, fread_string(fp));
                KEY ("LLev", ch->pcdata->last_level, fread_number (fp));
                KEY ("Level", ch->level, fread_number (fp));
                KEY ("Lev", ch->level, fread_number (fp));
                KEY ("Levl", ch->level, fread_number (fp));
				KEY ("Level2", ch->level2, fread_number (fp));
                KEY ("LogO", lastlogoff, fread_number (fp));
                KEY ("LongDescr", ch->long_descr, fread_string (fp));
                KEY ("LnD", ch->long_descr, fread_string (fp));
                break;

			case 'M':
				KEY ("Mclass", ch->mClass, fread_number (fp));
				KEY ("Mkills", ch->mkill, fread_number (fp));
				break;
				
            case 'N':
                KEYS ("Name", ch->name, fread_string (fp));
				KEY ("NoExp1", ch->no_exp[0], fread_number (fp));
				KEY ("NoExp2", ch->no_exp[1], fread_number (fp));
                break;

            case 'P':
				
				if (!str_cmp(word, "PP"))
				{
					ch->pp = fread_number (fp);
                    ch->max_pp = fread_number (fp);	
					fMatch = TRUE;
					break;
				}
				
                KEY ("Password", ch->pcdata->pwd, fread_string (fp));
                KEY ("Pass", ch->pcdata->pwd, fread_string (fp));
                KEY ("Played", ch->played, fread_number (fp));
                KEY ("Plyd", ch->played, fread_number (fp));
				KEY ("Pkills", ch->pkill, fread_number (fp));
				KEY ("Pdeath", ch->pdeath, fread_number (fp));
				KEY ("Points", ch->pcdata->points, fread_number (fp));
                KEY ("Pnts", ch->pcdata->points, fread_number (fp));
                KEY ("Position", ch->position, fread_number (fp));
                KEY ("Pos", ch->position, fread_number (fp));
                KEY ("Practice", ch->practice, fread_number (fp));
                KEY ("Prac", ch->practice, fread_number (fp));
				KEY ("PreT", ch->pcdata->pretitle, fread_string (fp));
                KEYS ("Prompt", ch->prompt, fread_string (fp));
                KEY ("Prom", ch->prompt, fread_string (fp));
                break;

            case 'R':
                KEY ("Race", ch->race, race_lookup (fread_string (fp)));
				KEY ("Recall", ch->pcdata->recall, fread_number(fp));
				
                if (!str_cmp (word, "Room"))
                {
                    ch->in_room = get_room_index (fread_number (fp));
                    if (ch->in_room == NULL)
                        ch->in_room = get_room_index (ROOM_VNUM_LIMBO);
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'S':
                KEY ("SavingThrow", ch->saving_throw, fread_number (fp));
                KEY ("Save", ch->saving_throw, fread_number (fp));
                KEY ("Scro", ch->lines, fread_number (fp));
                KEY ("Sex", ch->sex, fread_number (fp));
                KEY ("ShortDescr", ch->short_descr, fread_string (fp));
                KEY ("ShD", ch->short_descr, fread_string (fp));
                KEY ("Sec", ch->pcdata->security, fread_number (fp));    /* OLC */
                KEY ("Silv", ch->silver, fread_number (fp));
				KEY ("Society", ch->society_rank, fread_number (fp));


                if (!str_cmp (word, "Skill") || !str_cmp (word, "Sk"))
                {
                    int sn;
                    int value;
                    char *temp;

                    value = fread_number (fp);
                    temp = fread_word (fp);
                    sn = skill_lookup (temp);
                    /* sn    = skill_lookup( fread_word( fp ) ); */
                    if (sn < 0)
                    {
                        fprintf (stderr, "%s", temp);
                        bug ("Fread_char: unknown skill. ", 0);
                    }
                    else
                        ch->pcdata->learned[sn] = value;
                    fMatch = TRUE;
                }

                break;

            case 'T':
                KEY ("Talent", ch->has_talent, fread_number(fp));
				KEY ("TrueSex", ch->pcdata->true_sex, fread_number (fp));
                KEY ("TSex", ch->pcdata->true_sex, fread_number (fp));
                KEY ("Trai", ch->train, fread_number (fp));
                KEY ("Trust", ch->trust, fread_number (fp));
                KEY ("Tru", ch->trust, fread_number (fp));				

                if (!str_cmp (word, "Title") || !str_cmp (word, "Titl"))
                {
                    ch->pcdata->title = fread_string (fp);
                    if (ch->pcdata->title[0] != '.'
                        && ch->pcdata->title[0] != ','
                        && ch->pcdata->title[0] != '!'
                        && ch->pcdata->title[0] != '?')
                    {
                        sprintf (buf, " %s", ch->pcdata->title);
                        free_string (ch->pcdata->title);
                        ch->pcdata->title = str_dup (buf);
                    }
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'V':
                KEY ("Version", ch->version, fread_number (fp));
                KEY ("Vers", ch->version, fread_number (fp));
                if (!str_cmp (word, "Vnum"))
                {
                    ch->pIndexData = get_mob_index (fread_number (fp));
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'W':
				KEY ("WpnSpec", ch->pcdata->weaponSpec, fread_number( fp ) );
                KEY ("Wimpy", ch->wimpy, fread_number (fp));
                KEY ("Wimp", ch->wimpy, fread_number (fp));
                KEY ("Wizn", ch->wiznet, fread_flag (fp));
				KEY ("WalkDesc", ch->walk_desc, fread_string (fp));
                break;
        }

        if (!fMatch)
        {
            bug ("Fread_char: no match.", 0);
            bug (word, 0);
            fread_to_eol (fp);
        }
    }
}

void fwrite_ship (CHAR_DATA * ch, FILE * fp)
{    

	if (ch->ship)
	{
		fprintf (fp, "#SHIP\n");
		fprintf (fp, "Name %s~\n", ch->ship->name);
		//fprintf (fp, "Room %d\n", ch->ship->in_room->vnum);
		fprintf (fp, "\nEnd\n\n");
		return;
	}
}

void fread_ship (CHAR_DATA * ch, FILE * fp)
{
	char *word;    
    bool fMatch;    
	
	for (;;)
    {
        word = feof (fp) ? "End" : fread_word (fp);
        fMatch = FALSE;

        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;

            case 'N':
                KEY ("Name", ch->ship->name, fread_string (fp));
                break;
			
			//case 'R':
				//KEY ("Room", ch->ship->in_room->vnum, fread_number(fp));
				//break;

                if (!fMatch)
                {
                    bug ("Fread_ship: no match.", 0);
                    fread_to_eol (fp);
                }

        }
		
		if (!fMatch)
        {
            bug ("Fread_char (ship code): no match.", 0);
            bug (word, 0);
            fread_to_eol (fp);
        }

    }
	
}

/* load a pet from the forgotten reaches */
void fread_pet (CHAR_DATA * ch, FILE * fp)
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;
    int vnum = 0;

    /* first entry had BETTER be the vnum or we barf */
    word = feof (fp) ? "END" : fread_word (fp);
    if (!str_cmp (word, "Vnum"))
    {

        vnum = fread_number (fp);
        if (get_mob_index (vnum) == NULL)
        {
            bug ("Fread_pet: bad vnum %d.", vnum);
            pet = create_mobile (get_mob_index (MOB_VNUM_FIDO));
        }
        else
            pet = create_mobile (get_mob_index (vnum));
    }
    else
    {
        bug ("Fread_pet: no vnum in file.", 0);
        pet = create_mobile (get_mob_index (MOB_VNUM_FIDO));
    }

    for (;;)
    {
        word = feof (fp) ? "END" : fread_word (fp);
        fMatch = FALSE;

        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                KEY ("Act", pet->act, fread_flag (fp));
                KEY ("AfBy", pet->affected_by, fread_flag (fp));
                KEY ("Alig", pet->alignment, fread_number (fp));

                if (!str_cmp (word, "ACs"))
                {
                    int i;

                    for (i = 0; i < 4; i++)
                        pet->armor[i] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "AffD"))
                {
                    AFFECT_DATA *paf;
                    int sn;

                    paf = new_affect ();

                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("Fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    paf->next = pet->affected;
                    pet->affected = paf;
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Affc"))
                {
                    AFFECT_DATA *paf;
                    int sn;

                    paf = new_affect ();

                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("Fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->where = fread_number (fp);
                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
					/* Added here after Chris Litchfield (The Mage's Lair)
					 * pointed out a bug with duplicating affects in saved
					 * pets. -- JR 2002/01/31
					 */
					if (!check_pet_affected(vnum,paf))
					{
						paf->next       = pet->affected;
						pet->affected   = paf;
					} else{
						free_affect(paf);
					}
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "AMod"))
                {
                    int stat;

                    for (stat = 0; stat < MAX_STATS; stat++)
                        pet->mod_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Attr"))
                {
                    int stat;

                    for (stat = 0; stat < MAX_STATS; stat++)
                        pet->perm_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'C':
                KEY ("Clan", pet->clan, clan_lookup (fread_string (fp)));
                KEY ("Comm", pet->comm, fread_flag (fp));
                break;

            case 'D':
                KEY ("Dam", pet->damroll, fread_number (fp));
                KEY ("Desc", pet->description, fread_string (fp));
                break;

            case 'E':
                if (!str_cmp (word, "End"))
                {
                    pet->leader = ch;
                    pet->master = ch;
                    ch->pet = pet;
                    /* adjust hp mana move up  -- here for speed's sake */
                    percent =
                        (current_time - lastlogoff) * 25 / (2 * 60 * 60);

                    if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
                        && !IS_AFFECTED (ch, AFF_PLAGUE))
                    {
                        percent = UMIN (percent, 100);
                        pet->hit += (pet->max_hit - pet->hit) * percent / 100;
                        pet->mana +=
                            (pet->max_mana - pet->mana) * percent / 100;
                        pet->move +=
                            (pet->max_move - pet->move) * percent / 100;
                    }
                    return;
                }
                KEY ("Exp", pet->exp, fread_number (fp));
                break;

            case 'G':
                KEY ("Gold", pet->gold, fread_number (fp));
                break;

            case 'H':
                KEY ("Hit", pet->hitroll, fread_number (fp));

                if (!str_cmp (word, "HMV"))
                {
                    pet->hit = fread_number (fp);
                    pet->max_hit = fread_number (fp);
                    pet->mana = fread_number (fp);
                    pet->max_mana = fread_number (fp);
                    pet->move = fread_number (fp);
                    pet->max_move = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'L':
                KEY ("Levl", pet->level, fread_number (fp));
                KEY ("LnD", pet->long_descr, fread_string (fp));
                KEY ("LogO", lastlogoff, fread_number (fp));
                break;

            case 'N':
                KEY ("Name", pet->name, fread_string (fp));
                break;

            case 'P':
                KEY ("Pos", pet->position, fread_number (fp));
                break;

            case 'R':
                KEY ("Race", pet->race, race_lookup (fread_string (fp)));
                break;

            case 'S':
                KEY ("Save", pet->saving_throw, fread_number (fp));
                KEY ("Sex", pet->sex, fread_number (fp));
                KEY ("ShD", pet->short_descr, fread_string (fp));
                KEY ("Silv", pet->silver, fread_number (fp));
                break;

                if (!fMatch)
                {
                    bug ("Fread_pet: no match.", 0);
                    fread_to_eol (fp);
                }

        }
    }
}

extern OBJ_DATA *obj_free;


void fread_obj (CHAR_DATA * ch, FILE * fp, ACCOUNT_DATA * acc)
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;            /* to prevent errors */
    bool make_new;                /* update object */	

    fVnum = FALSE;
    obj = NULL;
    first = TRUE;                /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word = feof (fp) ? "End" : fread_word (fp);
    if (!str_cmp (word, "Vnum"))
    {
        int vnum;
        first = FALSE;            /* fp will be in right place */

        vnum = fread_number (fp);
        if (get_obj_index (vnum) == NULL)
        {
            bug ("Fread_obj: bad vnum %d.", vnum);
        }
        else
        {
            obj = create_object (get_obj_index (vnum), -1);
            new_format = TRUE;
        }

    }

    if (obj == NULL)
    {                            /* either not found or old style */
        obj = new_obj ();
        obj->name = str_dup ("");
        obj->short_descr = str_dup ("");
        obj->description = str_dup ("");
    }

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;

    for (;;)
    {
        if (first)
            first = FALSE;
        else
            word = feof (fp) ? "End" : fread_word (fp);
        fMatch = FALSE;

        switch (UPPER (word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                if (!str_cmp (word, "AffD"))
                {
                    AFFECT_DATA *paf;
                    int sn;

                    paf = new_affect ();

                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("Fread_obj: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    paf->next = obj->affected;
                    obj->affected = paf;
                    fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Affc"))
                {
                    AFFECT_DATA *paf;
                    int sn;

                    paf = new_affect ();

                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("Fread_obj: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->where = fread_number (fp);
                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    paf->next = obj->affected;
                    obj->affected = paf;
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'C':
                KEY ("Cond", obj->condition, fread_number (fp));
                KEY ("Cost", obj->cost, fread_number (fp));
                break;

            case 'D':
                KEY ("Description", obj->description, fread_string (fp));
                KEY ("Desc", obj->description, fread_string (fp));
                break;

            case 'E':

                if (!str_cmp (word, "Enchanted"))
                {
                    obj->enchanted = TRUE;
                    fMatch = TRUE;
                    break;
                }

                KEY ("ExtraFlags", obj->extra_flags, fread_number (fp));
				KEY ("Extra2Flags", obj->extra2_flags, fread_number (fp));
                KEY ("ExtF", obj->extra_flags, fread_number (fp));

                if (!str_cmp (word, "ExtraDescr") || !str_cmp (word, "ExDe"))
                {
                    EXTRA_DESCR_DATA *ed;

                    ed = new_extra_descr ();

                    ed->keyword = fread_string (fp);
                    ed->description = fread_string (fp);
                    ed->next = obj->extra_descr;
                    obj->extra_descr = ed;
                    fMatch = TRUE;
                }

                if (!str_cmp (word, "End"))
                {
                    if (!fNest || (fVnum && obj->pIndexData == NULL))
                    {
                        bug ("Fread_obj: incomplete object.", 0);
                        free_obj (obj);
                        return;
                    }
                    else
                    {
                        if (!fVnum)
                        {
                            free_obj (obj);
                            obj =
                                create_object (get_obj_index (OBJ_VNUM_DUMMY),
                                               0);
                        }

                        if (!new_format)
                        {
                            obj->next = object_list;
                            object_list = obj;
                            obj->pIndexData->count++;
                        }

                        if (!obj->pIndexData->new_format
                            && obj->item_type == ITEM_ARMOR
                            && obj->value[1] == 0)
                        {
                            obj->value[1] = obj->value[0];
                            obj->value[2] = obj->value[0];
                        }
                        if (make_new)
                        {
                            int wear;

                            wear = obj->wear_loc;
                            extract_obj (obj);

                            obj = create_object (obj->pIndexData, 0);
                            obj->wear_loc = wear;
                        }
						
						//vault
						
						if (acc != NULL && ch == NULL)
						{							
							obj_to_vault (obj, acc);
						}
						else
						{
                        if (iNest == 0 || rgObjNest[iNest] == NULL)
                            obj_to_char (obj, ch);
                        else
                            obj_to_obj (obj, rgObjNest[iNest - 1]);
						}
                        return;
                    }
                }
                break;

            case 'I':
                KEY ("ItemType", obj->item_type, fread_number (fp));
                KEY ("Ityp", obj->item_type, fread_number (fp));
                break;

            case 'L':
                KEY ("Level", obj->level, fread_number (fp));
                KEY ("Lev", obj->level, fread_number (fp));
                break;

			case 'M':
				KEY ("Material", obj->material, fread_string (fp));
				break;
				
            case 'N':
                KEY ("Name", obj->name, fread_string (fp));

                if (!str_cmp (word, "Nest"))
                {
                    iNest = fread_number (fp);
                    if (iNest < 0 || iNest >= MAX_NEST)
                    {
                        bug ("Fread_obj: bad nest %d.", iNest);
                    }
                    else
                    {
                        rgObjNest[iNest] = obj;
                        fNest = TRUE;
                    }
                    fMatch = TRUE;
                }
                break;

            case 'O':
                if (!str_cmp (word, "Oldstyle"))
                {
                    if (obj->pIndexData != NULL
                        && obj->pIndexData->new_format)
                        make_new = TRUE;
                    fMatch = TRUE;
                }
                break;


            case 'S':
                KEY ("ShortDescr", obj->short_descr, fread_string (fp));
                KEY ("ShD", obj->short_descr, fread_string (fp));

                if (!str_cmp (word, "Spell"))
                {
                    int iValue;
                    int sn;

                    iValue = fread_number (fp);
                    sn = skill_lookup (fread_word (fp));
                    if (iValue < 0 || iValue > 3)
                    {
                        bug ("Fread_obj: bad iValue %d.", iValue);
                    }
                    else if (sn < 0)
                    {
                        bug ("Fread_obj: unknown skill.", 0);
                    }
                    else
                    {
                        obj->value[iValue] = sn;
                    }
                    fMatch = TRUE;
                    break;
                }

                break;

            case 'T':
                KEY ("Timer", obj->timer, fread_number (fp));
                KEY ("Time", obj->timer, fread_number (fp));
                break;

            case 'V':
                if (!str_cmp (word, "Values") || !str_cmp (word, "Vals"))
                {
                    obj->value[0] = fread_number (fp);
                    obj->value[1] = fread_number (fp);
                    obj->value[2] = fread_number (fp);
                    obj->value[3] = fread_number (fp);
                    if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
                        obj->value[0] = obj->pIndexData->value[0];
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Val"))
                {
                    obj->value[0] = fread_number (fp);
                    obj->value[1] = fread_number (fp);
                    obj->value[2] = fread_number (fp);
                    obj->value[3] = fread_number (fp);
                    obj->value[4] = fread_number (fp);
                    fMatch = TRUE;
                    break;
                }

                if (!str_cmp (word, "Vnum"))
                {
                    int vnum;

                    vnum = fread_number (fp);
                    if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
                        bug ("Fread_obj: bad vnum %d.", vnum);
                    else
                        fVnum = TRUE;
                    fMatch = TRUE;
                    break;
                }
                break;

            case 'W':
                KEY ("WearFlags", obj->wear_flags, fread_number (fp));
                KEY ("WeaF", obj->wear_flags, fread_number (fp));
                KEY ("WearLoc", obj->wear_loc, fread_number (fp));
                KEY ("Wear", obj->wear_loc, fread_number (fp));
                KEY ("Weight", obj->weight, fread_number (fp));
                KEY ("Wt", obj->weight, fread_number (fp));
                break;

        }

        if (!fMatch)
        {
            bug ("Fread_obj: no match.", 0);
            fread_to_eol (fp);
        }
    }
}


void do_last(CHAR_DATA * ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char name[MAX_INPUT_LENGTH];
   struct stat fst;

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      SEND("Usage: people <playername>\r\n", ch);
      SEND("Usage: people <# of entries OR '-1' for all entries OR 'today' for all of today's entries>\r\n", ch);
      SEND("Usage: people <playername> <count>\r\n", ch);
      return;
   }
   if (isdigit(arg[0]) || atoi(arg) == -1 || !str_cmp(arg, "today")) //View list instead of players
   {
      SEND("{rName                     Time                        Host/Ip\r\n{D-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n", ch);
      if (!str_cmp(arg, "today"))
         read_last_file(ch, -2, NULL);
      else
         read_last_file(ch, atoi(arg), NULL);
      return;
   }  
   strcpy(name, capitalize(arg));
   if (argument[0] != '\0')
   {
      SEND("{rName                     Time                        Host/Ip\r\n{D-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n", ch);
      read_last_file(ch, atoi(argument), name);
      return;
   }
   sprintf(buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name);
   
   if (stat(buf, &fst) != -1)
      sprintf(buf, "%s was last on: %s\r", name, ctime(&fst.st_mtime));
   else
      sprintf(buf, "%s was not found.\r\n", name);
   SEND(buf, ch);
}
   
   