
#include "merc.h"




typedef struct		kingdom_data		KINGDOM_DATA;


struct		kingdom_data
{
	KINGDOM_DATA *		prev;
	KINGDOM_DATA *		next;
	char *		name;
	long		gold;
	int			happiness;			//general happiness of people throughout kingdom. (1-5)
	int			financial;			//financial stability. (1-5)
	int			living_tax;			//how much taxes per month to reside.
	int			purchase_tax; 		//How much tax per purchase.
	char *		cities[5];			//Max five cities/towns per kingdom for now.
};

#define KINGDOM_DIR     "../kingdoms/"          /* Nation data dir              */
#define KINGDOM_LIST    "kingdom.lst"           /* List of nations              */
#define TROOP_DIR       "../kingdoms/troops/"   /* Army data dir                */
#define TROOP_LIST      "troops.lst"            /* List of army troops          */
#define HERO_DIR		"../kingdoms/heroes/"	/* Hero data dir		*/
#define HERO_LIST		"heroes.lst"		/* List of heroes		*/
#define KGDM_AREA_DIR	"../kingdoms/areas/"	/* Dir of conquered areas	*/
#define KGDM_AREA_LIST  "kgdmarea.lst"	/* List of conquered areas	*/



#define KD	KINGDOM_DATA
KD *    get_kingdom	args( ( char *name ) );
void 	remove_kmember	args( ( CHAR_DATA *ch ) );
void 	save_kingdom	args( ( KINGDOM_DATA *kingdom ) );
#undef KD