#ifndef _GAME_H
#define _GAME_H

#include "battleserver.h"

// MACROS

#define WELCOME_MESSAGE "Enter your name: "
#define BROADCAST_MESSAGE  " has entered the arena."

// Battle commands
#define ATTACKMOVE 'a'
#define SPEAKMOVE 's'
#define POWERMOVE 'p'

// States
#define NONAME 0
#define LOBBY 1
#define YOURTURN 2
#define MYTURN 3
#define ISPEAK 4

// FUNCTION PROTOTYPES
int match(struct client *c);
int randrange(int a, int b);
void set_battlefield(struct client *c1, struct client *c2);
void battlecast(struct client *c);
void battlespeak(struct client *c, char *s);
void gameover(struct client *c);
int handle_command(struct client *c, char option);

#endif