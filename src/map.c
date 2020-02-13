#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "merc.h"

#define MAX_MAP_DIR 4
#define MAX_BFS 100

int map[11][7];
int g_list[MAX_BFS][3];
int c_list[MAX_BFS][3];

char * room_wall;
char * room_floor;

int rev_dirs[4][2] = {{ 0,-1},
                      { 1, 0},
                      { 0, 1},
                      {-1, 0}};
int min_x = 1;
int min_y = 1;
int max_x = 10;
int max_y = 6;
int center_x = 5;
int center_y = 3;

char *format_map_desc (char *oldstring);

char * get_ascii (ROOM_INDEX_DATA * pRoom, bool fWall)
{
    if (pRoom == NULL) return " ";
    if (!fWall)
    {
        switch (pRoom->sector_type)
        {
            case SECT_HILLS:    return "{Gn";
            case SECT_DESERT:   			
				return "{Y.";
			case SECT_BEACH:
				return "{Y+";
            case SECT_UNUSED:   
			case SECT_CAVE:
			case SECT_UNDERGROUND:
				return "{D.";
            case SECT_CITY:     return "{D.";
            case SECT_MOUNTAIN: return "{D^";
            case SECT_INSIDE:   return "{x.";
            case SECT_FIELD:    
				if (number_range (1, 4) < 3)
					return "{gv";
				else
					return "{Gv";
			case SECT_DOCK:		return "{y#";
			case SECT_ROAD:		return "{y+";
            case SECT_FOREST: 	return "{G.";
			case SECT_JUNGLE:	
				if (number_range (1,4) < 3)
					return "{gv";
				else
					return "{Dv";
			case SECT_SNOW:		return "{W.";
			case SECT_ICE:		return "{W_";
			case SECT_WASTELAND:
			case SECT_RUINS:
				if (number_range (1,4) < 3)
					return "{Dx";
				else
					return "{xx";
			case SECT_SHORELINE:
				return "{y.";
			case SECT_LAVA:
				switch (number_range(1,8))
				{
					default:
					case 1:
					case 2:					
						return "{R~";
					case 3:
					case 4:
					case 5:
						return "{r~";
					case 6:
						return "{Y~";
					case 7:
					case 8:
						return "{y~";
				}				
			case SECT_SWAMP:
                if (number_range (1, 3) < 2)
                    return "{x~";
                else
                    return "{D~";
            case SECT_WATER_SWIM:
                if (number_range (1, 3) < 2)
                    return "{C~";
                else
                    return "{c~";
            case SECT_WATER_NOSWIM:
			case SECT_OCEANFLOOR:
                if (number_range (1, 3) < 2)
                    return "{B~";
                else
                    return "{b~";
            default: return " ";
        }
    }
    else
    {
        switch (pRoom->sector_type)
        {            
            case SECT_UNUSED:
            case SECT_CITY:
            case SECT_MOUNTAIN:
            case SECT_ROAD: 						
			case SECT_CAVE:
			case SECT_UNDERGROUND:
			case SECT_INSIDE: 
				return "{8 {1";
			case SECT_SWAMP:
				return "{7 {1";
			case SECT_DESERT:
			case SECT_WASTELAND:
			case SECT_RUINS:
			case SECT_DOCK:
			case SECT_BEACH:
			case SECT_SHORELINE:
				return "{4 {1";							
            case SECT_HILLS:
            case SECT_FIELD: 
				return "{3 {1";
            case SECT_FOREST: 
			case SECT_JUNGLE:
				return "{g@";
			case SECT_LAVA: return "{2 {1";	
			case SECT_SNOW:
			case SECT_ICE: 
				return "{W#";
            case SECT_WATER_SWIM:
                if (number_range (1, 3) < 2)
                    return "{C~";
                else
                    return "{c~";
            case SECT_WATER_NOSWIM:
			case SECT_OCEANFLOOR:
                if (number_range (1, 3) < 2)
                    return "{B~";
                else
                    return "{b~";
            default: return " ";
        }
    }
    return " ";
}

char * add_map_char (CHAR_DATA *ch, int x, int y, int x_pos, int y_pos)
{
    ROOM_INDEX_DATA * pRoom;
    ROOM_INDEX_DATA * xRoom;
    ROOM_INDEX_DATA * yRoom;
    EXIT_DATA * pExit;
    CHAR_DATA * och;
    bool fPlayer = FALSE;
    bool found = FALSE;
    bool fCombat = FALSE;

    pRoom = get_room_index (map[x][y]);

    if (pRoom == NULL)
        return " ";
    else if (x_pos == 0 && y_pos == 0) //Upper Left
    {
        //Display upward exits.
        if ((pExit = pRoom->exit[DIR_UP]) != NULL && pExit->u1.to_room != NULL
            && !IS_SET (pExit->exit_info, EX_HIDDEN))
        {
            pExit = pRoom->exit[DIR_UP];

            //Display doors.  Grey are locked, off white are just closed.
            if (!IS_SET (pExit->exit_info, EX_CLOSED))
                return "{B^";
            else if (IS_SET (pExit->exit_info, EX_LOCKED))
                return "{Y^";
            else
                return "{D^";
        }

        //Make sure map rooms aren't NULL.
        else if ((pExit = pRoom->exit[DIR_WEST]) == NULL
            || (xRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN)
            || (pExit = pRoom->exit[DIR_NORTH]) == NULL
            || (yRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        //Pretty up fields and large halls by removing unsightly corners.
        else if (  (pExit = pRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = pRoom->exit[DIR_WEST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_SOUTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_WEST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_floor;
        else
            return room_wall;
    }
    else if (x_pos == 1 && y_pos == 0) //Upper Mid
    {
        //Make sure map rooms aren't NULL.
        if ((pExit = pRoom->exit[DIR_NORTH]) == NULL
            || (yRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        else if (  (pExit = pRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                && (pExit = yRoom->exit[DIR_SOUTH]) != NULL)
        {
            pExit = pRoom->exit[DIR_NORTH];

            //Display doors.  Grey are locked, off white are just closed.
            if (!IS_SET (pExit->exit_info, EX_CLOSED))
                return room_floor;
            else if (IS_SET (pExit->exit_info, EX_LOCKED))
                return "{Y-";
            else
                return "{D-";
        }
        else
            return room_wall;
    }
    else if (x_pos == 2 && y_pos == 0) //Upper Right
    {
        //Make sure map rooms aren't NULL.
        if ((pExit = pRoom->exit[DIR_EAST]) == NULL
            || (xRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN)
            || (pExit = pRoom->exit[DIR_NORTH]) == NULL
            || (yRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        //Pretty up fields and large halls by removing unsightly corners.
        else if (  (pExit = pRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = pRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_WEST]) != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_SOUTH]) != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_floor;

        else
            return room_wall;
    }
    else if (x_pos == 0 && y_pos == 1) //Left Side
    {
        //Make sure map rooms aren't NULL.
        if ((pExit = pRoom->exit[DIR_WEST]) == NULL
            || (xRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        else if (  (pExit = pRoom->exit[DIR_WEST]) != NULL && pExit->u1.to_room != NULL
                && (pExit = xRoom->exit[DIR_EAST]) != NULL)
        {
            pExit = pRoom->exit[DIR_WEST];

            //Display doors.  Grey are locked, off white are just closed.
            if (!IS_SET (pExit->exit_info, EX_CLOSED))
                return room_floor;
            else if (IS_SET (pExit->exit_info, EX_LOCKED))
                return "{Y|";
            else
                return "{D|";
        }
        else
            return room_wall;
    }
    else if (x_pos == 1 && y_pos == 1) //Center
    {
        for (och = pRoom->people; och; och = och->next_in_room)
        {
            if (och == ch || !can_see (ch, och))
                continue;
            
            found = TRUE;
            
            if (!IS_NPC (och))
                fPlayer = TRUE;

            if (och->fighting != NULL)
                fCombat = TRUE;
        }

        if (x == center_x && y == center_y) //Ch's position.
            return "{Y*";
        else if (fCombat)
            return "{R*";
        else if (fPlayer)
            return "{C*";
        else if (found)
            return "{m*";
        else
            return room_floor;
    }
    else if (x_pos == 2 && y_pos == 1) //Right Side
    {
        //Make sure map rooms aren't NULL.
        if ((pExit = pRoom->exit[DIR_EAST]) == NULL
            || (xRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        else if (  (pExit = pRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                && (pExit = xRoom->exit[DIR_WEST]) != NULL)
        {
            pExit = pRoom->exit[DIR_EAST];

            //Display doors.  Grey are locked, off white are just closed.
            if (!IS_SET (pExit->exit_info, EX_CLOSED))
                return room_floor;
            else if (IS_SET (pExit->exit_info, EX_LOCKED))
                return "{Y|";
            else
                return "{D|";
        }
        else
            return room_wall;
    }
    else if (x_pos == 0 && y_pos == 2) //Lower Left
    {
        //Make sure map rooms aren't NULL.
        if ((pExit = pRoom->exit[DIR_WEST]) == NULL
            || (xRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN)
            || (pExit = pRoom->exit[DIR_SOUTH]) == NULL
            || (yRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        //Pretty up fields and large halls by removing unsightly corners.
        else if (  (pExit = pRoom->exit[DIR_SOUTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = pRoom->exit[DIR_WEST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_SOUTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_WEST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_floor;

        else
            return room_wall;
    }
    else if (x_pos == 1 && y_pos == 2) //Lower Mid
    {
        //Make sure map rooms aren't NULL.
        if ((pExit = pRoom->exit[DIR_SOUTH]) == NULL
            || (yRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        else if (  (pExit = pRoom->exit[DIR_SOUTH]) != NULL && pExit->u1.to_room != NULL
                && (pExit = yRoom->exit[DIR_NORTH]) != NULL)
        {
            pExit = pRoom->exit[DIR_SOUTH];

            //Display doors.  Grey are locked, off white are just closed.
            if (!IS_SET (pExit->exit_info, EX_CLOSED))
                return room_floor;
            else if (IS_SET (pExit->exit_info, EX_LOCKED))
                return "{Y-";
            else
                return "{D-";
        }
        else
            return room_wall;
    }
    else if (x_pos == 2 && y_pos == 2) //Lower Right
    {
        //Display downward exits.
        if ((pExit = pRoom->exit[DIR_DOWN]) != NULL && pExit->u1.to_room != NULL
            && !IS_SET (pExit->exit_info, EX_HIDDEN))
        {
            pExit = pRoom->exit[DIR_DOWN];

            //Display doors.  Grey are locked, off white are just closed.
            if (!IS_SET (pExit->exit_info, EX_CLOSED))
                return "{Bv";
            else if (IS_SET (pExit->exit_info, EX_LOCKED))
                return "{Yv";
            else
                return "{Dv";
        }

        //Make sure map rooms aren't NULL.
        else if ((pExit = pRoom->exit[DIR_EAST]) == NULL
            || (xRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN)
            || (pExit = pRoom->exit[DIR_SOUTH]) == NULL
            || (yRoom = pExit->u1.to_room) == NULL
            || IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_wall;

        //Pretty up fields and large halls by removing unsightly corners.
        else if (  (pExit = pRoom->exit[DIR_SOUTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = pRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_WEST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = xRoom->exit[DIR_SOUTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_NORTH]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN)
                && (pExit = yRoom->exit[DIR_EAST]) != NULL && pExit->u1.to_room != NULL
                    && !IS_SET (pExit->exit_info, EX_ISDOOR)
                    && !IS_SET (pExit->exit_info, EX_HIDDEN))
            return room_floor;

        else
            return room_wall;
    }
    return " ";
}

void create_map (CHAR_DATA * ch)
{
    ROOM_INDEX_DATA * pRoom;
    EXIT_DATA *pexit;
    int x = center_x;
    int y = center_y;
    int free_g = 1;
    int free_c = 1;
    int door;
    int i;
    bool fSearch = FALSE;

    bzero (g_list, sizeof(g_list));
    bzero (c_list, sizeof(c_list));

    g_list[0][0] = ch->in_room->vnum;
    g_list[0][1] = center_x;
    g_list[0][2] = center_y;

    while (g_list[0][0] != 0)
    {
        pRoom = get_room_index (g_list[0][0]);
        x = g_list[0][1];
        y = g_list[0][2];

        map[g_list[0][1]][g_list[0][2]] = g_list[0][0];
        for (door = 0; door < MAX_MAP_DIR; door++)
        {
            if ((pexit = pRoom->exit[door]) != NULL
                    &&   pexit->u1.to_room != NULL
                    &&   !IS_SET (pexit->exit_info, EX_CLOSED)
                    &&   !IS_SET (pexit->exit_info, EX_HIDDEN))
            {

                if (  x + rev_dirs[door][0] > 11 || x + rev_dirs[door][0] < 0
                    ||y + rev_dirs[door][1] > 7 || y + rev_dirs[door][1] < 0)
                    continue;
    
                fSearch = FALSE;
                for (i = 0; i < free_g; i++)
                {
                    if (g_list[i][0] == pexit->u1.to_room->vnum)
                    {
                        fSearch = TRUE;
                        break;
                    }
                }
                if (!fSearch)
                {
                    for (i = 0; i < free_c; i++)
                    {
                        if (c_list[i][0] == pexit->u1.to_room->vnum)
                        {
                            fSearch = TRUE;
                            break;
                        }
                    }
                }
                if (fSearch || map[x+rev_dirs[door][0]][y+rev_dirs[door][1]] != 0)
                    continue;

                g_list[free_g][0] = pexit->u1.to_room->vnum;
                g_list[free_g][1] = x + rev_dirs[door][0];
                g_list[free_g][2] = y + rev_dirs[door][1];
                free_g++;
                continue;
            }
        }
        c_list[free_c][0] = g_list[0][0];
        c_list[free_c][1] = g_list[0][1];
        c_list[free_c][2] = g_list[0][2];
        for (i = 0; i < free_g; i++)
        {
            g_list[i][0] = g_list[i+1][0];
            g_list[i][1] = g_list[i+1][1];
            g_list[i][2] = g_list[i+1][2];
        }
        free_c++;
        free_g--;
        if (free_c == MAX_BFS)
            break;
        continue;
    }
}

void display_map (CHAR_DATA * ch)
{
    char map_buf[MSL*3];
    int x, y, str_pos=0, desc_pos=0, start;
    int x_pos, y_pos;
    char desc[MSL];
//	char random_desc[MSL];
    char line[100];
    char buf[100];
    int linum = 0;

    bzero (map, sizeof(map));
    create_map (ch);
    
	//if (!IS_NULLSTR(ch->in_room->description))
	strcpy (desc, format_map_desc (ch->in_room->description));
	// else
	// {
		// //strcpy (random_desc, get_random_room_desc(ch));
		// //strcpy (desc, random_desc);
	// }
	
    map_buf[0] = '\0';

    /* --------------- */

    if (IS_IMMORTAL (ch))
    {
        sprintf (buf, "%6ld", ch->in_room->vnum);
        strcat (map_buf, "\n\r{W+{D-------------------{b[{x");
        strcat (map_buf, buf);
        strcat (map_buf, "{b]{D-{W+{x\n\r");
    }
    else
        strcat (map_buf, "\n\r{W+{D----------------------------{W+{x\n\r");

    for (y = min_y; y < max_y; ++y)
    {
        for (y_pos = 0; y_pos < 3; y_pos++)
        {
            strcat (map_buf, "{D| {x");

            for (x = min_x; x < max_x; ++x)
            {
                room_wall = get_ascii (get_room_index (map[x][y]), TRUE);
                room_floor = get_ascii (get_room_index (map[x][y]), FALSE);
                for (x_pos = 0; x_pos < 3; x_pos++)
                    strcat (map_buf, add_map_char (ch, x, y, x_pos, y_pos));
            }

            if (y == min_y && y_pos == 0)
            {
                sprintf (buf, "{D| {b[ {x%s{b ]{x",  ch->in_room->name);
                strcat (map_buf, buf);
            }
            else if (y != max_y && y_pos < 3)
            {
                start = str_pos;
                for (desc_pos = desc_pos ; desc[desc_pos]!='\0' ; desc_pos++)
                {
                    if (desc[desc_pos]=='\n') {
                        line[str_pos-start]='\0';str_pos+=3;
                        desc_pos += 2;
                        break;
                    }
                    else if (desc[desc_pos]=='\r') {
                        line[str_pos-start]='\0';str_pos+=2;
                        break;
                    }
                    else {
                        line[str_pos-start]=desc[desc_pos];
                        str_pos+=1;
                    }
                }
                strcat (map_buf, "{D|{x ");
                line[str_pos-start]='\0';

                if (y == min_y && y_pos == 1)
                    strcat (map_buf, "  ");

                strcat (map_buf, "   {x");
                strcat (map_buf, line);
                strcat (map_buf, "{x");
            }
            strcat (map_buf, "\n\r");
        }
    }
    strcat (map_buf, "{W+{D----------------------------{W+{x");
    while (desc[desc_pos]!='\0')
    {
        start = str_pos;
        for (desc_pos = desc_pos ; desc[desc_pos]!='\0' ; desc_pos++)
        {
            if (desc[desc_pos]=='\n')
            {
                line[str_pos-start]='\0';str_pos+=3;
                desc_pos += 2;
                break;
            }
            else if (desc[desc_pos]=='\r')
            {
                line[str_pos-start]='\0';str_pos+=2;
                break;
            }
            else
            {
                line[str_pos-start]=desc[desc_pos];
                str_pos+=1;
            }
        }

        line[str_pos-start]='\0';
        if (linum > 0)
            strcat (map_buf, "{x                                  ");
        strcat (map_buf, line);
        strcat (map_buf, "\n\r");
        linum++;
    }
    strcat (map_buf, "{x\n\r\n\r");
    send_to_char (map_buf, ch);
    return;
}

/* Small hack of the format_string function as included with OLC.
Original comments from string.c:

 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ---------------------------------------------------------------------------
 *
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 *

*/
char *format_map_desc (char *oldstring)
{
    char xbuf[MAX_STRING_LENGTH];
    char xbuf2[MAX_STRING_LENGTH];
    char *rdesc;
    int i = 0;
    bool cap = TRUE;

    xbuf[0] = xbuf2[0] = 0;

    i = 0;

    for (rdesc = oldstring; *rdesc; rdesc++)
    {
        if (*rdesc == '\n')
        {
            if (xbuf[i - 1] != ' ')
            {
                xbuf[i] = ' ';
                i++;
            }
        }
        else if (*rdesc == '\r');
        else if (*rdesc == ' ')
        {
            if (xbuf[i - 1] != ' ')
            {
                xbuf[i] = ' ';
                i++;
            }
        }
        else if (*rdesc == ')')
        {
            if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                 || xbuf[i - 3] == '!'))
            {
                xbuf[i - 2] = *rdesc;
                xbuf[i - 1] = ' ';
                xbuf[i] = ' ';
                i++;
            }
            else
            {
                xbuf[i] = *rdesc;
                i++;
            }
        }
        else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
        {
            if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                 || xbuf[i - 3] == '!'))
            {
                xbuf[i - 2] = *rdesc;
                if (*(rdesc + 1) != '\"')
                {
                    xbuf[i - 1] = ' ';
                    xbuf[i] = ' ';
                    i++;
                }
                else
                {
                    xbuf[i - 1] = '\"';
                    xbuf[i] = ' ';
                    xbuf[i + 1] = ' ';
                    i += 2;
                    rdesc++;
                }
            }
            else
            {
                xbuf[i] = *rdesc;
                if (*(rdesc + 1) != '\"')
                {
                    xbuf[i + 1] = ' ';
                    xbuf[i + 2] = ' ';
                    i += 3;
                }
                else
                {
                    xbuf[i + 1] = '\"';
                    xbuf[i + 2] = ' ';
                    xbuf[i + 3] = ' ';
                    i += 4;
                    rdesc++;
                }
            }
            cap = TRUE;
        }
        else
        {
            xbuf[i] = *rdesc;
            if (cap)
            {
                cap = FALSE;
                xbuf[i] = UPPER (xbuf[i]);
            }
            i++;
        }
    }
    xbuf[i] = 0;
    strcpy (xbuf2, xbuf);

    rdesc = xbuf2;

    xbuf[0] = 0;

    for (;;)
    {
        for (i = 0; i < 46; i++)
        {
            if (!*(rdesc + i))
                break;
        }
        if (i < 46)
        {
            break;
        }
        for (i = (xbuf[0] ? 46 : 42); i; i--)
        {
            if (*(rdesc + i) == ' ')
                break;
        }
        if (i)
        {
            *(rdesc + i) = 0;
            strcat (xbuf, rdesc);
            strcat (xbuf, "\n\r");
            rdesc += i + 1;
            while (*rdesc == ' ')
                rdesc++;
        }
        else
        {
            bug ("No spaces", 0);
            *(rdesc + 45) = 0;
            strcat (xbuf, rdesc);
            strcat (xbuf, "-\n\r");
            rdesc += 46;
        }
    }
    while (*(rdesc + i) && (*(rdesc + i) == ' ' ||
                            *(rdesc + i) == '\n' || *(rdesc + i) == '\r'))
        i--;
    *(rdesc + i + 1) = 0;
    strcat (xbuf, rdesc);
    if (xbuf[strlen (xbuf) - 2] != '\n')
        strcat (xbuf, "\n\r");

	//prevent color bleeding - Upro 2/19/2012
	strcat (xbuf, "{x");
	
    free_string (oldstring);
    return (str_dup (xbuf));
}
