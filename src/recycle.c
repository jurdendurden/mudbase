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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"


/* stuff for recycling ban structures */
BAN_DATA *ban_free;

BAN_DATA *new_ban (void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if (ban_free == NULL)
        ban = alloc_perm (sizeof (*ban));
    else
    {
        ban = ban_free;
        ban_free = ban_free->next;
    }

    *ban = ban_zero;
    VALIDATE (ban);
    ban->name = &str_empty[0];
    return ban;
}

void free_ban (BAN_DATA * ban)
{
    if (!IS_VALID (ban))
        return;

    free_string (ban->name);
    INVALIDATE (ban);

    ban->next = ban_free;
    ban_free = ban;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor (void)
{
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *d;

    if (descriptor_free == NULL)
        d = alloc_perm (sizeof (*d));
    else
    {
        d = descriptor_free;
        descriptor_free = descriptor_free->next;
    }

    *d = d_zero;
    VALIDATE (d);

    d->connected = CON_GET_NAME;
    d->showstr_head = NULL;
    d->showstr_point = NULL;
    d->outsize = 2000;
    d->outbuf = alloc_mem (d->outsize);

    return d;
}

void free_descriptor (DESCRIPTOR_DATA * d)
{
    if (!IS_VALID (d))
        return;

    free_string (d->host);
    free_mem (d->outbuf, d->outsize);
    INVALIDATE (d);
    d->next = descriptor_free;
    descriptor_free = d;
}

/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;

GEN_DATA *new_gen_data (void)
{
    static GEN_DATA gen_zero;
    GEN_DATA *gen;

    if (gen_data_free == NULL)
        gen = alloc_perm (sizeof (*gen));
    else
    {
        gen = gen_data_free;
        gen_data_free = gen_data_free->next;
    }
    *gen = gen_zero;
    VALIDATE (gen);
    return gen;
}

void free_gen_data (GEN_DATA * gen)
{
    if (!IS_VALID (gen))
        return;

    INVALIDATE (gen);

    gen->next = gen_data_free;
    gen_data_free = gen;
}

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr (void)
{
    EXTRA_DESCR_DATA *ed;

    if (extra_descr_free == NULL)
        ed = alloc_perm (sizeof (*ed));
    else
    {
        ed = extra_descr_free;
        extra_descr_free = extra_descr_free->next;
    }

    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
    VALIDATE (ed);
    return ed;
}

void free_extra_descr (EXTRA_DESCR_DATA * ed)
{
    if (!IS_VALID (ed))
        return;

    free_string (ed->keyword);
    free_string (ed->description);
    INVALIDATE (ed);

    ed->next = extra_descr_free;
    extra_descr_free = ed;
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect (void)
{
    static AFFECT_DATA af_zero;
    AFFECT_DATA *af;

    if (affect_free == NULL)
        af = alloc_perm (sizeof (*af));
    else
    {
        af = affect_free;
        affect_free = affect_free->next;
    }

    *af = af_zero;


    VALIDATE (af);
    return af;
}

void free_affect (AFFECT_DATA * af)
{
    if (!IS_VALID (af))
        return;

    INVALIDATE (af);
    af->next = affect_free;
    affect_free = af;
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj (void)
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if (obj_free == NULL)
        obj = alloc_perm (sizeof (*obj));
    else
    {
        obj = obj_free;
        obj_free = obj_free->next;
    }
    *obj = obj_zero;
    VALIDATE (obj);

    return obj;
}

void free_obj (OBJ_DATA * obj)
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;

    if (!IS_VALID (obj))
        return;

    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
        paf_next = paf->next;
        free_affect (paf);
    }
    obj->affected = NULL;

    for (ed = obj->extra_descr; ed != NULL; ed = ed_next)
    {
        ed_next = ed->next;
        free_extra_descr (ed);
    }
    obj->extra_descr = NULL;

    free_string (obj->name);
    free_string (obj->description);
    free_string (obj->short_descr);
    free_string (obj->owner);
    INVALIDATE (obj);

    obj->next = obj_free;
    obj_free = obj;
}


/* stuff for recyling characters */
CHAR_DATA *char_free;

CHAR_DATA *new_char (void)
{
    static CHAR_DATA ch_zero;
    CHAR_DATA *ch;
    int i;

    if (char_free == NULL)
        ch = alloc_perm (sizeof (*ch));
    else
    {
        ch = char_free;
        char_free = char_free->next;
    }

    *ch = ch_zero;
    VALIDATE (ch);
    ch->name = &str_empty[0];
    ch->short_descr = &str_empty[0];
    ch->long_descr = &str_empty[0];
    ch->description = &str_empty[0];
    ch->prompt = &str_empty[0];
    ch->prefix = &str_empty[0];
    ch->logon = current_time;
    ch->lines = PAGELEN;
    for (i = 0; i < 4; i++)
        ch->armor[i] = 100;
    ch->position = POS_STANDING;
    ch->hit = 20;
    ch->max_hit = 20;
    ch->mana = 100;
    ch->max_mana = 100;
    ch->move = 120;
    ch->max_move = 120;
	ch->version = CURR_PC_VERSION;
    for (i = 0; i < MAX_STATS; i++)
    {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }

    return ch;
}


void free_char (CHAR_DATA * ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if (!IS_VALID (ch))
        return;

    if (IS_NPC (ch))
        mobile_count--;

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
        obj_next = obj->next_content;
        extract_obj (obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
        paf_next = paf->next;
        affect_remove (ch, paf);
    }

    free_string (ch->name);
    free_string (ch->short_descr);
    free_string (ch->long_descr);
    free_string (ch->description);
	free_string (ch->email);
	free_string (ch->citizen);
    free_string (ch->prompt);
    free_string (ch->prefix);
	free_string (ch->walk_desc);
/*    free_note (ch->pnote); */
    free_pcdata (ch->pcdata);

    ch->next = char_free;
    char_free = ch;

    INVALIDATE (ch);
    return;
}

PC_DATA *pcdata_free;


PC_DATA *new_pcdata (void)
{
    int alias;

    static PC_DATA pcdata_zero;
    PC_DATA *pcdata;

    if (pcdata_free == NULL)
        pcdata = alloc_perm (sizeof (*pcdata));
    else
    {
        pcdata = pcdata_free;
        pcdata_free = pcdata_free->next;
    }

    *pcdata = pcdata_zero;

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
        pcdata->alias[alias] = NULL;
        pcdata->alias_sub[alias] = NULL;
    }
	
    pcdata->buffer = new_buf ();
	pcdata->explored = (EXPLORE_DATA *)calloc(1, sizeof(*(pcdata->explored) ) ); //Allocate explored data
    VALIDATE (pcdata);
    return pcdata;
}


void free_pcdata (PC_DATA * pcdata)
{
    int alias;

    if (!IS_VALID (pcdata))
        return;

    free_string (pcdata->pwd);
    free_string (pcdata->bamfin);
    free_string (pcdata->bamfout);
    free_string (pcdata->title);
    free_buf (pcdata->buffer);

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
        free_string (pcdata->alias[alias]);
        free_string (pcdata->alias_sub[alias]);		
    }
	
	for (alias = 0; alias < MAX_SKILL; alias++)
    {
		pcdata->seen_improve[alias] = TRUE;
	}
	
	{
		EXPLORE_HOLDER *pExp, *e_next;
		for(pExp = pcdata->explored->bits ; pExp ; pExp = e_next )
		{	e_next = pExp->next;
			free(pExp);
		}				
	}
	
	
	
    INVALIDATE (pcdata);
    pcdata->next = pcdata_free;
    pcdata_free = pcdata;

    return;
}

SHIP_DATA *shipdata_free;

SHIP_DATA *new_shipdata(void)
{  

    static SHIP_DATA shipdata_zero;
    SHIP_DATA *shipdata;

    if (shipdata_free == NULL)
        shipdata = alloc_perm (sizeof (*shipdata));
    else
    {
        shipdata = shipdata_free;
        shipdata_free = shipdata_free->next;
    }

    *shipdata = shipdata_zero;    

    VALIDATE (shipdata);
	shipdata->name = &str_empty[0];
    return shipdata;
}



void free_shipdata (SHIP_DATA * shipdata)
{    

    if (!IS_VALID (shipdata))
        return;

    free_string (shipdata->name);
    //free_string (shipdata->in_room);
    
    INVALIDATE (shipdata);
    shipdata->next = shipdata_free;
    shipdata_free = shipdata;

    return;
}

BOUNTY_DATA *bounty_free;

BOUNTY_DATA *new_bounty(void)
{
	static BOUNTY_DATA b_zero;
	BOUNTY_DATA *b;

	if (bounty_free == NULL)
		b = alloc_perm(sizeof(*b));
	else
	{
		b = bounty_free;
		bounty_free = bounty_free->next;
	}
	*b = b_zero;
	VALIDATE(b);
	return b;
}

 

void free_bounty(BOUNTY_DATA *b)
{
	if (!IS_VALID(b))
		return;
	free_string( b->name );
	INVALIDATE(b);
	b->next = bounty_free;
	bounty_free = b;
}

/* stuff for setting ids */
long last_pc_id;
long last_mob_id;

long get_pc_id (void)
{
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id (void)
{
    last_mob_id++;
    return last_mob_id;
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data (void)
{
    MEM_DATA *memory;

    if (mem_data_free == NULL)
        memory = alloc_mem (sizeof (*memory));
    else
    {
        memory = mem_data_free;
        mem_data_free = mem_data_free->next;
    }

    memory->next = NULL;
    memory->id = 0;
    memory->reaction = 0;
    memory->when = 0;
    VALIDATE (memory);

    return memory;
}

void free_mem_data (MEM_DATA * memory)
{
    if (!IS_VALID (memory))
        return;

    memory->next = mem_data_free;
    mem_data_free = memory;
    INVALIDATE (memory);
}



/* buffer sizes */
const int buf_size[MAX_BUF_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
        if (buf_size[i] >= val)
        {
            return buf_size[i];
        }

    return -1;
}

BUFFER *new_buf ()
{
    BUFFER *buffer;

    if (buf_free == NULL)
        buffer = alloc_perm (sizeof (*buffer));
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }

    buffer->next = NULL;
    buffer->state = BUFFER_SAFE;
    buffer->size = get_size (BASE_BUF);

    buffer->string = alloc_mem (buffer->size);
    buffer->string[0] = '\0';
    VALIDATE (buffer);

    return buffer;
}

BUFFER *new_buf_size (int size)
{
    BUFFER *buffer;

    if (buf_free == NULL)
        buffer = alloc_perm (sizeof (*buffer));
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }

    buffer->next = NULL;
    buffer->state = BUFFER_SAFE;
    buffer->size = get_size (size);
    if (buffer->size == -1)
    {
        bug ("new_buf: buffer size %d too large.", size);
        exit (1);
    }
    buffer->string = alloc_mem (buffer->size);
    buffer->string[0] = '\0';
    VALIDATE (buffer);

    return buffer;
}


void free_buf (BUFFER * buffer)
{
    if (!IS_VALID (buffer))
        return;

    free_mem (buffer->string, buffer->size);
    buffer->string = NULL;
    buffer->size = 0;
    buffer->state = BUFFER_FREED;
    INVALIDATE (buffer);

    buffer->next = buf_free;
    buf_free = buffer;
}


bool add_buf (BUFFER * buffer, char *string)
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW)    /* don't waste time on bad strings! */
        return FALSE;

    len = strlen (buffer->string) + strlen (string) + 1;

    while (len >= buffer->size)
    {                            /* increase the buffer size */
        buffer->size = get_size (buffer->size + 1);
        {
            if (buffer->size == -1)
            {                    /* overflow */
                buffer->size = oldsize;
                buffer->state = BUFFER_OVERFLOW;
                bug ("buffer overflow past size %d", buffer->size);
                return FALSE;
            }
        }
    }

    if (buffer->size != oldsize)
    {
        buffer->string = alloc_mem (buffer->size);

        strcpy (buffer->string, oldstr);
        free_mem (oldstr, oldsize);
    }

    strcat (buffer->string, string);
    return TRUE;
}


void clear_buf (BUFFER * buffer)
{
    buffer->string[0] = '\0';
    buffer->state = BUFFER_SAFE;
}


char *buf_string (BUFFER * buffer)
{
    return buffer->string;
}

/* stuff for recycling mobprograms */
PROG_LIST *mprog_free;
PROG_LIST *oprog_free;
PROG_LIST *rprog_free;

PROG_LIST *new_mprog (void)
{
    static PROG_LIST mp_zero;
    PROG_LIST *mp;

    if (mprog_free == NULL)
        mp = alloc_perm (sizeof (*mp));
    else
    {
        mp = mprog_free;
        mprog_free = mprog_free->next;
    }

    *mp = mp_zero;
    mp->vnum = 0;
    mp->trig_type = 0;
    mp->code = str_dup ("");
    VALIDATE (mp);
    return mp;
}

void free_mprog (PROG_LIST * mp)
{
    if (!IS_VALID (mp))
        return;

    INVALIDATE (mp);
    mp->next = mprog_free;
    mprog_free = mp;
}

HELP_AREA *had_free;

HELP_AREA *new_had (void)
{
    HELP_AREA *had;
    static HELP_AREA zHad;

    if (had_free)
    {
        had = had_free;
        had_free = had_free->next;
    }
    else
        had = alloc_perm (sizeof (*had));

    *had = zHad;

    return had;
}

HELP_DATA *help_free;

/*
HELP_DATA *new_help (void)
{
    HELP_DATA *help;

    if (help_free)
    {
        help = help_free;
        help_free = help_free->next;
    }
    else
        help = alloc_perm (sizeof (*help));

    return help;
}
*/

void free_help (HELP_DATA * help)
{
    free_string (help->keyword);
    free_string (help->text);
    help->next = help_free;
    help_free = help;
}


PROG_LIST *new_oprog(void)
{
   static PROG_LIST op_zero;
   PROG_LIST *op;
 
   if (oprog_free == NULL)
       op = alloc_perm(sizeof(*op));
   else
   {
       op = oprog_free;
       oprog_free=oprog_free->next;
   }
 
   *op = op_zero;
   op->vnum             = 0;
   op->trig_type        = 0;
   op->code             = str_dup("");
   VALIDATE(op);
   return op;
}
 
void free_oprog(PROG_LIST *op)
{
   if (!IS_VALID(op))
      return;
 
   INVALIDATE(op);
   op->next = oprog_free;
   oprog_free = op;
}
 
PROG_LIST *new_rprog(void)
{
   static PROG_LIST rp_zero;
   PROG_LIST *rp;
 
   if (rprog_free == NULL)
       rp = alloc_perm(sizeof(*rp));
   else
   {
       rp = rprog_free;
       rprog_free=rprog_free->next;
   }
 
   *rp = rp_zero;
   rp->vnum             = 0;
   rp->trig_type        = 0;
   rp->code             = str_dup("");
   VALIDATE(rp);
   return rp;
}
 
void free_rprog(PROG_LIST *rp)
{
   if (!IS_VALID(rp))
      return;
 
   INVALIDATE(rp);
   rp->next = rprog_free;
   rprog_free = rp;
}


void do_addapply(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA *paf,*af_new;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int affect_modify = 1, enchant_type;  
 
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
 
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    SEND("Syntax: addapply <object> <apply> <value>\r\n", ch);
    return;
  }
 
  obj = get_obj_world(ch,arg1);
 
  if (obj == NULL)
  {
    SEND("No such object exists!\r\n",ch);
    return;
  }
 
  if (!str_cmp(arg2,"hp"))
	enchant_type=APPLY_HIT;
  else if (!str_cmp(arg2,"mana"))
	enchant_type=APPLY_MANA;
  else if (!str_cmp(arg2,"move"))
	enchant_type = APPLY_MOVE;
  else if (!str_cmp(arg2,"str"))
	enchant_type=APPLY_STR;
  else if (!str_cmp(arg2,"dex"))
	enchant_type=APPLY_DEX;
  else if (!str_cmp(arg2,"int"))
	enchant_type=APPLY_INT;
  else if (!str_cmp(arg2,"wis"))
	enchant_type=APPLY_WIS;
  else if (!str_cmp(arg2,"con"))
	enchant_type=APPLY_CON;
  else if (!str_cmp(arg2,"cha"))
	enchant_type=APPLY_CHA;
/*  else if (!str_cmp(arg2,"sex"))
	enchant_type=APPLY_SEX;
*/
  else if (!str_cmp(arg2,"mana"))
	enchant_type=APPLY_MANA;
  else if (!str_cmp(arg2,"move"))
	enchant_type=APPLY_MOVE;
  else if (!str_cmp(arg2,"ac"))
	enchant_type=APPLY_AC;
  else if (!str_cmp(arg2,"hitroll"))
	enchant_type=APPLY_HITROLL;
  else if (!str_cmp(arg2,"damroll"))
	enchant_type=APPLY_DAMROLL;
  else if (!str_cmp(arg2,"meleecrit"))
	enchant_type = APPLY_MELEE_CRIT;
  else if (!str_cmp(arg2,"spelldam"))
	enchant_type = APPLY_SPELL_DAM;
  else if (!str_cmp(arg2,"spellcrit"))
	enchant_type = APPLY_SPELL_CRIT;
  else if (!str_cmp(arg2,"saves"))
	enchant_type=APPLY_SAVING_SPELL;
  else
  {
    SEND("That apply is not possible!\r\n",ch);
    return;
  }
 
  if ( !is_number(arg3) )
  {
    SEND("Applies require a value.\r\n", ch);
    return;
  }
 
  if (enchant_type < 25)
	affect_modify=atoi(arg3);	
 
    if (!obj->enchanted)
    {
      obj->enchanted = TRUE;
 
      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	if (affect_free == NULL)
	af_new = alloc_perm(sizeof(*af_new));
	else
        {
	  af_new = affect_free;
	  affect_free = affect_free->next;
        }
 
      af_new->next = obj->affected;
      obj->affected = af_new;
      af_new->type        = UMAX(0,paf->type);
      af_new->level       = paf->level;
      af_new->duration    = paf->duration;
      af_new->location    = paf->location;
      af_new->modifier    = paf->modifier;
      af_new->bitvector   = paf->bitvector;
      }
    }
 
  if (affect_free == NULL)
    paf = alloc_perm(sizeof(*paf));
  else
  {
    paf = affect_free;
    affect_free = affect_free->next;
  }
 
    if (enchant_type > 24)	
	paf->type       = skill_lookup(arg2);
    else
	paf->type 	= 0;
        paf->level      = 100;
        paf->duration   = -1;
        paf->location   = enchant_type;
	paf->modifier   = affect_modify;
        paf->bitvector  = 0;
	paf->next       = obj->affected;
	obj->affected   = paf;
 
	SEND("Ok.\r\n", ch);
}




 SLEEP_DATA *sd_free;

 SLEEP_DATA *new_sleep_data(void)
 {
    SLEEP_DATA *sd;
    if (sd_free == NULL)
        sd = alloc_perm(sizeof(*sd));
    else
    {
        sd = sd_free;
        sd_free=sd_free->next;
    }
 
    sd->vnum             = 0;
    sd->timer            = 0;
    sd->line             = 0;
    sd->prog             = NULL;
    sd->mob              = NULL;
    sd->ch               = NULL;
    sd->next             = NULL;
    sd->prev             = NULL;
    VALIDATE(sd);
    return sd;
 }

 void free_sleep_data(SLEEP_DATA *sd)
 {
    if (!IS_VALID(sd))
       return;

    INVALIDATE(sd);
    sd->next = sd_free;
    sd_free = sd;
 }

