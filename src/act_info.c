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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"


/* Global variables */

BOUNTY_DATA *bounty_list;

/* for  keeping track of the player count */
int max_on = 0;

/* Command functions needed */

DECLARE_DO_FUN(do_say );

/* Function definitions */

int clan_lookup args( (const char *name) );
void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
int position_lookup(const char *name);
int total_levels 	args((CHAR_DATA *ch));
void ship_exits		args((CHAR_DATA *ch));
int				get_spell_crit_chance args	( (CHAR_DATA *ch) );
int				get_melee_crit_chance args	( (CHAR_DATA *ch) );


int expertise_allowed args ((CHAR_DATA*ch));

int total_expertise args ((CHAR_DATA *ch));


char *const scan_distance[4]=
{
"right here.", "nearby to the %s.", "not far %s.", "off in the distance %s."
};

char *const where_name[] = {
    "{r({x light    {r){x ",	
    "{r({x finger   {r){x ",
    "{r({x finger   {r){x ",
    "{r({x neck     {r){x ",
    "{r({x neck     {r){x ",
    "{r({x torso    {r){x ",
    "{r({x head     {r){x ",
    "{r({x legs     {r){x ",	
    "{r({x feet     {r){x ",
    "{r({x hands    {r){x ",
    "{r({x arms     {r){x ",    
    "{r({x body     {r){x ",
    "{r({x waist    {r){x ",
    "{r({x wrist    {r){x ",
    "{r({x wrist    {r){x ",
    "{r({x wielded  {r){x ",
	"{r({x shield   {r){x ",
	"{r({x offhand  {r){x ",
    "{r({x held     {r){x ",
    "{r({x floating {r){x ",	
	"{r({x sheath   {r){x ",
	"{r({x tail     {r){x ",
	"{r({x ears     {r){x ",
	"{r({x legs     {r){x "	
};

char *const time_descs_by_sector[] = {
//Midnight (12am-4am)

//INSIDE         		0
	" ",
//CITY           		1
	"The moon's light casts both dim light and eerie shadows on the buildings nearby. ",
//FIELD          		2
	"The moonlight dimly illuminates the fields around you. ",
//FOREST         		3
	"The moonlight trickles down softly through the surrounding canopy of trees.",
//HILLS          		4
	"The hills nearby glow with the soft pale light of the moon.",
//MOUNTAIN       		5
	"The mountains nearby seem to glow white in the moon's light.",
//WATER_SWIM     		6
	"The pale water reflects the moon's pale light.",
//WATER_NOSWIM   		7
	"The dark water reflects the moon's pale light.",
//UNUSED         		8
	"THIS IS AN UNUSED SECTOR BUT WHO KNOWS WHY??",
//AIR            	 	9
	"Soft rays of the moon's pale light illuminate the ground below. ",
//DESERT				10
	"The moon casts an eerie shadow across the lonely desert.",
//ROAD					11
	"The road is coated with rays of moonlight.",
//BEACH                 12
	"The sand before you appears a pale white in the soft moonlight.",
//SHORELINE             13
	"Waves crash upon the shoreline, the moon's reflection keen in the wake.",
//CAVE                  14
	"",
//OCEANFLOOR            15
	"Darkness threatens to consume the water around you.",
//LAVA                  16
	"Thick bubbles of magma sprout and suddenly burst, flicking white hot lava about.",
//SWAMP                 17
	"The pale moonlight plays off the murky swamp water casting light across the reeds and grass.",
//UNDERGROUND           18
	"",
//ICE                   19
	"A sea of glimmering ice surrounds you under the moon's gentle light.",
//SNOW                  20
	"A sea of glimmering snow surrounds you under the moon's gentle light.",
//VOID                  21
	"VOID",
//RUINS                 22
	"The moon casts shadows across a landscape of crumbling antiquities and rubble.",
//WASTELAND             23
	"The moon's pale rays illuminate a morose scene of despair before you.",
//JUNGLE                24
	"Eerie pale light softly illuminates the surrounding jungle.",
//DOCK		      		25
	"The moon casts a dim glow upon the docks before you.",
//SHIP					26
	"",
	
//Predawn (4am-6am)

//INSIDE         		0
	"",
//CITY           		1
	"The moon's light casts both dim light and eerie shadows on the buildings nearby.",
//FIELD          		2
	"The moonlight is beginning to fade as it chases the moon behind the horizon of a new day.",
//FOREST         		3
	"The receding moonlight across leaning across the surrounding trees alludes to the approaching dawn.",
//HILLS          		4
	"The hills nearby glow with the soft pale light of the moon.",
//MOUNTAIN       		5
	"The mountains nearby seem to glow white in the moon's light.",
//WATER_SWIM     		6
	"The pale water reflects the moon's pale light.",
//WATER_NOSWIM   		7
	"The dark water reflects the moon's pale light.",
//UNUSED         		8
	"THIS IS UNUSED BUT WHO KNOWS WHY??",
//AIR            	 	9
	"Soft rays of the moon's pale light illuminate the ground below.",
//DESERT				10
	"The moon casts an eerie shadow across the lonely desert.",
//ROAD					11
	"The features of the road become clearer as the sun begins it's ascent into the sky.",
//BEACH                 12
	"Patches of golden sunlight begin to make their way onto the sand of the beach.",
//SHORELINE             13
	"Patches of golden sunlight begin to make their way onto the shoreline.",
//CAVE                  14
	"Darkness threatens to swallow the cave whole.",
//OCEANFLOOR            15
	"Darkness threatens to consume the water around you.",
//LAVA                  16
	"Thick bubbles of magma sprout and suddenly burst, flicking white hot lava about.",
//SWAMP                 17
	"",
//UNDERGROUND           18
	" ",
//ICE                   19
	"Pre dawn rays of orange sunlight glimmer across the ice before you.",
//SNOW                  20
	"Pre dawn rays of orange sunlight glimmer across the snow before you.",
//VOID                  21
	"",
//RUINS                 22
	" ",
//WASTELAND             23
	"The retreating moons casts long shadows across the landscape, shrouding the barren landscape from view.",
//JUNGLE                24
	" ",
//DOCK		      		25
	" ",
//SHIP					26
	" ",
	
	
////////////////////////////////////////////////////////////////////
//Early Morning (6am-11am)//////////////////////////////////////////
////////////////////////////////////////////////////////////////////

//INSIDE         		0
	" ",
//CITY           		1
	"The moon's light casts both dim light and eerie shadows on the buildings nearby.",
//FIELD          		2
	"The moonlight dimly illuminates the fields around you.",
//FOREST         		3
	"A crowning sun unleashes the first rays of sunlight across the forest floor.",
//HILLS          		4
	"The early morning rays of sunshine pour across the rolling hills.",
//MOUNTAIN       		5
	"The rising sun blankets the mountain in warm morning light.",
//WATER_SWIM     		6
	"The pale water reflects the sun's morning light.",
//WATER_NOSWIM   		7
	"The dark water reflects the sun's morning light.",
//UNUSED         		8
	"THIS IS UNUSED BUT WHO KNOWS WHY??",
//AIR            	 	9
	"Soft rays of the moon's pale light illuminate the ground below.",
//DESERT				10
	"Early morning sunlight showers the surface of the desert sand in brilliant orange.",
//ROAD					11
	"The road before you is illuminated by the bright morning sun.",
//BEACH                 12
	" ",
//SHORELINE             13
	"Streaking rays of the morning sun's light stretch lazily across the shoreline.",
//CAVE                  14
	" ",
//OCEANFLOOR            15
	"Darkness threatens to consume the water around you.",
//LAVA                  16
	"The morning sun adds a fraction of heat to the lava surrounding you.",
//SWAMP                 17
	"Night's shadows retreat from the swamp under the rising morning sun.",
//UNDERGROUND           18
	"",
//ICE                   19
	"The ice around you glistens in the morning light.",
//SNOW                  20
	"The snow around you glistens in the morning light.",
//VOID                  21
	"VOID",
//RUINS                 22
	"",
//WASTELAND             23
	"The sun's morning rise gives way to the grim and desolate landscape before you.",
//JUNGLE                24
	"The humidity seems to thicken as the sun rises into the sky.",
//DOCK		      		25
	"Looming shadows stretch across the docks as the sun rises.",
//SHIP					26
	"",
	
////////////////////////////////////////////////////////////////////
//Midday (11am-4pm)/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

//INSIDE         		0
	" ",
//CITY           		1
	"The moon's light casts both dim light and eerie shadows on the buildings nearby.",
//FIELD          		2
	"The moonlight dimly illuminates the fields around you.",
//FOREST         		3
	"The light of midday brings the surrounding forest scenery alive.",
//HILLS          		4
	"The hills nearby glow with the soft pale light of the moon.",
//MOUNTAIN       		5
	"The mountains nearby seem to glow white in the moon's light.",
//WATER_SWIM     		6
	"The pale water reflects the moon's pale light.",
//WATER_NOSWIM   		7
	"The dark water reflects the moon's pale light.",
//UNUSED         		8
	"THIS IS UNUSED BUT WHO KNOWS WHY??",
//AIR            	 	9
	"Soft rays of the moon's pale light illuminate the ground below.",
//DESERT				10
	"The moon casts an eerie shadow across the lonely desert.",
//ROAD					11
	"Your surroundings are fully in view thanks to the midday sun.",
//BEACH                 12
	" ",
//SHORELINE             13
	" ",
//CAVE                  14
	" ",
//OCEANFLOOR            15
	"Darkness threatens to consume the water around you.",
//LAVA                  16
	"Thick bubbles of magma sprout and suddenly burst, flicking white hot lava about.",
//SWAMP                 17
	" ",
//UNDERGROUND           18
	" ",
//ICE                   19
	" ",
//SNOW                  20
	" ",
//VOID                  21
	" ",
//RUINS                 22
	" ",
//WASTELAND             23
	" ",
//JUNGLE                24
	"The sun's heat coupled with the thick air of this place makes it uncomfortable for travel.",
//DOCK		      		25
	"",
//SHIP					26
	"",
	
////////////////////////////////////////////////////////////////////
//Dusk (5pm-8pm)////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

//INSIDE         		0
	" ",
//CITY           		1
	"The moon's light casts both dim light and eerie shadows on the buildings nearby.",
//FIELD          		2
	"The moonlight dimly illuminates the fields around you.",
//FOREST         		3
	"The trees around seem to slowly be engulfed by the shadows of dusk.",
//HILLS          		4
	"The hills nearby glow with the soft pale light of the moon.",
//MOUNTAIN       		5
	"The mountains nearby seem to glow white in the moon's light.",
//WATER_SWIM     		6
	"The pale water reflects the moon's pale light.",
//WATER_NOSWIM   		7
	"The dark water reflects the moon's pale light.",
//UNUSED         		8
	"THIS IS UNUSED BUT WHO KNOWS WHY??",
//AIR            	 	9
	"Soft rays of the moon's pale light illuminate the ground below.",
//DESERT				10
	"The moon casts an eerie shadow across the lonely desert.",
//ROAD					11
	"Long shadows trickle across the road as the sun dips slowly into the horizon.",
//BEACH                 12
	" ",
//SHORELINE             13
	" ",
//CAVE                  14
	" ",
//OCEANFLOOR            15
	"Darkness threatens to consume the water around you.",
//LAVA                  16
	"Thick bubbles of magma sprout and suddenly burst, flicking white hot lava about.",
//SWAMP                 17
	" ",
//UNDERGROUND           18
	" ",
//ICE                   19
	" ",
//SNOW                  20
	" ",
//VOID                  21
	" ",
//RUINS                 22
	" ",
//WASTELAND             23
	" ",
//JUNGLE                24
	" ",
//DOCK		      		25
	" ",
//SHIP					26
	" ",
	
////////////////////////////////////////////////////////////////////
//Night Time (8pm - 12am)///////////////////////////////////////////
////////////////////////////////////////////////////////////////////

//INSIDE         		0
	" ",
//CITY           		1
	"The moon's light casts both dim light and eerie shadows on the buildings nearby.",
//FIELD          		2
	"The moonlight dimly illuminates the fields around you.",
//FOREST         		3
	"As the moon makes it's return to the night sky, the surrounding forest darkens.",
//HILLS          		4
	"The hills nearby glow with the soft pale light of the moon.",
//MOUNTAIN       		5
	"The mountains nearby seem to glow white in the moon's light.",
//WATER_SWIM     		6
	"The pale water reflects the moon's pale light.",
//WATER_NOSWIM   		7
	"The dark water reflects the moon's pale light.",
//UNUSED         		8
	"THIS IS UNUSED BUT WHO KNOWS WHY??",
//AIR            	 	9
	"Soft rays of the moon's pale light illuminate the ground below.",
//DESERT				10
	"The moon casts an eerie shadow across the lonely desert.",
//ROAD					11
	"The road is coated with rays of moonlight.",
//BEACH                 12
	"The sand before you appears a pale white in the soft moonlight.",
//SHORELINE             13
	"",
//CAVE                  14
	"",
//OCEANFLOOR            15
	"Darkness threatens to consume the water around you.",
//LAVA                  16
	"The moon's light is ignored here, by the glowing molten lava surrounding you.",
//SWAMP                 17
	"The pale moonlight plays off the murky swamp water casting light across the reeds and grass.",
//UNDERGROUND           18
	"",
//ICE                   19
	"",
//SNOW                  20
	"A sea of glimmering snow surrounds you under the moon's gentle light.",
//VOID                  21
	"",
//RUINS                 22
	"The moon casts shadows across a landscape of crumbling antiquities and rubble.",
//WASTELAND             23
	"The moon's pale rays illuminate a morose scene of wasteland before you.",
//JUNGLE                24
	"Eerie pale light softly illuminates the surrounding jungle.",
//DOCK		      		25
	"",
//SHIP					26
	""
};

char *const generic_desc_by_sector[] = {
//INSIDE         		0
	"You are inside a room.",
//CITY           		1
	"You are in a bustling city.",
//FIELD          		2
	"A field spreads out before you.",
//FOREST         		3
	"A canopy of trees denotes your location in the forest.",
//HILLS          		4
	"Rolling hills stretch lazily before you.",
//MOUNTAIN       		5
	"A majestic mountain range looms all around you.",
//WATER_SWIM     		6
	"You are swimming in the water.",
//WATER_NOSWIM   		7
	"",
//UNUSED         		8
	"unused",
//AIR            	 	9
	"in the air",
//DESERT				10
	"Dunes of varying sizes present themselves before you.",
//ROAD					11
	"You are on a road.",
//BEACH                 12
	"Beautiful, fine sand stretches before your vision.",
//SHORELINE             13
	"You are near the shore. A vast sea stretches before your eyes.",
//CAVE                  14
	"You are somewhere in a cave.",
//OCEANFLOOR            15
	"You are on the ocean floor.",
//LAVA                  16
	"Molten lava covers the ground here.",
//SWAMP                 17
	"The depressing scenery of a swamp unfolds before you.",
//UNDERGROUND           18
	"You are somewhere underground.",
//ICE                   19
	"A thick sheet of ice covers the ground in your field of vision.",
//SNOW                  20
	"A blanket of snow covers the ground in your field of vision.",
//VOID                  21
	"You are in the void.",
//RUINS                 22
	"Decrepit ruins dot the landscape about you.",
//WASTELAND             23
	"The scene of a barren wasteland presents itself before you.",
//JUNGLE                24
	"Thick shrubbery and vines obscure vision as untold things move around in the jungle.",
//DOCK		      		25
	"You are on a dock.",
//SHIP					26
	"You are on a ship."
};






/*
 * Local functions.
 */
char *format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort, bool eq));
char *blind_format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort, bool eq));
void show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch,
                              bool fShort, bool fShowNothing, bool eq));
void show_char_to_char_0 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args ((CHAR_DATA * list, CHAR_DATA * ch));
bool check_blind args ((CHAR_DATA * ch));
extern int  find_door       args( ( CHAR_DATA *ch, char *arg ) );
bool is_trapped args((OBJ_DATA *obj));
int get_bulk args((CHAR_DATA *ch, bool spells));
void show_quest_log_to_char args((QUEST_DATA * list, CHAR_DATA * ch));

char *blind_format_obj_to_char (OBJ_DATA *obj, CHAR_DATA *ch, bool fShort, bool eq)
{
  static char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];	
    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
        || (obj->description == NULL || obj->description[0] == '\0'))
        return buf;
	if (IS_IMMORTAL(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	{
		sprintf (message, "{d({x%ld{d){x ", obj->pIndexData->vnum);		              					
        strcat  (buf, message);		
		
		sprintf (message, "{r<{x%d{r>{x ", obj->pIndexData->level);
		strcat  (buf, message);
	}
	if ((eq == FALSE && ch->pcdata->item_condition == TRUE) || (eq == TRUE && ch->pcdata->eqcondition == TRUE)) // new hook for eqcondition Upro 1/24/2020
	{
		if (obj->item_type <= ITEM_CLOTHING || obj->item_type == ITEM_MINING_TOOL)
		{
			if ( obj->condition >= 100 )
				strcat( buf, "{G[{xpfct{G]{x ");
			else if ( obj->condition >= 85 )
				strcat( buf, "{g[{xnear{g]{x " );
			else if ( obj->condition >= 70 )
				strcat( buf, "{Y[{xgood{Y]{x " );
			else if ( obj->condition >= 45 )
				strcat( buf, "{y[{xworn{x ");
			else if ( obj->condition >= 20 )
				strcat( buf,  "{R[poor{R]{x ");
			else
				strcat( buf,  "{r[{xbrkn{r]{x ");	
		}
	}
	
	if ((number_percent() < get_skill(ch, gsn_find_trap)) || is_affected(ch, gsn_detect_traps))
	{		
		if (is_trapped(obj))
		{
			strcat (buf, "Trapped ");		
			if (get_skill(ch, gsn_find_trap) > 2)
				check_improve (ch, gsn_find_trap, TRUE, 4);		
		}	
	}	
	if (obj->item_type == ITEM_CONTAINER)
		strcat (buf, "<{cC{x> ");	
	if (IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM))
		strcat (buf, "Quest Item ");	
	if (IS_SET(obj->extra2_flags, ITEM_EPIC))
		strcat (buf, "Epic ");
	if (IS_SET(obj->extra2_flags, ITEM_LEGENDARY))
		strcat (buf, "Legendary ");
	if (IS_SET(obj->extra2_flags, ITEM_ARTIFACT))
		strcat (buf, "Artifact ");	
    if (IS_OBJ_STAT (obj, ITEM_INVIS))
        strcat (buf, "Invis ");    
	if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
        strcat (buf, "Red Aura ");
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
        strcat (buf, "Golden Aura ");
    if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
        strcat (buf, "Magical ");
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        strcat (buf, "Glowing ");
    if (IS_OBJ_STAT (obj, ITEM_HUM))
        strcat (buf, "Humming ");
    if (fShort)
    {
		
        if (obj->short_descr != NULL)		
		{
			if (IS_SET(obj->extra2_flags, ITEM_HIDDEN))
				strcat (buf, "Hidden ");
				
            strcat (buf, obj->short_descr);
						
		}
    }
    else
    {
        if (obj->description != NULL)
        {
			if (IS_SET(obj->extra2_flags, ITEM_HIDDEN))
				strcat (buf, "Hidden ");
				
            strcat (buf, obj->description);
						
		}
    }	
	if (ch->pcdata->learned[gsn_wands] > 39)
	{
		if (obj->item_type == ITEM_WAND || obj->item_type == ITEM_STAFF || obj->item_type == ITEM_FIGURINE)
		{
			sprintf (message, " (%d/%d)", obj->value[2], obj->value[1]);
			strcat  (buf, message);		
		}
	}
    return buf;
	
}

char *format_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, bool fShort, bool eq)
{	
    static char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];	
    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
        || (obj->description == NULL || obj->description[0] == '\0'))
        return buf;
	if (IS_IMMORTAL(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	{
		if (obj->altitude < 0)
		{
			sprintf (message, "{m*{x ");
			strcat  (buf, message);
		}
		
		sprintf (message, "{d({x%5ld{d){x ", obj->pIndexData->vnum);		              					
        strcat  (buf, message);		
		
		sprintf (message, "{r<{x%2d{r>{x ", obj->pIndexData->level);
		strcat  (buf, message);		
	}	
	if ((eq == FALSE && ch->pcdata->item_condition == TRUE) || (eq == TRUE && ch->pcdata->eqcondition == TRUE)) // new hook for eqcondition Upro 1/24/2020
	{
		if (obj->item_type <= ITEM_CLOTHING || obj->item_type == ITEM_MINING_TOOL)
		{
			if ( obj->condition >= 100 )
				strcat( buf, "{G[{xpfct{G]{x ");
			else if ( obj->condition >= 85 )
				strcat( buf, "{g[{xnear{g]{x " );
			else if ( obj->condition >= 70 )
				strcat( buf, "{Y[{xgood{Y]{x " );
			else if ( obj->condition >= 45 )
				strcat( buf, "{y[{xworn{x ");
			else if ( obj->condition >= 20 )
				strcat( buf,  "{R[poor{R]{x ");
			else
				strcat( buf,  "{r[{xbrkn{r]{x ");	
		}
	}
	if ((number_percent() < get_skill(ch, gsn_find_trap)) || is_affected(ch, gsn_detect_traps))
	{		
		if (is_trapped(obj))
		{
			strcat (buf, "({CTrapped{x) ");		
			if (get_skill(ch, gsn_find_trap) > 2)
				check_improve (ch, gsn_find_trap, TRUE, 4);
		}	
	}		
	if (obj->item_type == ITEM_CONTAINER)
		strcat (buf, "<{cC{x> ");	
	if (IS_SET(obj->extra2_flags, ITEM_QUEST_ITEM))
		strcat (buf, "[{CQ{x] ");	
	if (IS_SET(obj->extra2_flags, ITEM_EPIC))
		strcat (buf, "<{yE{x> ");
	if (IS_SET(obj->extra2_flags, ITEM_LEGENDARY))
		strcat (buf, "<{cL{x> ");
	if (IS_SET(obj->extra2_flags, ITEM_ARTIFACT))
		strcat (buf, "<{BA{x> ");	
    if (IS_OBJ_STAT (obj, ITEM_INVIS))
        strcat (buf, "({DInvis{x) ");    
	if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
        strcat (buf, "({rRed Aura{x) ");
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
        strcat (buf, "({yGolden Aura{x) ");
    if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
        strcat (buf, "({bMagical{x) ");
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        strcat (buf, "({WGlowing{x) ");
    if (IS_OBJ_STAT (obj, ITEM_HUM))
        strcat (buf, "({GHumming{x) ");
    if (fShort)
    {
		
        if (obj->short_descr != NULL)		
		{
			if (IS_SET(obj->extra2_flags, ITEM_HIDDEN))
				strcat (buf, "{r({x");
				
            strcat (buf, obj->short_descr);
			
			if (IS_SET(obj->extra2_flags, ITEM_HIDDEN))
				strcat (buf, "{r){x");
		}
    }
    else
    {
        if (obj->description != NULL)
        {
			if (IS_SET(obj->extra2_flags, ITEM_HIDDEN))
				strcat (buf, "{r({x");
				
            strcat (buf, obj->description);
			
			if (IS_SET(obj->extra2_flags, ITEM_HIDDEN))
				strcat (buf, "{r){x");
		}
    }	
	
	if (ch->pcdata->learned[gsn_wands] > 39)
	{
		if (obj->item_type == ITEM_WAND || obj->item_type == ITEM_STAFF || obj->item_type == ITEM_FIGURINE)
		{
			sprintf (message, " (%d/%d)", obj->value[2], obj->value[1]);
			strcat  (buf, message);		
		}
	}
    return buf;
	
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char (OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
                        bool fShowNothing, bool eq)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;	
    bool fCombine;

    if (ch->desc == NULL)
        return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf ();

    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_content)
        count++;
		if ( count > 120 )
		{
			SEND( "That is WAY too much junk!  Drop some of it!\r\n", ch );
			return;
		}
    prgpstrShow = alloc_mem (count * sizeof (char *));
    prgnShow = alloc_mem (count * sizeof (int));
    nShow = 0;

    /*
     * Format the list of objects.
     */
    for (obj = list; obj != NULL; obj = obj->next_content)
    {
        if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
        {
			if (!ch->blind)
				pstrShow = format_obj_to_char (obj, ch, fShort, eq);
			else
				pstrShow = blind_format_obj_to_char (obj, ch, fShort, eq);

            fCombine = FALSE;

            if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
            {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords.
                 */
                for (iShow = nShow - 1; iShow >= 0; iShow--)
                {
                    if (!strcmp (prgpstrShow[iShow], pstrShow))
                    {
                        prgnShow[iShow]++;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if (!fCombine)
            {
                prgpstrShow[nShow] = str_dup (pstrShow);
                prgnShow[nShow] = 1;
                nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    for (iShow = 0; iShow < nShow; iShow++)
    {
        if (prgpstrShow[iShow][0] == '\0')
        {
            free_string (prgpstrShow[iShow]);
            continue;
        }

        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
        {
            if (prgnShow[iShow] != 1)
            {
                sprintf (buf, "(%2d) ", prgnShow[iShow]);
                add_buf (output, buf);
            }
            else
            {
                add_buf (output, "     ");
            }
        }
        add_buf (output, prgpstrShow[iShow]);
        add_buf (output, "\r\n");
        free_string (prgpstrShow[iShow]);
    }

    if (fShowNothing && nShow == 0)
    {
        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
            SEND ("     ", ch);
        SEND ("Nothing.\r\n", ch);
    }
    page_to_char (buf_string (output), ch);

    /*
     * Clean up.
     */
    free_buf (output);
    free_mem (prgpstrShow, count * sizeof (char *));
    free_mem (prgnShow, count * sizeof (int));

    return;
}


//Do_look without an argument. (Not looking directly at the char)
void show_char_to_char_0 (CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];	
	char holy[MSL];
	QUEST_INDEX_DATA *quest;
	
    buf[0] = '\0';
		
	if ( IS_IMMORTAL(ch) && IS_SET(ch->act,PLR_HOLYLIGHT) && IS_NPC(victim) && !IS_NPC(ch))
	{
	    sprintf( holy, "{d({x%ld{d){x ", victim->pIndexData->vnum);
	    strcat( buf, holy );
	}	
	
	if (IS_NPC(victim))
	{
		if ((quest = get_quest_index(mob_has_quest(victim))) != NULL)
		{
			//Quests are buggy - Upro 1/13/2020
			//if (quest->accept_vnum == victim->pIndexData->vnum)			
			//	strcat (buf, "[{CQ{x] ");						
		}
	}
	
    if (IS_SET (victim->comm, COMM_AFK))
        strcat (buf, "<{bAFK{x> ");		
	if (is_affected(ch, gsn_detect_undead) && IS_UNDEAD(victim))
		strcat (buf, "({DUndead{x) ");	
    if (IS_AFFECTED (victim, AFF_INVISIBLE))
        strcat (buf, "({DInvis{x) ");
	if (is_affected (victim, gsn_stone_meld))
        strcat (buf, "({DMeld{x) ");
    if (victim->invis_level >= LEVEL_HERO)
        strcat (buf, "({rWizi{x) ");
    if (IS_AFFECTED (victim, AFF_HIDE))
        strcat (buf, "({rHide{x) ");
    if (IS_AFFECTED (victim, AFF_CHARM))
        strcat (buf, "({YCharmed{x) ");
    if (IS_AFFECTED (victim, AFF_PASS_DOOR))
        strcat (buf, "({DTranslucent{x) ");
    if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
        strcat (buf, "({MPink Aura{x) ");
    if (IS_EVIL (victim) && IS_AFFECTED (ch, AFF_DETECT_EVIL))
        strcat (buf, "({rRed Aura{x) ");
    if (IS_GOOD (victim) && IS_AFFECTED (ch, AFF_DETECT_GOOD))
        strcat (buf, "({yGolden Aura{x) ");
    if (IS_AFFECTED (victim, AFF_SANCTUARY))
        strcat (buf, "({WWhite Aura{x) ");    
	if (victim->clan && !IS_NPC(victim) && total_levels(victim) >= 15)
		strcat (buf, "({RPvP{x) ");
	if (!IS_NPC (victim) && IS_SET (victim->act, PLR_KILLER))
        strcat (buf, "(KILLER) ");
    if (!IS_NPC (victim) && IS_SET (victim->act, PLR_THIEF))
        strcat (buf, "(THIEF) ");
	if (IS_NPC (victim) && IS_SET(victim->act2, ACT2_ELITE))
		strcat (buf, "({YElite{x) ");
	if (IS_NPC (victim) && IS_SET(victim->act2, ACT2_BOSS))
		strcat (buf, "({RBoss{x) ");
    if ((victim->position == victim->start_pos && victim->long_descr[0] != '\0') || ((victim->pIndexData) && IS_SET(victim->act2, ACT2_INANIMATE)))
    {
        strcat (buf, victim->long_descr);
        SEND (buf, ch);
        return;
    }

    strcat (buf, PERS (victim, ch));
    // if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF)
        // && victim->position == POS_STANDING && ch->on == NULL)
        // strcat (buf, victim->pcdata->title);

    switch (victim->position)
    {
        case POS_DEAD:
            strcat (buf, " is DEAD!!");
            break;
        case POS_MORTAL:
            strcat (buf, " is mortally wounded.");
            break;
        case POS_INCAP:
            strcat (buf, " is incapacitated.");
            break;
        case POS_STUNNED:
            strcat (buf, " is lying here stunned.");
            break;
        case POS_SLEEPING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], SLEEP_AT))
                {
                    sprintf (message, " is sleeping at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], SLEEP_ON))
                {
                    sprintf (message, " is sleeping on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is sleeping in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is sleeping here.");
            break;
        case POS_RESTING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], REST_AT))
                {
                    sprintf (message, " is resting at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], REST_ON))
                {
                    sprintf (message, " is resting on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is resting in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is resting here.");
            break;
        case POS_SITTING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], SIT_AT))
                {
                    sprintf (message, " is sitting at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], SIT_ON))
                {
                    sprintf (message, " is sitting on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is sitting in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is sitting here.");
            break;
        case POS_STANDING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], STAND_AT))
                {
                    sprintf (message, " is standing at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], STAND_ON))
                {
                    sprintf (message, " is standing on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is standing in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is here.");
            break;
        case POS_FIGHTING:
            strcat (buf, " is here, fighting ");
            if (victim->fighting == NULL)
                strcat (buf, "thin air??");
            else if (victim->fighting == ch)
                strcat (buf, "YOU!");
            else if (victim->in_room == victim->fighting->in_room)
            {
                strcat (buf, PERS (victim->fighting, ch));
                strcat (buf, ".");
            }
            else
                strcat (buf, "someone who left??");
            break;
    }

    strcat (buf, "\r\n");
    buf[0] = UPPER (buf[0]);
    SEND (buf, ch);
    return;
}


//Look <char name>
void show_char_to_char_1 (CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if (can_see (victim, ch))
    {
        if (ch == victim)
            act ("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
        else
        {
            act ("$n looks at you.", ch, NULL, victim, TO_VICT);
            act ("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
        }
    }

    if (victim->description[0] != '\0')
        SEND (victim->description, ch);
    else
        act ("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);

	if (victim->bleeding > 0)
	{
		if (victim->bleeding < 5)
			SEND ("They seem to be slightly bleeding.\r\n",ch);
		else
			SEND ("They seem to be bleeding profusely!\r\n",ch);
	}



    if (victim->max_hit > 0)
        percent = (100 * victim->hit) / victim->max_hit;
    else
        percent = -1;

    strcpy (buf, PERS (victim, ch));

	if (IS_NPC(victim) && (victim->pIndexData) && IS_SET(victim->act2, ACT2_INANIMATE) && (valid_material(victim->material)))
	{
		if (IS_METAL(victim->material))
		{
			if (percent >= 100)
				strcat (buf, " looks brand new.\r\n");
			else if (percent >= 90)
				strcat (buf, " has a couple slight dents.\r\n");
			else if (percent >= 75)
				strcat (buf, " is tarnished and dented.\r\n");
			else if (percent >= 50)
				strcat (buf, " has several dents and scratches.\r\n");
			else if (percent >= 30)
				strcat (buf, " is looking pretty battered.\r\n");
			else if (percent >= 15)
				strcat (buf, " has more dents than not.\r\n");
			else if (percent >= 0)
				strcat (buf, " is barely recognizable.\r\n");
			else
				strcat (buf, " is pretty much scrap.\r\n");
		}
		else if (IS_GEM(victim->material))
		{
			if (percent >= 100)
				strcat (buf, " shines like a fresh cut.\r\n");
			else if (percent >= 90)
				strcat (buf, " has a couple slight scratches.\r\n");
			else if (percent >= 75)
				strcat (buf, " appears somewhat scratched.\r\n");
			else if (percent >= 50)
				strcat (buf, " has multiple visible scratches.\r\n");
			else if (percent >= 30)
				strcat (buf, " is looking pretty scratched.\r\n");
			else if (percent >= 15)
				strcat (buf, " has more scratches than not.\r\n");
			else if (percent >= 0)
				strcat (buf, " is barely recognizable.\r\n");
			else
				strcat (buf, " needs to be recut.\r\n");
		}
		else if (IS_CLOTH(victim->material))
		{
			if (percent >= 100)
				strcat (buf, " looks brand new.\r\n");
			else if (percent >= 90)
				strcat (buf, " has some slight fraying.\r\n");
			else if (percent >= 75)
				strcat (buf, " appears somewhat frayed.\r\n");
			else if (percent >= 50)
				strcat (buf, " is pretty badly ripped.\r\n");
			else if (percent >= 30)
				strcat (buf, " is tearing apart.\r\n");
			else if (percent >= 15)
				strcat (buf, " has more rips and frays than not.\r\n");
			else if (percent >= 0)
				strcat (buf, " is barely recognizable.\r\n");
			else
				strcat (buf, " is torn to shreds.\r\n");
		}
		else if (IS_WOOD(victim->material))
		{
			if (percent >= 100)
				strcat (buf, " appears unscathed.\r\n");
			else if (percent >= 90)
				strcat (buf, " is slightly chipped.\r\n");
			else if (percent >= 75)
				strcat (buf, " is somewhat splintered.\r\n");
			else if (percent >= 50)
				strcat (buf, " is cracked and split.\r\n");
			else if (percent >= 30)
				strcat (buf, " is splintered and worn.\r\n");
			else if (percent >= 15)
				strcat (buf, " is battered, splintered, and worn.\r\n");
			else if (percent >= 0)
				strcat (buf, " is barely recognizable.\r\n");
			else
				strcat (buf, " is completely splintered.\r\n");
		}
		else
		{
			if (percent >= 100)
				strcat (buf, " is in excellent condition.\r\n");
			else if (percent >= 90)
				strcat (buf, " has a few scratches.\r\n");
			else if (percent >= 75)
				strcat (buf, " has some small bufs and bruises.\r\n");
			else if (percent >= 50)
				strcat (buf, " has quite a few bufs.\r\n");
			else if (percent >= 30)
				strcat (buf,
						 " has some big nasty bufs and scratches.\r\n");
			else if (percent >= 15)
				strcat (buf, " looks pretty hurt.\r\n");
			else if (percent >= 0)
				strcat (buf, " is in awful condition.\r\n");
			else
				strcat (buf, " is bleeding to death.\r\n");
		}
	}
	else
	{
		if (percent >= 100)
			strcat (buf, " appears unharmed.\r\n");
		else if (percent >= 90)
			strcat (buf, " has a few scratches.\r\n");
		else if (percent >= 75)
			strcat (buf, " has some small wounds and bruises.\r\n");
		else if (percent >= 50)
			strcat (buf, " has quite a few wounds.\r\n");	
		else if (percent >= 30)
			strcat (buf, " has some big nasty wounds and scratches.\r\n");
		else if (percent >= 15)
			strcat (buf, " is hurt pretty bad.\r\n");
		else if (percent >= 0)
			strcat (buf, " appears near death.\r\n");
		else
			strcat (buf, " is dying before your eyes.\r\n");
	}
	
	
   
		
    buf[0] = UPPER (buf[0]);
    SEND (buf, ch);

    if (IS_NPC(victim) && IS_SET(victim->act2, ACT2_INANIMATE))
			return;
			
	found = FALSE;
    for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
        if ((obj = get_eq_char (victim, iWear)) != NULL
            && can_see_obj (ch, obj))
        {
            if (!found)
            {
                SEND ("\r\n", ch);
                act ("$N is using:", ch, NULL, victim, TO_CHAR);
                found = TRUE;
            }
            SEND (where_name[iWear], ch);
			if (!ch->blind)
				SEND (format_obj_to_char (obj, ch, TRUE, FALSE), ch);
			else
				SEND (blind_format_obj_to_char (obj, ch, TRUE, FALSE), ch);
            SEND ("\r\n", ch);
        }
    }

    if (victim != ch && !IS_NPC (ch)
        && number_percent () < get_skill (ch, gsn_peek))
    {		
        SEND ("\r\nYou peek at the inventory:\r\n", ch);
        check_improve (ch, gsn_peek, TRUE, 4);
        show_list_to_char (victim->carrying, ch, TRUE, TRUE, FALSE);
    }

    return;
}



void show_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
    CHAR_DATA *rch;

    for (rch = list; rch != NULL; rch = rch->next_in_room)
    {
        if (rch == ch)
            continue;

		if ( IS_NPC(rch) && IS_SET(rch->act2, ACT2_MOBINVIS))
			continue;
			
        if (get_trust (ch) < rch->invis_level)
            continue;

        if (can_see (ch, rch))
        {
            show_char_to_char_0 (rch, ch);
        }
		else if (room_is_dark (ch->in_room) && IS_AFFECTED (ch, AFF_INFRARED) && can_see(ch, rch))
		{
            show_char_to_char_0 (rch, ch);
        }
		else if (room_is_dark (ch->in_room) && IS_AFFECTED (ch, AFF_INFRARED) && !can_see(ch, rch))
		{
			if (IS_NPC(rch) && !IS_SET(rch->act2, ACT2_INANIMATE))
				SEND ("You see the outline of a living creature.\r\n", ch);
        }
        else if (room_is_dark (ch->in_room)
                 && IS_AFFECTED (rch, AFF_INFRARED))
        {
			if (IS_NPC(rch) && !IS_SET(rch->act2, ACT2_INANIMATE))
				SEND ("You see glowing red eyes watching YOU!\r\n", ch);
        }
    }

    return;
}



bool check_blind (CHAR_DATA * ch)
{
    if (is_affected (ch, gsn_blindness) && !IS_SET(ch->act, PLR_HOLYLIGHT))
    {		
        SEND ("You can't see a thing!\r\n", ch);
        return FALSE;
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        if (ch->lines == 0)
            SEND ("You do not page long messages.\r\n", ch);
        else
        {
            sprintf (buf, "You currently display %d lines per page.\r\n",
                     ch->lines + 2);
            SEND (buf, ch);
        }
        return;
    }

    if (!is_number (arg))
    {
        SEND ("You must provide a number.\r\n", ch);
        return;
    }

    lines = atoi (arg);

    if (lines == 0)
    {
        SEND ("Paging disabled.\r\n", ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
        SEND ("You must provide a reasonable number.\r\n", ch);
        return;
    }

    sprintf (buf, "Scroll set to %d lines.\r\n", lines);
    SEND (buf, ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;

    col = 0;

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
        sprintf (buf, "%-12s", social_table[iSocial].name);
        SEND (buf, ch);
        if (++col % 6 == 0)
            SEND ("\r\n", ch);
    }

    if (col % 6 != 0)
        SEND ("\r\n", ch);
    return;
}



/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "motd");
}

void do_imotd (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "imotd");
}

void do_rules (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "rules");
}

void do_story (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "story");
}

void do_wizlist (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist (CHAR_DATA * ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC (ch))
        return;

    SEND ("   action     status\r\n", ch);
    SEND ("---------------------\r\n", ch);

    
	if (IS_IMMORTAL(ch))
	{
		SEND ("autodig        ", ch);
		if (IS_SET (ch->act, PLR_AUTODIG))
			SEND ("{GON{x\r\n", ch);
		else
			SEND ("{ROFF{x\r\n", ch);
	}
	
	SEND ("autoassist     ", ch);
    if (IS_SET (ch->act, PLR_AUTOASSIST))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("autoexit       ", ch);
    if (IS_SET (ch->act, PLR_AUTOEXIT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("autogold       ", ch);
    if (IS_SET (ch->act, PLR_AUTOGOLD))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("autoloot       ", ch);
    if (IS_SET (ch->act, PLR_AUTOLOOT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("autosac        ", ch);
    if (IS_SET (ch->act, PLR_AUTOSAC))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);

    SEND ("autosplit      ", ch);
    if (IS_SET (ch->act, PLR_AUTOSPLIT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
		
	SEND ("autosheathe    ", ch);
    if (IS_SET (ch->act, PLR_AUTO_SHEATHE))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
		
	SEND ("autodrink      ", ch);
    if (IS_SET (ch->act, PLR_AUTO_DRINK))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
		
	SEND ("autoeat        ", ch);
    if (IS_SET (ch->act, PLR_AUTO_EAT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
		
	SEND ("autoscroll     ", ch);
    if (IS_SET (ch->act, PLR_AUTO_SCROLL_LOOT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
		
	SEND ("automap        ", ch);
    if (IS_SET (ch->act, PLR_AUTOMAP))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
	
	SEND ("combine items  ", ch);
    if (IS_SET (ch->comm, COMM_COMBINE))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
	
	SEND ("compact mode   ", ch);
    if (IS_SET (ch->comm, COMM_COMPACT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
    
	SEND ("prompt         ", ch);
    if (IS_SET (ch->comm, COMM_PROMPT))
        SEND ("{GON{x\r\n", ch);
    else
        SEND ("{ROFF{x\r\n", ch);
	
	SEND ("telnetga       ", ch);
    if (IS_SET (ch->comm, COMM_TELNET_GA))
	    SEND ("{GON{x\r\n", ch);
    else
	    SEND ("{ROFF{x\r\n",ch);	
    
	SEND ("timestamps     ", ch);
    if (IS_SET (ch->comm, COMM_TIME_STAMPS))
	    SEND ("{GON{x\r\n", ch);
    else
	    SEND ("{ROFF{x\r\n",ch);	

  

	SEND("\r\n",ch);

	if (ch->pcdata->item_condition == TRUE)	
		SEND ("- You choose to see item conditions.\r\n",ch);
	else
		SEND ("- You choose not to see item conditions.\r\n",ch);
		
	if (ch->pcdata->eqcondition == TRUE)	
		SEND ("- You choose to see equipment conditions.\r\n",ch);
	else
		SEND ("- You choose not to see equipment conditions.\r\n",ch);

	if (ch->pcdata->beeptells == TRUE)	
		SEND ("- You choose to hear beeps in {Ctells{x.\r\n",ch);
	else
		SEND ("- You choose not to hear beeps in {Ctells{x.\r\n",ch);
    // if (!IS_SET (ch->act, PLR_CANLOOT))
        // SEND ("Your corpse is safe from thieves.\r\n", ch);
    // else
        // SEND ("Your corpse may be looted.\r\n", ch);

    if (IS_SET (ch->act, PLR_NOSUMMON))
        SEND ("- You cannot be summoned.\r\n", ch);
    else
        SEND ("- You can be summoned.\r\n", ch);

    if (IS_SET (ch->act, PLR_NOFOLLOW))
        SEND ("- You do not welcome followers.\r\n", ch);
    else
        SEND ("- You accept followers.\r\n", ch);
		
	if (IS_RANGER(ch))
	{
		if (IS_SET(ch->act, PLR_TRACKING))
			SEND ("- You see tracks on the ground.\r\n",ch);
		else
			SEND ("- You choose to ignore the tracks on the ground.\r\n",ch);
	}
	
	if (IS_MCLASSED(ch))
	{
		if (ch->no_exp[0] == FALSE)		
			SEND("- You {Ggain{x experience points in your primary class normally.\r\n",ch);
		else
			SEND("- You are {Rnot gaining{x experience points in your primary class.\r\n",ch);
			
		if (ch->no_exp[1] == FALSE)		
			SEND("- You {Ggain{x experience points in your secondary class normally.\r\n",ch);
		else
			SEND("- You are {Rnot gaining{x experience points in your secondary class.\r\n",ch);
	}
	else
	{
		if (ch->no_exp[0] == FALSE)
		{
			if (total_levels(ch) < MAX_LEVEL)
				SEND ("- You are {Ggaining{x experience normally.\r\n",ch);
		}
		else
			SEND ("- You are currently {Rnot gaining{x experience.\r\n",ch);
	}
	
}

void do_autosheathe (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTO_SHEATHE))
    {
        SEND ("Auto sheathing removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTO_SHEATHE);
    }
    else
    {
        SEND ("Automatic sheathing set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTO_SHEATHE);
    }
}

void do_timestamp (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->comm, COMM_TIME_STAMPS))
    {
        SEND ("Timestamps turned on.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_TIME_STAMPS);
    }
    else
    {
        SEND ("Short descriptions activated.\r\n", ch);
        SET_BIT (ch->comm, COMM_TIME_STAMPS);
    }
}

void do_autoeat (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTO_EAT))
    {
        SEND ("Auto eating removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTO_EAT);
    }
    else
    {
        SEND ("Automatic eating set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTO_EAT);
    }
}

void do_autodrink (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTO_DRINK))
    {
        SEND ("Auto Drinking removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTO_DRINK);
    }
    else
    {
        SEND ("Automatic drinking set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTO_DRINK);
    }
}

void do_autoscroll (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTO_SCROLL_LOOT))
    {
        SEND ("Auto scroll looting removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTO_SCROLL_LOOT);
    }
    else
    {
        SEND ("Automatic corpse scroll looting set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTO_SCROLL_LOOT);
    }
}


void do_automap (CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
		
	if (IS_SET (ch->act, PLR_AUTOMAP))
    {
        SEND ("Automap removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOMAP);
    }
    else
    {
        SEND ("You will now see an automap.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOMAP);
    }
}

void do_autotrack (CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch) || !IS_RANGER(ch))
		return;
		
	if (IS_SET (ch->act, PLR_TRACKING))
    {
        SEND ("Autotracking removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_TRACKING);
    }
    else
    {
        SEND ("You will now see tracks on the ground.\r\n", ch);
        SET_BIT (ch->act, PLR_TRACKING);
    }
}

void do_autoassist (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOASSIST))
    {
        SEND ("Autoassist removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOASSIST);
    }
    else
    {
        SEND ("You will now assist when needed.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOASSIST);
    }
}

void do_autoexit (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOEXIT))
    {
        SEND ("Exits will no longer be displayed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOEXIT);
    }
    else
    {
        SEND ("Exits will now be displayed.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOEXIT);
    }
}

void do_autogold (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOGOLD))
    {
        SEND ("Autogold removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOGOLD);
    }
    else
    {
        SEND ("Automatic gold looting set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOGOLD);
    }
}

void do_autoloot (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOLOOT))
    {
        SEND ("Autolooting removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOLOOT);
    }
    else
    {
        SEND ("Automatic corpse looting set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOLOOT);
    }
}

void do_autosac (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOSAC))
    {
        SEND ("Autosacrificing removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOSAC);
    }
    else
    {
        SEND ("Automatic corpse sacrificing set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOSAC);
    }
}

void do_autosplit (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOSPLIT))
    {
        SEND ("Autosplitting removed.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_AUTOSPLIT);
    }
    else
    {
        SEND ("Automatic gold splitting set.\r\n", ch);
        SET_BIT (ch->act, PLR_AUTOSPLIT);
    }
}

void do_autoall (CHAR_DATA *ch, char * argument)
{
    if (IS_NPC(ch))
        return;

    if (!strcmp (argument, "on"))
    {
        SET_BIT(ch->act,PLR_AUTOASSIST);
        SET_BIT(ch->act,PLR_AUTOEXIT);
        SET_BIT(ch->act,PLR_AUTOGOLD);
        SET_BIT(ch->act,PLR_AUTOLOOT);
        SET_BIT(ch->act,PLR_AUTOSAC);
        SET_BIT(ch->act,PLR_AUTOSPLIT);
		SET_BIT(ch->act,PLR_AUTOMAP);
		SET_BIT(ch->act,PLR_AUTO_SCROLL_LOOT);
        SEND("All autos turned on.\r\n",ch);
    }
    else if (!strcmp (argument, "off"))
    {
        REMOVE_BIT (ch->act, PLR_AUTOASSIST);
        REMOVE_BIT (ch->act, PLR_AUTOEXIT);
        REMOVE_BIT (ch->act, PLR_AUTOGOLD);
        REMOVE_BIT (ch->act, PLR_AUTOLOOT);
        REMOVE_BIT (ch->act, PLR_AUTOSAC);
        REMOVE_BIT (ch->act, PLR_AUTOSPLIT);
		REMOVE_BIT (ch->act, PLR_AUTOMAP);
		REMOVE_BIT (ch->act,PLR_AUTO_SCROLL_LOOT);

        SEND("All autos turned off.\r\n", ch);
    }
    else
        SEND("Usage: autoall [on|off]\r\n", ch);
}

void do_brief (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_BRIEF))
    {
        SEND ("Full descriptions activated.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_BRIEF);
    }
    else
    {
        SEND ("Short descriptions activated.\r\n", ch);
        SET_BIT (ch->comm, COMM_BRIEF);
    }
}

void do_compact (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_COMPACT))
    {
        SEND ("Compact mode removed.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_COMPACT);
    }
    else
    {
        SEND ("Compact mode set.\r\n", ch);
        SET_BIT (ch->comm, COMM_COMPACT);
    }
}

void do_show (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
    {
        SEND ("Affects will no longer be shown in score.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_SHOW_AFFECTS);
    }
    else
    {
        SEND ("Affects will now be shown in score.\r\n", ch);
        SET_BIT (ch->comm, COMM_SHOW_AFFECTS);
    }
}

void do_prompt (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_PROMPT))
        {
            SEND ("You will no longer see prompts.\r\n", ch);
            REMOVE_BIT (ch->comm, COMM_PROMPT);
        }
        else
        {
            SEND ("You will now see prompts.\r\n", ch);
            SET_BIT (ch->comm, COMM_PROMPT);
        }
        return;
    }

	if (!strcmp (argument, "default"))
        strcpy (buf, "(%h/%H %m/%M %v/%V) ({Y%g{xg {D%s{xs) (%t / %b)");

    if (!strcmp (argument, "all"))
        strcpy (buf, "(%h/%H %m/%M %v/%V) ({Y%g{xg {D%s{xs) (%t / %b)");
    else
    {
        if (strlen (argument) > 50)
            argument[50] = '\0';
        strcpy (buf, argument);
        smash_tilde (buf);
        if (str_suffix ("%c", buf))
            strcat (buf, " ");

    }

    free_string (ch->prompt);
    ch->prompt = str_dup (buf);
    sprintf (buf, "Prompt set to %s\r\n", ch->prompt);
    SEND (buf, ch);
    return;
}

void do_combine (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_COMBINE))
    {
        SEND ("Long inventory selected.\r\n", ch);
        REMOVE_BIT (ch->comm, COMM_COMBINE);
    }
    else
    {
        SEND ("Combined inventory selected.\r\n", ch);
        SET_BIT (ch->comm, COMM_COMBINE);
    }
}

void do_noloot (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_CANLOOT))
    {
        SEND ("Your corpse is now safe from thieves.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_CANLOOT);
    }
    else
    {
        SEND ("Your corpse may now be looted.\r\n", ch);
        SET_BIT (ch->act, PLR_CANLOOT);
    }
}

void do_nofollow (CHAR_DATA * ch, char *argument)
{
    
	if ( IS_NPC(ch) || IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if (IS_SET (ch->act, PLR_NOFOLLOW))
    {
        SEND ("You now accept followers.\r\n", ch);
        REMOVE_BIT (ch->act, PLR_NOFOLLOW);
    }
    else
    {
        SEND ("You no longer accept followers.\r\n", ch);
        SET_BIT (ch->act, PLR_NOFOLLOW);
        die_follower (ch);
    }
}

void do_nosummon (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
    {
        if (IS_SET (ch->imm_flags, IMM_SUMMON))
        {
            SEND ("You are no longer immune to summon.\r\n", ch);
            REMOVE_BIT (ch->imm_flags, IMM_SUMMON);
        }
        else
        {
            SEND ("You are now immune to summoning.\r\n", ch);
            SET_BIT (ch->imm_flags, IMM_SUMMON);
        }
    }
    else
    {
        if (IS_SET (ch->act, PLR_NOSUMMON))
        {
            SEND ("You are no longer immune to summon.\r\n", ch);
            REMOVE_BIT (ch->act, PLR_NOSUMMON);
        }
        else
        {
            SEND ("You are now immune to summoning.\r\n", ch);
            SET_BIT (ch->act, PLR_NOSUMMON);
        }
    }
}

void do_look (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
	char wbuf[MSL] = "";
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj = NULL;
	AREA_DATA *pArea;
    char *pdesc;
    int door;
    int number, count;

    if (ch->desc == NULL)
        return;

    if (ch->position < POS_SLEEPING)
    {
        SEND ("You can't see anything but stars!\r\n", ch);
        return;
    }

    if (ch->position == POS_SLEEPING)
    {
        SEND ("You can't see anything, you're sleeping!\r\n", ch);
        return;
    }
	
    if (!check_blind (ch))
        return;	
		
		if (!IS_NPC (ch)
        && !IS_SET (ch->act, PLR_HOLYLIGHT) && room_is_dark (ch->in_room))
		{
			if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
				SEND ("Your light is out.\r\n",ch);
			SEND ("{DIt is pitch black...{x \r\n", ch);
			if (IS_AFFECTED(ch, AFF_INFRARED))
			{
				if (!IS_NPC (ch) && IS_SET (ch->act, PLR_AUTOEXIT))
				{
					SEND ("\r\n", ch);
					do_function (ch, &do_exits, "auto");
				}
			}
			show_char_to_char (ch->in_room->people, ch);		
			return;
		}
	

		argument = one_argument (argument, arg1);
		argument = one_argument (argument, arg2);
		number = number_argument (arg1, arg3);
		count = 0;

		if (!str_cmp(arg1, "overboard"))
		{			
			
			if (!ch->onBoard)
				return;			
			
			SEND ("{b[{x", ch);
			SEND (ch->ship->in_room->name, ch);
			SEND ("{b]{x", ch);

			if ((IS_IMMORTAL (ch)
				&& (IS_NPC (ch) || IS_SET (ch->act, PLR_HOLYLIGHT)))
				|| IS_BUILDER (ch, ch->ship->in_room->area))
			{
				sprintf (buf, "-{b[{xRoom %ld{b]{x", ch->ship->in_room->vnum);
				SEND (buf, ch);
				sprintf (buf, "-{b[{x%s{b]{x", ch->ship->in_room->area->name);
				SEND (buf, ch);
			}
			
			SEND ("\r\n", ch);
	
			if (arg1[0] == '\0'
				|| (!IS_NPC (ch) && !IS_SET (ch->comm, COMM_BRIEF)))
			{
				SEND ("  ", ch);
				SEND ("{x", ch);
				SEND (ch->ship->in_room->description, ch);
				SEND ("{x", ch);				
			}

			if (!IS_NPC (ch) && IS_SET (ch->act, PLR_AUTOEXIT))
			{
				SEND ("\r\n", ch);
				ship_exits(ch);
				//do_function (ch, &do_exits, "auto");
			}
						

			if (ch->switched == TRUE)
			{
				SEND ("\r\n", ch);
				ship_exits(ch);
				//do_function (ch, &do_exits, "auto");
			}
		
			if ((number_percent() + 5 <  get_skill(ch, gsn_herbalism) || is_affected(ch, gsn_detect_plants)) && ch->ship->in_room->can_forage == TRUE)			
			{
				SEND ("A nearby plant catches your eye.\r\n",ch);
				check_improve (ch, gsn_herbalism, TRUE, 2);
				
			}
		
			show_list_to_char (ch->ship->in_room->contents, ch, FALSE, FALSE, FALSE);
			show_char_to_char (ch->ship->in_room->people, ch);
			return;
		}
		
		
		if (arg1[0] == '\0' || !str_cmp (arg1, "auto"))
		{
			if (IS_SET(ch->act, PLR_AUTOMAP))
				display_map (ch);
				//show_the_map(ch);
			else
			{
		
				/* 'look' or 'look auto' */
				SEND ("{b[{x", ch);
				SEND (ch->in_room->name, ch);
				SEND ("{b]{x", ch);

				if ((IS_IMMORTAL (ch)
					&& (IS_NPC (ch) || IS_SET (ch->act, PLR_HOLYLIGHT)))
					|| IS_BUILDER (ch, ch->in_room->area))
				{
					sprintf (buf, "-{b[{xRoom %ld{b]{x", ch->in_room->vnum);
					SEND (buf, ch);
					sprintf (buf, "-{b[{x%s{b]{x", ch->in_room->area->name);
					SEND (buf, ch);
				}
				
				SEND ("\r\n", ch);
		
				if (arg1[0] == '\0'
					|| (!IS_NPC (ch) && !IS_SET (ch->comm, COMM_BRIEF)))
				{
					SEND ("  ", ch);
					SEND ("{x", ch);
					if (!IS_NULLSTR(ch->in_room->description))
						SEND (ch->in_room->description, ch);
					else
					{
					//Random room description code:
						char generic_desc[MSL];
						char room_desc[MSL];
						char time_desc[MSL];						
						char *formatted_desc;												
						char walk[MSL];
						
						// bzero (room_desc, (sizeof, room_desc));
						// bzero (time_desc, (sizeof, time_desc));
						
						
						generic_desc[0] = '\0';
						room_desc[0] = '\0';
						time_desc[0] = '\0';						
						
						//Grab a quick generic description.
						sprintf(generic_desc, "%s", generic_desc_by_sector[ch->in_room->sector_type]);						
						
						switch (time_info.hour)
						{
							default:
							case 0:
							case 1:
							case 2:
							case 3:
							case 4:
								sprintf(time_desc,"%s ", time_descs_by_sector[ch->in_room->sector_type]);
								break;
								
							
							case 5:
							case 6:
							case 7:														
								sprintf(time_desc,"%s ", time_descs_by_sector[ch->in_room->sector_type + 27]);
								break;
							
							
							case 8:
							case 9:
							case 10:
							case 11:
								sprintf(time_desc,"%s ", time_descs_by_sector[ch->in_room->sector_type + (27 * 2)]);
								break;
							
							case 12:
							case 13:
							case 14:
							case 15:
							case 16:
								sprintf(time_desc,"%s ", time_descs_by_sector[ch->in_room->sector_type + (27 * 3)]);
								break;
								
							case 17:
							case 18:
							case 19:							
								sprintf(time_desc,"%s ", time_descs_by_sector[ch->in_room->sector_type + (27 * 4)]);
								break;
						

							case 20:
							case 21:
							case 22:
							case 23:							
								sprintf(time_desc,"%s ", time_descs_by_sector[ch->in_room->sector_type + (27 * 5)]);
								break;						
						}
						
						
						sprintf(room_desc, "%s ", generic_desc);
						strcat(room_desc, time_desc);
						
						
						
						if (ch->in_room->area->curr_wind > 9)
						{
							OBJ_DATA *cloak = NULL;
							char cloak_desc[MSL];
							
							cloak_desc[0] = '\0';
						
							if (((cloak = get_eq_char (ch, ITEM_WEAR_NECK)) != NULL) && is_name("cloak", cloak->name))
							{								
								if (ch->in_room->area->curr_wind > 20)
									sprintf(cloak_desc, "Your cloak flaps violently in the wind. ");
								else
									sprintf(cloak_desc, "Your cloak flaps loosely in the wind. ");
							}
							
							if (!IS_NULLSTR(cloak_desc))	
								strcat (room_desc, cloak_desc);
						}
						
						//Leaves crunching in the forest.
						if (ch->in_room->sector_type == SECT_FOREST)
						{		
							OBJ_DATA *boots = NULL;
							walk[0] = '\0';
														
							if (ch->in_room->area->curr_precip < 0 && ch->in_room->area->precip_time)
							{
								boots = get_eq_char(ch, ITEM_WEAR_FEET);
								if (boots)								
									sprintf(walk, "Leaves crunch beneath your %s with each step. ", boots->short_descr);
								else
									sprintf(walk, "Leaves crunch beneath your bare feet. ");
							}
							
							if (!IS_NULLSTR(walk))
								strcat (room_desc, walk);
						
						}									
						
						//Do they see some tracks?////////////
						OBJ_DATA *obj_next;			
						for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)            
						{
							obj_next = obj->next_content;
							if (obj->item_type == ITEM_TRACKS && can_see_obj(ch,obj))
							{
								strcat (room_desc, "Recent passage is easily visible here. ");
								break;
							}
						}
						
						formatted_desc = str_dup(format_string(room_desc));
						SEND(formatted_desc,ch);
					}
					SEND ("{x", ch);
					if (IS_OUTSIDE(ch))
					{
						if (ch->in_room->area->precip_time == 1 )
						{
							switch (ch->in_room->area->curr_precip)
							{
								default: break;
								case RAIN:
									sprintf(wbuf, "\r\nSmall rain puddles are forming on the ground.\r\n");
									break;
								case SNOW:
									if (ch->in_room->area->curr_temp > 36)								
										sprintf(wbuf, "\r\nHalf melted snow puddles can be seen.\r\n");
									else
										sprintf(wbuf, "\r\nSmall snow piles are forming on the ground.\r\n");
									break;
								case HAIL:
									sprintf(wbuf, "\r\nThere are hail stones on the ground here and there.\r\n");
									break;
								case SLEET:
									sprintf(wbuf, "\r\nThe ground is very slightly wet.\r\n");
									break;
							}
						}
						if (ch->in_room->area->precip_time > 1 && ch->in_room->area->precip_time <= 3)
						{
							switch (ch->in_room->area->curr_precip)
							{
								default: break;
								case RAIN:
									sprintf(wbuf, "\r\nThe ground is a little wet.\r\n");
									break;
								case SNOW:
									if (ch->in_room->area->curr_temp > 36)								
										sprintf(wbuf, "\r\nHalf melted snow puddles can be seen.\r\n");
									else
										sprintf(wbuf, "\r\nThe ground is lightly covered in snow.\r\n");
									break;
								case HAIL:
									sprintf(wbuf, "\r\nThere are hail stones on the ground here and there.\r\n");
									break;
								case SLEET:
									sprintf(wbuf, "\r\nThe ground is a little wet.\r\n");
									break;
							}
						}
						else if (ch->in_room->area->precip_time > 3 && ch->in_room->area->precip_time <= 6)
						{
							switch (ch->in_room->area->curr_precip)
							{
								default: break;
								case RAIN:
									sprintf(wbuf, "\r\nThe ground is pretty wet.\r\n");
									break;
								case SNOW:
									if (ch->in_room->area->curr_temp > 36)								
										sprintf(wbuf, "\r\nThe ground is covered in slush.\r\n");
									else
										sprintf(wbuf, "\r\nThe ground has accumulated a blanket of snow.\r\n");
									break;
								case HAIL:
									sprintf(wbuf, "\r\nThe ground is dotted with small hail stones.\r\n");
									break;
								case SLEET:
									sprintf(wbuf, "\r\nThe ground is pretty wet and slightly slick.\r\n");
									break;
							}
						}
						else
						{
							switch (ch->in_room->area->curr_precip)
							{
								default: break;
								case RAIN:
									sprintf(wbuf, "\r\nThe ground is very wet and muddy.\r\n");
									break;
								case SNOW:
									if (ch->in_room->area->curr_temp > 36)								
										sprintf(wbuf, "\r\nThe ground is covered in slush.\r\n");
									else
										sprintf(wbuf, "\r\nThere is several inches of snow on the ground.\r\n");
									break;
								case HAIL:
									sprintf(wbuf, "\r\nThe ground is covered with hail stones.\r\n");
									break;
								case SLEET:
									sprintf(wbuf, "\r\nThe ground is very wet and slick.\r\n");
									break;
							}
						}
						switch (ch->in_room->sector_type)
						{				
							default:
								if (IS_OUTSIDE(ch))
									SEND(wbuf, ch);
								break;
							case SECT_WATER_SWIM:
							case SECT_WATER_NOSWIM:
							case SECT_OCEANFLOOR:
							case SECT_AIR:
							case SECT_LAVA:
							case SECT_DOCK:
							case SECT_SHIP:
							case SECT_VOID:
								break;					
						}
					}
				}
			}
			if (!IS_NPC (ch) && IS_SET (ch->act, PLR_AUTOEXIT))
			{
				SEND ("\r\n", ch);
				do_function (ch, &do_exits, "auto");
			}
						

			if (ch->switched == TRUE)
			{
				SEND ("\r\n", ch);
				do_function (ch, &do_exits, "auto");
			}
		
			if (ch->in_room->sector_type != SECT_LAVA && ch->in_room->sector_type != SECT_WASTELAND && ch->in_room->sector_type != SECT_DESERT && ch->in_room->sector_type != SECT_VOID)
			{
				if ((number_percent() + 5 <  get_skill(ch, gsn_herbalism) || is_affected(ch, gsn_detect_plants)) || IS_IMMORTAL(ch) && ch->in_room->can_forage == TRUE)			
				{
					SEND ("A nearby plant catches your eye.\r\n",ch);			
					check_improve (ch, gsn_herbalism, TRUE, 2);
				}
			}
			for (pArea = area_first; pArea; pArea = pArea->next)
			{
				if (IS_SET(pArea->area_flags, AREA_SHIP) && pArea->ship_vnum == ch->in_room->vnum)
				{
					sprintf(buf, "You see the ship, '%s', floating in the water here.\r\n", pArea->name);
					SEND(buf, ch);
				}
			}
			show_list_to_char (ch->in_room->contents, ch, FALSE, FALSE, FALSE);
			show_char_to_char (ch->in_room->people, ch);
			return;
		}		
		//Now look at something with an argument.
		if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in")
			|| !str_cmp (arg1, "on"))
		{
        /* 'look in' */
        if (arg2[0] == '\0')
        {
            SEND ("Look in what?\r\n", ch);
            return;
        }

        if ((obj = get_obj_here( ch, NULL, arg2)) == NULL)
        {
            SEND ("You do not see that here.\r\n", ch);
            return;
        }

        switch (obj->item_type)
        {
            default:
                SEND ("That is not a container.\r\n", ch);
                break;

            case ITEM_DRINK_CON:
                if (obj->value[1] <= 0)
                {
                    SEND ("It is empty.\r\n", ch);
                    break;
                }

                sprintf (buf, "It's %sfilled with  a %s liquid.\r\n",
                         obj->value[1] < obj->value[0] / 4
                         ? "less than half-" :
                         obj->value[1] < 3 * obj->value[0] / 4
                         ? "about half-" : "more than half-",
                         liq_table[obj->value[2]].liq_color);

                SEND (buf, ch);
                break;

            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                if (IS_SET (obj->value[1], CONT_CLOSED))
                {
                    SEND ("It is closed.\r\n", ch);
                    break;
                }

                act ("$p holds:", ch, obj, NULL, TO_CHAR);
                show_list_to_char (obj->contains, ch, TRUE, TRUE, FALSE);
                break;
        }
        return;
    }

    if ((victim = get_char_room ( ch, NULL, arg1)) != NULL)
    {
        show_char_to_char_1 (victim, ch);
        return;
    }

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
        if (can_see_obj (ch, obj))
        {                        /* player can see object */
            pdesc = get_extra_descr (arg3, obj->extra_descr);
            if (pdesc != NULL)
            {
                if (++count == number)
                {
                    SEND (pdesc, ch);
                    return;
                }
                else
                    continue;
            }

            pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
            if (pdesc != NULL)
            {
                if (++count == number)
                {
                    SEND (pdesc, ch);
                    return;
                }
                else
                    continue;
            }

            if (is_name (arg3, obj->name))
                if (++count == number)
                {
                    SEND (obj->description, ch);
                    SEND ("\r\n", ch);
                    return;
                }
        }
    }

    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
        if (can_see_obj (ch, obj))
        {
            pdesc = get_extra_descr (arg3, obj->extra_descr);
            if (pdesc != NULL)
                if (++count == number)
                {
                    SEND (pdesc, ch);
                    return;
                }

            pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
            if (pdesc != NULL)
                if (++count == number)
                {
                    SEND (pdesc, ch);
                    return;
                }

            if (is_name (arg3, obj->name))
                if (++count == number)
                {
                    SEND (obj->description, ch);
                    SEND ("\r\n", ch);
                    return;
                }
        }
    }

    pdesc = get_extra_descr (arg3, ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
        if (++count == number)
        {
            SEND (pdesc, ch);
            return;
        }
    }

    if (count > 0 && count != number)
    {
        if (count == 1)
            sprintf (buf, "You only see one %s here.\r\n", arg3);
        else
            sprintf (buf, "You only see %d of those here.\r\n", count);

        SEND (buf, ch);
        return;
    }

    if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
        door = 0;
    else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
        door = 1;
    else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
        door = 2;
    else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
        door = 3;
    else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
        door = 4;
    else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
        door = 5;
    else
    {
        SEND ("You do not see that here.\r\n", ch);
        return;
    }

    /* 'look direction' */
    if ((pexit = ch->in_room->exit[door]) == NULL)
    {
        SEND ("Nothing special there.\r\n", ch);
        return;
    }

    if (pexit->description != NULL && pexit->description[0] != '\0')
        SEND (pexit->description, ch);
    else
        SEND ("Nothing special there.\r\n", ch);

    if (pexit->keyword != NULL
        && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ')
    {
        if (IS_SET (pexit->exit_info, EX_CLOSED))
        {
            act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        }
        else if (IS_SET (pexit->exit_info, EX_ISDOOR))
        {
            act ("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
        }
    }

	wbuf[0] = '\0';
	free_string(wbuf);
    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_look, argument);
}

void do_examine (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Examine what?\r\n", ch);
        return;
    }

    do_function (ch, &do_look, arg);

    if ((obj = get_obj_here( ch, NULL, arg)) != NULL)
    {
        switch (obj->item_type)
        {
            default:
                break;

            case ITEM_JUKEBOX:
                do_function (ch, &do_play, "list");
                break;

            case ITEM_MONEY:
                if (obj->value[0] == 0)
                {
                    if (obj->value[1] == 0)
                        sprintf (buf,
                                 "Odd...there's no coins in the pile.\r\n");
                    else if (obj->value[1] == 1)
                        sprintf (buf, "Wow. One gold coin.\r\n");
                    else
                        sprintf (buf,
                                 "There are %d gold coins in the pile.\r\n",
                                 obj->value[1]);
                }
                else if (obj->value[1] == 0)
                {
                    if (obj->value[0] == 1)
                        sprintf (buf, "Wow. One silver coin.\r\n");
                    else
                        sprintf (buf,
                                 "There are %d silver coins in the pile.\r\n",
                                 obj->value[0]);
                }
                else
                    sprintf (buf,
                             "There are %d gold and %d silver coins in the pile.\r\n",
                             obj->value[1], obj->value[0]);
                SEND (buf, ch);
                break;

            case ITEM_DRINK_CON:
            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                sprintf (buf, "in %s", argument);
                do_function (ch, &do_look, buf);
        }
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits (CHAR_DATA * ch, char *argument)
{
    extern char *const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    fAuto = !str_cmp (argument, "auto");

    if (!check_blind (ch))
        return;

		if (fAuto)
			sprintf (buf, "{b[{xExits:");
		else if (IS_IMMORTAL (ch))
			sprintf (buf, "Obvious exits from room %ld:\r\n", ch->in_room->vnum);
		else
			sprintf (buf, "Obvious exits:\r\n");

		found = FALSE;
		for (door = 0; door <= 5; door++)
		{
			if ((pexit = ch->in_room->exit[door]) != NULL
				&& pexit->u1.to_room != NULL
				&& can_see_room (ch, pexit->u1.to_room))            
			{
				found = TRUE;
				if (fAuto)
				{
					if ( IS_SET (pexit->exit_info, EX_HIDDEN) )
					{
						if (!IS_IMMORTAL(ch) && !is_affected(ch, gsn_detect_hidden) && number_range(1,100) > (get_skill(ch, gsn_detect_secret) / 2))
							continue;
					}
					
					if ( IS_SET (pexit->exit_info, EX_SECRET) )
					{
						if (!IS_IMMORTAL(ch) && number_range(1,100) > (get_skill(ch, gsn_detect_secret) / 2))
							continue;
					}
					
					strcat (buf, " ");
					
					if (IS_SET (pexit->exit_info, EX_CLOSED))
					{
						if (IS_SET (pexit->exit_info, EX_HIDDEN))
							strcat (buf, "{r({x");					
						else if (IS_SET (pexit->exit_info, EX_SECRET))
							strcat (buf, "{m({x");					
						else
							strcat (buf, "{x(");
						
						strcat (buf, MXPTAG ("Ex"));
						strcat (buf, dir_name[door]);
						// if (door < 5)
						// {
							// if ((pexit = ch->in_room->exit[door+1]) != NULL)
								// strcat (buf, ",");
						// }
						strcat (buf, MXPTAG ("/Ex"));
						if (IS_SET (pexit->exit_info, EX_HIDDEN))
							strcat (buf, "{r){x");
						else if (IS_SET (pexit->exit_info, EX_SECRET))
							strcat (buf, "{m){x");
						else
							strcat (buf, "{x)");							
					}
					else
					{
						strcat (buf, MXPTAG ("Ex"));
						strcat (buf, dir_name[door]);
						// if (door < 5)
						// {
							// if ((pexit = ch->in_room->exit[door+1]) != NULL)
								// strcat (buf, ",");
						// }
						strcat (buf, MXPTAG ("/Ex"));
					}
				}
				else
				{
					sprintf (buf + strlen (buf), "%-5s - %s",
							 capitalize (dir_name[door]),
							 room_is_dark (pexit->u1.to_room)
							 ? "Too dark to tell" : pexit->u1.to_room->name);
					if (IS_IMMORTAL (ch))
						sprintf (buf + strlen (buf),
								 " (room %ld)\r\n", pexit->u1.to_room->vnum);
					else
						sprintf (buf + strlen (buf), "\r\n");
				}
			}
		}
    if (!found)
        strcat (buf, fAuto ? " none" : "None.");

    if (fAuto)
        strcat (buf, "{b]{x");
	
	if (IS_IMMORTAL(ch) && (ch->in_room->reset_first))
		strcat (buf, " {b[{xHas Resets{b]{x");
	
	strcat (buf, "\r\n");
	
    SEND (buf, ch);
    return;
}

void ship_exits(CHAR_DATA *ch)
{
	extern char *const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

	fAuto = TRUE;
	
	if (fAuto)
		sprintf (buf, "{b[{xExits:");
	else if (IS_IMMORTAL (ch))
		sprintf (buf, "Obvious exits from room %ld:\r\n", ch->ship->in_room->vnum);
	else
		sprintf (buf, "Obvious exits:\r\n");

	found = FALSE;
	for (door = 0; door <= 5; door++)
	{
		if ((pexit = ch->ship->in_room->exit[door]) != NULL
			&& pexit->u1.to_room != NULL
			&& can_see_room (ch, pexit->u1.to_room))            
		{
			found = TRUE;
			if (fAuto)
			{
				if ( IS_SET (pexit->exit_info, EX_HIDDEN) )
				{
					if (!IS_IMMORTAL(ch) && !is_affected(ch, gsn_detect_hidden) && number_range(1,100) > (get_skill(ch, gsn_detect_secret) / 2))
						continue;
				}
				
				if ( IS_SET (pexit->exit_info, EX_SECRET) )
				{
					if (!IS_IMMORTAL(ch) && number_range(1,100) > (get_skill(ch, gsn_detect_secret) / 2))
						continue;
				}
				
				strcat (buf, " ");
				if (IS_SET (pexit->exit_info, EX_CLOSED))
				{
					if (IS_SET (pexit->exit_info, EX_HIDDEN))
						strcat (buf, "{r({x");					
					else if (IS_SET (pexit->exit_info, EX_SECRET))
						strcat (buf, "{m({x");					
					else
						strcat (buf, "{x(");
					
					strcat (buf, dir_name[door]);
					
					if (IS_SET (pexit->exit_info, EX_HIDDEN))
						strcat (buf, "{r){x");
					else if (IS_SET (pexit->exit_info, EX_SECRET))
						strcat (buf, "{m){x");
					else
						strcat (buf, "{x)");
				}
				else
					strcat (buf, MXPTAG ("Ex"));
					strcat (buf, dir_name[door]);
					strcat (buf, MXPTAG ("/Ex"));
			}
			else
			{
				sprintf (buf + strlen (buf), "%-5s - %s",
						 capitalize (dir_name[door]),
						 room_is_dark (pexit->u1.to_room)
						 ? "Too dark to tell" : pexit->u1.to_room->name);
				if (IS_IMMORTAL (ch))
					sprintf (buf + strlen (buf),
							 " (room %ld)\r\n", pexit->u1.to_room->vnum);
				else
					sprintf (buf + strlen (buf), "\r\n");
			}
		}
	}
		
	 if (!found)
        strcat (buf, fAuto ? " none" : "None.\r\n");

    if (fAuto)
        strcat (buf, "{b]{x\r\n");

    SEND (buf, ch);
    return;
}

void do_worth (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        sprintf (buf, "You have {y%ld{x gold and {D%ld{x silver.\r\n",
                 ch->gold, ch->silver);
        SEND (buf, ch);
        return;
    }	
	if (!IS_MCLASSED(ch))
	{
		sprintf (buf,
				"You have {y%ld{x gold, {D%ld{x silver, and %ld experience (%ld exp to level).\r\n",
				ch->gold, ch->silver, ch->exp, exp_per_level (ch, FALSE));				
	}
	else
	{
		sprintf (buf,
				"You have {y%ld{x gold, {D%ld{x silver, and %ld / %ld experience (%ld / %ld exp to level).\r\n",
				ch->gold, ch->silver, ch->exp, ch->exp2, exp_per_level (ch, FALSE), exp_per_level (ch, TRUE));
	}
	
    SEND (buf, ch);
	
	if (ch->bank_act > 0)
	{
		sprintf (buf, "You currently have {y%ld{x gold in your bank account.\r\n", ch->bank_amt);
		SEND(buf, ch);
	}
	
	if (ch->clan > CLAN_LONER)
	{
		sprintf (buf, "You currently have {W%d{X clan points.\r\n", ch->clan_points);
		SEND(buf, ch);
	}
    return;
}


void do_score2 (CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	//int i = 0;
	
	sprintf(buf, "Name:  %12s			Alignment: %8d\r\n", ch->name, ch->alignment);
	SEND(buf, ch);
	
	sprintf(buf, "Race:  %12s			Deity: %8s\r\n", race_table[ch->race].name, god_table[ch->god].name);
	SEND(buf, ch);
	
	if (!IS_MCLASSED(ch))
		sprintf(buf, "Class: %12s 			Level: %8d\r\n",ch_class_table[ch->ch_class].name, ch->level);
	else
		sprintf(buf, "Class: %5s/%5s 			Level: %4d/%4d\r\n",ch_class_table[ch->ch_class].who_short,ch_class_table[ch->ch_class2].who_short, ch->level, ch->level2);
	SEND(buf, ch);
	
	return;
}

void do_score (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];    
    //int i;
	
	if (ch->guildpoints >= 0)
		ch->guildrank = 0;
	if (ch->guildpoints >= 200)
		ch->guildrank = 1;
	if (ch->guildpoints >= 1000)
		ch->guildrank = 2;
	if (ch->guildpoints >= 3000)
		ch->guildrank = 3;
	if (ch->guildpoints >= 10000)
		ch->guildrank = 4;
	if (ch->guildpoints >= 20000)
		ch->guildrank = 5;
	
	// if (IS_MCLASSED(ch))
	// {
	// sprintf (buf,
             // "You are %s%s%s, level (%d / %d), %d years old.\r\n",
             // ch->name,			 
			 // (IS_NULLSTR(ch->last_name) || !str_cmp(ch->citizen, "(null)")) ? " " : "",
			 // (IS_NULLSTR(ch->last_name) || !str_cmp(ch->citizen, "(null)")) ? ch->last_name : "",
             // ch->level, ch->level2, GET_AGE(ch));
    // SEND (buf, ch);
	// }
	// else
	// {
    // sprintf (buf,
             // "You are %s%s%s, level %d, %d years old (%d hours).\r\n",
             // ch->name,			 
			 // (IS_NULLSTR(ch->last_name) || !str_cmp(ch->last_name, "(null)")) ? " " : "",
			 // (IS_NULLSTR(ch->last_name) || !str_cmp(ch->last_name, "(null)")) ? ch->last_name : "",
             // ch->level, GET_AGE(ch),
             // (ch->played + (int) (current_time - ch->logon)) / 3600);
    // SEND (buf, ch);
	// }
	
/*
	sprintf (buf, "You have %s eyes and %s hair styled as a %s.\r\n",eye_color_flags[ch->eyes].name, hair_color_flags[ch->hair].name, haircut_flags[ch->haircut].name);
	SEND (buf, ch);

	if (!IS_SET(ch->facial_hair, FACIAL_NONE))
	{
		sprintf(buf, "You are sporting %s.\r\n", facial_hair_flags[ch->facial_hair].name);
		SEND(buf, ch);
	}
*/	
	// if (!IS_NULLSTR(ch->citizen) && str_cmp(ch->citizen, "(null)"))
	// {
		// sprintf (buf, "You are a citizen of %s, part of the %s faction.\r\n", ch->citizen, faction_table[ch->faction].name);
		// SEND (buf, ch);
	// }
	// else
		// SEND ("You call no town or city home, which means you favor no faction.\r\n", ch);
	
		
	// sprintf (buf, "Your guild rank is %s.\r\n", GUILD_RANK_STRING(ch));
	// SEND(buf, ch);
	
    // if (IS_MCLASSED(ch))
	// {
		// sprintf (buf,
             // "You have scored %ld / %ld experience points.\r\n",
             // ch->exp, ch->exp2);
		// SEND (buf, ch);
		// /* RT shows exp to level */
		// if (!IS_IMMORTAL(ch))
		// {
			// if (!IS_NPC (ch) && ch->level < MCLASS_ONE && ch->level2 < MCLASS_TWO)
			// {
				// sprintf (buf,
					 // "You need to reach %ld / %ld experience to raise a level.\r\n",
					 // (exp_per_level (ch, FALSE)),
					 // (exp_per_level (ch, TRUE)));
				// SEND (buf, ch);
			// }
			// if (!IS_NPC (ch) && ch->level < MCLASS_ONE && ch->level2 >= MCLASS_TWO)
			// {
				// sprintf (buf,
					 // "You need to reach %ld experience to raise a level.\r\n",
					 // exp_per_level (ch, FALSE));
				// SEND (buf, ch);
			// }
			// if (!IS_NPC (ch) && ch->level >= MCLASS_ONE && ch->level2 < MCLASS_TWO)
			// {
				// sprintf (buf,
					 // "You need to reach %ld experience to raise a level.\r\n",                 
					 // (exp_per_level (ch, TRUE)));
				// SEND (buf, ch);
			// }
		// }
	// }
	// else
	// {
		// sprintf (buf,
             // "You have scored %ld experience points.\r\n",
             // ch->exp);
		// SEND (buf, ch);
		
		// /* RT shows exp to level */
		// if (!IS_NPC (ch) && ch->level < LEVEL_HERO)
		// {
        // sprintf (buf,
                 // "You need to reach %ld experience to raise a level.\r\n",
                 // exp_per_level (ch, FALSE));
        // SEND (buf, ch);
		// }
	// }	

	//sprintf (buf, "Alignment: %d.\r\n", ch->alignment);
    //SEND (buf, ch);

    // SEND ("You are the ", ch);
    // if (ch->alignment == 1000)
        // SEND ("lawful good follower of ", ch);
    // else if (ch->alignment == 750)
        // SEND ("chaotic good follower of ", ch);
    // else if (ch->alignment == 500)
        // SEND ("neutral good follower of ", ch);
    // else if (ch->alignment == 250)
        // SEND ("lawful neutral follower of ", ch);
    // else if (ch->alignment == 0)
        // SEND ("true neutral follower of ", ch);
    // else if (ch->alignment == -250)
        // SEND ("chaotic neutral follower of ", ch);
    // else if (ch->alignment == -500)
        // SEND ("neutral evil follower of ", ch);
    // else if (ch->alignment == -750)
        // SEND ("chaotic evil follower of ", ch);
    // else if (ch->alignment == -1000)
        // SEND ("lawful evil follower of ", ch);
	// else
	// {		
		// SEND ("broken alignment follower of ", ch);
		// sprintf(buf, "Invalid alignment for %s, (%d)", ch->name, ch->alignment);
		// log_string(buf);
		// wiznet ("$N has an invalid alignment.", ch, NULL,
                    // WIZ_SECURE, 0, get_trust(ch));
	// }	
	// sprintf( buf, "%s.\r\n",(god_table[ch->god].name));
    // SEND( buf, ch);
	

	// if (IS_MCLASSED(ch))
	// {
    // sprintf (buf, "Race: %s  Size: %s  Sex: %s ch_class: %s / %s\r\n",
             // race_table[ch->race].name, size_table[ch->size].name,
             // ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
             // IS_NPC (ch) ? "mobile" :ch_class_table[ch->ch_class].name, 
			 // IS_NPC (ch) ? "mobile" :ch_class_table[ch->ch_class2].name);
    // SEND (buf, ch);
	// }	
	// else
	// {
	// sprintf (buf, "Race: %s  Size: %s  Sex: %s ch_class: %s\r\n",
             // race_table[ch->race].name, size_table[ch->size].name,
             // ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
             // IS_NPC (ch) ? "mobile" :ch_class_table[ch->ch_class].name);
    // SEND (buf, ch);
	// }
	
	sprintf (buf,"\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{r|{x Name    %12s {r|{x Hometown  %14s {r|{x\r\n{r|{x Deity   %12s {r|{x Society Rank  %10s {r|{x\r\n{r|{x Position  %10s {r|{x Guild Rank    %10s {r|{x\r\n",
		ch->name,
		(ch->citizen == NULL ? "" : ch->citizen),
		god_table[ch->god].name,
		(ch->society_rank > -1 ? (ch->sex == SEX_FEMALE ? society_table[ch->society_rank].female_name : society_table[ch->society_rank].male_name) : ""),
		SHORT_POSITION_STRING(ch),
		GUILD_RANK_STRING(ch)
		);
		
	SEND(buf, ch);
	
	if (IS_MCLASSED(ch))
	{
		sprintf (buf,
             "{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{r|{x Level      %2d/%2d     {r|{x Align   %16s {r|{x\r\n{r|{x Health     %4d/%4d {r|{x Gender  %16s {r|{x\r\n{r|{x Mana       %4d/%4d {r|{x Race    %16s {r|{x\r\n{r|{x Movement   %4d/%4d {r|{x Class    %7s/%7s {r|{x\r\n{r|{x %6s     %4d%1s%4d {r|{x Size    %16s {r|{x\r\n",
			 ch->level, ch->level2,
			 GET_ALIGN_STRING(ch),
             ch->hit, 
			 ch->max_hit,
			 ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",             
             ch->mana, 
			 ch->max_mana, 
			 race_table[ch->race].name,
			 ch->move, 
			 ch->max_move,
			ch_class_table[ch->ch_class].name,			 
			ch_class_table[ch->ch_class2].name,			 
			 ch->has_talent ? "Talent" : "      ",
			 ch->has_talent ? ch->pp : 0,
			 ch->has_talent ? "/" : " ",
			 ch->has_talent ? ch->max_pp : 0,
			 size_table[what_size(ch)].name);
		SEND (buf, ch);
	}
	else
	{
		sprintf (buf,
             "{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{r|{x Level      %5d     {r|{x Align   %16s {r|{x\r\n{r|{x Health     %4d/%4d {r|{x Gender  %16s {r|{x\r\n{r|{x Mana       %4d/%4d {r|{x Race    %16s {r|{x\r\n{r|{x Movement   %4d/%4d {r|{x Class   %16s {r|{x\r\n{r|{x %6s     %4d%1s%4d {r|{x Size    %16s {r|{x\r\n",
             ch->level,
			 GET_ALIGN_STRING(ch),
			 ch->hit, 
			 ch->max_hit,
			 ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",             
             ch->mana, 
			 ch->max_mana, 
			 race_table[ch->race].name,
			 ch->move, 
			 ch->max_move,
			ch_class_table[ch->ch_class].name,			 
			 ch->has_talent ? "Talent" : "      ",
			 ch->has_talent ? ch->pp : 0,
			 ch->has_talent ? "/" : " ",
			 ch->has_talent ? ch->max_pp : 0,
			 size_table[what_size(ch)].name);
		SEND (buf, ch);
	}
	// if (ch->has_talent && ch->max_pp > 0)
	// {
		// sprintf (buf, "You have %d/%d psionic points.\r\n",
			// ch->pp, ch->max_pp);
		// SEND (buf, ch);
	// }

	// sprintf (buf,
             // "Pkills: {D%d{x       Pdeaths: {D%d{x       Mkills: {D%d{x\r\n",
             // ch->pkill, ch->pdeath, ch->mkill);
    // SEND (buf, ch);

    sprintf (buf,
	
             "{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{r|{x Str: %2d {r({x%2d{r) |{x Practices %3d {r|{x PKills       %3d {r|{x\r\n{r| {xInt: %2d {r({x%2d{r) |{x Trains     %2d {r|{x PDeaths      %3d {r|{x\r\n{r| {xWis: %2d {r({x%2d{r) |{x GuildP  %5d {r|{x MKills     %5d {r|{x\r\n{r| {xDex: %2d {r({x%2d{r) |-=-=-=-=-=-=-=-| {xExplored  %6d {r|{x\r\n{r| {xCon: %2d {r({x%2d{r) |{x Gold   {Y%6ld{x {r|{x Age          %3d {r|{x\r\n{r| {xCha: %2d {r({x%2d{r) |{x Silver {D%6ld{x {r|{x Hours       %4d {r|{x\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",
             ch->perm_stat[STAT_STR],
             GET_STR(ch),
			 ch->practice,
			 ch->pkill,
             ch->perm_stat[STAT_INT],
             GET_INT(ch),
			 ch->train,
			 ch->pdeath,
             ch->perm_stat[STAT_WIS],
             GET_WIS(ch),
			 ch->guildpoints,
			 ch->mkill,
             ch->perm_stat[STAT_DEX],
             GET_DEX(ch),
			 ch->pcdata->explored->set,
             ch->perm_stat[STAT_CON], 
			 GET_CON(ch),
			 ch->gold,
			 ch->age,
			 ch->perm_stat[STAT_CHA], 
			 GET_CHA(ch),
			 ch->silver,
			 (ch->played + (int) (current_time - ch->logon)) / 3600);
    SEND (buf, ch);	
	
	ch->bulk = get_bulk(ch, TRUE);    
	
	
	sprintf (buf, "{r|{x Armor Class:            {r|{x Hitroll:     {r({x%3d{r)    |{x\r\n{r|{x pierce %4d  bash  %4d {r|{x Melee Dam:   {r({x%3d{r/{x+%2d{r){x{r|\r\n|{x slash  %4d  magic %4d {r|{x Spell Dam:   {r({x%3d{r/{x%3d{r)|{x\r\n{r|{x Spell Saves: {r({x%3d{r){x      {r|{x Melee Crit:  {r({x%2d%s{r){x    {r|{x\r\n{r|{x Armor Bulk:  {r({x%3d{r){x      {r|{x Spell Crit:  {r({x%2d%s{r)    |{x\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",
		GET_HITROLL (ch),
		GET_AC (ch, AC_PIERCE),
		GET_AC (ch, AC_BASH),
		GET_DAMROLL (ch), (int)(GET_DAMROLL(ch) * 4 / 10),
		GET_AC (ch, AC_SLASH), 
		GET_AC (ch, AC_EXOTIC),
		ch->spell_dam,get_spell_damage_bonus(ch),
		ch->saving_throw,
		get_melee_crit_chance(ch), "%", 
		ch->bulk,
		get_spell_crit_chance(ch), "%");
	

	SEND (buf, ch);
	if (IS_MCLASSED(ch))
	{
		sprintf (buf,
				 "{r|{x Experience                    %8ld/%8ld {r|{x\r\n{r|{x Needed                        %8ld/%8ld {r|{x\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",
					ch->exp, ch->exp2,
					exp_per_level (ch, FALSE),
					exp_per_level (ch, TRUE)
					);
		SEND (buf, ch);
	}
	else
	{
		sprintf (buf,
				 "{r|{x Experience                     %16ld {r|{x\r\n{r|{x Needed                         %16ld {r|{x\r\n{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",
					ch->exp,
					exp_per_level (ch, FALSE)					
					);
		SEND (buf, ch);
	}		
	//New alcohol modifiers. 12/04/2010
	if (DRINKS(ch) > 0 || ch->pcdata->condition[COND_EUPHORIC] > 0 || ch->pcdata->condition[COND_THIRST] == 0 || ch->pcdata->condition[COND_HUNGER] == 0)
	{
		if (!IS_NPC (ch) && IS_BUZZED(ch) && DRINKS(ch) < 6)
			SEND ("{r|{x You feel slightly buzzed.                       {r|{x\r\n", ch);
		if (!IS_NPC (ch) && IS_TIPSY(ch) && DRINKS(ch) < 10)
			SEND ("{r|{x You feel a bit tipsy.                           {r|{x\r\n", ch);
		if (!IS_NPC (ch) && IS_DRUNK(ch) && DRINKS(ch) < 13)
			SEND ("{r|{x You are drunk.                                  {r|{x\r\n", ch);
		if (!IS_NPC (ch) && IS_HAMMERED(ch))	
			SEND ("{r|{x You are hammered drunk!                         {r|{x\r\n", ch);
			
		if (!IS_NPC (ch) && ch->pcdata->condition[COND_EUPHORIC] > 0)
			SEND ("{r|{x You feel euphoric.                              {r|{x\r\n", ch);
		if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
			SEND ("{r|{x You are thirsty.                                {r|{x\r\n", ch);
		if (!IS_NPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
			SEND ("{r|{x You are hungry.                                 {r|{x\r\n", ch);
		
		SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n{x",ch);
	}
	if (ch->has_talent)
	{
		SEND ("You have unrealized psionic potential.\r\n",ch);
	}
	
	//Ranger favored enemy:
	if ( IS_RANGER(ch) )
	{
		PRINTF (ch, "Your favored enemy is the %s.\r\n", race_table[ch->favored_enemy].name);
	}
	
	//Elemental Spec check:
	if ( IS_WIZARD(ch) && ch->pcdata->elementalSpec > 0 )
	{
		PRINTF(ch,"Your elemental affinity is towards %s.\r\n",specTable[ch->pcdata->elementalSpec].name);
	}	

	if ( IS_FIGHTER(ch) && ch->pcdata->weaponSpec > 0 )
	{
		PRINTF(ch,"Your weapon specialization lies in %ss.\r\n",wpnSpecTable[ch->pcdata->weaponSpec].name);
	}
	
	if (!IS_NULLSTR(ch->walk_desc))
		PRINTF(ch, "Your walk description is set to: %s\r\n", ch->walk_desc);	

		
	// if (!IS_NULLSTR(ch->email))
		// PRINTF(ch, "Your email is set to: %s\r\n", ch->email);	
	
	/* RT wizinvis and holy light */
    if (IS_IMMORTAL (ch))
    {
        SEND ("Holy Light: ", ch);
        if (IS_SET (ch->act, PLR_HOLYLIGHT))
            SEND ("{Gon{x", ch);
        else
            SEND ("{Roff{x", ch);

        if (ch->invis_level)
        {
            sprintf (buf, "  Invisible: level %d", ch->invis_level);
            SEND (buf, ch);
        }

        if (ch->incog_level)
        {
            sprintf (buf, "  Incognito: level %d", ch->incog_level);
            SEND (buf, ch);
        }
        SEND ("\r\n", ch);
    }	
	
	if (get_trust (ch) != ch->level)
    {
        sprintf (buf, "You are trusted at level %d.\r\n", get_trust (ch));
        SEND (buf, ch);
    }
	
	if (ch->wimpy > 0)
	{
		sprintf (buf, "Wimpy set to %d hit points.\r\n", ch->wimpy);
		SEND (buf, ch);
	}
	
    // /* print AC values */
	
	// SEND( "{r|{x  Armorch_class Values:\r\n",ch);
	// sprintf (buf, "{r|{x  pierce: %d  bash: %d\r\n",
			 // GET_AC (ch, AC_PIERCE),
			 // GET_AC (ch, AC_BASH));
	// SEND (buf, ch);		 
	// sprintf ( buf,  "{r|{x  slash: %d  magic: %d \r\n",
			 // GET_AC (ch, AC_SLASH), 
			 // GET_AC (ch, AC_EXOTIC));
	// SEND (buf, ch);    
	
	// sprintf (buf, "{r|{x  Spell Saves: {r({x%d{r){x Armor Bulk: {r({x%d{r){x\r\n", ch->saving_throw, ch->bulk);
	// SEND (buf, ch);
	
    // for (i = 0; i < 4; i++)
    // {
        // char *temp;

        // switch (i)
        // {
            // case (AC_PIERCE):
                // temp = "piercing";
                // break;
            // case (AC_BASH):
                // temp = "bashing";
                // break;
            // case (AC_SLASH):
                // temp = "slashing";
                // break;
            // case (AC_EXOTIC):
                // temp = "magic";
                // break;
            // default:
                // temp = "error";
                // break;
        // }

        // SEND ("You are ", ch);

        // if (GET_AC (ch, i) >= 101)
            // sprintf (buf, "hopelessly vulnerable to %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= 80)
            // sprintf (buf, "defenseless against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= 60)
            // sprintf (buf, "barely protected from %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= 40)
            // sprintf (buf, "slightly armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= 20)
            // sprintf (buf, "somewhat armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= 0)
            // sprintf (buf, "armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= -20)
            // sprintf (buf, "well-armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= -40)
            // sprintf (buf, "very well-armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= -60)
            // sprintf (buf, "heavily armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= -80)
            // sprintf (buf, "superbly armored against %s.\r\n", temp);
        // else if (GET_AC (ch, i) >= -100)
            // sprintf (buf, "almost invulnerable to %s.\r\n", temp);
        // else
            // sprintf (buf, "divinely armored against %s.\r\n", temp);

        // SEND (buf, ch);
    // }
	
    //if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
    //    do_function (ch, &do_affects, "");
}




void do_affects (CHAR_DATA * ch, char *argument)
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MSL];	

    if (ch->affected != NULL)
    {
        //SEND ("You are affected by the following spells:\r\n\r\n", ch);
		SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",ch);
		SEND ("{r|{x Spell/Skill:             Affect:                                    {r|{x\r\n",ch);
		SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",ch);
        for (paf = ch->affected; paf != NULL; paf = paf->next)
        {
            if (paf_last != NULL && paf->type == paf_last->type)                
                sprintf (buf, "{r|{x                          ");                
            else
                sprintf (buf, "{r|{x %-25s", skill_table[paf->type]	.name);

            SEND (buf, ch);

           
			sprintf (buf,
					 "modifies %12s by %3d ",
					 affect_loc_name (paf->location), paf->modifier);
			SEND (buf, ch);
			if (paf->duration == -1)
				sprintf (buf, "permanently.           {r|{x");
			else
				sprintf (buf, "for %2d hour%s.%s {r|{x", paf->duration, paf->duration > 1 ? "s" : "", paf->duration > 1 ? "" : " ");
			SEND (buf, ch);
            

            SEND ("\r\n", ch);
            paf_last = paf;
        }
		SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n",ch);
    }
    else
        SEND ("You are not affected by any spells.\r\n", ch);
	if (ch->bleeding < 1)
		SEND("\r\nYou aren't bleeding at all.\r\n",ch);
	if (ch->bleeding == 1)
		SEND("\r\n\r\nYou are very slightly {Rbleeding{x.\r\n",ch);
	if (ch->bleeding > 1 && ch->bleeding < 4)
		SEND("\r\n\r\nYou are dripping a bit of {Rblood{x.\r\n",ch);
	if (ch->bleeding > 3 && ch->bleeding < 6)
		SEND("\r\n\r\nYour wounds are {Rbleeding{x pretty steadily.\r\n",ch);
	if (ch->bleeding == 6 || ch->bleeding == 7)
		SEND ("\r\n\r\nHeavily flowing {RBLOOD{X pours from your wounds!\r\n",ch);
	if (ch->bleeding > 6)
		SEND("\r\n\r\nYou're gushing {RBLOOD{x everywhere!\r\n",ch);	
		
	int i = 0;	
	
	if (HAS_COOLDOWNS(ch))
	{
		SEND("\r\n\r\n{r-=-=-={xCooldowns{r=-=-=-{x\r\n\r\n",ch);
		
		for (i = 1; i < MAX_SKILL; i++)
		{
			if (ch->cooldowns[i] > 0 && !IS_NULLSTR(skill_table[i].name))
			{
				if (ch->cooldowns[i] >= 60)
				{
					if (ch->cooldowns[i] % 60 == 0)
						sprintf(buf, "{r%-26s{x: %2ld minute%s.\r\n", skill_table[i].name, (ch->cooldowns[i] / 60), ((ch->cooldowns[i] / 60) > 1 ? "s" : ""));
					else
						sprintf(buf, "{r%-26s{x: %2ld minute%s, %ld second%s.\r\n", skill_table[i].name, (ch->cooldowns[i] / 60), ((ch->cooldowns[i] / 60) > 1 ? "s" : ""), (ch->cooldowns[i] % 60), (ch->cooldowns[i] % 60 > 1 ? "s" : ""));
				}		
				else if (ch->cooldowns[i] >= 30)
					sprintf(buf, "{y%-26s{x: %2ld seconds.\r\n", skill_table[i].name, ch->cooldowns[i]);
				else
					sprintf(buf, "{g%-26s{x: %2ld second%s.\r\n", skill_table[i].name, ch->cooldowns[i], ch->cooldowns[i] > 1 ? "s" : "");
					
				SEND(buf, ch);			
			}
		}
	}
	else
		SEND("\r\nYou have no skills or spells on cooldown.\r\n",ch);
	
    return;
}

/* For time/dates */
char *const day_name[] = {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *const month_name[] = {
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time (CHAR_DATA * ch, char *argument)
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

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

	SEND ("{BTIME INFORMATION FOR ARAGOND:{x\r\n\r\n",ch);
		
    sprintf (buf,
             "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\r\n",
             (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
             time_info.hour >= 12 ? "pm" : "am",
             day_name[day % 7], day, suf, month_name[time_info.month]);
    SEND (buf, ch);
    sprintf (buf, "Aragond: the Chronicles was last rebooted on %s\r\nThe current system time is %s\r\n",
             str_boot_time, (char *) ctime (&current_time));

    SEND (buf, ch);
	
    return;
}


void do_gameinfo (CHAR_DATA *ch, char *argument)
{
	char buf[MSL];	
	
	SEND( "{CGAME INFORMATION FOR ARAGOND:{x\r\n\r\n",ch);
	
	if (double_exp)
	{
		sprintf (buf, "The double {Gexperience{x bonus is currently: {Gon{x for %d ticks.\r\n", global_exp);
		SEND (buf, ch);
	}
	else
		SEND ("The double experience bonus is currently: {Roff{x.\r\n", ch);
	
	if (double_gold)
	{
		sprintf (buf, "The double {Ygold{x bonus is currently:       {Gon{x for %d ticks.\r\n", global_gold);
		SEND (buf, ch);
	}
	else
		SEND ("The double gold bonus is currently:       {Roff{x.\r\n", ch);
	
	if (double_skill)
	{
		sprintf (buf, "The double {Dskill{x bonus is currently:      {Gon{x for %d ticks.\r\n", global_skill);
		SEND (buf, ch);
	}
	else
		SEND ("The double skill bonus is currently:      {Roff{x.\r\n", ch);
	
	
	sprintf (buf, "\r\nThere are currently %ld rooms in %d areas linked to the game world.\r\n", count_linked_rooms(), count_linked_areas());
    SEND (buf, ch);
	
	if (IS_IMMORTAL(ch))
	{
		SEND("\r\n{CMUD STATUS:{x\r\n", ch);
		sprintf(buf,"   Newlock is: %s\r\n", newlock ? "{Ron{x" : "{Goff{x");
		SEND(buf, ch);
		sprintf(buf,"   Wizlock is: %s\r\n", wizlock ? "{Ron{x" : "{Goff{x");
		SEND(buf, ch);
	}
	
	SEND ("\r\n\r\n",ch);
	
/*
	sprintf (buf, "The most players ever on was {B%d{x\r\n", game_data.most_players);
	SEND(buf, ch);
	
	sprintf (buf, "The most exploratory character so far is {B%s{x, with {B%ld{x explored rooms.\r\n", game_data.explored, game_data.explorer);
	SEND(buf, ch);
*/
	return;
}

void do_experience (CHAR_DATA *ch, char *argument)
{
	char buf[MSL];	
	
	if (IS_MCLASSED(ch))
	{
		sprintf (buf,
				 "You are level (%d / %d).\r\n",
				 ch->level, ch->level2);
		SEND (buf, ch);
	}
	else
	{
		sprintf (buf,
				 "You are level %d.\r\n",
				 ch->level);
		SEND (buf, ch);
	}
	
	if (IS_MCLASSED(ch))
	{
		sprintf (buf,
             "You have scored %ld / %ld experience points\r\n",
             ch->exp, ch->exp2);
		SEND (buf, ch);
		/* RT shows exp to level */
		if (!IS_NPC (ch) && ch->level < MCLASS_ONE && ch->level2 < MCLASS_TWO)
		{
			sprintf (buf,
                 "You need %ld / %ld exp to level.\r\n",
                 (exp_per_level (ch, FALSE)),
				 (exp_per_level (ch, TRUE)));
			SEND (buf, ch);
		}
		if (!IS_NPC (ch) && ch->level < MCLASS_ONE && ch->level2 >= MCLASS_TWO)
		{
			sprintf (buf,
                 "You need %ld exp to level.\r\n",
                 exp_per_level (ch, FALSE));
			SEND (buf, ch);
		}
		if (!IS_NPC (ch) && ch->level >= MCLASS_ONE && ch->level2 < MCLASS_TWO)
		{
			sprintf (buf,
                 "You need %ld exp to level.\r\n",                 
				 (exp_per_level (ch, TRUE)));
			SEND (buf, ch);
		}
	}
	else
	{
		sprintf (buf,
             "You have scored %ld experience points.\r\n",
             ch->exp);
		SEND (buf, ch);
		
		/* RT shows exp to level */
		if (!IS_NPC (ch) && ch->level < LEVEL_HERO)
		{
        sprintf (buf,
                 "You need %ld exp to level.\r\n",
                 exp_per_level (ch, FALSE));
        SEND (buf, ch);
		}
	}
	

	return;
}

void do_weather (CHAR_DATA * ch, char *argument)
{
    char buf[MSL];
	char windbuf[MSL];

    static char *const clouds[4] = 
	{
        "cloudless",
        "partly cloudy",
        "cloudy",
        "heavily cloudy"
    };

	static char *const precip[4] = 
	{		
		"raining",
		"sleeting",
		"snowing",
		"hailing"
	};
	
	static char *const amount[5] = 
	{
		"very lightly",
		"lightly",
		"moderately",
		"heavily",
		"very heavily"
	};
	
	static char *const wind[6] =
	{
		"There seems to be no wind blowing whatsoever",
		"There is a slight breeze",
		"There is a moderate wind",
		"The wind is blowing steadily",		
		"There are heavy winds passing through",
		"The wind is blowing incredibly hard"
	};
	
	static char *const temp[9] =
	{
		"feels very hot",
		"is fairly hot",
		"is rather warm",		
		"is slightly chilly",
		"is quite chilly",
		"is cold",
		"feels very cold",
		"is freezing",
		"is DANGEROUSLY cold"		
	};
	
    if (!IS_OUTSIDE (ch))
    {
        SEND ("You can't see the weather indoors.\r\n", ch);
        return;
    }

    
	sprintf (buf, "The sky is %s. ", clouds[ch->in_room->area->curr_cloud]);
	SEND(buf, ch);
	if (ch->in_room->area->curr_precip_amount > -1)
	{
		sprintf(buf, "It is currently %s %s.\r\n", amount[ch->in_room->area->curr_precip_amount], precip[ch->in_room->area->curr_precip]);
	}
	else
		sprintf(buf, "There appears to be no current precipitation.\r\n");
    SEND (buf, ch);
	
	if (ch->in_room->area->curr_wind == 0)
		sprintf(windbuf, "%s and the temperature ", wind[0]);
	else if (ch->in_room->area->curr_wind <= 10 && ch->in_room->area->curr_wind > 0)
		sprintf(windbuf, "%s and the temperature ", wind[1]);
	else if (ch->in_room->area->curr_wind > 10 && ch->in_room->area->curr_wind <= 20)
		sprintf(windbuf, "%s and the temperature ", wind[2]);
	else if (ch->in_room->area->curr_wind > 20 && ch->in_room->area->curr_wind <= 30)
		sprintf(windbuf, "%s and the temperature ", wind[3]);
	else if (ch->in_room->area->curr_wind > 30 && ch->in_room->area->curr_wind <= 50)
		sprintf(windbuf, "%s and the temperature ", wind[4]);
	else
		sprintf(windbuf, "%s and the temperature ", wind[5]);
	
	SEND(windbuf,ch);
	
	if (ch->in_room->area->curr_temp >= 95)
		sprintf(buf, "%s.\r\n", temp[0]);
	else if (ch->in_room->area->curr_temp < 95 && ch->in_room->area->curr_temp >= 85)
		sprintf(buf, "%s.\r\n", temp[1]);
	else if (ch->in_room->area->curr_temp < 85 && ch->in_room->area->curr_temp >= 70)
		sprintf(buf, "%s.\r\n", temp[2]);
	else if (ch->in_room->area->curr_temp < 70 && ch->in_room->area->curr_temp >= 60)
		sprintf(buf, "%s.\r\n", temp[3]);
	else if (ch->in_room->area->curr_temp < 60 && ch->in_room->area->curr_temp >= 45)
		sprintf(buf, "%s.\r\n", temp[4]);
	else if (ch->in_room->area->curr_temp < 45 && ch->in_room->area->curr_temp >= 35)
		sprintf(buf, "%s.\r\n", temp[5]);
	else if (ch->in_room->area->curr_temp < 35 && ch->in_room->area->curr_temp >= 25)
		sprintf(buf, "%s.\r\n", temp[6]);
	else if (ch->in_room->area->curr_temp < 25 && ch->in_room->area->curr_temp >= 10)
		sprintf(buf, "%s.\r\n", temp[7]);
	else
		sprintf(buf, "%s.\r\n", temp[8]);
	
	SEND(buf, ch);
	
	buf[0] = '\0';
	windbuf[0] = '\0';
	free_string(buf);
	free_string(windbuf);
	
    return;
}

void do_help (CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    int level;
	char buf2[MSL];
	int tracker = 0;
	int i = 0;

    output = new_buf ();

    if (argument[0] == '\0')
        argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
        level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

        if (level > get_trust (ch))
            continue;

        if (is_name (argall, pHelp->keyword))
        {
            /* add seperator if found */
            if (found)
                add_buf (output,
                         "\r\n============================================================\r\r\n\n");
            if (pHelp->level >= -1 && str_cmp (argall, "imotd"))
            {				
				tracker = 0;
				for (i = 0; i < strlen(pHelp->keyword) + 4; i++)
				{					
					if (tracker == 0)
					{
						add_buf (output, "{r-{x");
						tracker++;
					}
					else
					{
						add_buf (output, "{r={x");
						tracker--;
					}
				}	
				add_buf (output, "\r\n");
				add_buf (output, "{r|{y ");
                add_buf (output, pHelp->keyword);
                add_buf (output, " {r|{x\r\n");
				
				tracker = 0;
				for (i = 0; i < strlen(pHelp->keyword) + 4; i++)
				{					
					if (tracker == 0)
					{
						add_buf (output, "{r-{x");
						tracker++;
					}
					else
					{
						add_buf (output, "{r={x");
						tracker--;
					}
				}				
				add_buf (output, "\r\r\n\n");
            }

            /*
             * Strip leading '.' to allow initial blanks.
             */
            if (pHelp->text[0] == '.')
                add_buf (output, pHelp->text + 1);
            else
                add_buf (output, pHelp->text);
            found = TRUE;
			
			if (!str_cmp (argall, "imotd") || !str_cmp (argall, "motd"))
			{
				add_buf (output, "\r\nLast editted by: {r({x ");
				if (!pHelp->last_editted_by || IS_NULLSTR(pHelp->last_editted_by))
					add_buf (output, "Nobody");
				else
					add_buf (output, pHelp->last_editted_by);
				add_buf (output, " {r){x\r\n");
			}
			
            /* small hack :) */
            if (ch->desc != NULL && ch->desc->connected != CON_PLAYING
                && ch->desc->connected != CON_ACCOUNT)
                break;
        }
    }

    if (!found)
	{
        SEND ("No help on that word.\r\n", ch);
		SEND ("See "MXPTAG("Help 'summary'")"help summary"MXPTAG("/Help")" for more information.\r\n",ch);	
		/*
		 * Let's log unmet help requests so studious IMP's can improve their help files ;-)
		 * But to avoid idiots, we will check the length of the help request, and trim to
		 * a reasonable length (set it by redefining MAX_CMD_LEN in merc.h).  -- JR
		 */
		if (strlen(argall) > MAX_CMD_LEN)
		{
			argall[MAX_CMD_LEN - 1] = '\0';
			log_f ("Excessive command length: %s requested %s.", ch, argall);
			SEND ("That was rude!\r\n", ch);
		}
		/* OHELPS_FILE is the "orphaned helps" files. Defined in merc.h -- JR */
		else
		{
			append_file (ch, OHELPS_FILE, argall);
		}
	}
    else
	{
		QUEST_DATA *quest;
		for (quest = ch->quests; quest != NULL; quest = quest->next_quest)
		{
			if (quest->pIndexData && IS_SET(quest->pIndexData->type, D))
			{
				if (quest->pIndexData->help1 && !quest->has_read_helps[0] && !str_cmp(quest->pIndexData->help1, argall))
				{
					sprintf(buf2, "You have read one of the helpfiles required for the quest {g%s{x\r\n", quest->pIndexData->name);
					SEND(buf2,ch);
					quest->has_read_helps[0] = TRUE;
					if (quest_complete(ch, quest->pIndexData))
					{
						sprintf(buf2, "You've met the requirements for {g%s{x! Go turn it in!\r\n",quest->name);
						SEND(buf2,ch);
					}
					break;
				}
				if (quest->pIndexData->help2 && !quest->has_read_helps[1] && !str_cmp(quest->pIndexData->help2, argall))
				{
					sprintf(buf2, "You have read one of the helpfiles required for the quest {g%s{x\r\n", quest->pIndexData->name);
					SEND(buf2,ch);
					quest->has_read_helps[1] = TRUE;
					if (quest_complete(ch, quest->pIndexData))
					{
						sprintf(buf2, "You've met the requirements for {g%s{x! Go turn it in!\r\n",quest->name);
						SEND(buf2,ch);
					}
					break;
				}
				if (quest->pIndexData->help3 && !quest->has_read_helps[2] && !str_cmp(quest->pIndexData->help3, argall))
				{
					sprintf(buf2, "You have read one of the helpfiles required for the quest {g%s{x\r\n", quest->pIndexData->name);
					SEND(buf2,ch);
					quest->has_read_helps[2] = TRUE;
					if (quest_complete(ch, quest->pIndexData))
					{
						sprintf(buf2, "You've met the requirements for {g%s{x! Go turn it in!\r\n",quest->name);
						SEND(buf2,ch);
					}
					break;
				}
			}
		}
				
		page_to_char (buf_string (output), ch);		
	}

free_buf (output);

}


/* whois command */
void do_whois (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("You must provide a name.\r\n", ch);
        return;
    }

    output = new_buf ();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *wch;
        char const *ch_class, *ch_class2;

		ch_class2 = "";
		
        if (d->connected != CON_PLAYING || !can_see (ch, d->character))
            continue;

        wch = (d->original != NULL) ? d->original : d->character;

        if (!can_see (ch, wch))
            continue;

        if (!str_prefix (arg, wch->name))
        {
            found = TRUE;

			
            /* work out the printing */
			if (IS_MCLASSED(wch) && !IS_IMMORTAL(wch)){
			   ch_class = ch_class_table[wch->ch_class].who_short;
				ch_class2 =ch_class_table[wch->ch_class2].who_short;
			}			
			else
				ch_class =ch_class_table[wch->ch_class].who_name;
            switch (wch->level)
            {
                case MAX_LEVEL - 0:                   
                case MAX_LEVEL - 1:                   
                case MAX_LEVEL - 2:                   
                case MAX_LEVEL - 3:                   
                case MAX_LEVEL - 4:                    
                case MAX_LEVEL - 5:                    
                case MAX_LEVEL - 6:                    
                case MAX_LEVEL - 7:                   
                case MAX_LEVEL - 8:ch_class = "Immortal ";
					break;
            
            }

            /* a little formatting */
            if (IS_MCLASSED(wch) && !IS_IMMORTAL(wch)){
			sprintf (buf, "{r[{x%2d/%2d %10s %4s/%4s{r]{x %s%s%s%s%s%s%s%s%s{x{1\r\n",
                     wch->level,wch->level2,
                     wch->race <
                     MAX_PC_RACE ? pc_race_table[wch->
                                                 race].who_name : "     ",
                    ch_class,ch_class2, wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
                     wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
                     clan_table[wch->clan].who_name, IS_SET (wch->comm,
                                                             COMM_AFK) ?
                     "{b<{xAFK{b>{x " : "", IS_SET (wch->act,
                                            PLR_KILLER) ? "(KILLER) " : "",
                     IS_SET (wch->act, PLR_THIEF) ? "(THIEF) " : "",
					 IS_NPC (wch) ? "" : wch->pcdata->pretitle, 
                     wch->name, IS_NPC (wch) ? "" : wch->pcdata->title);
            add_buf (output, buf);
			}
			else {
			sprintf (buf, "{r[{x%5d %10s %9s{r]{x %s%s%s%s%s%s%s%s%s{x{1\r\n",
                     wch->level,
                     wch->race <
                     MAX_PC_RACE ? pc_race_table[wch->
                                                 race].who_name : "     ",
                    ch_class, wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
                     wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
                     clan_table[wch->clan].who_name, IS_SET (wch->comm,
                                                             COMM_AFK) ?
                     "{b<{xAFK{b>{x " : "", IS_SET (wch->act,
                                            PLR_KILLER) ? "(KILLER) " : "",
                     IS_SET (wch->act, PLR_THIEF) ? "(THIEF) " : "",
					 IS_NPC (wch) ? "" : wch->pcdata->pretitle, 
                     wch->name, IS_NPC (wch) ? "" : wch->pcdata->title);
            add_buf (output, buf);
			}
        }
    }

    if (!found)
    {
        SEND ("No one of that name is playing.\r\n", ch);
        return;
    }

    page_to_char (buf_string (output), ch);
    free_buf (output);
}





/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
 
void do_who (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];	
    char buf3[MAX_STRING_LENGTH];
    BUFFER *output;	
	BUFFER *imm_output;
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
	char *suf;
    int day;
	
	
    
    // Set default arguments.
    
    iLevelLower = 0;
    iLevelUpper = MAX_LEVEL;
    for (iClass = 0; iClass < MAX_CLASS; iClass++)
        rgfClass[iClass] = FALSE;
    for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
        rgfClan[iClan] = FALSE;

    //Parse arguments.
     
    nNumber = 0;
    for (;;)
    {
        char arg[MAX_STRING_LENGTH];

        argument = one_argument (argument, arg);
        if (arg[0] == '\0')
            break;

        if (is_number (arg))
        {
            switch (++nNumber)
            {
                case 1:
                    iLevelLower = atoi (arg);
                    break;
                case 2:
                    iLevelUpper = atoi (arg);
                    break;
                default:
                    SEND ("Only two level numbers allowed.\r\n", ch);
                    return;
            }
        }
        else
        {

            
            //Look forch_classes to turn on.
             
            if (!str_prefix (arg, "immortals"))
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass =ch_class_lookup (arg);
                if (iClass == -1)
                {
                    iRace = race_lookup (arg);

                    if (iRace == 0 || iRace >= MAX_PC_RACE)
                    {
                        if (!str_prefix (arg, "clan"))
                            fClan = TRUE;
                        else
                        {
                            iClan = clan_lookup (arg);
                            if (iClan)
                            {
                                fClanRestrict = TRUE;
                                rgfClan[iClan] = TRUE;
                            }
                            else
                            {
                                SEND
                                    ("That's not a valid race,ch_class, or clan.\r\n",
                                     ch);
                                return;
                            }
                        }
                    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }

    //Now show matching chars.
     
    nMatch = 0;
    buf[0] = '\0';
	buf2[0] = '\0';	
	buf3[0] = '\0';
    output = new_buf ();
	imm_output = new_buf ();
	bool immfound = FALSE;
	
	
	SEND("\r\n",ch);	
	
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *wch;
        char const *ch_class="", *ch_class2="";
        //Check for match against restrictions.
        //Don't use trust as that exposes trusted mortals.
        
        if (d->connected != CON_PLAYING || !can_see (ch, d->character))
            continue;

        wch = (d->original != NULL) ? d->original : d->character;

        if (!can_see (ch, wch))
            continue;

        if (wch->level < iLevelLower
            || wch->level > iLevelUpper
            || (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
            || (fClassRestrict && !rgfClass[wch->ch_class])
            || (fRaceRestrict && !rgfRace[wch->race])
            || (fClan && !is_clan (wch))
            || (fClanRestrict && !rgfClan[wch->clan]))
            continue;

        nMatch++;

        //Figure out what to print forch_class.
         
		if (IS_MCLASSED(wch))
		{
			ch_class =ch_class_table[wch->ch_class].who_short;
			ch_class2 =ch_class_table[wch->ch_class2].who_short;
		}
		else
			ch_class = ch_class_table[wch->ch_class].who_name;        
		
		if (total_levels(wch) == 41)
			ch_class = "  {WAvatar{x ";
		if (total_levels(wch) > 41)
			ch_class = "Immortal ";

		if (total_levels(wch) < LEVEL_IMMORTAL) 
		{				
				
			//New display Upro 1/19/2020 (WAY easier to maintain).
			if (IS_MCLASSED(wch))
				sprintf (buf, "{r[{x%2d/%2d %10s %4s/%4s{r]{x ", wch->level, wch->level2, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",ch_class,ch_class2);
			else
				sprintf (buf, "{r[{x%5d %10s %9s{r]{x ", wch->level, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",ch_class);
			add_buf(output,buf);
			
			if (wch->clan > CLAN_NONE)
			{
				if (wch==ch || is_same_clan (wch,ch) && wch->clan > CLAN_LONER || total_levels(ch) > 46 && wch->clan > CLAN_LONER)		
					sprintf (buf, "%s{r[{x%d{r]{x ", clan_table[wch->clan].who_name, wch->clan_rank);
				else 
					sprintf (buf, "%s", clan_table[wch->clan].who_name);
				add_buf(output,buf);
			}
			
			
			//Flags here:
			if (wch->desc && wch->desc->account && ACC_IS_SET(wch->desc->account->act, ACC_NEWBIE))
			{
				sprintf(buf, "{m[{xNew Player{m]{x ");
				add_buf(output, buf);
			}
			
			sprintf(buf, "%s%s%s", IS_SET (wch->comm, COMM_AFK) ? "{b<{xAFK{b>{x " : "", IS_SET (wch->act, PLR_KILLER) ? "(KILLER) " : "", IS_SET (wch->act, PLR_THIEF) ? "(THIEF) " : "");
			add_buf(output, buf);

			
			sprintf(buf, "%s%s%s%s{x\r\n", IS_NPC (wch) ? "" : wch->pcdata->pretitle, wch->name, IS_NULLSTR(wch->last_name) ? "" : wch->last_name, IS_NPC (wch) ? "" : wch->pcdata->title);
			add_buf (output, buf);		 
			
			//wch->society_rank > -1 ? wch->sex == SEX_FEMALE ? society_table[wch->society_rank].female_name : society_table[wch->society_rank].male_name : "", wch->society_rank > -1 ? " " : "",
					 
		}
		else
		{
			immfound = TRUE;
							
			//New display Upro 1/19/2020 (WAY easier to maintain).
			if (IS_MCLASSED(wch))
				sprintf (buf3, "{r[{x%2d/%2d %10s %4s/%4s{r[{x ", wch->level, wch->level2, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",ch_class,ch_class2);
			else
				sprintf (buf3, "{r[{x%5d %10s %9s{r]{x ", wch->level, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",ch_class);
			add_buf(imm_output,buf3);
			
			if (wch->clan > CLAN_NONE)
			{
				if (wch==ch || total_levels(ch) > 46 && wch->clan > CLAN_LONER)		
					sprintf (buf3, "%s{r[{x%d{r]{x ", clan_table[wch->clan].who_name, wch->clan_rank);
				else
					sprintf (buf3, "%s", clan_table[wch->clan].who_name);
				add_buf(imm_output,buf3);
			}
			
			
			//Flags here:
			sprintf(buf3, "%s%s%s", IS_SET (wch->comm, COMM_AFK) ? "{b<{xAFK{b>{x " : "", IS_SET (wch->act, PLR_KILLER) ? "(KILLER) " : "", IS_SET (wch->act, PLR_THIEF) ? "(THIEF) " : "");
			add_buf(imm_output, buf3);

			
			sprintf(buf3, "%s%s%s%s{x\r\n", IS_NPC (wch) ? "" : wch->pcdata->pretitle, wch->name, IS_NULLSTR(wch->last_name) ? "" : wch->last_name, IS_NPC (wch) ? "" : wch->pcdata->title);
			add_buf (imm_output, buf3);		 
			
			//wch->society_rank > -1 ? wch->sex == SEX_FEMALE ? society_table[wch->society_rank].female_name : society_table[wch->society_rank].male_name : "", wch->society_rank > -1 ? " " : "",
			
		}
	}

	if (immfound == TRUE)
	{
		SEND("{r--{R+ {xImmortals{R +{r--{x\r\n\r\n",ch);	
		page_to_char (buf_string (imm_output), ch);
		SEND("\r\n", ch);		
	}
	SEND("{r--{R+  {xMortals{R  +{r--{x\r\n\r\n", ch);
    add_buf (output, buf2);
    page_to_char (buf_string (output), ch);
	SEND("\r\n", ch);
	do_count(ch, "");
	SEND("\r\n", ch);
	//Display game time at bottom of Who list:
	buf[0] = '\0';
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
	
	sprintf (buf,
             "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\r\n",
             (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
             time_info.hour >= 12 ? "pm" : "am",
             day_name[day % 7], day, suf, month_name[time_info.month]);
    SEND (buf, ch);
	// End game time	
	
	/*if (number_percent() < 20)		
	{
		if (number_percent() < 50)
			SEND ("\r\nPlease vote for us at http://www.topmudsites.com/vote-aragond.html\r\n",ch);	
		else
			SEND ("\r\nPlease vote for us at http://www.mudconnect.com/cgi-bin/vote_rank.cgi?mud=Aragond:%20The%20Chronicles\r\n",ch);	
	}*/	
    free_buf (output);	
	free_buf (imm_output);
    return;
}

void do_count (CHAR_DATA * ch, char * argument)
{
    int count, max;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];    
	FILE *fp;
	
    count = 0;

    if ( IS_NPC(ch) || ch->desc == NULL )
    	return;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);
	
	//Game Data Upro 1/14/2020
	//max = game_data.most_players;	

	if ( max_on > max )
	{
	//	game_data.most_players = max_on;
	//	save_game_data;
	}

    if (max_on == count)
		if (count < 2)
		{
			sprintf (buf,
                 "There is %d character on, the most on this reboot.\r\n",
                 count);
		}
		else
		{
			sprintf (buf,
                 "There are %d characters on, the most on this reboot.\r\n",
                 count);
		}	
    else
		if (count < 2)
		{
			sprintf (buf,
                 "There is %d character on, the most on this reboot was %d.\r\n",
                 count, max_on);	
		}
		else
		{
			sprintf (buf,
                 "There are %d characters on, the most on this reboot was %d.\r\n",
                 count, max_on);	
		}
    SEND(buf, ch);
	sprintf (buf, "The most on ever was %d.\r\n", max);
	SEND(buf, ch);
}

void do_inventory (CHAR_DATA * ch, char *argument)
{
	char buf[MSL];
	
    SEND ("You are carrying:\r\n", ch);
    show_list_to_char (ch->carrying, ch, TRUE, TRUE, FALSE);
	
	sprintf (buf,
             "\r\nYou are carrying %s%d{x/%d items with weight %s%ld{x/%d pounds.\r\n",
             (ch->carry_number > (can_carry_n (ch) / 10) * 8) ? "{R" : "",				
			 ch->carry_number, 			 
			 can_carry_n (ch),
			 (get_carry_weight(ch) > (can_carry_w (ch) / 10) * 8) ? "{R" : "",
             get_carry_weight (ch) / 10, 
			 can_carry_w (ch) / 10);
    SEND (buf, ch);
	
    return;
}



void do_equipment (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    int iWear;
    bool found;	

    SEND ("You are using:\r\n", ch);
    found = FALSE;	
	
    for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {     
        if (iWear != WEAR_TAIL || (iWear == WEAR_TAIL && HAS_TAIL(ch)))
		{
			if (iWear != WEAR_LEGS2 || (iWear == WEAR_LEGS2 && IS_CENTAUR(ch)))
			{
				SEND (where_name[iWear], ch);
				if ((obj = get_eq_char (ch, iWear)) != NULL)
				{
					found = TRUE;
					if (can_see_obj (ch, obj))
					{
						if (!ch->blind)
							SEND (format_obj_to_char (obj, ch, TRUE, TRUE), ch);
						else
							SEND (blind_format_obj_to_char (obj, ch, TRUE, TRUE), ch);
						SEND ("\r\n", ch);
					}
					else
					{
						SEND ("something.\r\n", ch);
					}			
				}
				else		
					SEND("nothing.\r\n",ch);			
			}
		}
    }

    if (!found)	
        SEND ("Nothing.\r\n", ch);		
	
    return;
}

void do_compare (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    if (arg1[0] == '\0')
    {
        SEND ("Compare what to what?\r\n", ch);
        return;
    }

    if ((obj1 = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        SEND ("You do not have that item.\r\n", ch);
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
        {
            if (obj2->wear_loc != WEAR_NONE && can_see_obj (ch, obj2)
                && obj1->item_type == obj2->item_type
                && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
                break;
        }

        if (obj2 == NULL)
        {
            SEND ("You aren't wearing anything comparable.\r\n", ch);
            return;
        }
    }

    else if ((obj2 = get_obj_carry (ch, arg2, ch)) == NULL)
    {
        SEND ("You do not have that item.\r\n", ch);
        return;
    }

    msg = NULL;
    value1 = 0;
    value2 = 0;

    if (obj1 == obj2)
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if (obj1->item_type != obj2->item_type)
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch (obj1->item_type)
        {
            default:
                msg = "You can't compare $p and $P.";
                break;

            case ITEM_ARMOR:
                value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
                value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
                break;

            case ITEM_WEAPON:
                if (obj1->pIndexData->new_format)
                    value1 = (1 + obj1->value[2]) * obj1->value[1];
                else
                    value1 = obj1->value[1] + obj1->value[2];

                if (obj2->pIndexData->new_format)
                    value2 = (1 + obj2->value[2]) * obj2->value[1];
                else
                    value2 = obj2->value[1] + obj2->value[2];
                break;
        }
    }

    if (msg == NULL)
    {
        if (value1 == value2)
            msg = "$p and $P look about the same.";
        else if (value1 > value2)
            msg = "$p looks better than $P.";
        else
            msg = "$p looks worse than $P.";
    }

    act (msg, ch, obj1, obj2, TO_CHAR);
    return;
}



void do_credits (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "diku");
    return;
}



void do_where (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        sprintf(buf, "Players near you in %s:\r\n", ch->in_room->area->name);
		SEND (buf, ch);
        found = FALSE;
        for (d = descriptor_list; d; d = d->next)
        {
            if (d->connected == CON_PLAYING
                && (victim = d->character) != NULL && !IS_NPC (victim)
                && victim->in_room != NULL
                && !IS_SET (victim->in_room->room_flags, ROOM_NOWHERE)
                && (is_room_owner (ch, victim->in_room)
                    || !room_is_private (victim->in_room))
                && victim->in_room->area == ch->in_room->area
                && can_see (ch, victim))
            {
                found = TRUE;
                sprintf (buf, "%-18s %s\r\n",
                         victim->name, victim->in_room->name);
                SEND (buf, ch);
            }
        }
        if (!found)
            SEND ("None\r\n", ch);
    }
    else
    {
        found = FALSE;
        for (victim = char_list; victim != NULL; victim = victim->next)
        {
            if (victim->in_room != NULL
                && victim->in_room->area == ch->in_room->area
                && !IS_AFFECTED (victim, AFF_HIDE)
                && !IS_AFFECTED (victim, AFF_SNEAK)
                && can_see (ch, victim) && is_name (arg, victim->name))
            {
                found = TRUE;
                sprintf (buf, "%-28s %s\r\n",
                         PERS (victim, ch), victim->in_room->name);
                SEND (buf, ch);
                break;
            }
        }
        if (!found)
            act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
    }

    return;
}


void consider (CHAR_DATA *ch, CHAR_DATA *victim)
{
	int diff, chance;
	char *msg;
	
	diff = victim->level - ch->level;

    if (diff <= -10)
        msg = "You can kill $N naked and weaponless.";
    else if (diff <= -5)
        msg = "$N is no match for you.";
    else if (diff <= -2)
        msg = "$N looks like an easy kill.";
    else if (diff <= 1)
        msg = "The perfect match!";
    else if (diff <= 4)
        msg = "$N says 'Do you feel lucky, punk?'.";
    else if (diff <= 9)
        msg = "$N laughs at you mercilessly.";
    else
        msg = "Death will thank you for your gift.";

    act (msg, ch, NULL, victim, TO_CHAR);
	chance = number_percent();
	if (!IS_NPC(victim) && chance < (GET_INT(ch) + GET_WIS(ch)))
	{
		msg = "$N appears to be sizing you up.";
		act (msg, victim, NULL, ch, TO_CHAR);
	}
	
	return;
}

void do_consider (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Consider killing whom?\r\n", ch);
        return;
    }

	if (!str_cmp(arg, "all"))
	{
		for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
		{
			if (!is_safe(ch, victim) && ch != victim)
				consider(ch, victim);
		}
		
		return;
	}
	
    if ((victim = get_char_room ( ch, NULL, arg)) == NULL)
    {
        SEND ("They're not here.\r\n", ch);
        return;
    }

    if (is_safe (ch, victim))
    {
        SEND ("Don't even think about it.\r\n", ch);
        return;
    }
	
	consider(ch,victim);
    return;
}



void set_title (CHAR_DATA * ch, char *title)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        bug ("Set_title: NPC.", 0);
        return;
    }

    if (title[0] != '.' && title[0] != ',' && title[0] != '!'
        && title[0] != '?')
    {
        buf[0] = ' ';
        strcpy (buf + 1, title);
    }
    else
    {
        strcpy (buf, title);
    }

    free_string (ch->pcdata->title);
    ch->pcdata->title = str_dup (buf);
    return;
}



void do_title (CHAR_DATA * ch, char *argument)
{
    int i;

    if (IS_NPC (ch))
        return;

	/* Changed this around a bit to do some sanitization first   *
	 * before checking length of the title. Need to come up with *
	 * a centralized user input sanitization scheme. FIXME!      *
	 * JR -- 10/15/00                                            */

    if (strlen (argument) > 45)
        argument[45] = '\0';

	i = strlen(argument);
    if (argument[i-1] == '{' && argument[i-2] != '{')
		argument[i-1] = '\0';

    if (argument[0] == '\0')
    {
        SEND ("Change your title to what?\r\n", ch);
        return;
    }

	strcat(argument, "{x\0");

    smash_tilde (argument);
    set_title (ch, argument);
    SEND ("Ok.\r\n", ch);
}



void do_description (CHAR_DATA * ch, char *argument)
{
	if (!str_cmp(argument, "edit"))	
    {
        string_append (ch, &ch->description);
        return;
    }

	SEND ("Your description is:\r\n", ch);
    SEND (ch->description ? ch->description : "(None).\r\n", ch);
    return;
}



void do_report (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    sprintf (buf,
             "You say 'I have %d/%d hp %d/%d mana %d/%d mv %ld xp.'\r\n",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);

    SEND (buf, ch);

    sprintf (buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv %ld xp.'",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);

    act (buf, ch, NULL, NULL, TO_ROOM);

    return;
}



void do_practice (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if (IS_NPC (ch))
        return;

    if (argument[0] == '\0')
    {
        int col;

        col = 0;
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
				            
			if (sn == gsn_recall) //recall is no longer a skill but retains a cooldown.
				continue;
			
			if (ch->pcdata->learned[sn] < 1)
				continue;
			
			if (!IS_MCLASSED(ch) && ch->level < skill_table[sn].skill_level[ch->ch_class])            
				continue;			
				
			if (IS_MCLASSED(ch) && ch->level < skill_table[sn].skill_level[ch->ch_class] && ch->level2 < skill_table[sn].skill_level[ch->ch_class2])
				continue;
			
            sprintf (buf, "%-18s %3d%%  ",
                     skill_table[sn].name, ch->pcdata->learned[sn]);
            SEND (buf, ch);
            if (++col % 3 == 0)
                SEND ("\r\n", ch);
        }

        if (col % 3 != 0)
            SEND ("\r\n", ch);

        sprintf (buf, "You have {G%d{x practice sessions left.\r\n",
                 ch->practice);
        SEND (buf, ch);
    }
    else
    {
        CHAR_DATA *mob;
        int adept;

        if (!IS_AWAKE (ch))
        {
            SEND ("In your dreams, or what?\r\n", ch);
            return;
        }

        for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
        {
            if (IS_NPC (mob) && IS_SET (mob->act, ACT_PRACTICE))
                break;
        }

        if (mob == NULL)
        {
            SEND ("There's no one here to help you practice.\r\n", ch);
            return;
        }

        if (ch->practice <= 0)
        {
            SEND ("You have no practice sessions left.\r\n", ch);
            return;
        }
				
		if ((sn = find_spell (ch, argument)) < 0 || !KNOWS(ch,sn) || sn == gsn_recall)
		{
			SEND ("You can't practice that.\r\n", ch);
			return;
		}
			
		adept = IS_NPC (ch) ? 100 : ch_class_table[ch->ch_class].skill_adept;
		
        if (ch->pcdata->learned[sn] >= (skill_table[sn].spell_level > -1 ? adept + 10 : adept))
        {
            sprintf (buf, "You are already learned at %s.\r\n",
                     skill_table[sn].name);
            SEND (buf, ch);
        }
        else
        {
		//Decided to make it cost to practice things. Easy money sink. - Upro 11/24/2009			
			if (ch->level > 3 && ch->silver < 200 && ch->gold < 2)
			{		
				SEND("You cannot afford to practice at this time.\r\n",ch);
				return;			
			}
			deduct_cost ( ch, 200 );
			ch->guildpoints+=number_range(2,3);
            ch->practice--;
			int orig = ch->pcdata->learned[sn];            
			
			ch->pcdata->learned[sn] += int_app[GET_INT(ch)].learn / (skill_table[sn].guildrank > 0 ? skill_table[sn].guildrank : 1);							
			
            if (ch->pcdata->learned[sn] < (skill_table[sn].spell_level > -1 ? adept + 10 : adept))
            {
                sprintf(buf, "You practice {g%s{x from {g%d{x%% to {g%d{x%%.\r\n", skill_table[sn].name, orig, ch->pcdata->learned[sn]);
				SEND(buf, ch);				
				if (ch->practice > 0)				
				{
					sprintf (buf, "You have {g%d{x practice%s left.\r\n", ch->practice, ch->practice > 1 ? "s" : "");
					SEND(buf, ch);
				}
				else
					SEND( "You have no practices left.\r\n", ch);					
					
                act ("$n practices $T.",
                     ch, NULL, skill_table[sn].name, TO_ROOM);
            }
            else
            {
                ch->pcdata->learned[sn] = (skill_table[sn].spell_level > -1 ? adept + 10 : adept);
                act ("You are now well knowledged in {g$T{x.",
                     ch, NULL, skill_table[sn].name, TO_CHAR);
                act ("$n is now well knowledged in {g$T{x.",
                     ch, NULL, skill_table[sn].name, TO_ROOM);
            }
        }
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument (argument, arg);

    if (arg[0] == '\0')
        wimpy = ch->max_hit / 5;
    else
        wimpy = atoi (arg);

    if (wimpy < 0)
    {
        SEND ("Your courage exceeds your wisdom.\r\n", ch);
        return;
    }

    if (wimpy > ch->max_hit / 2)
    {
        SEND ("Such cowardice ill becomes you.\r\n", ch);
        return;
    }

    ch->wimpy = wimpy;
    sprintf (buf, "Wimpy set to %d hit points.\r\n", wimpy);
    SEND (buf, ch);
    return;
}

void do_telnetga (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->comm, COMM_TELNET_GA))
	{
		SEND ("Telnet GA removed.\r\n", ch);
		REMOVE_BIT (ch->comm, COMM_TELNET_GA);
	}
	else
	{
		SEND ("Telnet GA enabled.\r\n", ch);
		SET_BIT (ch->comm, COMM_TELNET_GA);
	}
}


// Upro - 2009

void do_knock(CHAR_DATA *ch, char *argument)
{
  /* Constructs taken from do_open().  */
  int door;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);

  if (arg[0] == '\0')
    {
      SEND("Knock on what?\r\n",ch);
      return;
    }

  if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;

      pexit = ch->in_room->exit[door];

      act( "$n knocks on the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      act( "You knock on the $d.", ch, NULL, pexit->keyword, TO_CHAR);

      /* Notify the other side.  */
      if (   ( to_room   = pexit->u1.to_room            ) != NULL
          && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
          && pexit_rev->u1.to_room == ch->in_room )
        {
          CHAR_DATA *rch;
          for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
            act( "You hear someone knocking.", rch, NULL, pexit_rev->keyword, TO_CHAR);
        }
    }

  return;
}

int colorstrlen(const char *argument)
{
  const char *str;
  int strlength;

  if (argument == NULL || argument[0] == '\0')
    return 0;

  strlength = 0;
  str = argument;

  while (*str != '\0')
  {
    if ( *str != '{' ) 
    {
      str++;
      strlength++;
      continue;
    }

    if (*(++str) == '{')
      strlength++;

    str++;
  }
  return strlength;
}

void do_finger(CHAR_DATA *ch, char *argument)
{
    FILE *fp;
    char strsave[MAX_INPUT_LENGTH];
    bool email_found = FALSE;
    bool real_found = FALSE;
    bool laston_found = FALSE;
    bool host_found = FALSE;	
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
	char *real = "";
    char *email = "";
    char *host = "";
    long laston;
    bool finished = FALSE;
    int d, h, m, s = 0;

    if(argument[0] == '\0' || argument[0] == '/' || argument[0] == '\\')
    {
		SEND("Finger who?\r\n", ch);
		return;
    }

    smash_tilde(argument);
    argument = one_argument(argument, arg);
    strcpy(arg2, argument);
    if(!str_cmp(arg, "email"))
    {
		if(arg2[0] == '\0')
		{
			sprintf(buf, "Your email address is currently listed as %s.\r\n",
			ch->email);
			SEND(buf, ch);
			return;
		}
		ch->email = strdup(arg2);
		sprintf(buf, "Your email address will now be displayed as %s.\r\n",
		ch->email);
		SEND(buf, ch);
		return;
    }

    if(!str_cmp(arg, "real"))
    {
		if(arg2[0] == '\0')
		{
			sprintf(buf, "Your real name is currently listed as %s.\r\n",
			ch->pcdata->real_name);
			SEND(buf, ch);
			return;
		}
		ch->pcdata->real_name = strdup(arg2);
		sprintf(buf, "Your real name will now appear as %s.\r\n",
		ch->pcdata->real_name);
		SEND(buf, ch);
		return;
    }	
	
    if(((victim = get_char_world(ch, arg)) != NULL) && (!IS_NPC(victim)))
    {
		
		if (victim->pcdata->email != NULL)
		{		
			sprintf(buf, "E-mail address: %35s\r\n", victim->pcdata->email);
			SEND(buf, ch);
		}
		if(victim->desc != NULL)
			sprintf(buf, "%s is currently {Gonline{x.\r\n", victim->name);
		else
			sprintf(buf, "%s is currently {Rlinkdead{x.\r\n", victim->name);
			
		SEND(buf, ch);
		

		if (IS_IMMORTAL(ch) && victim->timer > 3)
		{
			sprintf(buf, "Idle for: %d ticks\r\n", victim->timer);
			SEND(buf, ch);					
		}
	}
	else
	{
		sprintf(buf, "%s is currently {Roffline{x.\r\n", capitalize(arg));
		SEND(buf, ch);
	}
	
	sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(arg));
	if((fp = fopen(strsave, "r")) == NULL)
	{
		sprintf(buf, "No such player: %s.\r\n", capitalize(arg));
		SEND(buf, ch);
		return;
	}
	
		for(;;)
		{
			char *word;
			fread_to_eol(fp);
			word = feof(fp) ? "End" : fread_word(fp);
			switch(UPPER(word[0]))
			{
				case 'E':
					if(!str_cmp(word, "Email"))
					{
						email = fread_string(fp);
						email_found = TRUE;
					}
					break;
					if(!str_cmp(word, "End"))
					{
						finished = TRUE;
						break;
					}
					break;
				case 'H':
					if(!str_cmp(word, "Host"))
					{
						host = fread_string(fp);
						host_found = TRUE;
					}
					break;
				case 'R':
					if(!str_cmp(word, "Real"))
					{
						real = fread_string(fp);
						real_found = TRUE;
					}
					break;
	  
				case 'L':
					if(!str_cmp(word, "LogO"))
					{
						laston = fread_number(fp);
						laston_found = TRUE;
					}
					break;
				case '#':
					finished = TRUE;
					break;
				default:
					fread_to_eol(fp);
					break;
			}
			if((finished)||(real_found&&laston_found&&email_found))
				break; 
		}

		
		s = current_time - laston;
		d = s/86400;
		s-=d*86400;
		h = s/3600;
		s -= h*3600;
		m = s/60;
		s -= m*60;
		sprintf(buf,"Real Name: %16s\r\n", real_found ? real: "Not specified.");
		SEND(buf, ch);
		
		sprintf(buf, "E-mail address: %20s\r\n", email_found ? email : "Not specified.");
		SEND(buf, ch);
		
		sprintf(buf, "%s last logged on at %s\r\n", capitalize(arg), laston_found ? ctime(&laston) : "Not found.");
		SEND(buf, ch);
		printf_to_char(ch, "That was %d days, %d hours, %d minutes and %d seconds ago.\r\n", d, h, m, s);
	#ifdef BUILDER_PORT
		if(ch->level < MAX_LEVEL)
		{
			fclose(fp);
			return;
		}
	#endif
		if(IS_IMMORTAL(ch))
		{
			if(host_found)
				sprintf(buf, "%s last logged on from %s\r\n", capitalize(arg), host);
			else
				sprintf(buf, "No logon site found for %s.\r\n", capitalize(arg));
			SEND(buf, ch);
		}
		fclose(fp);
		return;
}

void do_identify(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH]; 
    CHAR_DATA *sage;
    OBJ_DATA *obj;

    one_argument(argument, arg);

    for ( sage = ch->in_room->people; sage != NULL; sage = sage->next_in_room)
    {
        if (!IS_NPC(sage)) continue;
        if( IS_SET(sage->act2,ACT2_SAGE) )
            break; 
    }

    if (sage == NULL)
    {
        SEND("And just who was going to identify the item?\r\n",ch);
        return;
    }

    if (arg[0] == '\0')
    {
		SEND("Identify which item?\r\n", ch);
		return;
    }

    if ( (obj = get_obj_carry(ch, arg, ch)) == NULL)
    {
		SEND("You aren't carrying that.\r\n", ch);
		return;
    }

    if ( ((ch->gold * 100) + ch->silver) < 500 && !IS_IMMORTAL(ch))
    {
		SEND("You don't have the 5 gold required.\r\n", ch);
		return;
    }
    // made true because he is npc
    spell_identify(0, 0, ch, (void *) obj, TAR_OBJ_INV);
	
	if (!IS_IMMORTAL(ch))
		deduct_cost(ch, 500);
	return;
}

void do_lore(CHAR_DATA *ch, char*argument)
{
	char arg[MAX_INPUT_LENGTH];     
    OBJ_DATA *obj;

	one_argument(argument, arg);
	
	if (!IS_KNOWN(ch, gsn_lore))
	{
		SEND("You have no idea what you're doing!\r\n",ch);
		return;		
	}
	
    if (arg[0] == '\0')
    {
		SEND("Identify which item?\r\n", ch);
		return;
    }

    if ( (obj = get_obj_carry(ch, arg, ch)) == NULL)
    {
		SEND("You aren't carrying that.\r\n", ch);
		return;
    }
	
	if (number_percent() < ch->pcdata->learned[gsn_lore])
	{	
		spell_identify(0, 0, ch, (void *) obj, TAR_OBJ_INV);
		check_improve (ch, gsn_lore, TRUE, 2);
	}
	else
	{
		SEND("You fail to make any sense of the object before you.\r\n",ch);
		check_improve (ch, gsn_lore, TRUE, 3);
	}
	return;
}

void do_material(CHAR_DATA *ch, char*argument)
{
	int i = 0;
	int counter = 0;
	char buf[MSL];
	
	SEND("Materials:\r\n",ch);
	for ( i = 0; i < MAX_MATERIAL; i++ )
	{
		if (ch->mats[i] > 0)
		{
			sprintf(buf,"{r[{x%10s %3d piece%1s{r ]{x %s",mat_table[i].material,ch->mats[i],
            ch->mats[i] != 1 ? "s" : "",++counter % 3 == 0 ? "\r\n" : "");
			SEND(buf, ch);			        
		}		
	}
	SEND("\r\n",ch);
	return;
}


/* does aliasing and other fun stuff */
void substitute_alias (DESCRIPTOR_DATA * d, char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH], prefix[MAX_INPUT_LENGTH],
        name[MAX_INPUT_LENGTH];
    char *point;
    int alias;

    ch = d->original ? d->original : d->character;

    /* check for prefix */
    if (ch->prefix[0] != '\0' && str_prefix ("prefix", argument))
    {
        if (strlen (ch->prefix) + strlen (argument) > MAX_INPUT_LENGTH - 2)
            SEND ("Line to long, prefix not processed.\r\n", ch);
        else
        {
            sprintf (prefix, "%s %s", ch->prefix, argument);
            argument = prefix;
        }
    }

    if (IS_NPC (ch) || ch->pcdata->alias[0] == NULL
        || !str_prefix ("alias", argument) || !str_prefix ("una", argument)
        || !str_prefix ("prefix", argument))
    {
        interpret (d->character, argument);
        return;
    }

    strcpy (buf, argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {                            /* go through the aliases */
        if (ch->pcdata->alias[alias] == NULL)
            break;

        if (!str_prefix (ch->pcdata->alias[alias], argument))
        {
            point = one_argument (argument, name);
            if (!strcmp (ch->pcdata->alias[alias], name))
            {
            	/* More Edwin inspired fixes. JR -- 10/15/00 */
				buf[0] = '\0';
				strcat(buf,ch->pcdata->alias_sub[alias]);
				if (point[0])
				{
					strcat(buf," ");
					strcat(buf,point);
				}

                if (strlen (buf) > MAX_INPUT_LENGTH - 1)
                {
                    SEND
                        ("Alias substitution too long. Truncated.\r\n", ch);
                    buf[MAX_INPUT_LENGTH - 1] = '\0';
                }
                break;
            }
        }
    }
    interpret (d->character, buf);
}

void do_alia (CHAR_DATA * ch, char *argument)
{
    SEND ("I'm sorry, alias must be entered in full.\r\n", ch);
    return;
}

void do_alias (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int pos;

    smash_tilde (argument);

    if (ch->desc == NULL)
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC (rch))
        return;

    argument = one_argument (argument, arg);


    if (arg[0] == '\0')
    {

        if (rch->pcdata->alias[0] == NULL)
        {
            SEND ("You have no aliases defined.\r\n", ch);
            return;
        }
        SEND ("Your current aliases are:\r\n", ch);

        for (pos = 0; pos < MAX_ALIAS; pos++)
        {
            if (rch->pcdata->alias[pos] == NULL
                || rch->pcdata->alias_sub[pos] == NULL)
                break;

            sprintf (buf, "    %s:  %s\r\n", rch->pcdata->alias[pos],
                     rch->pcdata->alias_sub[pos]);
            SEND (buf, ch);
        }
        return;
    }

    if (!str_prefix ("una", arg) || !str_cmp ("alias", arg))
    {
        SEND ("Sorry, that word is reserved.\r\n", ch);
        return;
    }

	/* More Edwin-inspired fixes. JR -- 10/15/00 */
	if (strchr(arg,' ')||strchr(arg,'"')||strchr(arg,'\''))
	{
		SEND("The word to be aliased should not contain a space, "
			"a tick or a double-quote.\r\n",ch);
		return;
	}

    if (argument[0] == '\0')
    {
        for (pos = 0; pos < MAX_ALIAS; pos++)
        {
            if (rch->pcdata->alias[pos] == NULL
                || rch->pcdata->alias_sub[pos] == NULL)
                break;

            if (!str_cmp (arg, rch->pcdata->alias[pos]))
            {
                sprintf (buf, "%s aliases to '%s'.\r\n",
                         rch->pcdata->alias[pos],
                         rch->pcdata->alias_sub[pos]);
                SEND (buf, ch);
                return;
            }
        }

        SEND ("That alias is not defined.\r\n", ch);
        return;
    }

    if (!str_prefix (argument, "delete") || !str_prefix (argument, "prefix"))
    {
        SEND ("That shall not be done!\r\n", ch);
        return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
        if (rch->pcdata->alias[pos] == NULL)
            break;

        if (!str_cmp (arg, rch->pcdata->alias[pos]))
        {                        /* redefine an alias */
            free_string (rch->pcdata->alias_sub[pos]);
            rch->pcdata->alias_sub[pos] = str_dup (argument);
            sprintf (buf, "%s is now realiased to '%s'.\r\n", arg, argument);
            SEND (buf, ch);
            return;
        }
    }

    if (pos >= MAX_ALIAS)
    {
        SEND ("Sorry, you have reached the alias limit.\r\n", ch);
        return;
    }

    /* make a new alias */
    rch->pcdata->alias[pos] = str_dup (arg);
    rch->pcdata->alias_sub[pos] = str_dup (argument);
    sprintf (buf, "%s is now aliased to '%s'.\r\n", arg, argument);
    SEND (buf, ch);
}


void do_unalias (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC (rch))
        return;

    argument = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        SEND ("Unalias what?\r\n", ch);
        return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
        if (rch->pcdata->alias[pos] == NULL)
            break;

        if (found)
        {
            rch->pcdata->alias[pos - 1] = rch->pcdata->alias[pos];
            rch->pcdata->alias_sub[pos - 1] = rch->pcdata->alias_sub[pos];
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            continue;
        }

        if (!strcmp (arg, rch->pcdata->alias[pos]))
        {
            SEND ("Alias removed.\r\n", ch);
            free_string (rch->pcdata->alias[pos]);
            free_string (rch->pcdata->alias_sub[pos]);
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            found = TRUE;
        }
    }

    if (!found)
        SEND ("No alias of that name to remove.\r\n", ch);
}


void save_bounties()
{
	FILE *fp;
	BOUNTY_DATA *bounty;
	bool found = FALSE;

	fclose(fpReserve);
	if ( ( fp = fopen( BOUNTY_FILE, "w" ) ) == NULL )
	{
		perror( BOUNTY_FILE );
	}

	for( bounty = bounty_list; bounty != NULL; bounty = bounty->next)
	{
		found = TRUE;
		fprintf(fp,"%-12s %d\n",bounty->name,bounty->amount);	
	}

	fclose(fp);
	fpReserve = fopen( NULL_FILE, "r" );

	if(!found)
		unlink(BOUNTY_FILE);

}

 

void load_bounties()

{

FILE *fp;

BOUNTY_DATA *blist;

 

if ( ( fp = fopen( BOUNTY_FILE, "r" ) ) == NULL )

return;

 

	blist = NULL;

	for( ; ; )
	{

		BOUNTY_DATA *bounty;

		if ( feof(fp) )
		{

			fclose(fp);
			return;

		}
	 

		bounty = new_bounty();
		bounty->name = str_dup(fread_word(fp));
		bounty->amount = fread_number(fp);
		fread_to_eol(fp);

		if (bounty_list == NULL)
			bounty_list = bounty;
		else
			blist->next = bounty;

		blist = bounty;

	}
}

 

bool is_bountied(CHAR_DATA *ch)

{

BOUNTY_DATA *bounty;

bool found = FALSE;

 

if(IS_NPC(ch))

return FALSE;

 

for(bounty = bounty_list; bounty != NULL; bounty = bounty->next)

{

if( !str_cmp(ch->name,bounty->name) )

found = TRUE;

}

 

return found;

}

 

void do_bounty(CHAR_DATA *ch, char *argument)

{

	FILE *fp;
	CHAR_DATA *vch;
	BOUNTY_DATA *bounty;
	BOUNTY_DATA *prev=NULL;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char strsave[MAX_STRING_LENGTH];
	char *word;
	bool found = FALSE;
	bool clanned = FALSE;
	int cash, amount, surcharge;

	 

	if( IS_NPC(ch) )
		return;

	/* Check for the bounty collector */

	for(vch = ch->in_room->people; vch ; vch = vch->next_in_room)
	{
		if(IS_NPC(vch) && IS_SET(vch->act, ACT2_BOUNTY))
			break;
	}
	
	if(vch == NULL)
	{
		SEND("You cannot do that here.\r\n",ch);
		return;
	}
	 
	/* Only clanned chars/pkillers can place bounties. */
	if( !is_clan(ch) )
	{
		do_say(vch,"You must be in a clan or lonered to place or collect a bounty.");
		return;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if(arg[0] == '\0')
	{			
		do_say(vch,"These are the bounties I will honor:\r\n");
		SEND(" Name         Bounty\r\n",ch);
		SEND(" ============ ===============\r\n",ch);
		for(bounty = bounty_list;bounty != NULL;bounty=bounty->next)
		{
			sprintf(buf," %-12s %d\r\n",bounty->name,bounty->amount);
			SEND(buf,ch);
		}
		return;
	}

	 

	if( arg2[0] == '\0' || (str_cmp(arg,"remove") && str_cmp(arg,"collect") && !is_number(arg2)) )
	{
		SEND("Syntax: bounty\r\n",ch);
		SEND(" bounty remove <player>\r\n",ch);
		SEND(" bounty collect <player>\r\n",ch);
		SEND(" bounty <player> <amount>\r\n",ch);
		return;
	}

	 

	/* For collecting bounties. */

	if( !str_cmp(arg,"collect") )
	{

	OBJ_DATA *obj;	 

	for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
	{
		if( !str_cmp(capitalize(arg2),capitalize(bounty->name)) )
			break;
	}

	if( bounty == NULL )
	{
		do_say(vch,"That individual is not in the books.");
		return;
	}

	for(obj = ch->carrying; obj != NULL; obj = obj->next)
	{
	if( is_name(arg2,obj->name) && (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_HEAD) )
		break;
	}

	 

		if( obj == NULL )
		{
			do_say(vch,"I'm afraid I'll need proof of the death.");
			return;
		}

		extract_obj(obj);
		do_say(vch,"Very well, here is your payment.");
		ch->bank_amt += bounty->amount;

	 

		/*

		* OK, we've got a valid bounty and they've
		* been paid. Time to remove the bounty.

		*/

		if(prev == NULL)
			bounty_list = bounty_list->next;
		else
			prev->next = bounty->next;

		free_bounty(bounty);
		save_bounties();
		return;
	}

	 

	/* For buying off bounties. */

	if( !str_cmp(arg,"remove") )
	{
		for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
		{
			if( !str_cmp(arg2,bounty->name) )
				break;
		}

		if( bounty == NULL )
		{
			do_say(vch,"That individual is not in the books.");
			return;
		}
		
		surcharge = BOUNTY_REMOVE_CHARGE;
		amount = bounty->amount+(bounty->amount*surcharge/100);

		if(ch->gold < amount)
		{
			do_say(vch,"You cannot afford to remove that bounty.");
			return;
		}
		
		amount -= ch->gold;		
		amount = 0;

		/*
		* OK, we've got a valid bounty and they can
		* pay. Time to remove the bounty.
		*/

		if(prev == NULL)
			bounty_list = bounty_list->next;
		else
			prev->next = bounty->next;
		
		free_bounty(bounty);
		do_say(vch,"The name has been removed from the books.");
		save_bounties();
		return;
	}

	 

	/*

	* This code allows you to bounty those
	* offline as well as those not visible.

	*/

	 

	fclose(fpReserve);

	sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(arg));

	if ( ( fp = fopen( strsave, "r" ) ) == NULL )

	{

	do_say(vch,"I am afraid there is no such person.");

	fpReserve = fopen( NULL_FILE, "r" );

	return;

	}

	 

	strcpy(arg,capitalize(arg));

	 

	while( !found )

	{

	char *clanName;

	 

	word = feof(fp) ? "End" : fread_word( fp );

	 

	if( !str_cmp(word, "Clan") )

	{

	found = TRUE;

	clanName = fread_string(fp);

	clanned = clan_lookup(clanName) ? TRUE : FALSE;

	free_string(clanName);

	}

	else if( !str_cmp(word, "End") )

	{

	found = TRUE;

	}

	else

	{

	fread_to_eol( fp );

	}

	}

	 

	fclose( fp );

	fpReserve = fopen( NULL_FILE, "r" );

	 

	/* Only clanned chars can be bountied */

	if( !clanned )
	{
		do_say(vch,"You cannot place bounties on non-clanned characters.");
		return;
	}

	 

	/* Minimum bounty, change as desired */

	if( (amount = atoi(arg2)) < MIN_BOUNTY)
	{
		sprintf(buf,"I do not accept bounties under %d gold.",MIN_BOUNTY);
		do_say(vch,buf);
		return;
	}

	/*
	* Does the bounty already exist?
	* Let's find out.
	*/

	for(bounty = bounty_list;bounty != NULL;bounty = bounty->next)
	{
		if( !str_cmp(bounty->name,arg) )
			break;
	} 

	/*

	* Either create a new bounty
	* or add to an existing one.
	* Notice the charge added to
	* place a bounty and the
	* additional one to add to a
	* bounty. Don't want to make
	* it too cheap. Again, change
	* as desired.

	*/
	if(bounty == NULL)
	{
		surcharge = BOUNTY_PLACE_CHARGE;
		if(cash < (amount+(amount*surcharge/100)))
		{
			do_say(vch,"You cannot afford to place that bounty.");
			return;
		}

		bounty = new_bounty();
		bounty->name = str_dup(arg);
		bounty->amount = amount;
		bounty->next = bounty_list;
		bounty_list = bounty;
		amount += amount*surcharge/100;

	}
	else
	{
		sprintf(buf,"There is a %d%% surcharge to add to an existing bounty.",BOUNTY_ADD_CHARGE);
		do_say(vch,buf);
		surcharge = BOUNTY_PLACE_CHARGE + BOUNTY_ADD_CHARGE;
		amount += amount*surcharge/100;
		if(ch->gold < amount)
		{
			do_say(vch,"You cannot afford to place that bounty.");
			return;
		}
		else
		{
			ch->gold -= amount;
			bounty->amount += amount;		
			amount = 0;
			do_say(vch,"Your bounty has been recorded in the books.");
		}	
	}
	save_bounties();
	
}




void do_mclass (CHAR_DATA *ch, char *argument)
{	
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *trainer;
	int spellChance = 0;
	
	buf[0] = '\0';    

    if (IS_NPC (ch))
        return;

    /* find a trainer */
    for (trainer = ch->in_room->people;
         trainer != NULL; trainer = trainer->next_in_room)
        if (IS_NPC (trainer) && IS_SET (trainer->act, ACT_GAIN))
            break;

    if (trainer == NULL || !can_see (ch, trainer))
    {
        SEND ("You can't do that here.\r\n", ch);
        return;
    }
	
	if ( ch->level < 10 )
	{
		SEND("You need more time at your primary class.\r\n",ch);
		return;
	}	
	if (ch->mClass == TRUE)
	{
		SEND("You have already chosen a secondary class.\r\n",ch);
		return;
	}
	if (ch->level > 25)
	{
		SEND("It's too late, you're already set in your ways.\r\n",ch);
		return;
	}
	else
	{
		if (argument == NULL || argument[0] == '\0')
		{
			SEND("You have the following multi classing options:\r\n",ch);
			strcat (buf, "{r[{x ");
			switch (ch->ch_class)
			{
			default:
				strcat (buf, "None");
				break;
			case FIGHTER:
				strcat (buf, "Cleric Wizard Thief Bard");
				break;
			case WIZARD:
				strcat (buf, "Cleric Fighter Thief");
				break;
			case THIEF:
				strcat (buf, "Cleric Wizard Fighter Bard");
				break;
			case CLERIC:
				strcat (buf, "Wizard Fighter Bard Ranger");				
				break;
			case RANGER:
				strcat (buf, "Cleric Wizard");
				break;		
			case BARD:
				strcat (buf, "Thief");
				break;
			}
			strcat (buf, " {r]{x\r\n");
			buf[0] = UPPER (buf[0]);
			SEND (buf, ch);
			return;
		}
		
		
		if (!strcmp (argument, "wizard"))
		{
			if (IS_WIZARD(ch))
			{
				SEND("You're already a wizard!\r\n",ch);
				return;
			}
			else if (IS_DRUID(ch) || IS_BARD(ch) || IS_PALADIN(ch) || IS_MONK(ch))
			{
				SEND("Your primary class won't allow that.\r\n",ch);
				return;
			}
			else if (!qualify_ch_class(ch, WIZARD))
			{
				SEND ("You don't qualify for that class.\r\n",ch);
				return;
			}
			else
			{
				SEND("You have chosen to become a wizard!\r\n\r\n",ch);
				ch->ch_class2 = WIZARD;
				group_add (ch,ch_class_table[WIZARD].base_group, FALSE);
				ch->mClass = TRUE;
				ch->level2 = 1;
				ch->exp2 = 0;								
				//combat spell.
				spellChance = number_range(1,100);
				if (spellChance < 25)
				{
					group_add (ch, "chill touch", FALSE);				
				}
				else if (spellChance < 50 && spellChance > 24)
				{
					group_add (ch, "magic missile", FALSE);
				}
				else if (spellChance < 75 && spellChance > 49)
				{
					group_add (ch, "burning hands", FALSE);
				}			
				else
					group_add (ch, "shocking grasp", FALSE);
				
				//utility spell.
				spellChance = number_range(1,100);
				if (spellChance < 25)
				{
					group_add (ch, "copy", FALSE);				
				}
				else if (spellChance < 50 && spellChance > 24)
				{
					group_add (ch, "audible glamor", FALSE);
				}
				else 
				{
					group_add (ch, "knock", FALSE);
				}				
				return;	
			}
		}
		
		else if (!strcmp (argument, "cleric"))
		{
			if (IS_CLERIC(ch))
			{
				SEND("You're already a cleric!\r\n",ch);
				return;
			}
			else if (!IS_WIZARD(ch) && !IS_FIGHTER(ch) && !IS_RANGER(ch))
			{
				SEND("Your primary class won't allow that.\r\n",ch);
				return;
			}
			else if (!qualify_ch_class(ch, CLERIC))
			{
				SEND ("You don't qualify for that class.\r\n",ch);
				return;
			}
			else
			{
				SEND("You have chosen to become a cleric!\r\n",ch);
				ch->ch_class2 = CLERIC;
				group_add (ch,ch_class_table[CLERIC].base_group, FALSE);
				ch->mClass = TRUE;
				ch->level2 = 1;
				ch->exp2 = 0;
				//give clerics a random utility spell.				
				spellChance = number_range(1,100);
				
				if (spellChance < 30)
					group_add(ch, "create spring", FALSE);
				else if (spellChance > 29 && spellChance < 70)
					group_add(ch, "create food", FALSE);
				else
					group_add(ch, "create water", FALSE);	
			
				return;
			}
		}
		
		else if (!strcmp (argument, "bard"))
		{
			if (IS_BARD(ch))
			{
				SEND("You're already a bard!\r\n",ch);
				return;
			}
			else if (IS_DRUID(ch) || IS_RANGER(ch) || IS_PALADIN(ch) || IS_MONK(ch) || IS_WIZARD(ch))
			{
				SEND("Your primary class won't allow that.\r\n",ch);
				return;
			}
			else if (!qualify_ch_class(ch, BARD))
			{
				SEND ("You don't qualify for class.\r\n",ch);
				return;
			}
			else
			{
				SEND("You have chosen to become a bard!\r\n",ch);
				ch->ch_class2 = BARD;
				group_add (ch,ch_class_table[BARD].base_group, FALSE);
				ch->mClass = TRUE;
				ch->level2 = 1;
				ch->exp2 = 0;
				return;
			}
		}
		
		else if (!strcmp (argument, "fighter"))
		{
			if (IS_FIGHTER(ch))
			{
				SEND("You're already a fighter!\r\n",ch);
				return;
			}
			else if (IS_DRUID(ch) || IS_RANGER(ch) || IS_PALADIN(ch) || IS_MONK(ch))
			{
				SEND("Your primary class won't allow that.\r\n",ch);
				return;
			}
			else if (!qualify_ch_class(ch, FIGHTER))
			{
				SEND ("You don't qualify for that class.\r\n",ch);
				return;
			}
			else
			{
				SEND("You have chosen to become a fighter!\r\n",ch);
				ch->ch_class2 = FIGHTER;
				group_add (ch,ch_class_table[FIGHTER].base_group, FALSE);
				ch->mClass = TRUE;
				ch->level2 = 1;
				ch->exp2 = 0;
				return;
			}
		}
		
		else if (!strcmp (argument, "thief"))
		{
			if (IS_THIEF(ch))
			{
				SEND("You're already a thief!\r\n",ch);
				return;
			}
			else if (!IS_FIGHTER(ch) && !IS_WIZARD(ch) && !IS_BARD(ch))
			{
				SEND("Your primary class won't allow that.\r\n",ch);
				return;
			}
			else if (!qualify_ch_class(ch, THIEF))
			{
				SEND ("You don't qualify for that class.\r\n",ch);
				return;
			}
			else
			{
				SEND("You have chosen to become a thief!\r\n",ch);
				ch->ch_class2 = THIEF;
				group_add (ch,ch_class_table[THIEF].base_group, FALSE);
				ch->mClass = TRUE;
				ch->level2 = 1;
				ch->exp2 = 0;
				return;
			}
		}
		
		else
		{
			SEND ("Invalid argument for multi classing.\r\n",ch);
			return;
		}
		
	}
}

void do_scan(CHAR_DATA *ch, char *argument)
{
   extern char *const dir_name[];
   char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *scan_room;
   EXIT_DATA *pExit;
   sh_int door, depth;

   if ( ch->move <= ch->level / 5)
   {
       SEND( "You are much too tired for that.\r\n", ch );
       return;
   }

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
      SEND("Looking around you see:\r\n", ch);
          scan_list(ch->in_room, ch, 0, -1);

      for (door=0;door<6;door++)
      {
         if ((pExit = ch->in_room->exit[door]) != NULL)
            scan_list(pExit->u1.to_room, ch, 1, door);
      }
      /*
	  if ( !is_affected( ch, gsn_farsight ))
      {
        WAIT_STATE( ch, PULSE_VIOLENCE * 2);
        ch->move -= number_range( 2, ch->level / 5);
      }
	  */
      return;
   }
   else if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
   else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))  door = 1;
   else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
   else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))  door = 3;
   else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" ))   door = 4;
   else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))  door = 5;
   else {
       SEND("Which way do you want to scan?\r\n", ch);
       return;
   }

   act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
   act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
   sprintf(buf, "Looking %s you see:\r\n", dir_name[door]);
                                                                                  
   scan_room = ch->in_room;

	for (depth = 1; depth < 4; depth++)
	{
		if ((pExit = scan_room->exit[door]) != NULL)
		{
			if (!IS_SET(pExit->rs_flags, EX_CLOSED)	)
			{
				scan_room = pExit->u1.to_room;
				scan_list(pExit->u1.to_room, ch, depth, door);
			}
			else
			{
				SEND("You see a door.\r\n",ch);
				return;
			}
		}
	}
   /*
   if ( !is_affected( ch, gsn_farsight ))
   {
        WAIT_STATE( ch, PULSE_VIOLENCE * 2);
        ch->move -= number_range( 2, ch->level / 5);
   }
   */
   return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth,
               sh_int door)
{
   CHAR_DATA *rch;

   if (scan_room == NULL) return;
   for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
   {
      if (rch == ch) continue;
      if (!IS_NPC(rch) && rch->invis_level > get_trust(ch)) continue;
      if (can_see(ch, rch)) scan_char(rch, ch, depth, door);
   }
   return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
   extern char *const dir_name[];
   extern char *const scan_distance[];
   char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

   buf[0] = '\0';

   strcat(buf, PERS(victim, ch));
   //if(is_affected(ch,gsn_farsight)){
       switch ( victim->position )
       {
       case POS_DEAD:     strcat( buf, ", lying dead" );              break;
       case POS_MORTAL:   strcat( buf, ", nearly dead" );   break;
       case POS_INCAP:    strcat( buf, ", incapacitated" );      break;
       case POS_STUNNED:  strcat( buf, ", lying stunned" ); break;
       case POS_SLEEPING: strcat( buf, ", sleeping" ); break;
       case POS_RESTING:  strcat( buf, ", resting" ); break;
       case POS_SITTING:  strcat( buf, ", sitting" ); break;
       case POS_STANDING: strcat( buf, ", standing" ); break;
       case POS_FIGHTING: strcat( buf, ", engaged in combat" ); break;
       }
   //}
   strcat(buf, ", ");
   sprintf(buf2, scan_distance[depth], dir_name[door]);
   strcat(buf, buf2);
   strcat(buf, "\r\n");

   SEND(buf, ch);
   if (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
   {
       act("$N doesn't look very happy...",ch, NULL, victim, TO_CHAR);
   }
   //affect_strip( ch, gsn_farsight );
   return;
}


void do_gather (CHAR_DATA *ch, char *argument)
{
	if (argument[0] != '\0')
	{
		SEND("This command does not require arguments.\r\n",ch);
		return;
	}
	
	if (!IS_KNOWN(ch, gsn_channel))
	{
		SEND("You have no idea what you're doing!\r\n",ch);
		return;		
	}
	
	if (ch->fighting)
	{
		SEND("You can't concentrate enough!\r\n",ch);
		return;
	}
	
	if (ch->ki > total_levels(ch))
	{
		SEND("You cannot possibly channel more ki at this time.\r\n",ch);
		return;
	}
	
	if (number_percent() < get_skill(ch, gsn_channel))
	{
		SEND("You focus intently as your body gathers and stores ki energy.\r\n",ch);
		ch->ki += number_range(1,5);
	
		if (ch->ki > ch->level)
		{
			ch->ki = ch->level;
		}
	
		check_improve (ch, gsn_channel, TRUE, 4);
		return;
	}
	else
	{
		SEND("You fail to gather any ki.\r\n",ch);
		check_improve (ch, gsn_channel, TRUE, 6);
		return;
	}
	
	return;
}



void do_entrance (CHAR_DATA *ch, char *argument)
{

	if (argument[0] == '\0')
	{
		SEND("Syntax: entrance <walk_desc>\r\n",ch);
		return;
	}
	else
	{	free_string (ch->walk_desc);
		ch->walk_desc = str_dup (argument);
		SEND("Walk description set!\r\n",ch);
		return;
	}
	return;
}

void do_last_name (CHAR_DATA *ch, char *argument)
{
	if (argument[0] == '\0')
	{
		SEND("Syntax: lastname <surname/last name>\r\n",ch);
		return;
	}
	else
	{	free_string (ch->last_name);
		ch->last_name = str_dup (argument);
		SEND("Last name set!\r\n",ch);
		return;
	}
	return;
}

char *get_rank(int rank)
{		
	switch (rank)
	{
		default:
			return "outcast";
			
		case RANK_OUTCAST:
			return "outcast";
			
		case RANK_PEASANT:
			return "peasant";			
			
		case RANK_OFFICIAL:
			return "official";
			
		case RANK_LORD:
			return "lord";
			
		case RANK_DUKE:
			return "duke";
			
		case RANK_KING:
			return "king";
			
	}	
}

void do_faction (CHAR_DATA *ch, char *argument)
{
	int i = 0;
	char buf[MSL];	
	char factions[MSL];
	char arg[MSL];
	char arg2[MSL];
	CHAR_DATA *victim;
	
	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);
	
	if (arg[0] == '\0' && arg2[0] == '\0')
	{
		// Show faction rep, rank, and society status/faction loyalty.
		sprintf(buf, "{g%-14s      {g%-5s    {g%-8s\r\n","Faction{x:","Rep{x:","Rank{x:");
		SEND (buf,ch);
		SEND ("{D----------------------------------------{x\r\n",ch);	
		for (i = 0; i < MAX_FACTION; i++)
		{
			sprintf(factions, "%-14s      %-5d    %-8s\r\n", faction_table[i].name, ch->faction_rep[i], get_rank(ch->faction_rank[i]));
			SEND(factions, ch);
		}
		return;
	}
	else
	{
		switch (arg[0])
		{
			case 'a':
			case 'A':
				if (ch->faction_invite < 0)
				{
					SEND ("You haven't been invited to join any faction yet.\r\n",ch);
					return;
				}
				else
				{
					if (ch->faction > -1)
					{
						SEND ("You already belong to a faction.... leave it first.\r\n",ch);
						return;
					}
					sprintf (buf, "You have accepted the faction invitation for %s.\r\n", faction_table[ch->faction_invite].name);
					SEND (buf, ch);
					ch->faction = ch->faction_invite;
					ch->faction_invite = -1;					
					return;
				}
				break;
			case 'd':
			case 'D':
				if (ch->faction_invite < 0)
				{
					SEND ("You haven't been invited to join any faction yet.\r\n",ch);
					return;
				}
				else
				{
					sprintf (buf, "You have declined a faction invitation for %s.\r\n", faction_table[ch->faction_invite].name);
					SEND (buf, ch);
					ch->faction_invite = -1;
					return;
				}
				break;
			case 'I':
			case 'i':
				if (ch->faction < 0)
				{
					SEND ("You don't even belong to a faction yet!\r\n",ch);
					return;
				}
				if (arg2[0] == '\0')
				{
					SEND ("Invite whom to your faction?\r\n",ch);
					return;
				}
				if ((victim = get_char_world( NULL, arg2 )) == NULL)
				{
					SEND ("They aren't around.\r\n",ch);
					return;
				}
				if (victim->faction > -1)
				{
					SEND ("They are already part of some other faction.\r\n",ch);
					return;
				}
				if (victim->faction_invite > -1)
				{
					SEND ("They have already been invited to another faction.\r\n",ch);
				}
				
				sprintf (buf, "You have invited %s to join the %s faction.\r\n", victim->name, faction_table[ch->faction].name);
				SEND (buf, ch);
				sprintf (buf, "%s has invited you to join the %s faction.\r\n", ch->name, faction_table[ch->faction].name);
				SEND (buf, victim);
				victim->faction_invite = ch->faction;
				break;
			case 'l':
			case 'L':
				if (ch->faction < 0)
				{
					SEND ("You need to be in a faction to leave it.\r\n",ch);
					return;
				}
				else
				{					
					sprintf (buf, "You leave the %s faction.\r\n", faction_table[ch->faction].name);
					SEND (buf, ch);
					ch->faction = -1;					
					return;
				}
				break;		
			case 'p':
			case 'P':
				if (ch->faction < 0)
				{
					SEND ("You need to be in a faction to promote people in it.\r\n",ch);
					return;
				}
				else
				{					
					if (arg2[0] == '\0')
					{
						SEND ("Promote whom in your faction?\r\n",ch);
						return;
					}
					if ((victim = get_char_world( NULL, arg2 )) == NULL)
					{
						SEND ("They aren't around.\r\n",ch);
						return;
					}
					if (victim->faction != ch->faction)
					{
						SEND ("They are part of some other faction.\r\n",ch);
						return;
					}
					if (victim == ch && !IS_IMMORTAL(ch))
					{
						SEND ("You can't promote yourself.\r\n",ch);
						return;
					}
					if (victim->society_rank >= 21 && IS_IMMORTAL(ch)) //arch duke
					{
						if (victim->society_rank == MAX_SOCIETY_RANKS) //Still don't allow imms to crash us.
						{
							SEND ("They can't be promoted any further.\r\n",ch);
							return;
						}
					}
					if (victim->society_rank <= ch->society_rank - 2 || victim == ch)
					{
						sprintf (buf, "You promote %s from %s to %s.\r\n",victim->name, society_table[victim->society_rank].male_name, society_table[victim->society_rank+1].male_name );
						victim->society_rank++;
						SEND (buf, ch);
						SEND ("You have been promoted in society!\r\n",victim);
						return;
					}
					else
					{
						SEND ("You don't have authority to do that.\r\n",ch);
						return;
					}
					
				}
				break;		
			default:
				break;
		}
	}
	
}

void do_history (CHAR_DATA *ch, char *argument)
{	
	AREA_DATA *pArea;
	char buf[MSL];
	
	if (argument[0] != '\0')
	{
	
		for (pArea = area_first; pArea; pArea = pArea->next)
		{
			if ( pArea && !str_cmp(pArea->name, argument) )
			{
				if (!pArea->history)
				{
					SEND("There's no history information set for this area.\r\n",ch);
					break;
				}
				SEND( "\r\n", ch);
				sprintf(buf, "History for {g%s{x\r\n\r\n",pArea->name);
				SEND(buf, ch);
				SEND( pArea->history, ch );
				SEND( "\r\n", ch);
				sprintf(buf, "Faction: {g[{x%s{g]{x\r\n", pArea->faction);
				SEND(buf, ch);
				
				break;
			}
		}
	}
	else
	{
		SEND("Grab history info for which area?\r\n",ch);
		return;
	}
	return;
}


void show_quest_log_to_char (QUEST_DATA * list, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    
    QUEST_DATA *quest;
    int nShow;
    int count;	
    

    if (ch->desc == NULL)
        return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf ();

    count = 0;
    for (quest = list; quest != NULL; quest = quest->next_quest)
        count++;
		
    prgpstrShow = alloc_mem (count * sizeof (char *));
    prgnShow = alloc_mem (count * sizeof (int));
    nShow = 0;

   
    for (quest = list; quest != NULL; quest = quest->next_quest)
    {
		sprintf(buf, "{g%s{x\r\n------------------------\r\n", quest->name);
		SEND(buf, ch);
		sprintf(buf, "Completed {g[{x%s{g]{x\r\n", quest->completed ? "yes" : "no");
		SEND(buf, ch);		
		if (quest->completed == FALSE)
		{
			if (quest->time_limit > 0)
			{
				sprintf(buf, "You have {g%d{x ticks left to complete this quest.\r\n",quest->time_limit);
				SEND(buf, ch);
			}
			else
			{
				SEND("There is no time limit for this quest.\r\n",ch);
			}
		}
		SEND("\r\nDescription:\r\n",ch);
		sprintf(buf, "{D%s{x\r\n\r\n",quest->pIndexData->desc);
		SEND(buf, ch);
	}

    return;
}


void do_census (CHAR_DATA *ch, char *argument)
{
	
	
	return;
}


int total_expertise(CHAR_DATA *ch)
{
	int exp = 0;
	int i = 0;
	char buf[MAX_STRING_LENGTH];
	
	
	while (i < MAX_WEAPON)
	{
		if (ch->expertise[i] > 0)
			exp++;
		else					
		    sprintf(buf,"Loading Game Data Structure. (weapon expertise)");
			log_string(buf);				
		i++;
	}
	return exp;
}

int fighter_levels(CHAR_DATA *ch)
{
	if (!IS_MCLASSED(ch) && IS_FIGHTER(ch))
	{
		return total_levels(ch);		
	}
	else if (IS_MCLASSED(ch) && ch->ch_class == ch_class_FIGHTER)
	{
		return ch->level;
	}
	else if (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_FIGHTER)
	{
		return ch->level2;	
	}
	else
		return 0;
}

int thief_levels(CHAR_DATA *ch)
{
	if (!IS_MCLASSED(ch) && IS_THIEF(ch))
		return total_levels(ch);
		
	else if (IS_MCLASSED(ch) && ch->ch_class == ch_class_THIEF)
		return ch->level;
		
	else if (IS_MCLASSED(ch) && ch->ch_class2 == ch_class_THIEF)
		return ch->level2;
	
	else
		return 0;
}

int expertise_allowed(CHAR_DATA * ch)
{
	int exp = 1;	
	
	exp += fighter_levels(ch) / 5;
	
	return exp;
}


void do_expertise (CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	char buf2[MSL];
	char buf3[MSL];
	int i = 0;
	
	
	if (!IS_FIGHTER(ch) && !IS_IMMORTAL(ch))
	{
		SEND("You have no expertise in weaponry.\r\n",ch);
		return;
	}
	
	if (argument[0] == '\0')
	{
		SEND("                    {D-=-=- {rWeapon Expertise {D-=-=-{x\r\n",ch);
		i = 0;
		while(wpnSpecTable[++i].name != NULL)
		{
			switch (ch->expertise[i])
			{
				default:
				case 0:
					sprintf(buf2, "   ");
					break;
				case 1:
					sprintf(buf2, "{D*{x  ");
					break;
				case 2:
					sprintf(buf2, "** ");
					break;
				case 3:
					sprintf(buf2, "{W***{x");
					break;
			}
			sprintf(buf,"%15s {D({x%3s{D){x   %s", wpnSpecTable[i].name, buf2, i % 3 == 0 ? "\r\n" : "");
			SEND(buf, ch);
		}
		
		return;
	}
	else
	{
		while(wpnSpecTable[++i].name != NULL)
		{
			if (!str_cmp(argument, wpnSpecTable[i].name))
			{				
				if (expertise_allowed(ch) > total_expertise(ch))
				{
					sprintf(buf3, "You choose to put expertise into the %s\r\n", wpnSpecTable[i].name);
					SEND(buf3,ch);
					ch->expertise[i]++;
					return;
				}
				else
				{
					SEND("You aren't ready to put more expertise into your weapon skills.\r\n",ch);
					return;
				}
			}
		}
		
		SEND("Invalid weapon type to expertise in.\r\n",ch);
		return;
	}
	
	
}


void do_achievements (CHAR_DATA *ch, char * argument)
{
	char buf[MSL];	
	
	sprintf(buf, "Achievements for {r%s{x:\r\n", ch->name);
	SEND(buf, ch);
	SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n\r\n",ch);
	
	if (ch->rank_apprentice)
		SEND ("Guild Rank: Apprentice\r\n",ch);
	else
		SEND ("{DGuild Rank: Apprentice{x\r\n",ch);
	if (ch->rank_member)
		SEND ("Guild Rank: Member\r\n",ch);
	else
		SEND ("{DGuild Rank: Member{x\r\n",ch);
	if (ch->rank_officer)
		SEND ("Guild Rank: Officer\r\n",ch);
	else
		SEND ("{DGuild Rank: Officer{x\r\n",ch);
	if (ch->rank_master)
		SEND ("Guild Rank: Master\r\n",ch);
	else
		SEND ("{DGuild Rank: Master{x\r\n",ch);
		
	SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n\r\n",ch);
		
	if (ch->mob_1000)
		SEND ("Slay 1000 Monsters\r\n",ch);
	else
		SEND ("{DSlay 1000 Monsters{x\r\n",ch);
	if (ch->mob_5000)
		SEND ("Slay 5000 Monsters\r\n",ch);
	else
		SEND ("{DSlay 5000 Monsters\r\n{x",ch);
	if (ch->mob_10000)
		SEND ("Slay 10000 Monsters\r\n",ch);
	else
		SEND ("{DSlay 10000 Monsters\r\n{x",ch);
	if (ch->mob_30000)
		SEND ("Slay 30000 Monsters\r\n",ch);
	else
		SEND ("{DSlay 30000 Monsters{x\r\n",ch);
		
	SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n\r\n",ch);
	
	if (ch->player_1)
		SEND ("Stalker:       Kill Another Player\r\n",ch);
	else
		SEND ("{DStalker:       Kill Another Player\r\n{x",ch);
	if (ch->player_10)
		SEND ("Killer:        Kill Ten Other Players\r\n",ch);
	else
		SEND ("{DKiller:        Kill Ten Other Players{x\r\n",ch);
	if (ch->player_50)
		SEND ("Murderer:      Kill Fifty Other Players\r\n",ch);
	else
		SEND ("{DMurderer:      Kill Fifty Other Players\r\n{x",ch);
	if (ch->player_250)
		SEND ("Predator:      Kill 250 Other Players\r\n",ch);
	else
		SEND ("{DPredator:      Kill 250 Other Players\r\n{x",ch);
	if (ch->player_500)
		SEND ("Mass Murderer: Kill 500 Other Players\r\n",ch);
	else
		SEND ("{DMass Murderer: Kill 500 Other Players\r\n{x",ch);
	
		
	SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n\r\n",ch);

	if (ch->gold_1000)
		SEND ("Gather 1000 Gold On Your Person\r\n",ch);
	else
		SEND ("{DGather 1000 Gold On Your Person\r\n{x",ch);
	if (ch->bank_1000)
		SEND ("Gather 1000 Gold In Your Bank\r\n",ch);
	else
		SEND ("{DGather 1000 Gold In Your Bank\r\n{x",ch);
	if (ch->bank_10000)
		SEND ("Gather 10000 Gold In Your Bank\r\n",ch);
	else
		SEND ("{DGather 10000 Gold In Your Bank\r\n{x",ch);
	if (ch->bank_50000)
		SEND ("Gather 50000 Gold In Your Bank\r\n",ch);
	else
		SEND ("{DGather 50000 Gold In Your Bank\r\n{x",ch);
		
	SEND ("{r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\r\n\r\n",ch);
	return;
}

void do_peek ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1 [MAX_INPUT_LENGTH];
    int percent;
    char buf[MSL];

    argument = one_argument( argument, arg1 );

    victim = get_char_room( ch, NULL, arg1 ); 
    if (victim == ch)
    {
	SEND("You can't peek yourself.\r\n", ch );
	return;
    } 

    if ( (victim == NULL))
    {
		SEND( "They aren't here.\r\n\r\n", ch );
		return;
    }

   if ( !IS_IMMORTAL(ch) && !IS_NPC(victim) && total_levels(ch) > (total_levels(victim) + 8) )
   {
        SEND("Pick on someone your own size!\r\n", ch);
        return;
   }

   if ( !IS_IMMORTAL(ch) && !IS_NPC(victim) && total_levels(victim) > (total_levels(ch) + 8) )
   {
       SEND("You think you'd like to keep your digits attached for today.\r\n", ch);
       return;
   }

    percent = number_percent() + (3 * ( total_levels(victim) - total_levels(ch)) / 2);

    if ( IS_IMMORTAL(ch))
		percent = 0;

    if ( get_trust(victim) > get_trust(ch) && IS_IMMORTAL(victim))
    {
		SEND("Foolish thing to do...\r\n",ch);
		return;
    }

    if ( victim != ch && percent < get_skill(ch,gsn_peek))
    {
		SEND( "\n\rYou peek at the inventory:\r\n", ch );
		check_improve(ch,gsn_peek,TRUE,4);
		show_list_to_char( victim->carrying, ch, TRUE, TRUE, FALSE);
    }
    else if ( victim != ch && percent >= get_skill(ch,gsn_peek))
    {
		WAIT_STATE(ch,skill_table[gsn_peek].beats);
		check_improve(ch,gsn_peek,FALSE,4);	
		if ( IS_AWAKE( victim ) )
		{
			sprintf( buf, "%s is trying to see what you carry.\r\n", ch->name );
			SEND( buf, victim );
		}
		SEND( "Oops, better start running!\r\n", ch );
    }
    return;
}

void do_mercy( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch))
      return;
    
    if (ch->mercy)
    {
      SEND("You no longer show mercy to your opponents.\r\n",ch);
      ch->mercy = FALSE;
    }
    else
    {
      SEND("You show mercy to your opponents.\r\n",ch);
      ch->mercy = TRUE;
    }
}

void do_vuln (CHAR_DATA *ch, char *argument )
{
	int dam = 1;
	char buf[MSL];	
	char *dam_name;
	int res_percent = 0;
	
	if (IS_NPC(ch))
      return;	
  
	sprintf(buf, "--+ Resistances/Vulnerabilities for %s +--\r\n\r\n", ch->name);
	SEND(buf, ch);	
	SEND ("Note: Negatives indicate a resistance to something. -100 is immunity.\r\n",ch);
	
	buf[0] = '\0';
	while (dam < MAX_DAM_TYPE - 1)
	{			
		switch (dam)
		{
			case (DAM_NONE):
				dam_name = "None     ";
				break;
			case (DAM_BASH):
				dam_name = "Bash     ";
				break;
			case (DAM_PIERCE):
				dam_name = "Pierce   ";
				break;
			case (DAM_SLASH):
				dam_name = "Slash    ";
				break;
			case (DAM_FIRE):
				dam_name = "Fire     ";
				break;
			case (DAM_COLD):
				dam_name = "Cold     ";
				break;
			case (DAM_LIGHTNING):
				dam_name = "Lightning";
				break;
			case (DAM_ACID):
				dam_name = "Acid     ";
				break;
			case (DAM_POISON):
				dam_name = "Poison   ";
				break;
			case (DAM_NEGATIVE):
				dam_name = "Negative ";
				break;
			case (DAM_HOLY):
				dam_name = "Holy     ";
				break;
			case (DAM_ENERGY):
				dam_name = "Energy   ";
				break;
			case (DAM_MENTAL):
				dam_name = "Mental   ";
				break;
			case (DAM_DISEASE):
				dam_name = "Disease  ";
				break;
			case (DAM_DROWNING):
				dam_name = "Drowning ";
				break;
			case (DAM_LIGHT):
				dam_name = "Light    ";
				break;
			case (DAM_OTHER):
				dam++;
				continue;
			case (DAM_HARM):
				dam_name = "Magic    ";
				break;
			case (DAM_CHARM):
				dam_name = "Charm    ";
				break;
			case (DAM_SOUND):
				dam_name = "Sound    ";
				break;
			case (DAM_AIR):
				dam_name = "Air      ";
				break;
			case (DAM_EARTH):
				dam_name = "Earth    ";
				break;
			default:
				dam_name = "BUG      ";
				break;
				
		}
		/*
		switch (check_immune (ch, dam))
		{
			case (IS_IMMUNE):
				sprintf(buf, "{r[{BIMM {r]{x %s\r\n", dam_name);
				SEND(buf, ch);	
				break;
			case (IS_RESISTANT):
				sprintf(buf, "{r[{GRES {r]{x %s\r\n", dam_name);
				SEND(buf, ch);	
				break;
			case (IS_VULNERABLE):
				sprintf(buf, "{r[{RVULN{r]{x %s\r\n", dam_name);
				SEND(buf, ch);	
				break;			
			default:
				sprintf(buf, "{r[{x    {r]{x %s\r\n", dam_name);
				SEND(buf, ch);	
				break;				
		}
		*/	
		res_percent = (resist_table[ch->race].percent[dam]);
		
		if (dam == DAM_FIRE || dam == DAM_COLD || dam == DAM_EARTH || dam == DAM_LIGHTNING || dam == DAM_AIR || dam == DAM_DROWNING)
		{
			if (is_affected(ch,gsn_elemental_protection))
				res_percent -= 10;
		}
		
		if (dam == DAM_NEGATIVE)
		{
			if (is_affected(ch, gsn_negative_plane_protection))
				res_percent -= 25;
		}		
		if (res_percent < -100)
			res_percent = -100;
		
		if (res_percent > 100)
			res_percent = 100;
		
		
		
		if (res_percent < 0)
			sprintf(buf, "{r[{x{G%4d{x%%{r]{x %s\r\n", res_percent, dam_name);
		else if (res_percent == 0)
			sprintf(buf, "{r[{x%4d%%{r]{x %s\r\n", res_percent, dam_name);		
		else
			sprintf(buf, "{r[{x {R+%2d{x%%{r]{x %s\r\n", res_percent, dam_name);
		SEND(buf, ch);		
		buf[0] = '\0';
		dam++;
	}	
	return;
}

int what_size(CHAR_DATA *ch)
{
	int size = 0;
	
	size = ch->size;
	
	if (is_affected(ch,gsn_animal_growth))	
		size++;
	
	if (is_affected(ch,gsn_enlarge_person))	
		size++;

	if (is_affected(ch,gsn_reduce_person))
		size--;
	
	if (size > 5) //gargantuan
		size = 5;
	
	if (size < 0) //tiny
		size = 0;
	
	return size;
}