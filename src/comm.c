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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>                /* OLC -- for close read write etc */
#include <stdarg.h>                /* printf_to_char */

#include "merc.h"
#include "interp.h"
#include "recycle.h"

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern int malloc_debug args ((int));
extern int malloc_verify args ((void));
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if    defined(macintosh) || defined(MSDOS)
const char echo_off_str[] = { '\0' };
const char echo_on_str[] = { '\0' };
const char go_ahead_str[] = { '\0' };
#endif

#if    defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "telnet.h"
const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char go_ahead_str[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if    defined(_AIX)
#include <sys/select.h>
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
void bzero args ((char *b, int length));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
int listen args ((int s, int backlog));
int setsockopt args ((int s, int level, int optname, void *optval,
                      int optlen));
int socket args ((int domain, int type, int protocol));
#endif

#if    defined(apollo)
#include <unistd.h>
void bzero args ((char *b, int length));
#endif

#if    defined(__hpux)
int accept args ((int s, void *addr, int *addrlen));
int bind args ((int s, const void *addr, int addrlen));
void bzero args ((char *b, int length));
int getpeername args ((int s, void *addr, int *addrlen));
int getsockname args ((int s, void *name, int *addrlen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
int listen args ((int s, int backlog));
int setsockopt args ((int s, int level, int optname,
                      const void *optval, int optlen));
int socket args ((int domain, int type, int protocol));
#endif

#if    defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if    defined(linux)
/* 
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting these functions.
*/
/*
int    accept        args( ( int s, struct sockaddr *addr, int *addrlen ) );
int    bind        args( ( int s, struct sockaddr *name, int namelen ) );
int    getpeername    args( ( int s, struct sockaddr *name, int *namelen ) );
int    getsockname    args( ( int s, struct sockaddr *name, int *namelen ) );
int    listen        args( ( int s, int backlog ) );
*/

int close args ((int fd));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
/* int    read        args( ( int fd, char *buf, int nbyte ) ); */
int select args ((int width, fd_set * readfds, fd_set * writefds,
                  fd_set * exceptfds, struct timeval * timeout));
int socket args ((int domain, int type, int protocol));
/* int    write        args( ( int fd, char *buf, int nbyte ) ); *//* read,write in unistd.h */
#endif

#if    defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct timeval {
    time_t tv_sec;
    time_t tv_usec;
};
#if    !defined(isascii)
#define    isascii(c)        ( (c) < 0200 )
#endif
static long theKeys[4];

int gettimeofday args ((struct timeval * tp, void *tzp));
#endif

#if    defined(MIPS_OS)
extern int errno;
#endif

#if    defined(MSDOS)
int gettimeofday args ((struct timeval * tp, void *tzp));
int kbhit args ((void));
#endif

#if    defined(NeXT)
int close args ((int fd));
int fcntl args ((int fd, int cmd, int arg));
#if    !defined(htons)
u_short htons args ((u_short hostshort));
#endif
#if    !defined(ntohl)
u_long ntohl args ((u_long hostlong));
#endif
int read args ((int fd, char *buf, int nbyte));
int select args ((int width, fd_set * readfds, fd_set * writefds,
                  fd_set * exceptfds, struct timeval * timeout));
int write args ((int fd, char *buf, int nbyte));
#endif

#if    defined(sequent)
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
int close args ((int fd));
int fcntl args ((int fd, int cmd, int arg));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
#if    !defined(htons)
u_short htons args ((u_short hostshort));
#endif
int listen args ((int s, int backlog));
#if    !defined(ntohl)
u_long ntohl args ((u_long hostlong));
#endif
int read args ((int fd, char *buf, int nbyte));
int select args ((int width, fd_set * readfds, fd_set * writefds,
                  fd_set * exceptfds, struct timeval * timeout));
int setsockopt args ((int s, int level, int optname, caddr_t optval,
                      int optlen));
int socket args ((int domain, int type, int protocol));
int write args ((int fd, char *buf, int nbyte));
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
void bzero args ((char *b, int length));
int close args ((int fd));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int listen args ((int s, int backlog));
int read args ((int fd, char *buf, int nbyte));
int select args ((int width, fd_set * readfds, fd_set * writefds,
                  fd_set * exceptfds, struct timeval * timeout));

#if !defined(__SVR4)
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));

#if defined(SYSV)
int setsockopt args ((int s, int level, int optname,
                      const char *optval, int optlen));
#else
int setsockopt args ((int s, int level, int optname, void *optval,
                      int optlen));
#endif
#endif
int socket args ((int domain, int type, int protocol));
int write args ((int fd, char *buf, int nbyte));
#endif

#if defined(ultrix)
int accept args ((int s, struct sockaddr * addr, int *addrlen));
int bind args ((int s, struct sockaddr * name, int namelen));
void bzero args ((char *b, int length));
int close args ((int fd));
int getpeername args ((int s, struct sockaddr * name, int *namelen));
int getsockname args ((int s, struct sockaddr * name, int *namelen));
int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
int listen args ((int s, int backlog));
int read args ((int fd, char *buf, int nbyte));
int select args ((int width, fd_set * readfds, fd_set * writefds,
                  fd_set * exceptfds, struct timeval * timeout));
int setsockopt args ((int s, int level, int optname, void *optval,
                      int optlen));
int socket args ((int domain, int type, int protocol));
int write args ((int fd, char *buf, int nbyte));
#endif


//void init_signals   args( (void) );
void do_auto_shutdown args( (void) );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *descriptor_list;    /* All open descriptors     */
DESCRIPTOR_DATA *d_next;        /* Next descriptor in loop  */
FILE *fpReserve;                /* Reserved file handle     */
bool god;                        /* All new chars are gods!  */
bool merc_down;                    /* Shutdown         */
bool wizlock;                    /* Game is wizlocked        */
bool newlock;                    /* Game is newlocked        */
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time;            /* time of this pulse */
bool MOBtrigger = TRUE;            /* act() switch                 */


/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args ((void));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

#if defined(unix)
void game_loop_unix args ((int control));
int init_socket args ((int port));
void init_descriptor args ((int control));
bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
bool write_to_descriptor args ((int desc, char *txt, int length));
#endif




/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args ((char *name));
bool check_reconnect args ((DESCRIPTOR_DATA * d, char *name, bool fConn));
bool check_playing args ((DESCRIPTOR_DATA * d, char *name, bool account));
int main args ((int argc, char **argv));
void nanny args ((DESCRIPTOR_DATA * d, char *argument));
bool process_output args ((DESCRIPTOR_DATA * d, bool fPrompt));
void read_from_buffer args ((DESCRIPTOR_DATA * d));
void stop_idling args ((CHAR_DATA * ch));
void bust_a_prompt args ((CHAR_DATA * ch));

/* Needs to be global because of do_copyover */
int port, control;

/* Put global mud config values here. Look at qmconfig command for clues.     */
/*   -- JR 09/23/2000                                                         */
/* Set values for all but IP address in ../area/qmconfig.rc file.             */
/*   -- JR 05/10/2001                                                         */
int mud_ansiprompt, mud_ansicolor, mud_telnetga;

/* Set this to the IP address you want to listen on (127.0.0.1 is good for    */
/* paranoid types who don't want the 'net at large peeking at their MUD)      */
char *mud_ipaddress = "0.0.0.0";

int main (int argc, char **argv)
{
    struct timeval now_time;
    bool fCopyOver = FALSE;
#ifdef IMC
   int imcsocket = -1;
#endif

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug (2);
#endif

    /*
     * Init time.
     */
    gettimeofday (&now_time, NULL);
    current_time = (time_t) now_time.tv_sec;
    strcpy (str_boot_time, ctime (&current_time));

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow (stdout);
    csetmode (C_RAW, stdin);
    cecho2file ("log file", 1, stderr);
#endif

    /*
     * Reserve one channel for our use.
     */
    if ((fpReserve = fopen (NULL_FILE, "r")) == NULL)
    {
        perror (NULL_FILE);
        exit (1);
    }

    /*
     * Get the port number.
     */
    port = 4000;
    if (argc > 1)
    {
        if (!is_number (argv[1]))
        {
            fprintf (stderr, "Usage: %s [port #]\n", argv[0]);
            exit (1);
        }
        else if ((port = atoi (argv[1])) <= 1024)
        {
            fprintf (stderr, "Port number must be above 1024.\n");
            exit (1);
        }

        /* Are we recovering from a copyover? */
        if (argv[2] && argv[2][0])
        {
            fCopyOver = TRUE;
            control = atoi (argv[3]);
#ifdef IMC
	   imcsocket = atoi( argv[4] );
#endif
        }
        else
            fCopyOver = FALSE;

    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    qmconfig_read(); /* Here because it fits, no conflicts with Linux placement -- JR 05/06/01 */
    boot_db ();
    log_string ("Merc is ready to rock.");
    game_loop_mac_msdos ();
#endif

#if defined(unix)

    qmconfig_read(); /* Here so we can set the IP adress. -- JR 05/06/01 */
    if (!fCopyOver)
	{
        control = init_socket (port);
		//init_signals(); /* For the use of the signal handler. -Ferric */
	}

    boot_db ();
    log_f ("ROM is ready to rock on port %d (%s).", port, mud_ipaddress);


 //   if (fCopyOver)
 //       copyover_recover ();

    game_loop_unix (control);
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string ("Normal termination of game.");
    exit (0);
    return 0;
}



#if defined(unix)
int init_socket (int port)
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("Init_socket: socket");
        exit (1);
    }

    if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &x, sizeof (x)) < 0)
    {
        perror ("Init_socket: SO_REUSEADDR");
        close (fd);
        exit (1);
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
        struct linger ld;

        ld.l_onoff = 1;
        ld.l_linger = 1000;

        if (setsockopt (fd, SOL_SOCKET, SO_DONTLINGER,
                        (char *) &ld, sizeof (ld)) < 0)
        {
            perror ("Init_socket: SO_DONTLINGER");
            close (fd);
            exit (1);
        }
    }
#endif

    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons (port);
    sa.sin_addr.s_addr = inet_addr( mud_ipaddress );
    log_f("Set IP address to %s", mud_ipaddress);

    if (bind (fd, (struct sockaddr *) &sa, sizeof (sa)) < 0)
    {
        perror ("Init socket: bind");
        close (fd);
        exit (1);
    }


    if (listen (fd, 3) < 0)
    {
        perror ("Init socket: listen");
        close (fd);
        exit (1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos (void)
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;
	
	log_string("Last compiled on " __DATE__ " at " __TIME__".");
    /*
     * New_descriptor analogue.
     */
    dcon.descriptor = 0;
	if (!mud_ansiprompt)
		dcon.connected = CON_GET_ACCOUNT;
	else
		dcon.connected = CON_ANSI;
    dcon.ansi = mud_ansicolor;
    dcon.host = str_dup ("localhost");
    dcon.outsize = 2000;
    dcon.outbuf = alloc_mem (dcon.outsize);
    dcon.next = descriptor_list;
    dcon.showstr_head = NULL;
    dcon.showstr_point = NULL;
    dcon.pEdit = NULL;            /* OLC */
    dcon.pString = NULL;        /* OLC */
    dcon.editor = 0;            /* OLC */
    descriptor_list = &dcon;

    /*
     * First Contact!
     */
	if (!mud_ansiprompt)
	{
		extern char * help_greeting;
		if ( help_greeting[0] == '.' )
			send_to_desc ( help_greeting+1, &dcon );
		else
			send_to_desc ( help_greeting  , &dcon );
	}

	else
    	write_to_buffer (&dcon, "Do you want ANSI color? (Y)es/(N)o", 0);

    /* Main loop */
    while (!merc_down)
    {
        DESCRIPTOR_DATA *d;

        /*
         * Process input.
         */
        for (d = descriptor_list; d != NULL; d = d_next)
        {
            d_next = d->next;
            d->fcommand = FALSE;

#if defined(MSDOS)
            if (kbhit ())
#endif
            {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor (d))
                {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->daze > 0)
                --d->character->daze;

            if (d->character != NULL && d->character->wait > 0)
            {
                --d->character->wait;
                continue;
            }

            read_from_buffer (d);
            if (d->incomm[0] != '\0')
            {
                d->fcommand = TRUE;
                stop_idling (d->character);

                /* OLC */
                if (d->showstr_point)
                    show_string (d, d->incomm);
                else if (d->pString)
                    string_add (d->character, d->incomm);
                else
                    switch (d->connected)
                    {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                substitute_alias (d, d->incomm);
							
							/*
							switch (d->character->note_state)
							{
								default: break;
								case NOTE_TO:
									handle_con_note_to (d, d->incomm);
									break;

								case NOTE_SUB:
									handle_con_note_subject (d, d->incomm);
									break;

								case NOTE_EXPIRE:
									handle_con_note_expire (d, d->incomm);
									break;

								case NOTE_TEXT:
									handle_con_note_text (d, d->incomm);			
									break;

								case NOTE_FINISH:
									handle_con_note_finish (d, d->incomm);
									break;
							}
							*/
                            break;
                        default:
                            nanny (d, d->incomm);
                            break;
                    }

                d->incomm[0] = '\0';
            }
        }



        /*
         * Autonomous game motion.
         */
        update_handler ();



        /*
         * Output.
         */
        for (d = descriptor_list; d != NULL; d = d_next)
        {
            d_next = d->next;

            if ((d->fcommand || d->outtop > 0))
            {
                if (!process_output (d, TRUE))
                {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                }
            }
        }



        /*
         * Synchronize to a clock.
         * Busy wait (blargh).
         */
        now_time = last_time;
        for (;;)
        {
            int delta;

#if defined(MSDOS)
            if (kbhit ())
#endif
            {
                if (dcon.character != NULL)
                    dcon.character->timer = 0;
                if (!read_from_descriptor (&dcon))
                {
                    if (dcon.character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    dcon.outtop = 0;
                    close_socket (&dcon);
                }
#if defined(MSDOS)
                break;
#endif
            }

            gettimeofday (&now_time, NULL);
            delta = (now_time.tv_sec - last_time.tv_sec) * 1000 * 1000
                + (now_time.tv_usec - last_time.tv_usec);
            if (delta >= 1000000 / PULSE_PER_SECOND)
                break;
        }
        last_time = now_time;
        current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix (int control)
{
    static struct timeval null_time;
    struct timeval last_time;

    signal (SIGPIPE, SIG_IGN);
    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;
	log_string("Last compiled on " __DATE__ " at " __TIME__".");
	
    /* Main loop */
    while (!merc_down)
    {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        DESCRIPTOR_DATA *d;
        int maxdesc;

#if defined(MALLOC_DEBUG)
        if (malloc_verify () != 1)
            abort ();
#endif

        /*
         * Poll all active descriptors.
         */
        FD_ZERO (&in_set);
        FD_ZERO (&out_set);
        FD_ZERO (&exc_set);
        FD_SET (control, &in_set);
        maxdesc = control;
        for (d = descriptor_list; d; d = d->next)
        {
            maxdesc = UMAX (maxdesc, d->descriptor);
            FD_SET (d->descriptor, &in_set);
            FD_SET (d->descriptor, &out_set);
            FD_SET (d->descriptor, &exc_set);
        }

        if (select (maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
        {
            perror ("Game_loop: select: poll");
            exit (1);
        }

        /*
         * New connection?
         */
        if (FD_ISSET (control, &in_set))
            init_descriptor (control);

        /*
         * Kick out the freaky folks.
         */
        for (d = descriptor_list; d != NULL; d = d_next)
        {
            d_next = d->next;
            if (FD_ISSET (d->descriptor, &exc_set))
            {
                FD_CLR (d->descriptor, &in_set);
                FD_CLR (d->descriptor, &out_set);
                if (d->character && d->connected == CON_PLAYING)
                    save_char_obj (d->character);
                d->outtop = 0;
                close_socket (d);
            }
        }

        /*
         * Process input.
         */
        for (d = descriptor_list; d != NULL; d = d_next)
        {
            d_next = d->next;
            d->fcommand = FALSE;

            if (FD_ISSET (d->descriptor, &in_set))
            {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor (d))
                {
                    FD_CLR (d->descriptor, &out_set);
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->daze > 0)
                --d->character->daze;

            if (d->character != NULL && d->character->wait > 0)
            {
                --d->character->wait;
                continue;
            }

            read_from_buffer (d);
            if (d->incomm[0] != '\0')
            {
                d->fcommand = TRUE;
				
				if ( d->pProtocol != NULL )
					d->pProtocol->WriteOOB = 0;

				
                stop_idling (d->character);

                /* OLC */
                if (d->showstr_point)
                    show_string (d, d->incomm);
                else if (d->pString)
                    string_add (d->character, d->incomm);
                else
                    switch (d->connected)
                    {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                substitute_alias (d, d->incomm);
                            break;
                        default:
                            nanny (d, d->incomm);
                            break;
                    }

                d->incomm[0] = '\0';
            }
        }



        /*
         * Autonomous game motion.
         */
        update_handler ();



        /*
         * Output.
         */
        for (d = descriptor_list; d != NULL; d = d_next)
        {
            d_next = d->next;

            if ((d->fcommand || d->outtop > 0)
                && FD_ISSET (d->descriptor, &out_set))
            {
                if (!process_output (d, TRUE))
                {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                }
            }
        }



        /*
         * Synchronize to a clock.
         * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
         * Careful here of signed versus unsigned arithmetic.
         */
        {
            struct timeval now_time;
            long secDelta;
            long usecDelta;

            gettimeofday (&now_time, NULL);
            usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
                + 1000000 / PULSE_PER_SECOND;
            secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
            while (usecDelta < 0)
            {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while (usecDelta >= 1000000)
            {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
            {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;
                if (select (0, NULL, NULL, NULL, &stall_time) < 0)
                {
                    perror ("Game_loop: select: stall");
                    exit (1);
                }
            }
        }

        gettimeofday (&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)

void init_descriptor (int control)
{
	char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    size_t desc, size;

    size = sizeof (sock);
    getsockname (control, (struct sockaddr *) &sock, &size);
    if ((desc = accept (control, (struct sockaddr *) &sock, &size)) < 0)
    {
        perror ("New_descriptor: accept");
        return;
    }

    /*char buf[MSL];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    int desc;
    size_t size;
    

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }*/

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if (fcntl (desc, F_SETFL, FNDELAY) == -1)
    {
        perror ("New_descriptor: fcntl: FNDELAY");
        return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor ();
	//dnew->mxp = FALSE;   /* Initially MXP is off */

    dnew->descriptor = desc;
	dnew->connected = CON_INTRO;    
    dnew->showstr_head = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize = 2000;
    dnew->pEdit = NULL;            /* OLC */
    dnew->pString = NULL;        /* OLC */
    dnew->editor = 0;            /* OLC */
	//dnew->mxp           = TRUE;
    dnew->outbuf = alloc_mem (dnew->outsize);
	dnew->pProtocol     = ProtocolCreate(); 
	dnew->color = 1;	

    size = sizeof (sock);
    if (getpeername (desc, (struct sockaddr *) &sock, &size) < 0)
    {
        perror ("New_descriptor: getpeername");
        dnew->host = str_dup ("(unknown)");
    }
    else
    {
        /*
         * Would be nice to use inet_ntoa here but it takes a struct arg,
         * which ain't very compatible between gcc and system libraries.
         */
        int addr;

        addr = ntohl (sock.sin_addr.s_addr);
        sprintf (buf, "%d.%d.%d.%d",
                 (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
                 (addr >> 8) & 0xFF, (addr) & 0xFF);
        sprintf (log_buf, "Sock.sinaddr:  %s", buf);
        log_string (log_buf);
        from = gethostbyaddr ((char *) &sock.sin_addr,
                              sizeof (sock.sin_addr), AF_INET);
        dnew->host = str_dup (from ? from->h_name : buf);
    }

    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if (check_ban (dnew->host, BAN_ALL))
    {
        write_to_descriptor (desc,
                             "Your site has been banned from this mud.\r\n",
                             0);
        close (desc);
        free_descriptor (dnew);
        return;
    }
    /*
     * Init descriptor data.
     */
    dnew->next = descriptor_list;
    descriptor_list = dnew;

	ProtocolNegotiate(dnew); /* <--- Add this line */

	
	//Check for mxp
	//write_to_buffer(dnew, mxp_will, 0);
	
    /*
     * First Contact!
     */
	intro_menu(dnew);

    return;
}
#endif



void close_socket (DESCRIPTOR_DATA * dclose)
{
    CHAR_DATA *ch;

    if (dclose->outtop > 0)
        process_output (dclose, FALSE);

    if (dclose->snoop_by != NULL)
    {
        write_to_buffer (dclose->snoop_by,
                         "Your victim has left the game.\r\n", 0);
    }

    {
        DESCRIPTOR_DATA *d;

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if (d->snoop_by == dclose)
                d->snoop_by = NULL;
        }
    }

    if ((ch = dclose->character) != NULL)
    {
        sprintf (log_buf, "Closing link to %s.", ch->name);
        log_string (log_buf);
		//close out the pet too, if they have one. - Upro	
		/*if ( ch->pet && ch->pet->in_room == NULL )
		{
			char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
			extract_char( ch->pet, TRUE );
		}*/
		
        /* cut down on wiznet spam when rebooting */
        /* If ch is writing note or playing, just lose link otherwise clear char */
		if ( (dclose->connected == CON_PLAYING && !merc_down) || (dclose->character->desc->editor != ED_NONE) )
		{
            act ("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
            wiznet ("Net death has claimed $N.", ch, NULL, WIZ_LINKS, 0, 0);
            ch->desc = NULL;
        }
		
		/*if ((dclose->connected == CON_PLAYING && !merc_down)
				|| ((dclose->connected >= CON_NOTE_TO)
						&& (dclose->connected <= CON_NOTE_FINISH)))
		*/
        else
        {
			//nuke_pets (dclose->original ? dclose->original :
            //           dclose->character);
            free_char (dclose->original ? dclose->original :
                       dclose->character);
        }
    }

    if (d_next == dclose)
        d_next = d_next->next;

    if (dclose == descriptor_list)
    {
        descriptor_list = descriptor_list->next;
    }
    else
    {
        DESCRIPTOR_DATA *d;

        for (d = descriptor_list; d && d->next != dclose; d = d->next);
        if (d != NULL)
            d->next = dclose->next;
        else
            bug ("Close_socket: dclose not found.", 0);
    }

	ProtocolDestroy( dclose->pProtocol ); /* <--- Add this line */
	
    close (dclose->descriptor);
    free_descriptor (dclose);
#if defined(MSDOS) || defined(macintosh)
    exit (1);
#endif
    return;
}



bool read_from_descriptor (DESCRIPTOR_DATA * d)
{
    int iStart;
	
	static char read_buf[MAX_PROTOCOL_BUFFER]; /* <--- Add this line */
    read_buf[0] = '\0';                        /* <--- Add this line */

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return TRUE;

    /* Check for overflow. */
    iStart = 0;
    if ( strlen(d->inbuf) >= sizeof(d->inbuf) - 10 )
    {
        sprintf (log_buf, "%s input overflow!", d->host);
        log_string (log_buf);
        write_to_descriptor (d->descriptor,
                             "\r\n*** PUT A LID ON IT!!! ***\r\n", 0);
        return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for (;;)
    {
        int c;
        c = getc (stdin);
        if (c == '\0' || c == EOF)
            break;
        putc (c, stdout);
        if ( c == '\r' )
			putc( '\n', stdout );
		read_buf[iStart++] = c;
        if (iStart > sizeof (d->inbuf) - 10)
            break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for (;;)
    {
        int nRead;

        nRead = read( d->descriptor, read_buf + iStart,
	    sizeof(read_buf) - 10 - iStart );
		if ( nRead > 0 )
		{
			iStart += nRead;
			if ( read_buf[iStart-1] == '\n' || read_buf[iStart-1] == '\r' )
			break;
		}
        else if (nRead == 0)
        {
            log_string ("EOF encountered on read.");
            return FALSE;
        }
        else if (errno == EWOULDBLOCK)
            break;
        else
        {
            perror ("Read_from_descriptor");
            return FALSE;
        }
    }
#endif

    read_buf[iStart] = '\0';
    ProtocolInput( d, read_buf, iStart, d->inbuf );
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer (DESCRIPTOR_DATA * d)
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if (d->incomm[0] != '\0')
        return;

    /*
     * Look for at least one new line.
     */
    for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
    {
        if (d->inbuf[i] == '\0')
            return;
    }

    /*
     * Canonical input processing.
     */
    for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
    {
        if (k >= MAX_INPUT_LENGTH - 2)
        {
            write_to_descriptor (d->descriptor, "Line too long.\r\n", 0);

            /* skip the rest of the line */
            for (; d->inbuf[i] != '\0'; i++)
            {
                if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
                    break;
            }
            d->inbuf[i] = '\n';
            d->inbuf[i + 1] = '\0';
            break;
        }

        /*if (d->inbuf[i] == '\b' && k > 0)
            --k;
        else if (isascii (d->inbuf[i]) && isprint (d->inbuf[i]))
            d->incomm[k++] = d->inbuf[i];
		*/
		
		if ( d->inbuf[i] == (signed char) IAC)
        {
          if (!memcmp(&d->inbuf[i], mxp_do, strlen(mxp_do)))
          {
            i += strlen(mxp_do) - 1;
            init_mxp(d);
          }
          else if (!memcmp(&d->inbuf[i], mxp_dont, strlen(mxp_dont)))
          {
            i += strlen(mxp_dont) - 1;
            shutdown_mxp(d);
          }
        }
        else if ( d->inbuf[i] == '\b' && k > 0 )
            --k;
        else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
            d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if (k == 0)
        d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if (k > 1 || d->incomm[0] == '!')
    {
        if (d->incomm[0] != '!' && strcmp (d->incomm, d->inlast))
        {
            d->repeat = 0;
        }
        else
        {
            if (++d->repeat >= 25 && d->character
                && d->connected == CON_PLAYING)
            {
                sprintf (log_buf, "%s input spamming!", d->host);
                log_string (log_buf);
                wiznet ("Spam spam spam $N spam spam spam spam spam!",
                        d->character, NULL, WIZ_SPAM, 0,
                        get_trust (d->character));
                if (d->incomm[0] == '!')
                    wiznet (d->inlast, d->character, NULL, WIZ_SPAM, 0,
                            get_trust (d->character));
                else
                    wiznet (d->incomm, d->character, NULL, WIZ_SPAM, 0,
                            get_trust (d->character));

                d->repeat = 0;
/*
        write_to_descriptor( d->descriptor,
            "\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
        strcpy( d->incomm, "quit" );
*/
            }
        }
    }


    /*
     * Do '!' substitution.
     */
    if (d->incomm[0] == '!')
        strcpy (d->incomm, d->inlast);
    else
        strcpy (d->inlast, d->incomm);

    /*
     * Shift the input buffer.
     */
    while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
        i++;
    for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++);
    return;
}



/*
 * Low level output function.
 */
bool process_output (DESCRIPTOR_DATA * d, bool fPrompt)
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
	if ( d->pProtocol->WriteOOB ) /* <-- Add this, and the ";" and "else" */
        ; /* The last sent data was OOB, so do NOT draw the prompt */    
    else if (!merc_down)
    {
        if (d->showstr_point)
            write_to_buffer (d, "\r\n[Hit Return to continue]\r\n", 0);
        else if (fPrompt && d->pString && d->connected == CON_PLAYING)
            write_to_buffer (d, "> ", 2);
        else if (fPrompt && d->connected == CON_PLAYING)
        {
            CHAR_DATA *ch;
            CHAR_DATA *victim;

            ch = d->character;

            /* battle prompt */
            if ((victim = ch->fighting) != NULL && can_see (ch, victim))
            {
                int percent;
                char wound[100];
				char *pbuff;
                char buf[MSL];
                char buffer[MSL*2];

                if (victim->max_hit > 0)
                    percent = victim->hit * 100 / victim->max_hit;
                else
                    percent = -1;

				if (IS_NPC(victim) && (victim->pIndexData) && IS_SET(victim->act2, ACT2_INANIMATE) && (valid_material(victim->material)))
				{
					if (IS_METAL(victim->material))
					{
						if (percent >= 100)
							sprintf (wound, "looks brand new.");
						else if (percent >= 90)
							sprintf (wound, "has a couple slight dents.");
						else if (percent >= 75)
							sprintf (wound, "is tarnished and dented.");
						else if (percent >= 50)
							sprintf (wound, "has several dents and scratches.");
						else if (percent >= 30)
							sprintf (wound, "is looking pretty battered.");
						else if (percent >= 15)
							sprintf (wound, "has more dents than not.");
						else if (percent >= 0)
							sprintf (wound, "is barely recognizable.");
						else
							sprintf (wound, "is pretty much scrap.");
					}
					else if (IS_GEM(victim->material))
					{
						if (percent >= 100)
							sprintf (wound, "shines like a fresh cut.");
						else if (percent >= 90)
							sprintf (wound, "has a couple slight scratches.");
						else if (percent >= 75)
							sprintf (wound, "appears somewhat scratched.");
						else if (percent >= 50)
							sprintf (wound, "has multiple visible scratches.");
						else if (percent >= 30)
							sprintf (wound, "is looking pretty scratched.");
						else if (percent >= 15)
							sprintf (wound, "has more scratches than not.");
						else if (percent >= 0)
							sprintf (wound, "is barely recognizable.");
						else
							sprintf (wound, "needs to be recut.");
					}
					else if (IS_CLOTH(victim->material))
					{
						if (percent >= 100)
							sprintf (wound, "looks brand new.");
						else if (percent >= 90)
							sprintf (wound, "has some slight fraying.");
						else if (percent >= 75)
							sprintf (wound, "appears somewhat frayed.");
						else if (percent >= 50)
							sprintf (wound, "is pretty badly ripped.");
						else if (percent >= 30)
							sprintf (wound, "is tearing apart.");
						else if (percent >= 15)
							sprintf (wound, "has more rips and frays than not.");
						else if (percent >= 0)
							sprintf (wound, "is barely recognizable.");
						else
							sprintf (wound, "is torn to shreds.");
					}
					else
					{
						if (percent >= 100)
							sprintf (wound, "is in excellent condition.");
						else if (percent >= 90)
							sprintf (wound, "has a few scratches.");
						else if (percent >= 75)
							sprintf (wound, "has some small wounds and bruises.");
						else if (percent >= 50)
							sprintf (wound, "has quite a few wounds.");
						else if (percent >= 30)
							sprintf (wound,
									 "has some big nasty wounds and scratches.");
						else if (percent >= 15)
							sprintf (wound, "looks pretty hurt.");
						else if (percent >= 0)
							sprintf (wound, "is in awful condition.");
						else
							sprintf (wound, "is bleeding to death.");
					}
				}
				else
				{
					if (percent >= 100)
						sprintf (wound, "is in excellent condition.");
					else if (percent >= 90)
						sprintf (wound, "has a few scratches.");
					else if (percent >= 75)
						sprintf (wound, "has some small wounds and bruises.");
					else if (percent >= 50)
						sprintf (wound, "has quite a few wounds.");
					else if (percent >= 30)
						sprintf (wound,
								 "has some big nasty wounds and scratches.");
					else if (percent >= 15)
						sprintf (wound, "looks pretty hurt.");
					else if (percent >= 0)
						sprintf (wound, "is in awful condition.");
					else
						sprintf (wound, "is bleeding to death.");
				}
                sprintf (buf, "%s %s \r\n",
                         IS_NPC (victim) ? victim->short_descr : victim->name,
                         wound);
                buf[0] = UPPER (buf[0]);
				pbuff = buffer;
                colourconv (pbuff, buf, CH(d));
                write_to_buffer (d, buffer, 0);
            }


            ch = d->original ? d->original : d->character;
            if (!IS_SET (ch->comm, COMM_COMPACT))
                write_to_buffer (d, "\r\n", 2);


            if (IS_SET (ch->comm, COMM_PROMPT))
                bust_a_prompt (d->character);

            if (IS_SET (ch->comm, COMM_TELNET_GA))
                write_to_buffer (d, go_ahead_str, 0);
        }
    }

    /*
     * Short-circuit if nothing to write.
     */
    if (d->outtop == 0)
        return TRUE;

    /*
     * Snoop-o-rama.
     */
    if (d->snoop_by != NULL)
    {
        if (d->character != NULL)
            write_to_buffer (d->snoop_by, d->character->name, 0);
        write_to_buffer (d->snoop_by, "> ", 2);
        write_to_buffer (d->snoop_by, d->outbuf, d->outtop);
    }

    /*
     * OS-dependent output.
     */
    if (!write_to_descriptor (d->descriptor, d->outbuf, d->outtop))
    {
        d->outtop = 0;
        return FALSE;
    }
    else
    {
        d->outtop = 0;
        return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[MAX_STRING_LENGTH * 4];
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *dir_name[] = { "N", "E", "S", "W", "U", "D" };
    int door;
	int percent = 0;
	
	// char hit_col[MSL];
	// char mana_col[MSL];
	// char move_col[MSL];
	
	//put colored prompt shit here.
	

    point = buf;
    str = ch->prompt;
    if (str == NULL || str[0] == '\0')
    {
		//bleeding in prompt Upro 1/15/2020
		if (ch->bleeding < 1)
			sprintf(buf2, "not bleeding");
		else if (ch->bleeding > 0 && ch->bleeding < 3)
			sprintf(buf2, "slight bleeding");
		else if (ch->bleeding > 2 && ch->bleeding < 6)
			sprintf(buf2, "bleeding freely");
		else if (ch->bleeding > 5)
			sprintf(buf2, "gushing blood");
		else
			sprintf(buf2, "   BUG  ");		
		
        sprintf (buf, "<{G%d{x/%dhp {G%d{x/%dmp {G%d{X/%dmv> %s",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, buf2);
        SEND (buf, ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_AFK))
    {
        SEND ("<{bAFK{x>{x ", ch);
        return;
    }

    while (*str != '\0')
    {
        if (*str != '%')
        {
            *point++ = *str++;
            continue;
        }
        ++str;
        switch (*str)
        {
            default:
                i = " ";
                break;
            case 'e':
                found = FALSE;
                doors[0] = '\0';
                for (door = 0; door < 6; door++)
                {
                    if ((pexit = ch->in_room->exit[door]) != NULL
                        && pexit->u1.to_room != NULL
                        && (can_see_room (ch, pexit->u1.to_room)
                            || (IS_AFFECTED (ch, AFF_INFRARED)
                                && !IS_AFFECTED (ch, AFF_BLIND)))
                        && !IS_SET (pexit->exit_info, EX_CLOSED))
                    {
                        found = TRUE;
                        strcat (doors, dir_name[door]);
                    }
                }
                if (!found)
                    strcat (doors, "none");
                sprintf (buf2, "%s", doors);
                i = buf2;
                break;
            case 'c':
                sprintf (buf2, "%s", "\r\n");
                i = buf2;
                break;
			case 'b':
				//bleeding in prompt Upro 1/15/2020
				if (ch->bleeding < 1)
					sprintf(buf2, "not bleeding");
				else if (ch->bleeding > 0 && ch->bleeding < 3)
					sprintf(buf2, "{rslight bleeding{x");
				else if (ch->bleeding > 2 && ch->bleeding < 6)
					sprintf(buf2, "{rbleeding {Rfreely{x");
				else if (ch->bleeding > 5)
					sprintf(buf2, "{Rgushing blood{x");
				else
					sprintf(buf2, "   BUG  ");
				
				i = buf2;
				break;
				
            case 'h':               
				if ( ch->max_hit <= 0 )
					percent = 1;
				else
					percent = ( ch->hit * 100 ) / ch->max_hit;
		
				if ( percent >= 66 )
					sprintf( buf2, "{G%d{x", ch->hit );
				else if (percent >= 33)
					sprintf( buf2, "{Y%d{x", ch->hit );
				else
					sprintf( buf2, "{R%d{x", ch->hit );
				
				i = buf2; 
				break;
            case 'H':
                sprintf (buf2, "{G%d{x", ch->max_hit);
                i = buf2;
                break;
            case 'm':
               if ( ch->max_mana <= 0 )
					percent = 1;
				else
					percent = ( ch->mana * 100 ) / ch->max_mana;
		
				if ( percent >= 66 )
					sprintf( buf2, "{G%d{x", ch->mana );
				else if (percent >= 33)
					sprintf( buf2, "{Y%d{x", ch->mana );
				else
					sprintf( buf2, "{R%d{x", ch->mana );
				
				i = buf2; 
				break;
            case 'M':
                sprintf (buf2, "{G%d{x", ch->max_mana);
                i = buf2;
                break;
            case 'v':
                if ( ch->max_move <= 0 )
					percent = 1;
				else
					percent = ( ch->move * 100 ) / ch->max_move;
		
				if ( percent >= 66 )
					sprintf( buf2, "{G%d{x", ch->move );
				else if (percent >= 33)
					sprintf( buf2, "{Y%d{x", ch->move );
				else
					sprintf( buf2, "{R%d{x", ch->move );
				
				i = buf2; 
				break;
            case 'V':
                sprintf (buf2, "{G%d{x", ch->max_move);
                i = buf2;
                break;
            case 'x':
				if (!IS_MCLASSED(ch))
				{	
					sprintf (buf2, "%ld", ch->exp);
					i = buf2;
				}
				else
				{
					sprintf (buf2, "%ld/%ld", ch->exp, ch->exp2);
					i = buf2;
				}
                break;
            case 'X':
				if (!IS_MCLASSED(ch))
				{
					sprintf (buf2, "%ld", IS_NPC (ch) ? 0 : exp_per_level (ch, FALSE));
					i = buf2;
				}
				else
				{
					sprintf (buf2, "%ld/%ld", IS_NPC (ch) ? 0 :
                         exp_per_level (ch, FALSE), exp_per_level (ch, TRUE));
					i = buf2;
				}
                break;
            case 'g':
                sprintf (buf2, "{Y%ld{x", ch->gold);
                i = buf2;
                break;
			case 'G':
				sprintf (buf2, "%d", ch->guildpoints);
				i = buf2;
				break;
            case 's':
                sprintf (buf2, "{D%ld{x", ch->silver);
                i = buf2;
                break;
            case 'a':
                if (ch->level > 9)
                    sprintf (buf2, "%d", ch->alignment);
                else
                    sprintf (buf2, "%s",
                             IS_GOOD (ch) ? "good" : IS_EVIL (ch) ? "evil" :
                             "neutral");
                i = buf2;
                break;
            case 'r':
                if (ch->in_room != NULL)
                    sprintf (buf2, "%s",
                             ((!IS_NPC
                               (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
                              || (!IS_AFFECTED (ch, AFF_BLIND)
                                  && !room_is_dark (ch->
                                                    in_room))) ? ch->in_room->
                             name : "darkness");
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;
            case 'R':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
                    sprintf (buf2, "%ld", ch->in_room->vnum);
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;
            case 'z':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
                    sprintf (buf2, "%s", ch->in_room->area->name);
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;
            case '%':
                sprintf (buf2, "%%");
                i = buf2;
                break;
            case 'o':
				if (IS_IMMORTAL (ch) && ch->in_room != NULL)
				{
					sprintf (buf2, "%s", olc_ed_name (ch));
				
				}
				else
					sprintf (buf2, " ");
                i = buf2;
                break;
			 case 'i' :
				sprintf(buf2,"%d",  ch->carry_number );
				i = buf2; break;
			case 'I' :
				sprintf(buf2,"%d",  can_carry_n(ch));
				i = buf2; break;
			case 'S' :
				sprintf( buf2, "%s", sector_flags[ch->in_room->sector_type].name );
				i = buf2; 
				break;
			case 't' :
				if (time_info.hour >= 2  && time_info.hour < 5)
					sprintf(buf2, "{wnight{x");
				else if (time_info.hour >= 5  && time_info.hour < 8)
					sprintf(buf2, "{ydawn{x");
				else if (time_info.hour >= 8  && time_info.hour < 11)
					sprintf(buf2, "{Ymorning{x");
				else if (time_info.hour >= 11 && time_info.hour < 13)
					sprintf(buf2, "{Wmid-day{x");
				else if (time_info.hour >= 13 && time_info.hour < 16)
					sprintf(buf2, "{gafternoon{x");
				else if (time_info.hour >= 16 && time_info.hour < 19)
					sprintf(buf2, "{mdusk{x");
				else if (time_info.hour >= 19 && time_info.hour < 22)
					sprintf(buf2, "{Devening{x");
				else						
					sprintf(buf2, "{bmid-night{x");
				i = buf2;
				break;
            case 'O':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
				{
					sprintf (buf2, "%s", olc_ed_vnum (ch));
				
				}
				else
					sprintf (buf2, " ");
                i = buf2;
                break;
        }
        ++str;
        while ((*point = *i) != '\0')
            ++point, ++i;
    }
    *point = '\0';
    pbuff = buffer;
    colourconv (pbuff, buf, ch);
    SEND ("", ch);
    write_to_buffer (ch->desc, buffer, 0);
    SEND ("{x\r\n", ch);

    if (ch->prefix[0] != '\0')
        write_to_buffer (ch->desc, ch->prefix, 0);
    return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer (DESCRIPTOR_DATA * d, const char *txt, int length)
{
	int origlength = 0;
	 
	txt = ProtocolOutput( d, txt, &length );  /* <--- Add this line */
    if ( d->pProtocol->WriteOOB > 0 )         /* <--- Add this line */
        --d->pProtocol->WriteOOB;             /* <--- Add this line */

	/*
     * Find length in case caller didn't.
     */
    if (length <= 0)
        length = strlen (txt);

	origlength = length;
	/* work out how much we need to expand/contract it */
	length += count_mxp_tags (d->mxp, txt, length);
		
     /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand && !d->pProtocol->WriteOOB )
    {
        d->outbuf[0] = '\n';
        d->outbuf[1] = '\r';
        d->outtop = 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while (d->outtop + length >= d->outsize)
    {
        char *outbuf;

        if (d->outsize >= 32000)
        {
            bug ("Buffer overflow. Closing.\r\n", 0);
            close_socket (d);
            return;
        }
        outbuf = alloc_mem (2 * d->outsize);
        strncpy (outbuf, d->outbuf, d->outtop);
        free_mem (d->outbuf, d->outsize);
        d->outbuf = outbuf;
        d->outsize *= 2;
    }

    /*
     * Copy.
     */
	convert_mxp_tags (d->mxp, d->outbuf + d->outtop, txt, origlength );    
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor (int desc, char *txt, int length)
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if (desc == 0)
        desc = 1;
#endif

    if (length <= 0)
        length = strlen (txt);

    for (iStart = 0; iStart < length; iStart += nWrite)
    {
        nBlock = UMIN (length - iStart, 4096);
        if ((nWrite = write (desc, txt + iStart, nBlock)) < 0)
        {
            perror ("Write_to_descriptor");
            return FALSE;
        }
    }

    return TRUE;
}



void log_f (char *fmt, ...)
{
    char buf[2 * MSL];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    log_string (buf);
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name (char *name)
{
    int clan;

    /*
     * Reserved words.
     */
    if (is_name (name,
                    "all auto immortal self someone something the you loner none aragond no yes hello bye"))					  
        return FALSE;
    
	//Restricted words.
	if (is_name (name,
                    "death demise balance honor kill slay slayer conclave guild clan cabal kingdom evil good neutral new rear front flank"))					  
        return FALSE;
		
	//No naming yourself after a city/area
	AREA_DATA *pArea;
	
	for (pArea = area_first; pArea; pArea = pArea->next)
    {
		if (is_name (name, pArea->name))
		{
			return FALSE;
		}
	}	
	
	//Curse words
	if (is_name (name,
					"dick pussy faggot ass whore slut fag gay shit fuck bitch cunt clit queer prick bastard"))		    
        return FALSE;
    
	//Egyptian Gods
	if (is_name (name,
					"amun anubis aten atum bastet bes geb hapy hathor horus isis khepri khnum nun osiris ra sekhmet seshat seth shu thoth"))		    
        return FALSE;
	
	//King Arthur
	if (is_name (name,
					"arthur merlin morgana lancelot gawain galahad percival tristram bors ector mordred"))		    
        return FALSE;
	
	//Norse Gods
	if (is_name (name,
					"baldr odin njord freyr frigg freyja freya nanna skade skadi svipdag hermod bor thor tyr heimdall ulle sif bragi idun balder ve vili vidar hod forseti aegir ran hel fenrir jormangund"))		    
        return FALSE;
	
	//Roman Gods
	if (is_name (name,
					"apollo ceres diana juno jupiter mars mercury minerva neptune venus vesta vulcan"))		    
        return FALSE;
	
	//Greek Gods
	if (is_name (name,
					"aphrodite ares artemis athena atlas chronus cronus demeter dionysus eros hades helios hephaestus hera hermes hestia nyx poseidon prometheus styx uranus zeus"))
        return FALSE;
	
	//Famous (infamous) novel characters.
	if (is_name (name,
					"drizzt bruenor frodo bilbo thorin gimli aragorn legolas elrond hallistra pharaun silk"))
        return FALSE;
	
	//Famous Game Characters
	if (is_name (name,
					"bigby rary otiluke nystul vecna melf elminster magius"))
        return FALSE;
	
	
    /* check clans */
    for (clan = 0; clan < MAX_CLAN; clan++)
    {
        if (LOWER (name[0]) == LOWER (clan_table[clan].name[0])
            && !str_cmp (name, clan_table[clan].name))
            return FALSE;
    }

    if (str_cmp (capitalize (name), "Alander") && (!str_prefix ("Alan", name)
                                                   || !str_suffix ("Alander",
                                                                   name)))
        return FALSE;

    /*
     * Length restrictions.
     */

    if (strlen (name) < 2)
        return FALSE;

#if defined(MSDOS)
    if (strlen (name) > 8)
        return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if (strlen (name) > 12)
        return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
        char *pc;
        bool fIll, adjcaps = FALSE, cleancaps = FALSE;
        int total_caps = 0;

        fIll = TRUE;
        for (pc = name; *pc != '\0'; pc++)
        {
            if (!isalpha (*pc))
                return FALSE;

            if (isupper (*pc))
            {                    /* ugly anti-caps hack */
                if (adjcaps)
                    cleancaps = TRUE;
                total_caps++;
                adjcaps = TRUE;
            }
            else
                adjcaps = FALSE;

            if (LOWER (*pc) != 'i' && LOWER (*pc) != 'l')
                fIll = FALSE;
        }

        if (fIll)
            return FALSE;

        if (cleancaps
            || (total_caps > (strlen (name)) / 2
                && strlen (name) < 3)) return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
        extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
        MOB_INDEX_DATA *pMobIndex;
        int iHash;

        for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        {
            for (pMobIndex = mob_index_hash[iHash];
                 pMobIndex != NULL; pMobIndex = pMobIndex->next)
            {
                if (is_name (name, pMobIndex->player_name))
                    return FALSE;
            }
        }
    }

    /*
     * Edwin's been here too. JR -- 10/15/00
     * 
     * Check names of people playing. Yes, this is necessary for multiple
     * newbies with the same name (thanks Saro)
     */
    if (descriptor_list)
    {
        int count=0;
        DESCRIPTOR_DATA *d, *dnext;

        for (d = descriptor_list; d != NULL; d = dnext)
        {
            dnext=d->next;
            if (d->connected!=CON_PLAYING&&d->character&&d->character->name
                && d->character->name[0] && !str_cmp(d->character->name,name))
            {
                count++;
                close_socket(d);
            }
        }
        if (count)
        {
            sprintf(log_buf,"Double newbie alert (%s)",name);
            wiznet(log_buf,NULL,NULL,WIZ_LOGINS,0,0);

            return FALSE;
        }
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect (DESCRIPTOR_DATA * d, char *name, bool fConn)
{
    CHAR_DATA *ch;

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
        if (!IS_NPC (ch)
            && (!fConn || ch->desc == NULL)
            && !str_cmp (d->character->name, ch->name))
        {
            if (fConn == FALSE)
            {
                free_string (d->character->pcdata->pwd);
                d->character->pcdata->pwd = str_dup (ch->pcdata->pwd);
            }
            else
            {
				nuke_pets( d->character );
                free_char (d->character);
                d->character = ch;
                ch->desc = d;
                ch->timer = 0;
                SEND
                    ("Reconnecting. Type replay to see missed tells.\r\n",
                     ch);
                act ("$n has reconnected.", ch, NULL, NULL, TO_ROOM);

                sprintf (log_buf, "%s@%s reconnected.", ch->name, d->host);
                log_string (log_buf);
                wiznet ("$N groks the fullness of $S link.",
                        ch, NULL, WIZ_LINKS, 0, 0);
                d->connected = CON_PLAYING;
				/* Inform the character of a note in progress and the possbility
				 * of continuation!
				 */
				if (ch->pcdata->in_progress)
					SEND ("You have a note in progress. Type 'note edit' to continue it.\r\n", ch);
            }
            return TRUE;
        }
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing (DESCRIPTOR_DATA * d, char *name, bool account )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    if ( account )
	    {
		write_to_buffer(d, "You already have another character playing from your account.\n\r",0);
	    	write_to_buffer( d, "Please enter Y to accept and connect. (enter Y)",0);
	    }
	    else
	    {
	    	write_to_buffer( d, "That character is already playing.\n\r",0);
	    	write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    }
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }
    return FALSE;
}



void stop_idling (CHAR_DATA * ch)
{
    if (ch == NULL
        || ch->desc == NULL
        || ch->desc->connected != CON_PLAYING
        || ch->was_in_room == NULL
        || ch->in_room != get_room_index (ROOM_VNUM_LIMBO)) return;

    ch->timer = 0;
    char_from_room (ch);
    char_to_room (ch, ch->was_in_room);
    ch->was_in_room = NULL;
    act ("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
    return;
}



/*
 * Write to one char.
 */
void SEND_bw (const char *txt, CHAR_DATA * ch)
{
    if (txt != NULL && ch->desc != NULL)
        write_to_buffer (ch->desc, txt, strlen (txt));
    return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw (const char *txt, CHAR_DATA * ch)
{
    if (txt == NULL || ch->desc == NULL)
        return;

    if (ch->lines == 0)
    {
        SEND_bw (txt, ch);
        return;
    }

#if defined(macintosh)
    SEND_bw (txt, ch);
#else
    ch->desc->showstr_head = alloc_mem (strlen (txt) + 1);
    strcpy (ch->desc->showstr_head, txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string (ch->desc, "");
#endif
}

/*
 * Page to one char, new colour version, by Lope.
 */
void SEND (const char *txt, CHAR_DATA * ch)
{
    const char *point;
    char *point2;
    char buf[MAX_STRING_LENGTH * 4];
    int skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if (txt && ch->desc)
    {
        if (IS_SET (ch->act, PLR_COLOUR))
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    skip = colour (*point, ch, point2);
                    while (skip-- > 0)
                        ++point2;
                    continue;
                }
                *point2 = *point;
                *++point2 = '\0';
            }
            *point2 = '\0';
            write_to_buffer (ch->desc, buf, point2 - buf);
        }
        else
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    continue;
                }
                *point2 = *point;
                *++point2 = '\0';
            }
            *point2 = '\0';
            write_to_buffer (ch->desc, buf, point2 - buf);
        }
    }
    return;
}

/*
 * Page to one descriptor using Lope's color.
 */
void send_to_desc (const char *txt, DESCRIPTOR_DATA * d)
{
    const char *point;
    char *point2;
    char buf[MAX_STRING_LENGTH * 4];
    int skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if (txt && d)
    {
        if (d->color == TRUE)
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    skip = colour (*point, NULL, point2);
                    while (skip-- > 0)
                        ++point2;
                    continue;
                }
                *point2 = *point;
                *++point2 = '\0';
            }
            *point2 = '\0';
            write_to_buffer (d, buf, point2 - buf);
        }
        else
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    continue;
                }
                *point2 = *point;
                *++point2 = '\0';
            }
            *point2 = '\0';
            write_to_buffer (d, buf, point2 - buf);
        }
    }
    return;
}

void page_to_char (const char *txt, CHAR_DATA * ch)
{
    const char *point;
    char *point2;
    char buf[MAX_STRING_LENGTH * 4];
    int skip = 0;

#if defined(macintosh)
    SEND (txt, ch);
#else
    buf[0] = '\0';
    point2 = buf;
    if (txt && ch->desc)
    {
        if (IS_SET (ch->act, PLR_COLOUR))
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    skip = colour (*point, ch, point2);
                    while (skip-- > 0)
                        ++point2;
                    continue;
                }
                *point2 = *point;
                *++point2 = '\0';
            }
            *point2 = '\0';
            ch->desc->showstr_head = alloc_mem (strlen (buf) + 1);
            strcpy (ch->desc->showstr_head, buf);
            ch->desc->showstr_point = ch->desc->showstr_head;
            show_string (ch->desc, "");
        }
        else
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    continue;
                }
                *point2 = *point;
                *++point2 = '\0';
            }
            *point2 = '\0';
            ch->desc->showstr_head = alloc_mem (strlen (buf) + 1);
            strcpy (ch->desc->showstr_head, buf);
            ch->desc->showstr_point = ch->desc->showstr_head;
            show_string (ch->desc, "");
        }
    }
#endif
    return;
}

/* string pager */
void show_string (struct descriptor_data *d, char *input)
{
    char buffer[4 * MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument (input, buf);
    if (buf[0] != '\0')
    {
        if (d->showstr_head)
        {
            free_mem (d->showstr_head, strlen (d->showstr_head));
            d->showstr_head = 0;
        }
        d->showstr_point = 0;
        return;
    }

    if (d->character)
        show_lines = d->character->lines;
    else
        show_lines = 0;

    for (scan = buffer;; scan++, d->showstr_point++)
    {
        if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
            && (toggle = -toggle) < 0)
            lines++;

        else if (!*scan || (show_lines > 0 && lines >= show_lines))
        {
            *scan = '\0';
            write_to_buffer (d, buffer, strlen (buffer));
            for (chk = d->showstr_point; isspace (*chk); chk++);
            {
                if (!*chk)
                {
                    if (d->showstr_head)
                    {
                        free_mem (d->showstr_head, strlen (d->showstr_head));
                        d->showstr_head = 0;
                    }
                    d->showstr_point = 0;
                }
            }
            return;
        }
    }
    return;
}


/* quick sex fixer */
void fix_sex (CHAR_DATA * ch)
{
    if (ch->sex < 0 || ch->sex > 2)
        ch->sex = IS_NPC (ch) ? 0 : ch->pcdata->true_sex;
}

void act_new (const char *format, CHAR_DATA * ch, const void *arg1,
              const void *arg2, int type, int min_pos)
{
    static char *const he_she[] = { "it", "he", "she" };
    static char *const him_her[] = { "it", "him", "her" };
    static char *const his_her[] = { "its", "his", "her" };
	static char *const male_female[] = { "male", "female" };

    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA *) arg2;
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[MSL * 2];
    bool fColour = FALSE;


    /*
     * Discard null and zero-length messages.
     */
    if (format == NULL || format[0] == '\0')
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
        return;

    to = ch->in_room->people;
    if (type == TO_VICT)
    {
        if (vch == NULL)
        {
            bug ("Act: null vch with TO_VICT.", 0);
            return;
        }

        if (vch->in_room == NULL)
            return;

        to = vch->in_room->people;
    }

    for (; to != NULL; to = to->next_in_room)
    {
        if ((!IS_NPC (to) && to->desc == NULL)
            || (IS_NPC (to) && !HAS_TRIGGER_MOB(to, TRIG_ACT))
            || to->position < min_pos)
            continue;

        if ((type == TO_CHAR) && to != ch)
            continue;
        if (type == TO_VICT && (to != vch || to == ch))
            continue;
        if (type == TO_ROOM && to == ch)
            continue;
        if (type == TO_NOTVICT && (to == ch || to == vch))
            continue;

        point = buf;
        str = format;
        while (*str != '\0')
        {
            if (*str != '$')
            {
                *point++ = *str++;
                continue;
            }
            fColour = TRUE;
            ++str;
            i = " <@@@> ";

            if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
            {
                bug ("Act: missing arg2 for code %d.", *str);
                i = " <@@@> ";
            }
            else
            {
                switch (*str)
                {
                	/* Added checking of pointers to each case after
                	 * reading about the bug on Edwin's page.
                	 * JR -- 10/15/00
                	 */
                    default:
                        bug ("Act: bad code %d.", *str);
                        i = " <@@@> ";
                        break;
                        /* Thx alex for 't' idea */
                    case 't':
                    	if (arg1)
                    		i = (char *) arg1;
             			else
             				bug("Act: bad code $t for 'arg1'",0);
			            break;
                    case 'T':
                    	if (arg2)
                        	i = (char *) arg2;
                        else
                        	bug("Act: bad code $T for 'arg2'",0);
                        break;
                    case 'n':
                    	if (ch && to)
                        	i = PERS (ch, to);
                        else
                        	bug("Act: bad code $n for 'ch' or 'to'",0);
                        break;
                    case 'N':
                    	if (vch && to)
                        	i = PERS (vch, to);
                        else
                        	bug("Act: bad code $N for 'vch' or 'to'",0);
                        break;
                    case 'e':
                    	if (ch)
                        	i = he_she[URANGE (0, ch->sex, 2)];
                        else
                        	bug("Act: bad code $e for 'ch'",0);
                        break;
                    case 'E':
                    	if (vch)
                        	i = he_she[URANGE (0, vch->sex, 2)];
                        else
                        	bug("Act: bad code $E for 'vch'",0);
                        break;
					case 'g':
						if (vch)
							i = male_female[URANGE (0, vch->sex, 2)];
						else
							bug ("Act: bad code $g for 'vch'",0);
						break;
                    case 'm':
                    	if (ch)
                        	i = him_her[URANGE (0, ch->sex, 2)];
                        else
                        	bug("Act: bad code $m for 'ch'",0);
                        break;
                    case 'M':
                    	if (vch)
                        	i = him_her[URANGE (0, vch->sex, 2)];
                        else
                        	bug("Act: bad code $M for 'vch'",0);
                        break;
                    case 's':
                    	if (ch)
                        	i = his_her[URANGE (0, ch->sex, 2)];
                        else
                        	bug("Act: bad code $s for 'ch'",0);
                        break;
                    case 'S':
                    	if (vch)
                        	i = his_her[URANGE (0, vch->sex, 2)];
                        else
                        	bug("Act: bad code $S for 'vch'",0);
                        break;
                    case 'p':
                    	if (to && obj1)
                        	i = can_see_obj (to, obj1)
                            ? obj1->short_descr : "something";
                        else
                        	bug("Act: bad code $p for 'to' or 'obj1'",0);
                        break;
                    case 'P':
                    	if (to && obj2)
                        	i = can_see_obj (to, obj2)
                            ? obj2->short_descr : "something";
                        else
                        	bug("Act: bad code $P for 'to' or 'obj2'",0);
                        break;
                    case 'd':
                        if (arg2 == NULL || ((char *) arg2)[0] == '\0')
                        {
                            i = "door";
                        }
                        else
                        {
                            one_argument ((char *) arg2, fname);
                            i = fname;
                        }
                        break;
                }
            }

            ++str;
            while ((*point = *i) != '\0')
                ++point, ++i;
        }

        *point++ = '\n';
        *point++ = '\r';
        *point = '\0';
		/* Kludge to capitalize first letter of buffer, trying
		 * to account for { color codes. -- JR 09/09/00
		 */
		if (buf[0] == 123)
        	buf[2] = UPPER (buf[2]);
		else
        	buf[0] = UPPER (buf[0]);
        pbuff = buffer;
        colourconv (pbuff, buf, to);
		if (to->desc && (to->desc->connected == CON_PLAYING))						
			write_to_buffer( to->desc, buffer, 0); /* changed to buffer to reflect prev. fix */				
        else if (MOBtrigger)
            p_act_trigger(buf, to, NULL, NULL, ch, arg1, arg2, TRIG_ACT);
		
    }
	
	if ( type == TO_ROOM || type == TO_NOTVICT )
    {
		OBJ_DATA *obj, *obj_next;
		CHAR_DATA *tch, *tch_next;
 
		point   = buf;
		str     = format;
		while( *str != '\0' )
		{
			*point++ = *str++;
		}
			*point   = '\0';
 
		for( obj = ch->in_room->contents; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
			p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
		}
 
		for( tch = ch; tch; tch = tch_next )
		{
			tch_next = tch->next_in_room;
 
			for ( obj = tch->carrying; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
					p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
			}
		}
 
		if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_ACT ) )
			p_act_trigger( buf, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_ACT );
    }
	
    return;
}


int colour (char type, CHAR_DATA * ch, char *string)
{
    PC_DATA *col;
    char code[20];
    char *p = '\0';

    if (ch && IS_NPC (ch))
        return (0);

    col = ch ? ch->pcdata : NULL;

    switch (type)
    {
        default:
            strcpy (code, CLEAR);
            break;
        case 'x':
            strcpy (code, CLEAR);
            break;
        case 'p':
            if (col->prompt[2])
                sprintf (code, "\e[%d;3%dm%c", col->prompt[0],
                         col->prompt[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->prompt[0], col->prompt[1]);
            break;
        case 's':
            if (col->room_title[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->room_title[0], col->room_title[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->room_title[0],
                         col->room_title[1]);
            break;
        case 'S':
            if (col->room_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->room_text[0], col->room_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->room_text[0],
                         col->room_text[1]);
            break;
        case 'd':
            if (col->gossip[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->gossip[0], col->gossip[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->gossip[0], col->gossip[1]);
            break;
        case '9':
            if (col->gossip_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->gossip_text[0], col->gossip_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->gossip_text[0],
                         col->gossip_text[1]);
            break;
        case 'Z':
            if (col->wiznet[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->wiznet[0], col->wiznet[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->wiznet[0], col->wiznet[1]);
            break;
        case 'o':
            if (col->room_exits[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->room_exits[0], col->room_exits[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->room_exits[0],
                         col->room_exits[1]);
            break;
        case 'O':
            if (col->room_things[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->room_things[0], col->room_things[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->room_things[0],
                         col->room_things[1]);
            break;
        case 'i':
            if (col->immtalk_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->immtalk_text[0], col->immtalk_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm",
                         col->immtalk_text[0], col->immtalk_text[1]);
            break;
        case 'I':
            if (col->immtalk_type[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->immtalk_type[0], col->immtalk_type[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm",
                         col->immtalk_type[0], col->immtalk_type[1]);
            break;
        // case '2':
            // if (col->fight_yhit[2])
                // sprintf (code, "\e[%d;3%dm%c",
                         // col->fight_yhit[0], col->fight_yhit[1], '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->fight_yhit[0],
                         // col->fight_yhit[1]);
            // break;
        // case '3':
            // if (col->fight_ohit[2])
                // sprintf (code, "\e[%d;3%dm%c",
                         // col->fight_ohit[0], col->fight_ohit[1], '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->fight_ohit[0],
                         // col->fight_ohit[1]);
            // break;
        // case '4':
            // if (col->fight_thit[2])
                // sprintf (code, "\e[%d;3%dm%c",
                         // col->fight_thit[0], col->fight_thit[1], '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->fight_thit[0],
                         // col->fight_thit[1]);
            // break;
        // case '5':
            // if (col->fight_skill[2])
                // sprintf (code, "\e[%d;3%dm%c",
                         // col->fight_skill[0], col->fight_skill[1], '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->fight_skill[0],
                         // col->fight_skill[1]);
            // break;
        // case '1':
            // if (col->fight_death[2])
                // sprintf (code, "\e[%d;3%dm%c",
                         // col->fight_death[0], col->fight_death[1], '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->fight_death[0],
                         // col->fight_death[1]);
            // break;
        // case '6':
            // if (col->say[2])
                // sprintf (code, "\e[%d;3%dm%c", col->say[0], col->say[1],
                         // '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->say[0], col->say[1]);
            // break;
        // case '7':
            // if (col->say_text[2])
                // sprintf (code, "\e[%d;3%dm%c",
                         // col->say_text[0], col->say_text[1], '\a');
            // else
                // sprintf (code, "\e[%d;3%dm", col->say_text[0],
                         // col->say_text[1]);
            // break;
        case 'k':
            if (col->tell[2])
                sprintf (code, "\e[%d;3%dm%c", col->tell[0], col->tell[1],
                         '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->tell[0], col->tell[1]);
            break;
        case 'K':
            if (col->tell_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->tell_text[0], col->tell_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->tell_text[0],
                         col->tell_text[1]);
            break;
        case 'l':
            if (col->reply[2])
                sprintf (code, "\e[%d;3%dm%c", col->reply[0],
                         col->reply[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->reply[0], col->reply[1]);
            break;
        case 'L':
            if (col->reply_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->reply_text[0], col->reply_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->reply_text[0],
                         col->reply_text[1]);
            break;
        case 'n':
            if (col->gtell_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->gtell_text[0], col->gtell_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->gtell_text[0],
                         col->gtell_text[1]);
            break;
        case 'N':
            if (col->gtell_type[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->gtell_type[0], col->gtell_type[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->gtell_type[0],
                         col->gtell_type[1]);
            break;
        case 'a':
            if (col->auction[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->auction[0], col->auction[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->auction[0],
                         col->auction[1]);
            break;
        case 'A':
            if (col->auction_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->auction_text[0], col->auction_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->auction_text[0],
                         col->auction_text[1]);
            break;
        case 'q':
            if (col->question[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->question[0], col->question[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->question[0],
                         col->question[1]);
            break;
        case 'Q':
            if (col->question_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->question_text[0], col->question_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm",
                         col->question_text[0], col->question_text[1]);
            break;
        case 'f':
            if (col->answer[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->answer[0], col->answer[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->answer[0], col->answer[1]);
            break;
        case 'F':
            if (col->answer_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->answer_text[0], col->answer_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->answer_text[0],
                         col->answer_text[1]);
            break;
        case 'e':
            if (col->music[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->music[0], col->music[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->music[0], col->music[1]);
            break;
        case 'E':
            if (col->music_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->music_text[0], col->music_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->music_text[0],
                         col->music_text[1]);
            break;
        case 'h':
            if (col->quote[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->quote[0], col->quote[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->quote[0], col->quote[1]);
            break;
        case 'H':
            if (col->quote_text[2])
                sprintf (code, "\e[%d;3%dm%c",
                         col->quote_text[0], col->quote_text[1], '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->quote_text[0],
                         col->quote_text[1]);
            break;
        case 'j':
            if (col->info[2])
                sprintf (code, "\e[%d;3%dm%c", col->info[0], col->info[1],
                         '\a');
            else
                sprintf (code, "\e[%d;3%dm", col->info[0], col->info[1]);
            break;
        case 'b':
            strcpy (code, C_BLUE);
            break;
        case 'c':
            strcpy (code, C_CYAN);
            break;
        case 'g':
            strcpy (code, C_GREEN);
            break;
        case 'm':
            strcpy (code, C_MAGENTA);
            break;
        case 'r':
            strcpy (code, C_RED);
            break;
        case 'w':
            strcpy (code, C_WHITE);
            break;
        case 'y':
            strcpy (code, C_YELLOW);
            break;
        case 'B':
            strcpy (code, C_B_BLUE);
            break;
        case 'C':
            strcpy (code, C_B_CYAN);
            break;
        case 'G':
            strcpy (code, C_B_GREEN);
            break;
        case 'M':
            strcpy (code, C_B_MAGENTA);
            break;
        case 'R':
            strcpy (code, C_B_RED);
            break;
        case 'W':
            strcpy (code, C_B_WHITE);
            break;
        case 'Y':
            strcpy (code, C_B_YELLOW);
            break;
        case 'D':
            strcpy (code, C_D_GREY);
            break;
		case '1':
			strcpy (code, BG_BLACK);
			break;
		case '2':
			strcpy (code, BG_RED);
			break;
		case '3':
			strcpy (code, BG_GREEN);
			break;
		case '4':
			strcpy (code, BG_YELLOW);
			break;
		case '5':
			strcpy (code, BG_BLUE);
			break;
		case '6':
			strcpy (code, BG_MAGENTA);
			break;
		case '7':
			strcpy (code, BG_CYAN);
			break;
		case '8':
			strcpy (code, BG_WHITE);
			break;
        case '*':
            sprintf (code, "%c", '\a');
            break;
        case '/':
            strcpy (code, "\r\n");
            break;
        case '-':
            sprintf (code, "%c", '~');
            break;
        case '{':
            sprintf (code, "%c", '{');
            break;		
    }

    p = code;
    while (*p != '\0')
    {
        *string = *p++;
        *++string = '\0';
    }

    return (strlen (code));
}

void colourconv (char *buffer, const char *txt, CHAR_DATA * ch)
{
    const char *point;
    int skip = 0;

    if (ch && ch->desc && txt)
    {
        if (IS_SET (ch->act, PLR_COLOUR))
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
					if (*point != '\n') {
                    	skip = colour (*point, ch, buffer);
                    	while (skip-- > 0)
                       		++buffer;
                    	continue;
					}
                }
                *buffer = *point;
                *++buffer = '\0';
            }
            *buffer = '\0';
        }
        else
        {
            for (point = txt; *point; point++)
            {
                if (*point == '{')
                {
                    point++;
                    continue;
                }
                *buffer = *point;
                *++buffer = '\0';
            }
            *buffer = '\0';
        }
    }
    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday (struct timeval *tp, void *tzp)
{
    tp->tv_sec = time (NULL);
    tp->tv_usec = 0;
}
#endif

/* source: EOD, by John Booth <???> */

void printf_to_desc (DESCRIPTOR_DATA * d, char *fmt, ...)
{
    char buf[MSL];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    send_to_desc (buf, d);
}

void printf_to_char (CHAR_DATA * ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    SEND (buf, ch);
}

void bugf (char *fmt, ...)
{
    char buf[2 * MSL];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    bug (buf, 0);
}

/*void sig_handler(int sig)
{
switch(sig)
  {
  case SIGBUS:
    bug("Sig handler SIGBUS.",0);
    do_auto_shutdown();
    break;
  case SIGTERM:
    bug("Sig handler SIGTERM.",0);
    do_auto_shutdown();
    break;
  case SIGABRT:
    bug("Sig handler SIGABRT",0);  
    do_auto_shutdown();
    break;
  case SIGSEGV:
    bug("Sig handler SIGSEGV",0);
    do_auto_shutdown();
    break;
  }
}*/

/*
void init_signals()
{
  signal(SIGBUS,sig_handler);
  signal(SIGTERM,sig_handler);
  signal(SIGABRT,sig_handler);
  signal(SIGSEGV,sig_handler);
}
*/

int count_mxp_tags (const int bMXP, const char *txt, int length)
  {
  char c;
  const char * p;
  int count;
  int bInTag = FALSE;
  int bInEntity = FALSE;

  for (p = txt, count = 0; 
       length > 0; 
       p++, length--)
    {
    c = *p;

    if (bInTag)  /* in a tag, eg. <send> */
      {
      if (!bMXP)
        count--;     /* not output if not MXP */   
      if (c == MXP_ENDc)
        bInTag = FALSE;
      } /* end of being inside a tag */
    else if (bInEntity)  /* in a tag, eg. <send> */
      {
      if (!bMXP)
        count--;     /* not output if not MXP */   
      if (c == ';')
        bInEntity = FALSE;
      } /* end of being inside a tag */
    else switch (c)
      {

      case MXP_BEGc:
        bInTag = TRUE;
        if (!bMXP)
          count--;     /* not output if not MXP */   
        break;

      case MXP_ENDc:   /* shouldn't get this case */
        if (!bMXP)
          count--;     /* not output if not MXP */   
        break;

      case MXP_AMPc:
        bInEntity = TRUE;
        if (!bMXP)
          count--;     /* not output if not MXP */   
        break;

      default:
        if (bMXP)
          {
          switch (c)
            {
            case '<':       /* < becomes &lt; */
            case '>':       /* > becomes &gt; */
              count += 3;    
              break;

            case '&':
              count += 4;    /* & becomes &amp; */
              break;

            case '"':        /* " becomes &quot; */
              count += 5;    
              break;

            } /* end of inner switch */
          }   /* end of MXP enabled */
      } /* end of switch on character */

     }   /* end of counting special characters */

  return count;
  } /* end of count_mxp_tags */



  void convert_mxp_tags (const int bMXP, char * dest, const char *src, int length)
  {
char c;
const char * ps;
char * pd;
int bInTag = FALSE;
int bInEntity = FALSE;

  for (ps = src, pd = dest; 
       length > 0; 
       ps++, length--)
    {
    c = *ps;
    if (bInTag)  /* in a tag, eg. <send> */
      {
      if (c == MXP_ENDc)
        {
        bInTag = FALSE;
        if (bMXP)
          *pd++ = '>';
        }
      else if (bMXP)
        *pd++ = c;  /* copy tag only in MXP mode */
      } /* end of being inside a tag */
    else if (bInEntity)  /* in a tag, eg. <send> */
      {
      if (bMXP)
        *pd++ = c;  /* copy tag only in MXP mode */
      if (c == ';')
        bInEntity = FALSE;
      } /* end of being inside a tag */
    else switch (c)
      {
      case MXP_BEGc:
        bInTag = TRUE;
        if (bMXP)
          *pd++ = '<';
        break;

      case MXP_ENDc:    /* shouldn't get this case */
        if (bMXP)
          *pd++ = '>';
        break;

      case MXP_AMPc:
        bInEntity = TRUE;
        if (bMXP)
          *pd++ = '&';
        break;

      default:
        if (bMXP)
          {
          switch (c)
            {
            case '<':
              memcpy (pd, "&lt;", 4);
              pd += 4;    
              break;

            case '>':
              memcpy (pd, "&gt;", 4);
              pd += 4;    
              break;

            case '&':
              memcpy (pd, "&amp;", 5);
              pd += 5;    
              break;

            case '"':
              memcpy (pd, "&quot;", 6);
              pd += 6;    
              break;

            default:
              *pd++ = c;
              break;  /* end of default */

            } /* end of inner switch */
          }
        else
          *pd++ = c;  /* not MXP - just copy character */
        break;  

      } /* end of switch on character */

    }   /* end of converting special characters */
  } /* end of convert_mxp_tags */
  
  