/*
 * socket demonstrations:
 * This is the server side of an "internet domain" socket connection, for
 * communicating over the network.
 *
 * In this case we are willing to wait either for chatter from the client
 * _or_ for a new connection.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "game.h" 

#ifndef PORT
    #define PORT 30101
#endif

// USE A FUCKING .h file
struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;   // next for the general list
    struct client *l_next; // next for the lobby list
    // Our stuff now
    //int last_played;         // fd of the last played player. -1 means NULL
    struct client *opp;   // pointer to opponent. also shows last played
    // char name[MAX_LENGTH/2]; 
    char const *name; 
    char buf[MAX_LENGTH];
    char inbuf;
    struct pokemon *pkmn; // to be used in battles
    int state;  // 0-> haven't given name, 1-> in lobby, 2-> in game without turn, 3-> in game and his turn
};

static struct client *addclient(struct client *top, int fd, struct in_addr addr, const char *name);
static struct client *removeclient(struct client *top, int fd);
static void broadcast(struct client *top, char *s, int size, struct client *c);
int handle_input(int fd, char *buf); 
int find_network_newline(char *buf, int inbuf);
int bindandlisten(void);

// Global Variables
struct client *head = NULL;  // Linked list of all clients
struct client *lobby = NULL; // Queue of clients waiting in the lobby 

int main(void) {
    int clientfd, maxfd, nready;
    socklen_t len;
    struct sockaddr_in q;
    struct timeval tv;
    fd_set allset;
    fd_set rset;

    // int i;

    // Our stuff
    char client_name[MAX_LENGTH/2];
    int read_len;
    int readyfd;
    struct client lobby[3]; // waitlist for the available players
    struct client *c;

    int listenfd = bindandlisten();
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        /* timeout in seconds (You may not need to use a timeout for
        * your assignment)*/
        tv.tv_sec = 10;
        tv.tv_usec = 0;  /* and microseconds */

        nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
        if (nready == 0) {
            printf("No response from clients in %ld seconds\n", tv.tv_sec);
            printf("Name: %s\n", head? head->name : "NOPE");
            continue;
        }

        if (nready == -1) {
            perror("select");
            continue;
        }

        // Then handle the new guy
        if (FD_ISSET(listenfd, &rset)) {
            printf("a new client is connecting\n");
            len = sizeof(q);
            if ((clientfd = accept(listenfd, (struct sockaddr *)&q, &len)) < 0) {
                perror("accept");
                exit(1);
            }

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("connection from %s\n", inet_ntoa(q.sin_addr));

            head = addclient(head, clientfd, q.sin_addr); 

            write(clientfd, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE)); // Ask for their name
        }

        for (c = head; c; c = c->next) {
            // Handle all the existing clients
            if (FD_ISSET(c->fd, &rset)) {
                if ((read_len = read(c->fd, c->buf + c->inbuf, sizeof(c->buf)-c->inbuf)) > 0) {
                    handle_existing(c, read_len);
                }
            }
        }

    }
    return 0;
}

int find_network_newline(char *buf, int inbuf) {
    int i;

    for(i=0; i<inbuf; i++) {
        if (buf[i] == '\n') {
            return i;
        }
    }
    return -1; // if '\n' not found
}

void handle_existing(struct client *c, int read_len) {
    char message[MAX_LENGTH];

    int state = c->state;
    switch(state) {
        case NONAME:   // Check if full message is ready. if ready, then give name
            where = find_network_newline(c->buf, c->inbuf);
            if (where != -1) {  // Then ready to be collected
                c->buf[where] = '\0'; // Change \n with \0
                strncpy(c->name, c->buf, c->inbuf);
                c->inbuf = 0;
                sprintf(message, "Welcome, %s! Awaiting opponent...\n", c->name);
                write(c->fd, message, MAX_LENGTH);
                sprintf(message, "**%s enters the arena**\n", c->name);
                broadcast(head, message, strlen(message), c);

                // Change the state to lobby. try to match, else enqueue
                c->state = LOBBY;
                match(c);
            }
            else {  // update inbuf since they are not done writing
                c->inbuf += read_len;
            }
            break;
        case LOBBY:    // Ignore the lobby talk
            // Don't care
            c->inbuf = 0;
            break;
        case YOURTURN: // Can't talk when it is not your turn!
            // Don't care
            c->inbuf = 0;
            break;
        case MYTURN:   // Handle their command
            if (handle_command(c, c->buf[c->inbuf-1]) == 1) { // game over if 1
                // add them to the end of lobby queue
                match(c);
                match(c->opp);
            }
            c->inbuf = 0;
            break;
        case ISPEAK:  // Check if full message is ready. if ready, then print it to the battle
            where = find_network_newline(c->buf, c->inbuf);
            if (where != -1) {  // Then ready to be collected
                c->buf[where+1] = '\0';   // Null terminate the message
                c->inbuf = 0;
                c->state = MYTURN; // Change back to his previous state
            }
            else {   // update inbuf since they are not done writing
                c->inbuf += read_len;
            }
            break;
    }

}

 /* bind and listen, abort on error
  * returns FD of listening socket
  */
int bindandlisten(void) {
    struct sockaddr_in r;
    int listenfd;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        perror("setsockopt");
    }
    memset(&r, '\0', sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}

static struct client *addclient(struct client *top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;

    // strncpy(p->name, name, MAX_LENGTH); // Client's name

    p->last_played = -1;  // Give the default value -1 
    p->buf = malloc(MAX_LENGTH);
    p->pkmn = malloc(sizeof (struct pokemon)); // Keep the pokemon field ready for battle
    p->inbuf = 0;

    p->next = top;
    top = p;

    p->l_next = NULL; // not enqueued to the lobby yet

    return top;
}

static struct client *removeclient(struct client *top, int fd) {
    struct client **p;

    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next
)        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
    return top;
}

/* 
    Broadcast to everyone the message s. Don't broadcast to yourself (c).
*/
static void broadcast(struct client *top, char *s, int size, struct client *c) {
    struct client *p;
    for (p = top; p; p = p->next) {
        if (c && p->fd != c->fd)     // don't broadcast to self. make sure c exists
            write(p->fd, s, size);
    }
    /* should probably check write() return value and perhaps remove client */
}


