#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"

// 
int match() {

}

// Picks a random integer from the interval [a, b]
int randrange(int a, int b) {
	srand(time(null));
	return rand()%(b-a)+a;
}

// Handles battle commands. Ignore if wrong command
int handle_command(struct client *c, char option) {

	switch(option) {
		case 'a':
			break;
		case 'p':
			break;
		case 's':
			break;
	}

}

// Beautiful battlecast
void battlecast(struct client *c) {
	// write tim + \n

	char message[MAX_LENGTH];
	sprinft(message, 
		   "Your hitpoints: %d\n
			Your powermoves: %d\n\n 
			%s's hitpoints: %d\n\n
			(a)ttacks\n
			(p)owermove\n
			(s)peak something\n", 
			c->pkmn->hp, c->pkmn->pm,
			c->opp->name, c->opp->pkmn->hp);
	write(c->fd, message, MAX_LENGTH);
}
