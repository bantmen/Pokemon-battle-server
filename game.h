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
#define 0
#define 1
#define 2
#define 3
#define 4
#define 5


// STRUCTS

struct pokemon {
	int hp;  // hitpoints
	int pm;  // powermove 
};

// FUNCTION PROTOTYPES

#endif

