#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"


int get_material			args((char *argument));
void wear_obj 		args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
void create_tree		args((ROOM_INDEX_DATA *pRoom));

#define WGT_ARMOR		5
#define WGT_WEAPON		8
#define WGT_CONTAINER	3

#define MAX_SMELL_ADJECTIVE		11
char *const smells_name[] = 
{
	"disagreeable", "flowery", "musty", "malodorous", "minty", "damp", "fishy", "rancid", "smoky", "moldy", "cheeselike"
};

#define MAX_CONSISTANCY_NAME				10
char *const consistancy_name[] = 
{
	"bubbling", "cloudy", "effervescent", "fuming", "oily", "smoky", "syrupy", "vaporous", "viscous", "watery"
};

#define MAX_TRANSPARENCY_NAME				12
char *const transparency_name[] = 
{
	"clear", "transparent", "flecked", "layered", "luminous", "opaline", "phosphorescent", "rainbowed", "ribboned", "translucent", "variegated", "opaque"
};

#define MAX_ANIMAL_NAMES 	26
char * const animal_names[] = 
{
	"fox", "hound", "whale", "deer", "pig", "hawk", "sparrow", "elephant", "horse", "lion", "tiger", "monkey", "ape", "snake", "spider",
	"human", "elf", "dwarf", "gnome", "giant", "gnoll", "goblin", "kobold", "dragon", "wyvern", "mindflayer"
};

#define MAX_PREFIX_NAME			21
char *const prefix_name[] =
{
	"masterwork", "excellent", "fine", "enchanted", "grand", "well crafted", "outstanding", "superb", "magnificent", "mighty", "supreme",
	"exquisite", "lordly", "resplendent", "majestic", "exceptional", "extraordinary", "magical",  "splendid", "masterwork", "wondrous"
};

#define MAX_SCROLL_ADJECTIVE		18
char *const scroll_adjective[] =
{
	"ancient", "worn", "tattered", "horizontal", "sacred", "sealed", "wrapped", "weathered", "vellum", "lambskin", "papyrus", "faded", "crisp", "mystic", 
	"mysterious", "scripted", "miniscule", "bloodstained"
};

#define MAX_STAT_ADJECTIVE		15
char *const stat_adjective[] = 
{
	"awesome ", "ultimate ", "optimum ", "paramount ", "amazing ", "astounding ", "astonishing ", "stunning ", "breathtaking ", "phenomenal ", "prodigious ",
	"sublime ", "remarkable ", "imposing ", "impressive "
};

#define MAX_COLOR_NAME			39
char *const color_named[] = 
{
	"blue", "red", "yellow", "green", "purple", "white", "orange", "brown", "light green", "dark green", "light blue", "dark blue", "light red", "dark red",
	"gray", "black", "silver colored", "gold colored", "copper colored", "fuchsia", "lavender", "lilac", "magenta", "plum", "amber", "crimson", "maroon", 
	"pink", "rose", "scarlet", "vermillion", "mahogany", "tan", "azure", "cerulean", "indigo", "ebony", "sable", "violet"
};


#define MAX_CONTAINER_NAME				10
char *const container_named[] = 
{
	"pouch", "satchel", "small bag", "purse", "belt pouch", "sack", "pack", "backpack", "large bag", "wrist pouch"
};

#define MAX_SHORT_SWORD_NAME 			13
#define MAX_LONG_SWORD_NAME 			12
char *const short_sword_named[] = 	
{
	"short sword", "scimitar", "epee", "saber", "brand", "blade", "gladius", "kopesh", "ninjato", "kodachi", "wakizashi", "kukri", "kopis"
};

char *const long_sword_named[] = 	
{
	"long sword", "bastard sword", "scimitar", "cutlass", "epee", "saber", "rapier", "brand", "blade", "katana", "falchion", "flamberge"
};

#define MAX_STAFF_NAME 			6
char *const staff_named[] = 	
{
	"staff", "pole", "rod", "cane", "pikestaff", "quarterstaff"
};
#define MAX_DAGGER_NAME 		8
char *const dagger_named[] = 	
{
	"dagger", "kris", "knife", "stiletto", "dirk", "skean", "karambit", "baselard"
};
#define MAX_MACE_NAME 			9
char *const mace_named[] = 	
{
	"mace", "club", "cudgel", "mallet", "hammer", "truncheon", "blackjack", "sledgehammer", "tonfa"
};
#define MAX_FLAIL_NAME 			6
char *const flail_named[] = 	
{
	"flail", "morning star", "flog", "scourge", "nunchaku", "kusarigama"
};
#define MAX_AXE_NAME 			9
char *const axe_named[] = 	
{
	"axe", "hatchet", "battle axe", "hand axe", "adze", "tomahawk", "broadaxe", "palstave", "masakari"
};
#define MAX_WHIP_NAME 			6
char *const whip_named[] = 	
{
	"whip", "bullwhip", "scourge", "knout", "lasso", "surujin"
};
#define MAX_SPEAR_NAME 			9
char *const spear_named[] = 	
{
	"spear", "halberd", "javelin", "lancet", "pike", "trident", "harpoon", "lance", "spetum"
};
#define MAX_POLEARM_NAME 		14
char *const polearm_named[] = 	
{
	"polearm", "partisan", "guisarme", "fauchard", "glaive", "voulge", "bardiche", "scythe", "halberd", "lochaber", "bill hook", "bill guisarme", "naginata", "bisento"
};

//Armor starts here

#define MAX_FINGER_NAME 		4
char *const finger_name[] = 
{
	"ring", "signet", "band", "loop"
};

#define MAX_NECK_NAME 			12
char *const neck_name[] = 
{
	"necklace", "amulet", "choker", "gorget", "mantle", "scarf", "collar", "clasp", "cloak", "cape", "shawl", "pixane"
};

#define MAX_BODY_CLOTH_NAME 		9
char *const body_cloth_name[] = 
{
	"robe", "dress", "gown", "mantle", "kimono", "blouse", "smock", "doublet", "garment"
};

#define MAX_BODY_METAL_NAME 		8
char *const body_metal_name[] = 
{
	"chest guard", "breastplate", "mail", "ringmail", "chainmail", "hauberk", "cuirass", "plackart"
};

#define MAX_HEAD_CLOTH_NAME 		5
char *const head_cloth_name[] = 
{
	"hat", "hood", "cowl", "cap", "veil"
};

#define MAX_HEAD_METAL_NAME 		7
char *const head_metal_name[] = 
{
	"helmet", "helm", "skullcap", "kabuto", "crown", "burgonet", "barbute"
};

#define MAX_LEGS_CLOTH_NAME 		8
char *const legs_cloth_name[] = 
{
	"leggings", "pants", "skirt", "stockings", "breeches", "hose", "trousers", "pantaloons"
};

#define MAX_LEGS_METAL_NAME 		6
char *const legs_metal_name[] = 
{
	"greaves", "leg guards", "hakama", "anklet", "poleyn", "tasset"
};

#define MAX_FEET_CLOTH_NAME 		5
char *const feet_cloth_name[] = 
{
	"boots", "shoes", "sandals", "slippers", "mocassins"
};

#define MAX_FEET_METAL_NAME 		3
char *const feet_metal_name[] = 
{
	"boots", "sabatons", "bases"
};

#define MAX_HANDS_CLOTH_NAME 		3
char *const hands_cloth_name[] = 
{
	"gloves", "mittens", "fingerless gloves"
};

#define MAX_HANDS_METAL_NAME 		4
char *const hands_metal_name[] = 
{
	"gloves", "gauntlets", "knuckles", "tekko"
};

#define MAX_ARMS_CLOTH_NAME 		2
char *const arms_cloth_name[] = 
{
	"sleeves", "arm guards"
};

#define MAX_ARMS_METAL_NAME 		3
char *const arms_metal_name[] = 
{
	"arm guards", "kote", "pauldrons"
};

#define MAX_SHIELD_NAME 			9
char *const shield_name[] = 
{
	"buckler", "tower shield", "shield", "round shield", "skull shield", "aegis", "rampart", "ward", "bulwark"
};

#define MAX_ABOUT_NAME 				6
char *const about_name[] = 
{
	"robe", "toga", "coat", "jacket", "petticoat" , "habit"
};

#define MAX_WAIST_CLOTH_NAME		6
char *const waist_cloth_name[] = 
{
	"belt", "cord", "waistband", "sash", "ribbon", "strap"
};

#define MAX_WAIST_METAL_NAME		2
char *const waist_metal_name[] = 
{
	"belt", "girdle"
};

#define MAX_WRIST_CLOTH_NAME 		4
char *const wrist_cloth_name[] = 
{
	"bracer", "wrist guard", "wristlet", "bangle"
};

#define MAX_WRIST_METAL_NAME 		7
char *const wrist_metal_name[] = 
{
	"bracer", "wrist guard", "wristlet", "vambrace", "bracelet", "manacle", "bangle"
};

#define MAX_FLOAT_NAME 				10
char *const float_name[] = 
{
	"gem", "spirit", "orb", "sprite", "sphere", "globe", "ball", "bauble", "ornament", "stone"
};

#define MAX_LIGHT_NAME 				5
char *const light_name[] = 
{
	"torch", "lantern", "candle", "beacon", "lamp"
};

#define MAX_GEM_ADJECTIVE			27
char *const gem_adjectives[] = 
{
	"translucent", "shiny", "dull", "brilliant", "uncut", "rough", "radiant", "fine", "sparkling", "clear", "perfect", "milky", "magnificent",
	"soft", "bright", "opaque", "ornamental", "rare", "precious", "flat", "smooth", "multifaceted", "semi-precious", "spectacular", "flawed",
	"chipped", "smoky" 
};

void create_random_obj( CHAR_DATA *mob, OBJ_DATA *container, ROOM_INDEX_DATA *room, int level, char *type, char *material, int target, char *sub_type )
{
	char prefix[MSL], output[MSL], slot[MSL], best_stat[MSL];
	char aff_name[MSL];	
	char descriptor[MSL];
	char fabric_color[MSL];	
	char gem_adj[MSL];
	OBJ_DATA *obj;
	AFFECT_DATA *paf;
	AFFECT_DATA *affect_free = NULL;
	int mod = 0;
	int value = 0, affect = 0, i = 0, num_aff = 0, seed = 0, chance = 0, bitvector = 0;
	int add_str = 0, add_dex = 0, add_int = 0, add_wis = 0, add_con = 0,
		add_mana = 0, add_hit = 0, add_move = 0, add_ac = 0, add_hr = 0,
		add_dr = 0, add_save = 0, add_best = 0, add_tot = 0, add_cha = 0,
		add_spell_crit = 0, add_melee_crit = 0, add_spell_dam = 0;
	
	int adjective = 0;
	
	
	int MAX_EFFECTS = 1;		
	bool plural = FALSE;
		
	descriptor[0] = '\0'; prefix[0] = '\0'; output[0] = '\0'; slot[0] = '\0'; best_stat[0] = '\0'; aff_name[0] = '\0';	
	seed = number_range(1,200);
	chance = 10;
	
	bzero(descriptor,'\0');
	bzero(prefix,'\0');
	bzero(output,'\0');
	bzero(slot,'\0');
	bzero(best_stat,'\0');
	bzero(aff_name,'\0');
	bzero(fabric_color,'\0');

	if (mob && IS_NPC(mob))
	{
		if( IS_SET(mob->act2,ACT2_ELITE) )
		{
			mod += 1;
			//num_aff -= 1;
			MAX_EFFECTS += 1;
			chance += 25;
		}
		if( IS_SET(mob->act2,ACT2_BOSS) )
		{
			mod += 2;
			//num_aff -= 2;
			MAX_EFFECTS += 2;
			chance += 100;
		}
	}
	else if (mob && IS_IMMORTAL(mob))
	{
		mod += 2;
		MAX_EFFECTS += 2;
		chance += 75;
	}	

	if( number_percent() >= chance )
		return;
	if (level < 5)
		return;
	obj = create_object(get_obj_index(OBJ_VNUM_PROTOTYPE),0);
	free_string(obj->material);
	obj->level = number_range((level * 8 / 10), level);
	if (obj->level > 42)
		obj->level = 42;
	SET_BIT(obj->wear_flags,ITEM_TAKE);


	//Figure out what kind of item it's going to be:
	
	if( seed <= 40 )
		obj->item_type = ITEM_WEAPON;
	else if ( (seed > 40 && seed <= 100)  )
		obj->item_type = ITEM_ARMOR;
	else if ( (seed > 100 && seed <= 120)  )
		obj->item_type = ITEM_LIGHT;
	else if ( (seed > 120 && seed <= 140) )
		obj->item_type = ITEM_GEM;
	else
		obj->item_type = ITEM_MONEY;
	
	//Overrides. For calling the function manually in game.
	if (!IS_NULLSTR(type))
	{
		if (!str_cmp(type, "weapon"))
			obj->item_type = ITEM_WEAPON;
		if (!str_cmp(type, "armor"))
			obj->item_type = ITEM_ARMOR;
		if (!str_cmp(type, "gem"))
			obj->item_type = ITEM_GEM;
		if (!str_cmp(type, "light"))
			obj->item_type = ITEM_LIGHT;
		if (!str_cmp(type, "money"))
			obj->item_type = ITEM_MONEY;	
		if (!str_cmp(type, "figurine"))
			obj->item_type = ITEM_FIGURINE;
		if (!str_cmp(type, "tree"))
			obj->item_type = ITEM_TREE;
	}
			
	if( obj->item_type == ITEM_WEAPON )
	{
		SET_BIT(obj->wear_flags,ITEM_WIELD);
		if (level <= 5)
		{
			obj->value[1] = 1;		
			obj->value[2] = number_range(4,6);
		}
		else if (level > 5 && level <= 10)
		{
			obj->value[1] = number_range(1,2);
			obj->value[2] = number_range(4,6);
		}
		else if (level > 10 && level <= 20)
		{
			obj->value[1] = 2;
			obj->value[2] = number_range(5,9);
		}
		else if (level > 20 && level <= 30)
		{
			obj->value[1] = number_range(2,3);
			obj->value[2] = number_range(6,9);
		}
		else
		{
			obj->value[1] = number_range(3,4);
			obj->value[2] = number_range(6,9);
		}

		
		if (mod > 0)
		{
			obj->value[1] += mod / 2;
			obj->value[2] += mod;
		}
		

		if (!IS_NULLSTR(material))
			obj->material = str_dup(material);
		else
			obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
		obj->weight = number_fuzzy(WGT_WEAPON);
		int wpn_type = 0;
		if (IS_NULLSTR(sub_type))
			wpn_type = number_range(1, 10);
		else 
			wpn_type = weapon_type(sub_type);
		switch (wpn_type)
		{
			default:
			case 1:
				sprintf(slot,"%s",short_sword_named[number_range(0,MAX_SHORT_SWORD_NAME-1)]); 
				obj->value[3] = DAM_SLASH;
				obj->value[0] = WEAPON_SHORT_SWORD;
				obj->weight += 1;
				break;				
			case 2:
				sprintf(slot,"%s",staff_named[number_range(0,MAX_STAFF_NAME-1)]); 
				obj->value[3] = DAM_BASH;
				obj->value[0] = WEAPON_STAFF;
				obj->weight -= number_range(1,2);
				break;
			case 3:
				sprintf(slot,"%s",dagger_named[number_range(0,MAX_DAGGER_NAME-1)]);
				obj->value[3] = DAM_PIERCE;
				obj->value[0] = WEAPON_DAGGER;
				obj->weight /= 2;
				break;
			case 4:
				sprintf(slot,"%s",mace_named[number_range(0,MAX_MACE_NAME-1)]);
				obj->value[3] = DAM_BASH;
				obj->value[0] = WEAPON_MACE;
				obj->weight += number_range(2,4);
				break;
			case 5:
				sprintf(slot,"%s",flail_named[number_range(0,MAX_FLAIL_NAME-1)]);
				obj->value[3] = DAM_BASH;
				obj->value[0] = WEAPON_FLAIL;
				break;
			case 6:
				sprintf(slot,"%s",axe_named[number_range(0,MAX_AXE_NAME-1)]);
				obj->value[3] = DAM_SLASH;
				obj->value[0] = WEAPON_AXE;
				obj->weight += number_range(2,5);
				break;
			case 7:
				sprintf(slot,"%s",whip_named[number_range(0,MAX_WHIP_NAME-1)]);
				obj->value[3] = DAM_SLASH;
				obj->value[0] = WEAPON_WHIP;
				obj->weight /= 2;
				break;
			case 8:
				sprintf(slot,"%s",spear_named[number_range(0,MAX_SPEAR_NAME-1)]);
				obj->value[3] = DAM_PIERCE;
				obj->value[0] = WEAPON_SPEAR;
				obj->weight += number_range(2,4);
				break;
			case 9:
				sprintf(slot,"%s",polearm_named[number_range(0,MAX_POLEARM_NAME-1)]);
				obj->value[3] = DAM_PIERCE;
				obj->value[0] = WEAPON_POLEARM;
				obj->weight += number_range(4,6);
				break;
			case 10:
				sprintf(slot,"%s",long_sword_named[number_range(0,MAX_LONG_SWORD_NAME-1)]); 
				obj->value[3] = DAM_SLASH;
				obj->value[0] = WEAPON_LONG_SWORD;
				obj->weight += 1;
				break;				
		}				
	}
	else if ( obj->item_type == ITEM_ARMOR )
	{
		switch( number_range(1,23) )
		{
			default: extract_obj(obj); return;
			case 1:  SET_BIT(obj->wear_flags,ITEM_WEAR_FINGER);  sprintf(slot,"%s",finger_name[number_range(0,MAX_FINGER_NAME-1)]); 
				obj->material = str_dup(mat_table[get_gem(number_range(1,4))].material);
				break;		
			case 2:  SET_BIT(obj->wear_flags,ITEM_WEAR_FINGER);  sprintf(slot,"%s",finger_name[number_range(0,MAX_FINGER_NAME-1)]); 
				obj->material = str_dup(mat_table[get_gem(number_range(1,4))].material);
				break;
			case 3:  SET_BIT(obj->wear_flags,ITEM_WEAR_NECK);    sprintf(slot,"%s",neck_name[number_range(0,MAX_NECK_NAME-1)]);       
				obj->material = str_dup(mat_table[get_gem(number_range(1,4))].material);	
				break;
			case 4:  SET_BIT(obj->wear_flags,ITEM_WEAR_NECK);    sprintf(slot,"%s",neck_name[number_range(0,MAX_NECK_NAME-1)]);       
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				break;
			case 5:  SET_BIT(obj->wear_flags,ITEM_WEAR_BODY);    sprintf(slot,"%s",body_cloth_name[number_range(0,MAX_BODY_CLOTH_NAME-1)]);       
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				break;
			case 6:  SET_BIT(obj->wear_flags,ITEM_WEAR_BODY);    sprintf(slot,"%s",body_metal_name[number_range(0,MAX_BODY_METAL_NAME-1)]);       
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				break;
			case 7:  SET_BIT(obj->wear_flags,ITEM_WEAR_HEAD);    sprintf(slot,"%s",head_cloth_name[number_range(0,MAX_HEAD_CLOTH_NAME-1)]);       
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				break;
			case 8:  SET_BIT(obj->wear_flags,ITEM_WEAR_HEAD);    sprintf(slot,"%s",head_metal_name[number_range(0,MAX_HEAD_METAL_NAME-1)]);       
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				break;
			case 9:  SET_BIT(obj->wear_flags,ITEM_WEAR_LEGS);    sprintf(slot,"%s",legs_cloth_name[number_range(0,MAX_LEGS_CLOTH_NAME-1)]);       
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				plural = TRUE;
				break;
			case 10:  SET_BIT(obj->wear_flags,ITEM_WEAR_LEGS);    sprintf(slot,"%s",legs_metal_name[number_range(0,MAX_LEGS_METAL_NAME-1)]);       
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				plural = TRUE;
				break;
			case 11:  SET_BIT(obj->wear_flags,ITEM_WEAR_FEET);    sprintf(slot,"%s",feet_cloth_name[number_range(0,MAX_FEET_CLOTH_NAME-1)]);       
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				plural = TRUE;
				break;
			case 12:  SET_BIT(obj->wear_flags,ITEM_WEAR_FEET);    sprintf(slot,"%s",feet_metal_name[number_range(0,MAX_FEET_METAL_NAME-1)]);       
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				plural = TRUE;
				break;
			case 13:  SET_BIT(obj->wear_flags,ITEM_WEAR_HANDS);   sprintf(slot,"%s",hands_cloth_name[number_range(0,MAX_HANDS_CLOTH_NAME-1)]);     
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				plural = TRUE;
				break;
			case 14:  SET_BIT(obj->wear_flags,ITEM_WEAR_HANDS);   sprintf(slot,"%s",hands_metal_name[number_range(0,MAX_HANDS_METAL_NAME-1)]);     
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				plural = TRUE;
				break;
			case 15: SET_BIT(obj->wear_flags,ITEM_WEAR_ARMS);    sprintf(slot,"%s",arms_cloth_name[number_range(0,MAX_ARMS_CLOTH_NAME-1)]);       
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				plural = TRUE;
				break;
			case 16: SET_BIT(obj->wear_flags,ITEM_WEAR_ARMS);    sprintf(slot,"%s",arms_metal_name[number_range(0,MAX_ARMS_METAL_NAME-1)]);       
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				plural = TRUE;
				break;
			case 17: SET_BIT(obj->wear_flags,ITEM_WEAR_SHIELD);  sprintf(slot,"%s",shield_name[number_range(0,MAX_SHIELD_NAME-1)]);   		
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				break;
			case 18: SET_BIT(obj->wear_flags,ITEM_WEAR_ABOUT);   sprintf(slot,"%s",about_name[number_range(0,MAX_ABOUT_NAME-1)]);     
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				break;
			case 19: SET_BIT(obj->wear_flags,ITEM_WEAR_WAIST);   sprintf(slot,"%s",waist_cloth_name[number_range(0,MAX_WAIST_CLOTH_NAME-1)]);     
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				break;
			case 20: SET_BIT(obj->wear_flags,ITEM_WEAR_WAIST);   sprintf(slot,"%s",waist_metal_name[number_range(0,MAX_WAIST_METAL_NAME-1)]);     
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				break;
			case 21: SET_BIT(obj->wear_flags,ITEM_WEAR_WRIST);   sprintf(slot,"%s",wrist_cloth_name[number_range(0,MAX_WRIST_CLOTH_NAME-1)]);     
				obj->material = str_dup(mat_table[get_cloth(number_range(0,3))].material);		
				sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
				break;
			case 22: SET_BIT(obj->wear_flags,ITEM_WEAR_WRIST);   sprintf(slot,"%s",wrist_metal_name[number_range(0,MAX_WRIST_METAL_NAME-1)]);     
				obj->material = str_dup(mat_table[get_ore(number_range(0,4))].material);
				break;
			case 23: SET_BIT(obj->wear_flags,ITEM_WEAR_FLOAT);   sprintf(slot,"%s",float_name[number_range(0,MAX_FLOAT_NAME-1)]); 
				obj->material = str_dup(mat_table[get_gem(number_range(1,4))].material);
				break;
		}
		
		if (!IS_NULLSTR(material))
			obj->material = str_dup(material);
		
		//Set bulk:
		obj->value[4] = mat_table[get_material(obj->material)].bulk;	
		//Set ac values:
		obj->value[0] = number_range((level * 0.18),(level * 0.28));
		obj->value[1] = number_range((level * 0.18),(level * 0.28));
		obj->value[2] = number_range((level * 0.18),(level * 0.28));
		obj->value[3] = number_range((level * 0.08),(level * 0.18));
		
		//Determine weight.
		int i = 0;
		
		obj->weight = number_fuzzy(WGT_ARMOR);
		
		if (obj->value[4] > 0) //bulk makes armor heavier.
			obj->weight = obj->weight * obj->value[4];

		obj->weight = number_fuzzy(obj->weight);		
		
		if( obj->value[0] < 1 )
			obj->value[0] = 1;
		if( obj->value[1] < 1 )
			obj->value[1] = 1;
		if( obj->value[2] < 1 )
			obj->value[2] = 1;
		if( obj->value[3] < 1 )
			obj->value[3] = 1;
			
		obj->value[0] += mod * 5;
		obj->value[1] += mod * 5;
		obj->value[2] += mod * 5;
		obj->value[3] += mod * 5;
	}
	else if ( obj->item_type == ITEM_MONEY )
	{
		OBJ_DATA *pouch;
		int gp = 1, sp = 1;
		
		if (level < 5)
			level = 5;
		
		if (number_percent() < 75)
			gp = number_range(level / 2, level * 3);
		
		sp = number_range(level * 2, level * 12);
		if (mod > 0)
		{
			gp *= mod;
			sp *= mod;		
		}
		
		pouch = create_object(get_obj_index(OBJ_VNUM_POUCH),0);
		pouch->material = str_dup(mat_table[get_cloth(ALL_RARITY)].material);	
		pouch->weight = number_fuzzy(WGT_CONTAINER);	
		
		//Throw some gems in bags sometimes. Upro 1/21/2020
		int gems = 0;
		gems += number_percent();
		while (gems > 24)
		{
			create_random_obj(NULL, pouch, NULL, number_fuzzy(level), "gem", mat_table[get_gem(ALL_RARITY)].material, TARG_OBJ, "");
			gems -= 25;
		}
		
		//Sometimes they can have scrolls too. Upro 1/21/2020
		int scrolls = 0;
		scrolls += number_percent();
		OBJ_DATA *sc;
		while (scrolls > 49)
		{			
			sc = create_random_consumable(0, number_fuzzy(level));
			obj_to_obj(sc, pouch);
			scrolls -= 50;
		}
		
		obj_to_obj (create_money (gp, sp), pouch);		
		obj = pouch;
		
		//Bag flags
		if (number_percent() < 10)
		{
			AFFECT_DATA af;
			
			af.where = TO_OBJECT;
			af.type = gsn_curse;
			af.level = level;
			af.duration = -1;
			af.location = APPLY_SAVES;
			af.modifier = + number_range(1,2);
			switch (number_range(1,6))
			{
				default:
				case 1:
				case 6:
					af.bitvector = ITEM_BURN_PROOF;
					sprintf(aff_name,"flame retardance");
					break;
				case 2:
					af.bitvector = ITEM_FIRE_TRAP;					
					break;
				case 3:
					af.bitvector = ITEM_GAS_TRAP;
					break;
				case 4:
					af.bitvector = ITEM_DART_TRAP;
					break;
				case 5:
					af.bitvector = ITEM_POISON_TRAP;
					break;
			}
			
			affect_to_obj (obj, &af);        
		}	
		
		int bag_type;
		
		bag_type = number_range(0,MAX_CONTAINER_NAME-1);
		
		//Where can they equip this container?
		if (bag_type == 0 || bag_type == 4)
			SET_BIT(obj->wear_flags,ITEM_WEAR_WAIST);
		else if (bag_type == 1 || bag_type == 6 || bag_type == 7)
			SET_BIT(obj->wear_flags,ITEM_WEAR_ABOUT);
		else if (bag_type == 9)
			SET_BIT(obj->wear_flags,ITEM_WEAR_WRIST);
		else
			SET_BIT(obj->wear_flags,ITEM_HOLD);
		
		sprintf(slot,"%s",container_named[bag_type]); 		
		sprintf(fabric_color,"%s",color_named[number_range(0,MAX_COLOR_NAME-1)]); 		
	}
	else if ( obj->item_type == ITEM_FIGURINE )
	{
		int sn = 0;			
		
		sn = (number_range(1,MAX_SKILL));	
		while ( skill_table[sn].spell_fun == spell_null || skill_table[sn].name == NULL || skill_table[sn].can_scribe == FALSE || skill_table[sn].spell_level > 8 )
		{			
			if (number_percent() < 60)
				sn = (number_range(1,MAX_SKILL));																	
			else
				sn = get_random_healing_spell(level);
		}		
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
			
		if (number_percent() < 40)
			SET_BIT(obj->extra_flags, ITEM_GLOW);
		if (number_percent() < 20)
			SET_BIT(obj->extra_flags, ITEM_HUM);
		if (number_percent() < 20)
			SET_BIT(obj->extra_flags, ITEM_BURN_PROOF);
		if (number_percent() < 10)
			SET_BIT(obj->extra_flags, ITEM_INDESTRUCTABLE);
		if (number_percent() < 15)
			SET_BIT(obj->extra2_flags, ITEM_NO_RECHARGE);
					
		SET_BIT(obj->wear_flags, ITEM_TAKE);
		
		obj->value[1] = number_range (4, 15);
		obj->value[2] = number_range (1, obj->value[1]);		
		obj->value[3] = sn;
		obj->cost = number_fuzzy(skill_table[sn].spell_level * number_range(250, 500));
		SET_BIT(obj->wear_flags, ITEM_HOLD);		
		obj->material = str_dup(material);
	}
	else if ( obj->item_type == ITEM_LIGHT )
	{
		if (number_percent() < 10)
			obj->value[2] = -1;
		else
			obj->value[2] = number_range(400,999); 
		
		obj->value[1] = 1; //always "enclosed" since it's magical
		
		SET_BIT(obj->wear_flags,ITEM_HOLD);
		sprintf(slot,"%s",light_name[number_range(0,MAX_LIGHT_NAME-1)]); 
		obj->material = str_dup(mat_table[get_gem(number_range(1,4))].material);
	}
	else if ( obj->item_type == ITEM_TREE )
	{
		if (room)		
			create_tree(room);
		return;		
	}
	else
	{
		//It's a gem
		SET_BIT(obj->wear_flags,ITEM_HOLD);		
		obj->material = str_dup(mat_table[get_gem(number_range(1,4))].material);
		sprintf(slot, "%s", mat_table[get_material(obj->material)].material);		
		sprintf(gem_adj, "%s", gem_adjectives[number_range(0,MAX_GEM_ADJECTIVE-1)]);
	}

	int aff_modifier = 0; //modifies how many affects are on an item.

	if (level < 20)
		aff_modifier = 35;
	else
		aff_modifier = 20;

	if (level < 5)
		MAX_EFFECTS = 1;
	else if (level > 4 && level < 10)
		MAX_EFFECTS = 2;
	else if (level > 9 && level < 20)
		MAX_EFFECTS = 3;
	else if (level > 19 && level < 30)
		MAX_EFFECTS = 4;
	else
		MAX_EFFECTS = 5;
		
	if (obj->item_type == ITEM_GEM)
		MAX_EFFECTS = 0;
		
	for( i = 0; i < MAX_EFFECTS; i++ )
	{
		if( number_percent() >= (num_aff * aff_modifier) )
		{
			int aff_type = 0;
			aff_type = number_range (1,22);
			num_aff += 1;			
			if (aff_type > 6 && aff_type < 13)
			{
				value = number_range((level / 5),(level - 10));
				add_tot += (int)(value * 1.5); //add quality for each affect.				
			}
			else 
			{
				if (level < 15)
					value = number_range(1,2);
				else if ( level > 14 && level < 25 )
					value = number_range(1,3);
				else if ( level > 24 && level < 35 )
					value = number_range(1,4);
				else
					value = number_range(2,5);
				add_tot += value * 15; //add quality for each affect.
			}
			
			if (aff_type == 20 || aff_type == 21)
			{
				value *= (level / 8);
			}
			
			if( value < 1 )
				value = 1;					
			if (level > 25)
				add_tot += 100;
			

			switch( aff_type )
			{
				 default: extract_obj(obj); return;  //Number_range seems to return out of range at
													 //random; handle it.
				 case 1:  add_str = value;         break;
				 case 2:  add_dex = value;         break;
				 case 3:  add_int = value;         break;
				 case 4:  add_wis = value;         break;
				 case 5:  add_con = value;         break;
				 case 6:  add_cha = value;			break;
				 case 7:  
				 case 8:
							add_mana = value;			break;						  
				 case 9:  
				 case 10:
							add_hit = value;         	break;
				 case 11:  
				 case 12:
							add_move = value;        	break;
				 case 13: 
				 case 14:
				 case 15:
							add_ac = (value * -1);   	break;
				 case 16: 
							add_hr = value;          	break;
				 case 17: 
							add_dr = value;          	break;
				 case 18: 
				 case 19:
							add_save = (value * -1);	break;		
				 case 20:
							add_spell_crit = value;	break;
				 case 21:
							add_melee_crit = value;	break;
				 case 22:
							add_spell_dam = value;		break;
			}
		}
	}
	
	if (number_percent() < 50)	
		adjective = number_range(0, MAX_STAT_ADJECTIVE-1);
	else 
		adjective = -1;
	
	int noun = 0;
	for( i = 0; i <= 16; i++ )
	{
		affect = APPLY_NONE;
		switch( i )
		{
			case 1: 
				if( add_str > 0 )
				{
					affect = APPLY_STR;
					value = add_str;
					add_best = add_str;
					
					
					noun = number_range(1,5);
					switch (noun)
					{
						default: sprintf(best_stat,"strength"); break;
						case 1: sprintf(best_stat,"power"); break;
						case 2: sprintf(best_stat,"might"); break;
						case 3: sprintf(best_stat,"strength"); break;
						case 4: sprintf(best_stat,"brawn"); break;
						case 5: sprintf(best_stat,"clout"); break;
					}
				}
				break;
			case 2:
				if( add_dex > 0 )
				{
					affect = APPLY_DEX;
					value = add_dex;
					if( add_dex > add_best )
					{
						add_best = add_dex;
						noun = number_range(1,7);
						switch (noun)
						{
							default: sprintf(best_stat,"dexterity"); break;
							case 1: sprintf(best_stat,"agility"); break;
							case 2: sprintf(best_stat,"grace"); break;
							case 3: sprintf(best_stat,"celerity"); break;
							case 4: sprintf(best_stat,"dexterity"); break;
							case 5: sprintf(best_stat,"nimbleness"); break;
							case 6: sprintf(best_stat,"deftness"); break;
							case 7: sprintf(best_stat,"finesse"); break;
						}
					}
				}
				break;
			case 3:
				if( add_int > 0 )
				{
					affect = APPLY_INT;
					value = add_int;
					if( add_int > add_best )
					{
						add_best = add_int;
						noun = number_range(1,8);
						switch (noun)
						{
							default: sprintf(best_stat,"intellect"); break;
							case 1: sprintf(best_stat,"prowess"); break;
							case 2: sprintf(best_stat,"cunning"); break;
							case 3: sprintf(best_stat,"intellect"); break;
							case 4: sprintf(best_stat,"intelligence"); break;
							case 5: sprintf(best_stat,"aptitude"); break;
							case 6: sprintf(best_stat,"brilliance"); break;
							case 7: sprintf(best_stat,"sagacity"); break;
							case 8: sprintf(best_stat,"knowledge"); break;
						}
					}
				}
				break;
			case 4:
				if( add_wis > 0 )
				{
					affect = APPLY_WIS;
					value = add_wis;
					if( add_wis > add_best )
					{
						add_best = add_wis;
						noun = number_range(1,6);
						switch (noun)
						{
							default: sprintf(best_stat,"wisdom"); break;
							case 1: sprintf(best_stat,"acumen"); break;
							case 2: sprintf(best_stat,"enlightenment"); break;
							case 3: sprintf(best_stat,"wisdom"); break;
							case 4: sprintf(best_stat,"judgement"); break;
							case 5: sprintf(best_stat,"erudition"); break;
							case 6: sprintf(best_stat,"reason"); break;
						}
					}
				}
				break;
			case 5:
				if( add_con > 0 )
				{
					affect = APPLY_CON;
					value = add_con;
					if( add_con > add_best )
					{
						add_best = add_con;
						noun = number_range(1,6);
						switch (noun)
						{
							default: sprintf(best_stat,"constitution"); break;
							case 1: sprintf(best_stat,"vitality"); break;
							case 2: sprintf(best_stat,"vigor"); break;
							case 3: sprintf(best_stat,"constitution"); break;
							case 4: sprintf(best_stat,"robustness"); break;
							case 5: sprintf(best_stat,"endurance"); break;
							case 6: sprintf(best_stat,"fortitude"); break;
						}
					}
				}
				break;
			case 6:
				if( add_cha > 0 )
				{
					affect = APPLY_CHA;
					value = add_cha;
					if( add_cha > add_best )
					{
						add_best = add_cha;
						noun = number_range(1,6);
						switch (noun)
						{
							default: sprintf(best_stat,"charisma"); break;
							case 1: sprintf(best_stat,"charm"); break;
							case 2: sprintf(best_stat,"appeal"); break;
							case 3: sprintf(best_stat,"charisma"); break;
							case 4: sprintf(best_stat,"allure"); break;
							case 5: sprintf(best_stat,"glamour"); break;
							case 6: sprintf(best_stat,"beauty"); break;
						}
					}
				}     
				break;
			case 7:
				if( add_mana > 0 )
				{
					affect = APPLY_MANA;
					value = add_mana;
					if( add_mana > add_best )
					{
						add_best = add_mana;
						noun = number_range(1,6);
						switch (noun)
						{
							default: sprintf(best_stat,"mana"); break;
							case 1: sprintf(best_stat,"magic"); break;
							case 2: sprintf(best_stat,"witchcraft"); break;
							case 3: sprintf(best_stat,"mana"); break;
							case 4: sprintf(best_stat,"augury"); break;
							case 5: sprintf(best_stat,"wizardry"); break;
							case 6: sprintf(best_stat,"conjury"); break;
						}
					}
				}
				break;
			case 8:
				if( add_hit > 0 )
				{
					affect = APPLY_HIT;
					value = add_hit;
					if( add_hit > add_best )
					{
						add_best = add_hit;
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"health"); break;
							case 1: sprintf(best_stat,"longevity"); break;
							case 2: sprintf(best_stat,"toughness"); break;
							case 3: sprintf(best_stat,"health"); break;
							case 4: sprintf(best_stat,"mettle"); break;
							case 5: sprintf(best_stat,"hardiness"); break;
						}
					}
				}
				break;
			case 9:
				if( add_move > 0 )
				{
					affect = APPLY_MOVE;
					value = add_move;
					if( add_move > add_best )
					{
						add_best = add_move;
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"movement"); break;
							case 1: sprintf(best_stat,"longwalking"); break;
							case 2: sprintf(best_stat,"striding"); break;
							case 3: sprintf(best_stat,"movement"); break;
							case 4: sprintf(best_stat,"voyaging"); break;
							case 5: sprintf(best_stat,"wandering"); break;
						}
					}
				}
				break;
			case 10:
				if( add_ac < 0 )
				{
					affect = APPLY_AC;
					value = add_ac;
					if( (add_ac * -1) > add_best )
					{
						add_best = (add_ac * -1);
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"protection"); break;
							case 1: sprintf(best_stat,"armor"); break;
							case 2: sprintf(best_stat,"deflection"); break;
							case 3: sprintf(best_stat,"protection"); break;
							case 4: sprintf(best_stat,"defense"); break;
							case 5: sprintf(best_stat,"security"); break;
						}
					}
				}
				break;
			case 11:
				if( add_hr > 0 )
				{
					affect = APPLY_HITROLL;
					value = add_hr;
					if( add_hr > add_best )
					{
						add_best = add_hr;
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"striking"); break;
							case 1: sprintf(best_stat,"accuracy"); break;
							case 2: sprintf(best_stat,"acuteness"); break;
							case 3: sprintf(best_stat,"striking"); break;
							case 4: sprintf(best_stat,"sureness"); break;
							case 5: sprintf(best_stat,"exactitude"); break;
						}
					}
				}
				break;
			case 12:
				if( add_dr > 0 )
				{
					affect = APPLY_DAMROLL;
					value = add_dr;
					if( add_dr > add_best )
					{
						add_best = add_dr;
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"damage"); break;
							case 1: sprintf(best_stat,"ferocity"); break;
							case 2: sprintf(best_stat,"pain"); break;
							case 3: sprintf(best_stat,"damage"); break;
							case 4: sprintf(best_stat,"tormenting"); break;
							case 5: sprintf(best_stat,"injury"); break;
						}
					}
				}
				break;
			case 13:
				if( add_save < 0 )
				{
					affect = APPLY_SAVING_SPELL;
					value = add_save;
					if( (add_save * -1) > add_best )
					{
						add_best = (add_save * -1);
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"resistance"); break;
							case 1: sprintf(best_stat,"rebuff"); break;
							case 2: sprintf(best_stat,"contention"); break;
							case 3: sprintf(best_stat,"resistance"); break;
							case 4: sprintf(best_stat,"rebuff"); break;
							case 5: sprintf(best_stat,"impedence"); break;
						}
					}
				}
				break;
			case 14:
				if( add_spell_crit > 0 )
				{
					affect = APPLY_SPELL_CRIT;
					value = add_spell_crit;		
					if( add_spell_crit > add_best )
					{
						add_best = (add_spell_crit);
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"penetration"); break;
							case 1: sprintf(best_stat,"force"); break;
							case 2: sprintf(best_stat,"ferocity"); break;
							case 3: sprintf(best_stat,"deadliness"); break;
							case 4: sprintf(best_stat,"inhumanity"); break;
							case 5: sprintf(best_stat,"penetration"); break;
						}
					}
				}
				break;
			case 15:
				if( add_melee_crit > 0 )
				{
					affect = APPLY_MELEE_CRIT;
					value = add_melee_crit;					
					
					if( add_melee_crit > add_best )
					{
						add_best = add_melee_crit;
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"penetration"); break;
							case 1: sprintf(best_stat,"barbarity"); break;
							case 2: sprintf(best_stat,"brutality"); break;
							case 3: sprintf(best_stat,"cruelty"); break;
							case 4: sprintf(best_stat,"violence"); break;
							case 5: sprintf(best_stat,"penetration"); break;
						}
					}
				}				
				break;			
			case 16:
				if( add_spell_dam > 0 )
				{
					affect = APPLY_SPELL_DAM;
					value = add_spell_dam;			
					
					if( add_spell_dam > add_best )
					{
						add_best = add_spell_dam;
						noun = number_range(1,5);
						switch (noun)
						{
							default: sprintf(best_stat,"damage"); break;
							case 1: sprintf(best_stat,"control"); break;
							case 2: sprintf(best_stat,"capacity"); break;
							case 3: sprintf(best_stat,"damage"); break;
							case 4: sprintf(best_stat,"dominion"); break;
							case 5: sprintf(best_stat,"potency"); break;
						}
					}
				}				
				break;			
		}	
	

		if( affect )
		{
			if( affect_free == NULL )
				paf = alloc_perm(sizeof(*paf));

				else
			{
				paf = affect_free;
				affect_free = affect_free->next;
			}

			paf->type = 0;
			paf->duration = -1;
			paf->location = affect;
			paf->modifier = value;
			paf->bitvector = 0;
			paf->next = obj->affected;
			obj->affected = paf;
		}   
	}
	
	if (add_tot > 350)
	{		
		sprintf(prefix,"%s",prefix_name[number_range(0,MAX_PREFIX_NAME-1)]);
	}
	
	if (num_aff > 0)
		SET_BIT (obj->extra_flags, ITEM_MAGIC);
	
	if( add_tot < 100 )
		obj->pIndexData->quality = QUALITY_POOR;
	else if( add_tot >= 100 && add_tot < 250 )
		obj->pIndexData->quality = QUALITY_COMMON;
	else if( add_tot >= 250 && add_tot < 450 )
		obj->pIndexData->quality = QUALITY_UNCOMMON;
	else if( add_tot >= 450 && add_tot < 550 )
		obj->pIndexData->quality = QUALITY_SUPERIOR;
	else if( add_tot >= 550 && add_tot < 725 )
	{
		obj->pIndexData->quality = QUALITY_EPIC;
		SET_BIT (obj->extra2_flags, ITEM_EPIC);
	}
	else
	{
		obj->pIndexData->quality = QUALITY_LEGENDARY;		
		SET_BIT (obj->extra2_flags, ITEM_LEGENDARY);
	}
	free_string(obj->name);
	free_string(obj->short_descr);
	free_string(obj->description);

	switch (obj->item_type)
	{
		default:
			obj->cost = (number_range(obj->level, obj->level * 2) * 40) + (num_aff * 350) + (add_tot * 50);	
			break;
			
		case (ITEM_GEM):
			obj->cost = number_range(mat_table[get_material(obj->material)].cost / 2, mat_table[get_material(obj->material)].cost * 2);
			obj->cost += number_range(obj->level * 100, obj->level * 200);
			if (mat_table[get_material(obj->material)].rarity > UNCOMMON)
				obj->cost *= 2;	
			break;
		case (ITEM_CONTAINER):
			obj->cost = obj->level * 10;
			break;
		
	}
	
	
	if (obj->item_type == ITEM_FIGURINE)
	{
		if (!plural)
		{
			if (IS_VOWEL(slot[0]))
				sprintf(descriptor, "an");
			else
				sprintf(descriptor, "a");
		}
		else
			sprintf(descriptor, "the");
	}
	else if (obj->item_type == ITEM_CONTAINER)
	{
		if (!plural)
		{
			if (IS_VOWEL(fabric_color[0]))
				sprintf(descriptor, "an");
			else
				sprintf(descriptor, "a");
		}
		else
			sprintf(descriptor, "the");
	}
	else if (obj->item_type == ITEM_GEM)
	{
		if (!plural)
		{
			if (IS_VOWEL(gem_adj[0]))
				sprintf(descriptor, "an");
			else
				sprintf(descriptor, "a");
		}
		else
			sprintf(descriptor, "the");
	}
	else
	{
		if (add_tot < 450)
		{
			if (!plural)
			{
				if (IS_VOWEL(slot[0]))
					sprintf(descriptor, "an");
				else
					sprintf(descriptor, "a");
			}		
			else
				sprintf(descriptor, "the");
		}
		else
		{
			if (!plural)
			{
				if (IS_VOWEL(prefix[0]))
					sprintf(descriptor, "an");
				else
					sprintf(descriptor, "a");
			}
			else
				sprintf(descriptor, "the");
		}
	}
	
	

	if (add_tot >= 450)
	{			
		switch (obj->item_type)
		{
			default:
				if (num_aff < 1)
				{
					sprintf(output,"%s %s %s",descriptor,prefix,slot);
					break;
				}
				if (IS_CLOTH(obj->material) && number_percent() < 50)	
					sprintf(output,"%s %s %s %s of %s%s",descriptor,prefix,fabric_color,slot,(adjective > -1) ? stat_adjective[adjective] : "", best_stat);
				else
					sprintf(output,"%s %s %s of %s%s",descriptor,prefix,slot, (adjective > -1) ? stat_adjective[adjective] : "", best_stat);
				break;
			case (ITEM_CONTAINER):		
				sprintf(output,"%s %s %s %s",descriptor,fabric_color,obj->material,slot);
				break;
			case (ITEM_GEM):
				sprintf(output,"%s %s %s",descriptor,gem_adj,slot);
				break;
			case (ITEM_FIGURINE):
				sprintf(output,"%s %s%s figurine of a %s",descriptor, IS_SET(obj->extra_flags, ITEM_GLOW) ? "glowing " : "", obj->material, animal_names[number_range(0, MAX_ANIMAL_NAMES -1)]);
				break;
		}
	}
	else
	{		
		

		switch (obj->item_type)
		{
			default:
				if (num_aff < 1)
				{
					sprintf(output,"%s %s",descriptor,slot);
					break;
				}
				if (IS_CLOTH(obj->material) && number_percent() < 50)	
					sprintf(output,"%s %s %s of %s%s",descriptor,fabric_color,slot, (adjective > -1) ? stat_adjective[adjective] : "", best_stat);		
				else
					sprintf(output,"%s %s of %s%s",descriptor,slot,(adjective > -1) ? stat_adjective[adjective] : "", best_stat);	
				break;
			case (ITEM_CONTAINER):		
				sprintf(output,"%s %s %s %s",descriptor,fabric_color,obj->material,slot);
				break;
			case (ITEM_GEM):
				sprintf(output,"%s %s %s",descriptor,gem_adj,slot);
				break;
			case (ITEM_FIGURINE):
				sprintf(output,"%s %s%s figurine of a %s", descriptor, IS_SET(obj->extra_flags, ITEM_GLOW) ? "glowing " : "", obj->material, animal_names[number_range(0, MAX_ANIMAL_NAMES -1)]);
				break;
		}
	}				

	//Is it cursed?? :D
	if (number_percent() < 4)
	{
		AFFECT_DATA af;
		
		af.where = TO_OBJECT;
        af.type = gsn_curse;
        af.level = level;
        af.duration = (number_percent() < 50 ? -1 : (3 * level));
        af.location = APPLY_SAVES;
        af.modifier = + number_range(1,2);
        switch (number_range(1,3))
		{
			default:
			case 1:
				af.bitvector = ITEM_EVIL;
				sprintf(aff_name,"evil");
				break;
			case 2:
				af.bitvector = ITEM_NOREMOVE;
				sprintf(aff_name,"stickiness");
				break;
			case 3:
				af.bitvector = ITEM_NODROP;
				sprintf(aff_name,"magnetism");
				break;
		}
		
        affect_to_obj (obj, &af);        
	}	
		
	if( bitvector > 0 )
	{
		strcat(output," and ");
		strcat(output,aff_name);
	}

	obj->short_descr = str_dup(output);
	obj->description = str_dup(output);

	if (IS_NULLSTR(obj->short_descr))
	{	
		extract_obj(obj);
		return;
	}	
	
	obj->name = str_dup(output);

	
	switch (target)
	{
		default:
			extract_obj(obj);
			break;
		case TARG_OBJ:
			if (container)
				obj_to_obj(obj, container);
			break;
		case TARG_MOB:
			if (mob)
				obj_to_char(obj,mob);
			break;
		case TARG_ROOM:
			if (room)
				obj_to_room(obj, room);
			break;
	}
	
	return;
	}

	
	
int get_random_healing_spell(int level)
{
//	int sn = 0;
	
	if (level < 8)
		return gsn_cure_light;
	else if (level >= 8 && level < 16)
		return gsn_cure_serious;
	else if (level >= 16 && level < 24)
		return gsn_cure_critical;
	else
		return gsn_heal;
	
	return -1;
}

OBJ_DATA * create_random_consumable(int type, int level)
{	
	int sn;
	char buf[MSL];
	int scroll_adj = 0;
	int smell_adj = 0;
	OBJ_DATA * random;
	
	switch (type)
	{
		default:
		case 0:
		case 1:
		{
			random = create_object (get_obj_index (OBJ_VNUM_SCROLL), 0);
			break;
		}
		case 2:				
		{
			random = create_object (get_obj_index (OBJ_VNUM_POTION), 0);
			break;
		}
		case 3:
		{
			random = create_object (get_obj_index (OBJ_VNUM_WAND), 0);
			break;
		}
	}
	
	sn = (number_range(1,MAX_SKILL));					
	while ( skill_table[sn].spell_fun == spell_null || skill_table[sn].name == NULL || skill_table[sn].can_scribe == FALSE || skill_table[sn].spell_level > 8 )
	{			
		if (number_percent() < 60)
			sn = (number_range(1,MAX_SKILL));																	
		else
			sn = get_random_healing_spell(level);
		//Running list of spells that we don't want to overpower/break the mud.
		if (random->item_type == ITEM_POTION || random->item_type == ITEM_FIGURINE)
		{
			if (sn == gsn_summon_earth || sn == gsn_summon_water || sn == gsn_summon_fire || sn == gsn_summon_air || sn == gsn_charm_person ||
				sn == gsn_animate_dead || sn == gsn_animate_shadow || sn == gsn_summon_monster ||/*|| sn == gsn_summon || sn == gsn_gate*/
				sn == gsn_enchant_armor || sn == gsn_enchant_weapon || sn == gsn_flaming_forge || sn == gsn_vampiric_forge || sn == gsn_frost_forge)
			{
				sn = (number_range(1,MAX_SKILL));
			}
		}
	}		
	
	random->level = number_fuzzy(level);			
	random->value[0] = number_fuzzy(level);
	SET_BIT(random->extra_flags, ITEM_MAGIC);
	
	if (number_percent() < 40)
		SET_BIT(random->extra_flags, ITEM_GLOW);
	if (number_percent() < 20)
		SET_BIT(random->extra_flags, ITEM_HUM);
	if (number_percent() < 20)
		SET_BIT(random->extra_flags, ITEM_BURN_PROOF);		
	if (number_percent() < 10)
		SET_BIT(random->extra_flags, ITEM_INDESTRUCTABLE);
	if (number_percent() < 15) 
		SET_BIT(random->extra2_flags, ITEM_NO_RECHARGE);
				
	SET_BIT(random->wear_flags, ITEM_TAKE);
	
	if (random->item_type == ITEM_SCROLL)
	{
		random->value[1] = (sn);		
		scroll_adj = number_range(0, MAX_SCROLL_ADJECTIVE-1);
		if (number_percent() < 50)
		{
			smell_adj = number_range(0, MAX_SMELL_ADJECTIVE-1);
			sprintf (buf, "a %s %s scroll of %s",smells_name[smell_adj], scroll_adjective[scroll_adj], skill_table[sn].name);
		}
		else		
			sprintf (buf, "a %s scroll of %s",scroll_adjective[scroll_adj], skill_table[sn].name);
		
		random->cost = number_fuzzy(level * 200);
		SET_BIT(random->wear_flags, ITEM_HOLD);
	}
	if (random->item_type == ITEM_WAND)
	{
		random->value[1] = number_range(5,20);
		random->value[2] = number_range(random->value[1] / 2, random->value[1]);
		random->value[3] = (sn);			
		random->cost = number_fuzzy(level * 250);
		switch (number_range(1,6))
		{
			default:
			case 0:
			case 1:
			{
				SET_BIT(random->wear_flags, ITEM_WEAR_FINGER);
				sprintf (buf, "a ring of %s", skill_table[sn].name);	
				break;
			}
			case 2:
			{
				SET_BIT(random->wear_flags, ITEM_WEAR_NECK);
				sprintf (buf, "a necklace of %s", skill_table[sn].name);	
				break;
			}
			case 3:
			{
				SET_BIT(random->wear_flags, ITEM_WEAR_HANDS);
				sprintf (buf, "the gloves of %s", skill_table[sn].name);	
				break;
			}
			case 4:
			{
				SET_BIT(random->wear_flags, ITEM_WEAR_WRIST);
				sprintf (buf, "bracer of %s", skill_table[sn].name);	
				break;
			}
			case 5:
			{
				SET_BIT(random->wear_flags, ITEM_HOLD);
				sprintf (buf, "a sphere of %s", skill_table[sn].name);	
				break;
			}
			case 6:
			{						
				SET_BIT(random->wear_flags, ITEM_WEAR_FLOAT);
				sprintf (buf, "a floating sphere of %s", skill_table[sn].name);	
				break;
			}
		}
	}
	if (random->item_type == ITEM_POTION)
	{
		char * consistancy;
		char * transparency;
		
		consistancy = str_dup(consistancy_name[number_range(0, MAX_CONSISTANCY_NAME -1)]);
		transparency = str_dup(transparency_name[number_range(0, MAX_TRANSPARENCY_NAME -1)]);
		
		random->value[1] = (sn);
		sprintf (buf, "a %s, %s potion of %s", transparency, consistancy, skill_table[sn].name);
		random->cost = number_fuzzy(level * 150);
		SET_BIT(random->wear_flags, ITEM_HOLD);
	}
	
	random->short_descr = str_dup(buf);
	random->name = str_dup(buf);
	random->description = str_dup (buf);
	
	
	return random;
}