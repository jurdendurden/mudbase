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
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"


// #define SOC_SET_TAX					(A)	// Can set the taxes in a city.
// #define SOC_PARDON					(B)	// Can pardon people of their crimes.
// #define SOC_LAW						(C)	// Are a law figure, and can arrest/fine people.
// #define SOC_CAN_GRANT				(D)	// Means they can grant social status up to 2 levels below their own.
// #define SOC_DECLARE_WAR				(E)	// Can they declare war on other factions?
// #define SOC_CAN_BANISH				(F)	// Can they banish from the city?
// #define SOC_CAN_OUTCAST				(G)	// Can they outcast from the faction?
// #define SOC_CAN_CLAIM				(H)	// Can they claim land for the faction?
// #define SOC_CAN_TRADE				(I) // Can negotiate trade agreements with other factions.
// #define SOC_CAN_NATURALIZE			(J)	// They can make people citizens of a faction/city.
// #define SOC_CAN_PURCHASE				(K) // Authorized to make purchases for hometown using city money
// #define SOC_CAN_BUILD				(L) // Authorized to build in any area of the faction.

//Society Table:
const struct society_data society_table[] = {
	{ "Slave",		"Slave",			0 }, //0
	{ "Serf",		"Serf", 			0 },
	{ "Peasant",	"Peasant",			0 },
	{ "Freeman",	"Freewoman",		0 },
	{ "Squire",		"Squire",			0 },
	{ "Adept", 		"Clergywoman",		0 }, //5
	{ "Knight",		"Knight",			D | J | L },	
	{ "Vassal",		"Vassal",			D | J | L },
	{ "Militia",	"Militia",			C },
	{ "Sargeant",	"Sargeant",			C | D },		
	{ "Constable",	"Constable",		B | C | D | E | F | K }, //10
	{ "General",	"General",			B | C | D | E | F | H | K },
	{ "Hero",		"Heroine",			H },
	{ "Magus",		"Magus",			J },
	{ "Bishop",		"Priestess",		D | J },
	{ "Arch Magus",	"Arch Magus",		E | F | J | K }, //15
	{ "Arch Bishop","Arch Priestess",	D | F | J | K },
	{ "Lord",		"Lady",				A | B | C | D | F | J | K },
	{ "Count",		"Countess",			A | B | C | D | F | J | K },
	{ "Baron",		"Baroness",			A | B | C | D | F | G | J | K },
	{ "Duke",		"Dutchess",			A | B | C | D | F | G | J | K | L }, //20
	{ "Arch Duke",  "Arch Duchess",		A | B | C | D | F | G | I | J | K | L },	
	{ "Prince",		"Princess",			A | B | C | D | F | G | H | I | J | K | L },
	{ "King",		"Queen",			A | B | C | D | E | F | G | H | I | J | K | L } //23
};

//Faction tables:
const struct faction_data faction_table[] = {
	//name, tax rate (%), areas within
	{"Renfall", 5 , {"renfall", "greenrock barrows"}
	},
	{"Golthek", 8 , {"golthek", "ebonshear"}
	}
};


/* item type list */
const struct item_type item_table[] = {
    {ITEM_LIGHT, 				"light"			},
    {ITEM_SCROLL, 				"scroll"		},
    {ITEM_WAND, 				"wand"			},
    {ITEM_STAFF, 				"staff"			},
    {ITEM_WEAPON, 				"weapon"		},
    {ITEM_TREASURE, 			"treasure"		},
    {ITEM_ARMOR, 				"armor"			},
    {ITEM_POTION, 				"potion"		},
    {ITEM_CLOTHING, 			"clothing"		},
    {ITEM_FURNITURE, 			"furniture"		},
    {ITEM_TRASH, 				"trash"			},
    {ITEM_CONTAINER, 			"container"		},
    {ITEM_DRINK_CON, 			"drink"			},
    {ITEM_KEY, 					"key"			},
    {ITEM_FOOD, 				"food"			},
    {ITEM_MONEY, 				"money"			},
    {ITEM_BOAT, 				"boat"			},
    {ITEM_CORPSE_NPC, 			"npc_corpse"	},
    {ITEM_CORPSE_PC, 			"pc_corpse"		},
    {ITEM_FOUNTAIN, 			"fountain"		},
    {ITEM_PILL, 				"pill"			},
    {ITEM_PROTECT, 				"protect"		},
    {ITEM_MAP, 					"map"			},
    {ITEM_PORTAL, 				"portal"		},
    {ITEM_WARP_STONE, 			"warp_stone"	},
    {ITEM_ROOM_KEY, 			"room_key"		},
    {ITEM_GEM, 					"gem"			},
    {ITEM_JEWELRY, 				"jewelry"		},
    {ITEM_JUKEBOX, 				"jukebox"		},
	{ITEM_PEN,					"pen"			},
	{ITEM_PAPER,				"parchment"		},
	{ITEM_SKIN,					"skin"			},
	{ITEM_SCRY,					"scry"			},
	{ITEM_THIEVES_TOOLS,		"thieves_tools"	},
	{ITEM_HERB,					"herb"			},
	{ITEM_ALCHEMY_LAB,			"alchemy_lab"	},
	{ITEM_SALVE,				"salve"			},
	{ITEM_BANDAGE,				"bandage"		},
	{ITEM_MINING_TOOL,			"mining_tool"	},	
	{ITEM_FISH_POLE,			"fishing_pole"	},
	{ITEM_SIEVE,				"sieve"			},		
	{ITEM_MISSILE,				"missile"		},
	{ITEM_BLOOD_POOL,			"blood_pool"	},
	{ITEM_TRACKS,				"tracks"		},
	{ITEM_ANVIL,				"anvil"			},
	{ITEM_LOOM,					"loom"			},
	{ITEM_FIRE,					"fire"			},
	{ITEM_BUILDING,				"building"		},
	{ITEM_TREE,					"tree"			},
	{ITEM_FIGURINE,				"figurine"		},
	{ITEM_FLASK,				"flask"			},
	{ITEM_SHIP_HELM,			"ship_helm"		},
	{ITEM_ALCHEMY_RECIPE,		"alchemy_recipe"},
	{ITEM_COOKING_RECIPE,		"cooking_recipe"},
	{ITEM_BLACKSMITH_PLANS, 	"blacksmith_plans"},
	{ITEM_TAILORING_PLANS, 		"tailoring_plans"},
	{ITEM_INGREDIENT,			"ingredient"},
	{ITEM_SHOVEL,				"shovel"},
	{ITEM_INSTRUMENT,			"instrument"	},
	{ITEM_BLACKSMITH_HAMMER,	"blacksmith_hammer"},
    {0, NULL}
};


/* weapon selection table */
const struct weapon_type weapon_table[] = {
	//name, 		newbie vnum, 				type, 				gsn,				speed,		material_cost
    {"short_sword",	OBJ_VNUM_SCHOOL_SHORT_SWORD,WEAPON_SHORT_SWORD,	&gsn_short_sword,	1.5,		70},
    {"dagger", 		OBJ_VNUM_SCHOOL_DAGGER, 	WEAPON_DAGGER, 		&gsn_dagger,		1.25,		50},
    {"spear", 		OBJ_VNUM_SCHOOL_SPEAR, 		WEAPON_SPEAR, 		&gsn_spear,			3,			100},
	{"mace", 		OBJ_VNUM_SCHOOL_MACE, 		WEAPON_MACE, 		&gsn_mace,			2.7,		80},
    {"axe", 		OBJ_VNUM_SCHOOL_AXE, 		WEAPON_AXE, 		&gsn_axe,			3,			90},
    {"flail", 		OBJ_VNUM_SCHOOL_FLAIL, 		WEAPON_FLAIL, 		&gsn_flail,			2.6,		80},
    {"whip", 		OBJ_VNUM_SCHOOL_WHIP, 		WEAPON_WHIP, 		&gsn_whip,			2.2,		80},
    {"polearm", 	OBJ_VNUM_SCHOOL_POLEARM, 	WEAPON_POLEARM, 	&gsn_polearm,		3.7,		120},
	{"bow",			OBJ_VNUM_SCHOOL_BOW,		WEAPON_BOW,			&gsn_bow,			2.8,		80},
	{"crossbow",	OBJ_VNUM_SCHOOL_CROSSBOW, 	WEAPON_CROSSBOW,	&gsn_crossbow,		3.5,		100},
	{"staff", 		OBJ_VNUM_SCHOOL_STAFF, 		WEAPON_STAFF, 		&gsn_staff,			3,			100},
	{"long_sword",	OBJ_VNUM_SCHOOL_SWORD, 		WEAPON_LONG_SWORD,	&gsn_long_sword,	2.5,		90},	
    {NULL, 			0, 							0, 					NULL,				0,			0}
};

// Bard instruments

const struct instrument_type  instrument_table [] =
{
     { "lute",          INSTR_LUTE,      &gsn_lute      },
     { "harp",          INSTR_HARP,      &gsn_harp      },
     { "drums",         INSTR_DRUMS,     &gsn_drums     },
     { "piccolo",       INSTR_PICCOLO,   &gsn_piccolo   },  
     { "horn",          INSTR_HORN,      &gsn_horn      },
     { NULL  ,              0,           NULL           }
};


const struct flag_type instrument_type_flags[] =
{
     { "lute",          INSTR_LUTE,             true },
     { "harp",          INSTR_HARP,             true },
     { "drums",         INSTR_DRUMS,            true },
     { "piccolo",       INSTR_PICCOLO,          true },
     { "horn",          INSTR_HORN,             true },
     { NULL  ,              0,                  0       }
     
};
     

const struct flag_type instrument_quality_flags[] =
{
    { "horrible",       QUA_HORRIBLE,           true },
    { "decent",         QUA_DECENT,             true },
    { "good",           QUA_GOOD,               true },
    { "superb",         QUA_SUPERB,             true },
    { "exquisite",      QUA_EXQUISITE,          true },
    { NULL  ,           0,                      0       }
};

/* wiznet table and prototype for future flag setting */
const struct wiznet_type wiznet_table[] = {
    {"on", WIZ_ON, IM},
    {"prefix", WIZ_PREFIX, IM},
    {"ticks", WIZ_TICKS, IM},
    {"logins", WIZ_LOGINS, IM},
    {"sites", WIZ_SITES, L4},
    {"links", WIZ_LINKS, L7},
    {"newbies", WIZ_NEWBIE, IM},
    {"spam", WIZ_SPAM, L5},
    {"deaths", WIZ_DEATHS, IM},
    {"resets", WIZ_RESETS, L4},
    {"mobdeaths", WIZ_MOBDEATHS, L4},
    {"flags", WIZ_FLAGS, L5},
    {"penalties", WIZ_PENALTIES, L5},
    {"saccing", WIZ_SACCING, L5},
    {"levels", WIZ_LEVELS, IM},
    {"load", WIZ_LOAD, L2},
    {"restore", WIZ_RESTORE, L2},
    {"snoops", WIZ_SNOOPS, L2},
    {"switches", WIZ_SWITCHES, L2},
    {"secure", WIZ_SECURE, L1},
    {NULL, 0, 0}
};


/* attack table  -- not very organized :( */
const struct attack_type attack_table[MAX_DAMAGE_MESSAGE] = {
    {"none", "hit", -1},        /*  0 */
    {"slice", "slice", DAM_SLASH},
    {"stab", "stab", DAM_PIERCE},
    {"slash", "slash", DAM_SLASH},
    {"whip", "whip", DAM_SLASH},
    {"claw", "claw", DAM_SLASH},    /*  5 */
    {"blast", "blast", DAM_BASH},
    {"pound", "pound", DAM_BASH},
    {"crush", "crush", DAM_BASH},
    {"grep", "grep", DAM_SLASH},
    {"bite", "bite", DAM_PIERCE},    /* 10 */
    {"pierce", "pierce", DAM_PIERCE},
    {"suction", "suction", DAM_BASH},
    {"beating", "beating", DAM_BASH},
    {"digestion", "digestion", DAM_ACID},
    {"charge", "charge", DAM_BASH},    /* 15 */
    {"slap", "slap", DAM_BASH},
    {"punch", "punch", DAM_BASH},
    {"wrath", "wrath", DAM_ENERGY},
    {"magic", "magic", DAM_ENERGY},
    {"divine", "divine power", DAM_HOLY},    /* 20 */
    {"cleave", "cleave", DAM_SLASH},
    {"scratch", "scratch", DAM_PIERCE},
    {"peck", "peck", DAM_PIERCE},
    {"peckb", "peck", DAM_BASH},
    {"chop", "chop", DAM_SLASH},    /* 25 */
    {"sting", "sting", DAM_PIERCE},
    {"smash", "smash", DAM_BASH},
    {"shbite", "shocking bite", DAM_LIGHTNING},
    {"flbite", "flaming bite", DAM_FIRE},
    {"frbite", "freezing bite", DAM_COLD},    /* 30 */
    {"acbite", "acidic bite", DAM_ACID},
    {"chomp", "chomp", DAM_PIERCE},
    {"drain", "life drain", DAM_NEGATIVE},
    {"thrust", "thrust", DAM_PIERCE},
    {"slime", "slime", DAM_ACID},
    {"shock", "shock", DAM_LIGHTNING},
    {"thwack", "thwack", DAM_BASH},
    {"flame", "flame", DAM_FIRE},
    {"chill", "chill", DAM_COLD},
	{"earth", "crushing earth", DAM_EARTH},
	{"air",	  "gale wind",DAM_AIR},
    {NULL, NULL, 0}
};

/* race table */
const struct race_type race_table[] = {
/*
    {
    name,        pc_race?,
    act bits,    aff_by bits,    off bits,
    imm,        res,        vuln,
    form,        parts 
    },
*/
    {"unique", FALSE, 
	0, 0, 0, 
	0, 0, 0, 
	0, 0},

    {
     "human", TRUE,
     0, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

    {
     "elf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_AIR | RES_CHARM, VULN_COLD,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

    {
     "dwarf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_EARTH | RES_DISEASE, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

    {
     "half orc", TRUE,
     0, 0, 0,
     0, RES_COLD | RES_POISON, VULN_MENTAL | VULN_CHARM,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

	{
     "half elf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_CHARM, VULN_COLD,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "halfling", TRUE,
     0, 0, 0,
     0, RES_MENTAL, VULN_DROWNING | VULN_CHARM,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

	 {
     "gnome", TRUE,
     0, 0, 0,
     0, RES_MENTAL | RES_EARTH, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

	 {
     "duergar", TRUE,
     0, AFF_INFRARED, 0,
     IMM_DISEASE, RES_EARTH, VULN_DROWNING | VULN_HOLY | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "drow", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_POISON | RES_CHARM, VULN_LIGHT | VULN_HOLY | VULN_SOUND,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "half ogre", TRUE,
     0, 0, 0,
     0, RES_BASH | RES_LIGHTNING, VULN_MENTAL | VULN_CHARM,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "gnoll", TRUE,
     0, 0, OFF_BERSERK,
     0, RES_COLD | RES_DISEASE, VULN_FIRE | VULN_CHARM,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},
	 
	{
     "minotaur", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_CHARM | RES_LIGHTNING, VULN_FIRE | VULN_MENTAL,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "aarakocra", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_SOUND | RES_AIR | RES_LIGHTNING, VULN_FIRE | VULN_MENTAL,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | P},
	 
	{
     "revenant", TRUE,
     0, AFF_DARK_VISION, 0,
     IMM_NEGATIVE, RES_SOUND | RES_COLD, VULN_FIRE | VULN_HOLY,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "aasimar", TRUE,
     0, AFF_DARK_VISION, 0,
     IMM_HOLY, RES_LIGHT | RES_AIR, VULN_NEGATIVE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

	{
     "bugbear", TRUE,
     0, 0, 0,
     0, RES_DISEASE | RES_POISON, VULN_FIRE | VULN_CHARM,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	
	{
     "tabaxi", TRUE,
     0, AFF_DARK_VISION, 0,
     0, RES_COLD, VULN_FIRE,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},
	 
	{
     "lizardman", TRUE,
     0, 0, 0,
     0, RES_ACID | RES_BASH, VULN_COLD | VULN_PIERCE,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},
	
	{
     "tiefling", TRUE,
     0, AFF_DARK_VISION, 0,
     0, RES_MAGIC, VULN_LIGHT, 
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | V},
	 
	{
     "loxodon", TRUE,
     0, 0, 0,
     0, RES_MENTAL | RES_ENERGY, VULN_LIGHTNING | VULN_SOUND,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | V},
	
	{
     "centaur", TRUE,
     0, 0, 0,
     0, RES_EARTH, VULN_AIR,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "mermaid", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DROWNING, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "merman", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DROWNING, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "triton", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DROWNING, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "harpy", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DROWNING, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | P},
	 
	 {
     "owlbear", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DROWNING, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "animal", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | B | C | D | E | F | H | J | K | U | V},
	 
    {
     "bat", FALSE,
     0, AFF_FLYING | AFF_DARK_VISION, OFF_DODGE | OFF_FAST,
     0, 0, VULN_LIGHT,
     A | G | V, A | C | D | E | F | H | J | K | P},

    {
     "bear", FALSE,
     0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
     0, RES_BASH | RES_COLD, 0,
     A | G | V, A | B | C | D | E | F | H | J | K | U | V},
	
	 
    {
     "cat", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
	{
     "lion", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
	{
     "tiger", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
	{
     "leopard", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
	{
     "cheetah", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
	{
     "panther", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
	 {
     "puma", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},

	{
     "mountain lion", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V},
	 
    {
     "centipede", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH,
     A | B | G | O, A | C | K},

	{
     "millipede", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH,
     A | B | G | O, A | C | K},
	 
	{
     "worm", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH,
     A | B | G | O, A | K},
	 
	 {
     "deer", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 {
     "moose", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 
	 {
     "elk", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 {
	 	 
     "demon", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON | RES_MAGIC, VULN_HOLY | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
	 "devil", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON | RES_MAGIC, VULN_HOLY | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 

    {
     "doll", FALSE,
     0, 0, 0,
     IMM_COLD | IMM_POISON | IMM_HOLY | IMM_NEGATIVE | IMM_MENTAL | IMM_DISEASE | IMM_DROWNING, 
	 RES_BASH | RES_LIGHT,
     VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_LIGHTNING | VULN_ENERGY,
     E | J | M | cc, A | B | C | G | H | K},

    {"white dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_COLD, RES_DROWNING | RES_BASH | RES_CHARM | RES_MAGIC,
     VULN_PIERCE | VULN_FIRE,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"black dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_ACID, RES_POISON | RES_DISEASE | RES_BASH | RES_CHARM | RES_MAGIC,
     VULN_PIERCE | VULN_HOLY | VULN_DROWNING,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"blue dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_LIGHTNING, RES_BASH | RES_CHARM | RES_MAGIC | RES_DROWNING,
     VULN_PIERCE | VULN_EARTH | VULN_AIR,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"green dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_POISON | IMM_DISEASE, RES_BASH | RES_CHARM | RES_MAGIC | RES_NEGATIVE,
     VULN_PIERCE | VULN_DROWNING,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"red dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_FIRE, RES_BASH | RES_CHARM | RES_MAGIC | RES_NEGATIVE,
     VULN_PIERCE | VULN_COLD | VULN_DROWNING,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"brass dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     0, RES_FIRE | RES_BASH | RES_CHARM | RES_MAGIC | RES_AIR,
     VULN_PIERCE | VULN_COLD | VULN_EARTH,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"bronze dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
	 0, RES_DROWNING | RES_BASH | RES_CHARM | RES_MAGIC | RES_AIR,
     VULN_PIERCE | VULN_EARTH | VULN_LIGHTNING,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"copper dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_ACID, RES_EARTH | RES_BASH | RES_CHARM | RES_POISON,
     VULN_PIERCE | VULN_AIR | VULN_DROWNING,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},

	 {"silver dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     0, RES_AIR | RES_COLD | RES_BASH | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_PIERCE | VULN_FIRE,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"gold dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_FIRE, RES_LIGHTNING | RES_BASH | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_PIERCE | VULN_COLD,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"shadow dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_NEGATIVE | IMM_DISEASE | IMM_POISON, RES_COLD | RES_CHARM | RES_MAGIC,
     VULN_PIERCE | VULN_FIRE | VULN_HOLY | VULN_LIGHT,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	{"astral dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_CHARM | IMM_SUMMON, RES_SLASH | RES_BASH | RES_MAGIC | RES_PIERCE,
     VULN_SOUND | VULN_MENTAL,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"ethereal dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_CHARM | IMM_SUMMON, RES_PIERCE | RES_CHARM | RES_MAGIC,
     VULN_SLASH | VULN_SOUND,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 //50
	 {"rust dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     0, RES_FIRE | RES_SLASH | RES_CHARM | RES_MAGIC | RES_DROWNING,
     VULN_LIGHTNING | VULN_BASH,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"onyx dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_PIERCE, RES_FIRE | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_BASH | VULN_SOUND,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"ruby dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_PIERCE, RES_FIRE | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_BASH | VULN_SOUND,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"emerald dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_PIERCE, RES_FIRE | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_BASH | VULN_SOUND,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"sapphire dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_PIERCE, RES_FIRE | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_BASH | VULN_SOUND,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},
	 
	 {"crystal dragon", FALSE,
     0, AFF_INFRARED | AFF_FLYING, 0,
     IMM_PIERCE, RES_FIRE | RES_CHARM | RES_MAGIC | RES_MENTAL,
     VULN_BASH | VULN_SOUND,
     A | H | Z, A | C | D | E | F | G | H | I | J | K | P | Q | U | V | X},	 
	 
	 {"deva", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {"asura", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
    
	{
     "octopus", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | K | N | Q},
	
	{
     "squid", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | K | N | Q},
	
	{
     "jellyfish", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | N | Q},
	
	{
     "fish", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | O | K | Q | V},
	 
	{
     "shark", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | O | K | Q | V},
	 
	{
     "whale", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | O | K | Q | V},
	 
	{
     "dolphin", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | V, A | D | E | F | O | K | Q | V},
	
	{
     "dog", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | V | U | X},
	
	{
     "fido", FALSE,
     0, 0, OFF_DODGE | ASSIST_RACE,
     0, 0, VULN_MAGIC,
     A | B | G | V, A | C | D | E | F | H | J | K | Q | V},

    {
     "fox", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | V},
	 
	 {
     "wolf", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 //70
	{
     "jackal", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 {
     "coyote", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 {
     "warg", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, RES_POISON | RES_DISEASE, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 {
	 "monkey", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
	"ape", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	


	
    {
     "goblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_MAGIC,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

    {
     "hobgoblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},

	 {
     "insect", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH | VULN_FIRE,
     A | B | G | O, A | C | K},
    
	{
     "kobold", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON, VULN_MAGIC,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Q},

    {
     "lizard", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | cc, A | C | D | E | F | H | K | Q | V},
	 //80
	 {
     "alligator", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | cc, A | C | D | E | F | H | K | Q | V},
	 
	 {
     "crocodile", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | cc, A | C | D | E | F | H | K | Q | V},

    {
     "modron", FALSE,
     0, AFF_INFRARED, ASSIST_RACE | ASSIST_ALIGN,
     IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE,
     RES_FIRE | RES_COLD | RES_ACID, 0,
     H, A | B | C | G | H | J | K},

    {
     "orc", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

    {
     "pig", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K},
	 
	{
     "boar", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Y},
	 
    {
     "rabbit", FALSE,
     0, 0, OFF_DODGE | OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K},

	{
     "rat", FALSE,
     0, 0, OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K},
	 
     {
     "elemental", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC, VULN_SUMMON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},

    {
     "snake", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | Y | cc, A | D | E | F | K | L | Q | V | X},
//90
    {
     "song bird", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "owl", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},
	 
	{
     "hawk", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "crow", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "raven", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "eagle", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "penguin", FALSE,
     0, 0, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},
	 
	{
     "sparrow", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "chicken", FALSE,
     0, 0, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},
	
	{
     "turkey", FALSE,
     0, 0, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "roc", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},
	 
	{
     "duck", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "goose", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P},

	{
     "spider", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH | VULN_FIRE,
     A | B | G | O, A | C | K},
	
    {
     "troll", FALSE,
     0, AFF_REGENERATION | AFF_INFRARED | AFF_DETECT_HIDDEN,
     OFF_BERSERK,
     0, RES_CHARM | RES_BASH, VULN_FIRE | VULN_ACID,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},

    {
     "water fowl", FALSE,
     0, AFF_SWIM | AFF_FLYING, 0,
     0, RES_DROWNING, 0,
     A | G | W, A | C | D | E | F | H | K | P},

    {
     "wyvern", FALSE,
     0, AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,
     OFF_BASH | OFF_FAST | OFF_DODGE,
     IMM_POISON, 0, VULN_LIGHT,
     A | B | G | Z, A | C | D | E | F | H | J | K | Q | V | X},
//95
	{
     "undead", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "skeleton", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "wraith", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "zombie", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "ghoul", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "wight", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "mummy", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "vampire", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	 
	 {
     "ghost", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	
	{
     "lich", FALSE,
     0, AFF_INFRARED, ASSIST_RACE,
	 IMM_NEGATIVE, 0, VULN_LIGHT | VULN_FIRE | VULN_HOLY,     
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V},	 
	//105 
	{
     "ooze", FALSE,
     0, 0, 0,
     0, 0, 0,
     A, A | F},
	 
	 {
     "sphinx", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	{
     "griffon", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V | P},
	 
	{
     "chimera", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V | P},
	 
	{
     "manticore", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V | P},
	 //110
	{
     "hippogriff", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V | P}, 
	 
	 {
     "pegasus", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V | P}, 
	
	{
     "unicorn", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	
	{
     "horse", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	    
	{
     "donkey", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	{
     "mule", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 {
     "zebra", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	 {
     "giraffe", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	{
     "camel", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	{
     "sheep", FALSE,
     0, AFF_DARK_VISION, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 //120
	{
     "goat", FALSE,
     0, AFF_DARK_VISION, OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},
	 
	{
     "kuo-toa", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DROWNING, VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "pixie", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC, VULN_NEGATIVE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "djinn", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM, RES_MAGIC | RES_MENTAL, VULN_SUMMON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	{
     "gargoyle", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DISEASE | IMM_POISON, RES_PIERCE, VULN_BASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "illithid", FALSE,
     0, AFF_INFRARED | AFF_DARK_VISION, 0,
     0, RES_MENTAL, VULN_SLASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | N},
	 
	{
     "beholder", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MENTAL | RES_MAGIC, VULN_SLASH | VULN_LIGHT,
     A, A | M | E | L | K },
	
	{
     "imp", FALSE,
     0, AFF_INFRARED, 0,
     IMM_DISEASE, RES_POISON, VULN_HOLY,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
		 
//130
	{
     "quickling", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_CHARM, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},	
	 
	 {
     "githyanki", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON | RES_MAGIC, VULN_HOLY | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "githzerai", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON | RES_MAGIC, VULN_HOLY | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "satyr", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM, RES_COLD, VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	 {
     "dryad", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM, RES_MAGIC, VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "naiad", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM | IMM_DROWNING, RES_MAGIC, VULN_FIRE | VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "ogre", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_BASH | RES_DISEASE, VULN_MENTAL | VULN_CHARM,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "sahuagin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DROWNING, VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "slaad", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DROWNING | RES_POISON, VULN_LIGHTNING | VULN_PIERCE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
     "thri-kreen", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DROWNING | RES_MENTAL, VULN_FIRE | VULN_BASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 //140	
	 
	{
     "drider", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC, VULN_LIGHT | VULN_HOLY,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "treant", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_BASH, VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "yugoloth", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_MENTAL, VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	//144
	{
	"iron golem", FALSE,
     0, 0, 0,
     IMM_CHARM | IMM_MENTAL, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
	"stone golem", FALSE,
     0, 0, 0,
     IMM_CHARM | IMM_MENTAL, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{
	 "wood golem", FALSE,
     0, 0, 0,
     IMM_CHARM | IMM_MENTAL, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	
	{	 
	"flesh golem", FALSE,
     0, 0, 0,
     IMM_CHARM | IMM_MENTAL, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},
	 
	{
     "hill giant", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_LIGHTNING | RES_DROWNING, VULN_CHARM | VULN_MENTAL,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "mountain giant", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_LIGHTNING | RES_DROWNING, VULN_CHARM | VULN_MENTAL, 
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 //150
	 {
     "storm giant", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_LIGHTNING | RES_DROWNING, VULN_CHARM | VULN_EARTH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "fire giant", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_FIRE, VULN_CHARM | VULN_COLD,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "frost giant", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD | RES_DROWNING, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "titan", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_BASH | RES_DROWNING, VULN_CHARM | VULN_AIR,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "cyclops", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_BASH | RES_DROWNING, VULN_CHARM | VULN_MENTAL,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	  {
     "ettin", FALSE,
     0, 0, 0,
     0, RES_BASH | RES_DROWNING, VULN_CHARM | VULN_MENTAL | VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "cockatrice", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM | IMM_SUMMON, RES_AIR, VULN_FIRE | VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "basilisk", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM | IMM_SUMMON, RES_FIRE, VULN_PIERCE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "medusa", FALSE,
     0, AFF_INFRARED, 0,
     IMM_CHARM | IMM_SUMMON, RES_COLD, VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "banshee", FALSE,
     0, AFF_INFRARED, 0,
     IMM_SOUND, RES_COLD | RES_MAGIC, VULN_FIRE | VULN_HOLY,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "grippli", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "troglodyte", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_LIGHTNING, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "turtle", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD | RES_SLASH, VULN_CHARM | VULN_BASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "armadillo", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD | RES_SLASH, VULN_CHARM | VULN_BASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "bird", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "yuan-ti", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	 {
     "seahorse", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	
	{
     "crustacean", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_COLD, VULN_CHARM | VULN_FIRE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	{
     "yochlol", FALSE,
     0, AFF_INFRARED, 0,
    IMM_SUMMON, RES_MAGIC | RES_CHARM | RES_COLD | RES_LIGHTNING | RES_MENTAL | RES_DROWNING, VULN_FIRE | VULN_HOLY | VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y},
	 
	{
     "intellect devourer", FALSE,
     0, 0, 0,
     IMM_MENTAL, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},

	{
     "cow", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},

	{
     "bison", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V},

	{NULL, FALSE, 0, 0, 0, 0, 0, 0, 0, 0}
};

const struct pc_race_type pc_race_table[] = {
    {"null race", "", "",
    {""}, 
	{13, 13, 13, 13, 13, 13}, {18, 18, 18, 18, 18, 18}, SIZE_MEDIUM, 0, 80,
	FALSE, FALSE, FALSE},

/*
    {
    "race name",     short name,  short description,
    { bonus skills },
    { base stats },        { max stats },        size , 		base age,		max age,
		CAN_GOOD, CAN_NEUTRAL, CAN_EVIL
    },
*/
    {
     "human",     "  Human   ", "Humans are truly jacks of all trades, and can suit most classes well.",
     {"lore"},
     {13, 13, 13, 13, 13, 13}, {18, 18, 18, 18, 18, 18}, SIZE_MEDIUM, 	18,		80,
	 TRUE, TRUE, TRUE},

    {
     "elf",       "   Elf    ", "A shorter, thinner version of humans, elves tend to be very wise and charismatic.",
     {"sneak", "detect magic"},
     {12, 14, 13, 15, 11, 15}, {16, 20, 20, 21, 16, 21}, SIZE_SMALL,	80,		450,
	 TRUE, TRUE, FALSE},

    {
     "dwarf",     "  Dwarf   ", "Dwarves are short stocky, stout humanoids with a penchant for mining and fighting.",
     {"berserk", "detect magic", "axe"},
     {14, 12, 14, 10, 15, 10}, {21, 16, 19, 14, 21, 16}, SIZE_MEDIUM,	50,		300,
	 TRUE, TRUE, TRUE},

    {
     "half orc",  " Half Orc ", "Half orcs tend to be homely, with brutish features, but possess excellent strength.",
     {"bash", "fast healing"},
     {14, 11, 13, 12, 13, 9}, {19, 17, 18, 17, 19, 14}, SIZE_MEDIUM,		16,		65,
	 TRUE, TRUE, TRUE},
	 
	{
     "half elf",  " Half Elf ", "A cross breed of some race and elf, these creatures tend to be quite well rounded.",
     {"sneak", "detect magic"},
     {13, 13, 13, 14, 12, 14}, {17, 19, 18, 19, 17, 20}, SIZE_MEDIUM,	16,		175,
	 TRUE, TRUE, TRUE},
	 
	 {
     "halfling",  " Halfling ", "This short race of folk tend to be very dextrous, and make well rounded clerics.",
     {"sneak", "hide"},
     {10, 12, 13, 16, 10, 11}, {15, 18, 18, 22, 16, 18}, SIZE_SMALL,	30,		120,
	 TRUE, TRUE, FALSE},
	 
	 {
     "gnome",     "  Gnome   ", "Gnomes are a short folk with excellent magical capability. They make great wizards.",
     {"meditation", "detect magic", "recharge"},
     {11, 14, 13, 11, 13, 11}, {18, 21, 20, 18, 18, 17}, SIZE_SMALL,	45,		250,
	 TRUE, TRUE, TRUE},
	 
	 {
     "duergar",   " Duergar  ", "Considered evil dwarves, they live in the Underdark. These dwarves are quite strong.",
     {"berserk", "detect magic", "axe", "detect hidden"},
     {14, 12, 13, 10, 16, 10}, {20, 16, 19, 14, 22, 16}, SIZE_SMALL,	50,		300,
	 FALSE, TRUE, TRUE},
	 
	 {
     "drow",      "   Drow   ", "Dark elves with agility and cunning to match their surface dwelling cousins.",
     {"sneak", "detect magic", "invisibility"},
     {11, 14, 13, 16, 11, 15}, {17, 21, 18, 22, 16, 21}, SIZE_SMALL,	80,		500,
	 FALSE, TRUE, TRUE},
	 
	 {
     "half ogre", "Half Ogre ", "Ogres mixed with some other race, these creatures are very powerful and healthy.",
     {"bash", "enhanced damage"},
     {16, 10, 10, 10, 16, 10}, {23, 17, 17, 16, 23, 15}, SIZE_LARGE,	18,		75,
	 FALSE, TRUE, TRUE},
	 	  
	{
     "gnoll",     "   Gnoll  ", "A dog-like breed, with excellent tracking abilities and vicious fighting tendencies.",
     {"butcher", "spear"},
     {15, 10, 10, 12, 14, 10}, {19, 16, 19, 20, 19, 12}, SIZE_MEDIUM,	20,		90,
	 FALSE, TRUE, TRUE},
	 
	{
     "minotaur",  " Minotaur ", "The head and legs of a bull on the body of a man. Powerful creatures with little wits.",
     {"enhanced damage", "axe", "gore"},
     {15, 11, 12, 12, 15, 9}, {21, 17, 19, 18, 21, 13}, SIZE_LARGE,		25,		90,
	 FALSE, TRUE, TRUE},
	 
	{
     "aarakocra", "Aarakocra ", "Birdfolk, capable of prolonged flight. These creatures are both agile and quite strong.",
     {"dodge"},
     {13, 10, 13, 15, 12, 10}, {20, 16, 18, 21, 18, 15}, SIZE_MEDIUM,		20,		70,
	 TRUE, TRUE, FALSE},

	{
     "revenant",  " Revenant ", "Undead, fueled by hatred for the living. These creature are suitable for most classes.",
     {""},
     {12, 11, 11, 12, 12, 11}, {19, 19, 19, 19, 20, 15}, SIZE_MEDIUM,		200,		1000,
	 FALSE, FALSE, TRUE},

	{
     "aasimar",   " Aasimar  ", "Angelic creatures, and beings of pure good. Aasimar are very strong and beautiful to behold.",
     {"protection evil"},
     {12, 12, 11, 12, 13, 15}, {21, 18, 20, 18, 20, 23}, SIZE_MEDIUM,		200,		1000,
	 TRUE, FALSE, FALSE},

	{
     "bugbear",   " Bugbear  ", "Large, tusked, fur covered goblinoids of brute strength and little intelligence.",
     {"bash", "gore"},
     {13, 10, 13, 12, 15, 10}, {21, 15, 18, 18, 21, 15}, SIZE_LARGE,		15,		50,
	 FALSE, TRUE, TRUE},
	 
	{
     "tabaxi",    "  Tabaxi  ", "Tabaxi are an extremely agile humanoid cat race, with sharp wits. They excel at most all.",
     {"hand to hand"},
     {11, 12, 10, 16, 12, 10}, {18, 20, 20, 23, 19, 18}, SIZE_MEDIUM,		20,		80,
	 TRUE, TRUE, TRUE},
	 
	{
     "lizardman", "Lizardman ", "Large, strong and extremely physically healthy, these species tend to hide from the public.",
     {"fast healing"},
     {13, 10, 10, 13, 16, 10}, {21, 15, 18, 18, 23, 15}, SIZE_LARGE,		10,		150,
	 FALSE, TRUE, TRUE},
	 
	{
     "tiefling",  " Tiefling ", "Tieflings are hybrid resultant of an unholy pact. They are very intelligent and charismatic.",
     {"protection good"},
     {10, 13, 11, 12, 12, 15}, {18, 20, 20, 23, 19, 21}, SIZE_MEDIUM,		18,		90,
	 FALSE, TRUE, TRUE},
	 
	{
     "loxodon",   " Loxodon ", "The humanoid elephants of Aragond, these beasts are very wise and strong, with high fortitude.",
     {"meditation", "gore"},
     {12, 12, 15, 10, 14, 11}, {20, 19, 23, 17, 21, 18}, SIZE_LARGE,		60,		450,
	 TRUE, TRUE, TRUE},
	 
	{
     "centaur",   " Centaur ", "Centaurs are hybrid creatures mixed with bipedals and horses. They combine the best of both.",
     {"kick"},
     {12, 11, 14, 12, 14, 11}, {19, 18, 18, 20, 20, 18}, SIZE_MEDIUM,		20,		120,
	 TRUE, TRUE, TRUE}
};




/*
 *ch_class table.
 */
const struct ch_class_type ch_class_table[MAX_CLASS] = {    
	
	//name, who name, main stat, 2ndary stat, school weapon,
	//guild locations, adept %, thaco 0, thaco 32, hp min per lvl, mp max per lvl, are they a caster class??
	//base group, xp for first level, short who name (multiclass), max bulk, max item bulk, can cast?, is secret?
	
	{
     "wizard", " Wizard  ", STAT_INT, -1, OBJ_VNUM_SCHOOL_DAGGER,
     {1004, 9618}, 50, 20, 6, 2, 4, TRUE,
     "wizard basics", 2500, "Wzd ", 5, 1, TRUE, FALSE},

    {
     "cleric", " Cleric  ", STAT_WIS, -1, OBJ_VNUM_SCHOOL_MACE,
     {3003, 9619}, 50, 20, 2, 2, 8, TRUE,
     "cleric basics", 1500, "Clrc", 15, 2, TRUE, FALSE},

    {
     "thief", "  Thief  ", STAT_DEX, -1, OBJ_VNUM_SCHOOL_DAGGER,
     {3028, 9639}, 50, 20, -4, 2, 6, FALSE,
     "thief basics", 1250, "Thf ", 15, 2, FALSE, FALSE},

    {
     "fighter", " Fighter ", STAT_STR, STAT_CON, OBJ_VNUM_SCHOOL_SWORD,
     {1024, 9633}, 60, 20, -10, 3, 10, FALSE,
     "fighter basics",  2000, "Fgtr", 50, 5, FALSE, FALSE},
	 
	 {
     "druid", "  Druid  ", STAT_WIS, STAT_CON, OBJ_VNUM_SCHOOL_STAFF,
     {3003, 9619}, 50, 20, 0, 2, 8, TRUE,
     "druid basics", 1750, "Drd ", 20, 3, TRUE, FALSE},
	 
	 {
     "ranger", " Ranger  ", STAT_CON, STAT_WIS, OBJ_VNUM_SCHOOL_SHORT_SWORD,
     {3022, 9633}, 50, 20, -8, 2, 9, FALSE,
     "ranger basics", 2250, "Rngr", 35, 4, TRUE, FALSE},
	 
	 {
     "paladin", " Paladin ", STAT_CON, STAT_WIS, OBJ_VNUM_SCHOOL_LONG_SWORD,
     {3022, 9633}, 50, 20, -8, 2, 12, FALSE,
     "paladin basics", 2250, "Pldn", 50, 5, TRUE, FALSE},
	 
	 {
     "bard", "  Bard   ", STAT_CHA, STAT_INT, OBJ_VNUM_SCHOOL_SHORT_SWORD,
     {3022, 9633}, 50, 20, -2, 2, 7, TRUE,
     "bard basics", 1250, "Bard", 30, 3, TRUE, FALSE},
	 
	 {
     "monk", "  Monk   ", STAT_DEX, STAT_WIS, OBJ_VNUM_SCHOOL_MACE,
     {3022, 9633}, 50, 20, -6, 2, 5, FALSE,
     "monk basics", 2250, "Monk", 15, 2, TRUE, FALSE},
	 
	 {
     "psion", "  Psion  ", STAT_INT, STAT_CON, OBJ_VNUM_SCHOOL_DAGGER,
     {3022, 9633}, 50, 20, 4, 2, 5, FALSE,
     "monk basics", 2200, "Psi ", 15, 1, TRUE, TRUE}
};

const   struct  lang_type lang_table [ MAX_LANGUAGE ] =
{
        { "common"      },
        { "human"       },
        { "dwarvish"    },
        { "elvish"      },
        { "gnomish"     },
        { "goblin"      },
        { "orcish"      },
        { "ogre"        },
        { "drow"        },
        { "kobold"      },
        { "trollish"    },
        { "halfling"    },
		{ "gnollish"	},
};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] = {
    {-5, -4, 0, 0},                /* 0  */
    {-5, -4, 3, 1},                /* 1  */
    {-3, -2, 3, 2},
    {-3, -1, 15, 3},            /* 3  */
    {-2, -1, 30, 4},
    {-2, -1, 65, 5},            /* 5  */
    {-1, 0, 90, 6},
    {-1, 0, 100, 7},
    {0, 0, 105, 8},
    {0, 0, 115, 9},
    {0, 0, 125, 10},            /* 10  */
    {0, 0, 135, 11},
    {0, 0, 145, 12},
    {0, 0, 155, 13},            /* 13  */
    {0, 1, 170, 14},
    {1, 1, 185, 15},            /* 15  */
    {1, 2, 200, 16},
    {2, 3, 225, 22},
    {2, 3, 250, 25},            /* 18  */
    {3, 4, 275, 30},
    {3, 5, 300, 35},            /* 20  */
    {4, 6, 325, 40},
    {4, 6, 375, 45},
    {5, 7, 425, 50},
    {5, 8, 525, 55},
    {6, 9, 650, 60}                /* 25   */
};



const struct int_app_type int_app[26] = {
    {2},                        /*  0 */
    {3},                        /*  1 */
    {5},
    {6},                        /*  3 */
    {7},
    {8},                        /*  5 */
    {9},
    {10},
    {10},
    {10},
    {11},                        /* 10 */
    {11},
    {12},
    {13},
    {14},
    {15},                        /* 15 */
    {16},
    {17},
    {18},                        /* 18 */
    {19},
    {20},                        /* 20 */
    {21},
    {22},
    {23},
    {24},
    {25}                        /* 25 */
};



const struct wis_app_type wis_app[26] = {
//practice, inherent potential, heal_adj, magic defense adjustment
    {0,0,-20,-15},                        /*  0 */
    {0,0,-18,-13},                        /*  1 */
    {0,0,-16,-11},
    {0,0,-14,-9},                        /*  3 */
    {0,1,-12,-7},
    {1,1,-10,-6},                        /*  5 */
    {1,1,-8,-5},
    {1,2,-6,-4},
    {1,2,-4,-3},
    {1,5,-2,-2},
    {1,5,0,-1},                        /* 10 */
    {1,10,0,0},
    {1,10,0,0},
    {1,15,2,0},
    {1,15,2,0},
    {2,20,4,1},                        /* 15 */
    {2,22,4,2},
    {2,24,6,3},
    {3,26,7,4},                        /* 18 */
    {3,28,9,5},
    {3,30,11,6},                        /* 20 */
    {3,32,13,7},
    {4,34,15,9},
    {4,36,17,11},
    {4,38,20,13},
    {5,40,25,15}                            /* 25 */
};



const struct dex_app_type dex_app[26] = {
//AC modifier, reaction, attacks.
    {60, -10, -10, -2},                        /* 0 */
    {50, -9, -9, -2},                        /* 1 */
    {50, -8, -8, -2},
    {40, -6, -6, -2},
    {30, -4, -4, -2},
    {20, -3, -3, -2},                        /* 5 */
    {10, -2, -2, -2},
    {0, -1, -1, -1},
    {0, -1, -1, -1},
    {0, 0, 0, -1},
    {0, 0, 0, -1},                        /* 10 */
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {-10, 2, 2, 0},                        /* 15 */
    {-15, 2, 2, 1},
    {-20, 3, 3, 1},
    {-30, 3, 3, 1},
    {-40, 4, 4, 1},
    {-50, 5, 5, 2},                        /* 20 */
    {-60, 5, 5, 2},
    {-75, 6, 6, 2},
    {-90, 7, 7, 2},
    {-105, 8, 8, 3},
    {-120, 9, 9, 4}                        /* 25 */
};


const struct con_app_type con_app[26] = {
    {-4, 20,10,-5},                    /*  0 */
    {-3, 25,15,-2},                    /*  1 */
    {-2, 30,20,-2},
    {-2, 35,25,-2},                    /*  3 */
    {-1, 40,30,-2},
    {-1, 45,30,-2},                    /*  5 */
    {-1, 50,35,-2},
    {0, 55,35,-2},
    {0, 60,40,-2},
    {0, 65,45,-2},
    {0, 70,50,-1},                    /* 10 */
    {0, 75,55,-1},
    {0, 80,55,-1},
    {0, 85,60,-1},
    {0, 88,65,0},
    {1, 90,70,0},                    /* 15 */
    {2, 95,75,1},
    {2, 97,80,2},
    {3, 99,80,3},                    /* 18 */
    {3, 99,85,4},
    {4, 99,88,5},                    /* 20 */
    {4, 99,91,6},
    {5, 99,94,7},
    {6, 99,97,8},
    {7, 99,98,8},
    {8, 99,99,9}                        /* 25 */
};

const struct cha_app_type cha_app[26] = {
    {0, 0, -4},                    /*  0 */
    {0, 0, -4},                    /*  1 */
    {0, 0, -4},
    {0, 0, -4},                    /*  3 */
    {0, 0, -4},
    {0, 0, -3},                    /*  5 */
    {0, 0, -3},
    {0, 0, -3},
    {0, 0, -2},
    {0, 0, -2},
    {1, 1, -2},                    /* 10 */
    {1, 2, -1},
    {1, 4, -1},
    {1, 6, 0},
    {1, 7, 0},
    {2, 8, 1},                    /* 15 */
    {2, 10, 1},
    {3, 12, 2},
    {4, 15, 2},                   /* 18 */
    {4, 20, 3},
    {5, 30, 3},                   /* 20 */
    {5, 40, 4},
    {6, 50, 4},
    {6, 60, 5},
    {7, 80, 5},
    {8, 100, 6}                        /* 25 */
};


const struct food_type food_table[] = {
 // Name,	food hours, 	full hours,		poisoned?,		long descr,
	{"a test cake",2,2,FALSE,"a yummy looking test cake is here."},
	{"",0,0,FALSE,""}
 };


/*
 * Liquid properties.
 * Used in world.obj.
 */
const struct liq_type liq_table[] = {
/*    name            color    proof, full, thirst, food, serving size (ounces) */
    {"water", "clear", {0, 1, 10, 0, 16}},
    {"beer", "amber", {12, 1, 8, 1, 12}},
    {"red wine", "burgundy", {30, 1, 8, 1, 5}},
    {"ale", "brown", {15, 1, 8, 1, 12}},
    {"dark ale", "dark", {16, 1, 8, 1, 12}},

    {"whiskey", "golden", {120, 1, 5, 0, 2}}, //5
    {"lemonade", "pink", {0, 1, 9, 2, 12}},
    {"firebreather", "boiling", {190, 0, 4, 0, 2}},
    {"local specialty", "clear", {151, 1, 3, 0, 2}},
    {"slime mold juice", "green", {0, 2, -8, 1, 2}},

    {"milk", "white", {0, 2, 9, 3, 12}}, //10
    {"tea", "tan", {0, 1, 8, 0, 6}},
    {"coffee", "black", {0, 1, 8, 0, 6}},
    {"blood", "red", {0, 2, -1, 2, 6}},
    {"salt water", "clear", {0, 1, -2, 0, 1}},

    {"coke", "brown", {0, 2, 9, 2, 12}}, //15
    {"root beer", "brown", {0, 2, 9, 2, 12}},
    {"elvish wine", "green", {35, 2, 8, 1, 5}},
    {"white wine", "golden", {28, 1, 8, 1, 5}},
    {"champagne", "golden", {32, 1, 8, 1, 5}},

    {"mead", "honey-colored", {34, 2, 8, 2, 12}}, //20
    {"rose wine", "pink", {26, 1, 8, 1, 5}},
    {"benedictine wine", "burgundy", {40, 1, 8, 1, 5}},
    {"vodka", "clear", {130, 1, 5, 0, 2}},
    {"cranberry juice", "red", {0, 1, 9, 2, 12}},

    {"orange juice", "orange", {0, 2, 9, 3, 12}}, //25
    {"absinthe", "green", {200, 1, 4, 0, 2}},
    {"brandy", "golden", {80, 1, 5, 0, 4}},
    {"aquavit", "clear", {140, 1, 5, 0, 2}},
    {"schnapps", "clear", {90, 1, 5, 0, 2}},

    {"icewine", "purple", {50, 2, 6, 1, 5}}, //30
    {"amontillado", "burgundy", {35, 2, 8, 1, 5}},
    {"sherry", "red", {38, 2, 7, 1, 5}},
    {"framboise", "red", {50, 1, 7, 1, 5}},
    {"rum", "amber", {151, 1, 4, 0, 2}},

    {"cordial", "clear", {100, 1, 5, 0, 2}},
	{"holy water", "clear", {0, 1, 10, 0, 16 }}, //37
	{"unholy water", "clear", {0, 1, 10, 0, 16 }}, //38
    {NULL, NULL, {0, 0, 0, 0, 0}}
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)    n

const struct skill_type skill_table[MAX_SKILL] = {

/*
 * Magic spells.
 */
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "reserved", {99, 99, 99, 99, 99, 99, 99, 99, 99, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     0, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (0), 0, 0,
     "", "", "", FALSE, NULL, STAT_INT, INITIATE, 0, 0},

    {
     "acid blast", {15, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (70), 28, 12,
     "acid blast", "!Acid Blast!","", TRUE, &gsn_acid_arrow, STAT_INT, MEMBER, 8, 0},

    {
     "armor", {2, 2, 5, 5, 2, 15, 7, 9, 5, 41}, {1, 1, 0, 0, 1, 0, 2, 0, 1, 0},
     spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT (1), 9, 12,
     "", "You feel less armored.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "bless", {5, 7, 5, 5, 7, 40, 3, 9, 3, 41}, {0, 1, 0, 0, 1, 0, 2, 0, 2, 0},
     spell_bless, TAR_OBJ_CHAR_DEF, POS_STANDING,
     NULL, SLOT (3), 8, 12,
     "", "You feel less righteous.",
     "$p's holy aura fades.", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "blindness", {8, 8, 5, 5, 8, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_blindness, SLOT (4), 18, 12,
     "", "You can see again.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "burning hands", {1, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (5), 10, 12,
     "burning hands", "!Burning Hands!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "call lightning", {20, 18, 5, 5, 13, 5, 9, 9, 9, 41}, {1, 2, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_call_lightning, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (6), 32, 12,
     "lightning bolt", "!Call Lightning!", "", TRUE, NULL, STAT_INT, OFFICER, 7, 0},

    {"calm", {20, 16, 5, 5, 18, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_calm, TAR_IGNORE, POS_FIGHTING,
     &gsn_calm, SLOT (509), 30, 12,
     "", "You have lost your peace of mind.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "cancellation", {14, 18, 5, 5, 19, 5, 9, 20, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 2, 0, 0},
     spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT (507), 20, 12,
     "", "!cancellation!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "cause critical", {5, 13, 5, 5, 13, 5, 17, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (63), 20, 12,
     "cause critical wounds", "!Cause Critical!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_FIVE_SECONDS},

    {
     "cause light", {5, 1, 5, 5, 1, 5, 3, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (62), 8, 12,
     "cause light wounds", "!Cause Light!", "", TRUE, NULL, STAT_INT, INITIATE, 1, TIME_FIVE_SECONDS},

    {
     "cause serious", {5, 7, 5, 5, 7, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (64), 14, 12,
     "cause serious wounds", "!Cause Serious!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_FIVE_SECONDS},

    {
     "chain lightning", {25, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (500), 27, 12,
     "lightning", "!Chain Lightning!", "", TRUE, NULL, STAT_INT, OFFICER, 8, TIME_TEN_SECONDS},

    {
     "change sex", {9, 9, 5, 5, 9, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT (82), 15, 12,
     "", "Your body feels familiar again.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "charm person", {13, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_charm_person, SLOT (7), 15, 12,
     "", "You feel more self-confident.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "chill touch", {1, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (8), 15, 12,
     "chilling touch", "You feel less cold.", "", TRUE, NULL, STAT_INT, INITIATE, 1, TIME_FIVE_SECONDS},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "colour spray", {16, 5, 5, 5, 20, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (10), 16, 12,
     "colour spray", "!Colour Spray!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_FIVE_SECONDS},

    {
     "continual light", {6, 4, 5, 5, 5, 20, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_continual_light, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (57), 7, 12,
     "", "!Continual Light!", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},

    {
     "control weather", {15, 19, 5, 5, 9, 5, 9, 9, 9, 41}, {2, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_control_weather, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (11), 25, 12,
     "", "!Control Weather!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "create food", {1, 1, 5, 5, 4, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_create_food, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (12), 5, 12,
     "", "!Create Food!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "create rose", {1, 1, 10, 24, 10, 20, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_create_rose, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (511), 30, 12,
     "", "!Create Rose!", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},

    {
     "create spring", {1, 1, 5, 5, 17, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_create_spring, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (80), 20, 12,
     "", "!Create Spring!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "create water", {1, 1, 5, 5, 4, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_create_water, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT (13), 5, 12,
     "", "!Create Water!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "cure blindness", {5, 6, 5, 5, 6, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT (14), 12, 12,
     "", "!Cure Blindness!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "cure critical", {5, 13, 5, 5, 14, 5, 18, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 2, 0, 0, 0},
     spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     &gsn_cure_critical, SLOT (15), 21, 9,
     "spell", "!Cure Critical!", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},

    {
     "cure disease", {5, 13, 5, 5, 13, 5, 13, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 2, 0, 0, 0},
     spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT (501), 22, 12,
     "", "!Cure Disease!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "cure light", {5, 1, 5, 5, 1, 5, 5, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     &gsn_cure_light, SLOT (16), 8, 9,
     "spell", "!Cure Light!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "cure poison", {5, 14, 5, 5, 12, 25, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT (43), 15, 12,
     "", "!Cure Poison!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "cure serious", {5, 7, 5, 5, 7, 20, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     &gsn_cure_serious, SLOT (61), 15, 9,
     "spell", "!Cure Serious!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "curse", {10, 10, 5, 5, 18, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_curse, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
     &gsn_curse, SLOT (17), 20, 12,
     "curse", "The curse wears off.",
     "$p is no longer impure.", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "demonfire", {5, 25, 5, 5, 5, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (505), 20, 12,
     "demonic flame", "!Demonfire!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},

    {
     "detect evil", {11, 4, 5, 5, 4, 5, 2, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_detect_evil, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (18), 5, 12,
     "", "The red in your vision disappears.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "detect good", {11, 4, 5, 5, 4, 5, 2, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_detect_good, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (513), 5, 12,
     "", "The gold in your vision disappears.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "detect hidden", {15, 11, 5, 5, 11, 25, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING,
     &gsn_detect_hidden, SLOT (44), 5, 12,
     "", "You feel less aware of your surroundings.",
     "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "detect invis", {3, 8, 5, 5, 9, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_detect_invis, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (19), 5, 12,
     "", "You no longer see invisible objects.",
     "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "detect magic", {2, 6, 5, 8, 3, 6, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_detect_magic, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (20), 3, 12,
     "", "The detect magic wears off.", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},

    {
     "detect poison", {15, 7, 25, 5, 4, 10, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 1, 0, 0, 0, 0},
     spell_detect_poison, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT (21), 5, 12,
     "", "!Detect Poison!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "dispel evil", {5, 15, 5, 5, 15, 5, 8, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 2, 0, 0, 0},
     spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (22), 18, 12,
     "dispel evil", "!Dispel Evil!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "dispel good", {5, 15, 5, 5, 15, 5, 8, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 2, 0, 0, 0},
     spell_dispel_good, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (512), 18, 12,
     "dispel good", "!Dispel Good!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "dispel magic", {16, 24, 5, 5, 24, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (59), 28, 12,
     "", "!Dispel Magic!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "earthquake", {5, 10, 5, 5, 12, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_earthquake, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (23), 35, 12,
     "earthquake", "!Earthquake!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_TEN_SECONDS},

    {
     "enchant armor", {16, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_enchant_armor, TAR_OBJ_INV, POS_STANDING,
     &gsn_enchant_armor, SLOT (510), 100, 24,
     "", "!Enchant Armor!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "enchant weapon", {17, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
     &gsn_enchant_weapon, SLOT (24), 100, 24,
     "", "!Enchant Weapon!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "energy drain", {19, 22, 5, 5, 22, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (25), 31, 12,
     "energy drain", "!Energy Drain!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "faerie fire", {6, 3, 5, 5, 2, 25, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 1, 0, 0, 0, 0},
     spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (72), 5, 12,
     "faerie fire", "The pink aura around you fades away.",
     "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "faerie fog", {14, 21, 5, 5, 11, 25, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_faerie_fog, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (73), 10, 12,
     "faerie fog", "!Faerie Fog!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "farsight", {14, 16, 5, 5, 16, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_farsight, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (521), 16, 20,
     "farsight", "!Farsight!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "fireball", {22, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (26), 23, 12,
     "fireball", "!Fireball!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_TEN_SECONDS},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "fireproof", {1, 1, 19, 18, 1, 17, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_fireproof, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT (523), 10, 12,
     "", "", "$p's protective aura fades.", TRUE, NULL, STAT_INT, INITIATE, 0, 0},

    {
     "flamestrike", {5, 20, 5, 5, 25, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (65), 30, 12,
     "flamestrike", "!Flamestrike!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},

    {
     "fly", {5, 5, 25, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_fly, SLOT (56), 10, 18,
     "", "You slowly float to the ground.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "floating disc", {4, 10, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_floating_disc, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (522), 10, 24,
     "", "!Floating disc!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "frenzy", {5, 24, 5, 5, 20, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_frenzy, SLOT (504), 20, 24,
     "", "Your rage ebbs.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "gate", {25, 17, 5, 5, 27, 5, 9, 9, 9, 41}, {2, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_gate, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (83), 70, 12,
     "", "!Gate!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_ONE_MINUTE},

    {
     "giant strength", {8, 5, 5, 5, 12, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_giant_strength, SLOT (39), 16, 12,
     "", "You feel weaker.", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "harm", {5, 23, 5, 5, 34, 5, 9, 9, 9, 41}, {0, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (27), 30, 12,
     "harm spell", "!Harm!", "", TRUE, NULL, STAT_INT, OFFICER, 7, TIME_FIVE_SECONDS},

    {
     "haste", {10, 5, 5, 5, 14, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     &gsn_haste, SLOT (502), 22, 12,
     "", "You feel yourself slow down.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "heal", {5, 21, 5, 5, 25, 5, 9, 9, 9, 41}, {0, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     &gsn_heal, SLOT (28), 45, 12,
     "heal spell", "!Heal!", "", TRUE, NULL, STAT_INT, OFFICER, 7, TIME_TEN_SECONDS},

    {
     "heat metal", {5, 16, 5, 5, 29, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_heat_metal, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (516), 25, 18,
     "spell", "!Heat Metal!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "holy word", {5, 36, 5, 5, 40, 5, 9, 9, 9, 41}, {0, 3, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_holy_word, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (506), 200, 24,
     "divine wrath", "!Holy Word!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_ONE_DAY},

    {
     "identify", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_identify, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT (5), 12, 12,
     "", "!Identify!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "infravision", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT (77), 5, 18,
     "", "You no longer see in the dark.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "invisibility", {5, 5, 15, 5, 15, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_invis, TAR_OBJ_CHAR_DEF, POS_STANDING,
     &gsn_invis, SLOT (29), 9, 12,
     "", "You are no longer invisible.",
     "$p fades into view.", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},

    {
     "know alignment", {1, 1, 30, 5, 1, 42, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_know_alignment, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT (58), 9, 12,
     "", "!Know Alignment!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {//65
     "lightning bolt", {13, 23, 5, 5, 8, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_lightning_bolt, SLOT (30), 15, 12,
     "lightning bolt", "!Lightning Bolt!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_FIVE_SECONDS},

    {
     "locate object", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_locate_object, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (31), 20, 18,
     "", "!Locate Object!", "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "magic missile", {1, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (32), 18, 12,
     "magic missile", "!Magic Missile!", "", TRUE, NULL, STAT_INT, INITIATE, 1, TIME_FIVE_SECONDS},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "mass healing", {5, 35, 5, 5, 40, 5, 9, 9, 9, 41}, {0, 3, 0, 0, 3, 0, 0, 0, 0, 0},
     spell_mass_healing, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (508), 100, 24,
     "", "!Mass Healing!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},

    {
     "mass invis", {22, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_mass_invis, TAR_IGNORE, POS_STANDING,
     &gsn_mass_invis, SLOT (69), 40, 18,
     "", "You are no longer invisible.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "nexus", {40, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_nexus, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (520), 150, 36,
     "", "!Nexus!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_ONE_DAY},

    {
     "pass door", {5, 5, 43, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_pass_door, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (74), 20, 12,
     "", "You feel solid again.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "plague", {23, 17, 5, 5, 19, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_plague, SLOT (503), 20, 12,
     "sickness", "Your sores vanish.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "poison", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_poison, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
     &gsn_poison, SLOT (33), 10, 12,
     "poison", "You feel less sick.",
     "The poison on $p dries up.", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "portal", {30, 30, 5, 5, 30, 5, 9, 9, 9, 41}, {3, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_portal, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (519), 100, 24,
     "", "!Portal!", "", TRUE, NULL, STAT_INT, OFFICER, 7, TIME_TWICE_PER_DAY},

    {//75
     "protection evil", {1, 1, 17, 16, 1, 15, 5, 9, 5, 41}, {1, 1, 0, 0, 1, 1, 1, 0, 1, 0},
     spell_protection_evil, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (34), 5, 12,
     "", "You feel less protected.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "protection good", {1, 1, 17, 16, 1, 15, 5, 9, 5, 41}, {1, 1, 0, 0, 1, 1, 1, 0, 1, 0},
     spell_protection_good, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT (514), 5, 12,
     "", "You feel less protected.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "ray of truth", {5, 25, 5, 5, 25, 5, 9, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_ray_of_truth, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (518), 20, 12,
     "ray of truth", "!Ray of Truth!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},

    {
     "recharge", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_recharge, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT (517), 60, 24,
     "", "!Recharge!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "refresh", {1, 1, 12, 9, 1, 7, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT (81), 12, 18,
     "refresh", "!Refresh!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},

    {
     "remove curse", {5, 18, 5, 5, 24, 40, 14, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 2, 0, 0, 0},
     spell_remove_curse, TAR_OBJ_CHAR_DEF, POS_STANDING,
     NULL, SLOT (35), 15, 12,
     "", "!Remove Curse!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "sanctuary", {8, 8, 15, 15, 8, 15, 10, 9, 12, 41}, {1, 1, 0, 0, 1, 0, 2, 0, 2, 0},
     spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_sanctuary, SLOT (36), 60, 12,
     "", "The white aura around your body fades.",
     "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "shield", {1, 1, 35, 5, 1, 40, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT (67), 14, 18,
     "", "Your force shield shimmers then fades away.",
     "", TRUE, NULL, STAT_INT, INITIATE, 2, 0},

    {
     "shocking grasp", {1, 5, 5, 5, 26, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (999), 13, 12,
     "shocking grasp", "!Shocking Grasp!", "", TRUE, NULL, STAT_INT, INITIATE, 1, TIME_FIVE_SECONDS},

    {
     "sleep", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_sleep, SLOT (38), 15, 12,
     "", "You feel less tired.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},

    {
     "slow", {12, 14, 5, 5, 14, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_slow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_slow, SLOT (515), 25, 12,
     "", "You feel yourself speed up.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "stone skin", {10, 10, 10, 10, 10, 10, 10, 10, 10, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_stone_skin, TAR_CHAR_SELF, POS_STANDING,
     &gsn_stone_skin, SLOT (66), 22, 24,
     "", "Your skin feels soft again.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "summon", {20, 20, 5, 5, 20, 5, 9, 9, 9, 41}, {2, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_summon, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (40), 40, 12,
     "", "!Summon!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},

    {
     "teleport", {13, 22, 40, 5, 28, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_teleport, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (2), 30, 12,
     "", "!Teleport!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},

    {
     "ventriloquate", {1, 5, 2, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_ventriloquate, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (41), 5, 12,
     "", "!Ventriloquate!", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},

    {
     "weaken", {11, 14, 16, 5, 22, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (68), 20, 12,
     "spell", "You feel stronger.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},

    {
     "word of recall", {4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, {1, 1, 0, 0, 1, 1, 1, 1, 1, 1},
     spell_word_of_recall, TAR_CHAR_DEFENSIVE, POS_RESTING,
     NULL, SLOT (42), 5, 12,
     "", "!Word of Recall!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_THREE_PER_DAY},
	 
	 {
     "shadow bolt", {9, 9, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_shadow_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (602), 19, 12,
     "shadow bolt", "!Shadow Bolt!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_FIVE_SECONDS},
	 
	 {
     "manavert", {19, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_manavert, TAR_CHAR_SELF, POS_FIGHTING,
     NULL, SLOT (602), 35, 0,
     "manavert", "Your mystical energy alignment returns to normal.", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_THREE_PER_DAY},
	 
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
/*
 * Dragon breath
 */
    {
     "acid breath", {35, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (200), 150, 24,
     "blast of acid", "!Acid Breath!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},

    {
     "fire breath", {40, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (201), 150, 24,
     "blast of flame", "The smoke leaves your eyes.", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},

    {
     "frost breath", {34, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (202), 150, 24,
     "blast of frost", "!Frost Breath!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},

    {
     "gas breath", {39, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_gas_breath, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (203), 150, 24,
     "blast of gas", "!Gas Breath!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},

    {
     "lightning breath", {37, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (204), 150, 24,
     "blast of lightning", "!Lightning Breath!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},
	//Added by Upro below. 
	
	{
     "shadow breath", {38, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_shadow_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (205), 150, 24,
     "shadowy blast", "!Shadow Breath!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},
	
	{
     "energy breath", {36, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_energy_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (204), 150, 24,
     "blast of energy", "!Energy Breath!", "", TRUE, NULL, STAT_INT, MASTER, 9, TIME_TWICE_PER_DAY},
	
/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
     "general purpose", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (401), 0, 12,
     "general purpose ammo", "!General Purpose Ammo!", "", FALSE, NULL, STAT_INT, MASTER, 9, 0},

    {//100
     "high explosive", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (402), 0, 12,
     "high explosive ammo", "!High Explosive Ammo!", "", FALSE, NULL, STAT_INT, MASTER, 9, 0},


 //Spells added by Upro and co. 2009-
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	{
     "knock", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_knock, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (600), 10, 12,
     "", "!Knock!", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, (TIME_TEN_SECONDS * 2)},
	 
	{
     "fear", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_fear, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (601), 30, 12,
     "", "!Fear!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_ONE_MINUTE},	 
	 
	 {
     "audible glamor", {1, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_audible_glamor, TAR_CHAR_SELF, POS_STANDING,
     &gsn_audible_glamor, SLOT (603), 23, 12,
     "", "You feel less charismatic.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	 {
     "extension", {25, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_extension, TAR_CHAR_SELF, POS_STANDING,
     &gsn_extension, SLOT (604), 60, 18,
     "", "You are no longer extending your spells.", "", TRUE, NULL, STAT_INT, OFFICER, 8, 0},
	 
	 {
     "confusion", {20, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_confusion, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_confusion, SLOT (605), 35, 12,
     "", "Your mind no longer feels muddled.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
	 
	 {
     "web", {5, 5, 5, 5, 5, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_web, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_web, SLOT (606), 18, 12,
     "", "You are no longer covered in cobwebs.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	 {
     "silence", {15, 15, 5, 5, 15, 5, 9, 9, 9, 41}, {2, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_silence, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_silence, SLOT (607), 30, 12,
     "", "Your vocal chords feel normal once more.", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_TWICE_PER_DAY},
	 
	 {
     "scry", {15, 15, 5, 5, 15, 5, 9, 9, 9, 41}, {2, 2, 0, 0, 2, 0, 0, 0, 0, 0},
     spell_scry, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_scry, SLOT (608), 80, 12,
     "", "!Scry!", "", TRUE, NULL, STAT_INT, MEMBER, 6, 0},
	 
	 {
     "detect scrying", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_detect_scrying, TAR_CHAR_SELF, POS_STANDING,
     &gsn_detect_scrying, SLOT (609), 15, 12,
     "", "You are no longer sensitive to scrying.", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},
	 
	 {
     "displacement", {15, 25, 5, 5, 32, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_displacement, TAR_CHAR_SELF, POS_FIGHTING,
     &gsn_displacement, SLOT (610), 40, 12,
     "", "You are no longer displacing yourself.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
	 
	 {
     "disrupt undead", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_disrupt_undead, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_disrupt_undead, SLOT (611), 10, 12,
     "disruption", "!Disrupt Undead!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	 {
     "enlarge person", {15, 15, 5, 5, 15, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_enlarge_person, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_enlarge_person, SLOT (612), 20, 12,
     "", "You return to your normal size.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	{
     "reduce person", {15, 15, 5, 5, 15, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_reduce_person, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_reduce_person, SLOT (612), 20, 12,
     "", "You return to your normal size.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},

	 {
     "etherealness", {25, 25, 5, 5, 32, 5, 9, 9, 9, 41}, {2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_etherealness, TAR_CHAR_SELF, POS_STANDING,
     &gsn_etherealness, SLOT (613), 150, 18,
     "", "!Etherealness!", "", TRUE, NULL, STAT_INT, OFFICER, 7, 0},
	 
	 {
     "feeblemind", {15, 15, 5, 5, 15, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_feeblemind, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_feeblemind, SLOT (614), 35, 12,
     "", "Your mind returns to it's original state.", "", TRUE, NULL, STAT_INT, MEMBER, 6, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion	 
	 {
     "paralyzation", {15, 15, 5, 5, 15, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_paralyzation, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_paralyzation, SLOT (615), 45, 12,
     "", "You can move your limbs once more.", "", TRUE, NULL, STAT_INT, MEMBER, 6, 0},
	//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion 
	 {
     "detect undead", {1, 1, 5, 5, 1, 5, 2, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_detect_undead, TAR_CHAR_SELF, POS_STANDING,
     &gsn_detect_undead, SLOT (616), 10, 12,
     "", "You no longer sense the presence of undead.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	 {
     "detect traps", {1, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_detect_traps, TAR_CHAR_SELF, POS_STANDING,
     &gsn_detect_traps, SLOT (617), 10, 12,
     "", "You no longer sense the presence of traps.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	 {
     "mana shield", {25, 25, 5, 5, 32, 5, 9, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_mana_shield, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     &gsn_mana_shield, SLOT (618), 1, 6,
     "", "Your mana shield flickers then fades.", "", TRUE, NULL, STAT_INT, OFFICER, 8, 0},
	 
	 {
     "embalm", {6, 25, 5, 5, 32, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_embalm, TAR_OBJ_INV, POS_FIGHTING,
     &gsn_embalm, SLOT (618), 12, 6,
     "", "!embalm!", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	 {
     "acid arrow", {5, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_acid_arrow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_acid_arrow, SLOT (619), 17, 12,
     "acidic arrow", "!Acid Arrow!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	 {
     "flame arrow", {5, 1, 5, 5, 1, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_flame_arrow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_flame_arrow, SLOT (620), 17, 12,
     "flaming arrow", "!Flame Arrow!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	 {
     "frost arrow", {5, 25, 5, 5, 32, 5, 9, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_frost_arrow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_frost_arrow, SLOT (621), 17, 12,
     "frost arrow", "!Frost Arrow!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	{
     "blink", {11, 25, 5, 5, 32, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_blink, TAR_CHAR_SELF, POS_RESTING,
     &gsn_blink, SLOT (623), 35, 12,
     "", "You are no longer blinking in and out of existance.", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
	 
	 {//125
     "disintegrate", {12, 25, 5, 5, 32, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_disintegrate, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_disintegrate, SLOT (623), 23, 12,
     "ray of disintegration", "!disingtegrate!", "", TRUE, NULL, STAT_INT, MEMBER, 6, TIME_THREE_PER_DAY},
	 
	 {
     "stone meld", {15, 25, 5, 5, 15, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_stone_meld, TAR_CHAR_SELF, POS_RESTING,
     &gsn_stone_meld, SLOT (623), 25, 12,
     "", "You are no longer melded with the stone.", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	 
	 {
     "barkskin", {15, 25, 5, 5, 10, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_barkskin, TAR_CHAR_SELF, POS_RESTING,
     &gsn_barkskin, SLOT (623), 22, 12,
     "", "Your skin returns to it's normal texture.", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion 
	 {
     "shillelagh", {15, 25, 5, 5, 5, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_shillelagh, TAR_OBJ_INV, POS_RESTING,
     &gsn_shillelagh, SLOT (623), 20, 12,
     "", "!Shillelagh!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	 {
     "entangle", {15, 25, 5, 5, 5, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_entangle, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_entangle, SLOT (623), 18, 12,
     "", "The vines holding you recede into the ground.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	 {
     "detect plants", {15, 25, 5, 5, 1, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_detect_plants, TAR_CHAR_SELF, POS_RESTING,
     &gsn_detect_plants, SLOT (623), 10, 12,
     "", "You no longer sense individual plants.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	 {
     "resistance", {41, 25, 5, 5, 1, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_resistance, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_resistance, SLOT (623), 14, 12,
     "", "You no longer resist magic.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},

	  {
     "sunbeam", {5, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_sunbeam, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_sunbeam, SLOT (622), 32, 12,
     "sunbeam", "!Sunbeam!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
	{
     "fire seeds", {15, 25, 5, 5, 1, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_fire_seeds, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_fire_seeds, SLOT (623), 15, 12,
     "fire seeds", "!Fire Seeds!", "", TRUE, NULL, STAT_INT, INITIATE, 1, TIME_FIVE_SECONDS},
	 
	 {
     "thorns", {15, 25, 5, 5, 10, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_thorns, TAR_CHAR_SELF, POS_RESTING,
     &gsn_thorns, SLOT (623), 20, 12,
     "", "You are no longer protected by thorns.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	 {
     "animal growth", {15, 25, 5, 5, 10, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_animal_growth, TAR_CHAR_SELF, POS_RESTING,
     &gsn_animal_growth, SLOT (623), 25, 12,
     "", "Your return to your normal size.", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	 
	 {
     "changestaff", {15, 25, 5, 5, 15, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_changestaff, TAR_OBJ_INV, POS_RESTING,
     &gsn_changestaff, SLOT (623), 50, 12,
     "", "!Changestaff!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
	 
	 {
     "water breathing", {15, 25, 5, 5, 15, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_water_breathing, TAR_CHAR_SELF, POS_FIGHTING,
     &gsn_water_breathing, SLOT (623), 10, 12,
     "", "Your lungs return to normal.", "", TRUE, NULL, STAT_INT, MEMBER, 6, 0},
	 
	 {
     "regeneration", {15, 25, 5, 5, 10, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_regeneration, TAR_CHAR_SELF, POS_RESTING,
     &gsn_regeneration, SLOT (623), 30, 12,
     "", "You are no longer magically regenerating.", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_TWICE_PER_DAY},
	 
	 {
     "transmute metal", {41, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_transmute_metal, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_transmute_metal, SLOT (623), 100, 12,
     "", "!Transmute!", "", TRUE, NULL, STAT_INT, OFFICER, 8, TIME_THREE_PER_DAY},

	{
     "summon monster", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_summon_monster, TAR_IGNORE, POS_RESTING,
     &gsn_summon_monster, SLOT (623), 60, 12,
     "", "!summon monster!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_TWICE_PER_DAY},
	 
	{
     "summon fire", {15, 16, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_summon_fire, TAR_IGNORE, POS_RESTING,
     &gsn_summon_fire, SLOT (623), 80, 18,
     "", "!summon fire!", "", TRUE, NULL, STAT_WIS, MEMBER, 4, TIME_THREE_PER_DAY},
	 
	{
     "summon water", {15, 16, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_summon_water, TAR_IGNORE, POS_RESTING,
     &gsn_summon_water, SLOT (623), 80, 18,
     "", "!summon water!", "", TRUE, NULL, STAT_WIS, MEMBER, 4, TIME_THREE_PER_DAY},

	 
	{
     "summon air", {15, 16, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_summon_air, TAR_IGNORE, POS_RESTING,
     &gsn_summon_air, SLOT (623), 80, 18,
     "", "!summon air!", "", TRUE, NULL, STAT_WIS, MEMBER, 4, TIME_THREE_PER_DAY},
	 
	{
     "summon earth", {15, 16, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_summon_earth, TAR_IGNORE, POS_RESTING,
     &gsn_summon_earth, SLOT (623), 80, 18,
     "", "!summon earth!", "", TRUE, NULL, STAT_WIS, MEMBER, 4, TIME_THREE_PER_DAY},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	{
     "blade barrier", {25, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_blade_barrier, TAR_CHAR_SELF, POS_RESTING,
     &gsn_blade_barrier, SLOT (623), 40, 12,
     "", "Your blade barrier fades from existance.", "", TRUE, NULL, STAT_INT, MEMBER, 6, 0},
	 
	{
     "globe of invulnerability", {32, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_globe_invulnerability, TAR_CHAR_SELF, POS_RESTING,
     &gsn_globe_invulnerability, SLOT (623), 180, 24,
     "", "Your globe of invulnerability fades from view.", "", TRUE, NULL, STAT_INT, MEMBER, 6, TIME_ONE_DAY},
	 
	 {
     "gaseous form", {26, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_gaseous_form, TAR_CHAR_SELF, POS_RESTING,
     &gsn_gaseous_form, SLOT (623), 30, 12,
     "", "!gaseous form!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
	 
	 {
     "ghoul touch", {17, 18, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_ghoul_touch, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_ghoul_touch, SLOT (623), 45, 12,
     "ghoul touch", "!ghoul touch!", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	 
	 {
     "forbiddance", {25, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_forbiddance, TAR_CHAR_SELF, POS_RESTING,
     &gsn_forbiddance, SLOT (623), 85, 18,
     "", "!forbiddance!", "", TRUE, NULL, STAT_INT, MEMBER, 8, TIME_ONE_WEEK},
	 
	 {
     "explosive runes", {14, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_explosive_runes, TAR_OBJ_INV, POS_RESTING,
     &gsn_explosive_runes, SLOT (623), 35, 18,
     "explosive runes", "!explosive runes!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_ONE_MINUTE},
	 
	 {
     "fire storm", {28, 26, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_fire_storm, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_fire_storm, SLOT (623), 50, 12,
     "fiery blast", "!fire storm!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_TEN_SECONDS},
	 
	 {
     "gust of wind", {16, 25, 5, 5, 22, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_gust_of_wind, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_gust_of_wind, SLOT (623), 40, 12,
     "gust of wind", "!gust of wind!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_FIVE_SECONDS},
	 
	 {
     "ice storm", {27, 26, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_ice_storm, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_ice_storm, SLOT (623), 50, 12,
     "chilling blast", "!ice storm!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_TEN_SECONDS},
	 
	 {
     "meteor swarm", {40, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {5, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_meteor_swarm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_meteor_swarm, SLOT (623), 250, 24,
     "magical meteor", "!meteor swarm!", "", FALSE, NULL, STAT_INT, OFFICER, 8, TIME_ONE_DAY},
	 
	 {
     "nightmare", {11, 11, 5, 5, 11, 5, 1, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_nightmare, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_nightmare, SLOT (623), 40, 12,
     "", "Your dreams are no longer being disturbed.", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	 
	 {
     "create fire", {4, 25, 5, 5, 6, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_create_fire, TAR_CHAR_SELF, POS_RESTING,
     &gsn_create_fire, SLOT (623), 14, 12,
     "", "!create_fire!", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	 {
     "resist scrying", {10, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_resist_scrying, TAR_CHAR_SELF, POS_RESTING,
     &gsn_resist_scrying, SLOT (623), 25, 12,
     "", "!resist scrying!", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	 
	 {
     "astral spell", {30, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_astral_spell, TAR_CHAR_SELF, POS_RESTING,
     &gsn_astral_spell, SLOT (623), 120, 18,
     "", "!astral spell!", "", TRUE, NULL, STAT_INT, OFFICER, 7, TIME_ONE_DAY},
	 
	  {
     "animate object", {16, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_animate_object, TAR_CHAR_SELF, POS_RESTING,
     &gsn_animate_object, SLOT (623), 75, 18,
     "", "!animate object!", "", TRUE, NULL, STAT_INT, OFFICER, 6, TIME_ONE_DAY},
	 
	  {
     "banshee wail", {11, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_banshee_wail, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_banshee_wail, SLOT (623), 30, 12,
     "banshee wail", "!banshee wail!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
	  {
     "copy", {1, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_copy, TAR_CHAR_SELF, POS_RESTING,
     &gsn_copy, SLOT (623), 3, 6,
     "", "!copy!", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},
	 
	  {
     "chromatic orb", {19, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_chromatic_orb, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_chromatic_orb, SLOT (623), 30, 12,
     "chromatic orb", "!chromatic orb!", "", TRUE, NULL, STAT_INT, MEMBER, 6, 0},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	  {
     "corpse visage", {7, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_corpse_visage, TAR_CHAR_SELF, POS_RESTING,
     &gsn_corpse_visage, SLOT (623), 30, 12,
     "", "You are no longer masking yourself as a corpse.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	  {
     "death recall", {14, 12, 5, 5, 25, 5, 11, 9, 9, 41}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_death_recall, TAR_OBJ_CHAR_OFF, POS_RESTING,
     &gsn_death_recall, SLOT (623), 20, 12,
     "", "!death recall!", "", TRUE, NULL, STAT_INT, APPRENTICE, 4, 0},
	 
	  {
     "negative plane protection", {16, 11, 5, 5, 12, 5, 11, 9, 9, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_negative_plane_protection, TAR_CHAR_SELF, POS_RESTING,
     &gsn_negative_plane_protection, SLOT (623), 12, 12,
     "", "You are no longer protected from negative energies.", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	  {
     "obscure object", {9, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_obscure_object, TAR_OBJ_INV, POS_RESTING,
     &gsn_obscure_object, SLOT (623), 15, 12,
     "", "!obscure object!", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	  {
     "timestop", {30, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_timestop, TAR_CHAR_SELF, POS_RESTING,
     &gsn_timestop, SLOT (623), 100, 18,
     "", "!timestop!", "", TRUE, NULL, STAT_INT, OFFICER, 8, TIME_ONE_WEEK},
	 
	  {
     "planar door", {25, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_planar_door, TAR_CHAR_SELF, POS_RESTING,
     &gsn_planar_door, SLOT (623), 120, 18,
     "", "!planar door!", "", TRUE, NULL, STAT_INT, OFFICER, 8, 0},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	  {
     "free action", {14, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_free_action, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_free_action, SLOT (623), 30, 12,
     "", "!free action!", "", TRUE, NULL, STAT_INT, MEMBER, 5, 0},
	 
	  {
     "cone of cold", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_cone_cold, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_cone_cold, SLOT (623), 40, 12,
     "cone of cold", "!astral spell!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
	  {
     "cone of shadow", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_cone_shadow, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_cone_shadow, SLOT (623), 40, 12,
     "cone of shadow", "!astral spell!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
	  {
     "cone of sound", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_cone_sound, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_cone_sound, SLOT (623), 40, 12,
     "cone of sound", "!astral spell!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
	  {
     "cone of silence", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_cone_silence, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_cone_silence, SLOT (623), 40, 12,
     "cone of silence", "!astral spell!", "", TRUE, NULL, STAT_INT, MEMBER, 6, TIME_TWICE_PER_DAY},
	 
	  {
     "pass without trace", {10, 25, 5, 5, 10, 15, 11, 9, 9, 41}, {1, 0, 0, 0, 1, 1, 0, 0, 0, 0},
     spell_pass_without_trace, TAR_CHAR_SELF, POS_RESTING,
     &gsn_pass_without_trace, SLOT (623), 15, 12,
     "", "You are no longer passing without trace.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	  {
     "searing light", {7, 8, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_searing_light, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_searing_light, SLOT (623), 12, 12,
     "searing light", "!searing light!", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	  {
     "sunburst", {12, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_sunburst, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_sunburst, SLOT (623), 40, 12,
     "sunburst", "!sunburst!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
// Stat improvement spells:

	{
     "strength", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_strength, TAR_CHAR_SELF, POS_RESTING,
     &gsn_strength, SLOT (623), 60, 12,
     "", "!strength!", "", FALSE, NULL, STAT_INT, MASTER, 9, TIME_ONE_MONTH},
	
	{
     "intelligence", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_intelligence, TAR_CHAR_SELF, POS_RESTING,
     &gsn_intelligence, SLOT (623), 60, 12,
     "", "!intelligence!", "", FALSE, NULL, STAT_INT, MASTER, 9, TIME_ONE_MONTH},
	 
	 {//175
     "wisdom", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_wisdom, TAR_CHAR_SELF, POS_RESTING,
     &gsn_wisdom, SLOT (623), 60, 12,
     "", "!wisdom!", "", FALSE, NULL, STAT_INT, MASTER, 9, TIME_ONE_MONTH},
	 
	 {
     "dexterity", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_dexterity, TAR_CHAR_SELF, POS_RESTING,
     &gsn_dexterity, SLOT (623), 60, 12,
     "", "!dexterity!", "", FALSE, NULL, STAT_INT, MASTER, 9, TIME_ONE_MONTH},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	 {
     "constitution", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_constitution, TAR_CHAR_SELF, POS_RESTING,
     &gsn_constitution, SLOT (623), 60, 12,
     "", "!constitution!", "", FALSE, NULL, STAT_INT, MASTER, 9, TIME_ONE_MONTH},
	 
	 {
     "charisma", {15, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_charisma, TAR_CHAR_SELF, POS_RESTING,
     &gsn_charisma, SLOT (623), 60, 12,
     "", "!charisma!", "", FALSE, NULL, STAT_INT, MASTER, 9, TIME_ONE_MONTH},
	
// Imm Buffs:

	{
     "wrath of the ancients", {50, 5, 5, 5, 15, 5, 9, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_wrath_ancient, TAR_CHAR_DEFENSIVE, POS_STANDING,
	 &gsn_wrath_ancient, SLOT (39), 20, 12,
     "", "You feel less power coursing your veins.", "", FALSE, NULL, STAT_INT, MASTER, 9, 0},
	 
//Paladin spells

	{
     "lay hands", {5, 25, 5, 5, 32, 5, 1, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
     spell_lay_hands, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_lay_hands, SLOT (622), 45, 12,
     "lay hands", "!Lay Hands!", "", FALSE, NULL, STAT_INT, INITIATE, 1, TIME_ONE_DAY},
	 
	{
     "aid", {5, 25, 5, 5, 32, 5, 8, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
     spell_aid, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_aid, SLOT (622), 12, 12,
     "aid", "You're constitution returns to normal.", "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0},
	 
	{
     "retribution", {25, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
     spell_retribution, TAR_CHAR_SELF, POS_RESTING,
     &gsn_retribution, SLOT (623), 30, 12,
     "retribution", "You are no longer inflicting retribution.", "", FALSE, NULL, STAT_INT, MEMBER, 4, 0},	 

	 {
     "holy forge", {25, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
     spell_holy_forge, TAR_OBJ_INV, POS_RESTING,
     &gsn_holy_forge, SLOT (623), 150, 24,
     "holy forge", "!holy forge!", "", FALSE, NULL, STAT_INT, MEMBER, 5, TIME_ONE_DAY},	 
	 
	 {
     "flaming forge", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_flaming_forge, TAR_OBJ_INV, POS_RESTING,
     &gsn_flaming_forge, SLOT (623), 150, 24,
     "flaming forge", "!flaming forge!", "", FALSE, NULL, STAT_INT, MEMBER, 5, TIME_ONE_DAY},	 
	 
	 {
     "vampiric forge", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_vampiric_forge, TAR_OBJ_INV, POS_RESTING,
     &gsn_vampiric_forge, SLOT (623), 100, 24,
     "vampiric forge", "!vampiric forge!", "", FALSE, NULL, STAT_INT, MEMBER, 5, TIME_ONE_DAY},	 
	 
	 {
     "lightning forge", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_lightning_forge, TAR_OBJ_INV, POS_RESTING,
     &gsn_lightning_forge, SLOT (623), 150, 24,
     "lightning forge", "!lightning forge!", "", FALSE, NULL, STAT_INT, MEMBER, 5, TIME_ONE_DAY},	 
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion 
	 {
     "frost forge", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_frost_forge, TAR_OBJ_INV, POS_RESTING,
     &gsn_frost_forge, SLOT (623), 150, 24,
     "frost forge", "!frost forge!", "", FALSE, NULL, STAT_INT, MEMBER, 5, TIME_ONE_DAY},	 
	 
	 
	 {
     "antimagic shell", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_antimagic_shell, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_antimagic_shell, SLOT (623), 80, 12,
     "antimagic shell", "!antimagic shell!", "", TRUE, NULL, STAT_INT, MEMBER, 8, TIME_TWICE_PER_DAY},
	 
	  {
     "earth ward", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_earth_ward, TAR_IGNORE, POS_RESTING,
     &gsn_earth_ward, SLOT (623), 45, 12,
     "earth ward", "!earth ward!", "", TRUE, NULL, STAT_WIS, MEMBER, 4, TIME_THREE_PER_DAY},
	 
	  {
     "dimensional mine", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_dimensional_mine, TAR_OBJ_INV, POS_RESTING,
     &gsn_dimensional_mine, SLOT (623), 52, 12,
     "dimensional_mine", "!dimensional mine!", "", TRUE, NULL, STAT_INT, MEMBER, 8, (TIME_THREE_PER_DAY / 2)},
	 
	  {
     "shocking web", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_shocking_web, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_shocking_web, SLOT (623), 37, 12,
     "shocking web", "!shocking web!", "", TRUE, NULL, STAT_INT, MEMBER, 6, TIME_ONE_MINUTE},
	 
	  {
     "geyser", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_geyser, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_geyser, SLOT (623), 25, 12,
     "geyser", "!geyser!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_TEN_SECONDS},
	 
	 {
     "shocking runes", {14, 25, 5, 5, 25, 5, 1, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_shocking_runes, TAR_OBJ_INV, POS_RESTING,
     &gsn_shocking_runes, SLOT (623), 35, 18,
     "shocking runes", "!shocking runes!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_ONE_MINUTE},
     
	 
	  {
     "animate weapon", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_animate_weapon, TAR_OBJ_INV, POS_RESTING,
     &gsn_animate_weapon, SLOT (623), 75, 18,
     "animate weapon", "!animate weapon!", "", TRUE, NULL, STAT_INT, MEMBER, 6, TIME_TWICE_PER_DAY},
	  //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	  {
     "hardstaff", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_hardstaff, TAR_OBJ_CHAR_DEF, POS_RESTING,
     &gsn_hardstaff, SLOT (623), 25, 24,
     "hardstaff", "!hardstaff!", "", TRUE, NULL, STAT_INT, MEMBER, 4, TIME_ONE_WEEK},
	 
	  {
     "finger of death", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_finger_death, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_finger_death, SLOT (623), 120, 12,
     "finger of death", "!finger of death!", "", TRUE, NULL, STAT_INT, MEMBER, 8, TIME_THREE_PER_DAY},
	 
	  {
     "remove paralysis", {15, 25, 5, 5, 25, 5, 11, 9, 9, 41}, {2, 1, 0, 0, 1, 0, 1, 0, 0, 0},
     spell_remove_paralysis, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_remove_paralysis, SLOT (623), 30, 12,
     "remove paralysis", "!remove paralysis!", "", TRUE, NULL, STAT_WIS, MEMBER, 4, 0},
	 
	 /////////////
	 
	 
	  {
     "alarm", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_alarm, TAR_OBJ_INV, POS_RESTING,
     &gsn_alarm, SLOT (0), 15, 12,
     "alarm", "!alarm!", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	  {
     "animate dead", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_animate_dead, TAR_OBJ_INV, POS_RESTING,
     &gsn_animate_dead, SLOT (0), 60, 18,
     "animate dead", "!animate dead!", "", TRUE, NULL, STAT_INT, MEMBER, 3, TIME_TEN_SECONDS},
	 
	  {
     "raise dead", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_raise_dead, TAR_IGNORE, POS_RESTING,
     &gsn_raise_dead, SLOT (0), 90, 18,
     "raise dead", "!raise dead!", "", TRUE, NULL, STAT_INT, MEMBER, 7, TIME_THREE_PER_DAY},
	 
	  {
     "blasphemy", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_blasphemy, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_blasphemy, SLOT (0), 32, 12,
     "blasphemy", "!blasphemy!", "", TRUE, NULL, STAT_INT, MEMBER, 3, 0},
	 
	  {
     "wither", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_wither, TAR_IGNORE, POS_RESTING,
     &gsn_wither, SLOT (0), 20, 12,
     "wither", "!wither!", "", TRUE, NULL, STAT_INT, MEMBER, 4, 0},
	 
	  //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	  {
     "deafness", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_deafness, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_deafness, SLOT (0), 22, 12,
     "deafness", "You can hear again.", "", TRUE, NULL, STAT_INT, INITIATE, 4, TIME_TEN_SECONDS},
	 
	  {
     "disenchant", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_disenchant, TAR_OBJ_INV, POS_RESTING,
     &gsn_disenchant, SLOT (0), 48, 12,
     "disenchant", "!disenchant!", "", TRUE, NULL, STAT_INT, INITIATE, 5, TIME_TEN_SECONDS},
	 
	  {
     "command undead", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_command_undead, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_command_undead, SLOT (0), 70, 12,
     "command undead", "!command_undead!", "", TRUE, NULL, STAT_INT, INITIATE, 6, TIME_THREE_PER_DAY},
	 
	  {
     "commune nature", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_commune_nature, TAR_IGNORE, POS_RESTING,
     &gsn_commune_nature, SLOT (0), 42, 18,
     "commune nature", "!commune nature!", "", TRUE, NULL, STAT_INT, OFFICER, 7, TIME_THREE_PER_DAY},
	 
	  {
     "darkness", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_darkness, TAR_IGNORE, POS_RESTING,
     &gsn_darkness, SLOT (0), 17, 12,
     "darkness", "!darkness!", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	  {
     "erase", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_erase, TAR_OBJ_INV, POS_RESTING,
     &gsn_erase, SLOT (0), 7, 12,
     "erase", "!erase!", "", TRUE, NULL, STAT_INT, INITIATE, 0, 0},
	 
	  {
     "rehydrate", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_rehydrate, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_rehydrate, SLOT (0), 16, 12,
     "rehydrate", "!rehydrate!", "", TRUE, NULL, STAT_INT, INITIATE, 1, TIME_THREE_PER_DAY},
	 
	  {
     "frostbite", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_frostbite, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_frostbite, SLOT (0), 19, 12,
     "frostbite", "!frostbite!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_TEN_SECONDS},
	 
	  {
     "monsoon", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_monsoon, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_monsoon, SLOT (0), 25, 12,
     "monsoon", "!monsoon!", "", TRUE, NULL, STAT_INT, APPRENTICE, 4, TIME_FIVE_SECONDS},
	 
	  {
     "frostbolt", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_frostbolt, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_frostbolt, SLOT (0), 27, 12,
     "frostbolt", "!frostbolt!", "", TRUE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS},
	 
	  {
     "transfer life", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_transfer_life, TAR_CHAR_SELF, POS_RESTING,
     &gsn_transfer_life, SLOT (0), 2, 12,
     "transfer life", "!transfer life!", "", TRUE, NULL, STAT_INT, MEMBER, 6, TIME_TEN_SECONDS},
	 
	  {
     "comprehend languages", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_comprehend_languages, TAR_CHAR_SELF, POS_RESTING,
     &gsn_comprehend_languages, SLOT (0), 21, 12,
     "comprehend languages", "!comprehend_languages!", "", TRUE, NULL, STAT_INT, APPRENTICE, 1, 0},
	 
	  {
     "dimensional anchor", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_dimensional_anchor, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_dimensional_anchor, SLOT (0), 65, 12,
     "dimensional anchor", "!dimensional_anchor!", "", TRUE, NULL, STAT_INT, OFFICER, 8, 0},
	 
	  {
     "elemental protection", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
     spell_elemental_protection, TAR_CHAR_SELF, POS_RESTING,
     &gsn_elemental_protection, SLOT (0), 26, 12,
     "elemental protection", "You no longer feel protected from the elements.", "", TRUE, NULL, STAT_INT, APPRENTICE, 2, 0},
	 
	  {
     "feather fall", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_feather_fall, TAR_CHAR_DEFENSIVE, POS_RESTING,
     &gsn_feather_fall, SLOT (0), 12, 12,
     "feather fall", "You feel in tune with gravity once more.", "", TRUE, NULL, STAT_INT, INITIATE, 1, 0},
	 
	  {
     "bleeding touch", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_bleeding_touch, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_bleeding_touch, SLOT (0), 28, 12,
     "bleeding touch", "!bleeding_touch!", "", TRUE, NULL, STAT_INT, APPRENTICE, 4, TIME_TEN_SECONDS},
	 
	  {
     "spectral hand", {41, 41, 41, 41, 41, 41, 41, 41, 41, 41}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_spectral_hand, TAR_CHAR_OFFENSIVE, POS_RESTING,
     &gsn_spectral_hand, SLOT (0), 17, 12,
     "spectral hand", "!spectral_hand!", "", TRUE, NULL, STAT_INT, APPRENTICE, 3, TIME_FIVE_SECONDS},	 
     
	
//Bard spells


 {
        "vocal shield",     {41, 41, 41, 41, 41, 41, 41, 1, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_vocal_shield,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_vocal_shield,      SLOT(0),        15,     12,
        "",                     "The vocal barrier scrambles into random noise.",       "", FALSE, NULL, STAT_INT, INITIATE, 1, 0
    },
    {
        "adagio",			{41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_adagio,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_adagio,            SLOT(0),        25,     12,
        "",                     "You regain the original tempo.",       "", FALSE, NULL, STAT_INT, MEMBER, 4, 0
   },
   {
        "crescendo",		{41, 41, 41, 41, 41, 41, 41, 15, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_crescendo,        TAR_CHAR_SELF,          POS_STANDING,
        &gsn_crescendo,         SLOT(0),        19,     12,
        "",                     "The vocal crescendo fades into background noise.",     "", FALSE, NULL, STAT_INT, MEMBER, 4, 0
   },
   {
        "audible intoxication", {41, 41, 41, 41, 41, 41, 41, 5, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_audible_intoxication,     TAR_CHAR_SELF,  POS_STANDING,
        &gsn_audible_intoxication,      SLOT(0),        40,     18,
        "audible intoxication", "",                                     "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
   {
        "nourishing ballad",    {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_nourishing_ballad,TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        &gsn_nourishing_ballad, SLOT(0),        12,     12,
        "",                     "!Nourishing Ballad!",                  "", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
   },
   {
        "motivation",           {41, 41, 41, 41, 41, 41, 41, 15, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_motivation,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_motivation,        SLOT(0),        35,     24,
        "",                     "You feel less inspired.",              "", FALSE, NULL, STAT_INT, MEMBER, 4, 0
   },
   {
        "vitalizing verse",     {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_vitalizing_verse, TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_vitalizing_verse,  SLOT(0),        50,     24,
        "",                     "!Vitalizing Verse!",                   "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   {
        "ballad of the banshee",{41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_banshee_ballad,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_banshee_ballad,    SLOT(0),        35,     12,
        "ballad of the banshee","!Banshee Ballad!",                     "", FALSE, NULL, STAT_INT, MEMBER, 5, 0
   },
   {
        "hypnotic verse",       {41, 41, 41, 41, 41, 41, 41, 20, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_hypnotic_verse,   TAR_CHAR_OFFENSIVE,     POS_STANDING,
        &gsn_hypnotic_verse,    SLOT(0),        30,     18,
        "",                     "!Hypnotic Verse!",                     "", FALSE, NULL, STAT_INT, OFFICER, 6, 0
   },
   {
        "insidious chord",      {41, 41, 41, 41, 41, 41, 41, 1, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_insidious_chord,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_insidious_chord,   SLOT(0),        25,     12,
        "insidious chord",      "!Insidious Chord!",                    "", FALSE, NULL, STAT_INT, INITIATE, 2, 0
   },
   {
        "calming verse",        {41, 41, 41, 41, 41, 41, 41, 25, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_calming_verse,    TAR_IGNORE,             POS_STANDING,
        &gsn_calming_verse,     SLOT(0),        50,     18,
        "",                     "!Calming Verse!",                      "", FALSE, NULL, STAT_INT, OFFICER, 7, 0
   },
   {
        "diminishing scale",    {41, 41, 41, 41, 41, 41, 41, 30, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_diminishing_scale,TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_diminishing_scale, SLOT(0),        300,    24,
        "",                     "!Diminishing Scale!",                  "", FALSE, NULL, STAT_INT, OFFICER, 9, 0
   },
   {
        "aggressive staccato",  {41, 41, 41, 41, 41, 41, 41, 30, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_aggressive_staccato,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_aggressive_staccato,       SLOT(0),        300,    24,
        "",                     "!Aggressive Staccato!",                "", FALSE, NULL, STAT_INT, OFFICER, 9, 0
   },
   {
        "assisted coda",        {41, 41, 41, 41, 41, 41, 41, 30, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_assisted_coda,    TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        &gsn_assisted_coda,     SLOT(0),        300,    24,
        "",                     "!Assisted Coda!",                      "", FALSE, NULL, STAT_INT, OFFICER, 9, 0
   },
   {
        "dynamic diapason",     {41, 41, 41, 41, 41, 41, 41, 30, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_dynamic_diapason, TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        &gsn_dynamic_diapason,  SLOT(0),        200,    24,
        "",                     "!Dynamic Diapason!",                   "", FALSE, NULL, STAT_INT, OFFICER, 9, 0
   },
   {
        "tribute to gnomish ingenuity",{41, 41, 41, 41, 41, 41, 41, 35, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_gnomish_tribute,  TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        &gsn_gnomish_tribute,   SLOT(0),        200,    24,
        "",                     "!tribute to gnomish ingenuity!",       "", FALSE, NULL, STAT_INT, OFFICER, 8, 0
   },
   {//200
        "quicken tempo",        {41, 41, 41, 41, 41, 41, 41, 5, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_quicken_tempo,    TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        &gsn_quicken_tempo,     SLOT(0),        24,     18,
        "",                     "You begin to feel off beat from the original tempo...",        "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   {
        "orbitting rhythm and lead",   {41, 41, 41, 41, 41, 41, 41, 15, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_orbitting_rhythm_lead,    TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_orbitting_rhythm_lead,     SLOT(0),        75,     12,
        "",                     "The orbitting rhythm and lead falls off track and disperses into random notes.",       "", FALSE, NULL, STAT_INT, MEMBER, 5, 0
   },
   {
        "suggestive verse",     {41, 41, 41, 41, 41, 41, 41, 20, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_suggestive_verse, TAR_CHAR_OFFENSIVE,     POS_STANDING,
        &gsn_suggestive_verse,  SLOT(0),        20,     12,
        "",                     "You no longer feel open to suggestion through music.", "", FALSE, NULL, STAT_INT, MEMBER, 5, 0
   },
   {
        "melody of masked intent",     {41, 41, 41, 41, 41, 41, 41, 1, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_melody_masked_intent,     TAR_CHAR_SELF,  POS_STANDING,
        &gsn_melody_masked_intent,      SLOT(0),        15,     12,
        "",                     "You no longer feel safe from scrying.",        "", FALSE, NULL, STAT_INT, INITIATE, 1, 0
   },
   {
        "forgetful lullaby",   {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_forgetful_lullaby,TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_forgetful_lullaby, SLOT(0),        60,     24,
        "",                     "Your memory returns to normal.",       "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   {
        "hymn to the fallen", {41, 41, 41, 41, 41, 41, 41, 15, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_hymn_fallen,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_hymn_fallen,       SLOT(0),        50,     24,
        "",                     "You no longer mourn the passing.",     "", FALSE, NULL, STAT_INT, MEMBER, 5, 0
   },
   {
        "contradicting melody", {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_contradicting_melody,     TAR_CHAR_OFFENSIVE,     POS_STANDING,
        &gsn_contradicting_melody,      SLOT(0),        50,     18,
        "",                     "Contradicting Melody!",                "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
	},
   {
        "cacophonic shield",   {41, 41, 41, 41, 41, 41, 41, 5, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_cacophonic_shield,TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_cacophonic_shield, SLOT(0),        25,     12,
        "",                     "The chaotic noise slowly fades.",      "", FALSE, NULL, STAT_INT, INITIATE, 2, 0
   },
   {
        "dissonant chord",     {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_dissonant_chord,  TAR_CHAR_SELF,          POS_STANDING,
        &gsn_dissonant_chord,   SLOT(0),        22,     12,
        
        "",                     "The dissonance fades away into odd feedback.", "", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
   },
   {
        "dirge of dischord",   {41, 41, 41, 41, 41, 41, 41, 15, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_dirge_dischord,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_dirge_dischord,    SLOT(0),        14,     12,
        
        "",                     "",                                     "", FALSE, NULL, STAT_INT, MEMBER, 5, TIME_FIVE_SECONDS
   },
   {
        "harmonic chorus",    {41, 41, 41, 41, 41, 41, 41, 20, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_harmonic_chorus,  TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_harmonic_chorus,   SLOT(0),        30,     18,
        
        "",                     "You no longer feel enlightened.",      "", FALSE, NULL, STAT_INT, MEMBER, 7, 0
   },
   //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
   {
        "improvisation",       {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_improvisation,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_improvisation,     SLOT(0),        25,     12,
        
        "improvisation",        "",                                     "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   {
        "repulsive din",       {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_repulsive_din,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_repulsive_din,     SLOT(0),        25,     12,
        
        "repulsive din",        "You feel a bit more attractive once more.",    "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   {
        "infernal threnody",    {41, 41, 41, 41, 41, 41, 41, 15, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_infernal_threnody,TAR_CHAR_SELF,          POS_STANDING,
        &gsn_infernal_threnody, SLOT(0),        35,     12,
        
        "",                     "",                                     "", FALSE, NULL, STAT_INT, MEMBER, 5
   },
   {
        "listening jewel",     {41, 41, 41, 41, 41, 41, 41, 25, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_listening_jewel,  TAR_OBJ_INV,            POS_STANDING,
        &gsn_listening_jewel,   SLOT(0),        100,    18,
        
        "",                     "",                                     "", FALSE, NULL, STAT_INT, OFFICER, 7, 0
   },
   {
        "wail of doom",       {41, 41, 41, 41, 41, 41, 41, 30, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_wail_doom,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_wail_doom,         SLOT(0),        50,     12,
        
        "wail of doom",         "",                                     "", FALSE, NULL, STAT_INT, OFFICER, 7, TIME_FIVE_SECONDS
	},
                  
   {
        "resonating echo",     {41, 41, 41, 41, 41, 41, 41, 10, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_resonating_echo,  TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
        &gsn_resonating_echo,   SLOT(0),        26,     12,
        
        "",                     "",                                     "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
   },
   {
        "selective hearing",  {41, 41, 41, 41, 41, 41, 41, 5, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_selective_hearing,TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_selective_hearing, SLOT(0),        15,     12,
        
        "",                     "You seem to be able to pay more attention again.",     "", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
    },
    {//218
        "sacred euphony",     {41, 41, 41, 41, 41, 41, 41, 20, 41, 41}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        spell_sacred_euphony,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
        &gsn_sacred_euphony,    SLOT(0),        15,     12,
        "",                     "",                                     "", 
		FALSE, NULL, STAT_INT, MEMBER, 5, 0
	},
	 
	 
	 
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	 
	// //   Psionic-only devotions
    // {
	// "read object",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_read_object,	TAR_OBJ_INV,		POS_RESTING,
	// &gsn_read_object,	SLOT(0),	18,	12,
	// "",			"!OREAD!",				"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    // {
	// "absorb disease",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_absorb_disease,	TAR_CHAR_DEFENSIVE,	POS_RESTING,
	// &gsn_absorb_disease,	SLOT(300),	25,	12,
	// "",			"!Absorb disease!",			"", FALSE, NULL, STAT_INT, MEMBER, 5
    // },
    {
	"adrenaline control",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_adrenaline_control,TAR_CHAR_SELF,		POS_RESTING,
	&gsn_adrenaline_control,SLOT(301),	35,	12,
	"",			"Your natural high fades.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"agitation",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_agitation,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_agitation,		SLOT(302),	10,	12,
	"agitation",		"!agitation!",				"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"animate shadow",{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_animate_shadow,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_animate_shadow,	SLOT(303),	40,	12,
	"",			"!animate shadow!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "appraise",		{ 0, 0, 0, 0,17, 0, 0, 0, 0, 0},
						// { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},//
	// spell_appraise,		TAR_IGNORE,		POS_RESTING,
	// &gsn_appraise,		SLOT(304),	10,	12,
	// "",			"!appraise!",				"", FALSE, NULL, STAT_INT, MEMBER, 5
    // },
    {
	"aura sight",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_aura_sight,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_aura_sight,	SLOT(305),	12,	12,
	"",			"Your vision returns to normal.",	"", FALSE, NULL, STAT_INT, INITIATE, 1, 0
    },
    {
	"aversion",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_aversion,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_aversion,		SLOT(306),	10,	12,
	"",			"!aversion!",				"", FALSE, NULL, STAT_INT, MEMBER, 3, 0
    },
    {
	"awe",			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_awe,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_awe,		SLOT(307),	20,	12,
	"",			"Wait, they aren't so big!",		"", FALSE, NULL, STAT_INT, MEMBER, 2, 0
    },
    {
	"ballistic attack",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_ballistic_attack,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_ballistic_attack,	SLOT(308),	20,	12,
	"projected force",	"!ballistic attack!",			"", FALSE, NULL, STAT_INT, MEMBER, 3, 0
    },
    {
	"banishment",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_banishment,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_banishment,	SLOT(309),	35,	12,
	"",			"You have served your time.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"biofeedback",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	 spell_biofeedback,	TAR_CHAR_SELF,		POS_RESTING,
	 &gsn_biofeedback,	SLOT(310),	80,	12,
	 "",			"Your bloodflow resumes normally.",	"", FALSE, NULL, STAT_INT, MEMBER, 4, 0
	},
    {
	"body equilibrium",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_body_equilibrium,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_body_equilibrium,	SLOT(311),	20,	12,
	"",			"You feel much heavier.",		"", FALSE, NULL, STAT_INT, INITIATE, 1, 0
    },
    {
	"body weaponry",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_body_weaponry,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_body_weaponry,	SLOT(312),	25,	12,
	"",			"Your hands regain their normal texture.",	"", FALSE, NULL, STAT_INT, MEMBER, 3, 0
    },
    {
	"chemical simulation",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_chemical_simulation,	TAR_CHAR_SELF,	POS_RESTING,
	&gsn_chemical_simulation,	SLOT(313),	40,	12,
	"",			"Your hands become less corrosive.",	"", FALSE, NULL, STAT_INT, MEMBER, 3, 0
    },
    {
	"clairaudience",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_clairaudience,	TAR_IGNORE,		POS_RESTING,
	&gsn_clairaudience,	SLOT(314),	35,	36,
	"",			"Your hearing is less acute.",		"", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
    },
    {
	"clairvoyance",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_clairvoyance,	TAR_IGNORE,		POS_RESTING,
	&gsn_clairvoyance,	SLOT(315),	15,	24,
	"",			"!clairvoyance!",			"", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
    },
    {
	"clear thoughts",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_cancellation,	TAR_CHAR_DEFENSIVE,	POS_RESTING,
	&gsn_clear_thoughts,	SLOT(11),	20,	12,
	"",			"!cancellation!",			"", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
    },
    {
	"fighting mind",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_fighting_mind,	TAR_CHAR_DEFENSIVE,	POS_RESTING,
	&gsn_fighting_mind,	SLOT(316),	15,	12,
	"",			"Your mind is less keen in battle.",	"", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
    },
    {
	"contact",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_contact,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_contact,		SLOT(317),	10,	12,
	"",			"Your mind feels alone.",		"", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
    },
    {
	"control flame",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_control_flame,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_control_flame,	SLOT(318),	20,	12,
	"controlled flame",	"!Control Flame!",			"", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
    },
    {
	"daydream",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_daydream,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_daydream,		SLOT(319),	20,	12,
	"",			"You come back to attention.",		"", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
    },
    {
	"death field",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_death_field,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_death_field,	SLOT(320),	200,	12,
	"field of death",	"!Death Field!",			"", FALSE, NULL, STAT_INT, OFFICER, 7, TIME_THREE_PER_DAY
    },
    {
	"detonate",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_detonate,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_detonate,		SLOT(321),	20,	12,
	"detonation",		"!Detonation!",				"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "dimensional door",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_portal,		TAR_IGNORE,		POS_RESTING,
	// &gsn_dimensional_door,	SLOT(322),	100,	12,
	// "",			"!Domination!",				"", FALSE, NULL, STAT_INT, MEMBER, 5
    // },
    // {
	// "displacement",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_displacement,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	// &gsn_displacement,	SLOT(0),	30,	12,
	// "",			"!displacement!",			"", FALSE, NULL, STAT_INT, MEMBER, 5
    // },
    {
	"domination",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_domination,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_domination,	SLOT(323),	15,	12,
	"",			"!Domination!",				"", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0 
    },
    {
	"ectoplasmic form",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_ectoplasmic_form,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_ectoplasmic_form,	SLOT(324),	15,	12,
	"",			"You become more solid.",		"", FALSE, NULL, STAT_INT, APPRENTICE, 3, 0
    },
    {
	"ego whip",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_ego_whip,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_ego_whip,		SLOT(325),	20,	12,
	"",			"Your hurt ego feels better.",		"", FALSE, NULL, STAT_INT, MEMBER, 4, 0
    },
    {
	"ejection",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_ejection,		TAR_CHAR_SELF,		POS_RESTING,
	&gsn_ejection,		SLOT(326),	25,	12,
	"",			"Your mind is pure.",			"", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
    },
    {
	"energy containment",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_energy_containment,TAR_CHAR_SELF,		POS_RESTING,
	&gsn_energy_containment,SLOT(327),	25,	12,
	"",			"You feel more vulnerable to energies.",	"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"enhanced strength",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_enhanced_strength,TAR_CHAR_SELF,		POS_RESTING,
	&gsn_enhanced_strength,	SLOT(328),	20,	12,
	"",			"Your strength diminishes.",		"", FALSE, NULL, STAT_INT, INITIATE, 1, 0
    },
    // {
	// "epilepsy",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_epilepsy,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	// &gsn_epilepsy,		SLOT(0),	20,	12,
	// "mental seizure",	"The seizures stop.",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    {
	"expansion",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_expansion,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_expansion,		SLOT(3),	25,	12,
	"",			"Your body size returns to normal.",	"", FALSE, NULL, STAT_INT, APPRENTICE, 2, 0
    },
    {
	"flesh armor",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_flesh_armor,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_flesh_armor,	SLOT(330),	18,	12,
	"",			"Your skin softens.",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"immovability",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_immovability,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_immovability,	SLOT(331),	20,	12,
	"",			"Your feet regain mobility.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"inertial barrier",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_inertial_barrier,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_inertial_barrier,	SLOT(332),	10,	12,
	"",			"Your inertial barrier fades.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"intellect fortress",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_intellect_fortress,	TAR_CHAR_SELF,	POS_RESTING,
	&gsn_intellect_fortress,	SLOT(333),	25,	12,
	"",			"Your fortress fades.",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"heightened senses",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_heightened_senses,TAR_CHAR_SELF,		POS_RESTING,
	&gsn_heightened_senses,	SLOT(334),	15,	12,
	"",			"Your eye for detail dulls.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"know location",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_know_location,	TAR_IGNORE,		POS_RESTING,
	&gsn_know_location,	SLOT(335),	10,	12,
	"",			"!Know location!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"lend health",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_lend_health,	TAR_CHAR_DEFENSIVE,	POS_RESTING,
	&gsn_lend_health,	SLOT(336),	0,	12,
	"",			"!lend health!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "lethargy",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_lethargy,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	// &gsn_lethargy,		SLOT(0),	20,	12,
	// "",			"Your mental faculties return.",	"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    {
	"levitate",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_levitate,		TAR_CHAR_SELF,		POS_RESTING,
	&gsn_levitate,		SLOT(337),	10,	12,
	"",			"You descend to the ground.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"mental barrier",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_mental_barrier,	TAR_CHAR_DEFENSIVE,	POS_RESTING,
	&gsn_mental_barrier,	SLOT(338),	20,	12,
	"",			"Your mind feels more vulnerable.",	"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"metabolic reconfiguration",
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_metabolic_reconfiguration,TAR_CHAR_SELF, POS_RESTING,
	&gsn_metabolic_reconfiguration,	SLOT(0),	40,	12,
	"",			"!metabolic reconfiguration!",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "migraine",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_migraine,		TAR_IGNORE,		POS_FIGHTING,
	// &gsn_migraine,		SLOT(0),	30,	12,
	// "forced migraine",	"!migraine!",				"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    {
	"mind over body",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_mind_over_body,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_mind_over_body,	SLOT(344),	15,	12,
	"",			"!mind over body!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"mind thrust",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_mind_thrust,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_mind_thrust,	SLOT(339),	20,	12,
	"mental thrust",	"Your mind is yours once more.",	"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"mindwipe",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_mindwipe,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_mindwipe,		SLOT(345),	20,	12,
	"mind wipe",		"!mind wipe!",				"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "neural shock",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_neural_shock,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	// &gsn_neural_shock,	SLOT(0),	10,	12,
	// "neural shock",		"!neural shock!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    // {
	// "planar gate",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_gate,		TAR_IGNORE,		POS_RESTING,
	// &gsn_planar_gate,	SLOT(340),	65,	12,
	// "",			"!planar travel!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    {
	"probe",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_probe,		TAR_CHAR_DEFENSIVE,	POS_RESTING,
	&gsn_probe,		SLOT(346),	20,	12,
	"",			"Your probe ends.",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    {
	"psychic drain",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_psychic_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_psychic_drain,	SLOT(341),	15,	12,
	"",			"You don't feel so weak.",		"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "psi eruption",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_psi_eruption,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	// &gsn_psi_eruption,	SLOT(0),	15,	12,
	// "psi eruption",		"!psi eruption!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
    {
	"reduction",		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	spell_reduction,	TAR_CHAR_SELF,		POS_RESTING,
	&gsn_reduction,		SLOT(342),	25,	12,
	"",			"Your body size returns to normal.",	"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    },
    // {
	// "summon creature",	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
						// { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },//
	// spell_summon,		TAR_IGNORE,		POS_RESTING,
	// &gsn_summon_creature,	SLOT(343),	50,	12,
	// "",			"!summon creature!",			"", FALSE, NULL, STAT_INT, MEMBER, 5, 0
    // },
	 
	 
	 
	 
/* combat and weapons skills */

//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {//219
     "axe", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 1, 2, 1, 1, 2, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_axe, SLOT (0), 0, 0,
     "", "!Axe!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},

    {//220
     "dagger", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 3, 1, 1, 1, 1, 0, 1, 0, 1},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dagger, SLOT (0), 0, 0,
     "", "!Dagger!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {
     "flail", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {3, 1, 2, 1, 1, 1, 1, 2, 0, 2},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_flail, SLOT (0), 0, 0,
     "", "!Flail!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},

    {
     "mace", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_mace, SLOT (0), 0, 0,
     "", "!Mace!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},

    {
     "polearm", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 1, 0, 3, 2, 4, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_polearm, SLOT (0), 0, 0,
     "", "!Polearm!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},

    {
     "shield block", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {3, 2, 3, 1, 2, 1, 1, 2, 0, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_shield_block, SLOT (0), 0, 0,
     "", "!Shield!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "spear", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {4, 4, 3, 1, 2, 2, 1, 2, 3, 2},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_spear, SLOT (0), 0, 0,
     "", "!Spear!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},

	{
     "staff", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 2, 1, 1, 2, 1, 2, 2, 1},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_staff, SLOT (0), 0, 0,
     "", "!Staff!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	 
    {
     "short sword", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 1, 1, 2, 1, 1, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_short_sword, SLOT (0), 0, 0,
     "", "!short sword!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	 
	{
     "long sword", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 2, 1, 3, 1, 1, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_long_sword, SLOT (0), 0, 0,
     "", "!long sword!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
	 
	{
     "bow", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 2, 1, 1, 1, 1, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_bow, SLOT (0), 0, 0,
     "", "!bow!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	 
	{
     "crossbow", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 1, 1, 2, 1, 1, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_crossbow, SLOT (0), 0, 0,
     "", "!crossbow!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	 
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "whip", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {2, 2, 2, 1, 2, 1, 0, 2, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_whip, SLOT (0), 0, 0,
     "", "!Whip!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {
     "backstab", {5, 5, 1, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 3, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_backstab, SLOT (0), 0, 24,
     "backstab", "!Backstab!", "", FALSE, &gsn_dagger, STAT_DEX, APPRENTICE, -1, TIME_TEN_SECONDS},

    {
     "bash", {5, 5, 5, 1, 5, 6, 9, 9, 9, 1}, {0, 0, 0, 3, 0, 4, 3, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_bash, SLOT (0), 0, 18,
     "bash", "!Bash!", "", FALSE, NULL, STAT_STR, INITIATE, -1, TIME_FIVE_SECONDS},

	{
     "shield bash", {5, 5, 5, 8, 5, 8, 6, 9, 9, 1}, {0, 0, 0, 1, 0, 2, 1, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_shield_bash, SLOT (0), 0, 18,
     "shield bash", "!Shield Bash!", "", FALSE, NULL, STAT_STR, APPRENTICE, -1, TIME_FIVE_SECONDS},

	{
     "caltrops", {5, 5, 9, 10, 5, 15, 9, 9, 9, 1}, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_caltrops, SLOT (0), 0, 12,
     "caltrops", "!Caltrops!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, TIME_FIVE_SECONDS},
	 
	{
     "throw", {5, 5, 15, 10, 5, 15, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_throw, SLOT (0), 0, 12,
     "throw", "!Throw!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, TIME_TEN_SECONDS},
	 
	 {
     "power grip", {5, 5, 5, 10, 5, 15, 9, 9, 9, 1}, {0, 0, 0, 1, 0, 2, 1, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_power_grip, SLOT (0), 0, 12,
     "power grip", "!Power Grip!", "", FALSE, NULL, STAT_STR, APPRENTICE, -1, 0},
	 
    {
     "berserk", {5, 5, 5, 10, 5, 12, 9, 9, 9, 1}, {0, 0, 0, 3, 0, 3, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_berserk, SLOT (0), 0, 18,
     "", "You feel your pulse slow down.", "", FALSE, NULL, STAT_CON, APPRENTICE, -1, TIME_THREE_PER_DAY},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	{
     "bloodlust", {5, 5, 5, 23, 5, 26, 9, 9, 9, 1}, {0, 0, 0, 3, 0, 4, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_bloodlust, SLOT (0), 0, 18,
     "", "At last... your bloodlust subsides.", "", FALSE, &gsn_berserk, STAT_CON, OFFICER, -1, TIME_TWICE_PER_DAY},
	 
	 {
     "battlecry", {5, 5, 5, 15, 5, 16, 9, 9, 9, 1}, {0, 0, 0, 2, 0, 2, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_battlecry, SLOT (0), 0, 18,
     "battlecry", "You are no longer affected by a battle cry.", "", FALSE, NULL, STAT_STR, MEMBER, -1, TIME_ONE_MINUTE},

    {
     "dirt kicking", {5, 5, 3, 3, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 2, 0, 2, 2, 2, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dirt, SLOT (0), 0, 18,
     "kicked dirt", "You rub the dirt out of your eyes.", "", FALSE, NULL, STAT_DEX, INITIATE, -1, TIME_TEN_SECONDS},

    {
     "disarm", {5, 5, 12, 11, 5, 25, 9, 9, 9, 1}, {0, 0, 3, 1, 0, 2, 2, 3, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_disarm, SLOT (0), 0, 18,
     "", "!Disarm!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, TIME_FIVE_SECONDS},

    {
     "dodge", {14, 12, 1, 5, 12, 10, 7, 5, 3, 1}, {4, 4, 2, 2, 4, 2, 3, 4, 2, 2},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dodge, SLOT (0), 0, 0, 
     "", "!Dodge!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {
     "enhanced damage", {45, 26, 16, 1, 15, 1, 9, 9, 9, 1}, {0, 5, 3, 2, 5, 3, 3, 4, 1, 1},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_enhanced_damage, SLOT (0), 0, 0,
     "", "!Enhanced Damage!", "", FALSE, NULL, STAT_STR, APPRENTICE, -1, 0},	
	
    {
     "envenom", {5, 5, 10, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 3, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_envenom, SLOT (0), 0, 36,
     "", "!Envenom!", "", FALSE, NULL, STAT_INT, APPRENTICE, -1, TIME_TEN_SECONDS},

    {
     "hand to hand", {25, 10, 15, 6, 10, 8, 9, 9, 1, 1}, {3, 3, 3, 2, 3, 3, 0, 0, 1, 1},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_hand_to_hand, SLOT (0), 0, 0,
     "", "!Hand to Hand!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {
     "kick", {5, 12, 14, 8, 30, 10, 9, 9, 9, 1}, {0, 3, 3, 2, 3, 3, 3, 3, 2, 3},
     spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_kick, SLOT (0), 0, 12,
     "kick", "!Kick!", "", FALSE, NULL, STAT_STR, INITIATE, -1, TIME_FIVE_SECONDS},
	 
	 {
     "scribe", {10, 12, 53, 53, 53, 53, 9, 9, 9, 1}, {2, 3, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_scribe, SLOT (0), 0, 24,
     "", "!Scribe!", "", FALSE, NULL, STAT_INT, APPRENTICE, -1, (TIME_ONE_MINUTE * 2)},
	 
	 {
     "lay trap", {5, 12, 14, 8, 30, 10, 9, 9, 9, 1}, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_lay_trap, SLOT (0), 0, 24,
     "", "!Lay Trap!", "", FALSE, NULL, STAT_INT, INITIATE, -1, (TIME_ONE_MINUTE / 2)},
	 
	 {//250
     "remove trap", {15, 5, 5, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_remove_trap, SLOT (0), 0, 18,
     "", "!Remove Trap!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, TIME_ONE_MINUTE},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	 {
     "repair", {15, 5, 5, 5, 5, 5, 9, 9, 9, 1}, {3, 3, 2, 1, 3, 2, 2, 3, 3, 3},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_repair, SLOT (0), 0, 12,
     "", "!Repair!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},
	 
	{
    "forage", {5, 5, 5, 5, 5, 5, 5, 5, 5, 1}, {2, 2, 2, 2, 1, 1, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_forage, SLOT (0), 0, 10,
     "", "!Forage!", "", FALSE, &gsn_herbalism, STAT_INT, APPRENTICE, -1, 0},
	 
	 {
    "plant", {5, 5, 5, 5, 5, 5, 5, 5, 5, 1}, {0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_plant, SLOT (0), 0, 6,
     "", "!plant!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, 0},
	 
	 {
    "mine", {5, 5, 5, 5, 5, 5, 5, 5, 5, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_mine, SLOT (0), 0, 10,
     "", "!Mine!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
	 
	{
    "prospecting", {5, 5, 5, 5, 5, 5, 5, 5, 5, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_prospecting, SLOT (0), 0, 10,
     "", "!prospecting!", "", FALSE, NULL, STAT_WIS, INITIATE, -1, 0},
	 
	{
    "woodcutting", {3, 3, 3, 3, 3, 3, 3, 3, 3, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_woodcutting, SLOT (0), 0, 10,
     "", "!woodcutting!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
	 
	{
    "cooking", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_cooking, SLOT (0), 0, 8,
     "", "!Cooking!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},
	
	{
    "agriculture", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_agriculture, SLOT (0), 0, 8,
     "", "!Agriculture!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},
	
	{
    "herbalism", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {2, 2, 2, 2, 1, 1, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_herbalism, SLOT (0), 0, 8,
     "", "!Forage!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},
	 
	{
    "alchemy", {5, 5, 10, 10, 5, 10, 10, 10, 10, 1}, {1, 1, 2, 2, 1, 2, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_alchemy, SLOT (0), 0, 8,
     "", "!Alchemy!", "", FALSE, NULL, STAT_INT, APPRENTICE, -1, 0},
	 
	 {
    "first aid", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_first_aid, SLOT (0), 0, 8,
     "", "!First Aid!", "", FALSE, NULL, STAT_INT, INITIATE, -1, TIME_ONE_MINUTE},
	 
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "parry", {22, 20, 13, 1, 28, 4, 3, 9, 9, 1}, {4, 3, 2, 1, 3, 2, 2, 3, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_parry, SLOT (0), 0, 0,
     "", "!Parry!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {
     "rescue", {5, 5, 5, 1, 5, 4, 1, 5, 5, 1}, {0, 0, 0, 2, 0, 2, 1, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_rescue, SLOT (0), 0, 12,
     "", "!Rescue!", "", FALSE, NULL, STAT_CON, APPRENTICE, -1, TIME_FIVE_SECONDS},

    {
     "trip", {5, 5, 1, 15, 5, 20, 9, 9, 9, 7}, {0, 0, 2, 2, 0, 3, 0, 3, 2, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_trip, SLOT (0), 0, 18,
     "trip", "!Trip!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, TIME_FIVE_SECONDS},
	/*
    {
     "second attack", {30, 24, 12, 5, 30, 15, 9, 9, 9, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_second_attack, SLOT (0), 0, 0,
     "", "!Second Attack!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},

    {
     "third attack", {5, 5, 24, 12, 15, 15, 9, 9, 9, 1}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_third_attack, SLOT (0), 0, 0,
     "", "!Third Attack!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
	*/
	 {
     "blind fighting", {5, 5, 5, 5, 10, 10, 9, 9, 9, 1}, {0, 0, 3, 2, 0, 3, 3, 4, 2, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "", "!Blind Fighting!", "", FALSE, NULL, STAT_WIS, MEMBER, -1, 0},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	 {
     "counter", {26, 26, 26, 26, 26, 26, 26, 26, 26, 26}, {0, 0, 3, 2, 0, 3, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "counter", "!Counter!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, 0},

	 {
     "riposte", {26, 26, 26, 26, 26, 26, 26, 26, 26, 26}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "riposte", "!Riposte!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, 0},
	 
	 {
     "kidney punch", {5, 5, 8, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "kidney punch", "!Kidney Punch!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, TIME_TEN_SECONDS},
	 
	 {
     "earclap", {5, 5, 6, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "earclap", "!Earclap!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, (TIME_FIVE_SECONDS * 3)},
	 
	 {
     "gouge", {5, 5, 14, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "gouge", "!Gouge!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, (TIME_TEN_SECONDS * 2)},
	 
	{
     "sharpen", {5, 5, 5, 3, 5, 4, 9, 9, 9, 1}, {0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "sharpen", "!Sharpen!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, TIME_TEN_SECONDS},
	 
	{
     "hapkido", {5, 5, 5, 5, 8, 5, 9, 9, 6, 1}, {0, 0, 0, 0, 2, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "hapkido", "!Hapkido!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, (TIME_TEN_SECONDS * 2)},
	 
	{
     "martial arts", {5, 5, 11, 2, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 1, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT (0), 0, 0,
     "martial arts", "!Martial Arts!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, (TIME_TEN_SECONDS * 2)},
	 
/* non-combat skills */

    {
     "fast healing", {15, 9, 16, 6, 20, 7, 9, 9, 9, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_fast_healing, SLOT (0), 0, 0,
     "", "!Fast Healing!", "", FALSE, NULL, STAT_CON, INITIATE, -1, 0},

    {
     "haggle", {7, 18, 1, 14, 25, 14, 9, 9, 9, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_haggle, SLOT (0), 0, 0,
     "", "!Haggle!", "", FALSE, NULL, STAT_CHA, APPRENTICE, -1, 0},
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "hide", {42, 42, 1, 12, 42, 10, 42, 42, 42, 1}, {0, 0, 2, 0, 0, 5, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_hide, SLOT (0), 0, 12,
     "", "!Hide!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {
     "lore", {5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_lore, SLOT (0), 0, 36,
     "", "!Lore!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},

    {
     "meditation", {6, 6, 15, 15, 6, 15, 9, 9, 9, 9}, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_meditation, SLOT (0), 0, 0,
     "", "Meditation", "", FALSE, NULL, STAT_WIS, INITIATE, -1, 0},

    {
     "peek", {8, 21, 1, 14, 40, 10, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_peek, SLOT (0), 0, 0,
     "", "!Peek!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, TIME_FIVE_SECONDS},

    {
     "pick lock", {25, 25, 5, 25, 25, 10, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 2, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_pick_lock, SLOT (0), 0, 12,
     "", "!Pick!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, TIME_TEN_SECONDS},

    {
     "sneak", {42, 42, 4, 10, 10, 12, 42, 42, 9, 1}, {0, 0, 2, 2, 3, 2, 0, 0, 3, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_sneak, SLOT (0), 0, 12,
     "", "You no longer feel stealthy.", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},

    {//275
     "steal", {5, 5, 5, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_steal, SLOT (0), 0, 24,
     "", "!Steal!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, TIME_TEN_SECONDS},

    {
     "scrolls", {1, 1, 9, 9, 7, 1, 6, 4, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_scrolls, SLOT (0), 0, 24,
     "", "!Scrolls!", "", FALSE, NULL, STAT_INT, INITIATE, -1, TIME_FIVE_SECONDS},

    /*{
     "staves", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 0, 0, 0, 1, 0, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_staves, SLOT (0), 0, 12,
     "", "!Staves!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},*/
//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
    {
     "wands", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_wands, SLOT (0), 0, 12,
     "", "!Wands!", "", FALSE, NULL, STAT_INT, INITIATE, -1, TIME_FIVE_SECONDS},

    {
     "recall", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_recall, SLOT (0), 0, 12,
     "", "!Recall!", "", FALSE, NULL, STAT_INT, INITIATE, -1, TIME_THREE_PER_DAY},
	 
	 {
     "seafaring", {5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, {2, 2, 2, 2, 2, 2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT (0), 0, 0,
     "", "!Seafaring!", "", FALSE, NULL, STAT_INT, MEMBER, -1, 0},
	 
	 {
     "swim", {5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_swim, SLOT (0), 0, 0,
     "", "!Swim!", "", FALSE, NULL, STAT_CON, INITIATE, -1, 0},
	 
	 {
     "butcher", {5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, {2, 2, 1, 1, 1, 1, 1, 1, 0, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_butcher, SLOT (0), 0, 0,
     "", "!Butcher!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	 
	 {
     "tailoring", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_tailoring, SLOT (0), 0, 0,
     "", "!tailoring!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	 
	 {
     "blacksmithing", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_blacksmithing, SLOT (0), 0, 0,
     "", "!blacksmithing!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
	 	
	{
     "jewelcrafting", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_jewelcrafting, SLOT (0), 0, 0,
     "", "!jewelcrafting!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},
	//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion	
	{
     "find trap", {5, 5, 5, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_find_trap, SLOT (0), 0, 0,
     "", "!Find Trap!", "", FALSE, NULL, STAT_INT, INITIATE, -1, TIME_TEN_SECONDS},
	 
	 {
     "turn undead", {5, 5, 5, 5, 5, 5, 9, 9, 9, 1}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (0), 0, 12,
     "", "!Turn undead!", "", FALSE, NULL, STAT_WIS, INITIATE, -1, TIME_THREE_PER_DAY},
	 
	 {
     "engage", {5, 5, 5, 5, 5, 5, 5, 5, 5, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 2},
     spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT (0), 0, 6,
     "", "!Engage!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, 0},
	 
	{
     "dual wield", {5, 5, 5, 5, 5, 5, 5, 5, 5, 1}, {0, 0, 2, 2, 0, 2, 0, 0, 0, 0},
     spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_dual_wield, SLOT (0), 0, 6,
     "", "!dual wield!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, 0},
	 
	 {
     "detect secret", {5, 5, 1, 5, 5, 5, 9, 9, 9, 1}, {0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_detect_secret, SLOT (0), 0, 0,
     "", "!detect secret!", "", FALSE, NULL, STAT_INT, INITIATE, -1, 0},
	 
	 {
     "riding", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_riding, SLOT (0), 0, 0,
     "", "!riding!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, 0},
	 
	 {
     "tracking", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_tracking, SLOT (0), 0, 0,
     "", "!tracking!", "", FALSE, NULL, STAT_WIS, INITIATE, -1, 0},
	 	 
		 
	{
     "imbue", {16, 16, 16, 16, 16, 16, 16, 16, 16, 16}, {2, 3, 0, 0, 3, 0, 0, 0, 0, 0},
     spell_null, TAR_OBJ_CHAR_DEF, POS_STANDING,
     &gsn_imbue, SLOT (0), 0, 0,
     "", "!imbue!", "", FALSE, NULL, STAT_INT, APPRENTICE, -1, TIME_THREE_PER_DAY},
	 
	{
     "spellcraft", {11, 7, 5, 5, 7, 40, 9, 9, 9, 1}, {2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_spellcraft, SLOT (0), 0, 0,
     "", "!spellcraft!", "", FALSE, NULL, STAT_INT, APPRENTICE, -1, 0},
	//wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion 
	{
     "inlay", {12, 12, 12, 12, 12, 12, 12, 12, 12, 12}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     spell_null, TAR_OBJ_INV, POS_STANDING,
     &gsn_inlay, SLOT (0), 0, 0,
     "", "!inlay!", "", FALSE, &gsn_jewelcrafting, STAT_DEX, APPRENTICE, -1, 0},
	 
	/* Monk Skills */
	{
     "iron fist", {45, 30, 25, 1, 15, 1, 9, 9, 7, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_iron_fist, SLOT (0), 0, 0,
     "", "!iron fist!", "", FALSE, NULL, STAT_STR, INITIATE, -1, 0},
	 
	{
     "iron skin", {45, 30, 25, 1, 15, 1, 9, 9, 10, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_iron_skin, SLOT (0), 0, 0,
     "", "!iron skin!", "", FALSE, NULL, STAT_CON, APPRENTICE, -1, 0},
	 
	{
     "iron will", {45, 30, 25, 1, 15, 1, 9, 9, 15, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_iron_will, SLOT (0), 0, 0,
     "", "!iron will!", "", FALSE, NULL, STAT_CON, MEMBER, -1, 0},
	 
	{
     "palm strike", {45, 30, 25, 1, 15, 1, 9, 9, 20, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 4, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_palm_strike, SLOT (0), 0, 0,
     "palm strike", "!palmstrike!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, (TIME_ONE_MINUTE * 2)},
	 
	{
     "hand parry", {45, 30, 25, 1, 15, 1, 9, 9, 3, 41}, {0, 0, 0, 2, 0, 0, 0, 0, 1, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_hand_parry, SLOT (0), 0, 0,
     "", "!hand parry!", "", FALSE, NULL, STAT_DEX, INITIATE, -1, 0},	 
	
	{
     "presence of mind", {45, 30, 25, 1, 15, 1, 9, 9, 21, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_presence_of_mind, SLOT (0), 0, 0,
     "", "!presence of mind!", "", FALSE, &gsn_dodge, STAT_DEX, MEMBER, -1, 0},	 
	 
	 {//300
     "evasion", {45, 30, 25, 1, 15, 1, 9, 9, 16, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_evasion, SLOT (0), 0, 0,
     "", "!evasion!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, 0},
	 
	 {
     "pressure point", {45, 30, 25, 1, 15, 1, 9, 9, 11, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_pressure_point, SLOT (0), 0, 0,
     "", "!pressure point!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, 0},
	 
	 {
     "stun fist", {45, 30, 25, 1, 15, 1, 9, 9, 12, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_stun_fist, SLOT (0), 0, 18,
     "stunning fist", "!stun fist!", "", FALSE, &gsn_channel, STAT_STR, MEMBER, -1, TIME_TEN_SECONDS},
	 //wizard, cleric, thief, fighter, druid, ranger, paladin, bard, monk, psion
	 {
     "balance", {45, 30, 25, 1, 15, 1, 9, 9, 10, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_balance, SLOT (0), 0, 0,
     "", "!balance!", "", FALSE, NULL, STAT_DEX, APPRENTICE, -1, 0},
	 
	 {
     "channel", {45, 30, 25, 1, 15, 1, 9, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_channel, SLOT (0), 0, 6,
     "", "!channel!", "", FALSE, NULL, STAT_DEX, MEMBER, -1, TIME_FIVE_SECONDS},
	 
	 {//305
     "energy blast", {45, 30, 25, 1, 15, 1, 9, 9, 9, 41}, {0, 0, 0, 0, 0, 0, 0, 0, 3, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_energy_blast, SLOT (0), 0, 0,
     "channeled energy blast", "!energy blast!", "", FALSE, &gsn_channel, STAT_WIS, MEMBER, -1, TIME_TEN_SECONDS},

	{
     "lute", {45, 45, 45, 45, 45, 45, 45, 2, 45, 45}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_lute, SLOT (0), 0, 0,
     "", "!lute!", "", FALSE, NULL, STAT_CHA, APPRENTICE, -1, 0},
	
	{
     "piccolo", {45, 45, 45, 45, 45, 45, 45, 2, 45, 45}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_piccolo, SLOT (0), 0, 6,
     "", "!piccolo!", "", FALSE, NULL, STAT_CHA, APPRENTICE, -1, 0},
	 
	{
     "horn", {45, 45, 45, 45, 45, 45, 45, 2, 45, 45}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_horn, SLOT (0), 0, 6,
     "", "!horn!", "", FALSE, NULL, STAT_CHA, APPRENTICE, -1, 0},
	 
	{
     "drums", {45, 45, 45, 45, 45, 45, 45, 2, 45, 45}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_drums, SLOT (0), 0, 6,
     "", "!drums!", "", FALSE, NULL, STAT_CHA, APPRENTICE, -1, 0},
	 
	{
     "harp", {45, 45, 45, 45, 45, 45, 45, 2, 45, 45}, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_harp, SLOT (0), 0, 6,
     "", "!harp!", "", FALSE, NULL, STAT_CHA, APPRENTICE, -1, 0},
	 
	{
		"gore", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {0, 3, 3, 2, 3, 3, 3, 3, 2, 3},
		spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_gore, SLOT (0), 0, 12,
		"gore", "!Gore!", "", FALSE, NULL, STAT_STR, INITIATE, -1, TIME_FIVE_SECONDS
	},
	
};

const struct group_type group_table[MAX_GROUP] = {

    {
     "rom basics", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {"scrolls", "wands", "recall"}
     },

    {
     "wizard basics", {0, -1, -1, -1, -1, -1, -1, -1, -1, -1},
     {"detect magic", "dagger", "armor", "lore", "staff", "detect invis"}
     },

    {
     "cleric basics", {-1, 0, -1, -1, -1, -1, -1, -1, -1, -1},
     {"mace", "cure light", "bless", "cure poison", "detect poison", "refresh"}
     },

    {
     "thief basics", {-1, -1, 0, -1, -1, -1, -1, -1, -1, -1},
     {"dagger", "steal", "peek", "detect secret", "detect traps", "lay trap", "remove trap", "backstab", "crossbow"}
     },
 
    {
     "fighter basics", {-1, -1, -1, 0, -1, -1, -1, -1, -1, -1},
     {"short sword", "long sword", "spear", "mace", "axe", "dagger", "polearm", "whip", "flail", "staff", "bow", "crossbow", "berserk"}
     },
	 
	 {
     "druid basics", {-1, -1, -1, -1, 0, -1, -1, -1, -1, -1},
     {"cure light", "bless", "forage", "staff", "armor", "herbalism"}
     },
	 
	 {
     "ranger basics", {-1, -1, -1, -1, -1, 0, -1, -1, -1, -1},
     {"short sword", "long sword", "spear", "dagger", "pick lock", "forage", "tracking", "bow"}
     },
	 
	 {
     "paladin basics", {-1, -1, -1, -1, -1, -1, 0, -1, -1, -1},
     {"long sword", "short sword", "mace", "shield block", "parry", "rescue", "lay hands", "bless"}
     },
	 
	 {
     "bard basics", {-1, -1, -1, -1, -1, -1, -1, 0, -1, -1},
     {"short sword", "long sword", "dagger", "pick lock", "lore", "vocal shield"}
     },
	 
	 {
     "monk basics", {-1, -1, -1, -1, -1, -1, -1, -1, 0, -1},
     {"hand to hand", "dodge", "staff", "enhanced damage", "bless"}
     },

	 {
     "psion basics", {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0},
     {"dagger", "staff"}
     },
	 
	 //Groups, which are obsolete in this mud. - Upro
    {
     "weaponsmaster", {40, 40, 40, 20, 40, 30, 30, 30, 30},
     {"axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip"}
     },

    {
     "attack", {-1, 5, -1, 10, 6, 9, 9, 9, 9},
     {"demonfire", "dispel evil", "dispel good", "earthquake",
      "flamestrike", "heat metal", "ray of truth"}
     },

    {
     "beguiling", {4, -1, 6, -1, -1, -1, 9, 9, 9},
     {"calm", "charm person", "sleep"}
     },

    {
     "benedictions", {-1, 4, -1, 10, 4, 8, 9, 9, 9},
     {"bless", "calm", "frenzy", "holy word", "remove curse"}
     },

    {
     "combat", {6, -1, 10, 9, -1, -1, 9, 9, 9},
     {"acid blast", "burning hands", "chain lightning", "chill touch",
      "colour spray", "fireball", "lightning bolt", "magic missile",
      "shocking grasp"}
     },

    {
     "creation", {4, 4, 8, 8, 4, 8, 9, 9, 9},
     {"continual light", "create food", "create spring", "create water",
      "create rose", "floating disc"}
     },

    {
     "curative", {-1, 4, -1, 8, 3, 5, 9, 9, 9},
     {"cure blindness", "cure disease", "cure poison"}
     },

    {
     "detection", {4, 3, 6, -1, 4, -1, 9, 9, 9},
     {"detect evil", "detect good", "detect hidden", "detect invis",
      "detect magic", "detect poison", "farsight", "identify",
      "know alignment", "locate object"}
     },

    {
     "draconian", {8, -1, -1, -1, -1, -1, 9, 9, 9},
     {"acid breath", "fire breath", "frost breath", "gas breath",
      "lightning breath"}
     },

    {
     "enchantment", {6, -1, -1, -1, -1, -1, 9, 9, 9},
     {"enchant armor", "enchant weapon", "fireproof", "recharge"}
     },

    {
     "enhancement", {5, -1, 9, 9, 9, 9, 9, 9, 9},
     {"giant strength", "haste", "infravision", "refresh"}
     },

    {
     "harmful", {-1, 3, -1, 6, 9, 9, 9, 9, 9},
     {"cause critical", "cause light", "cause serious", "harm"}
     },

    {
     "healing", {-1, 3, -1, 6, 4, 8, 9, 9, 9},
     {"cure critical", "cure light", "cure serious", "heal",
      "mass healing", "refresh"}
     },

    {
     "illusion", {4, -1, 7, -1, 9, 9, 9, 9, 9},
     {"invis", "mass invis", "ventriloquate"}
     },

    {
     "maladictions", {5, 5, 9, 9, 9, 9, 9, 9, 9},
     {"blindness", "change sex", "curse", "energy drain", "plague",
      "poison", "slow", "weaken"}
     },

    {
     "protective", {4, 4, 7, 8, 9, 9, 9, 9, 9},
     {"armor", "cancellation", "dispel magic", "fireproof",
      "protection evil", "protection good", "sanctuary", "shield",
      "stone skin"}
     },

    {
     "transportation", {4, 4, 8, 9, 9, 9, 9, 9, 9},
     {"fly", "gate", "nexus", "pass door", "portal", "summon", "teleport",
      "word of recall"}
     },

    {
     "weather", {4, 4, 8, 8, 5, 4, 9, 9, 9},
     {"call lightning", "control weather", "faerie fire", "faerie fog",
      "lightning bolt"}
     }



};




/* god table */
 const   struct  god_type       god_table      []           = 
 {
 /*
     {
        name,
        player_good?,   	player_neutral?,	player_evil?,
        player_mage?,   	player_thief?,     	player_warrior?,  
		player_cleric?, 	player_ranger?, 	player_druid?, 
		player_paladin?, 	player_bard?, 		player_monk?,
		favor,
		domain1, domain2, domain3
     },
 */
 
    { "Nobody", 
		FALSE, FALSE, FALSE, 
		FALSE, FALSE, FALSE, 
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		"",
		-1,-1,-1
	},
 
    { "Maelik", //combat
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE, 	TRUE,
		FALSE,	TRUE,	TRUE,
	    "",
		DOM_COMBAT, -1, -1
    },
	
	{ "Omari", //magic
		TRUE,	TRUE,	TRUE,
		TRUE,	FALSE,	FALSE,
		TRUE,	FALSE,  TRUE,
		FALSE,	TRUE,	FALSE,
	    "",
		-1,-1,-1
    },
	
	{ "Nelenwe", //neutrality
		FALSE,	TRUE,	FALSE,
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE, 	TRUE,
		FALSE,	TRUE,	TRUE,
	    "",
		-1,-1,-1
    },
	
	{ "Nefyr", //chaos
		FALSE,	TRUE,	TRUE,
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE, 	TRUE,
		FALSE,	TRUE,	FALSE,
	    "",
		-1,-1,-1
    },
	
	{ "Lirannon", //light
		TRUE,	FALSE,	FALSE,
		TRUE,	FALSE,	TRUE,
		TRUE,	TRUE, 	TRUE,
		TRUE,	TRUE,	TRUE,
	    "",
		-1,-1,-1
    },
	
	{ "Daij", //dark
		FALSE,	FALSE,	TRUE,
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE, 	FALSE,
		FALSE,	TRUE,	TRUE,
	    "",
		-1,-1,-1
    },
		
	{ "Aziel", //discipline
		TRUE,	TRUE,	TRUE,
		TRUE,	FALSE,	TRUE,
		TRUE,	TRUE, 	TRUE,
		TRUE,	FALSE,	TRUE,
	    "",
		-1,-1,-1
    },
	
	{ "Andora", //nature
		TRUE,	TRUE,	FALSE,
		FALSE,	FALSE,	TRUE,
		TRUE,	TRUE, 	TRUE,
		FALSE,	TRUE,	TRUE,
	    "",
		-1,-1,-1
    },
	
	{ "Jopha", //fortune/ luck
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE,	TRUE,
		TRUE,	TRUE, 	FALSE,
		FALSE,	TRUE,	FALSE,
	    "",
		-1, -1, -1
    },
	
};


/* for clans */
const struct clan_type clan_table[MAX_CLAN] = {
    /*  name,       who entry,  death-transfer room,    independent?, clan channel color */
    /* independent should be FALSE if is a real clan */
    {"",			"",		 			ROOM_VNUM_ALTAR, 	0,							TRUE, "{x"},
    {"loner",		"({xLoner{x) ", 	ROOM_VNUM_ALTAR, 	0,							TRUE, "{x"},    
	{"conclave", 	"({c Con {x) ", 	ROOM_VNUM_ALTAR, 	CLAN_OBJ_CONCLAVE,			FALSE,"{c"}, //mages
	{"acolyte", 	"({W Aco {x) ", 	ROOM_VNUM_ALTAR, 	CLAN_OBJ_ACOLYTE,			FALSE,"{W"}, //do gooders
	{"guardian", 	"({Y Gua {x) ", 	ROOM_VNUM_ALTAR, 	CLAN_OBJ_GUARDIAN,			FALSE,"{Y"}, //paladins/fighters
	{"ravager",		"({R Rav {x) ", 	ROOM_VNUM_ALTAR, 	CLAN_OBJ_RAVAGER,			FALSE,"{R"}, //pk
	{"obsidian",	"({D Obs {x) ", 	ROOM_VNUM_ALTAR, 	CLAN_OBJ_OBSIDIAN,			FALSE,"{r"}, // minotaur clan, unwavering honor
	{"song",		"({B Song{x) ",		ROOM_VNUM_ALTAR,	CLAN_OBJ_SONG,  			FALSE,"{B"}, //bards
	{"corsair",		"({m Cor {x) ",		ROOM_VNUM_ALTAR,	CLAN_OBJ_CORSAIR,			FALSE,"{m"}, //thieves/bards/mercenaries
	{"jaezred",		"({y Jae {x) ",		ROOM_VNUM_ALTAR,	CLAN_OBJ_JAEZRED,			FALSE,"{y"}, //jaezred chaulssin (male drows trying to be free of Lloth)
	{"d'aerthe",	"({D D'ae{x) ",		ROOM_VNUM_ALTAR,	CLAN_OBJ_DAERTHE,			FALSE,"{D"}  //bregan d'aerthe		
};

/* for position */
const struct position_type position_table[] = {
    {"dead", "dead"},
    {"mortally wounded", "mort"},
    {"incapacitated", "incap"},
    {"stunned", "stun"},
    {"sleeping", "sleep"},
    {"resting", "rest"},
    {"sitting", "sit"},
    {"fighting", "fight"},
    {"standing", "stand"},
    {NULL, NULL}
};

/* for sex */
const struct sex_type sex_table[] = {
    {"none"},
    {"male"},
    {"female"},
    {"either"},
    {NULL}
};

/* for sizes */
const struct size_type size_table[] = {
    {"tiny"},
    {"small"},
    {"medium"},
    {"large"},
    {"huge"},    
	{"gargantuan"},
    {NULL}
};

/* various flag tables */

const struct flag_type quest_types[] = {
	{"mob_kill", 		A, 		TRUE},
	{"item_retrieve", 	B, 		TRUE},
	{"area_explore", 	C, 		TRUE},
	{"help_read",	 	D, 		TRUE},
	{"mat_retrieve", 	E, 		TRUE},
	{"repeatable",		F,		TRUE},
	{NULL, 0, FALSE}	
};

const struct flag_type eye_color_flags[] = 
{
	{"blue", 	A,		true},
	{"green",	B,		true},
	{"hazel",	C,		true},
	{"brown",	D,		true},
	{"red",		E,		true},
	{"black",	F,		true},
	{"purple",	G,		true},
	{NULL, 0, FALSE}
};

const struct flag_type hair_color_flags[] = 
{
	{"blonde", 	A,		true},
	{"brown",	B,		true},
	{"black",	C,		true},
	{"red",		D,		true},
	{"white",	E,		true},
	{"grey",	F,		true},
	{NULL, 0, FALSE}
};

const struct flag_type haircut_flags[] = 
{
	{"bald", 	A,		true},
	{"bowl",	B,		true},
	{"mullet",	C,		true},
	{"mohawk",	D,		true},
	{"crew",	E,		true},
	{"untamed",	F,		true},
	{"spiked",  G,		true},
	{"cropped",	H,		true},
	{"ponytail",I,		true},
	
	{"braided",	J,		true},
	{"double braided",	K,		true},
	{"curly",	L,		true},
	{"straight long",	M,		true},
	{"straight short",	N,		true},
	{"buns",			O,	true},
	{NULL, 0, FALSE}
};

const struct flag_type facial_hair_flags[] = 
{
	{"none", 		A,		true},
	{"sideburns",	B,		true},
	{"muttonchops",	C,		true},
	{"goatee",		D,		true},
	{"mustache",	E,		true},
	{"short beard",	F,		true},
	{"long beard",  G,		true},
	{NULL, 0, FALSE}
};

const struct flag_type act_flags[] = {
    {"npc", A, FALSE},
    {"sentinel", B, TRUE},
    {"scavenger", C, TRUE},
    {"aggressive", F, TRUE},
    {"stay_area", G, TRUE},
    {"wimpy", H, TRUE},
    {"pet", I, TRUE},
    {"train", J, TRUE},
    {"practice", K, TRUE},
    {"undead", O, TRUE},
    {"cleric", Q, TRUE},
    {"mage", R, TRUE},
    {"thief", S, TRUE},
    {"warrior", T, TRUE},
    {"noalign", U, TRUE},
    {"nopurge", V, TRUE},
    {"outdoors", W, TRUE},
    {"indoors", Y, TRUE},
    {"healer", aa, TRUE},
    {"gain", bb, TRUE},
    {"update_always", cc, TRUE},
    {"changer", dd, TRUE},
	{"banker", ee, TRUE},
    {NULL, 0, FALSE}
};

const struct flag_type act2_flags [] =
{
   {	"bounty",		A, TRUE  	},   
   {	"stay_sector",	B, TRUE  	},
   { 	"random_loot",	C, TRUE		},
   {	"elite",		D, TRUE		},
   {	"boss",			E, TRUE		},
   { 	"sage",			F, TRUE		},
   {    "peaceful",		G, TRUE		},
   {	"mobinvis",		H, TRUE		},
   {	"inanimate",	I, TRUE		},
   {    "mount",        J, TRUE		},
   {     NULL,      	0, FALSE	}
};

const struct flag_type plr_flags[] = {
    {"npc", A, FALSE},
	{"autodig", B, FALSE},
    {"autoassist", C, FALSE},
    {"autoexit", D, FALSE},
    {"autoloot", E, FALSE},
    {"autosac", F, FALSE},
    {"autogold", G, FALSE},
    {"autosplit", H, FALSE},
	{"mxp",	I,	FALSE},
    {"autotrack", J, FALSE},
	{"automap", K, TRUE},
	{"holylight", N, FALSE},
    {"can_loot", P, FALSE},
    {"nosummon", Q, FALSE},
    {"nofollow", R, FALSE},
    {"colour", T, FALSE},
    {"permit", U, TRUE},
    {"log", W, FALSE},
    {"deny", X, FALSE},
    {"freeze", Y, FALSE},
    {"thief", Z, FALSE},
    {"killer", aa, FALSE},
	{"autosheathe", bb, TRUE},
	{"autodrink", cc, TRUE},
	{"autoeat", dd, TRUE},
	{"autoscroll", ee, TRUE},
    {NULL, 0, 0}
};

const struct flag_type affect_flags[] = {
    {"blind", A, TRUE},
    {"invisible", B, TRUE},
    {"detect_evil", C, TRUE},
    {"detect_invis", D, TRUE},
    {"detect_magic", E, TRUE},
    {"detect_hidden", F, TRUE},
    {"detect_good", G, TRUE},
    {"sanctuary", H, TRUE},
    {"faerie_fire", I, TRUE},
    {"infrared", J, TRUE},
    {"curse", K, TRUE},
    {"poison", M, TRUE},
    {"protect_evil", N, TRUE},
    {"protect_good", O, TRUE},
    {"sneak", P, TRUE},
    {"hide", Q, TRUE},
    {"sleep", R, TRUE},
    {"charm", S, TRUE},
    {"flying", T, TRUE},
    {"pass_door", U, TRUE},
    {"haste", V, TRUE},
    {"calm", W, TRUE},
    {"plague", X, TRUE},
    {"weaken", Y, TRUE},
    {"dark_vision", Z, TRUE},
    {"berserk", aa, TRUE},
    {"swim", bb, TRUE},
    {"regeneration", cc, TRUE},
    {"slow", dd, TRUE},
	{"extension", ee, TRUE},
	//{"audible glamor", ff, TRUE},
    {NULL, 0, 0}
};

const struct flag_type off_flags[] = {
    {"area_attack", A, TRUE},
    {"backstab", B, TRUE},
    {"bash", C, TRUE},
    {"berserk", D, TRUE},
    {"disarm", E, TRUE},
    {"dodge", F, TRUE},
    {"fade", G, TRUE},
    {"fast", H, TRUE},
    {"kick", I, TRUE},
    {"dirt_kick", J, TRUE},
    {"parry", K, TRUE},
    {"rescue", L, TRUE},
    {"tail", M, TRUE},
    {"trip", N, TRUE},
    {"crush", O, TRUE},
    {"assist_all", P, TRUE},
    {"assist_align", Q, TRUE},
    {"assist_race", R, TRUE},
    {"assist_players", S, TRUE},
    {"assist_guard", T, TRUE},
    {"assist_vnum", U, TRUE},
	{"extra_attack", V, TRUE},
	{"extra_attack2", W, TRUE},
    {NULL, 0, 0}
};

const struct flag_type imm_flags[] = {
    {"summon", A, TRUE},
    {"charm", B, TRUE},
    {"magic", C, TRUE},
    {"weapon", D, TRUE},
    {"bash", E, TRUE},
    {"pierce", F, TRUE},
    {"slash", G, TRUE},
    {"fire", H, TRUE},
    {"cold", I, TRUE},
    {"lightning", J, TRUE},
    {"acid", K, TRUE},
    {"poison", L, TRUE},
    {"negative", M, TRUE},
    {"holy", N, TRUE},
    {"energy", O, TRUE},
    {"mental", P, TRUE},
    {"disease", Q, TRUE},
    {"drowning", R, TRUE},
    {"light", S, TRUE},
    {"sound", T, TRUE},
    {"wood", X, TRUE},
    {"silver", Y, TRUE},
    {"iron", Z, TRUE},
    {NULL, 0, 0}
};

const struct flag_type form_flags[] = {
    {"edible", FORM_EDIBLE, TRUE},
    {"poison", FORM_POISON, TRUE},
    {"magical", FORM_MAGICAL, TRUE},
    {"instant_decay", FORM_INSTANT_DECAY, TRUE},
    {"other", FORM_OTHER, TRUE},
    {"animal", FORM_ANIMAL, TRUE},
    {"sentient", FORM_SENTIENT, TRUE},
    {"undead", FORM_UNDEAD, TRUE},
    {"construct", FORM_CONSTRUCT, TRUE},
    {"mist", FORM_MIST, TRUE},
    {"intangible", FORM_INTANGIBLE, TRUE},
    {"biped", FORM_BIPED, TRUE},
    {"centaur", FORM_CENTAUR, TRUE},
    {"insect", FORM_INSECT, TRUE},
    {"spider", FORM_SPIDER, TRUE},
    {"crustacean", FORM_CRUSTACEAN, TRUE},
    {"worm", FORM_WORM, TRUE},
    {"blob", FORM_BLOB, TRUE},
    {"mammal", FORM_MAMMAL, TRUE},
    {"bird", FORM_BIRD, TRUE},
    {"reptile", FORM_REPTILE, TRUE},
    {"snake", FORM_SNAKE, TRUE},
    {"dragon", FORM_DRAGON, TRUE},
    {"amphibian", FORM_AMPHIBIAN, TRUE},
    {"fish", FORM_FISH, TRUE},
    {"cold_blood", FORM_COLD_BLOOD, TRUE},
    {NULL, 0, 0}
};

const struct flag_type part_flags[] = {
    {"head", PART_HEAD, TRUE},
    {"arms", PART_ARMS, TRUE},
    {"legs", PART_LEGS, TRUE},
    {"heart", PART_HEART, TRUE},
    {"brains", PART_BRAINS, TRUE},
    {"guts", PART_GUTS, TRUE},
    {"hands", PART_HANDS, TRUE},
    {"feet", PART_FEET, TRUE},
    {"fingers", PART_FINGERS, TRUE},
    {"ear", PART_EAR, TRUE},
    {"eye", PART_EYE, TRUE},
    {"long_tongue", PART_LONG_TONGUE, TRUE},
    {"eyestalks", PART_EYESTALKS, TRUE},
    {"tentacles", PART_TENTACLES, TRUE},
    {"fins", PART_FINS, TRUE},
    {"wings", PART_WINGS, TRUE},
    {"tail", PART_TAIL, TRUE},
    {"claws", PART_CLAWS, TRUE},
    {"fangs", PART_FANGS, TRUE},
    {"horns", PART_HORNS, TRUE},
    {"scales", PART_SCALES, TRUE},
    {"tusks", PART_TUSKS, TRUE},
    {NULL, 0, 0}
};

const struct flag_type comm_flags[] = {
    {"quiet", COMM_QUIET, TRUE},
    {"deaf", COMM_DEAF, TRUE},
	{"oocoff",COMM_OOCOFF,        TRUE    },
    {"nowiz", COMM_NOWIZ, TRUE},
    {"noclangossip", COMM_NOAUCTION, TRUE},
    {"nogossip", COMM_NOGOSSIP, TRUE},
    {"noquestion", COMM_NOQUESTION, TRUE},
    {"nomusic", COMM_NOMUSIC, TRUE},
    {"noclan", COMM_NOCLAN, TRUE},
    {"noquote", COMM_NOQUOTE, TRUE},
    {"shoutsoff", COMM_SHOUTSOFF, TRUE},
    {"compact", COMM_COMPACT, TRUE},
    {"brief", COMM_BRIEF, TRUE},
    {"prompt", COMM_PROMPT, TRUE},
    {"combine", COMM_COMBINE, TRUE},
    {"telnet_ga", COMM_TELNET_GA, TRUE},
    {"show_affects", COMM_SHOW_AFFECTS, TRUE},
    {"nograts", COMM_NOGRATS, TRUE},
    {"noemote", COMM_NOEMOTE, FALSE},
    {"noshout", COMM_NOSHOUT, FALSE},
    {"notell", COMM_NOTELL, FALSE},
    {"nochannels", COMM_NOCHANNELS, FALSE},
    {"snoop_proof", COMM_SNOOP_PROOF, FALSE},
    {"afk", COMM_AFK, TRUE},
    {NULL, 0, 0}
};

const struct flag_type mprog_flags[] = {
    {"act", TRIG_ACT, TRUE},
    {"bribe", TRIG_BRIBE, TRUE},
    {"death", TRIG_DEATH, TRUE},
    {"entry", TRIG_ENTRY, TRUE},
    {"fight", TRIG_FIGHT, TRUE},
    {"give", TRIG_GIVE, TRUE},
    {"greet", TRIG_GREET, TRUE},
    {"grall", TRIG_GRALL, TRUE},
    {"kill", TRIG_KILL, TRUE},
    {"hpcnt", TRIG_HPCNT, TRUE},
    {"random", TRIG_RANDOM, TRUE},
    {"speech", TRIG_SPEECH, TRUE},
    {"exit", TRIG_EXIT, TRUE},
    {"exall", TRIG_EXALL, TRUE},
    {"delay", TRIG_DELAY, TRUE},
    {"surr", TRIG_SURR, TRUE},
    {NULL, 0, TRUE}
};

const struct flag_type area_flags[] = {
    {"none",	AREA_NONE, 				FALSE},
    {"changed", AREA_CHANGED, 			TRUE},
    {"added", 	AREA_ADDED, 			TRUE},
    {"ship", 	AREA_SHIP, 				TRUE},
	{"loading", AREA_LOADING, 			FALSE},	
	{"forbiddance",	AREA_FORBIDDANCE,	TRUE},
	{"linked",	AREA_LINKED,			TRUE},
	{"random",	AREA_RANDOM,			TRUE},
	{"alarm",	AREA_ALARM,				TRUE},
	{"no_precip",AREA_PRECIP,			TRUE},
	{"clanhall", AREA_CLANHALL,			TRUE},
	
    {NULL, 0, 0}
};

const struct flag_type ship_flags[] = {
	{"anchored",	SHIP_ANCHORED,		TRUE},
	{"sail_raised",	SHIP_RAISED,		TRUE},
	{"docked",		SHIP_DOCKED,		TRUE},
	{NULL, 0, 0}
};



const struct flag_type sex_flags[] = {
    {"male", SEX_MALE, TRUE},
    {"female", SEX_FEMALE, TRUE},
    {"neutral", SEX_NEUTRAL, TRUE},
    {"random", 3, TRUE},        /* ROM */
    {"none", SEX_NEUTRAL, TRUE},
    {NULL, 0, 0}
};



const struct flag_type exit_flags[] = {
    {"door", EX_ISDOOR, TRUE},
    {"closed", EX_CLOSED, TRUE},
    {"locked", EX_LOCKED, TRUE},
    {"pickproof", EX_PICKPROOF, TRUE},
    {"nopass", EX_NOPASS, TRUE},
    {"easy", EX_EASY, TRUE},
    {"hard", EX_HARD, TRUE},
    {"infuriating", EX_INFURIATING, TRUE},
    {"noclose", EX_NOCLOSE, TRUE},
    {"nolock", EX_NOLOCK, TRUE},
	{"hidden", EX_HIDDEN, TRUE},
	{"secret", EX_SECRET, TRUE},
    {NULL, 0, 0}
};



const struct flag_type door_resets[] = {
    {"open and unlocked", 0, TRUE},
    {"closed and unlocked", 1, TRUE},
    {"closed and locked", 2, TRUE},
    {NULL, 0, 0}
};



const struct flag_type room_flags[] = {
    {"dark", ROOM_DARK, TRUE},
    {"no_mob", ROOM_NO_MOB, TRUE},
    {"indoors", ROOM_INDOORS, TRUE},
    {"private", ROOM_PRIVATE, TRUE},
    {"safe", ROOM_SAFE, TRUE},
    {"solitary", ROOM_SOLITARY, TRUE},
    {"pet_shop", ROOM_PET_SHOP, TRUE},
    {"no_recall", ROOM_NO_RECALL, TRUE},    
    {"gods_only", ROOM_GODS_ONLY, TRUE},
    {"heroes_only", ROOM_HEROES_ONLY, TRUE},
    {"newbies_only", ROOM_NEWBIES_ONLY, TRUE},
    {"law", ROOM_LAW, TRUE},
    {"nowhere", ROOM_NOWHERE, TRUE},
	{"anti_magic", ROOM_ANTI_MAGIC, TRUE},
	{"nosummon", ROOM_NOSUMMON, TRUE},
	{"nofight", ROOM_NOFIGHT, TRUE},
	{"anti_wizard", ROOM_ANTI_WIZARD, TRUE},
	{"anti_cleric", ROOM_ANTI_CLERIC, TRUE},
	{"anti_thief", ROOM_ANTI_THIEF, TRUE},
	{"anti_fighter", ROOM_ANTI_FIGHTER, TRUE},
	{"anti_druid", ROOM_ANTI_DRUID, TRUE},
	{"anti_ranger", ROOM_ANTI_RANGER, TRUE},
	{"anti_paladin", ROOM_ANTI_PALADIN, TRUE},
	{"anti_bard", ROOM_ANTI_BARD, TRUE},
	{"anti_monk", ROOM_ANTI_MONK, TRUE},
	{"no_tree", ROOM_NO_TREE, TRUE},
    {NULL, 0, 0}
};



const struct flag_type sector_flags[] = {
    {"inside", 		SECT_INSIDE, 		TRUE},
    {"city", 		SECT_CITY, 			TRUE},
    {"field", 		SECT_FIELD, 		TRUE},
    {"forest", 		SECT_FOREST, 		TRUE},
    {"hills", 		SECT_HILLS, 		TRUE},
    {"mountain", 	SECT_MOUNTAIN, 		TRUE},
    {"swim", 		SECT_WATER_SWIM, 	TRUE},
    {"noswim", 		SECT_WATER_NOSWIM, 	TRUE},
    {"unused", 		SECT_UNUSED, 		TRUE},
    {"air", 		SECT_AIR, 			TRUE},
    {"desert", 		SECT_DESERT, 		TRUE},
	{"road", 		SECT_ROAD, 			TRUE},
	{"beach", 		SECT_BEACH, 		TRUE},
	{"shoreline", 	SECT_SHORELINE, 	TRUE},
	{"cave", 		SECT_CAVE, 			TRUE},
	{"oceanfloor", 	SECT_OCEANFLOOR, 	TRUE},
	{"lava", 		SECT_LAVA, 			TRUE},
	{"swamp", 		SECT_SWAMP, 		TRUE},
	{"underground", SECT_UNDERGROUND, 	TRUE},
	{"ice", 		SECT_ICE, 			TRUE},
	{"snow", 		SECT_SNOW, 			TRUE},
	{"void", 		SECT_VOID, 			TRUE},
	{"ruins", 		SECT_RUINS, 		TRUE},
	{"wasteland", 	SECT_WASTELAND, 	TRUE},
	{"jungle", 		SECT_JUNGLE, 		TRUE},
	{"dock", 		SECT_DOCK, 			TRUE},
	{"ship", 		SECT_SHIP, 			TRUE},
    {NULL, 0, 0}
};
/*
const struct QUEST_INDEX_DATA quest_table[] = 
{
// quest name, index, repeatable, level, accept_vnum, return_vnum, quest info, quest rank, guild point reward, xp reward, obj reward vnum,
// quest type, vnum assoc., multiples, helps
	{
	"Newbie Quest", 0, FALSE, 1, -1, -1,
	"This quest is designed to teach you about guild ranks.\r\nGo kill a white rabbit! These are known to be found in Green Hills.\r\nThen read help guild/quest/gain, and return to your guild.", 
	INITIATE, 35, 100, 0, 0, MOB_HELP_COMBO, 2303, 5, "guild", "quest", "gain", -1
	},        	
	
	{
	"Assist the Alchemist", 1, FALSE, 2, -1, 1015,
	"The alchemist of Renfall, Rianna, seeks ten spider mandibles for a specific potion.\r\nThese are known to be found in the Mosgha Woods.", 
	INITIATE, 70, 200, 0, 0, ITEM_RETRIEVE, 2000, 10, "", "", "", -1
	},
	
	{
	"Mystara's Quest", 2, FALSE, 2, 2321, 2321,
	"Mystara seeks trophies from the fields in Green Hills, collect 5 and return them to her for a reward.", 
	INITIATE, 0, 300, 0, 2320, ITEM_RETRIEVE, 2319, 5, "", "", "", -1
	},
	
	{
	"Iron for Gold", 3, TRUE, 2, 1014, 1014,
	"The blacksmith in Renfall needs iron to forge things.\r\nBring 20 pieces of iron to him to receive 5 gold coins.", 
	INITIATE, 10, 25, 5, 0, MAT_RETRIEVE, MAT_IRON, 20, "", "", "", -1
	},
	
	{"", 1000, FALSE, 0, 0, 0, "", 0, 0, 0, 0, 0, 0, -1}
};
*/

const struct flag_type type_flags[] = {
    {"light", 			ITEM_LIGHT, 		TRUE},
    {"scroll", 			ITEM_SCROLL, 		TRUE},
    {"wand", 			ITEM_WAND, 			TRUE},
    {"staff", 			ITEM_STAFF, 		TRUE},
    {"weapon", 			ITEM_WEAPON, 		TRUE},
    {"treasure", 		ITEM_TREASURE, 		TRUE},
    {"armor", 			ITEM_ARMOR, 		TRUE},
    {"potion", 			ITEM_POTION, 		TRUE},
    {"furniture", 		ITEM_FURNITURE, 	TRUE},
    {"trash", 			ITEM_TRASH, 		TRUE},
    {"container", 		ITEM_CONTAINER, 	TRUE},
    {"drinkcontainer", 	ITEM_DRINK_CON, 	TRUE},
    {"key", 			ITEM_KEY, 			TRUE},
    {"food", 			ITEM_FOOD, 			TRUE},
    {"money", 			ITEM_MONEY, 		TRUE},
    {"boat", 			ITEM_BOAT, 			TRUE},
    {"npccorpse", 		ITEM_CORPSE_NPC,	TRUE},
    {"pc corpse", 		ITEM_CORPSE_PC, 	FALSE},
    {"fountain", 		ITEM_FOUNTAIN, 		TRUE},
    {"pill", 			ITEM_PILL, 			TRUE},
    {"protect", 		ITEM_PROTECT, 		TRUE},
    {"map", 			ITEM_MAP, 			TRUE},
    {"portal", 			ITEM_PORTAL,	 	TRUE},
    {"warpstone", 		ITEM_WARP_STONE, 	TRUE},
    {"roomkey", 		ITEM_ROOM_KEY, 		TRUE},
    {"gem", 			ITEM_GEM, 			TRUE},
    {"jewelry", 		ITEM_JEWELRY, 		TRUE},
    {"jukebox", 		ITEM_JUKEBOX, 		TRUE},
	{"pen",				ITEM_PEN, 			TRUE},
	{"parchment", 		ITEM_PAPER,		 	TRUE},
	{"skin", 			ITEM_SKIN, 			TRUE},
	{"scry", 			ITEM_SCRY, 			TRUE},
	{"thieves_tools", 	ITEM_THIEVES_TOOLS, TRUE},
	{"salve", 			ITEM_SALVE, 		TRUE},
	{"bandage", 		ITEM_BANDAGE, 		TRUE},
	{"herb", 			ITEM_HERB, 			TRUE},
	{"mining_tool", 	ITEM_MINING_TOOL, 	TRUE},
	{"sieve",		 	ITEM_SIEVE,			TRUE},
	{"fishing_pole",	ITEM_FISH_POLE,		TRUE},
	{"alchemy_lab",		ITEM_ALCHEMY_LAB,	TRUE},	
	{"missile",			ITEM_MISSILE,		TRUE},
	{"blood_pool",		ITEM_BLOOD_POOL,	TRUE},
	{"tracks",			ITEM_TRACKS,		TRUE},
	{"anvil",			ITEM_ANVIL,			TRUE},
	{"loom",			ITEM_LOOM,			TRUE},
	{"fire",			ITEM_FIRE,			TRUE},
	{"building",		ITEM_BUILDING,		TRUE},
	{"tree",			ITEM_TREE,			TRUE},
	{"figurine",		ITEM_FIGURINE,		TRUE},
	{"ship_helm",		ITEM_SHIP_HELM,		TRUE},
	{"alchemy_recipe",	ITEM_ALCHEMY_RECIPE,TRUE},
	{"cooking_recipe",	ITEM_COOKING_RECIPE,TRUE},
	{"blacksmith_plans",ITEM_BLACKSMITH_PLANS,TRUE},
	{"tailoring_plans", ITEM_TAILORING_PLANS,TRUE},
	{"flask",			ITEM_FLASK,			TRUE},
	{"ingredient",		ITEM_INGREDIENT,	TRUE},
	{"shovel",			ITEM_SHOVEL,		TRUE},	
	{"instrument",		ITEM_INSTRUMENT,	TRUE},
	{"blacksmith_hammer", ITEM_BLACKSMITH_HAMMER, TRUE},
	{"sheath",			ITEM_SHEATH,	TRUE},
	{"earring",			ITEM_EARRING,	TRUE},
    {NULL, 0, 0}
};

const struct flag_type extra2_flags[] = {
	{"hidden", 		ITEM_HIDDEN, 		TRUE},
	{"wear_cast", 	ITEM_WEAR_CAST, 	TRUE},
	{"epic", 		ITEM_EPIC, 			TRUE},
	{"legendary", 	ITEM_LEGENDARY, 	TRUE},
	{"artifact",	ITEM_ARTIFACT,		TRUE},
	{"quest_item",	ITEM_QUEST_ITEM,	TRUE},
	{"spiked",		ITEM_SPIKED,		TRUE},
	{"can_push",	ITEM_CAN_PUSH,		TRUE},
	{"can_pull",	ITEM_CAN_PULL,		TRUE},
	{"can_pry",		ITEM_CAN_PRY,		TRUE},
	{"can_press",	ITEM_CAN_PRESS,		TRUE},
	{"buoyant",		ITEM_BUOYANT,		TRUE},	
	{"obscure",		ITEM_OBSCURE,		TRUE},
	{"unique",		ITEM_UNIQUE,		TRUE},
	{"shock_trap", 	ITEM_SHOCK_TRAP, 	TRUE},
	{"hardstaff", 	ITEM_HARDSTAFF, 	TRUE},
	{"shillelagh", 	ITEM_SHILLELAGH, 	TRUE},
	{"waterproof", 	ITEM_WATERPROOF, 	TRUE},
	{"rusted", 		ITEM_RUSTED,	 	TRUE},
	{"no_recharge", ITEM_NO_RECHARGE, 	TRUE},
	
	{NULL, 0, 0}	
};
const struct flag_type extra_flags[] = {
    {"glow", ITEM_GLOW, TRUE},
    {"hum", ITEM_HUM, TRUE},
    {"dark", ITEM_DARK, TRUE},
    {"lock", ITEM_LOCK, TRUE},
    {"evil", ITEM_EVIL, TRUE},
    {"invis", ITEM_INVIS, TRUE},
    {"magic", ITEM_MAGIC, TRUE},
    {"nodrop", ITEM_NODROP, TRUE},
    {"bless", ITEM_BLESS, TRUE},
    {"antigood", ITEM_ANTI_GOOD, TRUE},
    {"antievil", ITEM_ANTI_EVIL, TRUE},
    {"antineutral", ITEM_ANTI_NEUTRAL, TRUE},
    {"noremove", ITEM_NOREMOVE, TRUE},
    {"inventory", ITEM_INVENTORY, TRUE},
    {"nopurge", ITEM_NOPURGE, TRUE},
    {"rotdeath", ITEM_ROT_DEATH, TRUE},
    {"visdeath", ITEM_VIS_DEATH, TRUE},
    {"nonmetal", ITEM_NONMETAL, TRUE},
	{"nolocate", ITEM_NOLOCATE, TRUE},
    {"meltdrop", ITEM_MELT_DROP, TRUE},
    {"hadtimer", ITEM_HAD_TIMER, TRUE},
    {"sellextract", ITEM_SELL_EXTRACT, TRUE},
    {"burnproof", ITEM_BURN_PROOF, TRUE},
    {"nouncurse", ITEM_NOUNCURSE, TRUE},
	{"fire_trap", ITEM_FIRE_TRAP, TRUE},
    {"poison_trap", ITEM_POISON_TRAP, TRUE},
    {"gas_trap", ITEM_GAS_TRAP, TRUE},
    {"dart_trap", ITEM_DART_TRAP, TRUE},    
	{"indestructable", ITEM_INDESTRUCTABLE, TRUE},
    {NULL, 0, 0}
};



const struct flag_type wear_flags[] = {
    {"take", ITEM_TAKE, TRUE},
    {"finger", ITEM_WEAR_FINGER, TRUE},
    {"neck", ITEM_WEAR_NECK, TRUE},
    {"body", ITEM_WEAR_BODY, TRUE},
    {"head", ITEM_WEAR_HEAD, TRUE},
    {"legs", ITEM_WEAR_LEGS, TRUE},
    {"feet", ITEM_WEAR_FEET, TRUE},
    {"hands", ITEM_WEAR_HANDS, TRUE},
    {"arms", ITEM_WEAR_ARMS, TRUE},
    {"shield", ITEM_WEAR_SHIELD, TRUE},
    {"about", ITEM_WEAR_ABOUT, TRUE},
    {"waist", ITEM_WEAR_WAIST, TRUE},
    {"wrist", ITEM_WEAR_WRIST, TRUE},
    {"wield", ITEM_WIELD, TRUE},
    {"hold", ITEM_HOLD, TRUE},
    {"nosac", ITEM_NO_SAC, TRUE},
    {"wearfloat", ITEM_WEAR_FLOAT, TRUE},
	{"tail",	ITEM_WEAR_TAIL, TRUE},
	{"sheath",	ITEM_WEAR_SHEATH, TRUE},
/*    {   "twohands",            ITEM_TWO_HANDS,         TRUE    }, */
    {NULL, 0, 0}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] = {
    {"none", APPLY_NONE, TRUE},
    {"strength", APPLY_STR, TRUE},
    {"dexterity", APPLY_DEX, TRUE},
    {"intelligence", APPLY_INT, TRUE},
    {"wisdom", APPLY_WIS, TRUE},
    {"constitution", APPLY_CON, TRUE},
	{"sex", APPLY_SEX, TRUE},
    {"class", APPLY_CLASS, TRUE},
    {"level", APPLY_LEVEL, TRUE},
    {"age", APPLY_AGE, TRUE},
    {"height", APPLY_HEIGHT, TRUE},
    {"weight", APPLY_WEIGHT, TRUE},
    {"mana", APPLY_MANA, TRUE},
    {"hp", APPLY_HIT, TRUE},
    {"move", APPLY_MOVE, TRUE},
    {"gold", APPLY_GOLD, TRUE},
    {"experience", APPLY_EXP, TRUE},
    {"ac", APPLY_AC, TRUE},
    {"hitroll", APPLY_HITROLL, TRUE},
    {"damroll", APPLY_DAMROLL, TRUE},
    {"saves", APPLY_SAVES, TRUE},
    {"savingpara", APPLY_SAVING_PARA, TRUE},
    {"savingrod", APPLY_SAVING_ROD, TRUE},
    {"savingpetri", APPLY_SAVING_PETRI, TRUE},
    {"savingbreath", APPLY_SAVING_BREATH, TRUE},
    {"savingspell", APPLY_SAVING_SPELL, TRUE},
	{"apply_size",  APPLY_SIZE, TRUE},
	{"spellcrit",	APPLY_SPELL_CRIT, TRUE},
	{"meleecrit",	APPLY_MELEE_CRIT, TRUE},	
	{"spelldam",	APPLY_SPELL_DAM, TRUE},
	{"charisma", 	APPLY_CHA, TRUE},
    {NULL, 0, 0}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] = {
    {"in the inventory", WEAR_NONE, TRUE},
    {"as a light", WEAR_LIGHT, TRUE},
    {"on the left finger", WEAR_FINGER_L, TRUE},
    {"on the right finger", WEAR_FINGER_R, TRUE},
    {"around the neck (1)", WEAR_NECK_1, TRUE},
    {"around the neck (2)", WEAR_NECK_2, TRUE},
    {"on the body", WEAR_BODY, TRUE},
    {"over the head", WEAR_HEAD, TRUE},
    {"on the legs", WEAR_LEGS, TRUE},
    {"on the feet", WEAR_FEET, TRUE},
    {"on the hands", WEAR_HANDS, TRUE},
    {"on the arms", WEAR_ARMS, TRUE},
    {"as a shield", WEAR_SHIELD, TRUE},
    {"about the shoulders", WEAR_ABOUT, TRUE},
    {"around the waist", WEAR_WAIST, TRUE},
    {"on the left wrist", WEAR_WRIST_L, TRUE},
    {"on the right wrist", WEAR_WRIST_R, TRUE},
    {"wielded", WEAR_WIELD, TRUE},
	{"held in the hands", WEAR_HOLD, TRUE},
    {"floating nearby", WEAR_FLOAT, TRUE},
	{"on the tail", WEAR_TAIL, TRUE},
	{"as a sheath", WEAR_SHEATH, TRUE},
    {NULL, 0, 0}
};


const struct flag_type wear_loc_flags[] = {
    {"none", WEAR_NONE, TRUE},
    {"light", WEAR_LIGHT, TRUE},
    {"lfinger", WEAR_FINGER_L, TRUE},
    {"rfinger", WEAR_FINGER_R, TRUE},
    {"neck1", WEAR_NECK_1, TRUE},
    {"neck2", WEAR_NECK_2, TRUE},
    {"body", WEAR_BODY, TRUE},
    {"head", WEAR_HEAD, TRUE},
    {"legs", WEAR_LEGS, TRUE},
    {"feet", WEAR_FEET, TRUE},
    {"hands", WEAR_HANDS, TRUE},
    {"arms", WEAR_ARMS, TRUE},
    {"shield", WEAR_SHIELD, TRUE},
    {"about", WEAR_ABOUT, TRUE},
    {"waist", WEAR_WAIST, TRUE},
    {"lwrist", WEAR_WRIST_L, TRUE},
    {"rwrist", WEAR_WRIST_R, TRUE},
    {"wielded", WEAR_WIELD, TRUE},
    {"hold", WEAR_HOLD, TRUE},
    {"floating", WEAR_FLOAT, TRUE},
	{"tail",	WEAR_TAIL, TRUE},
	{"sheath", WEAR_SHEATH, TRUE},
    {NULL, 0, 0}
};

const struct flag_type container_flags[] = {
    {"closeable", 1, TRUE},
    {"pickproof", 2, TRUE},
    {"closed", 4, TRUE},
    {"locked", 8, TRUE},
    {"puton", 16, TRUE},
	{"nocount",32,TRUE},
    {NULL, 0, 0}
};

/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/




const struct flag_type ac_type[] = {
    {"pierce", AC_PIERCE, TRUE},
    {"bash", AC_BASH, TRUE},
    {"slash", AC_SLASH, TRUE},
    {"exotic", AC_EXOTIC, TRUE},
    {NULL, 0, 0}
};


const struct flag_type size_flags[] = {
    {"tiny", SIZE_TINY, TRUE},
    {"small", SIZE_SMALL, TRUE},
    {"medium", SIZE_MEDIUM, TRUE},
    {"large", SIZE_LARGE, TRUE},
    {"huge", SIZE_HUGE, TRUE},
    {"giant", SIZE_GIANT, TRUE},
	{"gargantuan", SIZE_GARGANTUAN, TRUE},
    {NULL, 0, 0},
};


const struct flag_type weapon_class[] = {
    {"exotic", 		WEAPON_EXOTIC, 			TRUE},
    {"short_sword", WEAPON_SHORT_SWORD, 	TRUE},
    {"long_sword", 	WEAPON_LONG_SWORD, 		TRUE},
    {"dagger", 		WEAPON_DAGGER, 			TRUE},
    {"spear", 		WEAPON_SPEAR, 			TRUE},
    {"staff", 		WEAPON_STAFF, 			TRUE},	
    {"mace", 		WEAPON_MACE, 			TRUE},
    {"axe", 		WEAPON_AXE, 			TRUE},
    {"flail", 		WEAPON_FLAIL, 			TRUE},
    {"whip", 		WEAPON_WHIP, 			TRUE},
    {"polearm", 	WEAPON_POLEARM, 		TRUE},
	{"bow",			WEAPON_BOW,				TRUE},
	{"crossbow", 	WEAPON_CROSSBOW,		TRUE},	
    {NULL, 0, 0}
};


const struct flag_type weapon_type2[] = {
    {"flaming", WEAPON_FLAMING, TRUE},
    {"frost", WEAPON_FROST, TRUE},
    {"vampiric", WEAPON_VAMPIRIC, TRUE},
    {"sharp", WEAPON_SHARP, TRUE},
    {"vorpal", WEAPON_VORPAL, TRUE},
    {"twohands", WEAPON_TWO_HANDS, TRUE},
    {"shocking", WEAPON_SHOCKING, TRUE},
    {"poison", WEAPON_POISON, TRUE},
	//{"light",	WEAPON_LIGHT,	TRUE},
	{"light_dam",  WEAPON_LIGHT_DAM, TRUE},
	{"negative_dam",  WEAPON_NEGATIVE_DAM, TRUE},
	{"fire_dam",  WEAPON_FIRE_DAM, TRUE},
	{"cold_dam",  WEAPON_COLD_DAM, TRUE},
	{"lightning_dam",  WEAPON_LIGHTNING_DAM, TRUE},	
	{"air_dam",  WEAPON_AIR_DAM, TRUE},
	{"earth_dam",  WEAPON_EARTH_DAM, TRUE},
	{"holy_dam",  WEAPON_HOLY_DAM, TRUE},
	{"energy_dam",  WEAPON_ENERGY_DAM, TRUE},	
	{"water_dam",  WEAPON_WATER_DAM, TRUE},
    {NULL, 0, 0}
};

const struct flag_type res_flags[] = {
    {"summon", RES_SUMMON, TRUE},
    {"charm", RES_CHARM, TRUE},
    {"magic", RES_MAGIC, TRUE},
    {"weapon", RES_WEAPON, TRUE},
    {"bash", RES_BASH, TRUE},
    {"pierce", RES_PIERCE, TRUE},
    {"slash", RES_SLASH, TRUE},
    {"fire", RES_FIRE, TRUE},
    {"cold", RES_COLD, TRUE},
	{"light", RES_LIGHT, TRUE},
    {"lightning", RES_LIGHTNING, TRUE},
    {"acid", RES_ACID, TRUE},
    {"poison", RES_POISON, TRUE},
    {"negative", RES_NEGATIVE, TRUE},
    {"holy", RES_HOLY, TRUE},
    {"energy", RES_ENERGY, TRUE},
    {"mental", RES_MENTAL, TRUE},
    {"disease", RES_DISEASE, TRUE},
    {"drowning", RES_DROWNING, TRUE},    
    {"sound", RES_SOUND, TRUE},
    {"wood", RES_WOOD, TRUE},
    {"silver", RES_SILVER, TRUE},
    {"iron", RES_IRON, TRUE},
	{"earth", RES_EARTH, TRUE},
	{"air", RES_AIR, TRUE},
    {NULL, 0, 0}
};


const struct flag_type vuln_flags[] = {
    {"summon", VULN_SUMMON, TRUE},
    {"charm", VULN_CHARM, TRUE},
    {"magic", VULN_MAGIC, TRUE},
    {"weapon", VULN_WEAPON, TRUE},
    {"bash", VULN_BASH, TRUE},
    {"pierce", VULN_PIERCE, TRUE},
    {"slash", VULN_SLASH, TRUE},
    {"fire", VULN_FIRE, TRUE},
    {"cold", VULN_COLD, TRUE},
	{"light", VULN_LIGHT, TRUE},
    {"lightning", VULN_LIGHTNING, TRUE},
    {"acid", VULN_ACID, TRUE},
    {"poison", VULN_POISON, TRUE},
    {"negative", VULN_NEGATIVE, TRUE},
    {"holy", VULN_HOLY, TRUE},
    {"energy", VULN_ENERGY, TRUE},
    {"mental", VULN_MENTAL, TRUE},
    {"disease", VULN_DISEASE, TRUE},
    {"drowning", VULN_DROWNING, TRUE},    
    {"sound", VULN_SOUND, TRUE},
    {"wood", VULN_WOOD, TRUE},
    {"silver", VULN_SILVER, TRUE},
    {"iron", VULN_IRON, TRUE},
	{"earth", VULN_EARTH, TRUE},
	{"air", VULN_AIR, TRUE},
    {NULL, 0, 0}
};

const struct flag_type position_flags[] = {
    {"dead", POS_DEAD, FALSE},
    {"mortal", POS_MORTAL, FALSE},
    {"incap", POS_INCAP, FALSE},
    {"stunned", POS_STUNNED, FALSE},
    {"sleeping", POS_SLEEPING, TRUE},
    {"resting", POS_RESTING, TRUE},
    {"sitting", POS_SITTING, TRUE},
    {"fighting", POS_FIGHTING, FALSE},
    {"standing", POS_STANDING, TRUE},
	{"mounted",	POS_MOUNTED,	FALSE},
    {NULL, 0, 0}
};

const struct flag_type portal_flags[] = {
    {"normal_exit", GATE_NORMAL_EXIT, TRUE},
    {"no_curse", GATE_NOCURSE, TRUE},
    {"go_with", GATE_GOWITH, TRUE},
    {"buggy", GATE_BUGGY, TRUE},
    {"random", GATE_RANDOM, TRUE},
    {NULL, 0, 0}
};

const struct flag_type furniture_flags[] = {
    {"stand_at", STAND_AT, TRUE},
    {"stand_on", STAND_ON, TRUE},
    {"stand_in", STAND_IN, TRUE},
    {"sit_at", SIT_AT, TRUE},
    {"sit_on", SIT_ON, TRUE},
    {"sit_in", SIT_IN, TRUE},
    {"rest_at", REST_AT, TRUE},
    {"rest_on", REST_ON, TRUE},
    {"rest_in", REST_IN, TRUE},
    {"sleep_at", SLEEP_AT, TRUE},
    {"sleep_on", SLEEP_ON, TRUE},
    {"sleep_in", SLEEP_IN, TRUE},
    {"put_at", PUT_AT, TRUE},
    {"put_on", PUT_ON, TRUE},
    {"put_in", PUT_IN, TRUE},
    {"put_inside", PUT_INSIDE, TRUE},
    {NULL, 0, 0}
};

const struct flag_type apply_types[] = {
    {"affects", TO_AFFECTS, TRUE},
    {"object", TO_OBJECT, TRUE},
    {"immune", TO_IMMUNE, TRUE},
    {"resist", TO_RESIST, TRUE},
    {"vuln", TO_VULN, TRUE},
    {"weapon", TO_WEAPON, TRUE},
    {NULL, 0, TRUE}
};

const struct bit_type bitvector_type[] = {
    {affect_flags, "affect"},
    {apply_flags, "apply"},
    {imm_flags, "imm"},
    {res_flags, "res"},
    {vuln_flags, "vuln"},
    {weapon_type2, "weapon"}
};


const struct flag_type oprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {   "greet",		TRIG_GRALL,		TRUE	},
    {	"random",		TRIG_RANDOM,		TRUE	},
    {   "speech",		TRIG_SPEECH,		TRUE	},
    {	"exall",		TRIG_EXALL,		TRUE	},
    {	"delay",		TRIG_DELAY,		TRUE	},
    {	"drop",		TRIG_DROP,		TRUE	},
    {	"get",		TRIG_GET,		TRUE	},
    {	"sit",		TRIG_SIT,		TRUE	},
    {	NULL,			0,			TRUE	},
};
 
const struct flag_type rprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"drop",			TRIG_DROP,		TRUE	},
    {	"greet",		TRIG_GRALL,		TRUE	},
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exall",		TRIG_EXALL,		TRUE	},
    {	"delay",		TRIG_DELAY,		TRUE	},
    {	NULL,			0,			TRUE	},
};

const struct herb_type herb_table[] =
{
	{	"catnip",			VERY_COMMON,	GRASS	},	
	{	"dandelion",		VERY_COMMON,	GRASS	},
	{	"basil",			COMMON,			GRASS	},
	{	"daffodil",			COMMON,			GRASS	},	
	{	"marigold",			COMMON,			GRASS	},	
	{	"adderstongue",		UNCOMMON,		GRASS	},		//5	
	{	"angelica",			UNCOMMON,		GRASS	},
	{	"aster",			UNCOMMON,		GRASS	},	
	{	"saffron",			UNCOMMON,		GRASS	},
	{	"sage",				UNCOMMON,		GRASS	},
	{	"jasmine",			UNCOMMON,		GRASS	},		//10
	{	"henbane",			RARE,			GRASS	},
	
	
	{	"comfrey",			COMMON,			WATER	},		//12
	{	"agrimony",			UNCOMMON,		WATER	},		
	{	"butterbur",		UNCOMMON,		WATER	},	
	{	"coltsfoot",		UNCOMMON,		WATER	},		//15
	
	
	{	"birch",			COMMON,			FOREST	},
	{	"blackberry",		COMMON,			FOREST	},
	{	"camomile",			COMMON,			FOREST	},		
	{	"campion",			COMMON,			FOREST	},
	{	"cowslip",			COMMON,			FOREST	},		//20
	{	"fern",				COMMON,			FOREST	},
	{	"amaranth",			UNCOMMON,		FOREST	},
	{	"anemone",			UNCOMMON,		FOREST	},
	{	"holly",			UNCOMMON,		FOREST	},	
	{	"laurel",			UNCOMMON,		FOREST	},		//25
	{	"marjoram",			UNCOMMON,		FOREST	},
	{	"mistletoe",		UNCOMMON,		FOREST	},	
	{	"peachtree",		UNCOMMON,		FOREST	},		
	{	"euphorbia",		RARE,			FOREST	},	
	{	"hellebore",		RARE,			FOREST	},		//30
	{	"moonwort",			RARE,			FOREST	},	
	{	"mullein",			RARE,			FOREST	},
	
	
	{	"chickweed",		COMMON,			SWAMP	},	
	{	"hoarhound",		UNCOMMON,		SWAMP	},
	{	"hemlock",			RARE,			SWAMP	},		//35
	{	"quince",			RARE,			SWAMP	},
	{	"belladonna",		VERY_RARE,		SWAMP	},		
	{	"stinking arrach",	VERY_RARE,		SWAMP	},		
	
	
	{	"cactus",			VERY_COMMON,	DESERT	},		
	{	"sandalwood",		RARE,			DESERT	},		//40
	
	
	{	"elderberry",		COMMON,			HILL	},
	{	"calamint",			COMMON,			HILL	},		
	{	"fennel",			COMMON,			HILL	},		
	{	"goldenrod",		COMMON,			HILL	},	
	{	"stickwort",		COMMON,			HILL	},	
	{	"milkweed",			COMMON,			HILL	},		//45
	{	"figwort",			UNCOMMON,		HILL	},		
		
	
};

const struct resource_data mat_table [] = {
	//material type, cost (in silver) per pound, rarity, bulk, is_metal?, is_cloth?, is_wood?, is_gem?, durability, bouyant?
	//metals	
	{"iron",		30,		VERY_COMMON,		5,		TRUE,	FALSE,	FALSE,	FALSE,		75, FALSE},
	{"brass",		40,		COMMON,				2,		TRUE,	FALSE,	FALSE,	FALSE,		65, FALSE},
	{"bronze",		40,		COMMON,				3,		TRUE,	FALSE,	FALSE,	FALSE,		70, FALSE},
	{"steel",		50,		COMMON,				4,		TRUE,	FALSE,	FALSE,	FALSE,		85, FALSE},
	{"copper",		60,		COMMON,				3,		TRUE,	FALSE,	FALSE,	FALSE,		75, FALSE},
	{"silver", 		100,	UNCOMMON,			5,		TRUE,	FALSE,	FALSE,	FALSE,		80, FALSE}, //5
	{"gold",		200,	UNCOMMON,			5,		TRUE,	FALSE,	FALSE,	FALSE,		65, FALSE},
	{"platinum",	250,	RARE,				5,		TRUE,	FALSE,	FALSE,	FALSE,		90, FALSE},
	{"mithril",		350,	VERY_RARE,			2,		TRUE,	FALSE,	FALSE,	FALSE,		85, TRUE},
	{"adamantite",	500,	VERY_RARE,			2,		TRUE,	FALSE,	FALSE,	FALSE,		95, TRUE}, 	
	//cloths
	{"cotton", 		25,		VERY_COMMON,		0,		FALSE,	TRUE,	FALSE,	FALSE,		15, FALSE}, //10	
	{"leather",		50,		VERY_COMMON,		1,		FALSE,	TRUE,	FALSE,	FALSE,		30, FALSE},
	{"denim", 		80,		VERY_COMMON,		0,		FALSE,	TRUE,	FALSE,	FALSE,		25, FALSE},
	{"wool",		30,		COMMON,				0,		FALSE,	TRUE,	FALSE,	FALSE,		20, FALSE},
	{"velvet",		150,	UNCOMMON,			0,		FALSE,	TRUE,	FALSE,	FALSE,		20, FALSE}, 
	{"hide",		120,	UNCOMMON,			1,		FALSE,	TRUE,	FALSE,	FALSE,		35, FALSE}, //15
	{"silk",		250,	RARE,				0,		FALSE,	TRUE,	FALSE,	FALSE,		20, FALSE},
	//gems
	{"garnet", 		50,		COMMON,				0,		FALSE,	FALSE,	FALSE,	TRUE,		65, FALSE}, 
	{"onyx", 		100,	UNCOMMON,			0,		FALSE,	FALSE,	FALSE,	TRUE,		70, FALSE}, 
	{"topaz",		125,	UNCOMMON,			0,		FALSE,	FALSE,	FALSE,	TRUE,		60, FALSE},
	{"aquamarine",	150,	UNCOMMON,			0,		FALSE,	FALSE,	FALSE,	TRUE,		70, TRUE}, //20
	{"emerald",		500,	RARE,				0,		FALSE,	FALSE,	FALSE,	TRUE,		50, FALSE}, 
	{"ruby",		350,	RARE,				0,		FALSE,	FALSE,	FALSE,	TRUE,		55, FALSE},
	{"amethyst",	250,	RARE,				0,		FALSE,	FALSE,	FALSE,	TRUE,		65, FALSE},
	{"obsidian",	350,	RARE,				0,		FALSE,	FALSE,	FALSE,	TRUE,		85, FALSE},
	{"diamond",		1000,	VERY_RARE,			0,		FALSE,	FALSE,	FALSE,	TRUE,		90, FALSE}, //25
	
	//woods
	{"ash",			25,		UNCOMMON,			2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"crabapple",	25,		COMMON,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"aspen",		100,	UNCOMMON,			2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"maple",		150,	UNCOMMON,			2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"red oak",		350,	RARE,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"willow",		10,		UNCOMMON,			2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"cottonwood",	20,		COMMON,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"elm",			10,		COMMON,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"white oak",	250,	RARE,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"black oak",	400,	VERY_RARE,			2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"cherry",		300,	RARE,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"birch",		200,	UNCOMMON,			2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"pine",		20,		COMMON,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"cedar",		50,		COMMON,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
	{"spruce",		30,		COMMON,				2,		FALSE,  FALSE,  TRUE,	FALSE,		80,	TRUE},
		
	
	//other
	{"glass",		50,		VERY_COMMON,		0,		FALSE,  FALSE,  FALSE,	TRUE,		5,  FALSE},
	{"clay",		50,		VERY_COMMON,		0,		FALSE,	FALSE,  FALSE,  FALSE,		10, FALSE},
	{"sandstone",	50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		35, FALSE},
	{"wood",		50,		VERY_COMMON,		2,		FALSE,	FALSE,	TRUE,	FALSE,		40, TRUE},
	{"ivory",		50,		VERY_COMMON,		1,		FALSE,	FALSE,	FALSE,	TRUE,		55, TRUE}, //30
	{"wax",			50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		10, FALSE},
	{"stone", 		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		50, FALSE},
	{"dragon hide",	50,		VERY_RARE,			1,		FALSE,	TRUE,	FALSE,	FALSE,		50, FALSE}, 
	{"brick", 		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		60, FALSE},
	{"mortar",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		55, FALSE}, //35
	{"dirt",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		1, 	FALSE},
	{"topsoil",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		1, 	FALSE}, 
	{"flesh",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		10, FALSE},
	{"dust",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		1, 	FALSE},
	{"blood",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE}, //40
	{"twine",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		2, 	FALSE}, 
	{"liquid",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	TRUE},
	{"lava", 		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE}, 
	{"paper",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		1, 	FALSE},
	{"ink",			50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE},//45
	{"shell",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		15, FALSE}, 
	{"gas",			50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE},
	{"bone",		50,		VERY_COMMON,		1,		TRUE,	FALSE,	FALSE,	TRUE,		55, FALSE},
	{"energy",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	TRUE},
	{"magic",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	TRUE},//50
	{"fire",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE},
	{"ice",			50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE},
	{"mercury",		50,		VERY_COMMON,		0,		FALSE,	FALSE,	FALSE,	FALSE,		0, 	FALSE},
	{"skin",		50,		VERY_COMMON,		1,		FALSE,	TRUE,	FALSE,	FALSE,		20, FALSE},
	{NULL, 0, 0, 0, FALSE, FALSE, FALSE, FALSE, 0, FALSE}
};



const struct mob_stat_type mob_stat_table [] =
{
// 	hp,	mana,	dam,	hit, 	ac

    { 	4,	5,	1,	-1,	100}, // 0
    { 	5,	10,	2,	-1,	100},
    { 	7,	20,	2,	-1,	90},
    { 	13,	40,	3,	0,	80},
    { 	18,	60,	3,	1,	70},
    { 	22,	80,	4,	1,	70}, // 5
    { 	30,	100,	4,	1,	60},
    { 	34,	100,	5,	2,	60},
    { 	40,	100,	5,	2,	50},
    { 	55,	100,	6,	3,	50},
    { 	65,	100,	6,	3,	40}, // 10
    { 	85,	120,	7,	3,	40},
    { 	105,	140,	7,	4,	30},
    { 	120,	160,	8,	4,	30},
    { 	150,	180,	8,	4,	30},
    { 	180,	200,	9,	4,	20},
    { 	225,	200,	9,	5,	10},
    { 	250,	200,	10,	5,	0},
    { 	275,	200,	10,	5,	-10},
    { 	310,	220,	11,	5,	-20},
    { 	350,	240,	11,	6,	-30}, // 20
    { 	420,	250,	12,	6,	-40},
    { 	490,	260,	12,	6,	-50},
    { 	560,	270,	13,	6,	-60},
    { 	650,	280,	13,	7,	-70},
    { 	730,	290,	14,	7,	-80},
    { 	810,	300,	14,	7,	-85},
    { 	890,	300,	15,	7,	-90},
    { 	970,	300,	15,	8,	-95},
    { 	1050,	300,	16,	8,	-100},
    { 	1150,	350,	16,	8,	-100}, // 30
    { 	1250,	400,	17,	8,	-105},
    { 	1350,	400,	17,	8,	-110},
    { 	1500,	400,	18,	9,	-115},
    { 	1750,	500,	18,	9,	-120},
    { 	2000,	500,	19,	9,	-130},
    { 	2250,	600,	19,	10,	-140},
    { 	2500,	600,	20,	10,	-150},
    { 	2750,	700,	21,	11,	-160},
    { 	3000,	800,	22,	11,	-170},
    { 	3500,	800,	22,	11,	-180}, // 40
    { 	4000,	900,	23,	11,	-190},
    { 	4500,	900,	23,	12,	-210},
    { 	5000,	900,	24,	12,	-220},
    { 	5500,	950,	24,	12,	-240},
    { 	6000,	1000,	25,	13,	-250},
    { 	6500,	1200,	25,	13,	-260},
    { 	7000,	1300,	26,	14,	-270},
    { 	7500,	1400,	27,	14,	-285},
    { 	8000,	1500,	28,	14,	-300},
    { 	9000,	1500,	29,	15,	-310}, // 50
	{ 	10000,	1500,	30,	15,	-330},
    { 	11000,	2000,	30,	15,	-330},
    { 	12000,	2000,	31,	15,	-330},
    { 	13000,	2000,	31,	16,	-330},
    { 	15000,	2500,	32,	16,	-330},
    { 	17000,	2500,	32,	16,	-330},
    { 	19000,	2500,	33,	16,	-330},
    { 	21000,	2500,	33,	17,	-330},
    { 	23000,	3500,	33,	17,	-330},
    { 	25000,	3500,	34,	17,	-330}, // 60
	{ 	28000,	3500,	34,	17,	-330},
    { 	31000,	3500,	34,	18,	-330},
    { 	34000,	4500,	35,	18,	-330},
    { 	37000,	4500,	35,	18,	-330},
    { 	40000,	4500,	36,	18,	-330},
    { 	45000,	4500,	36,	19,	-330},
    { 	50000,	5500,	37,	19,	-330},
    { 	55000,	5500,	37,	19,	-330},
    { 	60000,	5500,	38,	19,	-330},
    { 	65000,	5500,	38,	20,	-360}, // 70
	{ 	70000,	6500,	39,	20,	-360},
    { 	75000,	6500,	39,	20,	-360},
    { 	80000,	6500,	40,	20,	-360},
    { 	85000,	6500,	40,	21,	-360},
    { 	90000,	7500,	40,	21,	-360},
    {   95000,	7500,	41,	21,	-360},
    { 	100000,	7500,	41,	21,	-360},
    { 	105000,	7500,	42,	21,	-360},
    { 	110000,	8500,	42,	22,	-360},
    { 	115000,	8500,	43,	22,	-400}, // 80
	{ 	120000,	8500,	43,	22,	-400},
    { 	125000,	8500,	44,	23,	-400},
    { 	130000,	9500,	44,	23,	-400},
    { 	135000,	9500,	45,	23,	-400},
    { 	140000,	9500,	45,	23,	-400},
    { 	145000,	9500,	46,	24,	-400},
    { 	150000,	10500,	46,	24,	-400},
    { 	160000,	10500,	46,	25,	-400},
    { 	170000,	10500,	47,	25,	-400},
    { 	180000,	10500,	47,	26,	-450}, // 90
	{ 	190000,	20500,	47,	26,	-450},
    { 	200000,	20500,	48,	26,	-450},
    { 	225000,	20500,	48,	27,	-450},
    { 	250000,	20500,	48,	27,	-450},
    { 	275000,	20500,	49,	28,	-450},
    { 	300000,	20500,	49,	28,	-450},
    { 	325000,	20500,	49,	29,	-450},
    { 	350000,	25500,	49,	29,	-450},
    { 	375000,	25500,	49,	30,	-450},
    { 	400000,	25500,	50,	30,	-450}, // 100
    { 	0,	0,	0,	0,	0}
};


const struct craft_data craft_table [] = 
{
	//type, ITEM_TYPE, can use gem?, can use_cloth?, can_use_wood?, can use metal?, mat requirement
	{ "sword", 	ITEM_WEAPON, 	TRUE, 	FALSE,	FALSE,	TRUE,	50 },
	
	{ "",		0,				0,		0,		0,		0,		0, }
};


const struct enchant_type enchant_table [] =
{
	//name, apply type, apply amount, slot.
	{ "enchant weapon: strength",		APPLY_STR,			1,		ITEM_WIELD },
	{ "enchant weapon: dexterity",		APPLY_DEX,			1,		ITEM_WIELD },
	{ "enchant weapon: damage",			APPLY_DAMROLL,		1,		ITEM_WIELD },	
	{ "enchant weapon: melee_crit",		APPLY_MELEE_CRIT,	1,		ITEM_WIELD },
	{ "enchant weapon: spell_crit",		APPLY_SPELL_CRIT,	1,		ITEM_WIELD },
	{ "enchant weapon: spell_damage",	APPLY_SPELL_DAM,	1,		ITEM_WIELD },
	{ "enchant weapon: flaming",		APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: drowning",		APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: chilling",		APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: shocking",		APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: energy",			APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: negative",		APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: holy",			APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: light",			APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: earth",			APPLY_NONE,			1,		ITEM_WIELD },
	{ "enchant weapon: air",			APPLY_NONE,			1,		ITEM_WIELD },
		
	{"",	0,	0,	0 }
};




// list of elemental specializations for wiz's/necro's (Upro 7/17/07)
const struct elemSpec specTable [8] =
{
    { "",		 	DAM_NONE},
    { "fire",		DAM_FIRE},
    { "water",		DAM_DROWNING},    
    { "shadow",		DAM_NEGATIVE},
    { "light",		DAM_LIGHT},
    { "ice",		DAM_COLD},
    { "lightning",	DAM_LIGHTNING},
    { NULL, 		DAM_NONE}
};

// list of weapon specs. 10/25/2009
const struct wpnSpec wpnSpecTable [14] =
{ 
    { "exotic",		WEAPON_EXOTIC},
    { "short sword",WEAPON_SHORT_SWORD},    
    { "long sword", WEAPON_LONG_SWORD},    
    { "dagger",		WEAPON_DAGGER},
    { "spear",		WEAPON_SPEAR},
    { "mace",		WEAPON_MACE},
    { "axe",		WEAPON_AXE},
	{ "flail",		WEAPON_FLAIL},
    { "whip",		WEAPON_WHIP},
    { "polearm",	WEAPON_POLEARM},	
    { "staff",		WEAPON_STAFF},	
    { "bow",		WEAPON_BOW},	
    { "crossbow",	WEAPON_CROSSBOW},	
    { NULL, 		DAM_NONE}
};

const struct planes plane_table[] =
{
	{"Material",	PLANE_MATERIAL},
	{"Ethereal",	PLANE_ETHEREAL},
	{"Astral",		PLANE_ASTRAL},
	{"Shadow",		PLANE_SHADOW},
	{"Fire",		PLANE_FIRE},
	{"Water",		PLANE_WATER},
	{"Air",			PLANE_AIR},
	{"Earth",		PLANE_EARTH},
	{"Nine Hells",	PLANE_NINE_HELLS},
	{"Seven Heavens",PLANE_SEVEN_HEAVENS},
	{NULL}
};

const struct resists resist_table[MAX_PC_RACE] =
{
		//- = resist
		//+ = vuln
	//Null Bash Prc  Slsh Fire Cold Ltng Acid Pois Neg  Holy Engy Mntl Dis  Drwn Lght Othr Harm Chrm Snd  Air  Earth
	//0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0}},   //null race
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0}},   //human
	{{0   ,0   ,0   ,0   ,0   ,25  ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,-25 ,0}},   //elf
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,25  ,0   ,0   ,0   ,0   ,0   ,0   ,-25}}, //dwarf
	{{0   ,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,-25 ,0   ,0   ,0   ,25  ,0   ,0   ,0   ,0   ,0   ,25  ,0   ,0   ,0}},   //half orc
	{{0   ,0   ,0   ,0   ,0   ,25  ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,0}},   //half elf
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,25  ,0   ,0   ,0   ,25  ,0   ,0   ,0}},   //halfling
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,25  ,0   ,0   ,0   ,0   ,0   ,0   ,-25}}, //gnome
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,10  ,0   ,0   ,-100,25  ,50  ,0   ,0   ,0   ,10  ,0   ,-25}}, //duergar
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,10  ,0   ,0   ,0   ,0   ,50  ,0   ,0   ,-25 ,10  ,0   ,0}},   //drow
	{{0   ,-25 ,0   ,0   ,0   ,0   ,-25  ,0   ,0  ,0   ,0   ,0   ,25  ,0   ,0   ,0   ,0   ,0   ,25  ,0   ,0   ,0}},   //half ogre
	{{0   ,0   ,0   ,0   ,25  ,-25 ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,0   ,0   ,25  ,0   ,0   ,0}},   //gnoll
	{{0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,0   ,0   ,0   ,50  ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,0}},   //minotaur
	{{0   ,0   ,0   ,0   ,25  ,0   ,25  ,0   ,0   ,0   ,0   ,0   ,10  ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,-25 ,0}},   //aarakocra
	{{0   ,0   ,0   ,0   ,25  ,-25 ,0   ,0   ,0   ,-100,50  ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-25 ,0   ,0}},   //revenant
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,50  ,-100,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,0   ,0   ,-25 ,0}},   //aasimar
	{{0   ,0   ,0   ,0   ,15  ,0   ,0   ,0   ,-25 ,0   ,0   ,0   ,0   ,-25 ,0   ,0   ,0   ,0   ,35  ,0   ,0   ,0}},   //bugbear
	{{0   ,0   ,0   ,0   ,50  ,-50 ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0}},   //tabaxi
	{{0   ,-10 ,10  ,0   ,0   ,25  ,0   ,-25 ,0   ,0   ,0   ,0   ,0   ,-100,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0}},   //lizardman
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,-10 ,0   ,0   ,0   ,25  ,0   ,-25 ,0   ,0   ,0   ,0}},   //tiefling
	{{0   ,0   ,0   ,0   ,0   ,0   ,25  ,0   ,0   ,0   ,0   ,-25 ,-25 ,0   ,0   ,0   ,0   ,0   ,0   ,50  ,0   ,0}},   //loxodon
	{{0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,25  ,-25}}  //centaur
};