#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
	


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

				
bool nifty_is_name_prefix args(( char *str, char *namelist ));
char *fread_string_nohash args(( FILE * fp ));
SHIP_DATA *new_ship 	  args((void));
		

SHIP_DATA *ship_first;
SHIP_DATA *ship_last;
		
		
void save_ship_list ()
{
    FILE *fp;
    SHIP_DATA *ship;        

    if ((fp = fopen (SHIP_LIST, "w")) == NULL)
    {
        bug ("save_ship_list: fopen", 0);
        perror ("ships.txt");
    }
    else
    {
        for (ship = ship_first; ship; ship = ship->next)
        {
            fprintf (fp, "%s\n", ship->file_name);
        }

        fprintf (fp, "$\n");
        fclose (fp);
    }

    return;
}
		
int num_ships()
{
	AREA_DATA * pArea;
	int i = 0;
	
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		if (IS_SET(pArea->area_flags, AREA_SHIP))
		{
			i++;
		}
	}
	return i;	
}
		
bool check_existing_ship(char * argument)
{
	AREA_DATA * pArea;
	
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		if (IS_SET(pArea->area_flags, AREA_SHIP))
		{
			if (!str_cmp(pArea->name, argument))
			{
				return TRUE;
				break;
			}
		}
	}

	return FALSE;
}

				
AREA_DATA * get_ship_area (char *argument)
{
	AREA_DATA * pArea;	
	
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		if (IS_SET(pArea->area_flags, AREA_SHIP))
		{
			if (!str_cmp(pArea->name, argument))
			{
				return pArea;
				break;
			}
		}
	}

	return NULL;
}
				
void do_shipstat ( CHAR_DATA *ch, char *argument )
{
/*
	//char buf[MAX_STRING_LENGTH];
	
	if IS_NPC(ch)	
		return;
		
	if (!ch->ship)
	{
		SEND("You don't even own a ship.\r\n",ch);
		return;
	}
	
	
	*/
}

void do_disembark (CHAR_DATA *ch, char *argument )
{
	//AREA_DATA *area;
	ROOM_INDEX_DATA *location;
	
	if (!ch->onBoard)
	{
		SEND("You aren't even on a ship.\r\n",ch);
		return;
	}
	else
	{
		if (ch->ship->in_room->sector_type == SECT_WATER_NOSWIM)
		{
			SEND ("You'd likely drown disembarking now.\r\n",ch);
			return;
		}
				
		if ((location = get_room_index (ch->ship->in_room->vnum)) != NULL) 
		{			
			//ch->ship->name = area->name;
			ch->onBoard = FALSE;			
			SEND("You disembark from the ship.\r\n",ch);			
			act ("$n disembarks the ship.", ch, NULL, NULL, TO_ROOM);
			char_from_room (ch);
			char_to_room (ch, location);			
			return;
		}		
	}
	
	return;
}

void do_embark ( CHAR_DATA *ch, char *argument )
{

	char buf[MAX_STRING_LENGTH];	
	AREA_DATA *area;
	ROOM_INDEX_DATA *location;
	
	if (ch->onBoard)
	{
		SEND("You're already on a boat!\r\n",ch);
		return;
	}

	if (argument[0] == '\0')
	{
		SEND("Board which ship?\r\n",ch);
		return;
	}
	
	/*if (!ch->ship)
	{
		SEND("You don't even own a boat.\r\n",ch);
		return;
	}*/	
        	
	if (!check_existing_ship(argument))
	{
		SEND("That ship isn't here.\r\n",ch);
		return;
	}	
	area = get_ship_area (argument);
	if (!area)
	{
		return;
	}
	
	if (area->ship_vnum != ch->in_room->vnum)
	{
		SEND("That ship isn't here.\r\n",ch);
		return;
	}
	
	sprintf (buf, "You board the %s.\r\n", area->name);	
	act ("$n boards a ship.", ch, NULL, NULL, TO_ROOM);
	SEND (buf, ch);	
	if ((location = get_room_index (area->min_vnum)) != NULL) //This means the deck must ALWAYS be the first vnum in the ship's area file.
	{		
		ch->ship->in_room = ch->in_room;
		ch->ship->name = area->name;
		char_from_room (ch);
		char_to_room (ch, location);
		act ("$n boards the ship.", ch, NULL, NULL, TO_NOTVICT);		
		ch->onBoard = TRUE;
	}
	return;
	
}


void do_raise ( CHAR_DATA *ch, char *argument )
{
	if (!ch->ship)
		return;
	if (!ch->onBoard)
	{
		SEND("You're not even on a boat.\r\n",ch);
		return;
	}	
	
	if (argument[0] == '\0')
	{
		SEND("Raise or lower it?\r\n",ch);
		return;
	}
	
	if (!str_cmp(argument, "raise"))
	{
		if (!ch->ship->anchored)
		{
			SEND("The anchor is already raised.\r\n",ch);
			return;
		}
		else
		{
			SEND("You raise the anchor.\r\n",ch);
			act ("$n raises the anchor, preparing to sail.\r\n", ch, NULL, NULL, TO_ROOM);
			WAIT_STATE (ch, PULSE_VIOLENCE);
			ch->ship->anchored = FALSE;
			return;
		}
	}
	
	if (!str_cmp(argument, "lower"))
	{
		if (ch->ship->anchored)
		{
			SEND("The anchor is already lowered.\r\n",ch);
			return;
		}
		else
		{
			SEND("You lower the anchor.\r\n",ch);
			act ("$n lowers the anchor, halting progress.\r\n", ch, NULL, NULL, TO_ROOM);
			WAIT_STATE (ch, PULSE_VIOLENCE);
			ch->ship->anchored = TRUE;
			return;
		}
	}
}


void do_anchor ( CHAR_DATA *ch, char *argument )
{

	//SHIP_DATA *ship;
	
	if (!ch->ship)
		return;

	if (argument[0] == '\0')
	{
		SEND("Raise or lower it?\r\n",ch);
		return;
	}
	
	if (!ch->onBoard)
	{
		SEND("You're not even on a boat.\r\n",ch);
		return;
	}
	
	if (!str_cmp(argument, "lower"))
	{	
		if (ch->ship->anchored)
		{
			SEND("The anchor is already dropped.\r\n",ch);
			return;
		}
		else
		{
			ch->ship->anchored = TRUE;
			SEND("You lower the anchor into the water.\r\n",ch);
			act ("$n lowers the anchor, halting progress.\r\n", ch, NULL, NULL, TO_ROOM);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			return;
		}
	}
	
	if (!str_cmp(argument, "raise"))
	{	
		if (!ch->ship->anchored)
		{
			SEND("The anchor is already raised.\r\n",ch);
			return;
		}
		else
		{
			ch->ship->anchored = FALSE;
			SEND("You raise the anchor from the water.\r\n",ch);
			act ("$n raises the anchor, preparing to sail.\r\n", ch, NULL, NULL, TO_ROOM);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			return;
		}
	}
	
}


void do_dock ( CHAR_DATA *ch, char *argument )
{
	//SHIP_DATA *ship;
	//if (!ch->ship)
	//	return;

	if (!ch->onBoard)
	{
		SEND("You're not even on a boat.\r\n",ch);
		return;
	}
	
	//ship->shipstate = SHIP_DOCKED;
	return;
	
}


void do_cannon( CHAR_DATA *ch, char *argument )
{
/*
    char arg[100];
	
	if (!ch->ship)
		return;

    if ( ch == NULL || IS_NPC( ch ))
        return;
	

    argument = one_argument( argument, arg );

	if (!ch->onBoard)
	{
		SEND("You're not even on a boat.\r\n",ch);
		return;
	}
	
    if ( arg[0] == '\0' )
    {
        SEND ( "What do you plan to do with this cannon?\r\n", ch );
        return;
    }

    if ( !str_prefix( arg, "fire" ))
    {
        // code for cannon firing function 
		SEND ( "You fire the cannon\r\n",ch );
        return;
    }

    if ( !str_prefix( arg, "stop" ))
    {
        // code for cannon firing function 
		SEND ( "You stop manning the cannon.\r\n", ch );
        return;
    }

    if ( !str_prefix( arg, "load" ))
    {
	 //code for loading cannon function 
		SEND ( "You begin reloading the cannon.\r\n", ch );
		return;
    }

    if ( !str_prefix( arg, "aim" ))
    {
	 //code for aiming cannon function 
		//grab a direction then go!
		SEND ( "You begin aiming the cannon.\r\n", ch );
		return;
    }
	*/

}



