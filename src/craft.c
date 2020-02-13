
//////////////////////////////////////////////////////////////////////////////////
//																				//
//																				//
//		Crafting code by Upro: 2010												//
//																			//
//////////////////////////////////////////////////////////////////////////////////

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"


int get_material		args ((char *argument));
void spell_null 	args((int sn, int level, CHAR_DATA * ch, void *vo, int target));



int what_mat_num (char * material)
{
	int i;

	for (i=0;i<MAX_MATERIAL;i++)
	{
		if (!strcmp(mat_table[i].material, material))
			return i;
	}
	return -1;
}

void do_blacksmith(CHAR_DATA *ch, char * argument)
{
	char arg1[MSL];
	char arg2[MSL];
	char arg3[MSL];
	char arg4[MSL];
	//char arg5[MSL];
	
	char buf[MSL];
	OBJ_DATA * hammer = NULL;
	OBJ_DATA * forge = NULL;	//weapon/armor making.
	OBJ_DATA *obj;
	int chance = 0;		
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	//argument = one_argument( argument, arg5 );
	
	if (!KNOWS(ch, gsn_blacksmithing))
	{
		SEND("You have no idea what you're doing!\r\n",ch);
		return;	
	}

	if (arg1[0] == '\0')
    {
        SEND ("Blacksmith what?\r\n", ch);
        return;
    }

    if (!str_cmp(arg1, "cost") && arg2[0] == '\0')
	{
		//leave room to specify cost per slot later on... just doing basic
		//functionality for now.
		
		SEND ("Slot:                Pounds of Ore:\r\n", ch);
		
		//List of crafted eq material costs goes here :P
		
		return;
	}
	
	if (!get_obj_list_by_type(ch, ITEM_ANVIL, ch->in_room->contents))
	{
		SEND("You need to be near a forge to do that.\r\n",ch);
		return;
	}

	if ((hammer = get_eq_char (ch, WEAR_WIELD)) == NULL || hammer->item_type != ITEM_BLACKSMITH_HAMMER)
	{
		SEND("You aren't carrying a proper hammer.\r\n",ch);
		return;
	}

	if (!str_cmp(arg1, "smelt"))
	{
		int amount = 0;
	
		if (arg2[0] == '\0')
		{
			SEND("What would you like to smelt?\r\n",ch);
			return;
		}

		if ((obj = get_obj_carry (ch, arg2, ch)) == NULL)
		{
			SEND("You aren't carrying that.\r\n",ch);
			return;
		}	
		
		if (!(mat_table[get_material(obj->material)].is_metal))
		{
			SEND("That's not made of any kind of metal.\r\n",ch);
			return;
		}
		if ((chance = number_percent()) <= get_skill(ch, gsn_blacksmithing))
		{
			amount = obj->weight / 5;
			if (obj->value[4] < 1) //gotta make up for the lack of weight for cloth
				amount *= 3;
			
			if (amount < 1)
				amount = 1;
			sprintf(buf, "You work %s on the anvil and smelt %d pounds of %s ore.\r\n", obj->short_descr, amount, obj->material);			
			SEND(buf, ch);
			ch->mats[get_material(obj->material)] += amount;
			ch->move -= number_range(10,20);
			
			int xp = 0;				
			xp = number_range(15, 30);
			if (total_levels(ch) > 5)
			{	
				xp += (total_levels(ch) / 2) * number_range(25,35);
			}
			
			give_exp(ch, xp, TRUE);
			check_improve (ch, gsn_blacksmithing, TRUE, 2);
		}
		else
		{
			sprintf(buf, "You botch the job, destroying %s.\r\n", obj->short_descr);
			check_improve (ch, gsn_blacksmithing, FALSE, 1);
			SEND (buf, ch);	
		}
		extract_obj(obj);
		return;
	}

	if (!str_cmp(arg1, "weapon"))
	{

		if (!valid_material(arg3))
		{
			SEND ("Pick a valid material first.\r\n",ch);
			return;
		}
		
		if (weapon_type(arg2) > 10 || weapon_type(arg2) < 1)
		{
			SEND ("That's not a valid weapon type.\r\n",ch);
			return;
		}

		if (atoi(arg4) < 1 || atoi(arg4) > 40)
		{
			SEND ("Pick a valid level first. (1 to 40)\r\n", ch);
			return;
		}

		if (atoi(arg4) > total_levels(ch))
		{
			SEND ("You cannot craft items higher than your level.\r\n",ch);
			return;
		}
		
		if (mat_table[get_material(arg3)].is_metal != TRUE && mat_table[get_material(arg3)].is_gem != TRUE && mat_table[get_material(arg3)].is_wood != TRUE)
		{
			SEND ("You must use metal, wood, or gems to craft swords.\r\n",ch);
			return;
		}

		int mat = what_mat_num(arg3);
		if (mat == -1)
		{
			SEND ("Not a valid material\r\n",ch);
			return;	
		}

		if (ch->mats[mat] < weapon_table[weapon_type(arg2)].material_cost)
		{
			SEND ("You don't have enough material for that.\r\n",ch);
			return;
		}

		ch->mats[mat] -= weapon_table[weapon_type(arg2)].material_cost;

		create_random_obj( ch, NULL, NULL, atoi(arg4), "weapon", arg3, TARG_MOB, arg2);		
		SEND("You create a new weapon!\r\n", ch);
		check_improve(ch,gsn_blacksmithing,TRUE,3);			
	}

	if (!str_cmp(arg1, "armor"))
	{
		SEND("Not implemented yet.\r\n",ch);
	}


	return;
}


void do_brew (CHAR_DATA *ch, char *argument)
{
	char arg1[MSL];	
	char arg2[MSL];	
	char buf[MSL];
	OBJ_DATA *ing1 = NULL, *ing2 = NULL, *ing3 = NULL;
	OBJ_DATA *flask;	
	OBJ_DATA *recipe;
	OBJ_DATA *potion;
	int chance;
	
	argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (IS_NPC(ch))
		return;

	if (!get_skill(ch, gsn_alchemy) > 1)
	{
		SEND("You have no clue what you're doing.\r\n",ch);
		return;
	}
	
	if (arg1[0] == '\0')
    {
        SEND ("Brew what?\r\n", ch);
        return;
    }
	
	if ((flask = get_obj_carry (ch, arg1, ch)) == NULL || flask->item_type != ITEM_FLASK)
	{
		SEND("You aren't carrying that flask.\r\n",ch);
		return;
	}
	
	if ((recipe = get_obj_carry (ch, arg2, ch)) == NULL || recipe->item_type != ITEM_ALCHEMY_RECIPE)
	{
		SEND("You aren't carrying that recipe.\r\n",ch);
		return;
	}
	
	if (!get_obj_list_by_type(ch, ITEM_ALCHEMY_LAB, ch->in_room->contents))
	{
		SEND("You need to be near an alchemy lab to do that.\r\n",ch);
		return;
	}
	
	if (recipe->value[0] > -1 && ((ing1 = get_obj_carry(ch, herb_table[recipe->value[0]].name, ch)) == NULL || ing1->item_type != ITEM_HERB))
	{
		SEND ("You are missing the main ingredient!\r\n",ch);
		return;
	}
	
	if (recipe->value[1] > -1 && ((ing2 = get_obj_carry(ch, herb_table[recipe->value[1]].name, ch)) == NULL || ing2->item_type != ITEM_HERB))
	{
		SEND ("You are missing the secondary ingredient!\r\n",ch);
		return;
	}
	
	if (recipe->value[2] > -1 && ((ing3 = get_obj_carry(ch, herb_table[recipe->value[2]].name, ch)) == NULL || ing3->item_type != ITEM_HERB))
	{
		SEND ("You are missing the tertiary ingredient!\r\n",ch);
		return;
	}
	
	chance = number_percent();
	if (recipe->level > total_levels(ch))
		chance += recipe->level - total_levels(ch);
	if (total_levels(ch) > recipe->level)
		chance -= total_levels(ch) - recipe->level;
	
	if (IS_GNOME(ch))
		chance -= 5;
	
	WAIT_STATE (ch, skill_table[gsn_alchemy].beats);
	
	if (chance < get_skill(ch, gsn_alchemy))
	{	
		sprintf(buf, "You brew a potion of %s.\r\n", skill_table[recipe->value[3]].name);		
		SEND (buf, ch);		
		//put act here
		potion = create_object (get_obj_index (OBJ_VNUM_POTION), 0);
		if (ing3)
			extract_obj(ing1);
		if (ing3)
			extract_obj(ing2);
		if (ing3)
			extract_obj(ing3);
		extract_obj(flask);
		potion->value[1] = recipe->value[3]; //apply spell.
		potion->value[0] = IS_GNOME(ch) ? total_levels(ch) + 1 : total_levels(ch);
		
		sprintf (buf, "a potion of %s", skill_table[recipe->value[3]].name);
		potion->short_descr = str_dup(buf);		
		obj_to_char(potion,ch);
		int xp = 0;				
		xp = number_range(50, 100);
		if (total_levels(ch) > 5)
		{
			xp += (total_levels(ch) / 2) * number_range(25,35);
		}
		
		give_exp(ch, xp, TRUE);
		
		check_improve(ch,gsn_alchemy,TRUE,3);	
		return;
	}
	else
	{
		sprintf(buf, "You failed to brew a potion of %s.\r\n", skill_table[recipe->value[3]].name);		
		SEND (buf, ch);		
		if (ing1)
			extract_obj(ing1);
		if (ing2)
			extract_obj(ing2);
		if (ing3)
			extract_obj(ing3);
		extract_obj(flask);
		return;
	}
}

void do_scribe (CHAR_DATA *ch, char *argument)

{
    OBJ_DATA *scroll, *parch, *pen;
	int sn;
	int chance = 0;
    char buf[MAX_STRING_LENGTH]; /* overkill, but what the heck */
	
	if (!IS_WIZARD(ch))
	{
		SEND("You don't even know what you're talking about.\r\n", ch);
		return;
	}

	if (ch->cooldowns[gsn_scribe] > 0)
	{
		SEND("You must wait to use this skill again.\r\n",ch);
		return;
	}
	
	if ( get_skill (ch, gsn_scribe) < 1 )
	{
		SEND("You don't have the knowledge to do that yet.\r\n",ch);
		return;
	}

	
    if (argument[0] == '\0') /* empty */
	{
		SEND("Scribe what, where?\r\n", ch);
		return;
	}

	for ( pen = ch->carrying; pen; pen = pen->next_content )
    {
        if (pen->item_type == ITEM_PEN)              
			break;
    }

   for ( parch = ch->carrying; parch; parch = parch->next_content )
     {
        if (parch->item_type == ITEM_PAPER)            
			break;
     }
	
	if (!pen)
       {
			SEND( "How do you propose scribing without some sort of writing utensil?\r\n",ch);
			return;
       }
    if (!parch)
       { 
			SEND( "What will you scribe this on?\r\n", ch );
			return;
       }
	
	sn = skill_lookup(argument);		

	if ( skill_table[sn].spell_fun == spell_null || skill_table[sn].name == NULL || 				    	skill_table[sn].can_scribe == FALSE )			
	{
		SEND("You can't scribe whatever it is you speak of.\r\n", ch);
		return;
	}
	else
	{
		if (pen && parch)
		{
			chance = number_range(1,200);
			chance += get_skill(ch, gsn_scribe) / 4;
			chance += get_skill(ch, sn) / 4;
			
			sprintf(buf, "You meticulously scribe a scroll of {B%s{x!\r\n", skill_table[sn].name);		
			SEND (buf, ch);		
			scroll = create_object (get_obj_index (OBJ_VNUM_SCROLL), 0);
			scroll->level = total_levels(ch);		
			scroll->value[0] = number_fuzzy(total_levels(ch));
			scroll->cost = number_fuzzy(skill_table[sn].skill_level[ch->ch_class] * 100);
			scroll->value[1] = (sn);
			sprintf (buf, "a scroll of %s", skill_table[sn].name);
			scroll->short_descr = str_dup(buf);
			scroll->description = str_dup(buf);
			scroll->name = str_dup(buf);
			obj_to_char ( scroll, ch );
			//extract_obj( pen );
			extract_obj( parch );		
			
			int xp = 0;				
			xp = number_range(100, 150);
			if (total_levels(ch) > 5)
			{
				xp += (total_levels(ch) / 2) * number_range(25,35);
			}
			
			give_exp(ch, xp, TRUE);
			check_improve (ch, gsn_scribe, TRUE, 1);
			ch->cooldowns[gsn_scribe] = skill_table[gsn_scribe].cooldown;
			return;
		}
	}
}