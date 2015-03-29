#ifndef _BATTLESERVER_H
#define _BATTLESERVER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// MACROS
#ifndef PORT
    #define PORT 30101
#endif
#define MAX_LENGTH 1024

// STRUCTS
struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;   // next for the general list
    struct client *l_next; // next for the lobby list
    // Our stuff now
    //int last_played;         // fd of the last played player. -1 means NULL
    struct client *opp;   // pointer to opponent. also shows last played
    // char name[MAX_LENGTH/2]; 
    char name[MAX_LENGTH]; 
    char buf[MAX_LENGTH];
    char inbuf;
    struct pokemon *pkmn; // to be used in battles
    int state;  // 0-> haven't given name, 1-> in lobby, 2-> in game without turn, 3-> in game and his turn
};

struct pokemon {
	int hp;  // hitpoints
	int pm;  // powermove 
};

// FUNCTION PROTOTYPES
static struct client *addclient(struct client *top, int fd, struct in_addr addr);
static struct client *removeclient(struct client *top, int fd);
static void broadcast(struct client *top, char *s, int size, struct client *c);
int handle_input(int fd, char *buf); 
int find_network_newline(char *buf, int inbuf);
int bindandlisten(void);
void handle_existing(struct client *c, int read_len);

#endif