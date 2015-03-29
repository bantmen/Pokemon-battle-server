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

// Max length of characters before \r\n
#define MAX_LENGTH 500
#define WELCOME_MESSAGE "Enter your name: "
#define BROADCAST_MESSAGE  " has entered the arena."

struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;
    // Our stuff now
    int last_played;         // fd of the last played player. -1 means NULL
    char name[MAX_LENGTH]; 
    struct pokemon *pokemon; // to be used in battles
};

static struct client *addclient(struct client *top, int fd, struct in_addr addr, const char *name);
static struct client *removeclient(struct client *top, int fd);
static void broadcast(struct client *top, char *s, int size);
//int handleclient(struct client *p, struct client *top);
int handle_input(int fd, char *buf); 
void handle_newclient(int clientfd, struct client *head, char *client_name);
int find_network_newline(char *buf, int inbuf);


    // char *WELCOME_MESSAGE = "Enter your name: ";
    // char *BROADCAST_MESSAGE = " has entered the arena";

int bindandlisten(void);

int main(void) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct client *head = NULL;
    socklen_t len;
    struct sockaddr_in q;
    struct timeval tv;
    fd_set allset;
    fd_set rset;

    int i;

    // Our stuff
    char client_name[MAX_LENGTH/2];

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

        if (FD_ISSET(listenfd, &rset)){
            printf("a new client is connecting\n");
            len = sizeof(q);
            if ((clientfd = accept(listenfd, (struct sockaddr *)&q, &len)) < 0) {
                perror("accept");
                exit(1);
            }

            //login function

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("connection from %s\n", inet_ntoa(q.sin_addr));

            // strncpy(client_name, handle_newclient(clientfd, head), MAX_LENGTH);

            handle_newclient(clientfd, head, client_name);

            head = addclient(head, clientfd, q.sin_addr, client_name); // Modified to get the name

            // Tell that he is waiting
        }

    }
    return 0;
}

// 
int handle_input(int fd, char *buf) {
    int len = read(fd, buf, sizeof(buf) - 1);
    int i = find_network_newline(buf, len); 

    while (i == -1) {
        len += read(fd, &buf[len], sizeof(buf) - 1);
        i = find_network_newline(buf, len); 
    }

    buf[len] = '\0';

    return len;
} 

void handle_newclient(int clientfd, struct client *head, char *client_name) {
    // char client_name[MAX_LENGTH/2];    // temp variable to hold client's name
    char client_message[MAX_LENGTH]; // client_name + BROADCAST_MESSAGE

    write(clientfd, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE)); // Welcome them
    int len = handle_input(clientfd, client_name); // Store their name
    client_name[len-1] = '\0';
    
    memmove(client_message, client_name, strlen(client_name)); 
    memmove(client_message+strlen(client_message), BROADCAST_MESSAGE, strlen(BROADCAST_MESSAGE)+1);   

    broadcast(head, client_message, strlen(client_message)); // Broadcast the new player

    // return client_name;
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

static struct client *addclient(struct client *top, int fd, struct in_addr addr, const char *name) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    strncpy(p->name, name, MAX_LENGTH); // Client's name
    p->pokemon = malloc(sizeof (struct pokemon)); // Keep the pokemon field ready for battle
    p->last_played = -1;  // Give the default value -1 
    
    p->next = top;
    top = p;

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


static void broadcast(struct client *top, char *s, int size) {
    struct client *p;
    for (p = top; p; p = p->next) {
        write(p->fd, s, size);
    }
    /* should probably check write() return value and perhaps remove client */
}

int find_network_newline(char *buf, int inbuf) {
  int i;

  for(i=0;i<inbuf;i++){
    if (buf[i] == '\n') {
        return i;
    }
  }
  return -1; // return the location of '\r' if found
}
