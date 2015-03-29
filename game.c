#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"

/* 
*/
int match() {

}

/* 
	Picks a random integer from the interval [a, b]
*/
int randrange(int a, int b) {
	srand(time(null));
	return rand()%(b-a+1)+a;
}

/* 
	Get the pokemons ready
*/
void set_battlefield(struct client *c1, struct client *c2) {
	// Set the rivalry
	c1->opp = c2;
	c2->opp = c1;
	// Toss a coin and decide on who starts
	int coin = randrange(0, 1); // 0-> c1 starts, 1-> c2 starts
	if (coin == 0) { 
		c1->state = MYTURN;
		c2->state = YOURTURN;
	}
	else {
		c1->state = YOURTURN;
		c2->state = MYTURN; 
	}
	// Set the hitpoints randomly between 20 to 30
	c1->pkmn->hp = randrange(20, 30);
	c2->pkmn->hp = randrange(20, 30);
	// Set the number of powermoves between 1 to 3
	c1->pkmn->pm = randrange(1,3);
	c2->pkmn->pm = randrange(1,3);
	// Print the engage messages to both sides
	char message[MAX_LENGTH];
	// Send to c1
	sprintf(message, "You engage %s!\n", c2->name);
	write(c1->fd, message, MAX_LENGTH);
	// Send to c2
	sprintf(message, "You engage %s!\n", c1->name);
	write(c2->fd, message, MAX_LENGTH);

	battlecast(c1);
}

/* 
	Beautiful battlecast
	c is me
*/
void battlecast(struct client *c) {
	// write tim + \n
	char message[MAX_LENGTH];
	sprintf(message, 
		   "Your hitpoints: %d\n
			Your powermoves: %d\n\n 
			%s's hitpoints: %d\n\n", 
			c->pkmn->hp, c->pkmn->pm,
			c->opp->name, c->opp->pkmn->hp);

	struct client *me;
	if (c->state == MYTURN) me = c;
	else me = c->opp;

	write(me->fd, message, MAX_LENGTH);
	write(me->fd, "(a)ttacks\n
		  (p)owermove\n(s)peak something\n", 
	      MAX_LENGTH);
	write(me->opp->fd, message, MAX_LENGTH);
	sprintf(message, "Waiting for %s to strike...\n", me->name);
	write(me->opp->fd, message, MAX_LENGTH);
}

/* 
	Do a status update. Will be called for both side.
	c is me.
*/
void battle_statuscast(struct client *c) {
	char message[MAX_LENGTH];
	sprintf(message, "Your hitpoints: %d\n");
	write(c->msg, message, MAX_LENGTH);
}

/* 
	c is the speaker who spoke s
*/
void battlespeak(struct client *c, char *s) {
	char message[MAX_LENGTH];
	// c will see
	sprintf(message, "You spoke: %s\n\n", s);
	write(c->fd, message, MAX_LENGTH);
	// c's opponent will see
	sprintf(message, "%s takes a break to tell you:\n 
					  %s\n\n", 
					  c->name, s);
	write(c->opp->fd, message, MAX_LENGTH);

	// Now c is done speaking and back to his turn
	c->state = MYTURN;
	battle_mycast(c);
}

/* 
	Handles battle commands. Ignore if wrong command.
	Returns 1 if game over, otherwise 0.
*/
int handle_command(struct client *c, char option) {
	int dmg = 0; // dmg rolled depending on case option
	int did_swing = 0;

	switch(option) {
		case ATTACKMOVE: 
			dmg = randrange(2, 6);
			did_swing = 1;
			break;
		case POWERMOVE:
			if (c->pkmn->pm > 0) {
				dmg = randrange(6, 18) * randrange(0, 1);
				c->pkmn->pm -= 1;
				did_swing = 1;
			}
			break;
		case SPEAKMOVE:
			write(c->fd, "Speak: \n", 7+1);
			c->state = SPEAK;
			break;
	}

	c->opp->pkmn->hp -= dmg;

	int game_over = (c->opp->pkmn->hp) <= 0;
	if (game_over) { // opponent dead, game done
		char message[MAX_LENGTH];
		// Win message to you
		sprintf(message, "%s gives up. You win!\n\n
						  Awaiting next opponent...\n", 
						  c->opp->name); 
		write(c->fd, message, MAX_LENGTH);
		// Lose message to them
		sprintf(message, "You are no match for %s. 
						  You scurry away...\n\n
						  Awaiting next opponent...\n", 
						  c->name);
		write(c->opp->fd, message, MAX_LENGTH); 

	}
	else if (did_swing) { // then switch the turns
		c->state = YOURTURN;
		c->opp->state = MYTURN;
		battlecast(c);
	}

	return game_over;
}


