#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include <time.h>
#include <ncurses.h>





/*####################################################*/
/*# Program to act as a buccaneer procedures trainer #*/
/*####################################################*/

#define WIDTH 30
#define HEIGHT 10 


/*####################################################*/
/*buccaneer state structure*/

typedef struct{
  /*inputs*/
  float throtPos;  /*throttle position*/

  /*outputs*/
  float rpm;       /*engine RPM*/
  float temp;      /*engine temperature*/

  /*control constants*/
  float throtRate; /*throttle rate of change*/
  float maxRPM;

  /*tracking variables*/
  time_t rawtime;
}buccState;


/*####################################################*/
/*display bits*/

char *choices[] = {
                        "100 %",
                        "80 %",
                        "60 %",
                        "40 %",
                        "20 %",
                        "00 %",
                  };
int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(WINDOW *hud, int highlight);



/*####################################################*/
/*main block*/

int main(){
  int i=0;
  void powerOn(buccState *);
  void checkExit(char *);
  buccState state;
  char doAtrip=1;
  /*the display*/
  int c;
  int startx = 0;
  int starty = 0;
  int highlight = n_choices;
  int choice = 0;
  WINDOW *hud;

  /*set up the screen*/
  initscr();
  clear();
  noecho();
  cbreak(); 
  startx = (80 - WIDTH) / 2;
  starty = (24 - HEIGHT) / 2;
  hud=newwin(HEIGHT, WIDTH, starty, startx);
  keypad(hud,TRUE);
  mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
  refresh();
  print_menu(hud, highlight);



  /*initialise*/
  powerOn(&state);


  /*the main loop*/
  do{
    /*update time*/
    time(&state.rawtime);  /*will be replaced by arduino millis()*/

    /*read keyboard*/
    c = wgetch(hud);
    switch(c){
      case KEY_UP:
        if(highlight == 1)highlight=n_choices;
        else                      --highlight;
        break;
      case KEY_DOWN:
        if(highlight == n_choices)highlight = 1;
        else            ++highlight;
        break;
      case 10:
        choice = highlight;
        break;
      default:
        mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
        refresh();
        break;
    }
    if(choice != 0)break;

    /*adjust state*/
    checkExit(&doAtrip);

    /*output state*/
    print_menu(hud, highlight);
  }while(doAtrip);

  mvprintw(23, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
  clrtoeol();
  refresh();
  endwin();

  return(0);
}/*main*/


/*####################################################*/
/*check for exit status*/

void checkExit(char *doAtrip)
{



  return;
}/*checkExit*/


/*####################################################*/
/*initialise buccaneer state*/

void powerOn(buccState *state)
{

  /*I/O*/
  state->throtPos=0.0;
  state->rpm=0.0;
  state->temp=0.0;

  /*constants*/
  state->maxRPM=21700.0;
  state->throtRate=state->maxRPM/10.0;

  return;
}/*powerOn*/


/*####################################################*/
/*print the menu*/

void print_menu(WINDOW *menu_win, int highlight)
{
  int x, y, i;

  x = 2;
  y = 2;
  box(menu_win, 0, 0);
  for(i = 0; i < n_choices; ++i){
    if(highlight == i + 1){ /* High light the present choice */
      wattron(menu_win, A_REVERSE);
      mvwprintw(menu_win, y, x, "%s", choices[i]);
      wattroff(menu_win, A_REVERSE);
    }else mvwprintw(menu_win, y, x, "%s", choices[i]);
    ++y;
  }
  wrefresh(menu_win);

  return;
}/*print_menu*/


/*the end*/
/*####################################################*/

