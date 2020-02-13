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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"

/*
 * Local functions.
 */
void 	birthday_update(void);


// Other functions.
int 	clan_lookup 		args ((const char *name));
int 	hit_gain 			args ((CHAR_DATA * ch));
int 	mana_gain 			args ((CHAR_DATA * ch));
int 	move_gain 			args ((CHAR_DATA * ch));
void 	mobile_update 		args ((void));
void 	weather_update 		args ((void));
void 	char_update 		args ((void));
void 	obj_update 			args ((void));
void 	aggr_update 		args ((void));
void	msdp_update	args( ( void ) ); /* <--- Add this line */
void    underwater_update   args( ( void ) );
void    bleed_update    	args( ( void ) );
void	iron_will_update	args((void));
void 	wind_effect_update	args((void));
void    spell_over_time_update  args((void));
void    regen_char_update   args((void));
void    weather_effect_update	args((void));
void	water_damage_update	args((void));
void 	raw_kill 			args((CHAR_DATA * victim));
bool 	check_iron_will		args((CHAR_DATA *ch));
void    cooldown_update		args((void));
void    achievement_update  args((void));


bool    get_obj_list_by_type		args((CHAR_DATA * ch, int type, OBJ_DATA * list));

/* used for saving */

int save_number = 0;
int     global_exp;
int		global_gold;
int 	global_skill;

const char *corpse_descs[] = {
   "The corpse of %s is in the last stages of decay.",
   "The corpse of %s is crawling with vermin.",
   "The corpse of %s fills the air with a foul stench.",
   "The corpse of %s is buzzing with flies.",
   "The corpse of %s lies here."
};

const char *rain_slowing_down[] = {
	"The seemingly relentless rain finally begins to let up a bit.\r\n",
	"The rain seems to be slowing down somewhat.\r\n",
	"The sky slows it's watery assault on the planet."
};

const char *rain_speeding_up[] = {
	"The rain seems to be picking up a bit.\r\n",
	"Rain starts pouring down quite a bit heavier.\r\n",
	"The rain starts coming down harder.\r\n",
	"Heavy droplets of rain continue their relentless assault.\r\n"
};

const char *rain_starting[] = {
	"Small droplets of rainwater trickle down from the sky.\r\n",
	"Random sprinkling from the skies above alert you to the probability of rain.\r\n",
	"Droplets of rainwater make their way to the ground from thick clouds above.\r\n"
};

const char *rain_stopping[] = {
};

const char *snow_slowing_down[] = {
};

const char *snow_speeding_up[] = {
};

const char *snow_starting[] = {
};

const char *snow_stopping[] = {
};



/*
 * Advancement stuff.
 */
void advance_level (CHAR_DATA * ch, bool hide, bool secondary)
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
	int add_pp; //psionic points
	int prime_stat = -1;	
	
    ch->pcdata->last_level =
        (ch->played + (int) (current_time - ch->logon)) / 3600;
	
	//Psionic point gain:
	if (ch->has_talent)
	{
		add_pp = number_range(2,6) + con_app[GET_CON(ch)].pp_bonus;
		if (GET_INT(ch) > 20)
			add_pp += GET_INT(ch) - 20;
			
		if (add_pp > 0)
			ch->max_pp += add_pp;
	}
		
    if (secondary == TRUE && IS_MCLASSED(ch))
	{	
		//HP GAIN
			add_hp =
			con_app[GET_CON(ch)].hitp +
			number_range ( ch_class_table[ch-> ch_class2].hp_min,
						  ch_class_table[ch-> ch_class2].hp_max);
		if (ch->sex == SEX_MALE)
			add_hp += 1;	
			
		if (ch->boon == BOON_HARDINESS)
			add_hp++;
	
		if (IS_FIGHTER(ch) && !IS_MCLASSED(ch))		
			add_hp++;
		
		if (IS_PALADIN(ch))
			add_hp += number_range(1,2);
		

		//ALL RACIAL MODIFIERS GO HERE - Upro		
		if (IS_LIZARDMAN(ch))
			add_hp ++;
		
		if (IS_GNOME(ch))
			add_mana++;
		//End Racial Modifiers
		
		
		//MANA GAIN
		add_mana = number_range (2, (2 * (GET_INT(ch) + GET_WIS(ch))) / 5);

		if (!ch_class_table[ch->ch_class].fMana && !ch_class_table[ch->ch_class2].fMana)
				add_mana /= 2;				
		
		if (IS_RANGER(ch) || IS_BARD(ch)) 
			add_mana += number_range (1, 2);		
		else if (IS_WIZARD(ch))
			add_mana += number_range (3, 4);
		else if (IS_CLERIC(ch) || IS_DRUID(ch)) 
			add_mana += number_range (1, 3);
		else 
			add_mana += 0;
		
		if (ch->sex == SEX_FEMALE)
			add_mana += 1;
		
		if (ch->boon == BOON_BRILLIANCE)
			add_mana++;
		
		if (IS_WIZARD(ch))
		{
			if (add_mana < 5)
				add_mana = 5;
		}
		
		
		//MOVE GAIN:
		add_move = number_range (2, (GET_CON(ch) + GET_DEX(ch)) / 5);
		if (GET_CON(ch) >= 20)
			add_move += 2;
		if (GET_DEX(ch) >= 20)
			add_move += 2;
		
		//PRACTICES GAIN:		
		//add_prac = wis_app[GET_WIS(ch)].practice;  //Old ROM way... out with the old, in with the new.
		
		//Practice gaining now based on primary stats.
		prime_stat = PRIME_STAT(ch, TRUE);
		
		add_prac = 1; //everybody gets at least one.
		add_prac += (GET_STAT(ch, prime_stat) / 5);		
		
		if (GET_STAT(ch, SECOND_STAT(ch, TRUE)) > 20 && GET_STAT(ch, SECOND_STAT(ch, TRUE) != -1))
			add_prac++;
		
		if (add_prac > 6)
			add_prac = 6;
		
		
		
		add_hp = add_hp * 9 / 10;
		add_mana = add_mana * 9 / 10;
		add_move = add_move * 9 / 10;

		add_hp = UMAX (2, add_hp);
		add_mana = UMAX (2, add_mana);
		add_move = UMAX (6, add_move);
	}
	else
	{	
		//HP GAIN
			add_hp =
			con_app[GET_CON(ch)].hitp +
			number_range ( ch_class_table[ch-> ch_class2].hp_min,
						  ch_class_table[ch-> ch_class2].hp_max);
		if (ch->sex == SEX_MALE)
			add_hp += 1;	
			
		if (ch->boon == BOON_HARDINESS)
			add_hp++;
	
		if (IS_FIGHTER(ch) && !IS_MCLASSED(ch))		
			add_hp++;
		
		if (IS_PALADIN(ch))
			add_hp += number_range(1,2);
		
		//MANA GAIN
		add_mana = number_range (2, (2 * (GET_INT(ch) + GET_WIS(ch))) / 5);

		if (!ch_class_table[ch->ch_class].fMana && !ch_class_table[ch->ch_class2].fMana)
				add_mana /= 2;				
		
		if (IS_RANGER(ch) || IS_BARD(ch)) 
			add_mana += number_range (1, 2);		
		else if (IS_WIZARD(ch))
			add_mana += number_range (3, 4);
		else if (IS_CLERIC(ch) || IS_DRUID(ch)) 
			add_mana += number_range (1, 3);
		else 
			add_mana += 0;
		
		if (ch->sex == SEX_FEMALE)
			add_mana += 1;
		
		if (ch->boon == BOON_BRILLIANCE)
			add_mana++;
		
		if (IS_WIZARD(ch))
		{
			if (add_mana < 5)
				add_mana = 5;
		}
		
		//MOVE GAIN:
		add_move = number_range (2, (GET_CON(ch) + GET_DEX(ch)) / 5);
		if (GET_CON(ch) >= 20)
			add_move += 2;
		if (GET_DEX(ch) >= 20)
			add_move += 2;
		
		//PRACTICES GAIN:	
		//add_prac = wis_app[GET_WIS(ch)].practice;  //Old ROM way... out with the old, in with the new.
		
		prime_stat = PRIME_STAT(ch, FALSE);
		
		add_prac = 1; //everybody gets at least one.
		add_prac += (GET_STAT(ch, prime_stat) / 5);		
		
		if (GET_STAT(ch, SECOND_STAT(ch, FALSE)) > 20 && GET_STAT(ch, SECOND_STAT(ch, FALSE) != -1))
			add_prac++;
		
		if (add_prac > 6)
			add_prac = 6;
		
		add_hp = add_hp * 9 / 10;
		add_mana = add_mana * 9 / 10;
		add_move = add_move * 9 / 10;

		add_hp = UMAX (2, add_hp);
		add_mana = UMAX (2, add_mana);
		add_move = UMAX (6, add_move);
	}
	ch->max_hit += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;
	ch->practice += add_prac;
	ch->train += 1;
	
	if (total_levels(ch) == 10 || total_levels(ch) == 20 || total_levels(ch) == 30 || total_levels(ch) == 40)
	{
		ch->train += 1;
	}
	
    ch->pcdata->perm_hit += add_hp;
    ch->pcdata->perm_mana += add_mana;
    ch->pcdata->perm_move += add_move;
	
	if (total_levels(ch) >= 10 && !ch->clan)
	{
		ch->clan = clan_lookup("Loner");	
		SEND("You have been auto-entered into the pk system. Be wary.\r\n",ch);
	}
	
    if (!hide)
    {
        sprintf (buf,
                 "You gain {G%d{x hit point%s, {G%d{x mana, {G%d{x move, and {B%d{x practice%s.\r\n",
                 add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
                 add_prac, add_prac == 1 ? "" : "s");
        SEND (buf, ch);
    }
    return;
	
}



void gain_exp (CHAR_DATA * ch, int gain, bool secondary)
{
    char buf[MAX_STRING_LENGTH];

	if (secondary == TRUE && !IS_MCLASSED(ch))
		return;
	
    if (IS_NPC (ch))
		return;
	
	if (total_levels(ch) >= LEVEL_HERO) 
		return;
	if (IS_IMMORTAL(ch))
		return;
		
	if (IS_MCLASSED(ch))
	{				
		if (secondary == FALSE && ch->level >= MCLASS_ONE)
			secondary = TRUE;
	}	
	
	if (secondary == TRUE)
	{
		ch->exp2 += gain;
		while (ch->level2 < MCLASS_TWO && ch->exp2 >=	
		   exp_per_level (ch, TRUE))		   
		{
			SEND ("{GYou raise a level in your secondary class!!  {x", ch);
			ch->level2 += 1;
			sprintf (buf, "%s gained level %d (secondary)", ch->name, ch->level2);
			log_string (buf);
			sprintf (buf, "$N has attained level %d ({Dsecondary{g){x!", ch->level2);
			wiznet (buf, ch, NULL, WIZ_LEVELS, 0, 0);
			advance_level (ch, FALSE, TRUE);
			save_char_obj (ch);		
		}
	}	
	else if (secondary == FALSE)
	{
		ch->exp += gain;		
		while ((IS_MCLASSED(ch) ? ch->level < MCLASS_ONE : ch->level < 40) && ch->exp >=	
			exp_per_level (ch, FALSE))		   
		{
			if (IS_MCLASSED(ch))
				SEND ("{GYou raise a level in your primary class!!  {x", ch);
			else
				SEND ("{GYou raise a level!!  {x", ch);
			ch->level += 1;
			sprintf (buf, "%s gained level %d (primary)", ch->name, ch->level);
			log_string (buf);
			sprintf (buf, "$N has attained level %d ({Dprimary{g){x!", ch->level);
			wiznet (buf, ch, NULL, WIZ_LEVELS, 0, 0);
			advance_level (ch, FALSE, TRUE);
			save_char_obj (ch);
		}
	}	
    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain (CHAR_DATA * ch)
{
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;

	gain = number_range (1,2);
		
    if (IS_NPC (ch))
    {        
        if (IS_AFFECTED (ch, AFF_REGENERATION))
            gain *= 2;

        switch (ch->position)
        {
            default:
                gain /= 2;
                break;
            case POS_SLEEPING:
                gain += 2;
                break;
            case POS_RESTING:
				gain += 1;
                break;
            case POS_FIGHTING:
                gain /= 3;
                break;
        }


    }
    else
    {
        gain += con_app[GET_CON(ch)].hitp;        
        
		if (IS_AFFECTED (ch, AFF_REGENERATION))
            gain *= 2;
		
		number = number_percent ();
        if (number < get_skill (ch, gsn_fast_healing))
        {
            gain += 2;
            if (ch->hit < ch->max_hit)
                check_improve (ch, gsn_fast_healing, TRUE, 8);
        }

        switch (ch->position)
        {
            default:
                gain = 1;
                break;
            case POS_SLEEPING:
				gain += 2;
                break;
            case POS_RESTING:
                gain += 1;
                break;
            case POS_FIGHTING:
                gain /= 6;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;

    }
	
	//Trolls heal faster.
	if (IS_TROLL(ch))
		gain = gain * 10 / 9;
	
	gain += (ch->in_room->heal_rate / 5);
	
	gain += (total_levels(ch) / 4);	

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW) || is_affected(ch, gsn_quicken_tempo))
        gain /= 2;

	if (ch->bleeding > 1 && ch->bleeding < 8 && number_percent() < 70)
	{
		ch->bleeding--;
		SEND("Your bleeding slows a bit.\r\n",ch);
	}
	
	if (is_affected(ch, gsn_nightmare))
		gain /= 2;	
			
	
	if (IS_NPC(ch) && IS_SET(ch->act2, ACT2_INANIMATE))
		gain = 0;
	
    return (gain);	
}



int mana_gain (CHAR_DATA * ch)
{
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;
	
	gain = number_range(2,8);
		
    if (IS_NPC (ch))
    {        
        switch (ch->position)
        {
            default:
                gain /= 2;
                break;
            case POS_SLEEPING:
                gain += 6;
                break;
            case POS_RESTING:
				gain += 1;
                break;
            case POS_FIGHTING:
                gain /= 3;
                break;
        }
    }
    else
    {
        gain = (GET_WIS(ch) + GET_INT(ch)) / 2;
        number = number_percent ();
        if (number < get_skill (ch, gsn_meditation))
        {
            gain += 5;
            if (ch->mana < ch->max_mana)
                check_improve (ch, gsn_meditation, TRUE, 8);
        }
        if (! ch_class_table[ch-> ch_class].fMana)
            gain /= 2;

        switch (ch->position)
        {
            default:
                gain /= 4;
                break;
            case POS_SLEEPING:
				gain += 6;
                break;
            case POS_RESTING:
                gain += 2;
                break;
            case POS_FIGHTING:
                gain /= 6;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;

    }
	
	if (IS_ELF(ch) || IS_GNOME(ch))
		gain = gain * 10 / 9;
		
	gain += (total_levels(ch) / 2);
    gain += (ch->in_room->mana_rate / 3);

	
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[4] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;
	
	if (is_affected(ch, gsn_nightmare))
		gain /= 2;
		
	if (IS_NPC(ch) && IS_SET(ch->act2, ACT2_INANIMATE))
		gain = 0;
		
    return (gain);
}


int talent_gain (CHAR_DATA * ch)
{
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;
	
	gain = number_range(2,4);
		
    if (IS_NPC (ch))
    {        
        switch (ch->position)
        {
            default:
                gain /= 2;
                break;
            case POS_SLEEPING:
                gain += 6;
                break;
            case POS_RESTING:
				gain += 1;
                break;
            case POS_FIGHTING:
                gain /= 3;
                break;
        }
    }
    else
    {
        gain = (GET_CON(ch) + GET_INT(ch)) / 3;
        number = number_percent ();
        if (number < get_skill (ch, gsn_meditation))
        {
            gain += 5;
            if (ch->mana < ch->max_mana)
                check_improve (ch, gsn_meditation, TRUE, 8);
        }
		if (number < get_skill (ch, gsn_fast_healing))
        {
            gain += 2;
            if (ch->hit < ch->max_hit)
                check_improve (ch, gsn_fast_healing, TRUE, 8);
        }

        switch (ch->position)
        {
            default:
                gain /= 4;
                break;
            case POS_SLEEPING:
				gain += 6;
                break;
            case POS_RESTING:
                gain += 2;
                break;
            case POS_FIGHTING:
                gain /= 6;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;

    }
	gain += (total_levels(ch) / 2);
    gain += (ch->in_room->mana_rate / 4);
	gain += (ch->in_room->heal_rate / 4);
	
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[4] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;
	
	if (is_affected(ch, gsn_nightmare))
		gain /= 2;
		
	if (IS_NPC(ch) && IS_SET(ch->act2, ACT2_INANIMATE))
		gain = 0;
		
    return (gain);
}



int move_gain (CHAR_DATA * ch)
{
    int gain;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch))
    {
        gain = ch->level / 2; 
    }
    else
    {
        gain = number_range(2,6);

        switch (ch->position)
        {
			default:
				gain /= 2;
				break;
            case POS_SLEEPING:
                gain += get_curr_stat (ch, STAT_DEX) / 2;
                break;
            case POS_RESTING:
                gain += get_curr_stat (ch, STAT_DEX) / 3;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;
    }

	gain += (total_levels(ch) / 3);
    gain += (ch->in_room->heal_rate / 8);

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;

	if (is_affected(ch, gsn_nightmare))
		gain /= 2;
		
	if (IS_NPC(ch) && IS_SET(ch->act2, ACT2_INANIMATE))
		gain = 0;
		
	if (gain < 1)
		gain = 1;
	
    return (gain);
}



void gain_condition (CHAR_DATA * ch, int iCond, int value)
{
    int condition;

    if (value == 0 || IS_NPC (ch) || ch->level >= LEVEL_IMMORTAL)
        return;

    condition = ch->pcdata->condition[iCond];    
	//Sector differences only affect when updating hunger/thirst, not when
	//eating/drinking. That would be silly. - Upro 11/29/2011
	if (value < 0)
	{
		if (iCond == COND_HUNGER)
		{
			if (ch->in_room->sector_type == SECT_SNOW || ch->in_room->sector_type == SECT_ICE)
				value -= 1;
		}
		if (iCond == COND_THIRST && ch->in_room->sector_type == SECT_DESERT)
			value -= 1;
	}
    ch->pcdata->condition[iCond] = (condition + value);

	OBJ_DATA *food = NULL;
	OBJ_DATA *drink = NULL;
	
    if (ch->pcdata->condition[iCond] <= 0)
    {
        switch (iCond)
        {
            case COND_HUNGER:
                if (ch->pcdata->condition[iCond] < -7)				
					SEND ("You are starving!\r\n", ch);
				else
				{
					SEND ("You are hungry.\r\n",ch);
					if (IS_SET (ch->act, PLR_AUTO_EAT) && ch->position > POS_SLEEPING)
					{
						for ( food = ch->carrying; food; food = food->next_content )
						{							
							if (food->item_type == ITEM_FOOD)            
								break;
						}
						if (food && food->item_type == ITEM_FOOD)
							do_function (ch, &do_eat, food->name);
					}
				}
                break;

            case COND_THIRST:
				if (ch->pcdata->condition[iCond] < -8)				
					SEND ("You are extremely dehydrated!\r\n", ch);
				else
					SEND ("You are thirsty.\r\n",ch);
					if (IS_SET (ch->act, PLR_AUTO_DRINK) && ch->position > POS_SLEEPING)
					{
						OBJ_DATA *fountain = NULL;
						
						for (fountain = ch->in_room->contents; fountain; fountain = fountain->next_content)
						{
							if (fountain->item_type == ITEM_FOUNTAIN)
								break;
						}
						
						if (fountain && fountain->item_type == ITEM_FOUNTAIN)						
							do_function (ch, &do_drink, fountain->name);
						else
						{						
							for ( drink = ch->carrying; drink; drink = drink->next_content )
							{							
								if (drink->item_type == ITEM_DRINK_CON)            
									break;
							}
							if (drink && drink->item_type == ITEM_DRINK_CON)
								do_function (ch, &do_drink, drink->name);
						}
					}
                break;

            case COND_DRUNK:
				if (condition != 0)
					SEND ("You are sober.\r\n", ch);
                break;
			
			case COND_EUPHORIC:		
				if (condition != 0)			
					SEND ("Your euphoric trip comes to an end.\r\n", ch);
				break;
        }	
    }
	if (ch->pcdata->condition[iCond] < 0)
		ch->pcdata->condition[iCond] = 0;
	
    return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;

        if (!IS_NPC (ch) || ch->in_room == NULL
            || IS_AFFECTED (ch, AFF_CHARM)) continue;

		if (!ch->fighting)
		{
			do_function (ch, &do_wear, "all");			
		}
			
        if (ch->in_room->area->empty && !IS_SET (ch->act, ACT_UPDATE_ALWAYS))
            continue;
			
        /* Examine call for special procedure */		
		if (ch->spec_fun != 0)
        {
            if ((*ch->spec_fun) (ch))
                continue;
        }

        if (ch->pIndexData->pShop != NULL)    /* give him some gold */
            if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
            {
                ch->gold +=
                    ch->pIndexData->wealth * number_range (1, 20) / 5000000;
                ch->silver +=
                    ch->pIndexData->wealth * number_range (1, 20) / 50000;
            }

        /*
         * Check triggers only if mobile still in default position
         */
        if (ch->position == ch->pIndexData->default_pos)
        {
            /* Delay */
            if (HAS_TRIGGER_MOB(ch, TRIG_DELAY) && ch->mprog_delay > 0)
            {
                if (--ch->mprog_delay <= 0)
                {
                    p_percent_trigger (ch, NULL, NULL, NULL, NULL, NULL, TRIG_DELAY);
                    continue;
                }
            }
            if (HAS_TRIGGER_MOB(ch, TRIG_RANDOM))
            {
                if (p_percent_trigger (ch, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM))
                    continue;
            }
        }

        /* That's all for sleeping / busy monster, and empty zones */
        if (ch->position != POS_STANDING)
            continue;

        /* Scavenge */
        if (IS_SET (ch->act, ACT_SCAVENGER)
            && ch->in_room->contents != NULL && number_bits (6) == 0)
        {
            OBJ_DATA *obj;
            OBJ_DATA *obj_best;
            int max;

            max = 1;
            obj_best = 0;
            for (obj = ch->in_room->contents; obj; obj = obj->next_content)
            {
                if (CAN_WEAR (obj, ITEM_TAKE) && can_loot (ch, obj)
                    && obj->cost > max && obj->cost > 0 && !IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM))
                {
                    obj_best = obj;
                    max = obj->cost;
                }
            }

            if (obj_best)
            {
                obj_from_room (obj_best);
                obj_to_char (obj_best, ch);
                act ("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
            }
        }

        /* Wander */
        if (!IS_SET (ch->act, ACT_SENTINEL)
            && number_bits (3) == 0
            && (door = number_bits (5)) <= 5
            && (pexit = ch->in_room->exit[door]) != NULL
            && pexit->u1.to_room != NULL
            && !IS_SET (pexit->exit_info, EX_CLOSED)
            && !IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)
            && (!IS_SET (ch->act, ACT_STAY_AREA)
                || pexit->u1.to_room->area == ch->in_room->area)
            && (!IS_SET (ch->act, ACT_OUTDOORS)
                || !IS_SET (pexit->u1.to_room->room_flags, ROOM_INDOORS))
            && (!IS_SET (ch->act, ACT_INDOORS)
                || IS_SET (pexit->u1.to_room->room_flags, ROOM_INDOORS))
			&& (!IS_SET (ch->act2, ACT2_STAY_SECTOR) && pexit->u1.to_room->sector_type == ch->in_room->sector_type)		)	
        {
				move_char (ch, door, FALSE);
        }
    }

    return;
}


void get_tropical ( AREA_DATA * area )
{	
	int i = number_percent();
	int chance = 10;
	char buf[MSL];
    DESCRIPTOR_DATA *d;
	
	buf[0] = '\0';
	
	
	//Keep track of how long it has or hasn't been precipitating.
	if (area->curr_precip_amount > -1)
		area->precip_time += 1;
	else
		area->precip_time -= 1;
	
	
	
	//Find chance for precipitation
	chance = 25; //high base chance since it's the tropics.
	
	chance += area->curr_cloud;		
	chance += area->humidity / 5;		
	//how long since it's rained?
	if (area->precip_time >= 1)
		chance -= area->precip_time;
	else
		chance += area->precip_time;
	//how much is it raining now?
	if (area->curr_precip_amount > -1)
		chance += 5;	
	else
		chance -= 5;
		
	//Do the weather change if conditions met.
	if ( area->curr_cloud < PARTLY_CLOUDY && area->curr_precip == -1 ) //no clouds no rain.			
		return;							
	else
	{				
		if (i == 100 || (i < chance && i != 1))
		{
			area->curr_precip = RAIN;			
			if (area->curr_precip_amount == -1)
				strcat(buf, "It begins lightly raining.\r\n");
			else if (area->curr_precip_amount == 0)
				strcat(buf, "The rain gently picks up a bit.\r\n");
			else if (area->curr_precip_amount == 1)
				strcat(buf, "The rain from above starts coming down fast and hard.\r\n");
			else
				strcat(buf, "Torrential rain pours mercilessly from the sky above.\r\n");
			
			area->curr_precip_amount++;			
		}
		else if (i == 1 || i == chance)
		{
			//Any precip stops immediately.
			if (area->curr_precip != -1)
			{
				area->curr_precip = -1;
				area->curr_precip_amount = -1;
				strcat(buf, "The rain suddenly stops.\r\n");				
			}
			//Or starts immediately if there is none.
			else
			{
				area->curr_precip = RAIN;
				area->curr_precip_amount = 0;
				strcat(buf, "It suddenly begins lightly raining.\r\n");				
			}			
		}
		else
		{	
			area->curr_precip_amount--;
			
			if (area->curr_precip_amount == -1)
			{
				strcat(buf, "The rain slowly comes to a stop.\r\n");
				area->curr_precip = -1;
			}
			else if (area->curr_precip_amount == 0)
				strcat(buf, "The rain lightens up to a slight drizzle.\r\n");
			else if (area->curr_precip_amount == 1)
				strcat(buf, "The skies seem to let up as the rain seems to subside somewhat.\r\n");
			else
				strcat(buf, "The heavens slow their assault of rain from above.\r\n");
		}			
	}	
	
	//Send message
	if (buf[0] != '\0')
	{
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->connected == CON_PLAYING && IS_OUTSIDE (d->character) && IS_AWAKE (d->character) && d->character->in_room->area == area)
				SEND (buf, d->character);
		}
	}
	
	return;
}

void get_dry ( AREA_DATA * area )
{
	int chance = 5;
	char buf[MSL];
    DESCRIPTOR_DATA *d;
	
	chance += area->curr_cloud * 5;
			
	buf[0] = '\0';
		
	
	switch (area->curr_precip_amount)
	{
		case -1:
			if ( area->curr_cloud < PARTLY_CLOUDY )
			{				
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = RAIN;
					area->curr_precip_amount++;					
					//area->curr_temp -= number_range (2,5);					
					strcat(buf, "It begins lightly raining.\r\n");		
					break;
				}
			}	
			else
			{			
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = RAIN;
					
					//area->curr_temp -= number_range (2,5);					
					
					if (area->curr_precip_amount < 1)
						strcat(buf, rain_starting[number_range(0,2)]);				
					else					
						strcat(buf, rain_speeding_up[number_range(0,2)]);
					
					area->curr_precip_amount++;
					break;
				}
			}
			
		case VERY_LIGHT:
		case LIGHT:
		case MEDIUM:
		case HEAVY:
			if ( area->curr_cloud < PARTLY_CLOUDY )
			{				
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = RAIN;
					area->curr_precip_amount--;
					if (area->curr_precip_amount < -1)
						area->curr_precip_amount = -1;
					if (area->curr_precip_amount == -1)
					{
						strcat(buf, "The rain stops coming down altogether.\r\n");
						//area->curr_temp += number_range (2,5);					
					}
					else
						strcat(buf, "The rain seems to be slowing down somewhat.\r\n");
					
					break;
				}				
			}	
			else
			{			
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = RAIN;
					area->curr_precip_amount++;
					strcat(buf, "The rain starts coming down quicker.\r\n");
					break;
				}				
			}
		
		case VERY_HEAVY:
			
			if (number_percent() <= chance)
				break;
			else
			{
				area->curr_precip = RAIN;
				area->curr_precip_amount--;
				if (area->curr_precip_amount < -1)
					area->curr_precip_amount = -1;
					
				if (area->curr_precip_amount == -1)				
				{
						strcat(buf, "The rain stops coming down altogether.\r\n");
						//area->curr_temp += number_range (2,5);					
				}
				else
					strcat(buf, "The rain seems to be slowing down somewhat.\r\n");				
				break;
			}			

		default:
			break;
	}
	
	if (buf[0] != '\0')
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->connected == CON_PLAYING && IS_OUTSIDE (d->character) && IS_AWAKE (d->character) && d->character->in_room->area == area && d->character->in_room->sector_type != SECT_SHIP)
                SEND (buf, d->character);
        }
    }	
	
	//Reset precipitation timer or do what we need with it.
	if (area->curr_precip_amount > -1)
		area->precip_time += 1;
	else
		area->precip_time = 0;
		
	
	return;
}

void get_arctic ( AREA_DATA * area )
{
	int chance = 10;
	char buf[MSL];
    DESCRIPTOR_DATA *d;
	
	chance += area->curr_cloud * 10;
			
	buf[0] = '\0';
		
	
	switch (area->curr_precip_amount)
	{
		case -1:
			if ( area->curr_cloud < PARTLY_CLOUDY )
			{				
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = SNOW;
					area->curr_precip_amount++;					
					strcat(buf, "It begins lightly snowing.\r\n");	
					//area->curr_temp -= number_range (4,10);					
					break;
				}
			}	
			else
			{			
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = SNOW;
					area->curr_precip_amount++;
					strcat(buf, "It begins lightly snowing.\r\n");					
					//area->curr_temp -= number_range (4,10);					
					break;
				}
			}
			
		case VERY_LIGHT:
		case LIGHT:
		case MEDIUM:
		case HEAVY:
			if ( area->curr_cloud < PARTLY_CLOUDY )
			{				
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = SNOW;
					area->curr_precip_amount--;
					if (area->curr_precip_amount < -1)
						area->curr_precip_amount = -1;
					if (area->curr_precip_amount == -1)
					{
						strcat(buf, "The snow stops coming down altogether.\r\n");
						//area->curr_temp += number_range (4,10);					
					}
					else
						strcat(buf, "The snow seems to be slowing down somewhat.\r\n");
					
					break;
				}				
			}	
			else
			{			
				if (number_percent() <= chance)
					break;
				else
				{
					area->curr_precip = SNOW;
					area->curr_precip_amount++;
					strcat(buf, "The snow starts to flurry a bit.\r\n");
					break;
				}				
			}
		
		case VERY_HEAVY:
			
			if (number_percent() <= chance)
				break;
			else
			{
				area->curr_precip = SNOW;
				area->curr_precip_amount--;
				if (area->curr_precip_amount < -1)
					area->curr_precip_amount = -1;
					
				if (area->curr_precip_amount == -1)				
				{
						strcat(buf, "The snow stops coming down altogether.\r\n");
						//area->curr_temp += number_range (4,10);					
				}
				else
					strcat(buf, "The snow seems to be slowing down somewhat.\r\n");				
				break;
			}			

		default:
			break;
	}
	
	if (buf[0] != '\0')
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->connected == CON_PLAYING && IS_OUTSIDE (d->character) && IS_AWAKE (d->character) && d->character->in_room->area == area)
                SEND (buf, d->character);
        }
    }	
	
	//Reset precipitation timer or do what we need with it.
	if (area->curr_precip_amount > -1)
		area->precip_time += 1;
	else
		area->precip_time = 0;
		
	
	return;
}

void get_wind (AREA_DATA *pArea)
{
	int mph = 0;
	mph = number_range(1,20);
	
	if (number_percent() < 50)
		pArea->curr_wind += mph;
	else
		pArea->curr_wind -= mph;
	
	if (pArea->curr_precip_amount < MEDIUM)
	{
		if (pArea->curr_wind > 30)
		{
			pArea->curr_wind = 30;
		}
	}
	
	if (pArea->curr_wind < 0)
		pArea->curr_wind = 0;
	
	return;
}

void get_temp (AREA_DATA *pArea)
{
	int max_temp = 0;
	int min_temp = 0;
	int increment = 0;
	int decrement = 0;
	int i = 0;

	switch (pArea->climate)
	{
		default:
			break;
		case CL_TROPICAL:
			max_temp = 110;
			min_temp = 70;
			increment = 2;
			decrement = 2;
			break;
		case CL_DRY:
			max_temp = 130;
			min_temp = 60;
			increment = 6;
			decrement = 4;
			break;
		case CL_COASTAL:
			max_temp = 90;
			min_temp = 30;
			increment = 3;
			decrement = 3;
			break;
		case CL_ARCTIC:
			max_temp = 40;
			min_temp = -50;
			increment = 1;
			decrement = 6;
			break;
		case CL_HIGHLAND:
			max_temp = 80;
			min_temp = 0;
			increment = 2;
			decrement = 4;
			break;
		case CL_TEMPERATE:
			max_temp = 100;
			min_temp = 20;
			increment = 3;
			decrement = 4;
			break;			
	}	
	
	//Seasons affect the temp.
	if (SEASON_SPRING)
		pArea->curr_temp += number_range(1,2);
	if (SEASON_SUMMER)
		pArea->curr_temp += number_range(3,6);
	if (SEASON_AUTUMN)
		pArea->curr_temp -= number_range(1,2);
	if (SEASON_WINTER)
		pArea->curr_temp -= number_range(3,6);
	
	//Rain/snow can lower the temp.
	for (i = 0; i <= pArea->curr_precip_amount; i++)
	{
		pArea->curr_temp += number_range(1,4);
		pArea->curr_temp -= number_range(1,4);
	}
	
	//Higher winds causes lower temp.
	
	for (i = 0; (i * 10) < pArea->curr_wind; i++)
	{
		pArea->curr_temp += number_range(1,2);
		pArea->curr_temp -= number_range(1,2);
		
	}
	
	
	//time of day affects temp.
	
	if (time_info.hour <= 5 || time_info.hour > 18)
		pArea->curr_temp -= decrement;
	else
		pArea->curr_temp += increment;
	
	//Random slow temperature change.
	
	if (number_percent() < 50)
		pArea->curr_temp += number_range(1,2);
	else
		pArea->curr_temp -= number_range(1,2);
		
	if (pArea->curr_temp > max_temp)
		pArea->curr_temp = max_temp;
	if (pArea->curr_temp < min_temp)
		pArea->curr_temp = min_temp;
}

/*
 * Update the weather.
 */
void weather_update (void)
{
	AREA_DATA * pArea;	
	char buf[MSL];
    DESCRIPTOR_DATA *d;
    //int diff;
	int chance = 0;  //chance for it to precipitate
	int dam = 0; // for lightning strikes
	bool lightning = FALSE; //does player get struck by lightning?
	buf[0] = '\0';
		
	switch (++time_info.hour)
    {
        case 5:
            weather_info.sunlight = SUN_LIGHT;
            strcat (buf, "The day has begun.\r\n");
            break;

        case 6:
            weather_info.sunlight = SUN_RISE;
            strcat (buf, "The sun rises in the east.\r\n");
            break;

        case 19:
            weather_info.sunlight = SUN_SET;
            strcat (buf, "The sun slowly disappears in the west.\r\n");
            break;

        case 20:
            weather_info.sunlight = SUN_DARK;
            strcat (buf, "The night has begun.\r\n");
            break;

        case 24:
            time_info.hour = 0;
            time_info.day++;
            break;
    }
	
	if (time_info.day >= 35)
    {
        time_info.day = 0;
        time_info.month++;
    }

    if (time_info.month >= 17)
    {
        time_info.month = 0;
        time_info.year++;
    }
	
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		//Dry up the precipitation over time.
		if (pArea->curr_precip < 0)
		{
			if (pArea->precip_time > 0)				
			{
				pArea->precip_time--;
				if (pArea->curr_temp >= 75 && number_percent() < 50)
					pArea->precip_time--;
					
				if (pArea->precip_time < -10) //Allow negatives for droughts and such.				
					pArea->precip_time = -10;
			}
		}
				
		//Find chance for cloud cover to worsen.
		chance = 15;
		
		chance += pArea->curr_cloud;		
		chance += pArea->humidity / 5;		
		
		if (pArea->precip_time >= 1)
			chance -= pArea->precip_time;
		else
			chance += pArea->precip_time;
			
		//Clouds
		if (number_percent() < chance)
		{
			pArea->curr_cloud ++;
			if (pArea->curr_precip == RAIN && pArea->curr_precip_amount > MEDIUM) //If it's precipitating at all...
			{
				switch (number_range(1,4))
				{
					case 1:
						strcat(buf, "Thick dark clouds gather in the sky.\r\n");				
						break;
					case 2:
						strcat (buf, "Thunder clouds swirl in the sky above.\r\n");
						break;
					case 3:
						strcat (buf, "Storm clouds roll ominously across the sky.\r\n");
						break;
					case 4:
						strcat (buf, "Dark wall clouds, drooping with rain, move slowly across the sky.\r\n");
						break;
				}
				//Lightning strikes
				if (number_percent() < 10)
				{
					if (number_percent() < 2) //1 in 1000 chance?
						lightning = TRUE;							
					else
						strcat (buf, "A brilliant flash of light marks a lightning strike from above.\r\n");					
				}
			}
			else
			{
				switch (number_range(1,4))
				{
					case 1:
						strcat(buf, "The sky is growing cloudier.\r\n");				
						break;
					case 2:
						strcat(buf, "Clouds begin hastily gathering in the sky.\r\n");				
						break;
					case 3:
						strcat(buf, "The light from the sky above is obscured by growing cloud cover.\r\n");				
						break;
					case 4:
						strcat(buf, "Stratus clouds line the sky in a blanket of pillowy fluff.\r\n");				
						break;
				}
			}
		}
		else
		{
			if (number_percent() < 50)
			{
				if (pArea->curr_cloud > CLOUDLESS)
				{
					pArea->curr_cloud --;				
					switch (number_range(1,4))
					{
						case 1:
							strcat(buf, "The sky is growing more clear.\r\n");			
							break;
						case 2:
							strcat(buf, "The clouds in the sky above begin to disperse.\r\n");			
							break;
						case 3:
							strcat(buf, "The clouds begin to part, allowing light to shine through.\r\n");			
							break;
						case 4:
							strcat(buf, "As the time passes, the cloud cover above subsides.\r\n");			
							break;
					}
					pArea->curr_temp += number_range(1,5);
				}
			}
		}
		
		if (buf[0] != '\0')
		{
			for (d = descriptor_list; d != NULL; d = d->next)
			{
				if (d->connected == CON_PLAYING && IS_OUTSIDE (d->character) && IS_AWAKE (d->character) && d->character->in_room->area == pArea)
				{
					//Send what we've got so far.
					SEND (buf, d->character);	
					
					//Strike character with lightning. 1/1000th chance to happen.
					if (lightning)
					{
						dam = dice ((pArea->curr_precip_amount * 4), 15);
						if (saves_spell (d->character->level, d->character, DAM_LIGHTNING))
							dam /= 2;						
						sprintf (buf, "Everything turns white as you are struck by {YLIGHTNING{x!! {r[{x%d{r]{x\r\n", dam);
						SEND (buf, d->character);	
						d->character->hit -= dam;
						if (d->character->hit < -9)
						{
							raw_kill(d->character);
							d->character->hit = 1;				
							SEND("You are DEAD!!\r\n", d->character );
						}
					}
				}
			}
			buf[0] = '\0';
		}
	
		if (pArea->curr_cloud > HEAVY_CLOUDS)
			pArea->curr_cloud = HEAVY_CLOUDS;
		
		if (pArea->curr_cloud < CLOUDLESS)
			pArea->curr_cloud = CLOUDLESS;
		
		get_wind(pArea);
		if (!IS_SET(pArea->area_flags, AREA_PRECIP))
		{
			switch (pArea->climate)
			{
				default:
					break;
				case CL_TROPICAL:
					get_tropical(pArea);
					break;
				case CL_DRY:
					get_dry(pArea);
					break;
				case CL_COASTAL:
					get_tropical(pArea);
					break;
				case CL_ARCTIC:
					get_arctic(pArea);
					break;
				case CL_HIGHLAND:
					get_dry(pArea);
					break;
				case CL_TEMPERATE:
					get_tropical(pArea);
					break;			
			}
		}
		get_temp(pArea);
		
		//If it's too warm to snow, but it's snowing, change it up to sleet.
		if (pArea->curr_precip == SNOW && pArea->curr_temp > 36)		
			pArea->curr_precip = SLEET;
	}
	
	return;
} 


/*
void weather_update (void)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch (++time_info.hour)
    {
        case 5:
            weather_info.sunlight = SUN_LIGHT;
            strcat (buf, "The day has begun.\r\n");
            break;

        case 6:
            weather_info.sunlight = SUN_RISE;
            strcat (buf, "The sun rises in the east.\r\n");
            break;

        case 19:
            weather_info.sunlight = SUN_SET;
            strcat (buf, "The sun slowly disappears in the west.\r\n");
            break;

        case 20:
            weather_info.sunlight = SUN_DARK;
            strcat (buf, "The night has begun.\r\n");
            break;

        case 24:
            time_info.hour = 0;
            time_info.day++;
            break;
    }

    if (time_info.day >= 35)
    {
        time_info.day = 0;
        time_info.month++;
    }

    if (time_info.month >= 17)
    {
        time_info.month = 0;
        time_info.year++;
    }

  if (time_info.month >= 9 && time_info.month <= 16)
        diff = weather_info.mmhg > 985 ? -2 : 2;
    else
        diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change += diff * dice (1, 4) + dice (2, 6) - dice (2, 6);
    weather_info.change = UMAX (weather_info.change, -12);
    weather_info.change = UMIN (weather_info.change, 12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg = UMAX (weather_info.mmhg, 960);
    weather_info.mmhg = UMIN (weather_info.mmhg, 1040);

    switch (weather_info.sky)
    {
        default:
            bug ("Weather_update: bad sky %d.", weather_info.sky);
            weather_info.sky = SKY_CLOUDLESS;
            break;

        case SKY_CLOUDLESS:
            if (weather_info.mmhg < 990
                || (weather_info.mmhg < 1010 && number_bits (2) == 0))
            {
                strcat (buf, "The sky is getting cloudy.\r\n");
                weather_info.sky = SKY_CLOUDY;
            }
            break;

        case SKY_CLOUDY:
            if (weather_info.mmhg < 970
                || (weather_info.mmhg < 990 && number_bits (2) == 0))
            {
                strcat (buf, "It starts to rain.\r\n");
				//climate code here
                weather_info.sky = SKY_RAINING;
            }

            if (weather_info.mmhg > 1030 && number_bits (2) == 0)
            {
                strcat (buf, "The clouds disappear.\r\n");
                weather_info.sky = SKY_CLOUDLESS;
            }
            break;

        case SKY_RAINING:
            if (weather_info.mmhg < 970 && number_bits (2) == 0)
            {
                strcat (buf, "Lightning flashes in the sky.\r\n");
                weather_info.sky = SKY_LIGHTNING;
            }

            if (weather_info.mmhg > 1030
                || (weather_info.mmhg > 1010 && number_bits (2) == 0))
            {
                strcat (buf, "The rain stopped.\r\n");
                weather_info.sky = SKY_CLOUDY;
            }
            break;
			
		case SKY_SNOWING:
            if (weather_info.mmhg < 970 && number_bits (2) == 0)
            {
                strcat (buf, "Snow falls freely from the sky.\r\n");
                weather_info.sky = SKY_LIGHTNING;
            }

            if (weather_info.mmhg > 1030
                || (weather_info.mmhg > 1010 && number_bits (2) == 0))
            {
                strcat (buf, "The snow has stopped.\r\n");
                weather_info.sky = SKY_CLOUDY;
            }
            break;

        case SKY_LIGHTNING:
            if (weather_info.mmhg > 1010
                || (weather_info.mmhg > 990 && number_bits (2) == 0))
            {
                strcat (buf, "The lightning has stopped.\r\n");
                weather_info.sky = SKY_RAINING;
                break;
            }
            break;
    }

    if (buf[0] != '\0')
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->connected == CON_PLAYING && IS_OUTSIDE (d->character)
                && IS_AWAKE (d->character))
                SEND (buf, d->character);
        }
    }

    return;
}
*/


/*
 * Update all chars, including mobs.
*/
void char_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
	DESCRIPTOR_DATA *d;
	char buf[MSL];
	CHAR_DATA *och, *och_next;

    ch_quit = NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
        save_number = 0;

	if (global_exp-- <= 0 && double_exp)
	{
		for (ch = char_list; ch != NULL; ch = ch_next)
		{
			ch_next = ch->next;
		
			sprintf(buf, "{r[{xINFO{r]{x:  Double experience bonus has ended.{x\r\n");				
			SEND(buf, ch);				
		}
		double_exp = FALSE;
		global_exp = 0;
		return;     
	}

	if (global_gold-- <= 0 && double_gold)
	{
		for (ch = char_list; ch != NULL; ch = ch_next)
		{
			ch_next = ch->next;
		
			sprintf(buf, "{r[{xINFO{r]{x:  Double {Ygold{x bonus has ended.{x\r\n");				
			SEND(buf, ch);				
		}
		double_gold = FALSE;
		global_gold = 0;
		return;     
	}
	
	if (global_skill-- <= 0 && double_skill)
	{
		for (ch = char_list; ch != NULL; ch = ch_next)
		{
			ch_next = ch->next;
		
			sprintf(buf, "{r[{xINFO{r]{x:  Double skill improvement bonus has ended.{x\r\n");				
			SEND(buf, ch);				
		}
		double_skill = FALSE;
		global_skill = 0;
		return;     
	}
		
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        ch_next = ch->next;
		
        if (ch->timer > 30 && !IS_IMMORTAL(ch) && ch->desc->connected == CON_PLAYING)
            ch_quit = ch;
			
        if (ch->position >= POS_STUNNED)
        {
            /* check to see if we need to go home */
            if (IS_NPC (ch) && ch->zone != NULL
                && ch->zone != ch->in_room->area && ch->desc == NULL
                && ch->fighting == NULL && !IS_AFFECTED (ch, AFF_CHARM)
                && number_percent () < 5)
            {
                act ("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
                extract_char (ch, TRUE);
                continue;
            }			
        }
		
		if (ch->guildpoints > 30000)
			ch->guildpoints = 30000;
		
        if (ch->position == POS_STUNNED)
            update_pos (ch);

        if (!IS_NPC (ch) && total_levels(ch) < LEVEL_IMMORTAL)
        {
            OBJ_DATA *obj;

            if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
                && obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
            {
                if (--obj->value[2] == 0 && ch->in_room != NULL)
                {
                    --ch->in_room->light;
                    act ("$p goes out.", ch, obj, NULL, TO_ROOM);
                    act ("$p flickers and goes out.", ch, obj, NULL, TO_CHAR);
                    extract_obj (obj);
                }
                else if (obj->value[2] <= 5 && ch->in_room != NULL)
                    act ("$p flickers.", ch, obj, NULL, TO_CHAR);
            }            
				
            if (++ch->timer >= 999 && !IS_IMMORTAL(ch))
            {
                if (ch->was_in_room == NULL && ch->in_room != NULL)
                {
                    ch->was_in_room = ch->in_room;
                    if (ch->fighting != NULL)
                        stop_fighting (ch, TRUE);
                    act ("$n disappears into the void.",
                         ch, NULL, NULL, TO_ROOM);
                    SEND ("You disappear into the void.\r\n", ch);
                    if (ch->level > 1)
                        save_char_obj (ch);
                    char_from_room (ch);
                    char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
                }
            }

            
			gain_condition (ch, COND_DRUNK, -1);
			gain_condition (ch, COND_EUPHORIC, -1);
			if (LOW_MOVE(ch))
			{
				
				gain_condition (ch, COND_FULL, ch->size > SIZE_MEDIUM ? -8 : -4);
				gain_condition (ch, COND_THIRST, ch->size > SIZE_MEDIUM ? -4 : -2);				
				gain_condition (ch, COND_HUNGER,
								ch->size > SIZE_MEDIUM ? -4 : -2);			
			}
			else
			{				
				gain_condition (ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2);
				gain_condition (ch, COND_THIRST, ch->size > SIZE_MEDIUM ? -2 : -1);
				gain_condition (ch, COND_HUNGER,
								ch->size > SIZE_MEDIUM ? -2 : -1);
			}
        }
	
        for (paf = ch->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next;
            if (paf->duration > 0)
            {
                paf->duration--;
                if (number_range (0, 4) == 0 && paf->level > 0)
                    paf->level--;    /* spell strength fades with time */
            }
            else if (paf->duration < 0)
				continue;
            else
            {
                if (paf_next == NULL
                    || paf_next->type != paf->type || paf_next->duration > 0)
                {
                    if (paf->type > 0 && skill_table[paf->type].msg_off)
                    {
                        SEND (skill_table[paf->type].msg_off, ch);
                        SEND ("\r\n", ch);
                    }
                }

                affect_remove (ch, paf);
            }
        }
		
        /*
         * Careful with the damages here,
         *   MUST NOT refer to ch after damage taken,
         *   as it may be lethal damage (on NPC).
         */

        if (is_affected (ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

            if (ch->in_room == NULL)
                continue;

            act ("$n writhes in agony as plague sores erupt from $s skin.",
                 ch, NULL, NULL, TO_ROOM);
            SEND ("You writhe in agony from the plague.\r\n", ch);
            for (af = ch->affected; af != NULL; af = af->next)
            {
                if (af->type == gsn_plague)
                    break;
            }

            if (af == NULL)
            {
                REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
                continue;
            }

            if (af->level == 1)
                continue;

            plague.where = TO_AFFECTS;
            plague.type = gsn_plague;
            plague.level = af->level - 1;
            plague.duration = number_range (1, 2 * plague.level);
            plague.location = APPLY_STR;
            plague.modifier = -5;
            plague.bitvector = AFF_PLAGUE;

            for (vch = ch->in_room->people; vch != NULL;
                 vch = vch->next_in_room)
            {
                if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
                    && !IS_IMMORTAL (vch)
                    && !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (4) == 0)
                {
                    SEND ("You feel hot and feverish.\r\n", vch);
                    act ("$n shivers and looks very ill.", vch, NULL, NULL,
                         TO_ROOM);
                    affect_join (vch, &plague);
                }
            }

            dam = UMIN (ch->level, af->level / 5 + 1);
            ch->mana -= dam;
            ch->move -= dam;
            damage (ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE);
        }
        else if (IS_AFFECTED (ch, AFF_POISON) && ch != NULL
                 && !IS_AFFECTED (ch, AFF_SLOW))
        {
            AFFECT_DATA *poison;

            poison = affect_find (ch->affected, gsn_poison);

            if (poison != NULL)
            {
                act ("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
                SEND ("You shiver and suffer.\r\n", ch);
                damage (ch, ch, poison->level / 10 + 1, gsn_poison,
                        DAM_POISON, FALSE);
            }
        }

        else if (ch->position == POS_INCAP && number_range (0, 1) == 0)
        {
            damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
        }
        else if (ch->position == POS_MORTAL)
        {
            damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
        }
		
	}
		
    

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
    	/*
    	 * Edwin's fix for possible pet-induced problem
    	 * JR -- 10/15/00
    	 */
    	if (!IS_VALID(ch))
    	{
        	bug("[Update.c] char_update: Trying to work with an invalidated character (pet).\n",0); 
        	break;
     	}

        ch_next = ch->next;

        if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
        {
            save_char_obj (ch);
        }

        if (ch == ch_quit)
        {
            do_function (ch, &do_quit, "");
        }
    }

    return;
}




/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update (void)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for (obj = object_list; obj != NULL; obj = obj_next)
    {
        CHAR_DATA *rch;
        char *message;

        obj_next = obj->next;
		
		//Metal rusts in rain and snow.
		if (/*obj->in_room->area->curr_precip && obj->in_room->area->curr_precip > -1 && */IS_METAL(obj->material) && number_percent() < 5 && !IS_SET(obj->extra2_flags, ITEM_INDESTRUCTABLE))
		{
			if (!obj->carried_by)
			{
				if (obj->condition > 0)
				{
					obj->condition--;
				}
			}
		}

		if (IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM) && number_percent() < 99 && obj->carried_by == NULL)
		{
			act ("$p crumbles into dust.", NULL, obj, NULL, TO_ROOM);
		}
		
        /* go through affects and decrement */
        for (paf = obj->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next;
            if (paf->duration > 0)
            {
                paf->duration--;
                if (number_range (0, 4) == 0 && paf->level > 0)
                    paf->level--;    /* spell strength fades with time */
            }
            else if (paf->duration < 0);
            else
            {
                if (paf_next == NULL
                    || paf_next->type != paf->type || paf_next->duration > 0)
                {
                    if (paf->type > 0 && skill_table[paf->type].msg_obj)
                    {
                        if (obj->carried_by != NULL)
                        {
                            rch = obj->carried_by;
                            act (skill_table[paf->type].msg_obj,
                                 rch, obj, NULL, TO_CHAR);
                        }
                        if (obj->in_room != NULL
                            && obj->in_room->people != NULL)
                        {
                            rch = obj->in_room->people;
                            act (skill_table[paf->type].msg_obj,
                                 rch, obj, NULL, TO_ALL);
                        }
                    }
                }

                affect_remove_obj (obj, paf);
            }
        }

		/*
		 * Oprog triggers!
		 */
		if ( obj->in_room || (obj->carried_by && obj->carried_by->in_room))
		{
			if ( HAS_TRIGGER_OBJ( obj, TRIG_DELAY )
			  && obj->oprog_delay > 0 )
			{
				if ( --obj->oprog_delay <= 0 )
				p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_DELAY );
			}
			else if ( ((obj->in_room && !obj->in_room->area->empty)
				|| obj->carried_by ) && HAS_TRIGGER_OBJ( obj, TRIG_RANDOM ) )
				p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_RANDOM );
		 }
		/* Make sure the object is still there before proceeding */
		if ( !obj )
			continue;
		
		
		/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */
 
        if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
        {		
					
			sh_int timerfrac = UMAX(1, obj->timer - 1);
			if ( obj->item_type == ITEM_CORPSE_PC )
				timerfrac = (int)(obj->timer / 8 + 1);
 
			if ( obj->timer > 0 && obj->value[2] > timerfrac )
			{
				char buf[MAX_STRING_LENGTH];
				char name[MAX_STRING_LENGTH];
				char *bufptr;
				bufptr = one_argument( obj->short_descr, name );
				bufptr = one_argument( bufptr, name ); 
				bufptr = one_argument( bufptr, name ); 
	 
				//separate_obj(obj);
				obj->value[2] = timerfrac; 
				sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ], 
							  bufptr ); 
	 
				free_string ( obj->description );
				obj->description = STRALLOC( buf ); 
			}  
			
			
			
        }
		
		
        if (obj->timer <= 0 || --obj->timer > 0)
		{
			if (obj->item_type == ITEM_TRACKS || obj->item_type == ITEM_BLOOD_POOL)
			{
				switch (obj->in_room->area->curr_precip_amount)
				{
					default: 
						if (number_percent() < 50)
							obj->timer--;
						break;
					case VERY_LIGHT:
					case LIGHT:
						obj->timer--;
						break;
					case MEDIUM:
					case HEAVY:
						obj->timer -= 2;
						break;
					case VERY_HEAVY:
						obj->timer -= 3;
						break;
				}
			}
            continue;
		}
			
        switch (obj->item_type)
        {
            default:
                message = "$p crumbles into dust.";
                break;
            case ITEM_FOUNTAIN:
                message = "$p dries up.";
                break;
			case ITEM_BLOOD_POOL:
				if (obj->in_room->area->curr_precip_amount > VERY_LIGHT)
					message = "$p washes away.";
				else
					message = "$p congeals, drying up.";
				break;
            case ITEM_CORPSE_NPC:
                //message = corpse_descs[number_range(0,4)
				message = "$p decays into dust.";
                break;
            case ITEM_CORPSE_PC:
                message = "$p decays into dust.";
                break;
            case ITEM_FOOD:
                message = "$p decomposes.";
                break;
            case ITEM_POTION:
                message = "$p has evaporated from disuse.";
                break;
            case ITEM_PORTAL:
                message = "$p fades out of existence.";
                break;
            case ITEM_CONTAINER:
                if (CAN_WEAR (obj, ITEM_WEAR_FLOAT))
				{
				    if (obj->contains)
                        message =
                            "$p flickers and vanishes, spilling its contents on the floor.";
                    else
                        message = "$p flickers and vanishes.";
				}
                else
                    message = "$p crumbles into dust.";
                break;
        }

        if (obj->carried_by != NULL)
        {
            if (IS_NPC (obj->carried_by)
                && obj->carried_by->pIndexData->pShop != NULL)
                obj->carried_by->silver += obj->cost / 5;
            else
            {
                act (message, obj->carried_by, obj, NULL, TO_CHAR);
                if (obj->wear_loc == WEAR_FLOAT)
                    act (message, obj->carried_by, obj, NULL, TO_ROOM);
            }
        }
        else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL)
        {
/*
            if (!(obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
                  && !CAN_WEAR (obj->in_obj, ITEM_TAKE)))
            {
                act (message, rch, obj, NULL, TO_ROOM);
                act (message, rch, obj, NULL, TO_CHAR);
            }
*/
        }

        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
            && obj->contains)
        {                        /* save the contents */
            OBJ_DATA *t_obj, *next_obj;

            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content;
                obj_from_obj (t_obj);

                if (obj->in_obj)    /* in another object */
                    obj_to_obj (t_obj, obj->in_obj);

                else if (obj->carried_by)    /* carried */
                    if (obj->wear_loc == WEAR_FLOAT)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj (t_obj);
                        else
                            obj_to_room (t_obj, obj->carried_by->in_room);
                    else
                        obj_to_char (t_obj, obj->carried_by);

                else if (obj->in_room == NULL)    /* destroy it */
                    extract_obj (t_obj);

                else            /* to a room */
                    obj_to_room (t_obj, obj->in_room);
            }
        }

        extract_obj (obj);
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update (void)
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for (wch = char_list; wch != NULL; wch = wch_next)
    {
		
        wch_next = wch->next;
        if (IS_NPC (wch)
            || wch->level >= LEVEL_IMMORTAL
            || wch->in_room == NULL || wch->in_room->area->empty) continue;

        for (ch = wch->in_room->people; ch != NULL; ch = ch_next)
        {
            int count;

            ch_next = ch->next_in_room;
			
            if (!IS_NPC (ch)
                || !IS_SET (ch->act, ACT_AGGRESSIVE)
                || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
                || IS_AFFECTED (ch, AFF_CALM)
                || ch->fighting != NULL || IS_AFFECTED (ch, AFF_CHARM)
                || !IS_AWAKE (ch)
                || (IS_SET (ch->act, ACT_WIMPY) && IS_AWAKE (wch))
                || !can_see (ch, wch) || number_bits (1) == 0)
                continue;

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count = 0;
            victim = NULL;
            for (vch = wch->in_room->people; vch != NULL; vch = vch_next)
            {
                vch_next = vch->next_in_room;

                if (!IS_NPC (vch)
                    && total_levels(vch) < LEVEL_IMMORTAL
                    && total_levels(ch) >= total_levels(vch) - 5
                    && (!IS_SET (ch->act, ACT_WIMPY) || !IS_AWAKE (vch))
                    && can_see (ch, vch)
					&& vch->grank == G_FRONT)
                {
                    if (number_range (0, count) == 0)
                        victim = vch;
                    count++;
                }
            }

            if (victim == NULL)
                continue;

            multi_hit (ch, victim, TYPE_UNDEFINED);
        }
    }

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

static int pulse_point;
 
 
  void sleep_update(void)
 {
     SLEEP_DATA *temp = first_sleep, *temp_next;
     
     for( ; temp != NULL; temp = temp_next)
     {
         bool delete = FALSE;
         
         temp_next = temp->next;
         
         /* checks to make sure the mob still exists*/
         if(!temp->mob)
             delete = TRUE;
         /*checks to make sure the character is still in the same room as the mob*/
         else if(temp->mob && temp->ch && temp->mob->in_room != temp->ch->in_room)
            delete = TRUE;
          if(delete)
         {
             /* some slick linked list manipulation */
             if(temp->prev)
                 temp->prev->next = temp->next;
             if(temp->next)
                 temp->next->prev = temp->prev;
             if( temp == first_sleep && (temp->next == NULL || temp->prev == NULL) )
                 first_sleep = temp->next;
             free_sleep_data(temp);
             continue;
         }
         
         if(--temp->timer <= 0)
         {
             program_flow(temp->vnum, temp->prog->code, temp->mob, NULL, NULL, temp->ch, NULL, NULL, temp->line);
             
             /* more slick linked list manipulation */
             if(temp->prev)
                 temp->prev->next = temp->next;
             if(temp->next)
                 temp->next->prev = temp->prev;
             if( temp == first_sleep && (temp->next == NULL || temp->prev == NULL) )
                 first_sleep = temp->next;
             free_sleep_data(temp);
         }
     }
 }
 
 
void update_handler (void)
{
    static int pulse_area;
    static int pulse_mobile;
    static int pulse_violence;
    //static int pulse_point;
    static int pulse_music;
	static int pulse_auction;	
	static int pulse_underwater;
	static int pulse_event;
	static int pulse_bleed;
	static int pulse_iron_will;
	static int pulse_spell_over_time;
	static int pulse_weather_effect;
	static int pulse_heal;
	static int pulse_climate;
	static int pulse_wind_effect;
	static int pulse_cooldown;
	static int pulse_water_damage;
	static int pulse_tree_age;
	static int pulse_achievement_update;
	static int pulse_birthday;
	static  int     pulse_msdp; /* <--- Add this line */
	
	OBJ_DATA *obj = NULL;
	
	if ( --pulse_msdp <= 0 )
    {
        pulse_msdp      = PULSE_PER_SECOND;
        msdp_update();
    }
	
	if ( --pulse_tree_age <= 0 )
	{
		for (obj = object_list; obj != NULL; obj = obj->next)
		{
			if (obj->item_type == ITEM_TREE && obj->value[1] < 4)
			{
				if (number_percent() < 3)
					obj->value[1] ++;
			}
		}	
		pulse_tree_age = PULSE_TREE_AGE;
	}
	
			
	if ( --pulse_achievement_update <= 0 )
	{
		pulse_achievement_update = PULSE_PER_SECOND * 5;
		achievement_update();
	}
			
	if ( --pulse_water_damage <= 0 )
	{
		pulse_water_damage = PULSE_WATER_DAMAGE;
		water_damage_update();
	}
	
	if ( --pulse_cooldown <= 0 )
	{
		pulse_cooldown = PULSE_COOLDOWN;
		cooldown_update();
	}
	
	if ( --pulse_birthday <= 0) 
	{
		pulse_birthday = 3;
		birthday_update();
    }
	
	if ( --pulse_heal <= 0 )
	{
		pulse_heal = PULSE_HEAL;
		regen_char_update();
	}	
	
	if (--pulse_wind_effect <= 0)
	{
		pulse_wind_effect = PULSE_WIND_EFFECT;
		wind_effect_update();
	}
	
	if ( --pulse_spell_over_time <= 0 )
	{
		//pulse_spell_over_time = PULSE_SPELL_OVER_TIME;
		//spell_over_time_update();
	}
	
	if ( --pulse_bleed <= 0)
	{
		pulse_bleed = PULSE_BLEED;		
		bleed_update();
	}
	
	if ( --pulse_weather_effect <= 0)
	{
		pulse_weather_effect = PULSE_WEATHER_EFFECT;
		weather_effect_update();
	}
	
	if ( --pulse_event <= 0)
    {
        pulse_event = PULSE_EVENT;
        event_update ();
    }

	if ( --pulse_underwater <= 0 )
    {
        pulse_underwater = PULSE_UNDERWATER;
        underwater_update( );		
    }
	
	if ( --pulse_iron_will <= 0 )
	{
		pulse_iron_will = PULSE_IRON_WILL;
		iron_will_update();
	}	


    if (--pulse_area <= 0)
    {
        pulse_area = PULSE_AREA;
        /* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
        area_update ();
    }

    if (--pulse_music <= 0)
    {
        pulse_music = PULSE_MUSIC;
        song_update ();
    }

    if (--pulse_mobile <= 0)
    {
        pulse_mobile = PULSE_MOBILE;
        mobile_update ();
    }

    if (--pulse_violence <= 0)
    {
        pulse_violence = PULSE_VIOLENCE;
        violence_update ();
    }	

	if (--pulse_climate <= 0)
	{
		pulse_climate = PULSE_CLIMATE;
		weather_update ();
	}
	
    if (--pulse_point <= 0)
    {
        wiznet ("TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
        pulse_point = PULSE_TICK;
        char_update ();
        obj_update ();
    }
	
	sleep_update ();
    aggr_update ();
    tail_chain ();
    return;
}



/* For time/dates */
char *const day_name2[] = {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};
//0 1 2 3 4 winter
// 5 6 7 8 spring
// 9 10 11 summer
// 12 13 14 15 16 fall
char *const month_name2[] = {
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void achievement_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;      

	for ( ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;
		
		if (ch == NULL)
			continue;
	
		if (ch->guildpoints >= RANK_APPRENTICE)
			ch->rank_apprentice = TRUE;
		if (ch->guildpoints >= RANK_MEMBER)
			ch->rank_member = TRUE;
		if (ch->guildpoints >= RANK_OFFICER)
			ch->rank_officer = TRUE;
		if (ch->guildpoints >= RANK_MASTER)
			ch->rank_master = TRUE;
		
		// if (ch->pcdata->explored->set)
		// {
			// if (ch->pcdata->explored->set >= 500)
				// ch->explore_500 = TRUE;
			// if (ch->pcdata->explored->set >= 2500)
				// ch->explore_2500 = TRUE;
			// if (ch->pcdata->explored->set >= 10000)
				// ch->explore_10000 = TRUE;
		// }
		
		if (ch->pkill >= 1)
			ch->player_1 = TRUE;
		if (ch->pkill >= 10)
			ch->player_10 = TRUE;
		if (ch->pkill >= 50)
			ch->player_50 = TRUE;
		if (ch->pkill >= 250)
			ch->player_250 = TRUE;
		if (ch->pkill >= 500)
			ch->player_500 = TRUE;
		
		if (ch->gold >= 1000)
			ch->gold_1000 = TRUE;
		if (ch->bank_amt >= 1000)
			ch->bank_1000 = TRUE;
		if (ch->bank_amt >= 10000)
			ch->bank_10000 = TRUE;
		if (ch->bank_amt >= 50000)
			ch->bank_50000 = TRUE;
	}
	return;
}


void water_damage_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;      
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;	
	static int rust_count;	
		
	for ( ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		//Took this out so people can't have their
		//pets carry around shit in the water without
		//fear of water damage.
		
		if (/*IS_NPC(ch) ||*/ !ch)		
			continue;
		
		if (!IN_WATER_SECTOR(ch))
			continue;
		
		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			
			if (!obj->in_obj)
			{
			
				if (obj->item_type == ITEM_PAPER)
				{
					
				}			
				
				if (obj->item_type == ITEM_SCROLL)
				{
					if (obj->value[1] > 0)
					{
						if (number_percent() < 20)
							obj->value[1] = -1;
					}
					if (obj->value[2] > 0)
					{
						if (number_percent() < 20)
							obj->value[2] = -1;
					}
					if (obj->value[3] > 0)
					{
						if (number_percent() < 20)
							obj->value[3] = -1;
					}
					if (obj->value[4] > 0)
					{
						if (number_percent() < 20)
							obj->value[4] = -1;
					}				
				}
				
				if (IS_METAL(obj->material))
				{
					if (++rust_count > 12)
					{
						rust_count = 0;
						SET_BIT(obj->extra2_flags, ITEM_RUSTED);
						obj->condition /= 2;
					}
				}
			}
		}
	}
	return;
}

void cooldown_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;      
	int i = 0;

	for ( ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		if (IS_NPC(ch) || !ch)		
			continue;
		
		for (i = 0; i <= MAX_SKILL; i++)
		{
			if (ch->cooldowns[i] > 0)
				ch->cooldowns[i]--;
		}
	}
	return;
}

void regen_char_update(void)
{

   CHAR_DATA *ch;
   CHAR_DATA *ch_next;      

   for ( ch = char_list; ch != NULL; ch = ch_next)
   {
		ch_next = ch->next;
				
		//Can't be fighting or bleeding a lot to regen.
		if (ch->fighting == NULL  && ch->position != POS_FIGHTING && ch->bleeding < 4)
		{			

			if (ch->hit < ch->max_hit)
			{
                ch->hit += hit_gain (ch);
				if (ch->hit > ch->max_hit)
					ch->hit = ch->max_hit;
            }

            if (ch->mana < ch->max_mana)
			{
                ch->mana += mana_gain (ch);
				if (ch->mana > ch->max_mana)
					ch->mana = ch->max_mana;
            }

			if (ch->pp < ch->max_pp)
			{
                ch->pp += talent_gain (ch);
				if (ch->pp > ch->max_pp)
					ch->pp = ch->max_pp;
            }
			
            if (ch->move < ch->max_move)
			{
                ch->move += move_gain (ch);
				if (ch->move > ch->max_move)
					ch->move = ch->max_move;
            }
		}
		
		if (ch->pk_timer > 0)
			ch->pk_timer -= 1;
	}
	
	return;
}

//This function is for the effect of wind.
void wind_effect_update(void)
{	
/*
	EXIT_DATA *pexit;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *to_room;
	ROOM_INDEX_DATA *in_room;
	int vnum;
	
	
	
	for ( vnum; vnum < top_obj_index; vnum++ )
    {
		if ( ( obj = get_obj_index( vnum ) ) != NULL )
		{
			
			if (IS_SET(pObj->extra2_flags, ITEM_BUOYANT))
			{
				if ( obj->in_room->sector_type == SECT_OCEANFLOOR || obj->in_room->sector_type == SECT_WATER_SWIM || obj->in_room->sector_type == SECT_WATER_NOSWIM )
				{
					bool exit_found = FALSE;
					int door = 0;				
					
					in_room = obj->in_room;
					
					while (exit_found == FALSE) 
					{
						door = number_range (0,3);
						
						if ((pexit = in_room->exit[door]) == NULL
							|| (to_room = pexit->u1.to_room) == NULL || IS_SET (pexit->exit_info, EX_CLOSED))
						{
							continue;
						}					
						
						exit_found = TRUE;
						break;
					}
					
					act ("$p bobs $T.", NULL, obj, dir_name[door], TO_ROOM);								
					obj_from_room(obj);
					obj_to_room(obj, pexit->u1.to_room);
					act ("$p bobs in from the $T.", NULL, obj, dir_name[door], TO_ROOM);				
					
				}
			}
		}
	}
*/
	return;
	
}

/* This function will control all the 'physics' of the effects of nature on players and items in the game. */
void weather_effect_update(void)
{

   CHAR_DATA *ch;
   CHAR_DATA *ch_next;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   char buf[MSL];   

   for ( ch = char_list; ch != NULL; ch = ch_next)
   {
		ch_next = ch->next;
		
		if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
		{			
			//Put out non magical torches in rain or snow.
			
			if (ch->in_room->area->curr_precip == RAIN || ch->in_room->area->curr_precip == SNOW)
			{
				for (obj = ch->carrying; obj != NULL; obj = obj_next)
				{
					obj_next = obj->next_content;
					if (obj->wear_loc != WEAR_NONE && (obj->item_type == ITEM_LIGHT))
					{
						if (obj->value[1] == 1)
							return;
							
						if (obj->value[2] < 0 && (number_percent() + (ch->in_room->area->curr_precip_amount * 10) > 75 ))
						{							
							obj->value[2] = 0;
							sprintf(buf, "Your %s extinguishes due to weather.\r\n",obj->short_descr);
							SEND(buf, ch);
							break;
						}

					}
				}
			}
			
			// End torch
		}
	}	
	
	return;
}

void underwater_update( void )
{
   CHAR_DATA *ch;
   CHAR_DATA *ch_next;
   char buf[MAX_STRING_LENGTH];
   int dam = 0;

   for ( ch = char_list; ch != NULL; ch = ch_next)
   {
		ch_next = ch->next;
		
		if (!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->in_room->sector_type == SECT_OCEANFLOOR )
		{
		 
			 if ( ch->hit >= 15)
			 {      
				dam = (number_range (5,10) + number_range(1, ch->level / 2));		
				
				if (ch->in_room->sector_type == SECT_OCEANFLOOR)
				{
					dam += (dam * 4/5);
					sprintf (buf, "You are drowning! The pressure of water upon your body is enormous! {r[{x%d{r]{x\r\n", dam);					
					SEND (buf, ch);			
				}
				else
				{
					sprintf (buf, "You are drowning! {r[{x%d{r]{x\r\n", dam);	  
					SEND (buf, ch);			
				}
				
				ch->hit -= dam;		
			 }
			 else
			 {
				ch->hit = 1;
				raw_kill(ch);
				SEND("You are DEAD!!\r\n", ch );
			 }
		}
   }
}

void iron_will_update (void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;        

		if (!ch->in_room)
			continue;		
			
		if ( ch->hit < 1 && IS_MONK(ch) )
		{
			if (KNOWS(ch, gsn_iron_will) && check_iron_will(ch))
			{
				SEND ("Your body continues to ignore the pull of death.\r\n",ch);
				return;
			}
			else
			{
				SEND ("Your body can no longer resist death.\r\n",ch);
				ch->position = POS_DEAD;
				raw_kill (ch);
				update_pos(ch);
				return;
			}
		}
	}
}

void spell_over_time_update(void)
{
	return;
}

void bleed_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
	OBJ_DATA *blood = NULL;	
	
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;        

		if (!ch->in_room)
			continue;		
			
		if (ch->bleeding > 0)
		{			
			
			//Take less damage at lower levels.
			if (total_levels(ch) < 9)
				ch->hit -= 1;
			else
				ch->hit -= number_range(1, ch->bleeding);
			
			
			if (!get_obj_list_by_type (ch, ITEM_BLOOD_POOL, ch->in_room->contents))            
            {
					blood = create_object (get_obj_index (OBJ_VNUM_BLOOD), 0);
					//scroll->value[0] = number_fuzzy(ch->level);
					blood->timer = 2;
					obj_to_room ( blood, ch->in_room );
            }			
			
			if (ch->bleeding > 0 && ch->bleeding < 3)
			{
				SEND("You drip a bit of blood on the ground.\r\n",ch);
				act ( "$n drips blood from an open wound.",ch,NULL,NULL,TO_ROOM);
				blood = get_obj_list (ch, "blood", ch->in_room->contents);
				if (blood != NULL)
				{				
						blood->timer++;						
				}			
			}
			if (ch->bleeding > 2 && ch->bleeding < 6)
			{
				SEND("Blood flows from your wound.\r\n",ch);
				act ( "Blood flows from $n's open wound.",ch,NULL,NULL,TO_ROOM);
				blood = get_obj_list (ch, "blood", ch->in_room->contents);
				if (blood != NULL)
				{				
						blood->timer++;						
				}			
			}
			if (ch->bleeding > 5 && ch->bleeding < 10)
			{
				SEND("Massive amounts of blood pour from your wound.\r\n",ch);
				act ( "$n drips massive amounts of blood from their wound.",ch,NULL,NULL,TO_ROOM);
				blood = get_obj_list (ch, "blood", ch->in_room->contents);
				if (blood != NULL)
				{				
						blood->timer+=2;						
				}			
			}			
			if (ch->hit < 1)
			{				
				if (IS_IMMORTAL(ch))
				{
					ch->hit = 1;
					return;
				}
				
				if (IS_NPC(ch))	
				{
					ch->hit = 1;
					return;					
				}				
				else
				{
					if (IS_KNOWN(ch, gsn_iron_will))
					{
						if (check_iron_will(ch))
						{				
							SEND ("Your body continues to ignore the pull of death.\r\n",ch);
							return;
						}
						else
						{						
							SEND ("Your body can no longer resist death.\r\n",ch);
							SEND ("You have {RDIED{x from blood loss!!\r\n", ch );															
							ch->position = POS_DEAD;
							ch->fighting = NULL;
							raw_kill (ch);
							update_pos(ch);
							return;
						}
					}
					else
					{						
						SEND("You have {RDIED{x from blood loss!!\r\n", ch );								
						stop_fighting(ch, FALSE);
						raw_kill(ch);				
					}
				}
			
			}
		
		}
		return;
	}
}


void do_ftick (CHAR_DATA * ch, char *argument)
{
	pulse_point = 1;
	update_handler();
	return;
}




/***************************************************************************
 * File: update.c
 * 
 * Add a new msdp_update() function.
 ***************************************************************************/

void msdp_update( void )
{
    DESCRIPTOR_DATA *d;
    int PlayerCount = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character && d->connected == CON_PLAYING && !IS_NPC(d->character) )
        {
            char buf[MAX_STRING_LENGTH];
            CHAR_DATA *pOpponent = d->character->fighting;
            ROOM_INDEX_DATA *pRoom = d->character->in_room;
            AFFECT_DATA *paf;

            ++PlayerCount;

            MSDPSetString( d, eMSDP_CHARACTER_NAME, d->character->name );
            MSDPSetNumber( d, eMSDP_ALIGNMENT, d->character->alignment );
            MSDPSetNumber( d, eMSDP_EXPERIENCE, d->character->exp );
/*
            MSDPSetNumber( d, eMSDP_EXPERIENCE_MAX, TBD );
            MSDPSetNumber( d, eMSDP_EXPERIENCE_TNL, TBD );
*/
            MSDPSetNumber( d, eMSDP_HEALTH, d->character->hit );
            MSDPSetNumber( d, eMSDP_HEALTH_MAX, d->character->max_hit );
            MSDPSetNumber( d, eMSDP_LEVEL, d->character->level );
/*
            MSDPSetNumber( d, eMSDP_RACE, TBD );
            MSDPSetNumber( d, eMSDP_CLASS, TBD );
*/
            MSDPSetNumber( d, eMSDP_MANA, d->character->mana );
            MSDPSetNumber( d, eMSDP_MANA_MAX, d->character->max_mana );
            MSDPSetNumber( d, eMSDP_WIMPY, d->character->wimpy );
            MSDPSetNumber( d, eMSDP_PRACTICE, d->character->practice );
            MSDPSetNumber( d, eMSDP_MONEY, d->character->gold );
            MSDPSetNumber( d, eMSDP_MOVEMENT, d->character->move );
            MSDPSetNumber( d, eMSDP_MOVEMENT_MAX, d->character->max_move );
            MSDPSetNumber( d, eMSDP_HITROLL, GET_HITROLL(d->character) );
            MSDPSetNumber( d, eMSDP_DAMROLL, GET_DAMROLL(d->character) );
            //MSDPSetNumber( d, eMSDP_AC, GET_AC(d->character) );
            MSDPSetNumber( d, eMSDP_STR, GET_STR(d->character) );
            MSDPSetNumber( d, eMSDP_INT, GET_INT(d->character) );
            MSDPSetNumber( d, eMSDP_WIS, GET_WIS(d->character) );
            MSDPSetNumber( d, eMSDP_DEX, GET_DEX(d->character) );
			MSDPSetNumber( d, eMSDP_CON, GET_CON(d->character) );
            MSDPSetNumber( d, eMSDP_STR_PERM, BASE_STR(d->character) );
            MSDPSetNumber( d, eMSDP_INT_PERM, BASE_INT(d->character) );
            MSDPSetNumber( d, eMSDP_WIS_PERM, BASE_WIS(d->character) );
            MSDPSetNumber( d, eMSDP_DEX_PERM, BASE_DEX(d->character) );
            MSDPSetNumber( d, eMSDP_CON_PERM, BASE_CON(d->character) );

            /* This would be better moved elsewhere */
            if ( pOpponent != NULL )
            {
                int hit_points = (pOpponent->hit * 100) / pOpponent->max_hit;
                MSDPSetNumber( d, eMSDP_OPPONENT_HEALTH, hit_points );
                MSDPSetNumber( d, eMSDP_OPPONENT_HEALTH_MAX, 100 );
                MSDPSetNumber( d, eMSDP_OPPONENT_LEVEL, pOpponent->level );
                MSDPSetString( d, eMSDP_OPPONENT_NAME, pOpponent->name );
            }
            else /* Clear the values */
            {
                MSDPSetNumber( d, eMSDP_OPPONENT_HEALTH, 0 );
                MSDPSetNumber( d, eMSDP_OPPONENT_LEVEL, 0 );
                MSDPSetString( d, eMSDP_OPPONENT_NAME, "" );
            }

            /* Only update room stuff if they've changed room */
            if ( pRoom && pRoom->vnum != d->pProtocol->pVariables[eMSDP_ROOM_VNUM]->ValueInt )
            {
                int i; /* Loop counter */
                buf[0] = '\0';

                for ( i = DIR_NORTH; i < MAX_DIR; ++i )
                {
                    if ( pRoom->exit[i] != NULL )
                    {
                        const char MsdpVar[] = { (char)MSDP_VAR, '\0' };
                        const char MsdpVal[] = { (char)MSDP_VAL, '\0' };
                        extern char *const dir_name[];

                        strcat( buf, MsdpVar );
                        strcat( buf, dir_name[i] );
                        strcat( buf, MsdpVal );

                        if ( IS_SET(pRoom->exit[i]->exit_info, EX_CLOSED) )
                            strcat( buf, "C" );
                        else /* The exit is open */
                            strcat( buf, "O" );
                    }
                }

                if ( pRoom->area != NULL )
                    MSDPSetString( d, eMSDP_AREA_NAME, pRoom->area->name );

                MSDPSetString( d, eMSDP_ROOM_NAME, pRoom->name );
                MSDPSetTable( d, eMSDP_ROOM_EXITS, buf );
                MSDPSetNumber( d, eMSDP_ROOM_VNUM, pRoom->vnum );
            }
/*
            MSDPSetNumber( d, eMSDP_WORLD_TIME, d->character-> );
*/

            buf[0] = '\0';
            for ( paf = d->character->affected; paf; paf = paf->next )
            {
                char skill_buf[MAX_STRING_LENGTH];
                sprintf( skill_buf, "%c%s%c%d",
                    (char)MSDP_VAR, skill_table[paf->type].name, 
                    (char)MSDP_VAL, paf->duration );
                strcat( buf, skill_buf );
            }
            MSDPSetTable( d, eMSDP_AFFECTS, buf );

            MSDPUpdate( d );
        }
    }

    /* Ideally this should be called once at startup, and again whenever 
     * someone leaves or joins the mud.  But this works, and it keeps the 
     * snippet simple.  Optimise as you see fit.
     */
    MSSPSetPlayers( PlayerCount );
}


void birthday_update(void)
{
    DESCRIPTOR_DATA *d;
    OBJ_DATA       *obj;
	int day;
	char *suf;
	char buf[MSL];
	int chance = 0;

    for (d = descriptor_list; d != NULL; d = d->next) 
	{
		if (d->connected != CON_PLAYING || d->character == NULL || IS_NPC(d->character)) 
			continue;

		if ((d->character->played + (int) (current_time - d->character->logon)) % 72000 == 0) 
		{    	    			
			SEND ("You have aged another year, today is your birthday!\r\n", d->character);
			d->character->age++;
			//If they're getting old, check to see if they die.
			if ((GET_AGE(d->character) > pc_race_table[d->character->race].max_age) * 9/10)
			{
				chance = (number_range(1,100));
				//high con allows you to live longer.
				if (get_curr_stat (d->character, STAT_CON) > 14)
					chance += get_curr_stat (d->character, STAT_CON) - 14;
				//low con gives possibility of early death.
				if (get_curr_stat (d->character, STAT_CON) < 11)
					chance -= 12 - get_curr_stat (d->character, STAT_CON);
				//closer you are to max_age, easier it is to die.
				if ((GET_AGE(d->character) > pc_race_table[d->character->race].max_age) * 19/20)
					chance -= 5;
				if (GET_AGE(d->character) > pc_race_table[d->character->race].max_age)
					chance -= 10;
				if (chance < 5)
				{
					if (!IS_IMMORTAL(d->character))
					{
						SEND("This character has lived a long life, but alas, has passed.\r\n", d->character);
						
						
						day = time_info.day + 1;
						 if (day > 4 && day < 20)
							suf = "th";
						else if (day % 10 == 1)
							suf = "st";
						else if (day % 10 == 2)
							suf = "nd";
						else if (day % 10 == 3)
							suf = "rd";
						else
							suf = "th";
						
						sprintf (buf, "[DEATH] (Player): %s (Time): At %d o'clock %s, Day of %s, %d%s the 	Month of %s.\r\n",
						d->character->name, (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
						time_info.hour >= 12 ? "pm" : "am", day_name2[day % 7], day, suf, month_name2[time_info.month]);
						log_string (buf);
						do_freeze(d->character, "");
						return;
					}
				}
			}
			
			create_random_obj( d->character, NULL, NULL, total_levels(d->character), "gem", str_dup(mat_table[get_gem(number_range(1,4))].material), TARG_MOB, "");							    
			SEND ("An Immortal gives you a present for your birthday!\r\n", d->character);
		}   	 
		else if ((d->character->played + (int) (current_time - d->character->logon)) == 777 * 60 * 60 && !IS_EVIL(d->character)) 
		{
			SEND("You've hit the {YJACKPOT{x! Congratulations!\r\n", d->character);
			SEND("You feel somewhat {Yholy{x.\r\n", d->character);
			d->character->train += 1;
		}
		else if ((d->character->played + (int) (current_time - d->character->logon)) == 666 * 60 * 60) 
		{
			SEND ("You feel a bit more vile... should you be rewarded?\r\n", d->character);
			if (IS_EVIL(d->character)) 
			{
				SEND ("You feel justified in your {Ddarkest{x time.\r\n", d->character);
				d->character->train += 1;
			}
		}
    }
    return;
}