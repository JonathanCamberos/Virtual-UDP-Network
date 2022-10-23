/* $Id: ls.h,v 1.1 2000/02/23 01:00:30 bobby Exp $
 * Link Set
 */

#ifndef _LS_C_
#define _LS_C_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"
#include "ls.h"
#include "queue.h"
#include "n2h.h"
#include "rt.h"
#include <sys/socket.h> //for socket() and bind()
#include <arpa/inet.h> //for sockaddr_in and inet_ntoa()


struct link *g_ls;

//added routing table global
//struct 


//struct link
//{
//    struct link *next; // next entry
//    struct link *prev; // prev entry
//    node peer;         // link peer(the node you will be sending to)
//    struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

//    int host_port, peer_port;
//    int sockfd; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
//    cost c;     // cost
//    char *name;
//};

static node g_host;
//typedef unsigned int node;


int create_ls()
{
	//creating structure
	//static struct link *g_ls;
	//struct link *g_ls;
	InitDQ(g_ls, struct link);
	//#define InitDQ(head, type)                   
    //{                                        
    //    head = (type *)malloc(sizeof(type)); 
    //    head->next = head->prev = head;      
    //}

	//struct link
	//{
    //	struct link *next; // next entry
    //	struct link *prev; // prev entry
    //	node peer;         // link peers
    //	int host_port, peer_port;
    //	int sockfd; // underlying socket for the link. Expected to be bound to link.host_port
    //	cost c;     // cost
    //	char *name;
	//};

	assert(g_ls);

	g_ls->peer = g_ls->c = -1;
	g_ls->name = 0x0;

	//static node g_host;
	//file n2h.c, returns id
	g_host = get_myid();

	//0x0 is the same as '0' in c (i think)  
	return (g_ls != 0x0);
}


// returns a socket descriptor that is bound on the port that current node will listen to
// return a negative value on error
// TODO: implement this!

//called by --> create_link_sock(nl->host_port);
//EDITED Code Tree
//READY?
int create_link_sock(int port)
{
	// UNUSED() is here to suppress unused variable warnings,
	// remove this and use this variable!
	//UNUSED(port);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in currSockAddr;

	char *hostName = gethostbynode(get_myid());

	memset(&currSockAddr, 0, sizeof(currSockAddr));
	currSockAddr.sin_family = AF_INET;
	currSockAddr.sin_addr.s_addr = INADDR_ANY;
	//currSockAddr.sin_addr = getaddrbyhost(hostName);    DOUBLE CHECK WITH TA

	currSockAddr.sin_port = htons(port);

	int res;

	const int enable = 1;
	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	if(res < 0){
		printf("setsockopt(SO_REUSEADDR) failed");
		exit(1);
	}


	res = bind(sock, (struct sockaddr *)&currSockAddr, sizeof(currSockAddr));

	if(res < 0){
		perror("error: bind() failed 2 ````````````````````````````````````````````\n");
		printf("For PORT %d\n", port);
		return -1;
	}
		
	return sock;

	// HINT: port numbers <1024 are reserved
	// when we grade your assignment, we will only use port numbers >1024
	// if you want to use a port number <1024, you will need to run your program as root
	//OFFICE HOURS ^^^^^
	// if you are simply testing your code, then simply modify the config file to use port numbers >=1024
}

// add a link to the global link set
//add_link_if_local(ev->peer0, ev->port0, ev->peer1, ev->port1,
//						  ev->cost, ev->name);
//called from ---> curr file add_link_if_local(node peer0, int port0, node peer1, int port1, cost c, char *name)
//add_link(host_port, peer, peer_port, c, name);
//EDITED Code Tree
//int add_link(int host_port, node peer, int peer_port, cost c, char *name)  //-- BEFORE
int add_link(int host_port, node peer, int peer_port, cost c, char *name) 
{
	//creating address struct
	struct in_addr peer_addr;
	memset(&peer_addr, 0, sizeof(peer_addr));

	//get node name
	//file: n2h.c
	const char* peer_hostname = gethostbynode(peer);
	//SCRIBE - if int 2 does not have a corresponding name - frostbolt return error
	if(peer_hostname == NULL){
		return -1;
	}

	//get address
	//file: n2h.c
	//function returns a - struct in_addr
	//SCRIBE - if frostbolt for example does not have an IP address, return error
	peer_addr = getaddrbyhost(peer_hostname);
	if(peer_addr.s_addr == 0){
		return -1;
	}

	//mallocing memory for new link, to be put into global link set
	struct link *nl = (struct link *)malloc(sizeof(struct link));
	if (!nl)
	{
		return ENOMEM;
	}
	memset(nl, 0, sizeof(*nl));
	//struct link
	//{
	//    struct link *next; // next entry
	//    struct link *prev; // prev entry
	//    node peer;         // link peer(the node you will be sending to)
	//    struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

	//    int host_port, peer_port;
	//    int sockfd; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
	//    cost c;     // cost
	//    char *name;
	//};

	//assinging values
	nl->host_port = host_port;
	nl->peer = peer;  //node for unique integer, (is actually a file descriptor)

	nl->peer_addr.sin_family = AF_INET;
	nl->peer_addr.sin_port = htons(peer_port);
	nl->peer_addr.sin_addr = peer_addr; //copied from getaddrbyhost(peer_hostname);

	nl->peer_port = peer_port;
	nl->c = c; //cost
	nl->name = (char *)malloc(strlen(name) + 1); //string name
	if (!(nl->name))
	{
		free(nl);
		return ENOMEM;
	}
	strcpy(nl->name, name);

	//file: curr file
	int rv = create_link_sock(nl->host_port);  //rv is -1 on error
	if (rv < 0)
	{
		//if port fails, free mallocs information (string for name)
		free(nl->name);
		free(nl);
		return rv;   //rv is -1, therefore return -1
	}

	//assign socket descriptor
	nl->sockfd = rv;

	//file: queue.h
	InsertDQ(g_ls, nl); //adds it as second element in list, pushes the rest foward

	//add_rte(host, c, peer); // -- ADDED
	update_rte(peer, c, peer); // -- ADDED

	//update_rte(node n, cost c, node nh)
	//#define InsertDQ(pos, element)     
    //{                              
    //    element->next = pos->next; 
    //    pos->next->prev = element; 
    //    pos->next = element;       
    //    element->prev = pos;       
    //}

	//struct link
	//{
	//    struct link *next; // next entry
	//    struct link *prev; // prev entry
	//    node peer;         // link peer(the node you will be sending to)
	//    struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

	//    int host_port, peer_port;
	//    int 	; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
	//    cost c;     // cost
	//    char *name;
	//};

	return 1;  //on success
}



// check if current host is participating in this link
// if so, call `add_link()` to add it to the global link set
// returns 0 if irrelevant, 1 if link was added, < 0 if error
//called from ----> dv.c dispatch_single_event(struct es *ev)
//EDITED Code Tree
int add_link_if_local(node peer0, int port0, node peer1, int port1, cost c, char *name)
{
	//ev
	//struct es   ---> ev
	//{
  	//	struct es *next;
  	//	struct es *prev;
	//	e_type ev_ty;
  	//	int peer0, port0, peer1, port1;
  	//	int cost;
  	//	char *name;
	//};

	//typedef unsigned int node; 

	node host = get_myid(); //curr node running
	int host_port = -1;
	int peer_port = -1;
	int peer = -1;
	if (peer0 == host)
	{
		host_port = port0;
		peer_port = port1;
		peer = peer1;
	}
	else if (peer1 == host)
	{
		host_port = port1;
		peer_port = port0;
		peer = peer0;
	}
	else
	{
		//remember, some of the events in the curr event list, may not pertain to this curr node
		//therefore we must ignore
		return 0; // This link is not relevant
	}

	//creating link here!!!
	//file: curr file
	//EDITED Code Tree
	return add_link(host_port, peer, peer_port, c, name); //returns 1 on successful add
}

// update cost of a link
// returns 0 if link was not found in my set (i.e. updated link is irrelevant)
// returns 1 if link was found and updated
//EDITED Code Tree
int ud_link(char *n, int cost)
{
	
	//file: curr file
	struct link *i = find_link(n);
	//struct link
	//{
	//    struct link *next; // next entry
	//    struct link *prev; // prev entry
	//    node peer;         // link peer(the node you will be sending to)
	//    struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

	//    int host_port, peer_port;
	//    int sockfd; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
	//    cost c;     // cost
	//    char *name;
	//};
	if (!i)
	{
		return 0;
	}

	i->c = cost;
	
	return 1;
}

//returns link pointer, else null
struct link *find_link(char *n)
{

	printf("find link 1\n");

	struct link *i;
	for (i = g_ls->next; i != g_ls; i = i->next)
	{
		printf("find link 1.1 found\n");
		if (!(strcmp(i->name, n)))
		{
			printf("find link 1.1 found\n");
			break;
		}
	}
	if (!strcmp(i->name, n))
	{
		return i;
	}
	else
	{
		printf("find link 1.1 NOT FOUND\n");
		return 0x0;
	}
}

// delete a link from the global link set
// return 0 if link was not found (i.e. deleted link is irrelevant)
// return 1 if link was found and deleted
int del_link(char *name)
{
	struct link *i = find_link(name);
	//struct link
	//{
	//    struct link *next; // next entry
	//    struct link *prev; // prev entry
	//    node peer;         // link peer(the node you will be sending to)
	//    struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

	//    int host_port, peer_port;
	//    int sockfd; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
	//    cost c;     // cost
	//    char *name;
	//};
	if (!i)
	{
		return 0;
	}
	if (i->sockfd >= 0)
	{
		close(i->sockfd);
	}

	printf("BEFORE SEGFAULT 1\n");
	
	DelDQ(i);

	printf("BEFORE SEGFAULT 2\n");
	//#define DelDQ(pos)                   
    //{                                
    //    pos->prev->next = pos->next; 
    //    pos->next->prev = pos->prev; 
    //}
	free(i->name);
	free(i);

	printf("BEFORE SEGFAULT 3\n");
	return 1;
}

void print_link(struct link *i)
{
	//struct link
	//{
    //	struct link *next; // next entry
    //	struct link *prev; // prev entry
    //	node peer;         // link peer(the node you will be sending to)
    //	struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

    //	int host_port, peer_port;
    //	int sockfd; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
    //	cost c;     // cost
    //	char *name;
	//};

	fprintf(stdout, "[ls]\t ----- link name(%s) ----- \n", i->name);
	fprintf(stdout, "[ls]\t node(%d)host(%s)port(%d) <--> node(%d)host(%s)port(%d)\n",
			g_host, gethostbynode(g_host), i->host_port,
			i->peer, gethostbynode(i->peer), i->peer_port);
	fprintf(stdout, "[ls]\t cost(%d), sock(%d)\n",
			i->c, i->sockfd);
}

void print_ls()
{
	struct link *i;
	//struct link
	//{
    //	struct link *next; // next entry
    //	struct link *prev; // prev entry
    //	node peer;         // link peer(the node you will be sending to)
    //	struct sockaddr_in peer_addr; //peer addr and port. This will be used with sendto

    //	int host_port, peer_port;
    //	int sockfd; // underlying socket for the link. Expected to be bound to link.host_port. Used for both sending and receiving
    //	cost c;     // cost
    //	char *name;
	//};

	fprintf(stdout, "\n[ls] ***** dumping link set *****\n");
	for (i = g_ls->next; i != g_ls; i = i->next)
	{
		assert(i);
		print_link(i);
	}
}

#endif
