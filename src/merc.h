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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/


 
#include "protocol.h"

//#include "protocols.h"

 
/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define DECLARE_OBJ_FUN( fun )		void fun( )
#define DECLARE_ROOM_FUN( fun )		void fun( )
#define const
#define args( list )            ( )
#define DECLARE_DO_FUN( fun )        	void fun( )
#define DECLARE_SPEC_FUN( fun )        	bool fun( )
#define DECLARE_SPELL_FUN( fun )    	void fun( )
#else
#define args( list )           			list
#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )    	SPELL_FUN fun
#define DECLARE_OBJ_FUN( fun )			OBJ_FUN	  fun
#define DECLARE_ROOM_FUN( fun )			ROOM_FUN  fun
#endif


#define SPELL( fun )    void fun ( int sn, int level, CHAR_DATA *ch, void *vo, int target )

/* system calls */
int unlink();
int system();

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
 

#define SUCCESS 1
#define FAILURE 0

#if    !defined(FALSE)
#define FALSE     0
#endif

#if    !defined(TRUE)
#define TRUE     1
#endif

#if    !defined(FALSE)
#define false    0
#endif

#if    !defined(true)
#define true     1
#endif

#if    defined(_AIX)
#if    !defined(const)
#define const
#endif
typedef int                sh_int;
typedef int                bool;
#define unix
#else
typedef short   int            sh_int;
typedef unsigned char            bool;
#endif



/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH          1024
#define MAX_STRING_LENGTH     4608
#define MAX_INPUT_LENGTH       256
#define PAGELEN                 22

/* ea */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

#define SEND	send_to_char
#define MXP 	mxp_to_char
#define WTB		write_to_buffer
#define PRINTF 	printf_to_char
#define STD		send_to_desc

#define LAST_FILE_SIZE			200

/*
 * Structure types.
 */
typedef struct    	affect_data      AFFECT_DATA;
typedef struct    	area_data        AREA_DATA;
typedef struct    	ban_data         BAN_DATA;
typedef struct    	buf_type         BUFFER;
typedef struct    	char_data        CHAR_DATA;
typedef struct    	descriptor_data  DESCRIPTOR_DATA;
typedef struct    	exit_data        EXIT_DATA;
typedef struct    	extra_descr_data EXTRA_DESCR_DATA;
typedef struct    	help_data        HELP_DATA;
typedef struct    	help_area_data   HELP_AREA;
typedef struct   	kill_data        KILL_DATA;
typedef struct    	mem_data         MEM_DATA;
typedef struct    	mob_index_data   MOB_INDEX_DATA;
typedef struct    	note_data        NOTE_DATA;
typedef struct    	obj_data         OBJ_DATA;
typedef struct    	obj_index_data   OBJ_INDEX_DATA;
typedef struct    	pc_data          PC_DATA;
typedef struct    	gen_data         GEN_DATA;
typedef struct    	reset_data       RESET_DATA;
typedef struct    	room_index_data  ROOM_INDEX_DATA;
typedef struct    	shop_data        SHOP_DATA;
typedef struct    	time_info_data   TIME_INFO_DATA;
typedef struct    	weather_data     WEATHER_DATA;
typedef struct    	bounty_data      BOUNTY_DATA;
typedef struct  	prog_list        PROG_LIST;
typedef struct  	prog_code        PROG_CODE;
typedef struct    	colour_data      COLOUR_DATA;
typedef struct 		account_data 	 ACCOUNT_DATA; 
typedef struct      game_data		 	GAME_DATA;
typedef struct		ship_data			SHIP_DATA;
typedef struct 		file_data 			FILE_DATA;
typedef struct    	sleep_data      	SLEEP_DATA;
typedef struct		craft_data			CRAFT_DATA;
typedef struct		quest_player_data	QUEST_DATA;
typedef struct		quest_index_data	QUEST_INDEX_DATA;
typedef struct		auction_data		AUCTION_DATA;
typedef struct  	bidder_data			BIDDER_DATA;




/*
 * Function types.
 */
typedef void DO_FUN    args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN  args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN args( ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) );
typedef void OBJ_FUN	args( ( OBJ_DATA *obj, char *argument ) );
typedef void ROOM_FUN	args( ( ROOM_INDEX_DATA *room, char *argument ) );
				
				
				
				
				
//ENUMS				
				
typedef enum { FIRE_TRAP, POISON_TRAP, GAS_TRAP, DART_TRAP, SHOCK_TRAP } trap_types;
typedef enum { FOREST, DESERT, GRASS, MOUNTAIN, SWAMP, HILL, WATER, CITY } eco_types;
typedef enum { QUALITY_NONE, QUALITY_POOR, QUALITY_COMMON, QUALITY_UNCOMMON, QUALITY_SUPERIOR, QUALITY_EPIC, QUALITY_LEGENDARY } quality_types;
typedef enum { INITIATE, APPRENTICE, MEMBER, OFFICER, MASTER, GUILDMASTER } guild_ranks;
typedef enum { QUEST_MOB_KILL, QUEST_ITEM_GET, QUEST_AREA_EXPLORE, QUEST_HELP_READ,/* MOB_HELP_COMBO, ITEM_HELP_COMBO, AREA_HELP_COMBO,*/ QUEST_MATERIAL_GET, QUEST_REPEATABLE } quest_type_list;
typedef enum { BAIT, CAST_LINE, WAIT, REEL_NO_FISH, REEL_FISH, CATCH_FISH } fish_states;
typedef enum { MISSILE_READY, MISSILE_AIM, MISSILE_RELOAD, MISSILE_FIRE } ranged_states;
typedef enum { ARM, LEG, HAND, FOOT, CHEST, STOMACH, HEAD, FACE, NECK } hit_locations;
typedef enum { CL_TROPICAL, CL_DRY, CL_COASTAL, CL_ARCTIC, CL_HIGHLAND, CL_TEMPERATE } climates;
typedef enum { G_NONE, G_FRONT, G_BACK, G_FLANK } group_ranks;
typedef enum { CONFIG, COMMON, COMM, INFO, OBJ, MOVE, COMBAT, IMM } command_types;
typedef enum { RANK_OUTCAST, RANK_PEASANT, RANK_OFFICIAL, RANK_LORD, RANK_DUKE, RANK_KING } faction_ranks;
typedef enum { FAC_UNKOWN, FAC_ADVENTURER, FAC_RENOWNED, FAC_HERO, FAC_LEGEND } faction_reputation;
typedef enum { DOM_ELEMENTAL, DOM_DEATH, DOM_LIFE, DOM_LAW, DOM_COMBAT, DOM_CHAOS } domain_types;
typedef enum { PLAYER_SHIP, MOB_SHIP } ship_types;
typedef enum { SHIP_DOCKED, SHIP_READY, SHIP_ANCHORED, SHIP_DISABLED, SHIP_SAILING } ship_states;
typedef enum { CANNON_READY, CANNON_FIRED, CANNON_RELOAD } cannon_states;
typedef enum { SHIP_TRIREME, SHIP_CARAVEL, SHIP_FRIGATE, SHIP_GALLEON } ship_classes;
typedef enum { CLOUDLESS, PARTLY_CLOUDY, CLOUDY, HEAVY_CLOUDS } cloud_types;
typedef enum { RAIN, SLEET, SNOW, HAIL } precipitation_types;
typedef enum { VERY_LIGHT, LIGHT, MEDIUM, HEAVY, VERY_HEAVY } precip_amount;
typedef enum { RANK_DUMMY, RANK_PRIVATE, RANK_CORPORAL, RANK_SARGEANT, RANK_MAJOR, RANK_GENERAL, RANK_LEADER } clan_ranks;


#define MAX_ACC_CLAN 4

#define MAX_ACCOUNT_CHAR 100
#define MAX_VAULT_OBJS 100
#define MAX_LINE_LENGTH 100

struct account_char
{
    char *	char_name;
    int 	level;
    int  	ch_class;
    int 	race;
    int 	clan;
    sh_int		rank;
    char *	password;
};

struct account_data
{
    DESCRIPTOR_DATA *   desc;
    char *              name;
    char *              password;
    struct account_char char_list[MAX_ACCOUNT_CHAR];
	OBJ_DATA *			vault;
    char *              email;
    long                act;
    int		        	verify;
    int		       	 	numb;
    long	        	hours;
    long	        	secret;
    long                sql_id;
    bool				valid;   
    int 				version;
    long                pen_pts;
    char *              pen_info;    
    char *              old_passwd;
	int					char_count;
};

#define ACCOUNT_VERSION 2

#define ACCOUNT_DIR "../account/"
#define ACC_VAULT_DIR "../plr_vaults/"

#define VERIFY		1    /* whether or not the character has been verified */
#define ACC_PERMIT	2    /* allows players to play even if site is banned */
#define ACC_DENY	4    /* if the player account was denied */
#define ACC_IMP		8    /* is there an Implementor alt here */
#define ACC_IMM		16   /* is there a Immortal alt here */
#define	ACC_NEWBIE	32   // is set if their account is under 20 hours total
#define	ACC_CLANMASTER	64   // let's them have multiple clans in account.  cheat?
#define ACC_HOSTMASK    128
#define ACC_IDLER       256  // if this is set, it will penalize chars for idling
#define ACC_NOCOLOR	512  // Prevents the player from using colors

#define ACC_MAX_LEVEL	15   /* max level for the unverified */


void account_main_menu(DESCRIPTOR_DATA *d);
void account_email_msg(DESCRIPTOR_DATA *d);
void create_account_menu(DESCRIPTOR_DATA *d);
void intro_who(DESCRIPTOR_DATA *d);
void account_help(DESCRIPTOR_DATA *d);
void intro_menu(DESCRIPTOR_DATA *d);
ACCOUNT_DATA * new_account(void);
void free_account(ACCOUNT_DATA * acc);

int ACC_IS_SET(int bits, int check);
void ACC_SET(ACCOUNT_DATA *, int check);
int create_key( void );
void send_mail(DESCRIPTOR_DATA *d);

bool load_account_obj 	args(( DESCRIPTOR_DATA *d, char *name ));
bool new_account_obj	args(( DESCRIPTOR_DATA *d, char *name ));

//Account vaults: Upro 2/4/2020
bool load_account_vault_obj		args((DESCRIPTOR_DATA *d, char * name ));
void save_account_vault_obj		args(( ACCOUNT_DATA *acc));

void save_account_obj	args(( ACCOUNT_DATA *acc ));
void list_characters	args(( DESCRIPTOR_DATA *d));
int check_account		args((DESCRIPTOR_DATA *, char *));
void del_account_name	args(( char * ));
void mail_pass			args((char *));
void account_char_save	args((ACCOUNT_DATA *, char *, int, int, int, int, int));
bool account_get_new_char_num	args((ACCOUNT_DATA *acc));
void account_free_char			args((ACCOUNT_DATA *, int));
bool check_account_name			args((char *));
void do_adeny			args(( CHAR_DATA *ch, char * ));
bool account_is_playing	args((DESCRIPTOR_DATA *));
int getCharNumb			args((ACCOUNT_DATA *, char *));


#define CLAN_NONE		0
#define CLAN_LONER		1
#define CLAN_CONCLAVE	2
#define CLAN_ACOLYTE	3
#define CLAN_GUARDIAN	4
#define CLAN_RAVAGER	5
#define CLAN_OBSIDIAN	6
#define CLAN_SONG		7
#define CLAN_CORSAIR	8
#define CLAN_JAEZRED	9
#define CLAN_BREGAN_DAERTHE	10

//Clan objects Upro 1/19/2020
#define CLAN_OBJ_GUARDIAN 41
#define CLAN_OBJ_CONCLAVE 42
#define CLAN_OBJ_ACOLYTE  43
#define CLAN_OBJ_RAVAGER  44
#define CLAN_OBJ_OBSIDIAN 45
#define CLAN_OBJ_SONG     46
#define CLAN_OBJ_CORSAIR  47
#define CLAN_OBJ_JAEZRED  48
#define CLAN_OBJ_DAERTHE  49



//Clan pits Upro 1/20/2020
#define CLAN_GUARDIAN_PIT 50
#define CLAN_CONCLAVE_PIT 51
#define CLAN_ACOLYTE_PIT  52
#define CLAN_RAVAGER_PIT  53
#define CLAN_OBSIDIAN_PIT 54
#define CLAN_SONG_PIT     55
#define CLAN_CORSAIR_PIT  56
#define CLAN_JAEZRED_PIT  57
#define CLAN_DAERTHE_PIT  58


 
#define IS_CLAN_PIT( container )   ( container != NULL &&\
                                    (container->pIndexData->vnum == CLAN_GUARDIAN_PIT \
								||   container->pIndexData->vnum == CLAN_CONCLAVE_PIT \
								||	 container->pIndexData->vnum == CLAN_ACOLYTE_PIT \
								||	 container->pIndexData->vnum == CLAN_RAVAGER_PIT \
								||	 container->pIndexData->vnum == CLAN_OBSIDIAN_PIT \
								||	 container->pIndexData->vnum == CLAN_SONG_PIT \
								||	 container->pIndexData->vnum == CLAN_CORSAIR_PIT \
								||	 container->pIndexData->vnum == CLAN_JAEZRED_PIT \
								||	 container->pIndexData->vnum == CLAN_DAERTHE_PIT ) )



//missile types:
#define MISSILE_ARROW 			0
#define MISSILE_BOLT			1

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_SOCIALS        		256
#define MAX_SKILL          		411
#define MAX_GROUP          		42
#define MAX_IN_GROUP       		15
#define MAX_CHAR_ACCOUNT		20		//Maximum characters allowed per account.
#define MAX_ALIAS          		15
#define MAX_CLASS          		11		//Always needs to be one higher than ACTUAL max.
#define MAX_PC_RACE        		22		//Always needs to be one higher than ACTUAL max.
#define MAX_WEAPON				13
#define MAX_FACTION				2
#define MAX_AREA_PER_FACTION	4
#define MAX_EYE_COLOR			7 //blue, green, hazel, brown, red, black, purple
#define MAX_HAIR_COLOR			6 // blonde, brown, black, red, white, grey
#define MAX_MALE_HAIRSTYLES		8 // bald, bowl, mullet, mohawk, crew, untamed, spiked, ponytail
#define MAX_FEMALE_HAIRSTYLES   7 //bald, cropped, braided, double_braided, curly, straight, buns	
#define MAX_FACIAL_HAIR			7 //none, sideburns, muttonchops, goatee, mustache, short_beard, long_beard
#define MAX_LANGUAGE	   		13
#define MAX_CLAN           		11
#define MAX_GOD                 11   //9 +2 for nulls
#define MAX_HERB				47
#define MAX_ENCHANTS			16
#define MAX_MATERIAL			69
#define MAX_CLIMATE				5
#define MAX_HERB_WORLD			125   // Use to make sure there's no overpopulation of herbs.
#define MAX_TREES_WORLD			200  // Make sure trees don't take over!
#define MAX_BURIED_WORLD		50   // Max amount of buried treasure in the world.
#define MAX_SHIP                1000
#define MAX_SHIP_ROOMS          25
#define MAX_DAMAGE_MESSAGE 		43
#define MAX_SOCIETY_RANKS		23
#define MAX_DAM_TYPE			23   // Keep it one higher than the actual last damtype
#define MAX_LEVEL          		50
#define MAX_SIZE				7
#define MAX_CONDITIONS			5	//drunk, hungry, etc...
#define MAX_VNUM        		99999      /* Upper vnum limit, can go to 2,000,000,000 */
#define CURR_PC_VERSION			11


//Version updates.
#define VER_CONDITION_UPDATE	10	//When I added euphoria.


#define LEVEL_HERO         		(MAX_LEVEL - 10)  //40 max level for single class
//Multiclassing Levels:
#define MCLASS_ONE				(MAX_LEVEL - 25)  //25 max level for primary class
#define MCLASS_TWO				(MAX_LEVEL - 35)  //15 max level for secondary class
//
#define LEVEL_IMMORTAL     		(MAX_LEVEL - 9)
#define L_IMM 					LEVEL_IMMORTAL


#define TIMER					50


/* And some bounty defines */

#define BOUNTY_PLACE_CHARGE 	3
#define BOUNTY_REMOVE_CHARGE 	20
#define BOUNTY_ADD_CHARGE 		5
#define MIN_BOUNTY 				100
#define MAX_BOUNTY				25000

// Fight positions, Upro 2/13/2010
#define FIGHT_NONE				0
#define FIGHT_PALM				1

// Common Material Defines
#define MAT_IRON				0
#define MAT_BRASS				1
#define MAT_BRONZE				2
#define MAT_STEEL				3
#define MAT_COPPER				4

//Guildranks 11/30/2011 - Should have done this a long time ago.
#define RANK_APPRENTICE			150
#define RANK_MEMBER				500
#define RANK_OFFICER			1500
#define RANK_MASTER				10000

//Boons, Upro 9/27/2011
#define BOON_AMBI				0 //Ambidexterity
#define BOON_RES_FIRE			1 //25%
#define BOON_RES_COLD			2
#define BOON_RES_SHOCK			3
#define BOON_RES_EARTH			4
#define BOON_RES_AIR			5
#define BOON_RES_DROWN			6
#define BOON_HARDINESS			7 //+1hp per lvl
#define BOON_BRILLIANCE			8 //+1mana per lvl
#define BOON_GUNSLINGER			9 //double wand zap rate, bonus to missile weaponry.


//Mat defines Upro 7/2010
#define IS_METAL(mat)			(mat_table[get_material(mat)].is_metal == TRUE)
#define IS_GEM(mat)				(mat_table[get_material(mat)].is_gem == TRUE)
#define IS_CLOTH(mat)			(mat_table[get_material(mat)].is_cloth == TRUE)
#define IS_WOOD(mat)			(mat_table[get_material(mat)].is_wood == TRUE)



//Event stuff Upro 1/22/2010
#define EVENT_NONE				0
#define EVENT_LEARN_SPELL		1
#define EVENT_MINING			2
#define EVENT_BREWING			3
#define EVENT_COOKING			4
#define EVENT_SAILING			5
#define EVENT_PROSPECTING		6
#define EVENT_FISHING			7
#define EVENT_WOOD_CUTTING		8
#define EVENT_DIGGING			9

//Economy stuff. Upro
#define MAX_WORLD_GOLD			99999 //this is in thousands. giving us 99999000 (99 million gold) to work with.

//Resource/Herb/Etc.. Junk: - Upro

#define ALL_RARITY				-1
#define VERY_COMMON				0
#define COMMON					1
#define UNCOMMON				2
#define	RARE					3
#define VERY_RARE				4



/* Added this for "orphaned help" code. Check do_help() -- JR */
#define MAX_CMD_LEN			50

//Time definitions
#define PULSE_PER_SECOND    		4
#define PULSE_SECOND				4
#define PULSE_SPELL_OVER_TIME		( 2 * PULSE_PER_SECOND)	  // 2 second delay
#define PULSE_VIOLENCE      		( 3 * PULSE_PER_SECOND)   // 3 second delay
#define PULSE_MOBILE        		( 4 * PULSE_PER_SECOND)   // 4 second delay
#define PULSE_WEATHER_EFFECT		( 5 * PULSE_PER_SECOND)   // 5 second delay
#define PULSE_MUSIC         		( 6 * PULSE_PER_SECOND)   // 6 second delay
#define PULSE_HEAL					(10 * PULSE_PER_SECOND)	  // ten seconds for healing
#define PULSE_WIND_EFFECT			(12 * PULSE_PER_SECOND)   // twelve seconds
#define PULSE_AUCTION       		(45 * PULSE_PER_SECOND)   /* 45 seconds */
#define PULSE_TICK          		(60 * PULSE_PER_SECOND)   // 1 minute.
#define PULSE_CLIMATE				(120 * PULSE_PER_SECOND)  // 2 minutes.
#define PULSE_AREA          		(120 * PULSE_PER_SECOND)  // 2 minutes.
#define PULSE_QUEUE               	( PULSE_PER_SECOND / 2)   /* Two chances per second. */
#define PULSE_UNDERWATER           	(20 * PULSE_PER_SECOND)
#define PULSE_EVENT      			( 8 * PULSE_PER_SECOND)
#define PULSE_BLEED					( 12 * PULSE_PER_SECOND )
#define PULSE_IRON_WILL				( 15 * PULSE_PER_SECOND ) 
#define PULSE_COOLDOWN				( 1 * PULSE_PER_SECOND )  //Cooldowns go down each second.
#define PULSE_WATER_DAMAGE			( 25 * PULSE_PER_SECOND ) //water damage to items.
#define PULSE_TREE_AGE				( 1200 * PULSE_PER_SECOND ) // trees have possibility to grow once ever 20 minutes.


//These are all cooldown times, but can be used for anything you want.

//Real time:
#define TIME_ONE_SECOND				(1)
#define TIME_FIVE_SECONDS			(5)
#define TIME_TEN_SECONDS			(10)
#define TIME_ONE_MINUTE				(60)
#define TIME_ONE_HOUR				(60 * 60)	
//Game time:
#define TIME_TWICE_PER_DAY			((24 * 60) / 2)
#define TIME_THREE_PER_DAY			((24 * 60) / 3)
#define TIME_ONE_DAY				(24 * 60)
#define TIME_ONE_WEEK				(168 * 60)
#define TIME_ONE_MONTH				((4 * 168) * 60)

#define IMPLEMENTOR MAX_LEVEL
#define CREATOR     (MAX_LEVEL - 1)
#define SUPREME     (MAX_LEVEL - 2)
#define DEITY       (MAX_LEVEL - 3)
#define GOD         (MAX_LEVEL - 4)
#define IMMORTAL    (MAX_LEVEL - 5)
#define DEMI        (MAX_LEVEL - 6)
#define ANGEL       (MAX_LEVEL - 7)
#define AVATAR      (MAX_LEVEL - 8)
#define HERO        LEVEL_HERO

/*
 * Language stuff - Upro
 */

#define CMN			    0
#define HUMAN           1
#define DWARVISH        2
#define ELVISH          3
#define GNOMISH         4
#define GOBLIN          5
#define ORCISH          6
#define OGRE            7
#define DROW            8
#define KOBOLD          9
#define TROLLISH        10
#define HALFLING        11



////////////////////////////Protocol Stuff/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////MXP (Mud Extension Protocol)////////////////////////////
																			///
#define USE_MXP(ch)		(IS_SET(ch->act, PLR_MXP) && ch->desc->mxp)			///
																			///
/* strings */																///
																			///
#define MXP_BEG "\x03"    /* becomes < */									///
#define MXP_END "\x04"    /* becomes > */									///
#define MXP_AMP "\x05"    /* becomes & */									///
																			///
/* characters */															///
																			///
#define MXP_BEGc '\x03'    /* becomes < */									///
#define MXP_ENDc '\x04'    /* becomes > */									///
#define MXP_AMPc '\x05'    /* becomes & */									///
																			///
/* constructs an MXP tag with < and > around it */							///
																			///
#define MXPTAG(arg) MXP_BEG arg MXP_END										///
																			///
#define ESC "\x1B"  /* esc character */										///
																			///
#define MXPMODE(arg) ESC "[" #arg "z"										///
																			///
//////////////////////MSSP (Mud Server Status Protocol)////////////////////////
																			///
struct mssp_info															///
{																			///
   char * hostname;															///
   char * contact;															///
   char * icon;																///
   char * language;															///
   char * location;															///
   char * website;															///
   char * family;															///
   char * genre;															///
   char * gamePlay;															///
   char * gameSystem;														///
   char * intermud;															///
   char * status;															///
   char * subgenre;															///
   char * equipmentSystem;													///
   char * multiplaying;														///
   char * playerKilling;													///
   char * questSystem;														///
   char * roleplaying;														///
   char * trainingSystem;													///
   char * worldOriginality;													///
   sh_int created;															///
   sh_int minAge;															///
   sh_int worlds;															///
   bool ansi;																///
   bool mccp;																///
   bool mcp;																///
   bool msp;																///
   bool ssl;																///
   bool mxp;																///
   bool pueblo;																///
   bool vt100;																///
   bool xterm256;															///
   bool pay2play;															///
   bool pay4perks;															///
   bool hiringBuilders;														///
   bool hiringCoders;														///
   bool adultMaterial;														///
   bool multiclassing;														///
   bool newbieFriendly;														///
   bool playerCities;														///
   bool playerClans;														///
   bool playerCrafting;														///
   bool playerGuilds;														///
};    																		///
																			///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/* flags for show_list_to_char */

enum {
  eItemNothing,   /* item is not readily accessible */
  eItemGet,     /* item on ground */
  eItemDrop,    /* item in inventory */
  eItemBid     /* auction item */
  };

						
//Class defines:
#define ch_class_WIZARD			0
#define ch_class_CLERIC			1
#define ch_class_THIEF			2
#define ch_class_FIGHTER		3
#define ch_class_DRUID			4
#define ch_class_RANGER			5
#define ch_class_PALADIN		6
#define ch_class_BARD			7
#define ch_class_MONK			8
#define ch_class_PSION			9

//Class defines: (version 2.... the lazy way heh)
#define WIZARD			0
#define CLERIC			1
#define THIEF			2
#define FIGHTER			3
#define DRUID			4
#define RANGER			5
#define PALADIN			6
#define BARD			7
#define MONK			8
#define PSION			9



//Class macros.
#define IS_WIZARD(ch)			(ch->ch_class == ch_class_WIZARD  || (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_WIZARD))
#define IS_CLERIC(ch)			(ch->ch_class == ch_class_CLERIC  || (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_CLERIC))
#define IS_THIEF(ch)			(ch->ch_class == ch_class_THIEF   || (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_THIEF))
#define IS_FIGHTER(ch)			(ch->ch_class == ch_class_FIGHTER || (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_FIGHTER))
#define IS_RANGER(ch)			(ch->ch_class == ch_class_RANGER  || (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_RANGER))

#define IS_DRUID(ch)			(ch->ch_class == ch_class_DRUID)
#define IS_PALADIN(ch)			(ch->ch_class == ch_class_PALADIN)
#define IS_BARD(ch)				(ch->ch_class == ch_class_BARD)
#define IS_MONK(ch)				(ch->ch_class == ch_class_MONK)
#define IS_PSION(ch)				(ch->ch_class == PSION || ch->ch_class2 == PSION)

//Item macros
#define IS_UNIQUE(obj)			(obj && IS_SET(obj->extra2_flags, ITEM_UNIQUE))


#define CHECK_UNIQUE(pObjIndex)	if (IS_UNIQUE(pObjIndex) && unique_exists(pObjIndex->vnum)) \
								{ \
									SEND("That object is {Bunique{x and already exists in the game world.\r\n",ch);	\
									return; \
								}

//Eye colors
#define EYE_BLUE	A
#define EYE_GREEN	B
#define EYE_HAZEL	C
#define EYE_BROWN	D
#define EYE_RED		E
#define EYE_BLACK	F
#define EYE_PURPLE	G

//Hair colors
#define HAIR_BLONDE		A
#define HAIR_BROWN		B
#define HAIR_BLACK		C
#define HAIR_RED		D
#define HAIR_WHITE		E
#define HAIR_GREY		F

//Hairstyles
#define STYLE_BALD		A
#define STYLE_BOWL		B
#define STYLE_MULLET	C
#define STYLE_MOHAWK	D
#define STYLE_CREW		E
#define STYLE_UNTAMED	F
#define STYLE_SPIKED	G
#define STYLE_CROPPED   H

#define STYLE_PONYTAIL  I

#define STYLE_BRAIDED   J
#define STYLE_DOUBLE_BRAIDED	K
#define STYLE_CURLY		L
#define STYLE_STRAIGHT	M
#define STYLE_SHORT		N		
#define STYLE_BUNS		O

//Facial Hair
#define FACIAL_NONE			A
#define FACIAL_SIDEBURNS	B
#define FACIAL_MUTTONCHOPS	C
#define FACIAL_GOATEE		D
#define FACIAL_MUSTACHE		E
#define FACIAL_BEARD_SHORT	F
#define FACIEL_BEARD_LONG	G


//#define IS_HUMANOID(ch)			(ch->race == 

#define IS_CASTER(ch)			(IS_WIZARD(ch) || IS_CLERIC(ch) || IS_DRUID(ch))
#define IS_MCLASSED(ch)			(ch->mClass == TRUE)

//Race defines:
#define RACE_HUMAN					1
#define RACE_ELF					2
#define RACE_DWARF					3
#define RACE_HALF_ORC				4
#define RACE_HALF_ELF				5
#define RACE_HALFLING				6
#define RACE_GNOME					7
#define RACE_DUERGAR				8
#define RACE_DROW					9
#define RACE_HALF_OGRE				10
#define RACE_GNOLL					11
#define RACE_MINOTAUR				12
#define RACE_AARAKOCRA				13
#define RACE_REVENANT				14
#define RACE_AASIMAR				15
#define RACE_BUGBEAR				16
#define RACE_TABAXI  				17
#define RACE_LIZARDMAN				18
#define RACE_TIEFLING				19
#define RACE_LOXODON				20
#define RACE_GOBLIN					75
#define RACE_HOBGOBLIN				76
#define RACE_KOBOLD					78
#define RACE_ORC					83



#define IS_HUMAN(ch)				(!str_cmp(race_table[ch->race].name, "human"))
#define IS_ELF(ch)					(!str_cmp(race_table[ch->race].name, "elf") || \
									 !str_cmp(race_table[ch->race].name, "drow"))
#define IS_SURFACE_ELF(ch)			(!str_cmp(race_table[ch->race].name, "elf"))
#define IS_DWARF(ch)				(!str_cmp(race_table[ch->race].name, "dwarf") || \
									 !str_cmp(race_table[ch->race].name, "duergar"))
#define IS_ORC(ch)					(!str_cmp(race_table[ch->race].name, "orc") || \
 									!str_cmp(race_table[ch->race].name, "half orc"))

#define IS_GOBLINOID(ch)			(!str_cmp(race_table[ch->race].name, "orc") || \
 									!str_cmp(race_table[ch->race].name, "half orc") || \
									!str_cmp(race_table[ch->race].name, "goblin") || \
									!str_cmp(race_table[ch->race].name, "kobold") || \
									!str_cmp(race_table[ch->race].name, "bugbear")  ||\
									!str_cmp(race_table[ch->race].name, "hobgoblin") || \
									!str_cmp(race_table[ch->race].name, "gnoll"))

#define IS_HALFLING(ch)				(!str_cmp(race_table[ch->race].name, "halfling"))
#define IS_GNOME(ch)				(!str_cmp(race_table[ch->race].name, "gnome"))
#define IS_DROW(ch)					(!str_cmp(race_table[ch->race].name, "drow"))
#define IS_DUERGAR(ch)				(!str_cmp(race_table[ch->race].name, "duergar"))
#define IS_HALF_OGRE(ch)			(!str_cmp(race_table[ch->race].name, "half ogre"))
#define IS_GNOLL(ch)				(!str_cmp(race_table[ch->race].name, "gnoll"))
#define IS_MINOTAUR(ch)				(!str_cmp(race_table[ch->race].name, "minotaur"))
#define IS_UNDEAD(ch)				(ch->race > 95 && ch->race < 106 || ch->race == RACE_REVENANT)
#define IS_DRAGON(ch)				(ch->race > 37 && ch->race < 57)
#define IS_ANGELIC(ch)				(ch->race > 56 && ch->race < 60 || ch->race == RACE_AASIMAR)
#define IS_GIANT(ch)				(ch->race > 147 && ch->race < 155)
#define IS_GOLEM(ch)				(ch->race > 143 && ch->race < 148)
#define IS_ELEMENTAL(ch)			(!str_cmp(race_table[ch->race].name, "elemental"))
#define IS_TROLL(ch)				(!str_cmp(race_table[ch->race].name, "troll"))
#define IS_BIRDFOLK(ch)				(!str_cmp(race_table[ch->race].name, "aarakocra"))
#define IS_AASIMAR(ch)				(!str_cmp(race_table[ch->race].name, "aasimar"))
#define IS_REVENANT(ch)				(!str_cmp(race_table[ch->race].name, "revenant"))
#define IS_BUGBEAR(ch)				(!str_cmp(race_table[ch->race].name, "bugbear"))
#define IS_TABAXI(ch)				(!str_cmp(race_table[ch->race].name, "tabaxi"))
#define IS_LIZARDMAN(ch)			(!str_cmp(race_table[ch->race].name, "lizardman"))
#define IS_TIEFLING(ch)				(!str_cmp(race_table[ch->race].name, "tiefling"))
#define IS_LOXODON(ch)				(!str_cmp(race_table[ch->race].name, "loxodon"))
#define IS_CENTAUR(ch)				(!str_cmp(race_table[ch->race].name, "centaur"))


#define IS_BIPED(ch)				(ch->race < MAX_PC_RACE || IS_GOBLINOID(ch))
//Determining species for skinning values and such.
#define WAS_DRAGON(obj)				((obj->corpse_race && obj->item_type == ITEM_CORPSE_NPC) && \
									 obj->corpse_race > 37 && obj->corpse_race < 57)
#define WAS_GIANT(obj)				((obj->corpse_race && obj->item_type == ITEM_CORPSE_NPC) && \
									 obj->corpse_race > 147 && obj->corpse_race < 155)
#define WAS_BIPED(obj)				((obj->corpse_race && obj->item_type == ITEM_CORPSE_NPC) && ((WAS_GIANT(obj)) || \
									 obj->corpse_race < MAX_PC_RACE))
#define WAS_ANGELIC(obj)			(obj->corpse_race > 56 && obj->corpse_race < 60)
#define WAS_UNDEAD(obj)				(obj->corpse_race > 95 && obj->corpse_race < 106)
									 
//Pet/mount junk:
#define IS_PET( ch )		( IS_NPC( ch ) 				\
				&& (( IS_SET( ch->act, ACT_PET )		) || IS_SET( ch->act2, ACT2_MOUNT )))	

		
//Horns and tusks 1/31/2020 Upro
#define HAS_TUSKS 1
#define HAS_HORNS 2
#define HAS_BOTH  4
		
//Alignment goodies:
#define ALIGN_LG	1000
#define ALIGN_CG	750
#define ALIGN_NG	500
#define ALIGN_LN	250
#define ALIGN_TN	0
#define ALIGN_CN	-250
#define ALIGN_NE	-500
#define ALIGN_CE	-750
#define ALIGN_LE	-1000

#define IS_LGOOD(ch)	(ch->alignment == ALIGN_LG)
#define IS_CGOOD(ch)	(ch->alignment == ALIGN_CG)
#define IS_NGOOD(ch)	(ch->alignment == ALIGN_NG)
#define IS_LNEUTRAL(ch)	(ch->alignment == ALIGN_LN)
#define IS_TNEUTRAL(ch)	(ch->alignment == ALIGN_TN)
#define IS_CNEUTRAL(ch)	(ch->alignment == ALIGN_CN)
#define IS_NEVIL(ch)	(ch->alignment == ALIGN_NE)
#define IS_CEVIL(ch)	(ch->alignment == ALIGN_CE)
#define IS_LEVIL(ch)	(ch->alignment == ALIGN_LE)

#define IS_GOOD(ch)		(ch->alignment > 250)
#define IS_EVIL(ch)		(ch->alignment < -250)
#define IS_NEUTRAL(ch)		(ch->alignment <= 250 && ch->alignment >= -250)

#define IS_LAWFUL(ch)	(IS_LGOOD(ch) || IS_LNEUTRAL(ch) || IS_LEVIL(ch))
#define IS_CHAOTIC(ch)	(IS_CGOOD(ch) || IS_CNEUTRAL(ch) || IS_CEVIL(ch))

#define IN_WATER_SECTOR(ch)			(ch->in_room->sector_type == SECT_WATER_SWIM || ch->in_room->sector_type == SECT_OCEANFLOOR || ch->in_room->sector_type == SECT_WATER_NOSWIM )

#define IN_DIGGING_SECTOR(ch)		(ch->in_room->sector_type != SECT_INSIDE && !IN_WATER_SECTOR(ch))
#define IN_MINING_SECTOR(ch)		(ch->in_room->sector_type == SECT_HILLS || ch->in_room->sector_type == SECT_MOUNTAIN \
									|| ch->in_room->sector_type == SECT_CAVE || ch->in_room->sector_type == SECT_UNDERGROUND)
#define IN_PROSPECTING_SECTOR(ch)	(ch->in_room->sector_type == SECT_WATER_SWIM || ch->in_room->sector_type == SECT_BEACH \
									|| ch->in_room->sector_type == SECT_SHORELINE)



//Misc Macros:
#define CAN_CLIMB(obj)		(obj->item_type == ITEM_BUILDING || obj->item_type == ITEM_TREE)


#define IS_DEVOTION(sn)				(is_devotion(sn))  //is it a psionic devotion?
#define IS_KNOWN(ch,sn)				(get_skill(ch,sn) > 0)
#define IS_SECONDARY_SKILL(ch,sn)   (skill_table[sn].rating[ch->ch_class] < 1 && skill_table[sn].rating[ch->ch_class2] > 0)
#define IS_PRIMARY_SKILL(ch, sn)	(skill_table[sn].rating[ch->ch_class] > 0 && skill_table[sn].rating[ch->ch_class2] < 1)
#define IS_CROSS_SKILL(ch, sn)		(skill_table[sn].rating[ch->ch_class] > 0 && skill_table[sn].rating[ch->ch_class2] > 0)
#define STAT_MOD(base,ch,stat,mod) 	((base - get_curr_stat(ch,stat)) * mod )								
#define KNOWS(ch,sn)				(get_skill(ch,sn) > 0)

										
#define COOLDOWN(ch, sn)			(ch->cooldowns[sn] = skill_table[sn].cooldown)
#define CAN_USE_SKILL(ch, sn)		(can_use_skill(ch,sn)) //are they in level range to use it/see it?
#define CHECK_COOLDOWN(ch, sn)		(ch->cooldowns[sn] > 0)			
#define STRALLOC(point) 	str_dup((point))

#define SIZE(ch)					(what_size(ch))

#define PAUSE    	system("PAUSE")

//Health Bar Macro

#define GAUGE(pct) ((pct) < 9 ? " " : \
					(pct) < 19 ? "- " : \
					(pct) < 29 ? "-- " : \
					(pct) < 39 ? "--- " : \
					(pct) < 49 ? "---- " : \
					(pct) < 59 ? "----- " : \
					(pct) < 69 ? "------ " : \
					(pct) < 79 ? "------- " : \
					(pct) < 89 ? "-------- " : \
					(pct) < 99 ? "--------- " : \
								 "----------")


/*
 * Utility macros.
 */
#define LOG(str)			(log_string(str))
#define IS_VALID(data)        ((data) != NULL && (data)->valid)
#define VALIDATE(data)        ((data)->valid = TRUE)
#define INVALIDATE(data)    ((data)->valid = FALSE)
#define UMIN(a, b)        ((a) < (b) ? (a) : (b))
#define UMAX(a, b)        ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)        ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define IS_NULLSTR(str)        ((str) == NULL || (str)[0] == '\0' || !str_cmp(str, "(null)"))
#define ENTRE(min,num,max)    ( ((min) < (num)) && ((num) < (max)) )
#define CHECK_POS(a, b, c)    {                            \
                    (a) = (b);                    \
                    if ( (a) < 0 )                    \
                    bug( "CHECK_POS : " c " == %d < 0", a );    \
                }                            
#define IS_VOWEL(c) (   (c) == 'a' || (c) == 'A'  \
                        || (c) == 'e' || (c) == 'E'  \
                        || (c) == 'i' || (c) == 'I'  \
                        || (c) == 'o' || (c) == 'O'  \
                        || (c) == 'u' || (c) == 'U')
							
						

#define CHECK_DATE(d, m)		( ( ( time_info.day + 1 ) == d ) && ( time_info.month == m ) )						

#define SEASON_SPRING			(time_info.month > 4 && time_info.month < 9)
#define SEASON_SUMMER			(time_info.month > 8 && time_info.month < 12)
#define SEASON_AUTUMN			(time_info.month > 11 && time_info.month < 17)
#define SEASON_FALL				(time_info.month > 11 && time_info.month < 17) //Just for my laziness.
#define SEASON_WINTER			(time_info.month > -1 && time_info.month < 5)

				
/*
 * Character macros.
 */
#define IS_NPC(ch)        		(IS_SET((ch)->act, ACT_IS_NPC))
#define LOW_HEALTH(ch)			(ch->hit < (ch->max_hit / 5))
#define LOW_MANA(ch)			(ch->mana < (ch->max_mana / 5))
#define LOW_MOVE(ch)			(ch->move < (ch->max_move / 5))
#define LOW_TALENT(ch)			(ch->pp < (ch->max_pp / 5))
#define IS_IMMORTAL(ch)        	(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)        		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)    (get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)		(IS_SET((ch)->affected_by, (sn)))
#define GET_AGE(ch)				(ch->age)
#define SET_COOLDOWN(ch, sn)	(ch->cooldowns[sn] = skill_table[sn].cooldown)

#define IS_AWAKE(ch)        	(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)        	((ch)->armor[type]                \
                + ( IS_AWAKE(ch)                \
            ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))  

//Equipment slot macros
#define HAS_TAIL(ch)			(IS_GNOLL(ch) || IS_TABAXI(ch) || IS_LIZARDMAN(ch))
			
//Base Stats:
#define BASE_STR(ch)				(ch->perm_stat[STAT_STR])
#define BASE_INT(ch)				(ch->perm_stat[STAT_INT])
#define BASE_WIS(ch)				(ch->perm_stat[STAT_WIS])
#define BASE_DEX(ch)				(ch->perm_stat[STAT_DEX])
#define BASE_CON(ch)				(ch->perm_stat[STAT_CON])
#define BASE_CHA(ch)				(ch->perm_stat[STAT_CHA])
#define GET_BASE(ch, stat)			(ch->perm_stat[stat])
			
//Curr Stats:
#define GET_STR(ch)				(get_curr_stat(ch, STAT_STR))
#define GET_INT(ch)				(get_curr_stat(ch, STAT_INT))
#define GET_WIS(ch)				(get_curr_stat(ch, STAT_WIS))
#define GET_DEX(ch)				(get_curr_stat(ch, STAT_DEX))
#define GET_CON(ch)				(get_curr_stat(ch, STAT_CON))
#define GET_CHA(ch)				(get_curr_stat(ch, STAT_CHA))
#define GET_STAT(ch, stat)		(get_curr_stat(ch, stat))


#define PRIME_STAT(ch, secondary)		(secondary == FALSE ? ch_class_table[ch->ch_class].attr_prime : ch_class_table[ch->ch_class2].attr_prime)

#define SECOND_STAT(ch, secondary)		(secondary == FALSE ? ch_class_table[ch->ch_class].attr_second : ch_class_table[ch->ch_class2].attr_second)

#define SPELL_CRIT(ch)			(get_spell_crit_chance(ch))
#define MELEE_CRIT(ch)			(get_melee_crit_chance(ch))

#define GROUP_SIZE(ch)			(group_size(ch))
#define ENCUMBERED(ch)			(get_carry_weight(ch) >= can_carry_w(ch))

//Drunk things.
#define DRINKS(ch)				(!IS_NPC(ch) ? ch->pcdata->condition[COND_DRUNK] : 0)  //How many have they had?

#define IS_BUZZED(ch)			(ch->pcdata->condition[COND_DRUNK] > 2 ? TRUE : FALSE)
#define IS_TIPSY(ch)			(ch->pcdata->condition[COND_DRUNK] > 5 ? TRUE : FALSE)
#define IS_DRUNK(ch)			(ch->pcdata->condition[COND_DRUNK] > 9 ? TRUE : FALSE)
#define IS_HAMMERED(ch)			(ch->pcdata->condition[COND_DRUNK] > 12 ? TRUE : FALSE)


#define GET_ALIGN_STRING(ch)	(ch->alignment == 1000 ? "lawful good" : \
								 ch->alignment == 750 ? "chaotic good" : \
								 ch->alignment == 500 ? "neutral good" : \
								 ch->alignment == 250 ? "lawful neutral" : \
								 ch->alignment == 0 ? "true neutral" : \
								 ch->alignment == -250 ? "chaotic neutral" : \
								 ch->alignment == -500 ? "neutral evil" : \
								 ch->alignment == -750 ? "chaotic evil" : \
								 ch->alignment == -1000 ? "lawful evil" : \
								 "bad alignment")
								 
#define GUILD_RANK_STRING(ch)	(ch->guildrank == 0 ? "Initiate" : \
								 ch->guildrank == 1 ? "Apprentice" : \
								 ch->guildrank == 2 ? "Member" : \
								 ch->guildrank == 3 ? "Officer" : \
								 ch->guildrank == 4 ? "Master" : \
								 ch->guildrank == 5 ? "Leader" : \
								 "Nothing")

#define POSITION_STRING(ch)		(ch->position == POS_DEAD ? "You are DEAD!!" : \
								 ch->position == POS_MORTAL ? "You are mortally wounded." : \
								 ch->position == POS_INCAP ? "You are incapacitated." : \
								 ch->position == POS_STUNNED ? "You are stunned." : \
								 ch->position == POS_SLEEPING ? "You are sleeping." : \
								 ch->position == POS_RESTING ? "You are resting." : \
								 ch->position == POS_SITTING ? "You are sitting." : \
								 ch->position == POS_STANDING ? "You are standing." : \
								 ch->position == POS_FIGHTING ? "You are fighting!" : \
								 "None")

#define SHORT_POSITION_STRING(ch)		(ch->position == POS_DEAD ? "{RDEAD{x" : \
								 ch->position == POS_MORTAL ? "{rMortal{x" : \
								 ch->position == POS_INCAP ? "Incapacitated" : \
								 ch->position == POS_STUNNED ? "Stunned" : \
								 ch->position == POS_SLEEPING ? "Sleeping" : \
								 ch->position == POS_RESTING ? "Resting" : \
								 ch->position == POS_SITTING ? "Sitting" : \
								 ch->position == POS_STANDING ? "Standing" : \
								 ch->position == POS_FIGHTING ? "Fighting" : \
								 "None")
								 
#define STAT_STRING(stat)		(stat == STAT_STR ? "Strength" : \
								 stat == STAT_INT ? "Intelligence" : \
								 stat == STAT_WIS ? "Wisdom" : \
								 stat == STAT_DEX ? "Dexterity" : \
								 stat == STAT_CON ? "Constitution" : \
								 stat == STAT_CHA ? "Charisma" : \
								 "None")


#define CON_CUSTOMIZE_APPEARANCE		-33
#define CON_GET_NEW_NAME				-32
#define CON_CONFIRM_NEW_PASSWORD		-31
#define CON_GET_PASS					-30
#define CON_GET_EMAIL					-29
#define CON_ACC_VERIFY					-28
#define CON_CONFIRM_NEW_ACC_NAME		-27
#define CON_CHNG_PASS					-26
#define CON_DEL_ACC						-25
#define CON_PLAY_CHAR					-24
#define CON_CONFIRM_DEL_ACC				-23
#define CON_SEND_PASS					-22
#define CON_EXTRACTED					-21
#define CON_INTRO						-20
#define CON_GET_BOON					-19
#define CON_GET_ETHOS					-18
#define CON_DECIDE_STATS				-17	//Will you roll stats or take a template?
#define CON_GET_EYES					-16
#define CON_GET_HAIR					-15
#define CON_GET_HAIRSTYLE				-14
#define CON_GET_FACIAL_HAIR				-13
#define CON_GET_OLD_ACCOUNT				-12
#define CON_ACCOUNT						-11
#define CON_GET_FAVORED_ENEMY			-10 //Ranger favored enemy.
#define CON_GET_NAME					-9
#define	CON_GET_LAST_NAME				-8
#define CON_GET_OLD_PASSWORD			-7
#define CON_CONFIRM_NEW_NAME			-6
#define CON_GET_NEW_RACE				-5
#define CON_GET_NEW_SEX					-4
#define CON_GET_NEW_CLASS				-3
#define CON_GET_ALIGNMENT				-2
#define CON_PICK_WEAPON					-1
#define CON_PLAYING						0
#define CON_READ_IMOTD					1
#define CON_READ_MOTD					2
#define CON_BREAK_CONNECT				3
#define CON_COPYOVER_RECOVER			4
#define CON_ELEMENTAL_SPECIALIZATION    5
#define CON_WEAPON_SPECIALIZATION 		6
#define CON_ROLL_STATS					7
#define CON_GET_GOD                     8
#define CON_VERIFY_EMAIL				9
#define CON_TUTORIAL					10

								 
#define GET_CONNECTED_STRING(d)	(	d->connected == -33 ? "customize appearance?" : \
									d->connected == -32 ? "get new name" : \
									d->connected == -31 ? "confirm new password" : \
									d->connected == -30 ? "get password" : \
									d->connected == -29 ? "get email" : \
									d->connected == -28 ? "account verify" : \
									d->connected == -27 ? "confirm new account name" : \
									d->connected == -26 ? "change password" : \
									d->connected == -25 ? "delete account password" : \
									d->connected == -24 ? "play char" : \
									d->connected == -23 ? "confirm delete account" : \
									d->connected == -22 ? "send password" : \
									d->connected == -21 ? "extracted" : \
									d->connected == -20 ? "intro screen" : \
									d->connected == -19 ? "pick boon" : \
									d->connected == -18 ? "get align ethos" : \
									d->connected == -17 ? "decide stats" : \
									d->connected == -16 ? "get eyes" : \
									d->connected == -15 ? "get hair" : \
									d->connected == -14 ? "get hairstyle" : \
									d->connected == -13 ? "get facial hair" : \
									d->connected == -12 ? "get old account" : \
									d->connected == -11 ? "account screen" : \
									d->connected == -10 ? "get favored enemy" : \
									d->connected == -9  ? "name" : \
									d->connected == -8  ? "last name" : \
									d->connected == -7  ? "get old password" : \
									d->connected == -6  ? "confirm new name" : \
									d->connected == -5  ? "pick race" : \
									d->connected == -4  ? "pick gender" : \
									d->connected == -3  ? "pick class" : \
									d->connected == -2  ? "get alignment" : \
									d->connected == -1  ? "pick weapon" : \
									d->connected == 0   ? "playing" : \
									d->connected == 1   ? "read imotd" : \
									d->connected == 2   ? "read motd" : \
									d->connected == 3   ? "break connect" : \
									d->connected == 4   ? "copyover recover" : \
									d->connected == 5   ? "elemental spec" : \
									d->connected == 6   ? "weapon spec" : \
									d->connected == 7   ? "roll stats" : \
									d->connected == 8   ? "pick deity" : \
									d->connected == 9   ? "verify email" : \
									d->connected == 10   ? "tutorial" : \
									"bad state")								 
								 
								 
								 
#define GET_HITROLL(ch)    	(	(ch)->hitroll \
								+ str_app[get_curr_stat(ch,STAT_STR)].tohit \
								+ dex_app[GET_DEX(ch)].tohit)
#define GET_DAMROLL(ch) 	(	(ch)->damroll \
								+ str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IS_OUTSIDE(ch)        (!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS) && ch->in_room->sector_type != SECT_INSIDE)

#define WAIT_STATE(ch, npulse)    ((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)    ((ch)->carry_weight + (ch)->silver/10 +  \
                              (ch)->gold * 2 / 5)

#define act(format,ch,arg1,arg2,type)\
    act_new((format),(ch),(arg1),(arg2),(type),POS_RESTING)
	
#define HAS_COOLDOWNS(ch)			(has_cooldowns(ch) == TRUE)
	
#define HAS_TRIGGER_MOB(ch,trig)    (IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_TRIGGER_OBJ(obj,trig) (IS_SET((obj)->pIndexData->oprog_flags,(trig)))
#define HAS_TRIGGER_ROOM(room,trig) (IS_SET((room)->rprog_flags,(trig)))
#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )
#define IS_BUILDER(ch, Area)    ( !IS_NPC(ch) && !IS_SWITCHED( ch ) &&      \
                ( ch->pcdata->security >= Area->security  \
                || strstr( Area->builders, ch->name )      \
                || strstr( Area->builders, "All" ) ) )
				
//Plane Stuff Upro 1/27/2020
#define PLANE_MATERIAL		1
#define PLANE_ETHEREAL		2
#define PLANE_ASTRAL		3
#define PLANE_SHADOW		4
#define PLANE_FIRE			5
#define PLANE_WATER			6
#define PLANE_AIR			7
#define PLANE_EARTH			8
#define PLANE_NINE_HELLS	9
#define PLANE_SEVEN_HEAVENS 10
#define MAX_PLANE		11


/*
 * Object macros.
 */
 
 #define IS_WEAPON(obj)		(obj && obj->item_type == ITEM_WEAPON)
 #define IS_SHIELD(obj)		(obj && obj->item_type == ITEM_ARMOR && IS_SET((obj)->wear_flags,ITEM_WEAR_SHIELD))
 

//Instrument Macros 1/13/2020 Upro	
#define IS_LUTE(obj)		(obj && obj->item_type == ITEM_INSTRUMENT && obj->value[0] == INSTR_LUTE)
#define IS_HARP(obj)		(obj && obj->item_type == ITEM_INSTRUMENT && obj->value[0] == INSTR_HARP)
#define IS_DRUMS(obj)		(obj && obj->item_type == ITEM_INSTRUMENT && obj->value[0] == INSTR_DRUMS)
#define IS_PICCOLO(obj)		(obj && obj->item_type == ITEM_INSTRUMENT && obj->value[0] == INSTR_PICCOLO)
#define IS_HORN(obj)		(obj && obj->item_type == ITEM_INSTRUMENT && obj->value[0] == INSTR_HORN)
#define IS_INSTRUMENT(obj)	(obj && obj->item_type == ITEM_INSTRUMENT)

/*  obj/wpn flags  */
#define IS_OBJ_FLAG(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WPN_FLAG(obj,stat)(IS_SET((obj)->value[4],(stat)))

#define CAN_WEAR(obj, part)       (IS_SET((obj)->wear_flags,  (part)))
#define WEARING_SHIELD(ch)		  (get_eq_char(ch, WEAR_SHIELD) && CAN_WEAR(get_eq_char(ch,ITEM_WEAR_SHIELD),ITEM_WEAR_SHIELD))
#define IS_OBJ_STAT(obj, stat)    (IS_SET((obj)->extra_flags, (stat)) || IS_SET((obj)->extra2_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)  (IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)    ((obj)->item_type == ITEM_CONTAINER ? \
    (obj)->value[4] : 100)

#define CAN_PUSH(obj)		(IS_SET(obj->extra2_flags, ITEM_CAN_PUSH))
#define CAN_PULL(obj)		(IS_SET(obj->extra2_flags, ITEM_CAN_PULL))
#define CAN_PRY(obj)		(IS_SET(obj->extra2_flags, ITEM_CAN_PRY))
#define CAN_PRESS(obj)		(IS_SET(obj->extra2_flags, ITEM_CAN_PRESS))

#define IS_BUOYANT(obj)		( IS_SET(obj->extra2_flags, ITEM_BUOYANT) || mat_table[get_material(obj->material)].buoyant == TRUE )


/*
 * Description macros.
 */
#define PERS(ch, looker)   ( can_see( looker, ch ) ? ( IS_NPC( ch ) ? ( ch )->short_descr : ( ch )->name ) \
						   : (!IS_NPC( ch ) && IS_IMMORTAL( ch ) && !IS_AFFECTED( looker, AFF_BLIND ) \
							? "An Immortal" : "Someone" ) ) 



// Weather Macros
#define PRECIPITATING(area)			(area->curr_precip > -1)
#define RAINING(area)				(area->curr_precip == RAIN)
#define SNOWING(area)				(area->curr_precip == SNOW)
#define HAILING(area)				(area->curr_precip == HAIL)
#define SLEETING(area)				(area->curr_precip == SLEET)
							
							
/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define CREATE(result, type, number)                                    \
do                                                                      \
{                                                                       \
   if (!((result) = (type *) calloc ((number), sizeof(type))))          \
   {                                                                    \
      perror("malloc failure");                                         \
      fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
      abort();                                                          \
   }                                                                    \
} while(0)

#define LINK(link, first, last, next, prev) \
do                                          \
{                                           \
   if ( !(first) )                          \
   {                                        \
      (first) = (link);                     \
      (last) = (link);                      \
   }                                        \
   else                                     \
      (last)->next = (link);                \
   (link)->next = NULL;                     \
   if ((first) == (link))                   \
      (link)->prev = NULL;                  \
   else                                     \
      (link)->prev = (last);                \
   (last) = (link);                         \
} while(0)

#define INSERT(link, insert, first, next, prev) \
do                                              \
{                                               \
   (link)->prev = (insert)->prev;               \
   if ( !(insert)->prev )                       \
      (first) = (link);                         \
   else                                         \
      (insert)->prev->next = (link);            \
   (insert)->prev = (link);                     \
   (link)->next = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)   \
do                                              \
{                                               \
   if ( !(link)->prev )                         \
   {                                            \
      (first) = (link)->next;                   \
      if ((first))                              \
         (first)->prev = NULL;                  \
   }                                            \
   else                                         \
   {                                            \
      (link)->prev->next = (link)->next;        \
   }                                            \
   if ( !(link)->next )                         \
   {                                            \
      (last) = (link)->prev;                    \
      if((last))                                \
         (last)->next = NULL;                   \
   }                                            \
   else                                         \
   {                                            \
      (link)->next->prev = (link)->prev;        \
   }                                            \
} while(0)								

#define UNLINK_SINGLE(pdata,pnext,type,list) \
do                                          \
{                                          \
	if (list == pdata)                      \
	{                                       \
		list = pdata->pnext;                 \
	}                                       \
	else                                    \
	{                                       \
		type *prev;                         \
		for (prev = list; prev != NULL; prev = prev->pnext) \
		{                                   \
			if (prev->pnext == pdata)        \
			{                               \
				prev->pnext = pdata->pnext;   \
				break;                      \
			}                               \
		}                                   \
		if (prev == NULL)                   \
		{                                   \
			bugf (#pdata " not found in " #list "."); \
		}                                   \
	}                                       \
} while(0)
 
#define LINK_SINGLE(pdata,pnext,list) \
do \
{ \
		pdata->pnext = list; \
		list = pdata; \
} \
while (0)

#define DISPOSE(point)                      \
do                                          \
{                                           \
   if( (point) )                            \
   {                                        \
      free( (point) );                      \
      (point) = NULL;                       \
   }                                        \
} while(0)

#define STRFREE(point)		DISPOSE((point))							




/*
 * ColoUr stuff v2.0, by Lope.
 */
#define CLEAR       "\e[0m"        /* Resets Colour    */
#define C_RED       "\e[0;31m"    /* Normal Colours    */
#define C_GREEN     "\e[0;32m"
#define C_YELLOW    "\e[0;33m"
#define C_BLUE      "\e[0;34m"
#define C_MAGENTA   "\e[0;35m"
#define C_CYAN      "\e[0;36m"
#define C_WHITE     "\e[0;37m"
#define C_D_GREY    "\e[1;30m"      /* Light Colors        */
#define C_B_RED     "\e[1;31m"
#define C_B_GREEN   "\e[1;32m"
#define C_B_YELLOW  "\e[1;33m"
#define C_B_BLUE    "\e[1;34m"
#define C_B_MAGENTA "\e[1;35m"
#define C_B_CYAN    "\e[1;36m"
#define C_B_WHITE   "\e[1;37m"
#define BG_BLACK	"\e[0;40m"
#define BG_RED		"\e[0;41m"
#define BG_GREEN  	"\e[0;42m"
#define BG_YELLOW	"\e[0;43m"
#define BG_BLUE		"\e[0;44m"
#define BG_MAGENTA	"\e[0;45m"
#define BG_CYAN		"\e[0;46m"
#define BG_WHITE	"\e[0;47m"
#define FRAMED      "\e[0;51m"

#define COLOUR_NONE 7        /* White, hmm...    */
#define RED         1        /* Normal Colours    */
#define GREEN       2
#define YELLOW      3
#define BLUE        4
#define MAGENTA     5
#define CYAN        6
#define WHITE       7
#define BLACK       0

#define NORMAL      0        /* Bright/Normal colours */
#define BRIGHT      1

#define ALTER_COLOUR( type )    if( !str_prefix( argument, "red" ) )        \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = RED;            \
                }                        \
                else if( !str_prefix( argument, "hi-red" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = RED;            \
                }                        \
                else if( !str_prefix( argument, "green" ) )    \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = GREEN;        \
                }                        \
                else if( !str_prefix( argument, "hi-green" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = GREEN;        \
                }                        \
                else if( !str_prefix( argument, "yellow" ) )    \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = YELLOW;        \
                }                        \
                else if( !str_prefix( argument, "hi-yellow" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = YELLOW;        \
                }                        \
                else if( !str_prefix( argument, "blue" ) )    \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = BLUE;        \
                }                        \
                else if( !str_prefix( argument, "hi-blue" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = BLUE;        \
                }                        \
                else if( !str_prefix( argument, "magenta" ) )    \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = MAGENTA;        \
                }                        \
                else if( !str_prefix( argument, "hi-magenta" ) ) \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = MAGENTA;        \
                }                        \
                else if( !str_prefix( argument, "cyan" ) )    \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = CYAN;            \
                }                        \
                else if( !str_prefix( argument, "hi-cyan" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = CYAN;            \
                }                        \
                else if( !str_prefix( argument, "white" ) )    \
                {                        \
                    ch->pcdata->type[0] = NORMAL;        \
                    ch->pcdata->type[1] = WHITE;        \
                }                        \
                else if( !str_prefix( argument, "hi-white" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = WHITE;        \
                }                        \
                else if( !str_prefix( argument, "grey" ) )    \
                {                        \
                    ch->pcdata->type[0] = BRIGHT;        \
                    ch->pcdata->type[1] = BLACK;        \
                }                        \
                else if( !str_prefix( argument, "beep" ) )    \
                {                        \
                    ch->pcdata->type[2] = 1;            \
                }                        \
                else if( !str_prefix( argument, "nobeep" ) )    \
                {                        \
                    ch->pcdata->type[2] = 0;            \
                }                        \
                else                        \
                {                        \
        SEND_bw( "Unrecognised colour, unchanged.\r\n", ch );    \
                    return;                    \
                }

#define LOAD_COLOUR( field )    ch->pcdata->field[1] = fread_number( fp );    \
                if( ch->pcdata->field[1] > 100 )        \
                {                        \
                    ch->pcdata->field[1] -= 100;        \
                    ch->pcdata->field[2] = 1;            \
                }                        \
                else                        \
                {                        \
                    ch->pcdata->field[2] = 0;            \
                }                        \
                if( ch->pcdata->field[1] > 10 )            \
                {                        \
                    ch->pcdata->field[1] -= 10;            \
                    ch->pcdata->field[0] = 1;            \
                }                        \
                else                        \
                {                        \
                    ch->pcdata->field[0] = 0;            \
                }

/*
 * Thanks Dingo for making life a bit easier ;)
 */
#define CH(d)                   ((d)->original ? (d)->original : (d)->character)


#define MAX_SONGS	20
#define MAX_LINES	100 /* this boils down to about 1k per song */
#define MAX_GLOBAL	10  /* max songs the global jukebox can hold */

struct song_data
{
    char *group;
    char *name;
    char *lyrics[MAX_LINES];
    int lines;
};

extern struct song_data song_table[MAX_SONGS];






/*
 * Site ban structure.
 */

#define BAN_SUFFIX     A
#define BAN_PREFIX     B
#define BAN_NEWBIES    C
#define BAN_ALL        D    
#define BAN_PERMIT     E
#define BAN_PERMANENT  F

struct    ban_data
{
    BAN_DATA *  next;
    bool        valid;
    sh_int      ban_flags;
    sh_int      level;
    char *      name;
};

struct bidder_data
{
  CHAR_DATA  	*who;
  BIDDER_DATA	*next;
  bool		valid;
  long	     	max_bid;
  long		current_bid;
  long		opening_bid;
  long		time_opening_bid;
  long		time_last_bid;
};
 
struct auction_data
{
    AUCTION_DATA *next;
    CHAR_DATA	*seller;
    OBJ_DATA	*item;
    BIDDER_DATA *bidders;
    bool	valid;
    char	*notes;
    time_t	time_opened;
    time_t	time_closed;
    sh_int	number_for_sale;
    sh_int	auction_number;
    long	auction_id;
    long	reserve_price;
    long	opening_price;
    long	bid_increment;
    long	high_bid;
    long	low_bid;
    bool	expired;
    int         going;
};


typedef struct twitter_data		TWITTER_DATA;

struct twitter_data
{
	TWITTER_DATA 	*next;
	TWITTER_DATA 	*prev;
	char		 	*author;
	char			*message;
	char			*date;
};

typedef struct  change_data             CHANGE_DATA;

struct change_data
{
  CHANGE_DATA *next;
  CHANGE_DATA *prev;
  char        *imm;
  char        *text;
  char        *date;
};

//Prog stuff.

#define MAX_IFS 20 /* should always be generous */
#define IN_IF 0
#define IN_ELSE 1
#define DO_IF 2
#define DO_ELSE 3

#define MAX_PROG_NEST 20

extern SLEEP_DATA *first_sleep;
extern SLEEP_DATA *last_sleep; /* - */
extern SLEEP_DATA *current_sleep; /* - */


/* Used to store sleeping mud progs. -rkb */
typedef enum {MP_MOB, MP_ROOM, MP_OBJ} mp_types;

struct sleep_data
{

	SLEEP_DATA * next;
	SLEEP_DATA * prev;

	int timer; /* Pulses to sleep */
	mp_types type; /* Mob, Room or Obj prog */
	ROOM_INDEX_DATA*room; /* Room when type is MP_ROOM */
	long vnum;
	int line;
	int valid;
	PROG_CODE * prog;

	/* mprog_driver state variables */
	int ignorelevel;
	int iflevel;
	bool ifstate[MAX_IFS][DO_ELSE+1];

	/* mprog_driver arguments */
	char * com_list;
	CHAR_DATA * mob;
	CHAR_DATA * ch;
	OBJ_DATA * obj;
	void * vo;
	bool single_step;
};

struct craft_data
{
	char * type;		//what kind of item?
	int item_type;		//it's item_type
	bool use_gem;
	bool use_cloth;
	bool use_wood;
	bool use_metal;		//can you make it from metal?
	int mat_req;		//How much material does it take?
};

struct mob_stat_type
{
    int hp;
    int mana;
    int dam;
    int hit;
    int ac;
};


struct game_data
{
	int 	most_players;
	
	long 	mkills;
	char 	* mkiller;
	
	int		pkills;
	char 	* pkiller;
	
	int		pdeaths;
	char	* pdeath;
	
	long		explored;
	char	* explorer;
	
	int		most_hours;
	char	* most_hour;
	
};

struct file_data
{
	FILE_DATA *next;
	FILE *fp;
	char *filename;
	char *mode;
 
	// *Where they were called from* //
	char *file;
	char *function;
	int line;
};

struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    sh_int      size;   /* size in k */
    char *      string; /* buffer's string */
};


struct bounty_data
{
	BOUNTY_DATA *next;
	bool valid;
	char * name;
	int amount;
};




#define MAX_MAP 77 // Shows 1/2 the given rooms (links) Always odd
#define MAP_MID 38 // Always 2x+1 to get MAX_MAP (makes the middle
#define MAX_MAP_HASH 16

// -1 is already taken, all positives are taken, so start lower
#define NS_2WAY		-2
#define NS_1WAYN	-3
#define NS_1WAYS	-4
#define NS_HIT		-5
#define EW_2WAY		-6
#define EW_1WAYE	-7
#define EW_1WAYW	-8
#define EW_HIT		-9
#define NS_2WAYD	-10
#define NS_1WAYND	-11
#define NS_1WAYSD	-12
#define NS_HITD		-13
#define EW_2WAYD	-14
#define EW_1WAYED	-15
#define EW_1WAYWD	-16
#define EW_HITD		-17
#define NS_UNN		-18
#define NS_UNS		-19
#define EW_UNE		-20
#define EW_UNW		-21
#define NS_UNND		-22
#define NS_UNSD		-23
#define EW_UNED		-24
#define EW_UNWD		-25

// Structures that are used.
typedef struct	map_qd		MAP_QD;
typedef struct	map_visit	MAP_VISIT;
typedef struct	map_param	MAP_PARAM;

// External information, used by many things.

extern MAP_QD *map_qd_free;
extern MAP_QD *map_queue;
extern MAP_VISIT *map_visit[MAX_MAP_HASH];
extern MAP_VISIT *map_visit_free;
extern int top_map_visit;
extern int top_map_qd;
//extern int map[MAX_MAP][MAX_MAP];
extern MAP_PARAM *mp;


//Game info stuff:
extern bool newlock;
extern bool wizlock;


/*
 * Time and weather stuff.
 */
#define SUN_DARK       0
#define SUN_RISE       1
#define SUN_LIGHT      2
#define SUN_SET        3

#define SKY_CLOUDLESS		    	0
#define SKY_CLOUDY		    		1
#define SKY_RAINING		    		2
#define SKY_LIGHTNING		   		3
#define SKY_SNOWING                 4
#define SKY_BLIZZARD                5
#define SKY_FOGGY                   6
#define SKY_HAILSTORM               7
#define SKY_THUNDERSTORM            8
#define SKY_ICESTORM                9

struct    time_info_data
{
    int        hour;
    int        day;
    int        month;
    int        year;
};

struct		faction_data
{
	char *		name;		/* faction name */
	int			tax_rate;	//tax rate of this faction (including all townships within)
	char *		areas[MAX_AREA_PER_FACTION];
};

//Faction Flags:

#define SOC_SET_TAX					(A)	// Can set the taxes in a city.
#define SOC_PARDON					(B)	// Can pardon people of their crimes.
#define SOC_LAW						(C)	// Are a law figure, and can arrest/fine people.
#define SOC_CAN_GRANT				(D)	// Means they can grant social status up to 2 levels below their own.
#define SOC_DECLARE_WAR				(E)	// Can they declare war on other factions?
#define SOC_CAN_BANISH				(F)	// Can they banish from the city?
#define SOC_CAN_OUTCAST				(G)	// Can they outcast from the faction?
#define SOC_CAN_CLAIM				(H)	// Can they claim land for the faction?
#define SOC_CAN_TRADE				(I) // Can negotiate trade agreements with other factions.
#define SOC_CAN_NATURALIZE			(J)	// They can make people citizens of a faction/city.
#define SOC_CAN_PURCHASE			(K) // Authorized to make purchases for hometown using city money
#define SOC_CAN_BUILD				(L) // Authorized to build in any area of the faction.

struct		society_data
{
	char *		male_name;
	char *		female_name;
	long		flags;
};

struct		resource_data
{
	char * 		material;		/* mat name */
	short int 	cost;  		/* in pounds of raw material */
	short int 	rarity; 	
	int			bulk;		//material bulk.
	bool		is_metal;
	bool		is_cloth;
	bool		is_wood;
	bool		is_gem;
	int			durability;
	bool 		buoyant;     //can it float on water?
	
};




struct    weather_data
{
    int        mmhg;
    int        change;
    int        sky;
    int        sunlight;	
};

/*
 * Connected state for a channel.
 */
#define CON_GET_NEW_NAME				-32
#define CON_CONFIRM_NEW_PASSWORD		-31
#define CON_GET_PASS					-30
#define CON_GET_EMAIL					-29
#define CON_ACC_VERIFY					-28
#define CON_CONFIRM_NEW_ACC_NAME		-27
#define CON_CHNG_PASS					-26
#define CON_DEL_ACC						-25
#define CON_PLAY_CHAR					-24
#define CON_CONFIRM_DEL_ACC				-23
#define CON_SEND_PASS					-22
#define CON_EXTRACTED					-21
#define CON_INTRO						-20
#define CON_GET_BOON					-19
#define CON_GET_ETHOS					-18
#define CON_DECIDE_STATS				-17	//Will you roll stats or take a template?
#define CON_GET_EYES					-16
#define CON_GET_HAIR					-15
#define CON_GET_HAIRSTYLE				-14
#define CON_GET_FACIAL_HAIR				-13
#define CON_GET_OLD_ACCOUNT				-12
#define CON_ACCOUNT						-11
#define CON_GET_FAVORED_ENEMY			-10 //Ranger favored enemy.
#define CON_GET_NAME					-9
#define	CON_GET_LAST_NAME				-8
#define CON_GET_OLD_PASSWORD			-7
#define CON_CONFIRM_NEW_NAME			-6
#define CON_GET_NEW_RACE				-5
#define CON_GET_NEW_SEX					-4
#define CON_GET_NEW_CLASS				-3
#define CON_GET_ALIGNMENT				-2
#define CON_PICK_WEAPON					-1
#define CON_PLAYING						0
#define CON_READ_IMOTD					1
#define CON_READ_MOTD					2
#define CON_BREAK_CONNECT				3
#define CON_COPYOVER_RECOVER			4
#define CON_ELEMENTAL_SPECIALIZATION    5
#define CON_WEAPON_SPECIALIZATION 		6
#define CON_ROLL_STATS					7
#define CON_GET_GOD                     8
#define CON_VERIFY_EMAIL				9
#define CON_TUTORIAL					10

/*
 * Descriptor (channel) structure.
 */
struct    descriptor_data
{		
	//char *get_host();
    //char *get_host(CHAR_DATA *ch);

    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    ACCOUNT_DATA *	account;
    bool		valid;
    bool		mxp;
    char *      host;
    char *      hostmask;
    char * 		ip_string;

    sh_int		descriptor;
    sh_int		connected;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    int			outsize;
    int			outtop;
    char *		showstr_head;
    char *		showstr_point;    
    
    void *              pEdit;		/* OLC */
    char **             pString;	/* OLC */
    int			editor;		        /* OLC */
    sh_int		strip;
    sh_int		color;
    bool		pload;
	protocol_t *        pProtocol;
};


/*
 * Attribute bonus structures.
 */
struct    str_app_type
{
    sh_int    tohit;
    sh_int    todam;
    sh_int    carry;
    sh_int    wield;
};

struct    int_app_type
{
    sh_int    learn;
};

struct    wis_app_type
{
    sh_int    	practice;
	sh_int		inherent_potential; 	//base psionic points
	sh_int		heal_adj;				//adjustment to healing spells
	sh_int		mag_def;				//magical defense adjustment.
};

struct    dex_app_type
{
    sh_int    	defensive;
	sh_int		reaction_adj;	//reactions in combat (defensive mostly)
	sh_int		tohit;			//adds to hit_roll
	sh_int		attacks;		//number of attacks.
	//sh_int	initAdj;
};

struct    con_app_type
{
    sh_int    hitp;
    sh_int    shock;
	sh_int	  ress_shock;
	sh_int	  pp_bonus; //psionic point bonus.
};

struct    cha_app_type
{
	sh_int	  	max_charms;			//for just charms
	sh_int	  	henchmen;			//for large scale stuff. (castles, kingdoms)
	sh_int		loyalty_adj;		//loytalty adjustment for henchmen.
};


/*
 * TO types for act.
 */
#define TO_ROOM           0
#define TO_NOTVICT        1
#define TO_VICT           2
#define TO_CHAR           3
#define TO_ALL            4



/*
 * Help table types.
 */

struct    help_data
{
    HELP_DATA *  next;
    HELP_DATA *  next_area;
    sh_int       level;
    char *       keyword;
    char *       text;
	bool 		 to_delete;    	
	char * 		 last_editted_by;		//who last editted this helpfile?
};

struct help_area_data
{
    HELP_AREA *    next;
    HELP_DATA *    first;
    HELP_DATA *    last;
    AREA_DATA *    area;
    char *         filename;
    bool           changed;
};


/*
 * Shop types.
 */
#define MAX_TRADE     5

struct    shop_data
{
    SHOP_DATA *  next;                  /* Next shop in list        */
    long         keeper;                /* Vnum of shop keeper mob    */
    sh_int       buy_type [MAX_TRADE];  /* Item types shop will buy    */
    sh_int       profit_buy;            /* Cost multiplier for buying    */
    sh_int       profit_sell;           /* Cost multiplier for selling    */
    sh_int       open_hour;             /* First opening hour        */
    sh_int       close_hour;            /* First closing hour        */
	bool		 random_gear;			//Can sell random gear.
};



/*
 * Per-class stuff.
 */

#define MAX_GUILD  2
#define MAX_STATS  6
#define STAT_STR   0
#define STAT_INT   1
#define STAT_WIS   2
#define STAT_DEX   3
#define STAT_CON   4
#define STAT_CHA   5


/*
  * Immortal table god fields.
  */
struct god_type
{
    char *		name;                   /* call name of the god */
    bool        pc_good;                /* can be chosen by good pcs */
    bool        pc_neutral;             /* can be chosen by neutral pcs*/
    bool        pc_evil;                /* can be chosen by evil pcs */
    bool        pc_wizard;                /* can be chosen by Mage pcs */
    bool        pc_thief;               /* can be chosen by Thief pcs*/
    bool        pc_fighter;             /* can be chosen by Warrior pcs*/
    bool        pc_cleric;              /* can be chosen by Cleric pcs*/
	bool		pc_ranger;
	bool		pc_druid;
	bool		pc_paladin;
	bool		pc_bard;
	bool		pc_monk;
	char *		favor;
	int			domain1;
	int			domain2;
	int			domain3;
};


struct    ch_class_type
{
    char *  name;              /* the full name of the class  */
    char    who_name    [10];   /* Eight letter who name */	    
	sh_int  attr_prime;        /* Prime attribute             */
	sh_int  attr_second;		//secondary attribute.
    sh_int  weapon;            /* First weapon                */
    long    guild[MAX_GUILD];  /* Vnum of guild rooms         */
    sh_int  skill_adept;       /* Maximum skill level         */
    sh_int  thac0_00;          /* Thac0 for level  0          */
    sh_int  thac0_32;          /* Thac0 for level 32          */
    sh_int  hp_min;            /* Min hp gained on leveling   */
    sh_int  hp_max;            /* Max hp gained on leveling   */
    bool    fMana;             /* Class gains mana on level   */
    char *  base_group;        /* base skills gained          */
    //char *  default_group;     /* default skills gained       */	
	//Upro
	int 	baseXp;			   /* base xp per level */
	char	who_short	[5];   /* shorter version for multi classers */
	int		max_bulk;			/* max bulk allowed per class */
	int     item_bulk;			//can they wear cloth? steel? wood? How bulky can it be?
	bool	can_cast;			//can they cast magic?
	bool 	secret;				//if secret it doesn't show up in creation.
};

/*for Elemental Specialization (Upro 7/17/2007)*/
struct elemSpec {
    char	*name;
    int		damType;
};
/*Upro 10/2009*/
struct wpnSpec {
	char 	*name;
	int		wpnType;
};

//New planes table for each area, will add more later.
//Upro 2/10/2020
struct planes 
{
	char * name;
	int plane_type;
};

struct resists
{
	int percent[MAX_DAM_TYPE];
};

struct item_type
{
    int       type;
    char *    name;
};

struct weapon_type
{
    char *    name;
    long      vnum;
    sh_int    type;
    sh_int    *gsn;
	double	  speed;
	int		  material_cost;
};

struct instrument_type
{
    char *	name;
    sh_int	type;
    sh_int	*gsn;
};


struct fruit_type
{
    char *	name;
    sh_int	type;
};

struct wiznet_type
{
    char *   name;
    long     flag;
    int      level;
};

struct attack_type
{
    char *    name;            /* name         */
    char *    noun;            /* message      */
    int       damage;          /* damage class */
};

struct lang_type
{
    char * 	name;
    /*char *  message;
    long	flag;
    bool	lang;
    MAKE_LANG_FUN *lang_fun;*/
};

struct race_type
{
    char *  name;     /* call name of the race          */
    bool    pc_race;  /* can be chosen by pcs           */
    long    act;      /* act bits for the race          */
    long    aff;      /* aff bits for the race          */
    long    off;      /* off bits for the race          */
    long    imm;      /* imm bits for the race          */
    long    res;      /* res bits for the race          */
    long    vuln;     /* vuln bits for the race         */
    long    form;     /* default form flag for the race */
    long    parts;    /* default parts for the race     */
	
};


struct pc_race_type                 /* additional data for pc races    */
{
    char *  name;                   /* MUST be in race_type            */
    char    who_name[11];			/* Ten letter who name				*/
	char *  short_description;		//Short description shown in creation.
    //sh_int  points;                 /* cost in points of the race      */
	//sh_int  class_mult[MAX_CLASS];  /* exp multiplier for class, * 100 */
    char *  skills[5];              /* bonus skills for the race       */
    sh_int  stats[MAX_STATS];       /* starting stats                  */
    sh_int  max_stats[MAX_STATS];   /* maximum stats                   */
    sh_int  size;                   /* aff bits for the race           */
	//Upro 
	sh_int  base_age;				/* base age for a race				*/
	sh_int  max_age;				/* base maximum age for a race		*/
	int     race_lang;				/* race language */
	bool	can_good;
	bool	can_neutral;
	bool	can_evil;
};


struct spec_type
{
    char *      name;      /* special function name */
    SPEC_FUN *  function;  /* the function          */
};



/*
 * Data structure for notes.
 */

struct    note_data
{
    NOTE_DATA *  next;
    bool         valid;
    sh_int       type;
    char *       sender;
    char *       date;
    char *       to_list;
    char *       subject;
    char *       text;
    time_t       date_stamp;
	time_t		 expire;
};



/*
 * An affect.
 */
struct    affect_data
{
    AFFECT_DATA *  next;
    bool           valid;
    sh_int         where;
    sh_int         type;
    sh_int         level;
    sh_int         duration;
    sh_int         location;
    sh_int         modifier;
    int            bitvector;
};

/* where definitions */
#define TO_AFFECTS   0
#define TO_OBJECT    1
#define TO_IMMUNE    2
#define TO_RESIST    3
#define TO_VULN      4
#define TO_WEAPON    5


/*
 * A kill structure (indexed by level).
 */
struct    kill_data
{
    sh_int  number;
    sh_int  killed;
};


//Note Defines

/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@pip.dknet.dk */

#define DEF_NORMAL  0 /* No forced change, but default (any string)   */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */

#define MAX_BOARD 	  7

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */
						   
#define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)
						
#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */


/* Data about a board */
struct board_data
{
	char *short_name; /* Max 8 chars */
	char *long_name;  /* Explanatory text, should be no more than 40 ? chars */
	
	int read_level; /* minimum level to see board */
	int write_level;/* minimum level to post notes */

	char *names;       /* Default recipient */
	int force_type; /* Default action (DEF_XXX) */
	
	int purge_days; /* Default expiration */

	/* Non-constant data */
		
	NOTE_DATA *note_first; /* pointer to board's first note */
	bool changed; /* currently unused */
		
};

typedef struct board_data BOARD_DATA;


/* External variables */

extern BOARD_DATA boards[MAX_BOARD]; /* Declare */


/* Prototypes */

void finish_note (BOARD_DATA *board, NOTE_DATA *note); /* attach a note to a board */
void free_note   (NOTE_DATA *note); /* deallocate memory used by a note */
void load_boards (void); /* load all boards */
int board_lookup (const char *name); /* Find a board with that name */
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note); /* is tha note to ch? */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void save_notes ();

/* for nanny */
void handle_con_note_to 		(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_subject 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_expire 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_text 		(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_finish 	(DESCRIPTOR_DATA *d, char * argument);

// End note Stuff


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

 
/*
 * Connected states for editor.
 */
#define ED_NONE			0
#define ED_AREA			1
#define ED_ROOM			2
#define ED_OBJECT		3
#define ED_MOBILE		4
#define ED_MPCODE		5
#define ED_OPCODE       6
#define ED_RPCODE       7
#define ED_HELP			8
#define ED_NOTE			9
#define ED_QUEST		10
#define ED_AUCTION		11
 
/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_DEMON				1
#define MOB_VNUM_TARRASQUE			2
#define MOB_VNUM_SHADOW				3
#define MOB_VNUM_FIRE_ELEM			10
#define MOB_VNUM_WATER_ELEM			11
#define MOB_VNUM_AIR_ELEM			12
#define MOB_VNUM_EARTH_ELEM			13
#define MOB_VNUM_ZOMBIE				14
 
#define MOB_VNUM_FIDO       1000
#define MOB_VNUM_CITYGUARD  1000
#define MOB_VNUM_VAMPIRE    1000

#define MOB_VNUM_PATROLMAN  1000
#define GROUP_VNUM_TROLLS   1000
#define GROUP_VNUM_OGRES    1000


/* RT ASCII conversions -- used so we can have letters in this file */

#define A            1
#define B            2
#define C            4
#define D            8
#define E           16
#define F           32
#define G           64
#define H          128

#define I          256
#define J          512
#define K         1024
#define L         2048
#define M         4096
#define N         8192
#define O        16384
#define P        32768

#define Q        65536
#define R       131072
#define S       262144
#define T       524288
#define U      1048576
#define V      2097152
#define W      4194304
#define X      8388608

#define Y     16777216
#define Z     33554432
#define aa    67108864  /* doubled due to conflicts */
#define bb   134217728
#define cc   268435456    
#define dd   536870912
#define ee  1073741824

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC         (A)   /* Auto set for mobs  */
#define ACT_SENTINEL       (B)   /* Stays in one room  */
#define ACT_SCAVENGER      (C)   /* Picks up objects   */
#define ACT_AGGRESSIVE     (F)   /* Attacks PC's       */
#define ACT_STAY_AREA      (G)   /* Won't leave area   */
#define ACT_WIMPY          (H)
#define ACT_PET            (I)   /* Auto set for pets  */
#define ACT_TRAIN          (J)   /* Can train PC's     */
#define ACT_PRACTICE       (K)   /* Can practice PC's  */
#define ACT_UNDEAD         (O)    
#define ACT_CLERIC         (Q)
#define ACT_MAGE           (R)
#define ACT_THIEF          (S)
#define ACT_WARRIOR        (T)
#define ACT_NOALIGN        (U)
#define ACT_NOPURGE        (V)
#define ACT_OUTDOORS       (W)
#define ACT_INDOORS        (Y)
#define ACT_IS_HEALER      (aa)
#define ACT_GAIN           (bb)
#define ACT_UPDATE_ALWAYS  (cc)
#define ACT_IS_CHANGER     (dd)
#define ACT_BANKER		   (ee)

/*ACT2 bits for mobiles. Upro 11/2009*/
#define ACT2_BOUNTY			(A)
#define ACT2_STAY_SECTOR	(B)
#define ACT2_RANDOM_LOOT	(C)
#define ACT2_ELITE			(D)
#define ACT2_BOSS			(E)
#define ACT2_SAGE			(F)
#define ACT2_PEACEFUL		(G)
#define ACT2_MOBINVIS		(H)
#define ACT2_INANIMATE		(I)
#define ACT2_MOUNT			(J)

/* damage classes */
#define DAM_NONE            0
#define DAM_BASH            1
#define DAM_PIERCE          2
#define DAM_SLASH           3
#define DAM_FIRE            4
#define DAM_COLD            5
#define DAM_LIGHTNING       6
#define DAM_ACID            7
#define DAM_POISON          8
#define DAM_NEGATIVE        9
#define DAM_HOLY           10
#define DAM_ENERGY         11
#define DAM_MENTAL         12
#define DAM_DISEASE        13
#define DAM_DROWNING       14
#define DAM_LIGHT          15
#define DAM_OTHER          16
#define DAM_HARM           17
#define DAM_CHARM          18
#define DAM_SOUND          19
#define DAM_AIR			   20
#define DAM_EARTH		   21


/* OFF bits for mobiles */
#define OFF_AREA_ATTACK    (A)
#define OFF_BACKSTAB       (B)
#define OFF_BASH           (C)
#define OFF_BERSERK        (D)
#define OFF_DISARM         (E)
#define OFF_DODGE          (F)
#define OFF_FADE           (G)
#define OFF_FAST           (H)
#define OFF_KICK           (I)
#define OFF_KICK_DIRT      (J)
#define OFF_PARRY          (K)
#define OFF_RESCUE         (L)
#define OFF_TAIL           (M)
#define OFF_TRIP           (N)
#define OFF_CRUSH          (O)
#define ASSIST_ALL         (P)
#define ASSIST_ALIGN       (Q)
#define ASSIST_RACE        (R)
#define ASSIST_PLAYERS     (S)
#define ASSIST_GUARD       (T)
#define ASSIST_VNUM        (U)
#define EXTRA_ATTACK		(V)
#define EXTRA_ATTACK2		(W)
#define STONE_STARE			(X)

/* return values for check_imm */
#define IS_NORMAL          0
#define IS_IMMUNE          1
#define IS_RESISTANT       2
#define IS_VULNERABLE      3

/* IMM bits for mobs */
#define IMM_SUMMON         (A)
#define IMM_CHARM          (B)
#define IMM_MAGIC          (C)
#define IMM_WEAPON         (D)
#define IMM_BASH           (E)
#define IMM_PIERCE         (F)
#define IMM_SLASH          (G)
#define IMM_FIRE           (H)
#define IMM_COLD           (I)
#define IMM_LIGHTNING      (J)
#define IMM_ACID           (K)
#define IMM_POISON         (L)
#define IMM_NEGATIVE       (M)
#define IMM_HOLY           (N)
#define IMM_ENERGY         (O)
#define IMM_MENTAL         (P)
#define IMM_DISEASE        (Q)
#define IMM_DROWNING       (R)
#define IMM_LIGHT          (S)
#define IMM_SOUND          (T)
#define IMM_WOOD           (X)
#define IMM_SILVER         (Y)
#define IMM_IRON           (Z)
#define IMM_AIR			   (aa)
#define IMM_EARTH          (bb)
 
/* RES bits for mobs */
#define RES_SUMMON         (A)
#define RES_CHARM          (B)
#define RES_MAGIC          (C)
#define RES_WEAPON         (D)
#define RES_BASH           (E)
#define RES_PIERCE         (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT        (S)
#define RES_SOUND        (T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
#define RES_EARTH				(aa)
#define RES_AIR					(bb)
 
/* VULN bits for mobs */
#define VULN_SUMMON        (A)
#define VULN_CHARM        (B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT        		(S)
#define VULN_SOUND        		(T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON        		(Z)
#define VULN_EARTH				(aa)
#define VULN_AIR				(bb)
 
 
/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB        (S)
 
#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD        (cc)    
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE        		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS        (Y)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND        (A)
#define AFF_INVISIBLE        (B)
#define AFF_DETECT_EVIL        (C)
#define AFF_DETECT_INVIS    (D)
#define AFF_DETECT_MAGIC    (E)
#define AFF_DETECT_HIDDEN    (F)
#define AFF_DETECT_GOOD        (G)
#define AFF_SANCTUARY        (H)
#define AFF_FAERIE_FIRE        (I)
#define AFF_INFRARED        (J)
#define AFF_CURSE        (K)
#define AFF_UNUSED_FLAG        (L)    /* unused */
#define AFF_POISON        (M)
#define AFF_PROTECT_EVIL    (N)
#define AFF_PROTECT_GOOD    (O)
#define AFF_SNEAK        (P)
#define AFF_HIDE        (Q)
#define AFF_SLEEP        (R)
#define AFF_CHARM        (S)
#define AFF_FLYING        (T)
#define AFF_PASS_DOOR        (U)
#define AFF_HASTE        (V)
#define AFF_CALM        (W)
#define AFF_PLAGUE        (X)
#define AFF_WEAKEN        (Y)
#define AFF_DARK_VISION        (Z)
#define AFF_BERSERK        (aa)
#define AFF_SWIM        (bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW        (dd)
#define AFF_EXTENSION (ee)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL  0
#define SEX_MALE     1
#define SEX_FEMALE   2

/* AC types */
#define AC_PIERCE    0
#define AC_BASH      1
#define AC_SLASH     2
#define AC_EXOTIC    3

/* dice */
#define DICE_NUMBER  0
#define DICE_TYPE    1
#define DICE_BONUS   2

/* size */
#define SIZE_TINY    		0
#define SIZE_SMALL   		1
#define SIZE_MEDIUM  		2
#define SIZE_LARGE   		3
#define SIZE_HUGE    		4
#define SIZE_GIANT   		5
#define SIZE_GARGANTUAN		6


/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE         1
#define OBJ_VNUM_GOLD_ONE           2
#define OBJ_VNUM_GOLD_SOME          3
#define OBJ_VNUM_SILVER_SOME        4
#define OBJ_VNUM_COINS              5
#define OBJ_VNUM_SCROLL				6 /*random scroll drops	 	10/2009*/
#define OBJ_VNUM_STEAK		        7 /*butcher skill  			11/18/2009*/
#define OBJ_VNUM_SKIN				8 /*skinning skill 			11/18/2009*/
#define OBJ_VNUM_ROSE               9
#define OBJ_VNUM_CORPSE_NPC        	10
#define OBJ_VNUM_CORPSE_PC         	11
/*parts for do_death_cry:*////////////////
#define OBJ_VNUM_SEVERED_HEAD      	12 	//
#define OBJ_VNUM_TORN_HEART        	13	//
#define OBJ_VNUM_SLICED_ARM        	14	//
#define OBJ_VNUM_SLICED_LEG        	15	//
#define OBJ_VNUM_GUTS              	16	//
#define OBJ_VNUM_BRAINS            	17	//
//////////////////////////////////////////
#define OBJ_VNUM_PROTOTYPE			18	//Random item gen. 12/29/2009
#define OBJ_VNUM_HERB				19	//For herbalism/foraging. 1/15/2010
#define OBJ_VNUM_MUSHROOM          	20 
#define OBJ_VNUM_LIGHT_BALL        	21
#define OBJ_VNUM_SPRING            	22
#define OBJ_VNUM_DISC              	23
#define OBJ_VNUM_TREE				24
#define OBJ_VNUM_PORTAL            	25
#define OBJ_VNUM_BLOOD				26 //pools of blood.
#define OBJ_VNUM_TRACKS				27 
#define OBJ_VNUM_POTION				28 //for alchemy AND random potion drops
#define OBJ_VNUM_DUMMY    			30 //Dummy object.

#define OBJ_VNUM_RING				31 // Some golden ring is 31??
									   
#define OBJ_VNUM_WAND				35 //random wand drops
#define OBJ_VNUM_BANDAGE			36	//First aid / bandages.
#define OBJ_VNUM_TREASURE_CHEST		37 //buried treasure!
#define OBJ_VNUM_POUCH				38 //generic pouch for random money drops
#define OBJ_VNUM_FIRE				39 // generic for create fire spell and other stuff.

#define OBJ_VNUM_SCHOOL_VEST       101
#define OBJ_VNUM_SCHOOL_SHIELD     102
#define OBJ_VNUM_SCHOOL_BANNER     103

#define OBJ_VNUM_MINOR_HEALTH_POTION	1039


#define OBJ_VNUM_SCHOOL_MACE       	600
#define OBJ_VNUM_SCHOOL_DAGGER     	601
#define OBJ_VNUM_SCHOOL_SHORT_SWORD 602
#define OBJ_VNUM_SCHOOL_SWORD		602
#define OBJ_VNUM_SCHOOL_SPEAR      	617
#define OBJ_VNUM_SCHOOL_STAFF      	618
#define OBJ_VNUM_SCHOOL_AXE        	619
#define OBJ_VNUM_SCHOOL_FLAIL      	620
#define OBJ_VNUM_SCHOOL_WHIP       	621
#define OBJ_VNUM_SCHOOL_POLEARM    	622
#define OBJ_VNUM_SCHOOL_BOW		   	624
#define OBJ_VNUM_SCHOOL_CROSSBOW  	625
#define OBJ_VNUM_SCHOOL_LONG_SWORD	626
#define OBJ_VNUM_ACADEMY_BANDAGE		627





/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT       1
#define ITEM_SCROLL      2
#define ITEM_WAND        3
#define ITEM_STAFF       4
#define ITEM_WEAPON      5
#define ITEM_TREASURE    8
#define ITEM_ARMOR       9
#define ITEM_POTION      10
#define ITEM_CLOTHING    11
#define ITEM_FURNITURE   12
#define ITEM_TRASH       13
#define ITEM_CONTAINER   15
#define ITEM_DRINK_CON   17
#define ITEM_KEY         18
#define ITEM_FOOD        19
#define ITEM_MONEY       20
#define ITEM_BOAT        22
#define ITEM_CORPSE_NPC  23
#define ITEM_CORPSE_PC   24
#define ITEM_FOUNTAIN    25
#define ITEM_PILL        26
#define ITEM_PROTECT     27
#define ITEM_MAP         28
#define ITEM_PORTAL      29
#define ITEM_WARP_STONE  30
#define ITEM_ROOM_KEY    31
#define ITEM_GEM         32
#define ITEM_JEWELRY     33
#define ITEM_JUKEBOX     34

#define ITEM_PEN		 	35
#define ITEM_PAPER		 	36
#define ITEM_SKIN			37 	/*for skinning skill 11/18/2009*/
#define ITEM_SCRY			38	/*used for scrying*/
#define ITEM_THIEVES_TOOLS	39  /*used for lock picking, trap setting/disarming, etc..*/
#define ITEM_BANDAGE		40  /*bandage used for healing and such*/
#define ITEM_SALVE			41  /*for use in herbalism*/
#define ITEM_HERB			42	/*used for herbalism and such.*/
#define ITEM_MINING_TOOL	43  /*used for mining*/
#define ITEM_FISH_POLE		44	/*used for fishing*/
#define ITEM_SIEVE			45  /*prospecting tool*/
#define ITEM_ALCHEMY_LAB	46	/*used for brewing*/
#define ITEM_MISSILE		47
#define ITEM_BLOOD_POOL		48
#define ITEM_TRACKS			49  /*for ranger tracking*/
#define ITEM_ANVIL			50
#define ITEM_LOOM			51
#define ITEM_FIRE			52
#define ITEM_BUILDING		53
#define ITEM_TREE			54
#define ITEM_BLACKSMITH_HAMMER	55
#define ITEM_FIGURINE		56
#define ITEM_SHIP_HELM		57
#define ITEM_ALCHEMY_RECIPE		58
#define ITEM_COOKING_RECIPE		59
#define ITEM_BLACKSMITH_PLANS	60
#define ITEM_TAILORING_PLANS	61
#define ITEM_FLASK				62
#define ITEM_INGREDIENT			63
#define ITEM_SHOVEL				64
#define ITEM_INSTRUMENT			65
#define ITEM_EARRING			66
#define ITEM_SHEATH				67

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW          (A)
#define ITEM_HUM           (B)
#define ITEM_DARK          (C)
#define ITEM_LOCK          (D)
#define ITEM_EVIL          (E)
#define ITEM_INVIS         (F)
#define ITEM_MAGIC         (G)
#define ITEM_NODROP        (H)
#define ITEM_BLESS         (I)
#define ITEM_ANTI_GOOD     (J)
#define ITEM_ANTI_EVIL     (K)
#define ITEM_ANTI_NEUTRAL  (L)
#define ITEM_NOREMOVE      (M)
#define ITEM_INVENTORY     (N)
#define ITEM_NOPURGE       (O)
#define ITEM_ROT_DEATH     (P)
#define ITEM_VIS_DEATH     (Q)
#define ITEM_NONMETAL      (S)
#define ITEM_NOLOCATE      (T)
#define ITEM_MELT_DROP     (U)
#define ITEM_HAD_TIMER     (V)
#define ITEM_SELL_EXTRACT  (W)
#define ITEM_BURN_PROOF    (Y)
#define ITEM_NOUNCURSE     (Z)
#define ITEM_FIRE_TRAP		(aa)
#define ITEM_GAS_TRAP		(bb)
#define ITEM_POISON_TRAP	(cc)
#define ITEM_DART_TRAP		(dd)
#define ITEM_INDESTRUCTABLE (ee)

// Extra2 flags - Upro 1/2010
#define ITEM_HIDDEN			(A)
#define ITEM_WEAR_CAST		(B)
#define ITEM_EPIC			(C)
#define ITEM_LEGENDARY		(D)
#define ITEM_ARTIFACT		(E)
#define ITEM_QUEST_ITEM		(F)
//item manipulation 8/27/2010
#define ITEM_CAN_PUSH		(G)
#define ITEM_CAN_PULL		(H)
#define ITEM_CAN_PRY		(I)
#define ITEM_CAN_PRESS		(J)

#define ITEM_BUOYANT		(K)
#define ITEM_SPIKED			(L)
#define ITEM_OBSCURE		(M)
#define ITEM_UNIQUE			(N)		//Cannot be loaded more than once.

//More traps
#define ITEM_SHOCK_TRAP		(O)
//Leaving room for some more traps.

#define ITEM_WATERPROOF		(P)
#define ITEM_RUSTED			(Q)
#define ITEM_NO_RECHARGE	(R)

//Minor enchants
#define ITEM_HARDSTAFF		(T)		//Fireproof and Acidproof
#define ITEM_SHILLELAGH		(U)		//Druid weapon enchant.
/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE         (A)
#define ITEM_WEAR_FINGER  (B)
#define ITEM_WEAR_NECK    (C)
#define ITEM_WEAR_BODY    (D)
#define ITEM_WEAR_HEAD    (E)
#define ITEM_WEAR_LEGS    (F)
#define ITEM_WEAR_FEET    (G)
#define ITEM_WEAR_HANDS   (H)
#define ITEM_WEAR_ARMS    (I)
#define ITEM_WEAR_SHIELD  (J)
#define ITEM_WEAR_ABOUT   (K)
#define ITEM_WEAR_WAIST   (L)
#define ITEM_WEAR_WRIST   (M)
#define ITEM_WIELD        (N)
#define ITEM_HOLD         (O)
#define ITEM_NO_SAC       (P)
#define ITEM_WEAR_FLOAT   (Q)
#define ITEM_WEAR_TAIL	  (R)
#define ITEM_WEAR_SHEATH  (S)
#define ITEM_WEAR_EAR		(T)


/* weapon class */
#define WEAPON_EXOTIC   0
#define WEAPON_SHORT_SWORD    1
#define WEAPON_DAGGER   2
#define WEAPON_SPEAR    3
#define WEAPON_MACE     4
#define WEAPON_AXE      5
#define WEAPON_FLAIL    6
#define WEAPON_WHIP     7    
#define WEAPON_POLEARM  8
#define WEAPON_STAFF	9
#define WEAPON_BOW		10
#define WEAPON_CROSSBOW 11
#define WEAPON_LONG_SWORD 12

// bard instruments
#define INSTR_LUTE		0
#define INSTR_HARP		1
#define INSTR_DRUMS		2
#define INSTR_PICCOLO	3
#define INSTR_HORN		4

#define QUA_HORRIBLE		0
#define QUA_DECENT		1
#define QUA_GOOD		2
#define QUA_SUPERB		3
#define QUA_EXQUISITE		4

/* weapon types */
#define WEAPON_FLAMING    	(A)
#define WEAPON_FROST      	(B)
#define WEAPON_VAMPIRIC   	(C)
#define WEAPON_SHARP      	(D)
#define WEAPON_VORPAL     	(E)
#define WEAPON_TWO_HANDS  	(F)
#define WEAPON_SHOCKING   	(G)
#define WEAPON_POISON     	(H)
#define WEAPON_LIGHT_DAM		(I)
#define WEAPON_NEGATIVE_DAM		(J)
#define WEAPON_FIRE_DAM			(K)
#define WEAPON_COLD_DAM			(L)
#define WEAPON_LIGHTNING_DAM	(M)
#define WEAPON_AIR_DAM			(N)
#define WEAPON_EARTH_DAM		(O)
#define WEAPON_HOLY_DAM			(P)
#define WEAPON_ENERGY_DAM		(Q)
#define WEAPON_WATER_DAM		(R)

/* armor flags - Upro */
#define ARMOR_SPIKED			(A)
#define ARMOR_REGEN             (B)
#define ARMOR_RES_SHOCK         (C)
#define ARMOR_RES_FIRE          (D)
#define ARMOR_RES_COLD          (E)
#define ARMOR_RES_POISON        (F)
#define ARMOR_RES_DROWNING      (G)
#define ARMOR_RES_ENERGY        (H)
#define ARMOR_RES_LIGHT         (I)
#define ARMOR_RES_AIR			(J)
#define ARMOR_RES_EARTH			(K)

/* gate flags */
#define GATE_NORMAL_EXIT  (A)
#define GATE_NOCURSE      (B)
#define GATE_GOWITH       (C)
#define GATE_BUGGY        (D)
#define GATE_RANDOM       (E)

/* furniture flags */
#define STAND_AT    (A)
#define STAND_ON    (B)
#define STAND_IN    (C)
#define SIT_AT      (D)
#define SIT_ON      (E)
#define SIT_IN      (F)
#define REST_AT     (G)
#define REST_ON     (H)
#define REST_IN     (I)
#define SLEEP_AT    (J)
#define SLEEP_ON    (K)
#define SLEEP_IN    (L)
#define PUT_AT      (M)
#define PUT_ON      (N)
#define PUT_IN      (O)
#define PUT_INSIDE  (P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE            0
#define APPLY_STR             1
#define APPLY_DEX             2
#define APPLY_INT             3
#define APPLY_WIS             4
#define APPLY_CON             5
#define APPLY_SEX             6
#define APPLY_CLASS           7
#define APPLY_LEVEL           8
#define APPLY_AGE             9
#define APPLY_HEIGHT         10
#define APPLY_WEIGHT         11
#define APPLY_MANA           12
#define APPLY_HIT            13
#define APPLY_MOVE           14
#define APPLY_GOLD           15
#define APPLY_EXP            16
#define APPLY_AC             17
#define APPLY_HITROLL        18
#define APPLY_DAMROLL        19
#define APPLY_SAVES          20
#define APPLY_SAVING_PARA    20
#define APPLY_SAVING_ROD     21
#define APPLY_SAVING_PETRI   22
#define APPLY_SAVING_BREATH  23
#define APPLY_SAVING_SPELL   24

/*Upro*/
#define APPLY_CHA			 26
#define APPLY_SIZE			 27
#define APPLY_SPELL_CRIT	 28
#define APPLY_MELEE_CRIT     29
#define APPLY_SPELL_DAM		 30

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE   1
#define CONT_PICKPROOF   2
#define CONT_CLOSED      4
#define CONT_LOCKED      8
#define CONT_PUT_ON     16
#define CONT_NOCOUNT	32



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
 
#define ROOM_VNUM_LIMBO    		1
#define ROOM_VNUM_CHAT     		100
#define ROOM_VNUM_TEMPLE   		1010
#define ROOM_VNUM_RECALL		1010	// Renfall City Square
#define ROOM_VNUM_START			2302    // crossroads in Green Hills.
#define ROOM_VNUM_ALTAR    		1010
#define ROOM_VNUM_SCHOOL   		3700
#define ROOM_DEFAULT_SHIPYARD	9796
//#define ROOM_VNUM_MORGUE		1175
#define ROOM_VNUM_MORGUE		1102
#define ROOM_VNUM_ASTRAL		10500


// Ship flags:
#define SHIP_ANCHORED			(A)
#define SHIP_RAISED				(B)
#define SHIP_DOCKED				(C)

/*
 * Room flags.
 * Used in #ROOMS.
 */
 /*Not sure why (B), and (E) thru (I) were missing. - Upro*/
#define ROOM_DARK        		(A)
#define ROOM_ANTI_WIZARD		(B)
#define ROOM_NO_MOB      		(C)
#define ROOM_INDOORS        	(D)
#define ROOM_ANTI_CLERIC		(E)
#define ROOM_ANTI_THIEF			(F)
#define ROOM_ANTI_FIGHTER		(G)
#define ROOM_ANTI_DRUID			(H)
#define ROOM_ANTI_RANGER		(I)
#define ROOM_PRIVATE        	(J)
#define ROOM_SAFE        		(K)
#define ROOM_SOLITARY        	(L)
#define ROOM_PET_SHOP        	(M)
#define ROOM_NO_RECALL        	(N)
#define ROOM_NO_TREE        	(O)
#define ROOM_GODS_ONLY        	(P)
#define ROOM_HEROES_ONLY    	(Q)
#define ROOM_NEWBIES_ONLY    	(R)
#define ROOM_LAW        		(S)
#define ROOM_NOWHERE        	(T)
#define ROOM_NOFIGHT			(U)
#define ROOM_ANTI_PALADIN		(V)
#define ROOM_ANTI_BARD			(W)
#define ROOM_WORSHIPPER			(X)
#define ROOM_STABLE				(Y)
#define ROOM_ANTI_MONK     		(Z)
#define ROOM_CELL				(aa)
#define ROOM_SAVEOBJS			(bb)
#define ROOM_CLIMBABLE			(cc)
#define ROOM_ANTI_MAGIC			(dd)
#define ROOM_NOSUMMON			(ee)



/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH  0
#define DIR_EAST   1
#define DIR_SOUTH  2
#define DIR_WEST   3
#define DIR_UP     4
#define DIR_DOWN   5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR       (A)
#define EX_CLOSED       (B)
#define EX_LOCKED       (C)
#define EX_PICKPROOF    (F)
#define EX_NOPASS       (G)
#define EX_EASY         (H)
#define EX_HARD         (I)
#define EX_INFURIATING  (J)
#define EX_NOCLOSE      (K)
#define EX_NOLOCK       (L)
#define EX_HIDDEN		(M)
#define EX_SECRET		(N)




/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE         		  0
#define SECT_CITY           		  1
#define SECT_FIELD          		  2
#define SECT_FOREST         		  3
#define SECT_HILLS          		  4
#define SECT_MOUNTAIN       		  5
#define SECT_WATER_SWIM     		  6
#define SECT_WATER_NOSWIM   		  7
#define SECT_UNUSED         		  8
#define SECT_AIR            		  9
#define SECT_DESERT        			  10
#define SECT_ROAD					  11
#define SECT_BEACH                    12
#define SECT_SHORELINE                13
#define SECT_CAVE                     14
#define SECT_OCEANFLOOR               15
#define SECT_LAVA                     16
#define SECT_SWAMP                    17
#define SECT_UNDERGROUND              18
#define SECT_ICE                      19
#define SECT_SNOW                     20
#define SECT_VOID                     21
#define SECT_RUINS                    22
#define SECT_WASTELAND                23
#define SECT_JUNGLE                   24
#define SECT_DOCK		      		  25
#define SECT_SHIP					  26
#define SECT_MAX                      27

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//																//
//			This is stuff for dynamically generated				//
//			descriptions.										//
//																//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct time_description_data
{	
	char * desc[SECT_MAX-1][4]; //second dimension of the array is for variation.
};

extern struct time_description_data time_desc_table[];

/*
 * Equipment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE     	-1
#define WEAR_LIGHT     	0
#define WEAR_FINGER_L  	1
#define WEAR_FINGER_R  	2
#define WEAR_NECK_1    	3
#define WEAR_NECK_2    	4
#define WEAR_BODY      	5
#define WEAR_HEAD      	6
#define WEAR_LEGS      	7
#define WEAR_FEET      	8
#define WEAR_HANDS     	9
#define WEAR_ARMS      	10
#define WEAR_ABOUT     	11
#define WEAR_WAIST     	12
#define WEAR_WRIST_L   	13
#define WEAR_WRIST_R   	14
#define WEAR_WIELD     	15
#define WEAR_SHIELD    	16
#define WEAR_SECONDARY 	17
#define WEAR_HOLD      	18
#define WEAR_FLOAT     	19
#define WEAR_SHEATH		20
#define WEAR_TAIL		21
#define WEAR_EAR		22
#define WEAR_LEGS2		23 //centaurs
#define MAX_WEAR       	24



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK   				  0
#define COND_FULL    				  1
#define COND_THIRST  				  2
#define COND_HUNGER  				  3
#define COND_STARVING                 4 
#define COND_PARCHED                  5
#define COND_EUPHORIC				  6


/*
 * Positions.
 */
#define POS_DEAD      	0
#define POS_MORTAL    	1
#define POS_INCAP     	2
#define POS_STUNNED   	3
#define POS_SLEEPING  	4
#define POS_RESTING   	5
#define POS_SITTING   	6
#define POS_FIGHTING  	7
#define POS_STANDING  	8
#define POS_MOUNTED		9



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC        (A)        /* Don't EVER set.    */

#define PLR_AUTODIG			(B)

/* RT auto flags */
#define PLR_AUTOASSIST      (C)
#define PLR_AUTOEXIT        (D)
#define PLR_AUTOLOOT        (E)
#define PLR_AUTOSAC         (F)
#define PLR_AUTOGOLD        (G)
#define PLR_AUTOSPLIT       (H)

/* MXP flag */ 
#define PLR_MXP				(I)

// Ranger tracking flag
#define PLR_TRACKING		(J)

//Map flag.
#define PLR_AUTOMAP			(K)

/* RT personal flags */
#define PLR_HOLYLIGHT       (N)
#define PLR_CANLOOT 	    (P)
#define PLR_NOSUMMON        (Q)
#define PLR_NOFOLLOW        (R)
#define PLR_COLOUR	        (T)
/* 1 bit reserved, S */

/* penalty flags */
#define PLR_PERMIT        	(U)
#define PLR_LOG            	(W)
#define PLR_DENY        	(X)
#define PLR_FREEZE        	(Y)
#define PLR_THIEF         	(Z)
#define PLR_KILLER          (aa)

//Other flags
#define PLR_AUTO_SHEATHE	(bb)
#define PLR_AUTO_DRINK		(cc)
#define PLR_AUTO_EAT		(dd)
#define PLR_AUTO_SCROLL_LOOT (ee)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN             (H)
#define COMM_NOQUOTE            (I)
#define COMM_SHOUTSOFF          (J)

/* display flags */
#define COMM_COMPACT            (L)
#define COMM_BRIEF              (M)
#define COMM_PROMPT             (N)
#define COMM_COMBINE            (O)
#define COMM_TELNET_GA          (P)
#define COMM_SHOW_AFFECTS       (Q)
#define COMM_NOGRATS            (R)

/* penalties */
#define COMM_NOEMOTE            (T)
#define COMM_NOSHOUT            (U)
#define COMM_NOTELL             (V)
#define COMM_NOCHANNELS         (W) 
#define COMM_SNOOP_PROOF        (Y)
#define COMM_AFK                (Z)

//Others
#define COMM_OOCOFF				(aa)
#define COMM_TIME_STAMPS		(bb)

/* WIZnet flags */
#define WIZ_ON                  (A)
#define WIZ_TICKS               (B)
#define WIZ_LOGINS              (C)
#define WIZ_SITES               (D)
#define WIZ_LINKS               (E)
#define WIZ_DEATHS              (F)
#define WIZ_RESETS              (G)
#define WIZ_MOBDEATHS           (H)
#define WIZ_FLAGS               (I)
#define WIZ_PENALTIES           (J)
#define WIZ_SACCING             (K)
#define WIZ_LEVELS              (L)
#define WIZ_SECURE              (M)
#define WIZ_SWITCHES            (N)
#define WIZ_SNOOPS              (O)
#define WIZ_RESTORE             (P)
#define WIZ_LOAD                (Q)
#define WIZ_NEWBIE              (R)
#define WIZ_PREFIX              (S)
#define WIZ_SPAM                (T)



/* memory settings */
#define MEM_CUSTOMER  A
#define MEM_SELLER    B
#define MEM_HOSTILE   C
#define MEM_AFRAID    D

/* memory for mobs */
struct mem_data
{
    MEM_DATA     *next;
    bool    valid;
    int        id;     
    int     reaction;
    time_t     when;
};

struct ship_data
{
   SHIP_DATA *next;
   SHIP_DATA *prev; 
   SHIP_DATA *next_in_room;
   SHIP_DATA *prev_in_room;
   ROOM_INDEX_DATA *in_room;  
   //AREA_DATA * area;
   char *area;
   long ship_flags;
   char *        	file_name;
   char *name;
   char *description;
   char *owner;
   char *dest; 
   short type;
   short shipclass;
   short comm;
   int cost;
   short realspeed;
   short currspeed;
   short shipstate;
   int hull;			//ship's 'hit points'
   int maxhull;			//ship's max hp
   int shipyard;
   bool		   valid;
   bool anchored;
};

SHIP_DATA *first_ship;
SHIP_DATA *last_ship;


/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct    mob_index_data
{
    MOB_INDEX_DATA *  next;
    SPEC_FUN *        spec_fun;
    SHOP_DATA *       pShop;
    PROG_LIST *      mprogs;
    AREA_DATA *       area;        /* OLC */
    long            vnum;
    sh_int            group;
    bool              new_format;
    sh_int            count;
    sh_int            killed;
    char *            player_name;
    char *            short_descr;
    char *            long_descr;
	char *			  walk_desc;
    char *            description;
    long              act;
	long			  act2;
    long              affected_by;
    sh_int            alignment;
    sh_int            level;
    sh_int            hitroll;
    sh_int            hit[3];
    sh_int            mana[3];
    sh_int            damage[3];
    sh_int            ac[4];
    sh_int            dam_type;
    long              off_flags;
    long              imm_flags;
    long              res_flags;
    long              vuln_flags;
    sh_int            start_pos;
    sh_int            default_pos;
    sh_int            sex;
    sh_int            race;
    long              wealth;
    long              form;
    long              parts;
    sh_int            size;
    char *            material;
    long              mprog_flags;
	
	//Ship shit.
	bool				onBoard;			
	SHIP_DATA *			ship;	
	
};


/*
 * One character (PC or NPC).
 */
struct    char_data
{
	char *		account;
    CHAR_DATA *        	next;
    CHAR_DATA *        	next_in_room;
    CHAR_DATA *        	master;
    CHAR_DATA *        	leader;
    CHAR_DATA *        	fighting;
    CHAR_DATA *        	reply;
    CHAR_DATA *        	pet;
    CHAR_DATA *        	mprog_target;
	char *			   	walk_desc;
    MEM_DATA *         	memory;
    SPEC_FUN *         	spec_fun;
    MOB_INDEX_DATA *   	pIndexData;
    DESCRIPTOR_DATA *  	desc;
    AFFECT_DATA *      	affected;
    OBJ_DATA *         	carrying;
    OBJ_DATA *         	on;	
    ROOM_INDEX_DATA *  	in_room;
    ROOM_INDEX_DATA *  	was_in_room;
    AREA_DATA *        	zone;
    PC_DATA *          	pcdata;
    GEN_DATA *         	gen_data;		
    bool               valid;
    char *             name;
    long               id;
    sh_int             version;
    char *             short_descr;
    char *             long_descr;
    char *             description;
    char *             prompt;
    char *             prefix;
    sh_int             group;
    
	//Upros Clan Stuff:
	sh_int             clan;
	int			   clan_rank;
	int			   clan_points;
    
	
	sh_int             sex;
    sh_int             ch_class;	
	char * 			 	hate;
    sh_int             race;
    sh_int             level;  	
	sh_int             trust;
    int                played;
    int                lines;  /* for the pager */
    time_t             logon;
    sh_int             timer;
    sh_int             wait;
    sh_int             daze;
    sh_int             hit;
    sh_int             max_hit;
    sh_int             mana;
    sh_int             max_mana;
    sh_int             move;
    sh_int             max_move;
    long               gold;
    long               silver;
    long               exp;
    long               act;
	long			   act2;
    long               comm;   /* RT added to pad the vector */
    long               wiznet; /* wiz stuff */
    long               imm_flags;
    long               res_flags;
    long               vuln_flags;
    sh_int             invis_level;
    sh_int             incog_level;
    long               affected_by;
    sh_int             position;
    sh_int             practice;
    sh_int             train;
    sh_int             carry_weight;
    sh_int             carry_number;
    sh_int             saving_throw;
    sh_int             alignment;
    sh_int             hitroll;
    sh_int             damroll;
    sh_int             armor[4];
    sh_int             wimpy;
    /* stats */
    sh_int             perm_stat[MAX_STATS];
    sh_int             mod_stat[MAX_STATS];
    /* parts stuff */
    long               	form;
    long               	parts;
    sh_int             	size;
    char*              	material;
    /* mobile stuff */
    long               	off_flags;
    sh_int             	damage[3];
    sh_int            	dam_type;
    sh_int              start_pos;
    sh_int              default_pos;

    sh_int              mprog_delay;
	//Upro
	sh_int				questpoints;
	int					pkill;
	int					pdeath;
	int					mkill;
	bool			    mClass;  			//have they already multi classed?
	sh_int			    ch_class2;   		//secondary class
	sh_int			    level2;	 			//secondary class level
	long				exp2;  				//secondary class exp
	sh_int				bank_act;			//bank account.
	long				bank_amt;	
	char *				last_name;
	int					pk_timer;
	bool				switched; 			//switched into mobile
	sh_int				god;				//which deity do they follow?
	sh_int				threat;				//how much threat w/current target they have built up.
	sh_int				guildrank;			
	sh_int				guildpoints;	
	sh_int				bulk;				//how much bulk are they wearing?
	sh_int				event;				//which	event are they doing?
	sh_int				learn_count;		//used for learning a spell through event system.
	sh_int				mats[MAX_MATERIAL];	//materials.
	sh_int 				bleeding;			//determines how bad they're bleeding, if at all.
	sh_int				grank;				//none, flanking, front, or back.
	sh_int				ki;					//Monk energy used for skills.
	bool				no_exp[2];			//will they gain exp?
	int					spell_crit;			//spell crit mod
	int					melee_crit;			//melee crit mod
	int					spell_dam;			//spell dam bonus
	bool				arena;				//Queued up for arena combat.
	bool				blind;				//Is the player him/herself blind? Sets blind mode
	int					age;
	int					pet_tracker;		//how many charmies we got?
	bool				mercy;				
	
	//Ship shit.
	bool				onBoard;
	bool				ownShip;			//do they own a ship?
	//char *			shipFile; 			//filename for ship.
	SHIP_DATA *			ship;	
	//Quest shit: 2/2010
	
	QUEST_DATA *		quests;
	
	int curr_quests;								//How many quests are they currently on?
	
	// Faction Shit:
	char *				citizen;					// which town do you hail from.
	
	
	signed int		faction_rep		[MAX_FACTION];
	signed int		faction_rank	[MAX_FACTION];
	int				society_rank;					//What are they in society.
	int 			faction;
	int				faction_invite;					//Have they been invited to join a faction?
	
	//Climbing/altitude stuff:
	int					altitude;					//how high in the air are they? (By tens of feet). Negative means underground.
	//Email
	char *				email;
	
	OBJ_DATA *		   chopping; 	//Keeps track of the tree you're chopping at.
	
	//Psionic Stuff - Upro 9/16/2010
	bool				has_talent;		//1% of all new players will have 'talent'
	int					pp;				//psionic points
	int					max_pp;			//max psionic points
	   
	//Ranger favored enemy: - 9/24/2010
	sh_int				favored_enemy;
	
	//Appearance.
	long				eyes;
	long				hair;
	long				haircut;
	long				facial_hair;
	
	//Cooldowns 10/15/2010 Upro
	long				cooldowns[MAX_SKILL];
	
	//Expertise for Fighters 9/27/2011 Upro
	short				expertise[MAX_WEAPON];
	
	//Boons/perks system, Upro 9/27/2011
	short				boon;
	//Keep track of charms.
	int					charms;
	
	//Achievements:
	//guild achievements:
	bool				rank_apprentice;
	bool				rank_member;
	bool				rank_officer;
	bool				rank_master;
	
	//exploration:
	bool				explore_500;
	bool				explore_2500;
	bool				explore_10000;
	
	//mob kills:
	bool				mob_1000;
	bool				mob_5000;
	bool				mob_10000;
	bool				mob_30000;
	
	//player kills:
	bool				player_1;
	bool				player_10;
	bool				player_50;
	bool				player_250;
	bool				player_500;
	
	//money/bank:
	bool				gold_1000;
	bool				bank_1000;
	bool				bank_10000;
	bool				bank_50000;
};


//Exploration holder.
//Basically just a linked list of int's where we only use the bits.
typedef struct explore_holder
{	struct explore_holder *next;
	unsigned int bits;
	int mask;
} EXPLORE_HOLDER;
 
 
typedef struct exploration_data
{	EXPLORE_HOLDER *bits;
	int set;
} EXPLORE_DATA;

/*
 * Data which only PC's have.
 */
struct    pc_data
{	
    PC_DATA *	   next;
    BUFFER *	   buffer;
    COLOUR_DATA *  code;        /* Data for color configuration    */
	EXPLORE_DATA *explored;
    bool		   valid;
    char *			pwd;
    char *			bamfin;
    char *			bamfout;
    char *			title;	
    sh_int			perm_hit;
    sh_int			perm_mana;
    sh_int			perm_move;
    sh_int			true_sex;
    int				last_level;
    sh_int			condition[MAX_CONDITIONS];
    sh_int			learned[MAX_SKILL];
	sh_int			seen_improve    [MAX_SKILL];
    bool			group_known[MAX_GROUP];
    sh_int			points;
    bool			confirm_delete;
    char *			alias[MAX_ALIAS];
    char *			alias_sub[MAX_ALIAS];
	BOARD_DATA *	board;                  /* The current board        */
	time_t			last_note[MAX_BOARD];   /* last note for the boards */
	NOTE_DATA *		in_progress;
    int				security;               /* OLC */ /* Builder security */
    int				text[3];                /* {t */
    int				auction[3];             /* {a */
    int				auction_text[3];        /* {A */
    int				gossip[3];              /* {d */
    int				gossip_text[3];         /* {9 */
    int				music[3];               /* {e */
    int				music_text[3];          /* {E */
    int				question[3];            /* {q */
    int				question_text[3];       /* {Q */
    int				answer[3];              /* {f */
    int				answer_text[3];         /* {F */
    int				quote[3];               /* {h */
    int				quote_text[3];          /* {H */
    int				immtalk_text[3];        /* {i */
    int				immtalk_type[3];        /* {I */
    int				info[3];                /* {j */
    int				say[3];                 /* {6 */
    int				say_text[3];            /* {7 */
    int				tell[3];                /* {k */
    int				tell_text[3];           /* {K */
    int				reply[3];               /* {l */
    int				reply_text[3];          /* {L */
    int				gtell_text[3];          /* {n */
    int				gtell_type[3];          /* {N */
    int				wiznet[3];              /* {B */
    int				room_title[3];          /* {s */
    int				room_text[3];           /* {S */   
    int				room_exits[3];          /* {o */
    int				room_things[3];         /* {O */
    int				prompt[3];              /* {p */
    int				fight_death[3];         /* {1 */
    int				fight_yhit[3];          /* {2 */
    int				fight_ohit[3];          /* {3 */
    int				fight_thit[3];          /* {4 */
    int				fight_skill[3];         /* {5 */              
	//Upro
	char	*		pretitle;
	int             language        [ MAX_LANGUAGE ];
    int             speaking;
    int             learn;
	int				elementalSpec;
	int				weaponSpec;
	char *			email;
	char *			real_name;	
	int 			fight_pos;	
	bool			item_condition;		//Do they see item conditions?
	bool			eqcondition;
	bool 			beeptells;
	sh_int			eye_color;
	sh_int			hair_color;
	sh_int			height;	//inches
	sh_int			weight; //pounds
	long			recall;		//vnum to recall to.	
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA *  next;
    bool        valid;
    bool        skill_chosen[MAX_SKILL];
    bool        group_chosen[MAX_GROUP];
    int         points_chosen;
};


struct		food_type
{
	char *	name;
	int		food_hours;
	int		full_hours;
	bool	poisoned;
	char *	long_descr;
};


/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_HOLY_WATER	 37
#define LIQ_UNHOLY_WATER 38

struct    liq_type
{
    char *    liq_name;
    char *    liq_color;
    sh_int    liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct    extra_descr_data
{
    EXTRA_DESCR_DATA *  next;         /* Next in list            */
    bool                valid;
    char *              keyword;      /* Keyword in look/examine */
    char *              description;  /* What to see             */
};




struct quest_index_data
{

	QUEST_INDEX_DATA *	next;	
	AREA_DATA *		area;
	char 	* name;	
	int		min_level;		//minimum level to go on quest.
	int		max_level;		//max level you can get rewards for it.
	long	accept_vnum;//where can pcs accept this quest?
	long 	return_vnum;//who do they have to go to in order to complete? (-1 for guild master)
	char 	* desc;		//short description of quest.
	sh_int	rank;		//which guild rank do you get this quest at.
	int 	gp_reward; 	//guildpoint reward
	int		xp_reward; 	//xp reward
	int		gold_reward;//gold reward
	long	obj_reward;	//obj_reward vnum
	long 	type;		//what kind of quest is this? (flags)
	long	vnum;		
	long	target_mob_vnum;//vnum of item/mob to kill, if applicable.	(Or can be the material index needed)
	long	target_obj_vnum;
	int		target_mat_index;
	int		target_herb_index;	
	sh_int	mob_mult;
	sh_int	obj_mult;
	sh_int	mat_mult;
	sh_int	herb_mult;
	char 	* help1;	//first helpfile to read.
	char 	* help2;	//second helpfile to read.
	char 	* help3;	//third helpfile to read.
	int		ch_class_type;	//if for a specific class
	int		race_type;	//if for a specific race
	long		quest_requisite[3];		//which quest is needed before this one can be taken.
	bool 	completed;
	int		time_limit;		//time limit in ticks to complete (-1 for infinite)
};

struct quest_player_data
{		
	QUEST_DATA			* next_quest;
	QUEST_INDEX_DATA	* pIndexData;
	CHAR_DATA 			* taken_by;
	char 				* name;		
	long				vnum;			
	sh_int  			obj_multiples;	//how many obj mults does the char have so far?
	sh_int  			mob_multiples;	//how many mob kills does the char have so far?
	sh_int				mat_multiples;	//how many pieces of mat have they collected so far?
	sh_int				herb_multiples;
	bool				has_read_helps[2]; //have they read helpfiles for the quest?	
	bool 				completed;
	bool                valid;
	int					time_limit;
};


/*
 * Prototype for an object.
 */
struct    obj_index_data
{
    OBJ_INDEX_DATA *    next;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    AREA_DATA *         area;        /* OLC */
    bool                new_format;
    char *              name;
    char *              short_descr;
    char *              description;
    long	            vnum;
    sh_int				quality;
    sh_int              reset_num;
    char *              material;
    sh_int              item_type;
    int                 extra_flags;	
    int                 wear_flags;
    sh_int              level;
    sh_int              condition;	
    sh_int              count;
    sh_int              weight;
    int                 cost;
    int                 value[5];
	PROG_LIST *			oprogs;
	long				oprog_flags;
	int 				extra2_flags;
	sh_int              timer;
};


/*
 * One object.
 */
struct    obj_data
{
    OBJ_DATA *          next;
    OBJ_DATA *          next_content;
    OBJ_DATA *          contains;
    OBJ_DATA *          in_obj;
    OBJ_DATA *          on;
    CHAR_DATA *         carried_by;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    OBJ_INDEX_DATA *    pIndexData;
    ROOM_INDEX_DATA *   in_room;
    bool                valid;
    bool                enchanted;
    char *              owner;
    char *              name;
    char *              short_descr;
    char *              description;
    sh_int              item_type;
    int                 extra_flags;
    int                 wear_flags;
    sh_int              wear_loc;
    sh_int              weight;
    int                 cost;
    sh_int              level;
    sh_int              condition;
    char *              material;
    sh_int              timer;
    int                 value [5];
	CHAR_DATA *			oprog_target;
	sh_int				oprog_delay;
	int					extra2_flags;	
	int 				altitude;			//for floating/flying objects, also negative means it's buried.
	char *				killer;				//for corpses and death recall spell.
	int					wetness;			//is the object soaked with water?
	int					corpse_race;		//to keep track of what the corpse was before it died.
};



/*
 * Exit data.
 */
struct    exit_data
{
    union
    {
		ROOM_INDEX_DATA *  to_room;
		long             vnum;
    } u1;
    sh_int       exit_info;
    sh_int       key;
    char *       keyword;
    char *       description;
    EXIT_DATA *  next;        /* OLC */
    int          rs_flags;    /* OLC */
    int          orig_door;   /* OLC */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct    reset_data
{
    RESET_DATA *    next;
	RESET_DATA *        prev;
    char        command;
    sh_int        arg1;
    sh_int        arg2;
    sh_int        arg3;
    sh_int        arg4;
};



/*
 * Area definition.
 */
struct    area_data
{
    AREA_DATA *     next;
    HELP_AREA *     helps;
    char *        	file_name;
    char *        	name;
    char *        	credits;
    sh_int        	age;
    sh_int        	nplayer;
    sh_int        	low_range;
    sh_int        	high_range;
    long         	min_vnum;
    long        	max_vnum;
    bool        	empty;
    char *        	builders;    /* OLC */ /* Listing of */
    int            	vnum;        /* OLC */ /* Area vnum  */
    int            	area_flags;    /* OLC */
    int            	security;    /* OLC */ /* Value 1-9  */	
	int				climate;
	int				ship_vnum; 	 //If it's a ship, where's it at?
	int				curr_precip;  //rain, snow, etc..
	int				curr_precip_amount; //how hard it's precipitating.
	int 			curr_cloud; 
	int				curr_wind;
	int				curr_wind_dir_dir; //0 = none, 1 = north, 2 = east, 3 = south, 4 = west
	int				curr_temp;
	int				humidity;		//humidity in the area. helps with random descs and weather code.
	int				precip_time; //how long it's been raining/snowing/etc..
	char *			main_resource;
	char * 			faction;
	char *			history;
	int				type;
	bool 			deprecated;		//backwards compatability for old DIKU based areas
	int				plane;
};


/*
 * Room type.
 */
struct    room_index_data
{
    ROOM_INDEX_DATA *    next;
    RESET_DATA *        last_mob_reset;
    RESET_DATA *        last_obj_reset;
    CHAR_DATA *        people;
    OBJ_DATA *        contents;	
    EXTRA_DESCR_DATA *    extra_descr;
    AREA_DATA *     		  	area;
	EXIT_DATA *     			exit    [6];
    RESET_DATA *    			reset_first;    /* OLC */
    RESET_DATA *    			reset_last;    /* OLC */
	SHIP_DATA 					*first_ship;
    SHIP_DATA 					*last_ship;
    char *        				name;
    char *        	description;
    char *        	owner;
    long        	vnum;
    int            	room_flags;
    sh_int        	light;
	bool			can_forage;  //can they forage this room? Used in room resets.
    sh_int        	sector_type;
    sh_int        	heal_rate;
    sh_int         	mana_rate;
    sh_int        	clan;
	PROG_LIST *		rprogs;
    CHAR_DATA *		rprog_target;
    long			rprog_flags;
    sh_int			rprog_delay;

	
	int				x;
	int 			y;
	int				z;
};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000




/*
 *  Target types.
 */
#define TAR_IGNORE            0
#define TAR_CHAR_OFFENSIVE        1
#define TAR_CHAR_DEFENSIVE        2
#define TAR_CHAR_SELF            3
#define TAR_OBJ_INV            4
#define TAR_OBJ_CHAR_DEF        5
#define TAR_OBJ_CHAR_OFF        6

#define TARGET_CHAR            0
#define TARGET_OBJ            1
#define TARGET_ROOM            2
#define TARGET_NONE            3



/*
 * Skills include spells as a particular case.
 */
struct    skill_type
{
    char *    	name;            			/* Name of skill        */
    sh_int    	skill_level[MAX_CLASS];   /* Level needed by class    */
    sh_int    	rating[MAX_CLASS];    	/* How hard it is to learn    */    
    SPELL_FUN *    spell_fun;       	/* Spell pointer (for spells)    */
    sh_int    	target;            		/* Legal targets        */
    sh_int    	minimum_position;    		/* Position for caster / user    */
    sh_int *    pgsn;            		/* Pointer to associated gsn    */
    sh_int    	slot;            			/* Slot for #OBJECT loading    */
    sh_int    	min_mana;        			/* Minimum mana used        */
    sh_int    	beats;            		/* Waiting time after use    */
    char *    	noun_damage;        		/* Damage message        */
    char *    	msg_off;        			/* Wear off message        */
    char *    	msg_obj;        			/* Wear off message for obects    */
	bool	  	can_scribe;				// can a wizard scribe this spell?
	sh_int  *  	requisite;				//pre-requisite skill.
	sh_int 	  	base_stat;				//stat that the skill runs off of.
	sh_int		guildrank;				//guildrank required to learn this.
	sh_int		spell_level;			//-1 for skills, 0-9 for all spells.
	long		cooldown;				//how long before they can use again? 0 for none.
};

struct		enchant_type
{
	char *		name;
	int			apply;					//what it applies.
	int			amt;					//how much it applies.
	int			slot;					//what slot it applies to.
};



struct  group_type
{
    char *    name;
    sh_int    rating[MAX_CLASS];
    char *    spells[MAX_IN_GROUP];
};

/*
 * MOBprog definitions
 */                   
#define TRIG_ACT    	(A)
#define TRIG_BRIBE    	(B)
#define TRIG_DEATH    	(C)
#define TRIG_ENTRY    	(D)
#define TRIG_FIGHT    	(E)
#define TRIG_GIVE    	(F)
#define TRIG_GREET    	(G)
#define TRIG_GRALL    	(H)
#define TRIG_KILL    	(I)
#define TRIG_HPCNT    	(J)
#define TRIG_RANDOM    	(K)
#define TRIG_SPEECH    	(L)
#define TRIG_EXIT    	(M)
#define TRIG_EXALL    	(N)
#define TRIG_DELAY    	(O)
#define TRIG_SURR    	(P)
#define TRIG_GET		(Q)
#define TRIG_DROP		(R)
#define TRIG_SIT		(S)


/*
 * Prog types
 */
#define PRG_MPROG	0
#define PRG_OPROG	1
#define PRG_RPROG	2


struct prog_list
{
    int                 trig_type;
    char *              trig_phrase;
    long              vnum;
    char *              code;
    PROG_LIST *         next;
    bool                valid;
};
 
struct prog_code
{
    long              vnum;
    char *              code;
    PROG_CODE *         next;
}; 
/*
 * These are skill_lookup return values for common skills and spells.
 */
extern    	sh_int    	gsn_backstab;
extern    	sh_int    	gsn_dodge;
extern  	sh_int  	gsn_envenom;
extern    	sh_int    	gsn_hide;
extern    	sh_int    	gsn_peek;
extern    	sh_int    	gsn_pick_lock;
extern   	sh_int    	gsn_sneak;
extern    	sh_int    	gsn_steal;

extern      sh_int      gsn_presence_of_mind;

extern    	sh_int    	gsn_disarm;
extern    	sh_int    	gsn_enhanced_damage;
extern    	sh_int    	gsn_kick;
extern    	sh_int    	gsn_parry;
extern    	sh_int    	gsn_rescue;
extern    	sh_int    	gsn_second_attack;
extern    	sh_int    	gsn_third_attack;
extern      sh_int		gsn_riding;

extern    	sh_int    	gsn_blindness;
extern    	sh_int    	gsn_charm_person;
extern    	sh_int    	gsn_curse;
extern    	sh_int    	gsn_invis;
extern    	sh_int    	gsn_mass_invis;
extern  	sh_int  	gsn_plague;
extern    	sh_int    	gsn_poison;
extern    	sh_int    	gsn_sleep;
extern		sh_int		gsn_weaken;
extern  	sh_int		gsn_fly;
extern  	sh_int  	gsn_sanctuary;
extern 		sh_int		gsn_detect_hidden;
extern 		sh_int		gsn_haste;
extern 		sh_int		gsn_slow;
extern		sh_int		gsn_calm;
extern		sh_int		gsn_frenzy;
extern		sh_int		gsn_armor;
extern		sh_int		gsn_shield;
extern		sh_int		gsn_stone_skin;
extern		sh_int		gsn_giant_strength;

//Upro added skills:
extern		sh_int		gsn_dual_wield;
extern		sh_int		gsn_tracking;
extern 		sh_int		gsn_imbue;
extern		sh_int		gsn_inlay;
extern 		sh_int		gsn_power_grip;
extern		sh_int		gsn_shield_bash;
extern		sh_int		gsn_caltrops;
extern		sh_int		gsn_throw;
extern 		sh_int gsn_gouge;
extern 		sh_int gsn_riposte;
extern 		sh_int gsn_earclap;
extern 		sh_int gsn_kidney_punch;

//Monk skills:
extern		sh_int		gsn_pressure_point;
extern		sh_int		gsn_evasion;
extern		sh_int		gsn_stun_fist;
extern		sh_int		gsn_balance;
extern		sh_int		gsn_channel;
extern		sh_int		gsn_energy_blast;

//Upro added spells:

//Druid specific:
extern      sh_int		gsn_resistance;
extern		sh_int		gsn_detect_plants;
extern		sh_int		gsn_entangle;
extern		sh_int		gsn_shillelagh;
extern		sh_int		gsn_barkskin;
extern		sh_int		gsn_stone_meld;
extern      sh_int		gsn_water_breathing;
extern		sh_int		gsn_animal_growth;
extern		sh_int		gsn_thorns;
extern		sh_int		gsn_fire_seeds;
extern		sh_int		gsn_transmute_metal;
extern		sh_int		gsn_sunbeam;
extern		sh_int		gsn_regeneration;
extern		sh_int		gsn_changestaff;
//End druid

extern sh_int  gsn_heal;
extern sh_int  gsn_cure_light;
extern sh_int  gsn_cure_serious;
extern sh_int  gsn_cure_critical;
extern sh_int  gsn_lightning_bolt;

extern sh_int  gsn_summon_fire;
extern sh_int  gsn_summon_air;
extern sh_int  gsn_summon_water;
extern sh_int  gsn_summon_earth;

extern sh_int  gsn_enchant_armor;
extern sh_int  gsn_enchant_weapon;

extern sh_int  gsn_confusion;
extern sh_int  gsn_web;
extern sh_int  gsn_silence;
extern sh_int  gsn_audible_glamor;
extern sh_int  gsn_extension;
extern sh_int  gsn_scry;
extern sh_int  gsn_detect_scrying;
extern sh_int  gsn_displacement;
extern sh_int  gsn_disrupt_undead;
extern sh_int  gsn_enlarge_person;
extern sh_int  gsn_reduce_person;
extern sh_int  gsn_etherealness;
extern sh_int  gsn_feeblemind;
extern sh_int  gsn_astral_spell;
extern sh_int  gsn_paralyzation;
extern sh_int  gsn_detect_undead;
extern sh_int  gsn_detect_traps;
extern sh_int  gsn_remove_trap;
extern sh_int  gsn_lay_trap;
extern sh_int  gsn_find_trap;
extern sh_int  gsn_mana_shield;
extern sh_int  gsn_acid_arrow;
extern sh_int  gsn_flame_arrow;
extern sh_int  gsn_frost_arrow;
extern sh_int  gsn_blink;
extern sh_int  gsn_summon_monster;
extern sh_int	gsn_embalm;
extern sh_int	gsn_disintegrate;
extern sh_int	gsn_blade_barrier;
extern sh_int  gsn_globe_invulnerability;
extern sh_int  gsn_gaseous_form;
extern sh_int  gsn_ghoul_touch;
extern sh_int  gsn_forbiddance;
extern sh_int  gsn_explosive_runes;
extern sh_int  gsn_fire_storm;
extern sh_int  gsn_gust_of_wind;
extern sh_int  gsn_ice_storm;
extern sh_int  gsn_meteor_swarm;
extern sh_int  gsn_nightmare;
extern sh_int  gsn_create_fire;
extern sh_int  gsn_resist_scrying;
extern sh_int  gsn_spellcraft;
extern sh_int	gsn_detect_secret;
extern sh_int	gsn_stone_skin;
extern sh_int  gsn_animate_object;
extern sh_int  gsn_banshee_wail;
extern sh_int  gsn_copy;
extern sh_int  gsn_chromatic_orb;
extern sh_int  gsn_corpse_visage;
extern sh_int  gsn_death_recall;
extern sh_int  gsn_negative_plane_protection;
extern sh_int  gsn_obscure_object;
extern sh_int  gsn_timestop;
extern sh_int  gsn_planar_door;
extern sh_int  gsn_free_action;
extern sh_int  gsn_cone_cold;
extern sh_int  gsn_cone_shadow;
extern sh_int  gsn_cone_sound;
extern sh_int  gsn_cone_silence;
extern sh_int  gsn_pass_without_trace;
extern sh_int  gsn_searing_light;
extern sh_int  gsn_sunburst;

extern sh_int  gsn_antimagic_shell;
extern sh_int  gsn_earth_ward;
extern sh_int  gsn_dimensional_mine;
extern sh_int  gsn_shocking_web;
extern sh_int  gsn_geyser;
extern sh_int  gsn_shocking_runes;
extern sh_int  gsn_animate_weapon;
extern sh_int  gsn_hardstaff;
extern sh_int  gsn_finger_death;
extern sh_int  gsn_remove_paralysis;
extern sh_int  gsn_giant_strength;

extern sh_int  gsn_alarm;
extern sh_int  gsn_animate_dead;
extern sh_int  gsn_raise_dead;
extern sh_int  gsn_wither;
extern sh_int  gsn_blasphemy;
extern sh_int  gsn_deafness;
extern sh_int  gsn_disenchant;
extern sh_int  gsn_command_undead;
extern sh_int  gsn_commune_nature;
extern sh_int  gsn_comprehend_languages;
extern sh_int  gsn_darkness;
extern sh_int  gsn_dimensional_anchor;
extern sh_int  gsn_elemental_protection;
extern sh_int  gsn_erase;
extern sh_int  gsn_feather_fall;
extern sh_int  gsn_rehydrate;
extern sh_int  gsn_frostbite;
extern sh_int  gsn_bleeding_touch;
extern sh_int  gsn_spectral_hand;
extern sh_int  gsn_monsoon;
extern sh_int  gsn_frostbolt;
extern sh_int  gsn_transfer_life;


//Stat spells:
extern sh_int	gsn_strength;
extern sh_int	gsn_intelligence;
extern sh_int	gsn_wisdom;
extern sh_int	gsn_dexterity;
extern sh_int	gsn_constitution;
extern sh_int	gsn_charisma;

//Imm spells:
extern sh_int		gsn_wrath_ancient;

//Paladin spells:
extern sh_int  gsn_lay_hands;
extern sh_int  gsn_aid;
extern sh_int  gsn_holy_forge;
extern sh_int  gsn_retribution;


extern sh_int  gsn_flaming_forge;
extern sh_int  gsn_vampiric_forge;
extern sh_int  gsn_frost_forge;
extern sh_int  gsn_lightning_forge;

// Bard spells:
extern sh_int      gsn_adagio;
extern sh_int      gsn_vocal_shield;
extern sh_int      gsn_vitalizing_verse;
extern sh_int      gsn_nourishing_ballad;
extern sh_int      gsn_hypnotic_verse;
extern sh_int      gsn_motivation;
extern sh_int      gsn_insidious_chord;
extern sh_int      gsn_banshee_ballad;
extern sh_int      gsn_calming_verse;
extern sh_int      gsn_crescendo;
extern sh_int      gsn_quicken_tempo;
extern sh_int      gsn_orbitting_rhythm_lead;
extern sh_int      gsn_suggestive_verse;
extern sh_int      gsn_melody_masked_intent;
extern sh_int      gsn_forgetful_lullaby;
extern sh_int      gsn_hymn_fallen;
extern sh_int      gsn_contradicting_melody;

extern sh_int      gsn_cacophonic_shield;
extern sh_int      gsn_dissonant_chord;
extern sh_int      gsn_dirge_dischord;
extern sh_int      gsn_harmonic_chorus;
extern sh_int      gsn_improvisation;
extern sh_int      gsn_infernal_threnody;
extern sh_int      gsn_listening_jewel;
extern sh_int      gsn_wail_doom;
extern sh_int      gsn_resonating_echo;
extern sh_int      gsn_selective_hearing;
extern sh_int      gsn_sacred_euphony;
extern sh_int      gsn_repulsive_din;
extern sh_int      gsn_audible_intoxication;



//Psionic devotions
extern sh_int  gsn_read_object;
extern sh_int  gsn_absorb_disease;
extern sh_int  gsn_adrenaline_control;
extern sh_int  gsn_agitation;
extern sh_int  gsn_animate_shadow;
extern sh_int  gsn_appraise;
extern sh_int  gsn_aura_sight;
extern sh_int  gsn_aversion;
extern sh_int  gsn_awe;
extern sh_int  gsn_ballistic_attack;
extern sh_int  gsn_banishment;
extern sh_int  gsn_biofeedback;
extern sh_int  gsn_body_equilibrium;
extern sh_int  gsn_body_weaponry;
extern sh_int  gsn_chemical_simulation;
extern sh_int  gsn_psionic_void;
extern sh_int  gsn_chemical_simulation;
extern sh_int  gsn_clairaudience;
extern sh_int  gsn_clairvoyance;
extern sh_int  gsn_clear_thoughts;
extern sh_int	gsn_contact;
extern sh_int  gsn_fighting_mind;
extern sh_int  gsn_control_flame;
extern sh_int  gsn_daydream;
extern sh_int  gsn_death_field;
extern sh_int  gsn_detonate;
extern sh_int  gsn_domination;
extern sh_int  gsn_dimensional_door;
extern sh_int  gsn_ectoplasmic_form;
extern sh_int  gsn_ego_whip;
extern sh_int  gsn_ejection;
extern sh_int  gsn_energy_containment;
extern sh_int  gsn_enhanced_strength;
extern sh_int  gsn_expansion;
extern sh_int  gsn_flesh_armor;
extern sh_int  gsn_immovability;
extern sh_int  gsn_inertial_barrier;
extern sh_int  gsn_intellect_fortress;
extern sh_int  gsn_heightened_senses;
extern sh_int  gsn_know_location;
extern sh_int  gsn_lend_health;
extern sh_int  gsn_lethargy;
extern sh_int  gsn_levitate;
extern sh_int  gsn_mental_barrier;
extern sh_int  gsn_metabolic_reconfiguration;
extern sh_int  gsn_mind_over_body;
extern sh_int  gsn_mind_thrust;
extern sh_int  gsn_mindwipe;
extern sh_int  gsn_planar_gate;
extern sh_int  gsn_probe;
extern sh_int  gsn_psychic_drain;
extern sh_int  gsn_reduction;
extern sh_int  gsn_summon_creature;
extern sh_int  gsn_manavert;


// bard skills and such
extern sh_int      gsn_perfect_pitch;
extern sh_int      gsn_harp;
extern sh_int      gsn_lute;
extern sh_int      gsn_piccolo;
extern sh_int      gsn_horn;
extern sh_int      gsn_drums;


//bard epic spells
extern sh_int      gsn_aggressive_staccato;
extern sh_int      gsn_diminishing_scale;
extern sh_int      gsn_assisted_coda;
extern sh_int	   gsn_gnomish_tribute;
extern sh_int	   gsn_dynamic_diapason;


//Upro added skills:

extern sh_int  	gsn_swim;
extern sh_int  	gsn_butcher;
extern sh_int  	gsn_skin;
extern sh_int  	gsn_seafaring;
extern sh_int  	gsn_counter;
extern sh_int  	gsn_forage;
extern sh_int	gsn_mine;
extern sh_int   gsn_prospecting;
extern sh_int	gsn_plant;
extern sh_int  	gsn_herbalism;
extern sh_int  	gsn_alchemy;
extern sh_int   gsn_tailoring;
extern sh_int   gsn_jewelcrafting;
extern sh_int   gsn_blacksmithing;
extern sh_int  	gsn_cooking;
extern sh_int  	gsn_agriculture;
extern sh_int	gsn_first_aid;
extern sh_int	gsn_engage;
extern sh_int   gsn_woodcutting;
extern sh_int   gsn_gore;

/* new gsns */
extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_spear;
extern sh_int  gsn_short_sword;
extern sh_int  gsn_long_sword;
extern sh_int  gsn_bow;
extern sh_int  gsn_crossbow;
extern sh_int  gsn_staff;
extern sh_int  gsn_whip;
 
extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_bloodlust;
extern sh_int  gsn_battlecry;
extern sh_int  gsn_dirt;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_trip;

 
extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_meditation;
 
extern sh_int  gsn_scrolls;
extern sh_int  gsn_staves;
extern sh_int  gsn_wands;
extern sh_int  gsn_recall;
extern sh_int  gsn_scribe;
extern sh_int  gsn_repair;

extern sh_int  gsn_sharpen;
extern sh_int  gsn_martial_arts;
extern sh_int  gsn_hapkido;

//Monk skills:

extern sh_int	gsn_iron_fist;
extern sh_int	gsn_iron_will;
extern sh_int	gsn_iron_skin;
extern sh_int	gsn_hand_parry;
extern sh_int	gsn_palm_strike;

//utility stuff:
extern int FilesOpen;
extern FILE_DATA *file_list;

/*
 * Structure for a social in the socials table.
 */
struct    social_type
{
    char      name[20];
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *    char_auto;
    char *    others_auto;
};



/*
 * Global constants.
 */
extern    const    struct    str_app_type    str_app        [26];
extern    const    struct    int_app_type    int_app        [26];
extern    const    struct    wis_app_type    wis_app        [26];
extern    const    struct    dex_app_type    dex_app        [26];
extern    const    struct    con_app_type    con_app        [26];
extern    const    struct	 cha_app_type	 cha_app		[26];

extern    const    struct    ch_class_type      ch_class_table    [MAX_CLASS];
extern    const    struct    weapon_type     weapon_table   [];
extern    const    struct    item_type       item_table     [];
extern    const    struct    wiznet_type     wiznet_table   [];
extern    const    struct    attack_type     attack_table   [];
extern    const    struct    race_type       race_table     [];
extern    const    struct    pc_race_type    pc_race_table  [];
extern    const    struct    spec_type       spec_table     [];
extern    const    struct    liq_type        liq_table      [];
extern    const    struct    skill_type      skill_table    [MAX_SKILL];
extern    const    struct    group_type      group_table    [MAX_GROUP];
extern             struct    social_type     social_table   [ MAX_SOCIALS ];
//Upro	
extern  const   struct  	lang_type        lang_table     [ MAX_LANGUAGE ];
extern  const   struct  	elemSpec		 specTable		[8];
extern  const   struct  	wpnSpec		 	 wpnSpecTable	[14];
extern  const   struct      planes			 plane_table[];
extern  const   struct      resists			 resist_table[MAX_PC_RACE];
extern  const   struct  	god_type         god_table      [MAX_GOD];
extern  const   struct		mob_stat_type	 mob_stat_table	[];
extern  const	struct		faction_data	 faction_table	[MAX_FACTION];
extern  const   struct      society_data     society_table  [];
extern  const   struct      enchant_type     enchant_table    [];
extern 	const 	struct 		resists			 resist_table [MAX_PC_RACE];

/*
 * Drunk struct
 */
struct struckdrunk
{
	int	min_drunk_level;
	int	number_of_rep;
	char	*replacement[11];
};

struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

struct clan_type
{	
    char	 	* name;
    char 		* who_name;
    sh_int 		hall;
    sh_int		item;
	bool		independent; /* true for loners */
	char		* clan_channel;
	
};

struct position_type
{
    char *name;
    char *short_name;
};

struct sex_type
{
    char *name;
};

struct size_type
{
    char *name;
};

struct	bit_type
{
	const	struct	flag_type *	table;
	char *				help;
};

struct  herb_type
{
	char * 		name;
	int			rarity;
	int			eco_system;
};

//  Lookup Info - Was lookup.h  //
int	        flag_lookup	    args( (const char *, const struct flag_type *) );


/* game tables */
extern	const	struct	clan_type	clan_table[MAX_CLAN];
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];
extern  const   struct  instrument_type instrument_table[];	

/* flag tables */
extern  const   struct  flag_type   quest_types[];
extern	const	struct	flag_type	act_flags[];
extern  const   struct  flag_type   act2_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	extra2_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern	const	struct	flag_type	exit_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern	const	struct	flag_type	area_flags[];
extern  const   struct  flag_type	ship_flags[];
extern	const	struct	flag_type	sector_flags[];
extern	const	struct	flag_type	door_resets[];
extern	const	struct	flag_type	wear_loc_strings[];
extern	const	struct	flag_type	wear_loc_flags[];
extern	const	struct	flag_type	res_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	vuln_flags[];
extern	const	struct	flag_type	type_flags[];
extern	const	struct	flag_type	apply_flags[];
extern	const	struct	flag_type	sex_flags[];
extern	const	struct	flag_type	furniture_flags[];
extern	const	struct	flag_type	weapon_class[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	weapon_type2[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	size_flags[];
extern	const	struct	flag_type	position_flags[];
extern	const	struct	flag_type	ac_type[];
extern	const	struct	bit_type	bitvector_type[];
extern	const	struct	flag_type	oprog_flags[];
extern	const	struct	flag_type	rprog_flags[];
extern  const   struct  herb_type 	herb_table[];
extern  const   struct  food_type	food_table[];
extern  const   struct  resource_data mat_table[];
extern  const   struct  flag_type eye_color_flags[];
extern  const   struct  flag_type hair_color_flags[];
extern  const   struct  flag_type haircut_flags[];
extern  const   struct  flag_type facial_hair_flags[];
extern  const   struct  flag_type ambience_flags[];
extern  const   struct  flag_type instrument_type_flags[];
extern  const   struct  flag_type instrument_quality_flags[];

/*
 * Global variables.
 */
extern  HELP_DATA		*	help_first;
extern  SHOP_DATA		*	shop_first;
extern  CHAR_DATA		*	char_list;
extern  DESCRIPTOR_DATA	*	descriptor_list;
extern  OBJ_DATA		*	object_list;
extern  QUEST_INDEX_DATA *	quest_list;
extern  AUCTION_DATA	*	house_auctions;
extern  BOUNTY_DATA     *   bounty_list;
extern          PROG_CODE         *     mprog_list;
extern          PROG_CODE         *     rprog_list;
extern          PROG_CODE         *     oprog_list;

extern  char				bug_buf[];
extern  time_t				current_time;
extern  bool				fLogAll;
extern  FILE			*	fpReserve;
extern  KILL_DATA			kill_table[];
extern  char				log_buf[];
extern  TIME_INFO_DATA		time_info;
extern  WEATHER_DATA		weather_info;
extern          char                    last_command[MAX_STRING_LENGTH];
extern	NOTE_DATA		*	note_free;
extern	OBJ_DATA		*	obj_free;
extern  bool				MOBtrigger;

extern          bool                    double_exp;
extern          int                     global_exp;
extern			bool					double_skill;
extern			int						global_skill;
extern			bool					double_gold;
extern			int						global_gold;
extern          sh_int                  display;



/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if    defined(_AIX)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(apollo)
int    atoi        args( ( const char *string ) );
void *    calloc        args( ( unsigned nelem, size_t size ) );
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(hpux)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(linux)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(macintosh)
#define NOCRYPT
#if    defined(unix)
#undef    unix
#endif
#endif

#if    defined(MIPS_OS)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(MSDOS)
#define NOCRYPT
#if    defined(unix)
#undef    unix
#endif
#endif

#if    defined(NeXT)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(sequent)
char *    crypt        args( ( const char *key, const char *salt ) );
int    fclose        args( ( FILE *stream ) );
int    fprintf        args( ( FILE *stream, const char *format, ... ) );
int    fread        args( ( void *ptr, int size, int n, FILE *stream ) );
int    fseek        args( ( FILE *stream, long offset, int ptrname ) );
void    perror        args( ( const char *s ) );
int    ungetc        args( ( int c, FILE *stream ) );
#endif

#if    defined(sun)
char *    crypt        args( ( const char *key, const char *salt ) );
int    fclose        args( ( FILE *stream ) );
int    fprintf        args( ( FILE *stream, const char *format, ... ) );
#if    defined(SYSV)
siz_t    fread        args( ( void *ptr, size_t size, size_t n, 
                FILE *stream) );
#elif !defined(__SVR4)
int    fread        args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int    fseek        args( ( FILE *stream, long offset, int ptrname ) );
void    perror        args( ( const char *s ) );
int    ungetc        args( ( int c, FILE *stream ) );
#endif

#if    defined(ultrix)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if    defined(NOCRYPT)
#define crypt(s1, s2)    (s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR      ""                  /* Player files            */
#define SHIP_DIR		"ships"
#define TEMP_FILE       "romtmp"
#define NULL_FILE       "proto.are"         /* To reserve one stream   */
#define ACCOUNT_DIR		"account"		//Account directory
#endif

#if defined(MSDOS)
#define SHIP_DIR		"\ships"
#define PLAYER_DIR      ""                   /* Player files           */
#
#define TEMP_FILE       "romtmp"
#define NULL_FILE       "nul"                /* To reserve one stream  */
#endif

#if defined(unix)
#define SHIP_DIR		"../ships/"
#define PLAYER_DIR      "../player/"         /* Player files           */
#define GOD_DIR         "../gods/"           /* list of gods           */
#define TEMP_FILE       "../player/romtmp"
#define NULL_FILE       "/dev/null"          /* To reserve one stream  */
#define LAST_COMMAND    "../last_command.txt"  /*For the signal handler.*/
#define ACCOUNT_DIR		"../account/"		//Account directory
#define LAST_LIST       "../log/last.lst" //last list
#define LAST_TEMP_LIST "../log/ltemp.lst" //temp file for the last list so the data can be copyover over
#endif


#define ACCOUNT_DIR		"../account/"	//For accounts.
#define NOTE_DIR  		"../notes" /* set it to something you like */

#define GAME_STATS_FILE	"game_stats.txt"
#define SHIP_AREA       "shipvnum.are"

#define AREA_LIST       "area.lst"            /* List of areas         */
#define BUG_FILE        "bugs.txt"            /* For 'bug' and bug()   */
#define TYPO_FILE       "typos.txt"           /* For 'typo'            */
#define SHUTDOWN_FILE   "shutdown.txt"        /* For 'shutdown'        */
#define BAN_FILE        "ban.txt"
#define MUSIC_FILE      "music.txt"
#define OHELPS_FILE	    "orphaned_helps.txt"  /* Unmet 'help' requests */
#define SHIP_LIST		"ships.txt"
#define BOUNTY_FILE 	"bounty.txt"
#define AUCTION_FILE	"auctions.log"


#define LAST_LIST       "../log/last.lst" //last list
#define LAST_TEMP_LIST "../log/ltemp.lst" //temp file for the last list so the data can be copyover over

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD   CHAR_DATA
#define MID  MOB_INDEX_DATA
#define OD   OBJ_DATA
#define OID  OBJ_INDEX_DATA
#define RID  ROOM_INDEX_DATA
#define SF   SPEC_FUN
#define AD   AFFECT_DATA
#define MPC  PROG_CODE
FILE *__FileOpen(char *filename, char *mode, const char *file, const char *function, int line);
 
void FileClose(FILE *fp);
 
#define FileOpen(filename, mode)  __FileOpen(file, mode, __FILE__, __FUNCTION__, __LINE__)


///////////////////////////////////////////////////////////////////////////
//////////////////// GLOBAL FUNCTION PROTOTYPES ///////////////////////////
///////////////////////////////////////////////////////////////////////////

//account.c
int	    stringSearch		args((char *, char *));
void    adeny				args((CHAR_DATA *, char*));



/* act_comm.c */
void     check_sex           args( ( CHAR_DATA *ch) );
void     add_follower        args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void     stop_follower       args( ( CHAR_DATA *ch ) );
void     nuke_pets           args( ( CHAR_DATA *ch ) );
void     die_follower        args( ( CHAR_DATA *ch ) );
bool     is_same_group       args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void     log_f                args( ( char * fmt, ... ) );


/* act_enter.c */
RID      *get_random_room    args( ( CHAR_DATA *ch ) );

/* act_info.c */
char 	*get_rank			args((int rank));
void 	 do_census 			args((CHAR_DATA *ch, char *argument));
void     set_title          args( ( CHAR_DATA *ch, char *title ) );
int		 what_size			args((CHAR_DATA *ch));

int 	fighter_levels 		args ((CHAR_DATA*ch));
int 	thief_levels 		args ((CHAR_DATA*ch));
char	get_random_room_desc args ((CHAR_DATA *ch));

/* act_move.c */
void check_explore args( ( CHAR_DATA *, ROOM_INDEX_DATA * ) );
void explore_vnum args( (CHAR_DATA *, int ) );
bool explored_vnum args( (CHAR_DATA *, int ) );
void move_char           args( ( CHAR_DATA *ch, int door, bool follow ) );

/* act_obj.c */
bool     can_loot            args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void     wear_obj            args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
void     get_obj             args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                                     OBJ_DATA *container ) );
void do_dual 				 args( (CHAR_DATA *ch, char *argument) );
									
// act_olc.c
bool valid_herb				args ((char * argument));
int get_herb				args ((char * argument));
bool create_random_area		args ((AREA_DATA *area));
									
/* act_wiz.c */
void     wiznet              args( ( char *string, CHAR_DATA *ch, OBJ_DATA *obj,
                                     long flag, long flag_skip, int min_level ) );
void     copyover_recover    args( ( void ) );
void	 qmconfig_read		 args( ( void ) );
                   
/* alias.c */
void     substitute_alias    args( ( DESCRIPTOR_DATA *d, char *input ) );

/* auction.c */
void talk_auction(char *argument);
void do_auction(CHAR_DATA *ch, char *argument);
void auction_update(void);

/* ban.c */
bool     check_ban           args( ( char *site, int type) );

/* board.c */
void note_nanny		args((DESCRIPTOR_DATA *d, char * argument));
void show_note_to_char 		args((CHAR_DATA *ch, NOTE_DATA *note, int num));
bool next_board 			args((CHAR_DATA *ch));
int board_number 				args((const BOARD_DATA *board));
void do_nread 					args((CHAR_DATA *ch, char *argument));

// changes.c
void load_changes args (());



/* comm.c */
int count_mxp_tags args((const int bMXP, const char *txt, int length));
void convert_mxp_tags args((const int bMXP, char * dest, const char *src, int length));
void     show_string         args( ( struct descriptor_data *d, char *input) );
void     close_socket        args( ( DESCRIPTOR_DATA *dclose ) );
void     write_to_buffer     args( ( DESCRIPTOR_DATA *d, const char *txt, int length ) );
void     send_to_desc        args( ( const char *txt, DESCRIPTOR_DATA *d ) );
void     send_to_char        args( ( const char *txt, CHAR_DATA *ch ) );
void     page_to_char        args( ( const char *txt, CHAR_DATA *ch ) );
void     act                 args( ( const char *format, CHAR_DATA *ch,
                                     const void *arg1, const void *arg2, int type ) );
void     act_new             args( ( const char *format, CHAR_DATA *ch, 
                                     const void *arg1, const void *arg2, int type,
                                     int min_pos) );
void     printf_to_char      args( ( CHAR_DATA *, char *, ... ) );
void     printf_to_desc      args( ( DESCRIPTOR_DATA *, char *, ... ) );
void     bugf                args( ( char *, ... ) );
bool     write_to_descriptor args( ( int desc, char *txt, int length ) );

/* db.c */
int count_linked_areas		args(());
long count_linked_rooms		args(());
void     reset_area          args( ( AREA_DATA * pArea ) );        /* OLC */
void     reset_room          args( ( ROOM_INDEX_DATA *pRoom ) );   /* OLC */
char *   print_flags         args( ( int flag ));
void     boot_db             args( ( void ) );
void     area_update         args( ( void ) );
void 	 sleep_update 		 args( ( ) );
CD *     create_mobile       args( ( MOB_INDEX_DATA *pMobIndex ) );
void     clone_mobile        args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *     create_object       args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void     clone_object        args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void     clear_char          args( ( CHAR_DATA *ch ) );
char *   get_extra_descr     args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *    get_mob_index       args( (long vnum ) );
OID *    get_obj_index       args( (long vnum ) );
RID *    get_room_index      args( (long vnum ) );
QUEST_INDEX_DATA *	 get_quest_index	args( ( long vnum ) );
MPC *    get_prog_index      args( ( long vnum, int type ) );
char     fread_letter        args( ( FILE *fp ) );
int      fread_number        args( ( FILE *fp ) );
long     fread_flag          args( ( FILE *fp ) );
char *   fread_string        args( ( FILE *fp ) );
char *   fread_string_eol    args( ( FILE *fp ) );
void     fread_to_eol        args( ( FILE *fp ) );
char *	 fread_line			 args( ( FILE *fp ) );
char *   fread_word          args( ( FILE *fp ) );
long     flag_convert        args( ( char letter) );
void *   alloc_mem           args( ( int sMem ) );
void *   alloc_perm          args( ( int sMem ) );
void     free_mem            args( ( void *pMem, int sMem ) );
char *   str_dup             args( ( const char *str ) );
void     free_string         args( ( char *pstr ) );
int      number_fuzzy        args( ( int number ) );
int      number_range        args( ( int from, int to ) );
int      number_percent      args( ( void ) );
int      number_door         args( ( void ) );
int      number_bits         args( ( int width ) );
long     number_mm           args( ( void ) );
int      dice                args( ( int number, int size ) );
int      interpolate         args( ( int level, int value_00, int value_32 ) );
void     smash_tilde         args( ( char *str ) );
void     smash_dollar        args( ( char *str ) );
bool     str_cmp             args( ( const char *astr, const char *bstr ) );
bool     str_prefix          args( ( const char *astr, const char *bstr ) );
bool     str_infix           args( ( const char *astr, const char *bstr ) );
bool     str_suffix          args( ( const char *astr, const char *bstr ) );
char *   capitalize          args( ( const char *str ) );
void     append_file         args( ( CHAR_DATA *ch, char *file, char *str ) );
void     bug                 args( ( const char *str, int param ) );
void     log_string          args( ( const char *str ) );
void     tail_chain          args( ( void ) );
bool     check_pet_affected  args( ( long vnum, AFFECT_DATA *paf) );


/* drunk.c */
char	*makedrunk	args( (char *string, CHAR_DATA *ch) );


 /*
  * Colour stuff by Lope
  */
 int   colour          args( ( char type, CHAR_DATA *ch, char *string ) );
 void  colourconv      args( ( char *buffer, const char *txt, CHAR_DATA*ch)); 
 void  SEND_bw args( ( const char *txt, CHAR_DATA *ch ) );
 void  page_to_char_bw args( ( const char *txt, CHAR_DATA *ch ) );    


/* effect.c */
void    acid_effect    args( (void *vo, int level, int dam, int target) );
void    cold_effect    args( (void *vo, int level, int dam, int target) );
void    fire_effect    args( (void *vo, int level, int dam, int target) );
void    poison_effect  args( (void *vo, int level, int dam, int target) );
void    shock_effect   args( (void *vo, int level, int dam, int target) );


//event.c
void give_exp 		args((CHAR_DATA *ch, int xp, bool show));
void stop_event		args((CHAR_DATA *ch));


/* fight.c */
bool	dex_check		args((CHAR_DATA*ch, int modifier));
bool	str_check		args((CHAR_DATA*ch, int modifier));
bool	con_check		args((CHAR_DATA*ch, int modifier));
bool	cha_check		args((CHAR_DATA*ch, int modifier));
bool	wis_check		args((CHAR_DATA*ch, int modifier));
bool	int_check		args((CHAR_DATA*ch, int modifier));
bool     is_safe        args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool     is_safe_spell    args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void    violence_update    args( ( void ) );
void    multi_hit    args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool    damage        args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int ch_class, bool show ) );
bool    damage_old      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int ch_class, bool show ) );
void    update_pos    args( ( CHAR_DATA *victim ) );
void    stop_fighting    args( ( CHAR_DATA *ch, bool fBoth ) );
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool    check_iron_will  args( ( CHAR_DATA *ch) );
void raw_kill 			args ((CHAR_DATA * victim));

/* handler.c */
void read_last_file args((CHAR_DATA *ch, int count, char *name));
void write_last_file args((char *entry));
int charisma_check args(( CHAR_DATA *ch));

int get_ore					args((int rarity));
int get_gem					args((int rarity));
int get_cloth				args((int rarity));

int		group_size			args (( CHAR_DATA *ch));
bool unique_exists			args (( long vnum ));
bool qualify_ch_class			args ((CHAR_DATA *ch, int iClass));
bool system_shock			args ((CHAR_DATA *ch, int modifier));
bool ress_shock				args ((CHAR_DATA *ch, int modifier));
bool skill_check			args(( CHAR_DATA *ch, int sn, int modifier));
bool has_cooldowns args (( CHAR_DATA *ch ));
bool get_obj_list_by_type args((CHAR_DATA * ch, int type, OBJ_DATA * list));
int food_lookup args((const char *name));
int get_faction_num args ((char * faction));
char	*		get_faction_name	args((int faction));
int				get_num_queued		args (());
int				get_material	args	((char * argument));
bool			valid_material	args	((char * argument));
int 			total_levels	args	((CHAR_DATA *ch));
int				get_spell_crit_chance args	( (CHAR_DATA *ch) );
int				get_melee_crit_chance args	( (CHAR_DATA *ch) );
int				get_spell_damage_bonus 	args (( CHAR_DATA *ch ));
void 			room_echo 		args	( (ROOM_INDEX_DATA *room, char *message) );
AD      		*affect_find 	args( (AFFECT_DATA *paf, int sn));
void    		affect_check    args( (CHAR_DATA *ch, int where, int vector) );
int    			count_users    	args( (OBJ_DATA *obj) );
void     		deduct_cost    	args( (CHAR_DATA *ch, int cost) );
void    		affect_enchant  args( (OBJ_DATA *obj) );
int     		check_immune    args( (CHAR_DATA *ch, int dam_type) );
int     		material_lookup args( ( int mat) );
int    			weapon_lookup   args( ( const char *name) );
int    			weapon_type    	args( ( const char *name) );
char     		*weapon_name    args( ( int weapon_Type) );
char			*instrument_name args( ( int instrument_type ));
char    		*item_name    	args( ( int item_type) ); 
int   			attack_lookup   args( ( const char *name) );
long    		wiznet_lookup   args( ( const char *name) );
int    			ch_class_lookup    args( ( const char *name) );
bool    		is_clan        	args( (CHAR_DATA *ch) );
bool    		is_same_clan    args( (CHAR_DATA *ch, CHAR_DATA *victim));
bool    		is_old_mob    	args ( (CHAR_DATA *ch) );
int    			get_skill    	args( ( CHAR_DATA *ch, int sn ) );
int    			get_weapon_sn   args( ( CHAR_DATA *ch, bool secondary ) );
int    			get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     		get_age         args( ( CHAR_DATA *ch ) );
void    		reset_char    	args( ( CHAR_DATA *ch )  );
int    			get_trust    	args( ( CHAR_DATA *ch ) );
int    			get_curr_stat   args( ( CHAR_DATA *ch, int stat ) );
int     		get_max_train   args( ( CHAR_DATA *ch, int stat ) );
int    			can_carry_n    	args( ( CHAR_DATA *ch ) );
int    			can_carry_w    	args( ( CHAR_DATA *ch ) );
bool    		is_name        	args( ( char *str, char *namelist ) );
bool			is_full_name	args( ( const char *str, char *namelist ) );
bool   	 		is_exact_name    args( ( char *str, char *namelist ) );
void    		affect_to_char    args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    		affect_to_obj    args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    		affect_remove    args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    		affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    		affect_strip    args( ( CHAR_DATA *ch, int sn ) );
bool    		is_affected    args( ( CHAR_DATA *ch, int sn ) );
void    		affect_join    args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    		char_from_room    args( ( CHAR_DATA *ch ) );
void    		char_to_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void    		obj_to_char    args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    		obj_from_char    args( ( OBJ_DATA *obj ) );
int    			apply_ac    args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *    		get_eq_char    args( ( CHAR_DATA *ch, int iWear ) );
OD *			get_eq_char_by_name		args( ( CHAR_DATA *ch, char *argument ));
void    		equip_char    args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void    		unequip_char    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int    			count_obj_list    args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void    		obj_from_room    args( ( OBJ_DATA *obj ) );
void    		obj_to_room    args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void    		obj_to_obj    args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void   		 	obj_from_obj    args( ( OBJ_DATA *obj ) );
void  		  	extract_obj    args( ( OBJ_DATA *obj ) );
void			extract     args( ( CHAR_DATA *ch, bool fPull ));
void    		extract_char    args( ( CHAR_DATA *ch, bool fPull ) );
CD *    		get_char_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
CD *    		get_char_world    args( ( CHAR_DATA *ch, char *argument ) );
OD *    		get_obj_type    args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *    		get_obj_list    args( ( CHAR_DATA *ch, char *argument,
                OBJ_DATA *list ) );
OD *    		get_obj_carry    args( ( CHAR_DATA *ch, char *argument, 
                CHAR_DATA *viewer ) );
OD *    		get_obj_wear    args( ( CHAR_DATA *ch, char *argument, bool character ) );
OD *    		get_obj_here    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
OD *    		get_obj_world    args( ( CHAR_DATA *ch, char *argument ) );
OD *    		create_money    args( ( int gold, int silver ) );
int    			get_obj_number    args( ( OBJ_DATA *obj ) );
int    			get_obj_weight    args( ( OBJ_DATA *obj ) );
int    			get_true_weight    args( ( OBJ_DATA *obj ) );
bool    		room_is_dark    args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    		is_room_owner    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool    		room_is_private    args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    		can_see        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    		can_see_obj    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    		can_see_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool    		can_drop_obj    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *    		affect_loc_name    args( ( int location ) );
char *    		affect_bit_name    args( ( int vector ) );
char *    		extra_bit_name    args( ( int extra_flags ) );
char *    		extra2_bit_name    args( ( int extra2_flags ) );
char *     		wear_bit_name    args( ( int wear_flags ) );
char *    		act_bit_name    args( ( int act_flags ) );
const char *	act2_bit_name args( ( int act2_flags) );
char *    		off_bit_name    args( ( int off_flags ) );
char *  		imm_bit_name    args( ( int imm_flags ) );
char *     		form_bit_name    args( ( int form_flags ) );
char *    		part_bit_name    args( ( int part_flags ) );
char *    		weapon_bit_name    args( ( int weapon_flags ) );
char *  		comm_bit_name    args( ( int comm_flags ) );
char *    		cont_bit_name    args( ( int cont_flags) );
int     		god_lookup      args( ( const char *name) );
bool			has_obj_type	args( ( CHAR_DATA *ch, int type) );
/*
 * Colour Config
 */
void    default_colour    args( ( CHAR_DATA *ch ) );
void    all_colour    args( ( CHAR_DATA *ch, char *argument ) );

/* interp.c */
void    interpret    args( ( CHAR_DATA *ch, char *argument ) );
bool    is_number    args( ( char *arg ) );
int    number_argument    args( ( char *argument, char *arg ) );
int    mult_argument    args( ( char *argument, char *arg) );
char *    one_argument    args( ( char *argument, char *arg_first ) );

/* language.c */
void    do_language     args( ( CHAR_DATA *ch, char *argument, int language) );
int     lang_lookup     args( ( const char *name ) );

/* magic.c */
bool    is_devotion		args( (int sn) ); //is the skill a psionic devotion/spell?
bool	can_cast_spell 	args( (CHAR_DATA *ch, int sn) );
int     find_spell    	args( ( CHAR_DATA *ch, const char *name) );
int     mana_cost     (CHAR_DATA *ch, int min_mana, int level);
int     skill_lookup   	args( ( const char *name ) );
int     slot_lookup    	args( ( int slot ) );
bool    saves_spell    	args( ( int level, CHAR_DATA *victim, int dam_type ) );
void    obj_cast_spell  args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) );
//map.c
void display_map args((CHAR_DATA * ch));					
			
//map2.c
void show_the_map args((CHAR_DATA * ch));					
			
/* mob_prog.c */ 
void	program_flow	args( ( long vnum, char *source, CHAR_DATA *mob,
 				OBJ_DATA *obj, ROOM_INDEX_DATA *room, 
				CHAR_DATA *ch, const void *arg1,
 				const void *arg2, int numlines ) );

PROG_CODE * get_mprog_by_vnum args( (long vnum) );


				
void	p_act_trigger	args( ( char *argument, CHAR_DATA *mob, 
				OBJ_DATA *obj, ROOM_INDEX_DATA *room,
				CHAR_DATA *ch, const void *arg1,
				const void *arg2, int type ) );
bool	p_percent_trigger args( ( CHAR_DATA *mob, OBJ_DATA *obj,
				ROOM_INDEX_DATA *room, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type ) );
void	p_bribe_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool	p_exit_trigger   args( ( CHAR_DATA *ch, int dir, int type ) );
void	p_give_trigger   args( ( CHAR_DATA *mob, OBJ_DATA *obj, 
				ROOM_INDEX_DATA *room, CHAR_DATA *ch,
				OBJ_DATA *dropped, int type ) );
void 	p_greet_trigger  args( ( CHAR_DATA *ch, int type ) );
void	p_hprct_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );

/* mob_cmds.c */
void    mob_interpret    args( ( CHAR_DATA *ch, char *argument ) );
void	obj_interpret	args( ( OBJ_DATA *obj, char *argument ) );
void 	room_interpret	args( ( ROOM_INDEX_DATA *room, char *argument ) );

/* mxp.c */
void  mxp_to_char               ( CHAR_DATA *ch, char *txt, int mxp_style );
void  shutdown_mxp              ( DESCRIPTOR_DATA *d );
void  init_mxp                  ( DESCRIPTOR_DATA *d );

//protocols
bool load_mssp_data args(( void ));
void send_mssp_data args(( DESCRIPTOR_DATA * d ));


// quest.c
QUEST_DATA *new_pc_quest args ((void));
QUEST_DATA *create_quest args((QUEST_INDEX_DATA *pQuestIndex));
void quest_to_char args((QUEST_DATA * quest, CHAR_DATA *ch));
void free_pc_quest	args((QUEST_DATA *pQuest));
bool quest_complete args((CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest));
bool has_completed_quest args((CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest));
void complete_quest args((CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest));
bool on_quest args((CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest));
long mob_has_quest args((CHAR_DATA *mob));
bool has_read_helps args((CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest));
bool can_take_quest args((CHAR_DATA *ch, QUEST_INDEX_DATA *quest));

// random_obj.c
void create_random_obj	args((CHAR_DATA *mob, OBJ_DATA *container, ROOM_INDEX_DATA *room, int level, char *type, char *material, int target, char *sub_type));
OBJ_DATA * create_random_consumable  args((int type, int level));

int get_random_healing_spell args (( int level ));

/* save.c */

void    save_char_obj    args( ( CHAR_DATA *ch ) );
bool    load_char_obj    args( ( DESCRIPTOR_DATA *d, char *name ) );
void    save_game_data  ( void );
void    load_game_data  ( void );

/* skills.c */
bool 	can_use_skill args(( CHAR_DATA *ch, int sn ));
bool     parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void     list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
long     exp_per_level    args( ( CHAR_DATA *ch, bool secondary ) );
void     check_improve    args( ( CHAR_DATA *ch, int sn, bool success, 
                    int multiplier ) );
int     group_lookup    args( (const char *name) );
void    gn_add        args( ( CHAR_DATA *ch, int gn) );
void     gn_remove    args( ( CHAR_DATA *ch, int gn) );
void     group_add    args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void    group_remove    args( ( CHAR_DATA *ch, const char *name) );

/* special.c */
SF *    spec_lookup    args( ( const char *name ) );
char *    spec_name    args( ( SPEC_FUN *function ) );

/* teleport.c */
RID *    room_by_name    args( ( char *target, int level, bool error) );

// trap.c
bool is_trapped args((OBJ_DATA *obj));
int get_trap_type args((OBJ_DATA *obj));
void remove_trap args((OBJ_DATA *obj));
void spring_trap args((CHAR_DATA *victim, OBJ_DATA *obj));
bool save_vs_trap args((CHAR_DATA *ch, OBJ_DATA *obj));

/* update.c */
void    advance_level    args( ( CHAR_DATA *ch, bool hide, bool secondary ) );
void    gain_exp    args( ( CHAR_DATA *ch, int gain, bool secondary ) );
void    gain_condition    args( ( CHAR_DATA *ch, int iCond, int value ) );
void    update_handler    args( ( void ) );
bool 	is_bountied        args( (CHAR_DATA *ch) );

/* string.c */
void    string_edit    args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *    string_replace    args( ( char * orig, char * old, char * new_str ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *    string_unpad    args( ( char * argument ) );
char *    string_proper    args( ( char * argument ) );

/* olc.c */
bool    run_olc_editor    args( ( DESCRIPTOR_DATA *d ) );
char    *olc_ed_name    args( ( CHAR_DATA *ch ) );
char    *olc_ed_vnum    args( ( CHAR_DATA *ch ) );

/* lookup.c */
int    race_lookup    args( ( const char *name) );
int    item_lookup    args( ( const char *name) );
int    liq_lookup    args( ( const char *name) );

/* Old lookup.h stuff */

int	        clan_lookup	    args( (const char *name) );
int	        position_lookup	args( (const char *name) );
int         sex_lookup	    args( (const char *name) );
int         size_lookup	    args( (const char *name) );

/* twitter.c */
void	load_twitters	args (( ));

/* update.c */
void    event_update    args( ( void ) );

#undef    CD
#undef    MID
#undef    OD
#undef    OID
#undef    RID
#undef    SF
#undef    AD

/* music.c */

void song_update args( (void) );
void load_songs	args( (void) );


//Targets for randomly created items.
#define			TARG_MOB		0
#define			TARG_OBJ		1
#define			TARG_ROOM		2

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/


/*
 * Area flags.
 */
#define         AREA_NONE       	0
#define         AREA_CHANGED    	1    	/* Area has been modified. */
#define         AREA_ADDED      	2    	/* Area has been added to. */
#define         AREA_LOADING    	4    	/* Used for counting in db.c */
#define			AREA_SHIP			8		/* Makes an area a ship */
#define			AREA_FORBIDDANCE	16		//Spell which disables planar travel in the area.
#define			AREA_LINKED			32		//If the area is not linked, players can't teleport, etc. to it. Upro
#define			AREA_RANDOM			64		//Is it a randomly generated area?
#define			AREA_ALARM			128		//Is there a magical alarm?
#define			AREA_PRECIP			256		//If set, the area cannot precipitate.
#define			AREA_CLANHALL		512		//it's a clanhall

#define MAX_DIR    6
#define NO_FLAG  -99    /* Must not be used in flags or stats. */

/*
 * Global Constants
 */
extern    char *    const    dir_name        [];
extern    const    sh_int    rev_dir         [];          /* sh_int - ROM OLC */
extern    const    struct    spec_type    spec_table    [];

/*
 * Global variables
 */
extern        AREA_DATA *  area_first;
extern        AREA_DATA *  area_last;
extern        SHOP_DATA *  shop_last;
extern		  QUEST_INDEX_DATA *	quest_last;
extern		  QUEST_INDEX_DATA *  quest_first;
extern		  GAME_DATA		game_data;

extern		  int   top_quest;
extern		  int	top_quest;
extern        int   top_affect;
extern        int   top_area;
extern        int   top_ed;
extern        int   top_exit;
extern 		  int   top_auction;
extern        int   top_help;
extern        int   top_mob_index;

extern        int   top_obj_index;
extern        int   top_reset;
extern        long   top_room;
extern        int   top_shop;

extern        long  top_vnum_mob;
extern        long  top_vnum_obj;
extern        long  top_vnum_room;
extern		  long  top_vnum_quest;

extern        char  str_empty [1];

extern        MOB_INDEX_DATA  *    mob_index_hash  [MAX_KEY_HASH];
extern        OBJ_INDEX_DATA  *    obj_index_hash  [MAX_KEY_HASH];
extern        ROOM_INDEX_DATA *    room_index_hash [MAX_KEY_HASH];
extern		  QUEST_INDEX_DATA *    quest_index_hash[MAX_KEY_HASH];



//how many new players have EVER been to this mud.
extern 		  long new_player_count;



