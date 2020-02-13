/*
 *	This file contains all the menus for the account system.
 *	They will actually be called from the nanny function.
 *	Most of the account information/defines have been moved
 *	from merc.h in an attempt to encapsulate all the account
 *	data.
**/

#include <unistd.h>
#include <stdio.h>
#include <time.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"



int clan_lookup     args( (const char *name) );
void stop_idling	(CHAR_DATA *ch );
bool reset_account	( ACCOUNT_DATA *acc );
bool find_account_online(DESCRIPTOR_DATA * d, char *name);
void fwrite_vault_obj (ACCOUNT_DATA *acc, OBJ_DATA * obj, FILE * fp);
void fread_obj (CHAR_DATA * ch, FILE * fp, ACCOUNT_DATA * acc);

void intro_menu(DESCRIPTOR_DATA *d)
{
	STD("\r\n",d); 
	STD("{r =-=-=-=-=-=-=-=-=-=-=-=-=-=-=+=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= {x\r\n",d);
	STD("{r|{x                             {r|{x                               {r|{x\r\n",d);
	STD("{r|{x   {r({xL{r){xog into your account   {r|{x  {r({xF{r){xorgot your password?      {r|{x\r\n",d);
	STD("{r|{x   {r({xC{r){xreate an account       {r|{x  {r({xT{r){xoggle ANSI color          {r|{x\r\n",d);
	STD("{r|{x                             {r|{x  {r({xE{r){xxit             	           {r|{x\r\n",d);
	STD("{r|{x                             {r|{x                        		      {r|{x\r\n",d);
	STD("{r =-=-=-=-=-=-=-=-=-=-=-=-=-=-=+=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= {x\r\n",d);
	STD(">  ",d);
}

void account_help(DESCRIPTOR_DATA *d)
{
	STD("\r\n\r\nAccounts have now been put into place to help players keep\r\n",d);
	STD("track of their characters. You can make up to one hundred characters\r\n",d);
	STD("on your account. If you are having issues with any of your characters,\r\n",d);
	STD("you can reach out to drew.haley@gmail.com.\r\n",d);
}

void create_account_menu(DESCRIPTOR_DATA *d)
{
	STD("{r ---------------------------{YIMPORTANT{W-------------------------- {x\n",d);
	STD("{r|{x  Remeber when you're creating an account that account info   {r|{x\n",d);
	STD("{r|{x  is OOC (out of character).  When choosing an account name   {r|{x\n",d);
	STD("{r|{x  try to make it something ooc oriented.  Also, when you're   {r|{x\n",d);
	STD("{r|{x  choosing a password, don't make it overly simple.  Logging  {r|{x\n",d);
	STD("{r|{x  into an account will allow you to access all of your chars. {r|{x\n",d);
	STD("{r -------------------------------+------------------------------ {x\n",d);
}

void account_main_menu(DESCRIPTOR_DATA *d)
{
	STD("\r\n{r---------------------------------+--------------------------------- {x\r\n",d);
	STD("{r|{x                                {r|{x                                {r|{x\r\n",d);
	STD("{r|{x  {r({xC{r){xreate a new character      {r|{x  {r({xW{r){xho is on                   {r|{x\r\n",d);
	STD("{r|{x  {r({xL{r){xoad an existing character  {r|{x  Change your {r({xE{r){xmail address   {r|{x\r\n",d);
	STD("{r|{x  {r({xS{r){xhow your characters        {r|{x  Change your {r({xP{r){xassword        {r|{x\r\n",d);
	STD("{r|{x  {r({xQ{r){xuit                        {r|{x                                {r|{x\r\n",d);
	STD("{r|{x                                {r|{x                                {r|{x\r\n",d);
	STD("{r---------------------------------+--------------------------------- {x\r\n",d);
	STD(">  ",d);
}

void intro_who(DESCRIPTOR_DATA *d)
{ 
    DESCRIPTOR_DATA *dlist, *d_next;
    char buf[200], buf2[MSL];
    sh_int i = 0;
    bool found = FALSE;

    write_to_buffer(d,"\r\n", 2 );
    for(dlist = descriptor_list; dlist != NULL; dlist = d_next)
    {
	CHAR_DATA *wch;
	
	d_next = dlist->next;

	if ( dlist == d )
	    continue;

	if(dlist->connected != CON_PLAYING )
	    continue;

      	wch = dlist->original ? dlist->original : dlist->character;

        if(!wch || wch->incog_level > 1 || wch->invis_level > 1)
            continue;

	if( IS_AFFECTED(wch, AFF_INVISIBLE )
	||  IS_AFFECTED(wch, AFF_HIDE ) )
	/*||  is_affected( wch, gsn_camoflage ))
	{
	    if ( !is_affected(wch, gsn_faerie_fog)
	    &&   !IS_AFFECTED(wch, AFF_FAERIE_FIRE ))
		continue;
	}*/
		continue;

	if ( IS_IMMORTAL(wch))
            sprintf(buf, " {R-{r %s {R-{x ", wch->name );
	else
            sprintf(buf, " {G-{x %s {G-{x ", wch->name );

	sprintf( buf2, "%-20.20s          %s", buf, ++i % 3 == 0 ? "\r\n" : "" );
	write_to_buffer( d, buf2 ,0);
	found = TRUE;
    }
    if ( !found )
		STD("No one else found online.  Perhaps you should {rinvestigate{x further.\r\n",d);
    else
    {
        write_to_buffer(d,"\r\n", 2 );
	sprintf( buf, "{r%d{x players visible online.  {rM{xore may be waiting for you.\r\n", i );
	STD(buf,d );
        write_to_buffer(d,"\r\n", 2 );
    }
    return;
}

bool reset_account( ACCOUNT_DATA *acc )
{
    int i;

    if ( acc == NULL )
	return FALSE;

	for(i = 0; i < MAX_ACCOUNT_CHAR; i++)
	{
	    free_string( acc->char_list[i].char_name );
	    acc->char_list[i].char_name = str_dup("");

	    free_string( acc->char_list[i].password );
	    acc->char_list[i].password = str_dup("");

	    acc->char_list[i].level 	= 0;
	    acc->char_list[i].ch_class 	= 0;
	    acc->char_list[i].race 	= 0;
	    acc->char_list[i].clan 	= 0;
	    acc->char_list[i].rank 	= 0;
	}

	free_string( acc->name );
	acc->name 	= str_dup("");

	free_string( acc->password );
	acc->password 	= str_dup("");

	free_string( acc->old_passwd );
	acc->old_passwd 	= str_dup("");

	free_string( acc->email );
	acc->email 	= str_dup("");
	acc->act 	= 0;
	acc->verify 	= 0;
	acc->numb 	= -1;
	acc->hours 	= 0;
	acc->secret 	= 0;	
	acc->version 	= 0;
	acc->pen_pts    = 0;
	
	free_string(acc->pen_info);
	acc->pen_info   = str_dup("");
		

	VALIDATE(acc);
	return TRUE;
}

/* alloc the memory for the account */
ACCOUNT_DATA * new_account( void )
{
	int i;
	ACCOUNT_DATA * acc;

	acc = (ACCOUNT_DATA *) alloc_mem( sizeof(*acc));
	memset(acc, 0, sizeof(*acc));

	for(i=0; i < MAX_ACCOUNT_CHAR; i++)
	{
	    acc->char_list[i].char_name = str_dup("");
	    acc->char_list[i].password = str_dup("");

	    acc->char_list[i].level 	= 0;
	    acc->char_list[i].ch_class 	= 0;
	    acc->char_list[i].race 	= 0;
	    acc->char_list[i].clan 	= 0;
	    acc->char_list[i].rank 	= 0;
	}

	acc->desc 	= NULL;
	acc->name 	= str_dup("");
	acc->password 	= str_dup("");
	acc->old_passwd = str_dup("");
	acc->email 	= str_dup("");

	acc->act 	= 0;
	acc->verify 	= 0;
	acc->numb 	= -1;
	acc->hours 	= 0;
	acc->secret 	= 0;
	acc->version 	= 0;
	acc->sql_id 	= 0;	

	VALIDATE(acc);
	return(acc);
}

/* free the memory structures for the account */
void free_account( ACCOUNT_DATA * acc )
{
   int i;
	OBJ_DATA *obj_next;
	OBJ_DATA *obj;

   /* want to make this robust so we don't lose memory to leaks, or crash */
   if( acc )
   {
	char buf[MSL];
	sprintf(buf, "free_account called on account: %s", acc->name);
	log_string(buf);

	if (!IS_VALID(acc))
	    return;

	free_string( acc->name );
	free_string( acc->password );
	free_string( acc->old_passwd );
	free_string( acc->email );

	for(i = 0; i < MAX_ACCOUNT_CHAR; i++)
	{
	    free_string( acc->char_list[i].char_name );
	    free_string( acc->char_list[i].password );

	    acc->char_list[i].level 	= 0;
	    acc->char_list[i].ch_class 	= 0;
	    acc->char_list[i].race 	= 0;
	    acc->char_list[i].clan 	= 0;
	    acc->char_list[i].rank 	= 0;
	}

    for (obj = acc->vault; obj != NULL; obj = obj_next)
    {
        obj_next = obj->next_content;
        extract_obj (obj);
    }

	acc->act 	= 0;
	acc->verify 	= 0;
	acc->numb 	= -1;
	acc->hours 	= 0;
	acc->secret 	= 0;
	acc->version 	= 0;

	INVALIDATE(acc);

// going to see if we just keep it as is.  descriptors do not get memory freed, just
// recycled over and over.

	if (acc->desc != NULL)
	    acc->desc->account = NULL;

	free_mem(acc, sizeof(*acc));
	return;
   }
}

bool load_account_obj( DESCRIPTOR_DATA *d, char *name )
{
   FILE *fp = NULL;
   bool found = FALSE;
   char buf2[50];

   if (d->account != NULL)
    return TRUE;

   if ( IS_NULLSTR(name) || !d )
	return FALSE;

   sprintf(buf2, "%s%s", ACCOUNT_DIR, name);

	if ( (fp = fopen(buf2, "r")) != NULL )
	{
		ACCOUNT_DATA *acc;
		char buf[MAX_STRING_LENGTH];
		char word[MAX_INPUT_LENGTH];
		char dump[MAX_LINE_LENGTH+1];
		char *string;
		int i = 0, version = 0;

		found = TRUE;
		
		while(TRUE)
		{
			if(feof( fp ))
			{
				sprintf(buf, "Account %s has no data", name);
				bug(buf, 0);
			}
			
			fscanf( fp, "%s %s", word, buf );

			if(IS_NULLSTR(word) || IS_NULLSTR(buf))
			    continue;

			if(!(strcmp(word, "Chars")))
			{
				int lvl, rank, cls, cln, rce;

				free_string(acc->char_list[i].char_name);
				acc->char_list[i].char_name = str_dup(buf);

				fscanf( fp, "%d %d %d %d %d", &lvl, &cls, &rce, &cln, &rank );
				acc->char_list[i].level = lvl;
				acc->char_list[i].ch_class = cls;
				acc->char_list[i].race = rce;
				if(cln <= MAX_CLAN)
					acc->char_list[i].clan = cln;
				else
					acc->char_list[i].clan = 0;
				acc->char_list[i].rank = rank;
				i++;
				acc->char_count = i + 1;
			}
			else if(!(strcmp(word, "Email")))
			{
				free_string( acc->email );
				acc->email = str_dup(buf);
			}
			else if(!(strcmp(word, "Version")))
			{
				version = atoi(buf);
				acc->version = version;
			}
			else if(!(strcmp(word, "Name")))
			{
				acc = new_account();
				free_string( acc->name );
				acc->name = str_dup(buf);

                		d->account = acc;
				acc->desc = d;
			}
			else if(!(strcmp(word, "Pass")))
			{
				free_string( acc->password );
				acc->password = str_dup(buf);
			}
			else if(!(strcmp(word, "End")))
			{
				acc->desc = d;				
				fclose(fp);
				return found;
			}
			else if(word[0] == '*')
				fgets(dump, MAX_LINE_LENGTH, fp);
			else if(!(strcmp(word, "Act")))
				acc->act = atoi(buf);
			else if(!(strcmp(word, "Verify")))
				acc->verify = atoi(buf);
			else if (!(strcmp(word, "Plyd")))
				acc->hours = atol(buf);
			else if (!(strcmp(word, "Secret")))
				acc->secret = atol(buf);
			else if (!(strcmp(word, "Pen_pts")))
				acc->pen_pts = atol(buf);
			else if (!(strcmp(word, "Pen_info")))
			{
				free_string( acc->pen_info );
				string = fread_string(fp);
				acc->pen_info = str_dup(string);
			}
			else if (!(strcmp(word, "Vault")))
			{
				fread_obj(NULL, fp, acc);
				//read in vault objs.
			}
			// read in all the clans in an array
			else if (!(strcmp(word,"Clan"))){
			    int clanCount;

			    if(version < 1){
					clanCount = MAX_CLAN - 1;
						// we start at 1 since 0 is nonclan, and already been read into the buffer
						for(int clan=1;clan < clanCount;clan++){
						fscanf(fp,"%s",buf);
						if(IS_NULLSTR(buf))
							break;				    
					}				
			    }
			    else{
					clanCount = MAX_CLAN;
						// we start at 1 since 0 is nonclan, and already been read into the buffer
						for(int clan=1;clan < clanCount;clan++){
						fscanf(fp,"%s",buf);
						if(IS_NULLSTR(buf))
							break;
					}
			    }
			}
			else
			{
				sprintf(buf,"fread_account %s: no match for '%s'.", name, word);
				bug( buf, 0 );
				fgets(dump, MAX_LINE_LENGTH, fp);
			}
		}
		fclose(fp);
	}
	return FALSE;
}


bool new_account_obj( DESCRIPTOR_DATA *d, char *name )
{
	ACCOUNT_DATA *acc;
	FILE *fp;
	char filename[50];

	sprintf(filename, "%s%s", ACCOUNT_DIR, name);
	if ((fp = fopen(filename, "r")) == NULL)
	{
		acc = new_account();

		free_string( acc->name );
		acc->name = str_dup(name);

		acc->desc = d;
		d->account = acc;
		return SUCCESS;
	}
	else
	{
		/* return 0 if an account wasn't made */
		fclose(fp);
		return FAILURE;
	}
}
/*
bool load_account_vault_obj	(DESCRIPTOR_DATA *d, char * name );
{	
	return FALSE;
}
*/


void save_account_obj( ACCOUNT_DATA *acc )
{
	FILE *fp;
	char filename[MSL];
	int i = 0;

	if ( !acc || IS_NULLSTR(acc->name))
	    return;

	sprintf(filename, "%s%s", ACCOUNT_DIR, acc->name);
	if((fp = fopen(filename, "w")) == NULL )
		return;

	fprintf(fp, "Name %s\n", acc->name);
	fprintf(fp, "Pass %s\n", acc->password);
	fprintf(fp, "Version %d\n", ACCOUNT_VERSION);
	if (!IS_NULLSTR(acc->email))
	    fprintf(fp, "Email %s\n", acc->email);
	for(i = 0; i < MAX_ACCOUNT_CHAR; i++)
	{
	    if(!IS_NULLSTR(acc->char_list[i].char_name))
	    {
		if(acc->char_list[i].clan > MAX_CLAN)
			acc->char_list[i].clan = 0;
		fprintf(fp, "Chars %-15s %3d %3d %3d %3d %3d\n", acc->char_list[i].char_name,
	 		acc->char_list[i].level, acc->char_list[i].ch_class, 
			acc->char_list[i].race, acc->char_list[i].clan, 
			acc->char_list[i].rank);
		if(acc->char_list[i].level >= 59)
			acc->act = acc->act | ACC_IMP;
		if(acc->char_list[i].level >= 52)
			acc->act = acc->act | ACC_IMM;
	    }
	}
	fprintf(fp, "Act %ld\n", acc->act);
	fprintf(fp, "Verify %d\n", acc->verify);
	fprintf(fp, "Plyd %ld\n", acc->hours);
	fprintf(fp, "Secret %ld\n", acc->secret);
	fprintf(fp, "Pen_pts %ld\n", acc->pen_pts);
	if (!IS_NULLSTR(acc->pen_info))
	    fprintf(fp, "Pen_info buffer %s~\n", acc->pen_info);

	if (acc->vault != NULL)
        fwrite_vault_obj (acc, acc->vault, fp);
	fprintf(fp,"\n");

	fprintf(fp, "End\n");
	fclose(fp);
}

int ACC_IS_SET(int bits, int check)
{
	if (bits & check)
		return 1;
	else 
		return 0;
}

void ACC_SET(ACCOUNT_DATA * acc, int check)
{
	int bits;

	bits = acc->act;
	if(bits & check)
		return;
	else
		bits = bits | check;
	acc->act = bits;
}

int create_key( void )
{
	struct timeval ts; 
	int key;

	/* this should create a verification key */
	gettimeofday(&ts, NULL);

	srandom(time(NULL)^getpid());
	key = ((ts.tv_sec + 644536) * random()) % 164832;
	if(key < 0)
		key = -key;
	return key;
}

void send_mail(DESCRIPTOR_DATA * d)
{
	/*char *email, *account;
	int verify;
	FILE * outmail;
	char buf[300], outfile[300];

	if(d->connected != CON_GET_EMAIL)
		return;

	if ( !d->account || IS_NULLSTR(d->account->email) || IS_NULLSTR(d->account->name))
	{
	    write_to_buffer(d, "There was problem in verification.  Seek a high level immortal for help.\r\n",0);
	    return;
	}

	email = d->account->email;
	verify = d->account->verify;
	account = d->account->name;

	// send an email to email with the account verify # verify 
	sprintf(outfile, "%s%s.txt", ACCOUNT_DIR, account); // save message
	sprintf(buf, "mail -s \"Aragond Account Verification\" %s < %s", email, outfile);
	outmail = fopen(outfile, "w");
	fprintf(outmail, "This is your verification number for account %s.\n\n%d\n", 
		account, verify);
	fclose(outmail);

	system(buf);*/
	return;
}

void list_characters( DESCRIPTOR_DATA *d)
{
	char buffer[MAX_STRING_LENGTH];
	int i=0;

	/* list of all the characters of an account */
	if (d->account)
	{
		STD("\r\n{r[{x# {r] [{x   Name   {r]    [{xLv{r]    [{x  Class   {r]    [{x   Race   {r]    [{x   Clan   {r]    [{xRnk{r]{x\r\r\n\n",d);
	    for(i=0; i<MAX_ACCOUNT_CHAR; i++)
	    {
		if(!IS_NULLSTR(d->account->char_list[i].char_name))
		{
			sprintf(buffer, "{r[{x%-2i{r]{x %-12s    %-4i    %-12s    %-12s    %-12s    -5%i\r\n", 
			    i, d->account->char_list[i].char_name, 
			    d->account->char_list[i].level, 
			    ch_class_table[d->account->char_list[i].ch_class].name, 
   		  	    d->account->char_list[i].race > MAX_PC_RACE ?
				"-bugged-" : pc_race_table[d->account->char_list[i].race].name,
				(d->account->char_list[i].clan > 0) ? clan_table[d->account->char_list[i].clan].name : "", (d->account->char_list[i].clan > 1) ? d->account->char_list[i].rank : 0);
			STD( buffer, d);
		}
	    }
	}
}

/* returns a 1 if the email address in account has already been used, 
   a 0 otherwise.  A 2 is returned if the email address is from a 
   free service (that we are checking */
int check_account( DESCRIPTOR_DATA *d, char * account )
{
/* alphabetize? */
	FILE * acmail;
	char name[100], acc[100], buf[100], outfile[100];
	sprintf(outfile, "%saccount.lst", ACCOUNT_DIR);
	
	if ( IS_NULLSTR(account))
	    return 1;

	acmail = fopen(outfile, "r+");
	while(fscanf(acmail, "%s %s", name, acc) == 2)
	{
		if(!(strcmp(name, account)))
		{ /* email already exists */
			sprintf(buf, "%s is already registered to the account %s.\r\n",
				account, acc);
			write_to_buffer( d, buf, 0 );
			fclose(acmail);
			return(1);
		}
	}
	fprintf(acmail,"%.40s\t%s\n", account, d->account->name);
	fclose(acmail);
	return(0);
}

void del_account_name( char * account )
{
	FILE * acmail, * outf;
	char name[100], buf[100], outfile[100];
	sprintf(outfile, "%saccount.txt", ACCOUNT_DIR);
	acmail = fopen(outfile, "r");
	sprintf(outfile, "%saccount.new", ACCOUNT_DIR);
	outf = fopen(outfile, "w");
	while(fscanf(acmail, "%s %s", name, buf))
	{
		if(strcmp(name, account))
			fprintf(outf, "%s %s", name, buf);
	}
	fclose(acmail);
	fclose(outf);
	sprintf(buf, "cp %saccount.new %saccount.txt", ACCOUNT_DIR, ACCOUNT_DIR);
	//system(buf);
}

void mail_pass(char * name)
{
      /*  char *email, *password;
        FILE * outmail, * infile;
        char buf[100], outfile[100], word[100];

        email = NULL;
        password = NULL;

        infile = fopen(name, "r");
	
	while(1)
	{
	    fscanf(infile, "%s %s", word, buf);
	    if(!(strcmp(word, "email")))
		email = str_dup(buf);
	    if(!(strcmp("Pass", word)))
		password = str_dup(buf);
	    if(!(strcmp("End", word)))
		break;
	}

	fclose(infile);
        // send an email to email with the account password 
        sprintf(outfile, "%s%s.txt", ACCOUNT_DIR, name); // save message 
        sprintf(buf, "mail -s \"Aragond Account Password\" %s < %s",email, outfile);
        outmail = fopen(outfile, "w");
        fprintf(outmail, "This is your password for account %s.\n\n%s\n",
                name, password);
        fclose(outmail);

        system(buf);*/
}

void account_char_save( ACCOUNT_DATA * acc, char * name, int lvl, 
			int ch_class, int race, int clan, int rank )
{
    if( !acc )
	return;
    /* save the character info here */

// if the account number isn't valid for some stupid reason, or name isn't find another.
    if ( acc->numb < 0 || acc->numb > MAX_ACCOUNT_CHAR || IS_NULLSTR(acc->char_list[acc->numb].char_name ))
	acc->numb = getCharNumb( acc, name );

// second check in case the name wasn't working.
    if ( acc->numb < 0 || acc->numb > MAX_ACCOUNT_CHAR )
	return;

    acc->char_list[acc->numb].level = lvl;
    acc->char_list[acc->numb].ch_class = ch_class;
    acc->char_list[acc->numb].race  = race;
    if(clan)
    {
	acc->char_list[acc->numb].clan = clan;
	acc->char_list[acc->numb].rank = rank;
    }
    else
    {
	acc->char_list[acc->numb].clan = -1;
	acc->char_list[acc->numb].rank = -1;
    }
    save_account_obj(acc);
}

bool account_get_new_char_num( ACCOUNT_DATA * acc )
{
	int i;
	if(!ACC_IS_SET(acc->act, VERIFY))
	{
// I had to modify these to check for name length - Yuki
		if( acc->char_list[0].char_name && acc->char_list[0].char_name[0] == '\0' )
		{
			acc->numb = 0;
			return TRUE;
		}
		return FALSE;
	}
	for( i=0; i<MAX_ACCOUNT_CHAR; i++ )
	{
// I had to modify these to check for name length - Yuki
		if( acc->char_list[i].char_name && acc->char_list[i].char_name[0] == '\0' )
		{
			acc->numb = i;
			return TRUE;
		}
	}
	return FALSE;
}

void account_free_char( ACCOUNT_DATA * acc, int numb )
{
	free_string(acc->char_list[numb].char_name);
	free_string(acc->char_list[numb].password);
	acc->char_list[numb].char_name = str_dup("");
	acc->char_list[numb].password = str_dup("");
	acc->char_list[numb].clan = 0;
}

int stringSearch(char * a, char * b)
{
    register int i, j=0, slot=0;
    for(i=0; a[i]; i++)
    {
        for(j=i, slot=0; a[j] == b[slot]; j++, slot++)
        {
            if(b[slot+1] == '\0')
                return(1);
        }
    }
    return(0);
}


bool check_account_name(char * name)
{
         /*
     * Reserved words.
     */
    if ( is_name( name,
        "all auto immortal self someone something the you demise balance circle loner honor none hassan") )
        return FALSE;

    if ( is_name( name,
        "horde rogue apostle warlock barbarian dragon loner outcast esper nocturne rose Unlinked powerimms" ))
        return FALSE;

    if( ( strlen(name) <=  2 ) || ( strlen(name) > 12 ) )
        return FALSE;

    /*
     * Alpha only.
     * Lock out IllIll twits.
     */
    {
        char *pc;
        bool fIll,adjcaps = FALSE,cleancaps = FALSE;
        size_t total_caps = 0;

        fIll = TRUE;
        for ( pc = name; *pc != '\0'; pc++ )
        {
            if ( !isalpha(*pc) )
                return FALSE;

            if ( isupper(*pc)) /* ugly anti-caps hack */
            {
                if (adjcaps)
                    cleancaps = TRUE;
                total_caps++;
                adjcaps = TRUE;
            }
            else
                adjcaps = FALSE;

            if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
                fIll = FALSE;
        }

        if ( fIll )
            return FALSE;

        if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
            return FALSE;
    }
    return TRUE;
}

/*
ACCOUNT_DATA * get_account_world( char * arg )
{

}
*/
/*
void do_adeny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    ACCOUNT_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Deny whom?\r\n", ch );
        return;
    }

    if ( ( victim = get_account_world( arg ) ) == NULL )
    {  
	DESCRIPTOR_DATA dnewtemp;
	ACCOUNT_DATA * temp = load_account_obj(&dnewtemp, arg);
	
	if ( ACC_IS_SET( victim->act, ACC_IMP ) )
    	{
            send_to_char( "You failed.\r\n", ch );
            return;
        }

    	ACC_SET(victim->act, ACC_DENY);
    	save_account_obj(temp);
	free_account(temp);
	sprintf(buf,"$N denies access to account %s",victim->name);
    	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	send_to_char( "OK.\r\n", ch );
        return;
    }

    if ( ACC_IS_SET( victim->act, ACC_IMP ) )
    {
        send_to_char( "You failed.\r\n", ch );
        return;
    }

    ACC_SET(victim->act, ACC_DENY);
    sprintf(buf,"$N denies access to account %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\r\n", ch );
    send_to_char( "Remember, this doesn't force the player to quit, just prevents them from relogging.\r\n", ch);
    return;
}

*/

bool account_is_playing(DESCRIPTOR_DATA * d)
{
    DESCRIPTOR_DATA *dold, *d_next;
    CHAR_DATA *vch;

    if ( !d || !d->account || IS_NULLSTR( d->account->name ))
	return FALSE;

    if ( !ACC_IS_SET(d->account->act, ACC_IMP) )
    {
	for ( dold = descriptor_list; dold != NULL; dold = d_next)
        {
	    d_next = dold->next;

	    if ( d == dold || !dold->account)
		continue;

	    if (IS_NULLSTR(dold->account->name ))
		continue;

            if ( !strcasecmp( d->account->name, dold->account->name))
	    {
		if ( !dold->character || 
		   ( dold->connected != CON_PLAYING && dold->connected != CON_BREAK_CONNECT))
		{
		    close_socket( dold );
		    return TRUE;
		}

		vch = dold->original ? dold->original : dold->character;

		// this does the checks to see if they have characters in the game and handles it.

                write_to_buffer(d, "\r\n{RYou already have another character playing from your account.{x\r\n",0);
                write_to_buffer( d, "\r\nYou will now be autoconnected to him/her/it/whatever.\r\n",0);

		/*if ( d->character )
		{
			if (d->character->pet)
			{
				CHAR_DATA *pet=d->character->pet;
				char_to_room(pet, get_room_index( ROOM_VNUM_LIMBO));
				extract(pet, TRUE);
			}
			if (d->character->npcmount)
			{   
				CHAR_DATA *pet = d->character->npcmount;
				char_to_room(pet,get_room_index( ROOM_VNUM_LIMBO));
				extract(pet,TRUE);
			}		   
		}*/

		save_account_obj( dold->account );

                d->character       	= dold->original ? dold->original : dold->character;
                dold->character->desc 	= d;

		if ( !load_account_obj( d, dold->account->name ))
		    return FALSE;

		dold->character 	= NULL;

                d->character->timer	= 0;		
		vch 			= d->character;

		close_socket( dold );


                send_to_char("Reconnecting. Type replay to see missed tells.\r\n", vch );
                if ( vch->invis_level < 53 && vch->incog_level < 53 )
                    act( "$n has reconnected.", vch, NULL, NULL, TO_ROOM );
                 
                sprintf( log_buf, "%s@%s reconnected.", vch->name, d->host);
                log_string( log_buf );
                    
                if (get_trust(vch) < 58)
                    wiznet("$N groks the fullness of $S link.",
                       vch,NULL,WIZ_LINKS,0,0);
		stop_idling(vch);
		if ( vch->in_room != NULL )
		    interpret( vch, "look" );
                d->connected = CON_PLAYING;	
		return TRUE;
	    }
        }
    }
    return FALSE;
}

bool find_account_online(DESCRIPTOR_DATA * d, char *name)
{
    DESCRIPTOR_DATA *dold, *d_next;

    if ( !d || !d->account || IS_NULLSTR(name))
	return FALSE;

    for ( dold = descriptor_list; dold != NULL; dold = d_next)
    {
	    d_next = dold->next;

	    if ( d == dold )
		continue;

	    if ( !dold->account || IS_NULLSTR(dold->account->name ))
		continue;

            if ( !strcasecmp( name, dold->account->name))
	    {
		save_account_obj( dold->account );

		if ( !load_account_obj( d, dold->account->name ))
		    return FALSE;

		return TRUE;
	    }
    }
    return FALSE;
}


int getCharNumb(ACCOUNT_DATA * acc, char * name)
{
    int i; 

    for( i=0; i<MAX_ACCOUNT_CHAR; i++ )
    {
	if(!IS_NULLSTR(acc->char_list[i].char_name))
	{
            if(!strcasecmp(acc->char_list[i].char_name, name ))
	    {
      	    	acc->numb = i;
	    	return i;
	    }
	}
    }

    // erm... character not found in yer char_list?
    acc->numb = -1;
    return -1;
}


