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
#include <unistd.h>                /* For execl in copyover() */
#include "merc.h"
#include "interp.h"

//Upros take on clans, Jan 2020

void do_guild (CHAR_DATA * ch, char *argument)
{
    char arg1[MSL], arg2[MIL];
    char buf[MSL];
    CHAR_DATA *victim;
	OBJ_DATA *obj;
    int clan;
	

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        SEND ("Syntax: guild <char> <cln name/none>\r\n", ch);
        return;
    }
    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("They aren't playing.\r\n", ch);
        return;
    }
	
    if (!str_prefix (arg2, "none") && IS_IMMORTAL(ch))
    {        
		SEND ("They are now clanless.\r\n", ch);
        SEND ("You are now a member of no clan!\r\n", victim);
		if (total_levels(ch) > 14)
			victim->clan = 1;
		else
			victim->clan = 0;		
        return;
    }

    if ((clan = clan_lookup (arg2)) == 0)
    {
        SEND ("No such clan exists.\r\n", ch);
        return;
    }
	if (victim->clan = CLAN_LONER)
	{
		if (IS_IMMORTAL(ch) || (ch->clan == clan && ch->clan_rank > 2))
		{
			sprintf (buf, "They are now a member of clan %s.\r\n",
				 capitalize (clan_table[clan].name));
			SEND (buf, ch);
			sprintf (buf, "You are now a member of clan %s.\r\n",
				 capitalize (clan_table[clan].name));

			if (clan_table[clan].independent == FALSE)
			{
				SEND(buf, victim);		 
				obj = create_object (get_obj_index(clan_table[clan].item), 0);        
				obj_to_char (obj, victim);
				victim->clan_rank = 0;
			}

			victim->clan = clan;
		}
	}
}

void do_rank (CHAR_DATA * ch, char *argument)
{
	
	char arg1[MIL], arg2[MIL];
    char buf[MSL];
    CHAR_DATA *victim;
	OBJ_DATA *obj;
    int clan;
	

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        SEND ("Syntax: rank <char> <rank>\r\n", ch);
        return;
    }
	
    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        SEND ("They aren't playing.\r\n", ch);
        return;
    }
	if (!is_same_clan(ch, victim) && total_levels(ch) < 47)
	{
		SEND ("They are not a part of your clan.\r\n",ch);
		return;
	}
	
	if (atoi(arg2) > -1 || atoi(arg2) < 6)
	{
		if (ch->clan_rank < 3 && total_levels(ch) < 47)
		{
			SEND("You cannot rank other members.\r\n",ch);
			return;
		}
		
		if (victim->clan_rank == 5)
		{
			SEND("Good luck doing that to your leader!\r\n",ch);
			return;
		}
		
		if (total_levels(ch) < 47)
		{
			if (atoi(arg2) < victim->clan_rank && ch->clan_rank < 4)
			{
				SEND("You don't have the power to do that.\r\n",ch);
				return;
			}		
		
			if (atoi(arg2) > 1 && ch->clan_rank < 4)
			{
				SEND ("You need to be a higher rank to do that!\r\n",ch);
				return;
			}
		
			if (atoi(arg2) > 3 && ch->clan_rank < 5)
			{
				SEND ("Only the leader can do that.\r\n",ch);
				return;
			}
		}
		
			victim->clan_rank = atoi(arg2);
			sprintf(buf, "You change %s's clan rank to %d!\r\n",victim->name, victim->clan_rank);
			SEND(buf, ch);
			sprintf(buf, "%s changes your clan rank to %d!\r\n",ch->name, victim->clan_rank);
			SEND(buf, victim);
	}
	else
	{
		SEND("Clan ranks run 0 through 5.\r\n",ch);
		return;
	}
	return;
}


bool is_clan(CHAR_DATA *ch)
{
	if (ch->clan > CLAN_NONE)
		return TRUE;
	
	return FALSE;
}

