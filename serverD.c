/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define UDP "24882" // the port client will be connecting to 
#define CLIENT_PORT "25882"
#define MAXBUFLEN 100 // max number of bytes we can get at once 
// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])
{

//-------------------------------------------PHASE 1----------------------------------------------------
    if(argc < 2)
    {
	printf("%s", "ERROR: Usage= ./server.out file_name.txt\n");
	return 0;
    }

    //phase 1
    char server_name;
    FILE *server_file = fopen(argv[1], "r");

    if(strchr(argv[1], 'A') != NULL)
	server_name = 'A';
    else if (strchr(argv[1], 'B') != NULL)
	server_name = 'B';
    else if (strchr(argv[1], 'C') != NULL)
	server_name = 'C';
    else if (strchr(argv[1], 'D') != NULL)
	server_name = 'D';

    int packet_idx = 0;
    char packet_info[8];	//this shit is nasty

    if(server_file) 
    {
	printf("The Server "); printf("%c" , server_name); printf(" is up and running.\n"); 
    }
    else
    {
	printf("%s", "ERROR: file input eror\n");
	return 0;
    }
    packet_info[packet_idx] = server_name;
    packet_idx++;
    char line[256];
    char *temp = NULL;
    char neighbor_name;
    char *a = "A";
    char *b = "B";
    char *c = "C";
    char *d = "D";
    int link_cost_array[5];
    int idx = 0;
    char server_name_array[5];
    

    if(server_file == NULL)
	exit(EXIT_FAILURE);

    while(fgets(line, sizeof(line), server_file))
    {
	temp = strtok(line, " \tr");
	temp = strtok(NULL, " \tr");
	temp = strtok(NULL, " \tr");

	if(strcmp(temp, a ) == 0)
		neighbor_name = 'A'; 
	else if (strcmp(temp, b ) == 0)
		neighbor_name = 'B'; 
	else if (strcmp(temp, c ) == 0)
		neighbor_name = 'C'; 
	else if (strcmp(temp, d ) == 0)
		neighbor_name = 'D'; 

	server_name_array[idx] = neighbor_name;

	//to convert char -> int, char - '0'
	packet_info[packet_idx] = neighbor_name;
	packet_idx++;

	while( temp != NULL)
	{
		temp = strtok(NULL, " \t");

		if(temp != NULL)
		{
			int temp1 = atoi(temp);
			link_cost_array[idx] = temp1;	
			//to convert int -> char, int + '0';
			packet_info[packet_idx] = temp1 + '0';

			packet_idx++;	
		}
		
	}
	idx++;
    }
    fclose(server_file);
   
    packet_info[packet_idx] = '\0';
    char* packet_string = &packet_info[0];

    int i;
    printf("The Server %c" , server_name); printf(" has the following neighbor information:\n\n");
    printf("%s", "Neighbor------Cost\n");
    for(i = 0; i < idx; i++)
    {
	printf("server%c " , server_name_array[i]); printf("\t"); printf("%d", link_cost_array[i]);  printf("\n");
    }
   

//-------------------------------------------PHASE 2----------------------------------------------------
//creation of socket and networking drived from beej "client" example
    int sockfd, numbytes;  
    //char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    if (argc != 2) {
        fprintf(stderr,"usage: client textfile\n");
	//fprintf(stderr,"usage: client localhost\n");
        exit(1);
    }

    //find our own IP addrses
    struct hostent *he_client;
    struct in_addr **addr_list;
    char* localhost = "localhost";
    if((he_client = gethostbyname(localhost)) == NULL) {herror("gethostbyname"); return 2;}
    addr_list = (struct in_addr **)he_client->h_addr_list;
    int ii;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(localhost, CLIENT_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Server: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "Server: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
 
    freeaddrinfo(servinfo); // all done with this structure
    if (send(sockfd, packet_string, packet_idx, 0) == -1) {
	perror("send");
        exit(1);
    }

    printf("\nThe Server %c" , server_name); printf(" finishes sending its neighbor information to the Client with TCP port number ");
    printf("%s", CLIENT_PORT); printf(" and IP address %s \n" , s);
 
    printf("\nFor this connection with the Client, the Server %c", server_name); printf(" has TCP port number ");
    printf("%d and IP address ", sockfd);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\n\n");
    close(sockfd);

//------------------------------UDP PACKET NOW -------------------------------------------------------
// UDP listener code drived from beej datagram example
    int sock_udp; //sockfd
    struct addrinfo hints_, *udpinfo, *u;	//*servinfo, *p
    int rcv;			//rv
    int num_udp_bytes;		//numbytes
    struct sockaddr_storage client_addr;		//their_addr
    char buf[MAXBUFLEN];			//buf
    socklen_t addr_len;
    char q[INET6_ADDRSTRLEN];			//s[]
    memset(&hints_, 0, sizeof hints_);
    hints_.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints_.ai_socktype = SOCK_DGRAM;
    hints_.ai_flags = AI_PASSIVE; // use my IP
    if ((rcv = getaddrinfo(NULL, UDP, &hints_, &udpinfo)) != 0) {	//was MYPORT
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rcv));
    	return 1;
    }

    // loop through all the results and bind to the first we can
    for(u = udpinfo; u != NULL; u = u->ai_next) {
 	if ((sock_udp = socket(u->ai_family, u->ai_socktype,
 		u->ai_protocol)) == -1) {
 		perror("UDP Listener: socket");
 		continue;
 		}
    	if (bind(sock_udp, u->ai_addr, u->ai_addrlen) == -1) {
 		close(sock_udp);
 		perror("UDP Listener: bind");
 		continue;
   	 }
    break;
    }
    if (u == NULL) {
     	fprintf(stderr, "UDP Listener: failed to bind socket\n");
     	return 2;
    }

    freeaddrinfo(udpinfo);
    printf("\nWaiting to receive info from client...\n");

    addr_len = sizeof client_addr;
    if ((num_udp_bytes = recvfrom(sock_udp, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
 	perror("recvfrom");
	exit(1);
    }
    printf("\nThe server D has received the network toplogy from the Client with UDP port number %d and IP address %s as follows:\n\n",
	 sock_udp,
    inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), q, sizeof q));
    buf[num_udp_bytes] = '\0';
    close(sock_udp);
    int count;

    //output toplogy from client
    printf("Edge------Cost\n");  
    for(count = 0; count < strlen(buf); count += 3)
    {
	printf(" %c%c\t  %d\n", buf[count], buf[count+1], buf[count+2] - '0');
    }
    printf("\nFor this connection with Client, The Server D has UDP port number %s and IP address ", UDP);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\n\n");

    return 0;
}
