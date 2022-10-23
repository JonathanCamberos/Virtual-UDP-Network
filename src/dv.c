#include <assert.h>
#include <stdio.h>
#include <time.h> //for timespec
#include <sys/poll.h> //for poll stuff??
#include <stdlib.h> //malloc
#include <string.h>

#include "dv.h"
#include "es.h"
#include "ls.h"
#include "rt.h"
#include "n2h.h"

// global variables
// you may want to take a look at each header files (es.h, ls.h, rt.h)
// to learn about what information and helper functions are already available to you
extern struct el *g_lst;  // 2-D linked list of parsed event config file
extern struct link *g_ls; // current host's link state storage
extern struct rte *g_rt;  // current host's routing table

typedef struct innerUpdate {
	uint16_t destination; //2 bytes
	uint16_t cost; //2 bytes
} innerUpdate;

typedef struct periodicUpdate{
	uint8_t type; //1 bytes
	uint8_t version; //1 byte
	uint16_t numUpdates;  //2 bytes
	innerUpdate updateArr[100];  //4 * x bytes
} periodicUpdate; //404



// this function is our "entrypoint" to processing "list of [event set]s"
//based on events from file: config9
//EDITED Code Tree
void walk_event_set_list(int pupdate_interval, int evset_interval, int verbose)
{

	//printf("\n\nINSIDE WALK ************************\n\n");
	//file: es.h
	struct el *es; // event set, as an element of the global list
	//struct el
    //{
    //   struct el *next;
    //   struct el *prev;
    //   struct es *es_head;
    //};


	//struct es
	//{
  	//	struct es *next;
  	//	struct es *prev;
	//  e_type ev_ty;
  	//	int peer0, port0, peer1, port1;
  	//	int cost;
  	//	char *name;
	//};



	// TODO: UNUSED() is here to suppress unused variable warnings,
	// remove this and use this variable!
	UNUSED(verbose);

	assert(g_lst->next);

	//printf("\n^^^^^^^^^^^^^^^^^ el ^^^^^^^^^^^^^^^^^\n");

	//file: es.c
	//print out the whole event LIST
	//print_el();

	//printf("\n^^^^^^^^^^^^^^^^^ el ^^^^^^^^^^^^^^^^^\n\n");

	int count = 0;
	// for each [event set] in global parsed 2-d list
	for (es = g_lst->next; es != g_lst; es = es->next)
	{
		count++;
		//file: curr file
		//EDITED Code Tree
		process_event_set(es);

		// HINT: this function should block for evset_interval seconds
		dv_process_updates(pupdate_interval, evset_interval);  //run this 

		//printf("\n\n[es] >>>>>>> Start dumping data stuctures <<<<<<<<<<<\n");
		
		//printf("Results");
		
		//printf("\n############### n2h #############\n");
		//file: n2h.c
		print_n2h();
		
		//printf("\n@@@@@@@@@@@@@@@@ ls @@@@@@@@@@@@@\n");
		//file: ls.c
		print_ls();
		
		//printf("\n*************** Rt **************\n");
		//file: rt.c
		print_rt();
	}

	//printf("Number of count: %d\n", count);

	// now all event sets have been processed
	// continue running, so "remaining" updates on the network can be processed (i.e. count to infinity)
	// TODO: uncomment line below, and modify dv_process_updates() to loop forever when evset_interval is 0
	// NOTE: I would advise you to make this modification last, after you have everything else working 
	// dv_process_updates(pupdate_interval, 0);
}

// iterate through individual "event" in single [event set]
// and dispatch each event using `dispatch_single_event()`
// you wouldn't need to modify this function, but make sure you understand what it's doing!
void process_event_set(struct el *es)
{
	struct es *ev_set; // event set, as a list of single events(1-D Array)
	struct es *ev;	   // single event
	//struct es   ---> ev
	//{
  	//	struct es *next;
  	//	struct es *prev;
	//	e_type ev_ty;
  	//	int peer0, port0, peer1, port1;
  	//	int cost;
  	//	char *name;
	//};


	//printf("\n!!!!!!!!!!! Inside Process Event Set !!!!!!!!!!\n\n");

	assert(es);

	// get actual event set's head from the element `struct el`
	ev_set = es->es_head;
	assert(ev_set);

	printf("[es] >>>>>>>>>> Dispatch next event set <<<<<<<<<<<<<\n");
	// for each "event"s in [event set] --> ev_set
	for (ev = ev_set->next; ev != ev_set; ev = ev->next)
	{
		printf("[es] Dispatching next event ... \n");
		//file: curr file
		//EDITED code Tree
		dispatch_single_event(ev);
	}
}

// dispatch a event, update data structures, and
// TODO: send link updates to current host's direct neighbors
//EDITED Code Tree
void dispatch_single_event(struct es *ev)
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
	
	//printf("00000  Inside Dispatch Single Event\n");

	assert(ev);

	// for each event types (establish / update / teardown), you should:
	// detect if this event is relevant to current host (check doc comments of each functions that update the data structures)
	// if yes, propagate updates to your direct neighbors
	// you might want to add your own helper that handles sending to neighbors

	
	//Sending the first trigger updates after the first routing table updates

	node hold0 = ev->peer0;
	node hold1 = ev->peer1;
	node host = get_myid();
	int res;

	//printf("Current 0: %d, Current 1: %d\n", hold0, hold1);

	switch (ev->ev_ty)
	{
	case _es_link:
		//printf("Inside Add\n");

		//file: ls.c
		//EDITED Code Tree    -- added to add to global routing table on add of link
		add_link_if_local(ev->peer0, ev->port0, ev->peer1, ev->port1,
						  ev->cost, ev->name); //returns 1 on success

		// ADDED
		if (hold0 == host)
		{
			res = update_rte(hold1, ev->cost, hold1);
			if(res == 0){
				//printf("add success -1\n");
			}else{
				//printf("add fail -1\n");
			}
		}
		else if (hold1 == host)
		{
			res = update_rte(hold0, ev->cost, hold0);
			if(res == 0){
				//printf("add success 0\n");
			}else{
				//printf("add fail 0\n");
			}
		}else{
			//printf("weird -1/0\n");
		}
		// ADDED
		
		break;
	case _ud_link:
		//printf("Inside Update\n");

		//ADDED
		struct link *y = find_link(ev->name);
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
		hold0 = y->peer;
		
		res = update_rte(hold0, ev->cost, hold0);
		if(res == 0){
			//printf("add success 1\n");
		}else{
			//printf("add fail 1\n");
		}
		// ADDED

		//file: ls.c
		//EDITED Code Tree    -- added to update global routing table on update of link
		//ud_link(ev->name, ev->cost);
		ud_link(ev->name, ev->cost);

		break;

	case _td_link:
		//printf("Inside Tear\n");

		// ADDED
		struct link *x = find_link(ev->name);
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

		hold0 = x->peer;
		
		res = update_rte(hold0, 1026, hold0); //all test cases will be < 1024
		if(res == 0){
			//printf("add success 3\n");
		}else{
			//printf("add fail 3\n");
		}
		// ADDED

		//file: ls.c
		//EDITED Code Tree    -- added to update global routing table on teardown of link
		del_link(ev->name);	
		
		break;
	default:
		//boorken base :)
		//printf("[es]\t\tUnknown event!\n");
		break;
	}
}

void sendToListOfNeighbors(struct sockaddr_in neighborAddr[], struct pollfd neighborFD[], uint16_t totalNeigh, periodicUpdate currPeriodicUpdate, uint16_t expectedSend){
	

	int pollRet = 0;

	//for each neighbor
	//int check = 0;	
	
	for(int i = 0; i < totalNeigh; i++){
		int bytesSent = 0;
		pollRet = poll(&neighborFD[i], 1, 1);

		if(pollRet == -1){
			//printf("VAMOSVAMOSVAMOSVAMOS\n");
			continue;
		}else if(pollRet == 0){
			//printf("LETSGOOLETSGOOLETSGOOLETSGOO\n");
		}else if(pollRet == 1){
			//connection made!
			//send buffer of update

			//printf("Connection with socketNum/file descriptor: %d in index: %d\n", neighborFD[i].fd, i);
			
			if(neighborFD[i].fd & POLLIN){
				//printf("BYEEE\n");
				//printf("Sending to socket: %d\n", neighborFD[i].fd);
				//printf("##############################\n");

				//bytesSent = send(neighborFD[i].fd, &sendBuf, expectedSend, 0);
				//ssize_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
				bytesSent = sendto(neighborFD[i].fd, &currPeriodicUpdate, expectedSend, 0, (struct sockaddr *)&neighborAddr[i], sizeof(neighborAddr[i]));

				if(0 < bytesSent){
					//printf("GOD IS REAL &&&&&&&&&&&&&&&&&&&&&&&&\n");
					//printf("Number of bytes sent: %d\n", bytesSent);
				}else{
					//printf("Suffer child\n");
				}

			}else{
				//printf("NOT READYYYYY\n");
			}


		}

	}
	
	

}

void sendTriggerUpdate(node finalDestination, cost newTotalCost){

	//checkNewRoutes.destination is final destination
	//checkNewRoutes

	int test = 1;
	int littleEndian = 0;
	if(*(char *)&test == 1){
		//printf("Little Endian\n");
		littleEndian = 1;
	}

	//printf("TRIGGER - INSIDE SEND TRIGGER UPDATE\n");


	//Step 1: Getting File Descriptor Information

	//getting addresses
	struct sockaddr_in neighborAddr[255];

	//pollfd requirements, for all possible neighbors
	struct pollfd neighborFD[255];
	//struct pollfd {
    //    int fd;
    //    short events;
    //    short revents;
    //};

	uint16_t totalNeigh = 0;
		
	//Step 1: Adding all direct neighbors to pollfd list, (nodes on the link set)
	for (struct link *ls = g_ls->next; ls != g_ls; ls = ls->next) {
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

		//sets correct on for the filedescriptor
		neighborFD[totalNeigh].fd = ls->sockfd;
		printf("TRIGGER - Found Neighbor: %d on socket: %d/%d for index: %d\n", ls->peer, ls->sockfd, neighborFD[totalNeigh].fd, totalNeigh);
		neighborFD[totalNeigh].events = POLLOUT;
		neighborAddr[totalNeigh] = ls->peer_addr;

		totalNeigh++; //moves index up by 1
	}


	//3 important info right now
	//neighborAddr[255];
	//neighborFD[255];
	//totalNeigh
	

	//Step 2: Finish making the Route Dissemination Packet
	//copy every routing entry

	periodicUpdate currPeriodicUpdate;

	//NO ENDIANES FOR 8 BITS
	currPeriodicUpdate.type = 7;
	currPeriodicUpdate.version = 1;
	
	printf("TRIGGER - type value ------------------ %u ---------------\n", currPeriodicUpdate.type);
	printf("TRIGGER - version value ------------------ %u ---------------\n", currPeriodicUpdate.version);

	print_bytes(&currPeriodicUpdate.type, 1);
	print_bytes(&currPeriodicUpdate.version, 1);
	printf("\n\n");

	printf("TRIGGER - neighbor value ------------------ %u ---------------\n", totalNeigh);
	
	

	//currRow = 4;
	uint16_t routeTableRow = 0;

	printf("TRIGGER - Parsing Single Trigger Update: \n");

	if(littleEndian == 1){ 
		//currPeriodicUpdate.numUpdates = htobe16(totalNeigh);
		currPeriodicUpdate.updateArr[routeTableRow].destination = htobe16(*(int16_t *)&finalDestination);
		currPeriodicUpdate.updateArr[routeTableRow].cost = htobe16(*(int16_t *)&newTotalCost);

	}else{
		//printf("Big Engian\n");
		currPeriodicUpdate.updateArr[routeTableRow].destination = *(int16_t *)&finalDestination;
		currPeriodicUpdate.updateArr[routeTableRow].cost = *(int16_t *)&newTotalCost;
	}

	//currRow += 4; //increases row
	routeTableRow++;
	

	//all neighbors
	for(int k = 0; k < totalNeigh; k++){
		printf("TRIGGER - file descriptor: %d, at index %d\n", neighborFD[k].fd, k);
	}

	//total rows in route table
	for(int x = 0; x < routeTableRow; x++){
		printf("TRIGGER - package destination: %d, cost of %d, at index %d\n", currPeriodicUpdate.updateArr[x].destination, currPeriodicUpdate.updateArr[x].cost, x);

	}

	if(littleEndian == 1){ 
		
		printf("TRIGGER - Little Endian\n");
		//*(int16_t *)&sendBuf[2] = htobe16(totalNeigh);  //double check tmr!!!
		currPeriodicUpdate.numUpdates = htobe16(1);

	}else{
		printf("TRIGGER - Big Engian\n");
		//sendBuf[2] = totalNeigh;
		currPeriodicUpdate.numUpdates = 1;
	}


	//working in bytes
	uint16_t expectedSendBytes = (routeTableRow+1) * 4;

	printf("TRIGGER - Size of Total Package: ----------------- %u -----------------\n", expectedSendBytes);
	//printf("Expected Size of Total Routing Table Package: %u\n", expectedSend);

	sendToListOfNeighbors(neighborAddr, neighborFD, totalNeigh, currPeriodicUpdate, expectedSendBytes);
	
}


void checkIfNeedUpdate(innerUpdate checkNewRoutes[], uint16_t recvNumUpdates, node peerIDInOrder[], int peerSenderIndex){

	printf("CHECK IF NEED - ---------- Inside Check If Need Update -----------\n");

	//check entire routing table
	struct rte *currRouteInCurrTable;
	//struct rte
	//{
    //	struct rte *next; // next entry
    //	struct rte *prev; // prev entry
    //	node d;           // dest
    //	cost c;           // cost
    //	node nh;          // next hop
	//};

	struct rte *neigh;

	printf("\n CHECK IF NEED -  @@@@@@@@@@@@@@@@@@ Analyzing package from id: %d @@@@@@@@@@@@@@@@@@@@@\n", peerIDInOrder[peerSenderIndex]);
	

	node myId = get_myid();
	
	//for every route update in recieved package
	for(int k = 0; k < recvNumUpdates; k++){

		printf("CHECK IF NEED ---------- Curr final Destination %u with cost of %u --------------\n", checkNewRoutes[k].destination, checkNewRoutes[k].cost);

		//check if current final destination is self node



		if(checkNewRoutes[k].destination == myId){     
			//this case is still valid could be letting you know that link you node running code has broken (1026 --> infinity) ???
			//dont point to ya self dummy				//TA OFFICE houRs ********************************************************************
			printf("CHECK IF NEED - !!!!!!!!!!!!!!!!!! pointing to self stupid idiot !!!!!!!!!!!!!\n");

			//cases to be sure



		}else{
			
			//final destination is not self node (node curr running code)
			
			//check if a routing entry exists inside self node rt to current final destination
			currRouteInCurrTable = find_rte(checkNewRoutes[k].destination);			

			//same code from update_rte
			if(currRouteInCurrTable == 0){

				//FINAL DESTINATION DOES NOT EXIST, in self node rt
				//peerIDInOrder[peerSenderIndex] sent current package
				//printf("CHECK IF NEED - ##########  UPDATEEEEEEEEEE  FROMM %d ############################ run number %d\n", peerIDInOrder[peerSenderIndex], k);

				//check update packet cost

				if(checkNewRoutes[k].cost == 1026){
					//neighbor does not have an entry either, (infinity)
					//update rt

					cost emptyCost = 1026;
					add_rte(checkNewRoutes[k].destination, emptyCost, checkNewRoutes[k].destination);   
					print_rt();
					printf("CHECK IF NEED - empty empty empty\n empty empty empty empty\n");   //we set to 1026


				}else{
					//neighbor does have an entry, (non-infinity)

					//check price to jump to neighbor
					neigh = find_rte(peerIDInOrder[peerSenderIndex]);
				
					if(neigh == 0){

						printf("CHECK IF NEED - ERRRRRRR2222\n");

					}else{
					
						//link exists to neighbor! add prices!
						cost newTotalCost = checkNewRoutes[k].cost + neigh->c;

						//automatically add because there is no current rte for final destination
						add_rte(checkNewRoutes[k].destination, newTotalCost, peerIDInOrder[peerSenderIndex]);
						print_rt();
						printf("CHECK IF NEED - eeeeeeeee\neeeeeeeeeeeeeee\n");
						sendTriggerUpdate(checkNewRoutes[k].destination, newTotalCost);
					
					}		



				}


				
			}else{

				//currRouteInCurrTable is accessible!!

				//FINAL DESTINATION DOES EXIST in self node
				//in node running the code, peerIDInOrder[peerSenderIndex] sent current package
				printf("CHECK IF NEED - kkkkkkkkkkkkkkkk   UPDATEEEEEEEEEE  FROMM %d kkkkkkkkkkkkkkkkkk %d\n", peerIDInOrder[peerSenderIndex], k);

				//check update packet cost

				if(checkNewRoutes[k].cost == 1026){
					//neighbor does not have an entry, (infinity)
					
					//check if this breaks your route

					if(currRouteInCurrTable->nh == peerIDInOrder[peerSenderIndex]){
						//curr node running code expected to jump to this neighbor in order to get to final destination
						//however, neighbor updated saying it can no longer get to the final destination
						//update routing table

						cost newCost = 1026;
						update_rte(checkNewRoutes[k].destination, newCost, peerIDInOrder[peerSenderIndex]);
						print_rt();

						printf("CHECK IF NEED - stay at current size\n stay at current sizey\n");					//ask if we set to 1026

					}else{

						//nextHop was not neighbor
						//you are safe, for now... muahahaha!!
					}	


				}else{
					
					//neighbor does have an entry, (non infinity)

					//check price to jump to neighbor
					neigh = find_rte(peerIDInOrder[peerSenderIndex]);
				
					if(neigh == 0){

						//printf("CHECK IF NEED - ERRRRRRR\n");

					}else{
					
						//link exists to neighbor! check cost!
						cost newTotalCost = checkNewRoutes[k].cost + neigh->c;

						if(newTotalCost < currRouteInCurrTable->c){  
							//new cost is less!
							//update routing table

							update_rte(checkNewRoutes[k].destination, newTotalCost, peerIDInOrder[peerSenderIndex]);
							print_rt();
							//printf("CHECK IF NEED - 3333333333333\nstay at current sizey\n");
							sendTriggerUpdate(checkNewRoutes[k].destination, newTotalCost);
							
							//triger update time!!!! :))))))))))))))
							//HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE stoped editing //HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE stoped editing
								
						}else{
							
							//new cost was not less!!
							//printf("CHECK IF NEED - 000000000000000000\n00000000000\n0000000000000\n");
						}
					}				
				
				


				}



				
			}

		}



		
		

	}

	//printf("\n TRIGGER - done FINISHED Analyzing package from id: %d done  \n", peerIDInOrder[peerSenderIndex]);

	
}





// this function should execute for `evset_interval` seconds
// it will recv updates from neighbors, update the routing table, and send updates back
// it should also handle sending periodic updates to neighbors
// TODO: implement this function, pseudocode has been provided below for your reference
void dv_process_updates(int pupdate_interval, int evset_interval)
{

	int test = 1;
	int littleEndian = 0;
	if(*(char *)&test == 1){
		//printf("Little Endian\n");
		littleEndian = 1;
	}

	//node host = get_myid(); // 0 will send while 1 will recv

	//printf("Inside of Dv_Process_Updates\n");

	
	
	time_t timeInFunc = 0;
	time_t timePassed = 0;
	time_t lastTimeCheck = time(NULL); //current time


	time_t periodicTimeInFunc = 0;

	int alive = 1;

	int pollRet;
	int totalListenNeigh = 0;
	struct pollfd listenNeighborsFD[255];

	node peerIDInOrder[255];

	periodicUpdate currPeriodicRecv;

	//Step 1: Adding all direct neighbors to pollfd list, (nodes on the link set)
	for (struct link *ls = g_ls->next; ls != g_ls; ls = ls->next) {
		listenNeighborsFD[totalListenNeigh].fd = ls->sockfd;
		//printf("Found Neighbor: %d on socket: %d/%d for index: %d\n", ls->peer, ls->sockfd, listenNeighborsFD[totalListenNeigh].fd, totalListenNeigh);
		listenNeighborsFD[totalListenNeigh].events = POLLIN;
		peerIDInOrder[totalListenNeigh] =  ls->peer; //to tell who is sending this (to determine next hop)

		//printf("peerID --- %d\n", peerIDInOrder[totalListenNeigh]);
		totalListenNeigh++; //moves index up by 1
	}

	print_rt();

	/*Working Before!!!
	NOT IN WHILE LOOP, only send entire routing table ONCE!!
	send update to all neighbords
	node 0 will send, node 1 will recv
	if(perioidcUpdateCheck == 0 && host == 0){
		//updateAllNeighbors();
		send_periodic_updates();
		perioidcUpdateCheck = 1;
	}
	*/

	

		//listen for any updates
		//int bytesRecv = 0;

		struct sockaddr_in currAddr;
		socklen_t currAddr_len = sizeof(currAddr);
		memset(&currAddr, 0, currAddr_len);

		//creating buffer to send
		//uint8_t recvBuf[65536]; //2^16 bytes

		
	while(alive){
		//only the listen should be in a loop
		timePassed = time(NULL) - lastTimeCheck;
		lastTimeCheck += timePassed;
		timeInFunc += timePassed;

		periodicTimeInFunc += timePassed;

		//printf("Curr timeInFunc: %d\n", timeInFunc);
		if(evset_interval < timeInFunc){
			alive = 0;
			break;
		}

		if(pupdate_interval < periodicTimeInFunc){
				
			//printf("\n\n ^^^^^^^^^^^^^^^^^^^^^^^^^^ PERIODIC TIME UPDATE ^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n");

			//send update to all neighbords
			//node 0 will send, node 1 will recv
			//if(host == 0){
			//	printf("\n\n ^^^^^^^^^^^^^^^^^^^^^^^^^^ PERIODIC TIME UPDATE ^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n");
			
			//	send_periodic_updates();
			//}

			send_periodic_updates();
				
			periodicTimeInFunc = 0;
		}
			



			

		//printf("before\n");
       	pollRet = poll(listenNeighborsFD, totalListenNeigh, 1);

			
        
		//printf("After\n");

       	if(pollRet == -1){
           	//printf("poll() failed\n");
        
       	}else if(pollRet == 0){
           	//printf("idle, listening\n");
       
       	}else if(pollRet == 1){
		
				
				
			for (int i = 0; i < totalListenNeigh; i++) {

					
				if (listenNeighborsFD[i].revents & POLLIN) {

					int bytesRecv = 0;

					//printf("\n");
					//printf("Recieved from socket: %d, on index %d\n", listenNeighborsFD[i].fd, i);										
					//send(listenNeighborsFD[i].fd, buf, 4 + 4 * num_routes, 0);

					bytesRecv = recvfrom(listenNeighborsFD[i].fd, &currPeriodicRecv, 404, 0, (struct sockaddr *)&currAddr, &currAddr_len);
						
					//recieved some type of update, read and break down!!! :))
					if(0 < bytesRecv){

							
						int leftOver = bytesRecv % 4;
						int cleanRecv = bytesRecv-leftOver;

						//printf("Recieved number of bytes: -------------- %d -----------------\n", bytesRecv);
						//printf("GOD IS REAL ALSO HERE!!!!!\n");
						//printf("Clean bytes recved: %d\n", cleanRecv);

						//typedef struct innerUpdate {
						//	uint16_t destination;
						//	uint16_t cost;
						//} innerUpdate;

						//typedef struct periodicUpdate{
						//	uint8_t type;
						//	uint8_t version;
						//	uint16_t numUpdates;
						//	innerUpdate updateArr[2047];
						//} periodicUpdate;

						uint8_t recvType =  currPeriodicRecv.type;
						uint8_t recvVersion =  currPeriodicRecv.version;
						uint16_t recvNumUpdates;

						//printf("\n");
						//print_bytes(&currPeriodicRecv.type, 1);
						//print_bytes(&currPeriodicRecv.version, 1);
						//printf("\n");
														
						//printf("First ------------- %u -----------------\n", recvType);     
						//printf("Second ------------ %u -----------------\n\n", recvVersion);
						//printf("Atempt 2 --------- Babaoye: %u\n", recvBuf[0]);     


						if(littleEndian == 1){
							//printf("Little Endian\n");
							//recvNumUpdates = be16toh(*(uint16_t *) &recvBuf[2]);
							recvNumUpdates = be16toh(currPeriodicRecv.numUpdates);
						}else{
							//printf("big Endian\n");
							//recvNumUpdates = (*(uint16_t *) &recvBuf[2]);
							recvNumUpdates = currPeriodicRecv.numUpdates;
						}

						//printf("recvBuf: ");
						//print_bytes(&currPeriodicRecv.numUpdates, 2);
						//printf("holder: ");
						//print_bytes(&recvNumUpdates, 2);
						//printf("\n");

						//printf("Third uin16_t ---------------- %u ----------------\n", *(int16_t *)&recvNumUpdates);
						//printf("\n\n");

						//int row = 4;
						//uint16_t currDestination;
						//uint16_t currCost;

						innerUpdate checkNewRoutes[recvNumUpdates];

						for(int i = 0; i < recvNumUpdates; i++){

							//checkNewRoutes[i].destination = currPeriodicRecv.updateArr[i].destination;
							//checkNewRoutes[i].cost = currPeriodicRecv.updateArr[i].cost;

							if(littleEndian == 1){
								//printf("Little Endian\n");
								checkNewRoutes[i].destination = be16toh(currPeriodicRecv.updateArr[i].destination);
								checkNewRoutes[i].cost = be16toh(currPeriodicRecv.updateArr[i].cost);
							}else{
								//printf("big Endian\n");
								checkNewRoutes[i].destination = currPeriodicRecv.updateArr[i].destination;
								checkNewRoutes[i].cost = currPeriodicRecv.updateArr[i].cost;
							}
								

							//("Neighbor Destination: %u with cost of %u\n", checkNewRoutes[i].destination, checkNewRoutes[i].cost);
						}


						//Step 4: we have access to all updates from current package (could be periodic or trigger doesnt rlly matter)
						//Update Routing table and send trigger updates as necessary
						checkIfNeedUpdate(checkNewRoutes, recvNumUpdates, peerIDInOrder, i);
							
							

							
							
					}else{
						
					}

						
						
						
					
				}
					


			}
				
				

				

		}
			
	}
		

		


	
			

	// UNUSED() are here to suppress unused variable warnings,
	// remove these and use these variables!
	UNUSED(pupdate_interval);
	UNUSED(evset_interval);

	// you may want to take a timestamp when function is first called (t0),
	// so you know when to break out of this function and continue to next event set

	// loop
	//     determine how long you should wait for (pupdate_interval for first iteration of loop)
	//     use `select()` to block for amount you determined above
	//     if `select()` returns because of timeout
	//         send periodic updates to neighbors using `send_periodic_updates()`
	//     if `select()` returns because of data available
	//         recv updates from neighbors (you may want to write your own recv helper function)
	//         update routing table
	//         send updates to neighbors
	//     determine how long you should wait for `select()` in next iteration of loop
	//     (HINT: which one comes early? next periodic update or next event set execution?)
	//     make sure that the whole function don't block for longer than `evset_interval` seconds!


	

}





// read current host's routing table, and send updates to all neighbors
// TODO: implement this function
// HINT: if you implemented a helper that handles sending to neighbors in `dispatch_single_event()`,
// you can reuse that here!
void send_periodic_updates() {

	int test = 1;
	int littleEndian = 0;
	if(*(char *)&test == 1){
		//printf("Little Endian\n");
		littleEndian = 1;
	}

	//printf("PERIODIC - INSIDE SEND PERIODIC UPDATES\n");


	//Step 1: Getting File Descriptor Information

	//getting addresses
	struct sockaddr_in neighborAddr[255];

	//pollfd requirements, for all possible neighbors
	struct pollfd neighborFD[255];
	//struct pollfd {
    //    int fd;
    //    short events;
    //    short revents;
    //};

	uint16_t totalNeigh = 0;
		
	//Step 1: Adding all direct neighbors to pollfd list, (nodes on the link set)
	for (struct link *ls = g_ls->next; ls != g_ls; ls = ls->next) {
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

		//sets correct on for the filedescriptor
		neighborFD[totalNeigh].fd = ls->sockfd;
		//printf("PERIODIC - Found Neighbor: %d on socket: %d/%d for index: %d\n", ls->peer, ls->sockfd, neighborFD[totalNeigh].fd, totalNeigh);
		neighborFD[totalNeigh].events = POLLOUT;
		neighborAddr[totalNeigh] = ls->peer_addr;

		totalNeigh++; //moves index up by 1
	}


	//3 important info right now
	//neighborAddr[255];
	//neighborFD[255];
	//totalNeigh
	

	//Step 2: Finish making the Route Dissemination Packet
	//copy every routing entry

	periodicUpdate currPeriodicUpdate;

	//NO ENDIANES FOR 8 BITS
	currPeriodicUpdate.type = 7;
	currPeriodicUpdate.version = 1;
	
	//printf("PERIODIC - type value ------------------ %u ---------------\n", currPeriodicUpdate.type);
	//printf("PERIODIC - version value ------------------ %u ---------------\n", currPeriodicUpdate.version);

	//print_bytes(&currPeriodicUpdate.type, 1);
	//print_bytes(&currPeriodicUpdate.version, 1);
	//printf("\n\n");

	//printf("PERIODIC - neighbor value ------------------ %u ---------------\n", totalNeigh);
	
	

	struct rte *i;
	//struct rte
	//{
    //	struct rte *next; // next entry
    //	struct rte *prev; // prev entry
    //	node d;           // dest
    //	cost c;           // cost
    //	node nh;          // next hop
	//};

	//currRow = 4;
	uint16_t routeTableRow = 0;

	//printf("PERIODIC - Parsing entire route table: \n");
	for (i = g_rt->next; i != g_rt; i = i->next)

	{
		//printf("PERIODIC - Destination: %d\n", i->d);
		//printf("PERIODIC -  with curr Cost: %d\n", i->c);
		//for single routing entry i, add to route disemation

		//currPeriodicUpdate.updateArr[routeTableRow].destination = *(int16_t *)&i->d;
		//currPeriodicUpdate.updateArr[routeTableRow].cost = *(int16_t *)&i->c;

		if(littleEndian == 1){ 
			//currPeriodicUpdate.numUpdates = htobe16(totalNeigh);
			currPeriodicUpdate.updateArr[routeTableRow].destination = htobe16(*(int16_t *)&i->d);
			currPeriodicUpdate.updateArr[routeTableRow].cost = htobe16(*(int16_t *)&i->c);

		}else{
			//printf("Big Engian\n");
			currPeriodicUpdate.updateArr[routeTableRow].destination = *(int16_t *)&i->d;
			currPeriodicUpdate.updateArr[routeTableRow].cost = *(int16_t *)&i->c;
		}




		//currRow += 4; //increases row
		routeTableRow++;
	}

	//all neighbors
	for(int k = 0; k < totalNeigh; k++){
		//printf("PERIODIC - file descriptor: %d, at index %d\n", neighborFD[k].fd, k);
	}

	//total rows in route table
	for(int x = 0; x < routeTableRow; x++){
		//printf("PERIODIC - package destination: %u, cost of %u, at index %d\n", *(int16_t *)&currPeriodicUpdate.updateArr[x].destination, *(int16_t *)&currPeriodicUpdate.updateArr[x].cost, x);

	}

	if(littleEndian == 1){ 
		
		//printf("PERIODIC - Little Endian\n");
		//*(int16_t *)&sendBuf[2] = htobe16(totalNeigh);  //double check tmr!!!
		currPeriodicUpdate.numUpdates = htobe16(routeTableRow);

	}else{
		//printf("PERIODIC - Big Engian\n");
		//sendBuf[2] = totalNeigh;
		currPeriodicUpdate.numUpdates = routeTableRow;
	}


	//working in bytes
	uint16_t expectedSendBytes = (routeTableRow+1) * 4;

	//printf("PERIODIC - Size of Total Package: ----------------- %u -----------------\n", expectedSendBytes);
	//printf("Expected Size of Total Routing Table Package: %u\n", expectedSend);

	sendToListOfNeighbors(neighborAddr, neighborFD, totalNeigh, currPeriodicUpdate, expectedSendBytes);
}


void print_bytes(void *ptr, int size) 
{
    unsigned char *p = ptr;
    int i;
    for (i=0; i<size; i++) {
        printf("%02hhX ", p[i]);
    }
    //printf("\n");
}
