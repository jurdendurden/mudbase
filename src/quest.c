#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <malloc.h>

#include "merc.h"
#include "recycle.h"

extern QUEST_DATA *quest_free;

void show_quest_log_to_char args((QUEST_DATA * list, CHAR_DATA * ch));


//free quest_data
QUEST_DATA *quest_free;

//Allocate memory for a new quest object.
QUEST_DATA *new_pc_quest (void)
{
    static QUEST_DATA quest_zero;
    QUEST_DATA *quest;

    if (quest_free == NULL)
        quest = alloc_perm (sizeof (*quest));
    else
    {
        quest = quest_free;
        quest_free = quest_free->next_quest;
    }
    *quest = quest_zero;
    VALIDATE (quest);

    return quest;
}


void free_pc_quest	(QUEST_DATA *pQuest)
{
	pQuest->next_quest = quest_free;
	quest_free = pQuest;
	return;
}



//Create a new quest instance for a player from the specified quest index.
QUEST_DATA *create_quest (QUEST_INDEX_DATA *pQuestIndex)
{
	QUEST_DATA *quest;
	
	 if (pQuestIndex == NULL)
    {        	
		bug ("Create_quest: NULL pQuestIndex.", 0);		
        exit (1);
    }

    quest = new_pc_quest ();

    quest->pIndexData = pQuestIndex;
    quest->name = str_dup (pQuestIndex->name);        	
	quest->vnum = pQuestIndex->vnum;
		
	quest->vnum = pQuestIndex->vnum;
	quest->obj_multiples = 0;
	quest->mob_multiples = 0;
	quest->mat_multiples = 0;
	quest->herb_multiples = 0;		
	quest->has_read_helps[0] = 0;
	quest->has_read_helps[1] = 0;
	quest->has_read_helps[2] = 0;
	quest->time_limit = 0;
	quest->completed = 0;
	
	int i;
	
	/*for (i = 0; i < 4; i++)
	{
		quest->has_read_helps[i] = FALSE;
	}*/

	quest->completed = FALSE;

	quest->pIndexData->next = quest_list;
    quest_list = quest->pIndexData;    
	pQuestIndex++;
	
	return quest;
}


//Attach a quest to a character.
void quest_to_char (QUEST_DATA * quest, CHAR_DATA *ch)
{	
	quest->next_quest = ch->quests;
	ch->quests = quest;
	quest->taken_by = ch;
	return;
}




bool can_take_quest (CHAR_DATA *ch, QUEST_INDEX_DATA *quest)
{
	if (total_levels(ch) < quest->min_level)
		return FALSE;
		
	if (total_levels(ch) > quest->max_level && quest->max_level)
		return FALSE;
		
	if (quest->race_type && ch->race != quest->race_type)
		return FALSE;
		
	return TRUE;
}

bool has_read_helps (CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest)
{
	QUEST_DATA *quest;
	
	for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
	{
		if (quest->vnum == pQuest->vnum)
		{
			if (pQuest->help1 && !quest->has_read_helps[0])
				return FALSE;
			if (pQuest->help2 && !quest->has_read_helps[1])
				return FALSE;
			if (pQuest->help3 && !quest->has_read_helps[2])
				return FALSE;
				
			return TRUE;
		}
	}
	return FALSE;
}

long mob_has_quest (CHAR_DATA *mob)
{
	QUEST_INDEX_DATA *quest;
	int vnum = 0;

	if (!mob)
		return -1;
	
	if (!IS_NPC(mob))
		return -1;
	
	/*for (quest = quest_list ; quest != NULL; quest = quest->next)
	{
		if (quest->accept_vnum == mob->pIndexData->vnum)
			return quest->vnum;
	}*/
	
	for (vnum = mob->pIndexData->area->min_vnum; vnum <= mob->pIndexData->area->max_vnum; vnum++)
    {
		//Quest code commented out for now as it is buggy 1/13/2020 Upro
        /*if ((quest = get_quest_index (vnum)) != NULL && mob->pIndexData && (quest->accept_vnum == mob->pIndexData->vnum))
        {
			return quest->vnum;
		}*/
	}
	return -1;
}

bool on_quest (CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest)
{	
	QUEST_DATA *quest;

	for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
	{
		if (quest->vnum == pQuest->vnum)
			return TRUE;
		else
			continue;
	}
	
	return FALSE;
}
//Set a quest as completed.
void complete_quest (CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest)
{
	QUEST_DATA *quest;
	
	for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
	{
		if (quest->vnum == pQuest->vnum && quest->completed == FALSE)
		{
			quest->completed = TRUE;
			return;
		}
	}
}
//Return if they've completed it or not.
bool has_completed_quest (CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest)
{	
	QUEST_DATA *quest;

	for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
	{
		if (quest->vnum == pQuest->vnum && quest->completed == TRUE)
			return TRUE;
	}
	
	return FALSE;
}
//See if they've completed what they need to for the quest to be complete.
bool quest_complete (CHAR_DATA *ch, QUEST_INDEX_DATA *pQuest)
{	
	QUEST_DATA *quest;

	for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
	{
		if (quest->vnum == pQuest->vnum)
		{	
			if (IS_SET (pQuest->type, D)) //help_read
			{
				if (!has_read_helps(ch, pQuest))
					return FALSE;
			}
			
			if (IS_SET (pQuest->type, A)) //mob_kill
			{
				if (quest->mob_multiples < pQuest->mob_mult)
					return FALSE;
			}
			
			if (IS_SET (pQuest->type, B)) //item_retrieve
			{
				if (quest->obj_multiples < pQuest->obj_mult)
					return FALSE;
			}
			break;
		}
	}
	
	return TRUE;
}

void do_quest (CHAR_DATA *ch, char *argument)
{
	char arg1[MIL];
	char arg2[MIL];
	char buf[MSL];
	CHAR_DATA *quest_mob;
	QUEST_INDEX_DATA *pQuest;
	bool found = FALSE;
	QUEST_DATA *quest;
	
	argument = one_argument (argument, arg1);	
	argument = one_argument (argument, arg2);
	
	if (arg1[0] == '\0')
	{
		SEND("Syntax: quest <request/complete/info/log/abandon>",ch);
		return;
	}
	
	for (quest_mob = ch->in_room->people; quest_mob != NULL; quest_mob = quest_mob->next_in_room)
	{
		if (IS_NPC (quest_mob) && (mob_has_quest(quest_mob) > 0))
		{
			found = TRUE;
			break;
		}					
	}

	if (!str_cmp(arg1, "log"))
	{
		show_quest_log_to_char (ch->quests, ch);
		return;
	}
	
	if (!str_cmp(arg1, "complete"))
	{
		if (!found)
		{
			SEND("There's no one to turn that quest in to here.\r\n",ch);
			return;
		}
	
		if (!on_quest(ch, get_quest_index(mob_has_quest(quest_mob))))
		{
			SEND("You're not on that quest.\r\n",ch);
			return;
		}
		if (!quest_complete(ch, get_quest_index(mob_has_quest(quest_mob))))
		{
			SEND("You're not finished completing that quest.\r\n",ch);
			return;
		}
		
		pQuest = get_quest_index(mob_has_quest(quest_mob));
		
		sprintf(buf, "You complete the quest {g%s{x!\r\n", pQuest->name);
		SEND(buf, ch);
		if (pQuest->gp_reward)
		{
			ch->guildpoints += pQuest->gp_reward;
			sprintf(buf, "You receive {B%d{x guild points!\r\n", pQuest->gp_reward);
			SEND(buf, ch);
		}
		
		if (pQuest->xp_reward)
		{
			gain_exp(ch, pQuest->xp_reward, FALSE);
			sprintf(buf, "You receive {B%d{x experience points!\r\n", pQuest->xp_reward);
			SEND(buf, ch);
		}
		
		if (pQuest->gold_reward)
		{
			ch->gold += pQuest->gold_reward;
			sprintf(buf, "You receive {Y%d{x gold pieces!\r\n", pQuest->gold_reward);
			SEND(buf, ch);
		}
		
		
		ch->exp += pQuest->xp_reward;
		complete_quest(ch, pQuest);
		return;
	}
	
	if (!str_cmp(arg1, "abandon"))
	{
		SEND("quest abandonment needs implementation.\r\n",ch);
		return;
	}	
	
	if (!str_cmp(arg1, "request"))
	{
		if (!found)
		{
			SEND("You can't get any quests here.\r\n",ch);
			return;
		}
		
		if (on_quest(ch, get_quest_index(mob_has_quest(quest_mob))))
		{
			if (!has_completed_quest(ch,get_quest_index(mob_has_quest(quest_mob))))
			{
				SEND("You're already on that quest.\r\n",ch);
				return;
			}
			else
			{
				if (!IS_SET(get_quest_index(mob_has_quest(quest_mob))->type, F)) //F = repeatable.
				{
					SEND("You've already completed that quest.\r\n",ch);
					return;
				}
			}
		}
		
		if (!can_take_quest(ch, get_quest_index(mob_has_quest(quest_mob))))
		{
			SEND("You're not eligible for that quest.\r\n",ch);
			return;
		}
		else
		{
			quest = create_quest(get_quest_index(mob_has_quest(quest_mob)));
			quest_to_char(quest, ch);
			sprintf(buf,"You accept the quest: {g%s{x!\r\n", quest->name);
			SEND(buf, ch);
			return;		
		}
	}
	
	SEND("Syntax: quest <request/complete/info/log/abandon>\r\n",ch);
	return;
}
