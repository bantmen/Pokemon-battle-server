#ifndef _GAME_H
#define _GAME_H

// MACROS

#define MAX_LENGTH 1024
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

// STRUCTS

struct pokemon {
	int hp;  // hitpoints
	int pm;  // powermove 
};

// FUNCTION PROTOTYPES

#endif

