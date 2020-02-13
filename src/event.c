//////////////////////////////////////////////////////////////////////////////////
//																				//
//		Event system by: Upro												    //
//		January 2010															//
//																				//
//		Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,			//
//		Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    	//
//																				//
//		Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          	//
//		Chastain, Michael Quan, and Mitchell Tse.                              	//
//																				//
//////////////////////////////////////////////////////////////////////////////////

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

void mining_update			(CHAR_DATA *ch);
void prospecting_update		(CHAR_DATA *ch);
void wood_cutting_update	(CHAR_DATA *ch);
void learn_update			(CHAR_DATA *ch, OBJ_DATA *scroll);
void digging_update			(CHAR_DATA *ch);

int get_ore					args((int rarity));
int get_gem					args((int rarity));
int get_cloth				args((int rarity));

void learn_update(CHAR_DATA *ch, OBJ_DATA *scroll)
{
	//CHAR_DATA *ch;
	
	return;
}


void event_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
	
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;        

		if (!ch->in_room)
			continue;
		
		if(ch->position == POS_SLEEPING)
			continue;
			
		switch (ch->event)
		{
			default: break;
			case EVENT_MINING:
				mining_update(ch); 
				break;
			case EVENT_PROSPECTING:
				prospecting_update(ch); 
				break;				
			case EVENT_FISHING:
				break;
			case EVENT_COOKING:
				break;
			case EVENT_LEARN_SPELL:
				break;
			case EVENT_SAILING:
				break;
			case EVENT_BREWING:
				break;			
			case EVENT_WOOD_CUTTING:
				wood_cutting_update(ch);
				break;
			case EVENT_DIGGING:
				digging_update(ch);
				break;
		}
	}
	return;
}
	
void stop_event(CHAR_DATA *ch)
{
	switch (ch->event)
	{
		default: SEND("You stop doing nothing. Bout time.\r\n",ch); break;
		case EVENT_MINING:
			SEND("You stop the exhausting work of mining.\r\n",ch);
			act ( "$n stops mining away.",ch,NULL,NULL,TO_ROOM);
			break;
		case EVENT_LEARN_SPELL:
			SEND("You stop studying the spell.\r\n",ch); break;						
		case EVENT_BREWING:
			SEND("You stop creating your concoction.\r\n",ch);
			act ( "$n stops breweing their concoction.",ch,NULL,NULL,TO_ROOM);
			break;
		case EVENT_COOKING:
			SEND("You stop cooking, ruining the meal.\r\n",ch);
			act ( "$n stops cooking, despite your hunger.",ch,NULL,NULL,TO_ROOM);
			break;
		case EVENT_SAILING:
			SEND("You stop sailing the ship, hope you don't crash.\r\n",ch);
			act ( "$n quits steering the ship.",ch,NULL,NULL,TO_ROOM);
			break;
		case EVENT_PROSPECTING:
			SEND("You stop prospecting for precious rocks.\r\n",ch);
			act ( "$n quits searching the water.",ch,NULL,NULL,TO_ROOM);
			break;
		case EVENT_FISHING:
			SEND("You reel in the empty line.\r\n",ch);
			act ( "$n reels in their line.",ch,NULL,NULL,TO_ROOM);
			break;
		case EVENT_WOOD_CUTTING:
			SEND("You stop swinging endlessly at the tree.\r\n",ch); 
			act ( "$n stops hacking a nearby tree.",ch,NULL,NULL,TO_ROOM);
			ch->chopping = NULL;
			break;
		case EVENT_DIGGING:
			SEND("You stop digging away at the dirt.\r\n",ch);
			act ("$n stops digging away in the dirt.",ch, NULL, NULL, TO_ROOM);
			break;
	}

	ch->event = EVENT_NONE;
	return;
}

void digging_update(CHAR_DATA *ch)
{
	char buf[MSL];	
	int chance = 0, amt = 0;	
	int msgnum = 0;
	OBJ_DATA *obj, *obj_next;
	
	if (ch->move >= 10)
	{		
		msgnum = number_range (1,4);
		switch (msgnum)
		{
			default: break;
			case 1:
				SEND( "You dig around in the dirt.\r\n", ch ); break;
			case 2:
				SEND( "You begin breaking a sweat as you continue to dig.\r\n", ch ); break;
			case 3:
				SEND( "You start to wonder if there is anything to be found here...\r\n", ch ); break;
			case 4:
				SEND( "You continue to shovel away dirt.\r\n", ch ); break;
		}		
		act ( "$n digs around in the dirt.",ch,NULL,NULL,TO_ROOM);
		ch->move -= number_range(7,13);
		chance = number_range(1,100);
		if (IS_ELF(ch))
			chance += 5;
		if (IS_BIRDFOLK(ch))
			chance += 15;
		if (IS_HUMAN(ch))
			chance -= 5;
		if (IS_DWARF(ch) || IS_HALF_OGRE(ch) || IS_GNOME(ch))
			chance -= 20;
		if (chance < number_percent())
		{	
			for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
				if (obj->altitude < 0)
				{
					obj->altitude++;
					
					if (obj->altitude > -1)
					{
						sprintf(buf, "You manage to dig up %s!\r\n", obj->short_descr);
						SEND(buf,ch);
						
						int xp = 0;				
						xp = number_range(25, 40) * amt;
						if (ch->level > 5)
						{
							xp += (ch->level / 2) * number_range(25,35);
						}
						
						give_exp(ch, xp, TRUE);			
					}
					break;
				}
			}
						
		}			
		return;
	}
	else
	{
		SEND("You are too exhausted to continue digging.\r\n",ch);
		stop_event(ch);
		return;
	}
	return;
}

void wood_cutting_update(CHAR_DATA *ch)
{
	char buf[MSL];	
	int chance = 0, amt = 0;	
	int wood = 0, msgnum = 0;
	
	if (ch->move >= 10)
	{		
		msgnum = number_range (1,4);
		switch (msgnum)
		{
			default: break;
			case 1:
				SEND( "You swing your axe tirelessly at the tree.\r\n", ch ); break;
			case 2:
				SEND( "Breathing heavily, you swing at the tree again.\r\n", ch ); break;
			case 3:
				SEND( "You begin to wonder why you decided to cut this tree down in the first place...\r\n", ch ); break;
			case 4:
				SEND( "This pays off eventually... doesn't it?\r\n", ch ); break;
		}		
		act ( "$n swings their axe at a nearby tree.",ch,NULL,NULL,TO_ROOM);
		ch->move -= number_range(7,13);
		chance = number_range(1,300);
		if (IS_ELF(ch))
			chance += 25;
		if (IS_HUMAN(ch))
			chance -= 15;
		if (IS_DWARF(ch) || IS_HALF_OGRE(ch))
			chance -= 30;
		if (chance < get_skill(ch, gsn_woodcutting))
		{	
			amt = number_range(1,2);			
				
			if (mat_table[get_material(ch->in_room->area->main_resource)].is_wood && number_percent() < 25)
				wood = get_material(ch->in_room->area->main_resource);
			else
				wood = get_material(ch->chopping->material);
				
			if (mat_table[wood].rarity > UNCOMMON)
				amt /= 2;
			if (amt < 1)
				amt = 1;
				
			if (wood > -1)
			{
				ch->mats[wood] += amt;
				if (amt > 1)
					sprintf(buf, "You manage to chop up %d logs of %s!\r\n", amt, mat_table[wood].material);
				else
					sprintf(buf, "You manage to chop up %d log of %s!\r\n", amt, mat_table[wood].material);				
				SEND(buf,ch);
				
				int xp = 0;				
				xp = number_range(25, 40) * amt;
				if (ch->level > 5)
				{
					xp += (ch->level / 2) * number_range(25,35);
				}
				
				give_exp(ch, xp, TRUE);
				check_improve (ch, gsn_woodcutting, TRUE, 4);
			}
		}			
		return;
	}
	else
	{
		SEND("You are too exhausted to continue chopping wood.\r\n",ch);
		stop_event(ch);
		return;
	}
	return;
}

void prospecting_update(CHAR_DATA *ch)
{
	char buf[MSL];	
	int chance = 0, amt = 0;
	int rarity = 0, i = 0;
	int ore = 0, msgnum = 0;
	
	if (ch->move >= 10)
	{		
		msgnum = number_range (1,4);
		switch (msgnum)
		{
			default: break;
			case 1:
				SEND( "You splash your hands about in the water.\r\n", ch ); break;
			case 2:
				SEND( "For a second... you think you might have spotted something shiny down there.\r\n", ch ); break;
			case 3:
				SEND( "Your knees ache from prospecting so long.\r\n", ch ); break;
			case 4:
				SEND( "This pays off eventually... doesn't it?\r\n", ch ); break;
		}		
		act ( "$n is hunched over, prospecting their life away.",ch,NULL,NULL,TO_ROOM);
		ch->move -= number_range(1,4);
		chance = number_range(1,300);
		if (ch->in_room->area->age > 500)
			chance += 10;
		if (ch->in_room->area->age > 1000)
			chance += 15;
		if (IS_HALFLING(ch))
			chance -= 25;
		if (IS_HUMAN(ch))
			chance -= 15;
		if (chance < get_skill(ch, gsn_prospecting))
		{	
			amt = number_range(1,2);
			i = number_percent();
			if (i < 50)
				rarity = VERY_COMMON;
			else if (i > 49 && i < 70)
				rarity = COMMON;
			else if (i > 69 && i < 85)
				rarity = UNCOMMON;
			else if (i > 84 && i < 95)
				rarity = RARE;
			else
				rarity = VERY_RARE;			
				
			if (rarity > UNCOMMON)
				amt /= 2;
			if (amt < 1)
				amt = 1;
				
			if (mat_table[get_material(ch->in_room->area->main_resource)].is_metal && number_percent() < 25)
				ore = get_material(ch->in_room->area->main_resource);
			else
				ore = get_ore(rarity);
				
			if (ore > -1)
			{
				ch->mats[ore] += amt;
				if (amt > 1)
					sprintf(buf, "You manage to shore up %d pounds of %s!\r\n", amt, mat_table[ore].material);
				else
					sprintf(buf, "You manage to shore up %d pound of %s!\r\n", amt, mat_table[ore].material);
				SEND(buf,ch);
				
				int xp = 0;				
				xp = number_range(25, 40) * amt;
				if (ch->level > 5)
				{
					xp += (ch->level / 2) * number_range(25,35);
				}
				
				give_exp(ch, xp, TRUE);
				check_improve (ch, gsn_prospecting, TRUE, 4);
			}
		}			
		return;
	}
	else
	{
		SEND("You are too exhausted to continue prospecting.\r\n",ch);
		stop_event(ch);
		return;
	}
	return;
}


void mining_update(CHAR_DATA *ch)
{
	char buf[MSL];
	int chance = 0, amt = 0;
	int rarity = 0, i = 0;
	int ore = 0, msgnum = 0;
	OBJ_DATA *pick = NULL;;
	
	if (ch->move >= 10)
	{		
		msgnum = number_range (1,4);
		switch (msgnum)
		{
			default: break;
			case 1:
				SEND( "You pick at the rock tirelessly.\r\n", ch ); break;
			case 2:
				SEND( "You whistle a tune while breaking away chunks of rock.\r\n", ch ); break;
			case 3:
				SEND( "Your back aches slightly as you continue to mine away.\r\n", ch ); break;
			case 4:
				SEND( "You're getting exausted, where's the beer?\r\n", ch ); break;
		}		
		act ( "$n picks hard, mining away.",ch,NULL,NULL,TO_ROOM);
		ch->move -= number_range(8,16);
		if ((pick = get_eq_char(ch, WEAR_HOLD)) != NULL)
		{
			if (number_percent() < 3)
			{
				pick->condition -= number_range(1,3);
			}
		}
		chance = number_range(1,300);
		if (ch->in_room->area->age > 500)
			chance += 10;
		if (ch->in_room->area->age > 1000)
			chance += 15;
		if (IS_DWARF(ch))
			chance -= 25;
		if (IS_GNOME(ch))
			chance -= 15;
		if (IS_ELF(ch))
			chance += 5;
		if (chance < get_skill(ch, gsn_mine))
		{
			amt = number_range(1,5);
			i = number_percent();
			if (i < 50)
				rarity = VERY_COMMON;
			else if (i > 49 && i < 70)
				rarity = COMMON;
			else if (i > 69 && i < 85)
				rarity = UNCOMMON;
			else if (i > 84 && i < 95)
				rarity = RARE;
			else
				rarity = VERY_RARE;			
				
			if (rarity > UNCOMMON)
				amt /= 2;
			if (amt < 1)
				amt = 1;
			
			if (mat_table[get_material(ch->in_room->area->main_resource)].is_metal && number_percent() < 25)
				ore = get_material(ch->in_room->area->main_resource);
			else
				ore = get_ore(rarity);
			
			
			if (ore > -1)
			{
				ch->mats[ore] += amt;
				if (amt > 1)
					sprintf(buf, "You manage to dig up %d pounds of %s!\r\n", amt, mat_table[ore].material);
				else
					sprintf(buf, "You manage to dig up %d pound of %s!\r\n", amt, mat_table[ore].material);
				SEND(buf,ch);
				int xp = 0;				
				xp = number_range(25, 40) * amt;
				if (ch->level > 5)
				{
					xp += (ch->level / 2) * number_range(25,35);
				}
				
				give_exp(ch, xp, TRUE);
				check_improve (ch, gsn_mine, TRUE, 4);	
			}

		}	
		else
		{
			check_improve (ch, gsn_mine, FALSE, 2);	
		}		
		return;
	}
	else
	{
		SEND("You are too exhausted to continue mining.\r\n",ch);
		stop_event(ch);
		return;
	}
	return;
}



void give_exp(CHAR_DATA *ch, int xp, bool show)
{
	int xp2 = 0;
	char buf[MSL];
	
	if (!IS_MCLASSED(ch) && ch->level < LEVEL_HERO)
	{		
		if (ch->no_exp[0] == TRUE)
			return;
		xp = xp;
		if (show)
		{
			sprintf (buf, "You receive {g%d{x experience points.\r\n", xp);
			SEND (buf, ch);
		}
		gain_exp (ch, xp, FALSE);		
	}
	else if (IS_MCLASSED(ch) && ch->level >= MCLASS_ONE)
	{
		if (ch->no_exp[1] == TRUE)
			return;
		if (ch->level2 >= MCLASS_TWO)
			return;
		xp2 = xp;
		if (show)
		{
			sprintf (buf, "You receive {g%d{x experience points.\r\n", xp);
			SEND (buf, ch);
		}
		gain_exp (ch, xp2, TRUE);
	}	
	else
	{
		if (ch->level2 >= MCLASS_TWO)
		{
			if (ch->no_exp[0])
				return;
						
			if (show)
			{
				sprintf (buf, "You receive {g%d{x experience points.\r\n", xp);
				SEND (buf, ch);
			}
			gain_exp (ch, xp, FALSE);		
		}
		else
		{
			xp2 = xp * 1/2;
			xp = xp * 1/2;	
			if (ch->no_exp[0] == TRUE && ch->no_exp[1] == FALSE)
			{
				sprintf (buf, "You receive {g%d{x experience points.\r\n", xp2);				
				gain_exp (ch, xp2, TRUE);		
			}
			if (ch->no_exp[0] == FALSE && ch->no_exp[1] == TRUE)
			{
				sprintf (buf, "You receive {g%d{x experience points.\r\n", xp);
				gain_exp (ch, xp, FALSE);								
			}	
			if (ch->no_exp[0] == TRUE && ch->no_exp[1] == TRUE)
			{				
				return;
			}
			if (ch->no_exp[0] == FALSE && ch->no_exp[1] == FALSE)
			{
				sprintf (buf, "You receive {g%d{x / {g%d{x experience points.\r\n", xp, xp2);
				gain_exp (ch, xp, FALSE);
				gain_exp (ch, xp2, TRUE);
				return;
			}				
			if (show)
				SEND (buf, ch);			
		}
	}
	return;
}