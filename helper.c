
/*

  HELPER.C
  ========
  (c) Copyright Paul Griffiths 2000
  Email: mail@paulgriffiths.net
  
  Helper functions.

*/


#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <curses.h>

#include "helper.h"
#include "worms.h"

extern int paused;


/*  Quit on error  */

void Error_Quit(char * msg) {

    extern WINDOW * mainwin;
    extern int oldcur;


    /*  Clean up nicely  */

    delwin(mainwin);
    curs_set(oldcur);
    endwin();
    refresh();
    FreeWorm();


    /*  Output error message and exit  */

    perror(msg);
    exit(EXIT_FAILURE);
}


/*  Quit successfully  */

void Quit(int reason) {

    extern WINDOW * mainwin;
    extern int oldcur;
    extern int score;


    /*  Clean up nicely  */

    delwin(mainwin);
    curs_set(oldcur);
    nodelay(mainwin, TRUE);
    refresh();
    endwin();
    FreeWorm();

    /*  Output farewell message  */

    switch ( reason ) {
    case HITWALL:
        printf("\nYou hit a wall!\n");
        printf("Your score is %d\n", score);
        break;

    case HITSELF:
        printf("\nYou ran into yourself!\n");
        printf("Your score is %d\n", score);
        break;

    default:
        printf("Your score is %d\n", score);
        printf("\n");
        break;
    }
    int hifd = open("./highscore.txt", O_RDONLY, 0644);
    int highscore = 0;
    if(hifd >= 0) {
        char buf[255];
        int true_size = read(hifd, buf, sizeof(buf));
        close(hifd);
        if(true_size > 0) {
            sscanf(buf, "%d", &highscore);
        }
    }
    if(score > highscore) {
        hifd = open("./highscore.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(hifd >= 0) {
            char buf[255];
            int len = snprintf(buf, sizeof(buf), "%d", score);
            write(hifd, buf, len);
        }
        printf("You got a new high score!\n");
    }
    else {
        printf("The high score remains %d!\n", highscore);
    }
    printf("Goodbye!\n");

    exit(EXIT_SUCCESS);
}


/*  Returns the x-y size of the terminal  */

void GetTermSize(int * rows, int * cols) {

    struct winsize ws;


    /*  Get terminal size  */

    if ( ioctl(0, TIOCGWINSZ, &ws) < 0 ) {
        perror("couldn't get window size");
        exit(EXIT_FAILURE);
    }
    

    /*  Update globals  */

    *rows = ws.ws_row;
    *cols = ws.ws_col;
}


long parsegraph_timediffMs(struct timespec* a, struct timespec* b)
{
    return (b->tv_sec - a->tv_sec) * 1000 + (b->tv_nsec/1e6 - a->tv_nsec/1e6);
}

/*  Signal handler  */
static struct timespec last_frame;
static int has_last_frame = 0;
extern int dir;
extern int TIMESTEP;

void handler(int signum) {

    extern WINDOW * mainwin;
    extern int oldcur;


    /*  Switch on signal number  */

    int key = ERR;
    switch ( signum ) {

    case SIGALRM:
        do {
            int changedDir = 0;
            key = getch();
            switch ( key ) {
            case KEY_UP:
            case 'W':
            case 'w':
                changedDir = 1;
                ChangeDir(UP);
                break;

            case KEY_DOWN:
            case 'S':
            case 's':
                changedDir = 1;
                ChangeDir(DOWN);
                break;

            case KEY_LEFT:
            case 'A':
            case 'a':
                changedDir = 1;
                ChangeDir(LEFT);
                break;

            case KEY_RIGHT:
            case 'd':
            case 'D':
                changedDir = 1;
                ChangeDir(RIGHT);
                break;
            case 'P':
            case 'p':
                paused = !paused;
                break;
            case 'Q':
            case 'q':
                Quit(USER_QUIT);
                break;
            default:
                break;
            }
            if(!paused) {
                if(!has_last_frame) {
                    has_last_frame = 1;
                }
                else {
                    if(!changedDir) {// && (parsegraph_timediffMs(&last_frame, &now) * 1000 < TIMESTEP / 10)) {
                        struct timespec now;
                        clock_gettime(CLOCK_REALTIME, &now);
                        if((dir == UP || dir == DOWN )&& (parsegraph_timediffMs(&last_frame, &now) * 1000 < TIMESTEP * 2)) {
                            return;
                        }
                        if((dir == LEFT || dir == RIGHT) && (parsegraph_timediffMs(&last_frame, &now) * 1000 < TIMESTEP)) {
                            return;
                        }
                    }   
                }
                /*  Received from the timer  */
                MoveWorm();
            }
        }
        while(key != ERR);
        clock_gettime(CLOCK_REALTIME, &last_frame);
        return;

    case SIGTERM:
    case SIGINT:

        /*  Clean up nicely  */

        delwin(mainwin);
        curs_set(oldcur);
        endwin();
        refresh();
        FreeWorm();
        exit(EXIT_SUCCESS);

    }
}
