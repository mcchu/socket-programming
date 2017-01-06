/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "25882"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define SERVERA "21882"
#define SERVERB "22882"
#define SERVERC "23882"
#define SERVERD "24882"

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int num_servers = 0;
char adjacency_matrix[4][4] = {{'0','0','0','0'},{'0','0','0','0'},{'0','0','0','0'},{'0','0','0','0'}};


//usage = ./client.out
int main(void)
{
    //phase 2
    //creation of socket and networking drived from beej "server" example

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Client: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Client: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "Client: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    //This segment of code gets our IP address
    struct hostent *he_client;
    struct in_addr **addr_list;
    char* localhost = "localhost";
    if((he_client = gethostbyname(localhost)) == NULL) {herror("gethostbyname"); return 2;}
    addr_list = (struct in_addr **)he_client->h_addr_list;


    char buf[MAXDATASIZE];	
    int numbytes;

    int dest1[2];
    int dest2[2];
    int dest3[2];
    int dest4[2];
    pipe(dest1);
    pipe(dest2);
    pipe(dest3);
    pipe(dest4);
    printf("The Client has TCP port number %s and IP address ", PORT);
    int ii;
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\nClient: waiting for connections...\n\n");
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("Client: got connection from %s\n\n", s);
	num_servers++;
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
	    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
        	perror("recv");
        	exit(1);
   		}
    	    buf[numbytes] = '\0';
	    
	    close(new_fd);

	    printf("The Client receives neighbor information from the Server %c with TCP port number %d and IP address %s\n",
		buf[0], sockfd, s);
	    printf("\nThe Server %c has the following neighbor information:\n\n", buf[0]);
	    printf("Neighbor-----Cost\n");
	    int countx;
	    for(countx = 1; countx < strlen(buf); countx += 2)
	    {
		printf("server%c\t      %d\n", buf[countx], buf[countx+1] -'0');
	    }
	    printf("\nFor this connection with Server %c, The Client has TCP port number %s and IP address ",
		buf[0], PORT);
	    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
	    printf("\n\n");
	    printf("--------------------------------------------------------------\n");

	    if(num_servers == 1)
	   	write(dest1[1], buf, strlen(buf));
	    else if (num_servers == 2)
		write(dest2[1], buf, strlen(buf));
	    else if (num_servers == 3)
		write(dest3[1], buf, strlen(buf));
	    else if (num_servers == 4)
		write(dest4[1], buf, strlen(buf));

            exit(0);  
        }
        close(new_fd);  // parent doesn't need this
	if(num_servers == 4)
		break;
    }
    //adjacency matrix creation
    int i, j, k, l;
    char buffer1[1025];
    char buffer2[1025];
    char buffer3[1025];
    char buffer4[1025];
    if ((i = read(dest1[0], buffer1, 1024)) >= 0) {
	    //printf("read %d bytes from the pipe: \"%s\"\n", i, buffer1);
	}	
    else
	    perror("read");

    if ((j = read(dest2[0], buffer2, 1024)) >= 0) {
	    //printf("read %d bytes from the pipe: \"%s\"\n", j, buffer2);
	}	
    else
	    perror("read");
    if ((k = read(dest3[0], buffer3, 1024)) >= 0) {
	    //printf("read %d bytes from the pipe: \"%s\"\n", k, buffer3);
	}	
    else
	    perror("read");
    if ((j = read(dest4[0], buffer4, 1024)) >= 0) {
	    //printf("read %d bytes from the pipe: \"%s\"\n", l, buffer4);
	}	
    else
	    perror("read");

//------------------------------------------------ 1ST BUFFER-----------------------------------------------
    if(buffer1[0] == 'A')
    {
	if(buffer1[1] == 'B') 
	{
		adjacency_matrix[0][1] = buffer1[2];}
	if(buffer1[1] == 'C') 
		adjacency_matrix[0][2] = buffer1[2];
	if(buffer1[1] == 'D') 
		adjacency_matrix[0][3] = buffer1[2];
	if(buffer1[3] != ('B' || 'C' || 'D')) {
	if(buffer1[3] == 'B') 
		adjacency_matrix[0][1] = buffer1[4];
	if(buffer1[3] == 'C') 
		adjacency_matrix[0][2] = buffer1[4];
	if(buffer1[3] == 'D') 
		adjacency_matrix[0][3] = buffer1[4];
	}//end of dest[3] != NULL
	if(buffer1[5] != ('B' || 'C' || 'D')) {
	if(buffer1[5] == 'B') 
		adjacency_matrix[0][1] = buffer1[6];
	if(buffer1[5] == 'C') 
		adjacency_matrix[0][2] = buffer1[6];
	if(buffer1[5] == 'D') 
		adjacency_matrix[0][3] = buffer1[6];
	}//end of dest[5] != NULL
    }
    else if(buffer1[0] == 'B')
    {
	if(buffer1[1] == 'A') 
		adjacency_matrix[1][0] = buffer1[2];
	if(buffer1[1] == 'C') 
		adjacency_matrix[1][2] = buffer1[2];
	if(buffer1[1] == 'D') 
		adjacency_matrix[1][3] = buffer1[2];
	if(buffer1[3] != ('A' || 'C' || 'D')) {
	if(buffer1[3] == 'A') 
		adjacency_matrix[1][0] = buffer1[4];
	if(buffer1[3] == 'C') 
		adjacency_matrix[1][2] = buffer1[4];
	if(buffer1[3] == 'D') 
		adjacency_matrix[1][3] = buffer1[4];
	}//end of dest[3] != NULL
	if(buffer1[5] != ('A' || 'C' || 'D')) {
	if(buffer1[5] == 'A') 
		adjacency_matrix[1][0] = buffer1[6];
	if(buffer1[5] == 'C') 
		adjacency_matrix[1][2] = buffer1[6];
	if(buffer1[5] == 'D') 
		adjacency_matrix[1][3] = buffer1[6];
	}//end of dest[5] != NULL

    }
    else if(buffer1[0] == 'C')
    {
	if(buffer1[1] == 'A') 
		adjacency_matrix[2][0] = buffer1[2];
	if(buffer1[1] == 'B') 
		adjacency_matrix[2][1] = buffer1[2];
	if(buffer1[1] == 'D') 
		adjacency_matrix[2][3] = buffer1[2];
	if(buffer1[3] != ('A' || 'B' || 'D')) {
	if(buffer1[3] == 'A') 
		adjacency_matrix[2][0] = buffer1[4];
	if(buffer1[3] == 'B') 
		adjacency_matrix[2][1] = buffer1[4];
	if(buffer1[3] == 'D') 
		adjacency_matrix[2][3] = buffer1[4];
	}//end of dest[3] != NULL
	if(buffer1[5] != ('A' || 'B' || 'D')) {
	if(buffer1[5] == 'A') 
		adjacency_matrix[2][0] = buffer1[6];
	if(buffer1[5] == 'B') 
		adjacency_matrix[2][1] = buffer1[6];
	if(buffer1[5] == 'D') 
		adjacency_matrix[2][3] = buffer1[6];
	}//end of dest[5] != NULL

    }
    else if(buffer1[0] == 'D')
    {
	if(buffer1[1] == 'A') 
		adjacency_matrix[3][0] = buffer1[2];
	if(buffer1[1] == 'B') 
		adjacency_matrix[3][1] = buffer1[2];
	if(buffer1[1] == 'C') 
		adjacency_matrix[3][2] = buffer1[2];
	if(buffer1[3] != ('A' || 'B' || 'C')) {
	if(buffer1[3] == 'A') 
		adjacency_matrix[3][0] = buffer1[4];
	if(buffer1[3] == 'B') 
		adjacency_matrix[3][1] = buffer1[4];
	if(buffer1[3] == 'C') 
		adjacency_matrix[3][2] = buffer1[4];
	}//end of dest[3] != NULL
	if(buffer1[5] != ('A' || 'B' || 'C')) {
	if(buffer1[5] == 'A') 
		adjacency_matrix[3][0] = buffer1[6];
	if(buffer1[5] == 'B') 
		adjacency_matrix[3][1] = buffer1[6];
	if(buffer1[5] == 'C') 
		adjacency_matrix[3][2] = buffer1[6];
	}//end of dest[5] != NULL

    }

//----------------------------------------------  2ND BUFFER-----------------------------------------------
    if(buffer2[0] == 'A')
    {
	if(buffer2[1] == 'B') 
		adjacency_matrix[0][1] = buffer2[2];
	if(buffer2[1] == 'C') 
		adjacency_matrix[0][2] = buffer2[2];
	if(buffer2[1] == 'D') 
		adjacency_matrix[0][3] = buffer2[2];
	if(buffer2[3] != ('B' || 'C' || 'D')) {
	if(buffer2[3] == 'B') 
		adjacency_matrix[0][1] = buffer2[4];
	if(buffer2[3] == 'C') 
		adjacency_matrix[0][2] = buffer2[4];
	if(buffer2[3] == 'D') 
		adjacency_matrix[0][3] = buffer2[4];
	}//end of dest[3] != NULL
	if(buffer2[5] != ('B' || 'C' || 'D')) {
	if(buffer2[5] == 'B') 
		adjacency_matrix[0][1] = buffer2[6];
	if(buffer2[5] == 'C') 
		adjacency_matrix[0][2] = buffer2[6];
	if(buffer2[5] == 'D') 
		adjacency_matrix[0][3] = buffer2[6];
	}//end of dest[5] != NULL
    }
    else if(buffer2[0] == 'B')
    {
	//adjacency_matrix[1][1] = '0';
	if(buffer2[1] == 'A') 
		adjacency_matrix[1][0] = buffer2[2];
	if(buffer2[1] == 'C') 
		adjacency_matrix[1][2] = buffer2[2];
	if(buffer2[1] == 'D') 
		adjacency_matrix[1][3] = buffer2[2];
	if(buffer2[3] != ('A' || 'C' || 'D')) {
	if(buffer2[3] == 'A') 
		adjacency_matrix[1][0] = buffer2[4];
	if(buffer2[3] == 'C') 
		adjacency_matrix[1][2] = buffer2[4];
	if(buffer2[3] == 'D') 
		adjacency_matrix[1][3] = buffer2[4];
	}//end of dest[3] != NULL
	if(buffer2[5] != ('A' || 'C' || 'D')) {
	if(buffer2[5] == 'A') 
		adjacency_matrix[1][0] = buffer2[6];
	if(buffer2[5] == 'C') 
		adjacency_matrix[1][2] = buffer2[6];
	if(buffer2[5] == 'D') 
		adjacency_matrix[1][3] = buffer2[6];
	}//end of dest[5] != NULL

    }
    else if(buffer2[0] == 'C')
    {
	//adjacency_matrix[2][2] = '0';
	if(buffer2[1] == 'A') 
		adjacency_matrix[2][0] = buffer2[2];
	if(buffer2[1] == 'B') 
		adjacency_matrix[2][1] = buffer2[2];
	if(buffer2[1] == 'D') 
		adjacency_matrix[2][3] = buffer2[2];
	if(buffer2[3] != ('A' || 'B' || 'D')) {
	if(buffer2[3] == 'A') 
		adjacency_matrix[2][0] = buffer2[4];
	if(buffer2[3] == 'B') 
		adjacency_matrix[2][1] = buffer2[4];
	if(buffer2[3] == 'D') 
		adjacency_matrix[2][3] = buffer2[4];
	}//end of dest[3] != NULL
	if(buffer2[5] != ('A' || 'B' || 'D')) {
	if(buffer2[5] == 'A') 
		adjacency_matrix[2][0] = buffer2[6];
	if(buffer2[5] == 'B') 
		adjacency_matrix[2][1] = buffer2[6];
	if(buffer2[5] == 'D') 
		adjacency_matrix[2][3] = buffer2[6];
	}//end of dest[5] != NULL

    }
    else if(buffer2[0] == 'D')
    {
	//adjacency_matrix[3][3] ='0';
	if(buffer2[1] == 'A') 
		adjacency_matrix[3][0] = buffer2[2];
	if(buffer2[1] == 'B') 
		adjacency_matrix[3][1] = buffer2[2];
	if(buffer2[1] == 'C') 
		adjacency_matrix[3][2] = buffer2[2];
	if(buffer2[3] != ('A' || 'B' || 'C')) {
	if(buffer2[3] == 'A') 
		adjacency_matrix[3][0] = buffer2[4];
	if(buffer2[3] == 'B') 
		adjacency_matrix[3][1] = buffer2[4];
	if(buffer2[3] == 'C') 
		adjacency_matrix[3][2] = buffer2[4];
	}//end of dest[3] != NULL
	if(buffer2[5] != ('A' || 'B' || 'C')) {
	if(buffer2[5] == 'A') 
		adjacency_matrix[3][0] = buffer2[6];
	if(buffer2[5] == 'B') 
		adjacency_matrix[3][1] = buffer2[6];
	if(buffer2[5] == 'C') 
		adjacency_matrix[3][2] = buffer2[6];
	}//end of dest[5] != NULL

    }

// --------------------------------------------- 3RD BUFFER ----------------------------------------------
    if(buffer3[0] == 'A')
    {
	if(buffer3[1] == 'B') 
		adjacency_matrix[0][1] = buffer3[2];
	if(buffer3[1] == 'C') 
		adjacency_matrix[0][2] = buffer3[2];
	if(buffer3[1] == 'D') 
		adjacency_matrix[0][3] = buffer3[2];
	if(buffer3[3] != ('B' || 'C' || 'D')) {
	if(buffer3[3] == 'B') 
		adjacency_matrix[0][1] = buffer3[4];
	if(buffer3[3] == 'C') 
		adjacency_matrix[0][2] = buffer3[4];
	if(buffer3[3] == 'D') 
		adjacency_matrix[0][3] = buffer3[4];
	}//end of dest[3] != NULL
	if(buffer3[5] != ('B' || 'C' || 'D')) {
	if(buffer3[5] == 'B') 
		adjacency_matrix[0][1] = buffer3[6];
	if(buffer3[5] == 'C') 
		adjacency_matrix[0][2] = buffer3[6];
	if(buffer3[5] == 'D') 
		adjacency_matrix[0][3] = buffer3[6];
	}//end of dest[5] != NULL
    }
    else if(buffer3[0] == 'B')
    {
	//adjacency_matrix[1][1] = '0';
	if(buffer3[1] == 'A') 
		adjacency_matrix[1][0] = buffer3[2];
	if(buffer3[1] == 'C') 
		adjacency_matrix[1][2] = buffer3[2];
	if(buffer3[1] == 'D') 
		adjacency_matrix[1][3] = buffer3[2];
	if(buffer3[3] != ('A' || 'C' || 'D')) {
	if(buffer3[3] == 'A') 
		adjacency_matrix[1][0] = buffer3[4];
	if(buffer3[3] == 'C') 
		adjacency_matrix[1][2] = buffer3[4];
	if(buffer3[3] == 'D') 
		adjacency_matrix[1][3] = buffer3[4];
	}//end of dest[3] != NULL
	if(buffer3[5] != ('A' || 'C' || 'D')) {
	if(buffer3[5] == 'A') 
		adjacency_matrix[1][0] = buffer3[6];
	if(buffer3[5] == 'C') 
		adjacency_matrix[1][2] = buffer3[6];
	if(buffer3[5] == 'D') 
		adjacency_matrix[1][3] = buffer3[6];
	}//end of dest[5] != NULL

    }
    else if(buffer3[0] == 'C')
    {
	//adjacency_matrix[2][2] = '0';
	if(buffer3[1] == 'A') 
		adjacency_matrix[2][0] = buffer3[2];
	if(buffer3[1] == 'B') 
		adjacency_matrix[2][1] = buffer3[2];
	if(buffer3[1] == 'D') 
		adjacency_matrix[2][3] = buffer3[2];
	if(buffer3[3] != ('A' || 'B' || 'D')) {
	if(buffer3[3] == 'A') 
		adjacency_matrix[2][0] = buffer3[4];
	if(buffer3[3] == 'B') 
		adjacency_matrix[2][1] = buffer3[4];
	if(buffer3[3] == 'D') 
		adjacency_matrix[2][3] = buffer3[4];
	}//end of dest[3] != NULL
	if(buffer3[5] != ('A' || 'B' || 'D')) {
	if(buffer3[5] == 'A') 
		adjacency_matrix[2][0] = buffer3[6];
	if(buffer3[5] == 'B') 
		adjacency_matrix[2][1] = buffer3[6];
	if(buffer3[5] == 'D') 
		adjacency_matrix[2][3] = buffer3[6];
	}//end of dest[5] != NULL

    }
    else if(buffer3[0] == 'D')
    {
	//adjacency_matrix[3][3] ='0';
	if(buffer3[1] == 'A') 
		adjacency_matrix[3][0] = buffer3[2];
	if(buffer3[1] == 'B') 
		adjacency_matrix[3][1] = buffer3[2];
	if(buffer3[1] == 'C') 
		adjacency_matrix[3][2] = buffer3[2];
	if(buffer3[3] != ('A' || 'B' || 'C')) {
	if(buffer3[3] == 'A') 
		adjacency_matrix[3][0] = buffer3[4];
	if(buffer3[3] == 'B') 
		adjacency_matrix[3][1] = buffer3[4];
	if(buffer3[3] == 'C') 
		adjacency_matrix[3][2] = buffer3[4];
	}//end of dest[3] != NULL
	if(buffer3[5] != ('A' || 'B' || 'C')) {
	if(buffer3[5] == 'A') 
		adjacency_matrix[3][0] = buffer3[6];
	if(buffer3[5] == 'B') 
		adjacency_matrix[3][1] = buffer3[6];
	if(buffer3[5] == 'C') 
		adjacency_matrix[3][2] = buffer3[6];
	}//end of dest[5] != NULL

    }

// ------------------------------------------- 4TH BUFFER ------------------------------------------------
    if(buffer4[0] == 'A')
    {
	if(buffer4[1] == 'B') 
		adjacency_matrix[0][1] = buffer4[2];
	if(buffer4[1] == 'C') 
		adjacency_matrix[0][2] = buffer4[2];
	if(buffer4[1] == 'D') 
		adjacency_matrix[0][3] = buffer4[2];
	if(buffer4[3] != ('B' || 'C' || 'D')) {
	if(buffer4[3] == 'B') 
		adjacency_matrix[0][1] = buffer4[4];
	if(buffer4[3] == 'C') 
		adjacency_matrix[0][2] = buffer4[4];
	if(buffer4[3] == 'D') 
		adjacency_matrix[0][3] = buffer4[4];
	}//end of dest[3] != NULL
	if(buffer4[5] != ('B' || 'C' || 'D')) {
	if(buffer4[5] == 'B') 
		adjacency_matrix[0][1] = buffer4[6];
	if(buffer4[5] == 'C') 
		adjacency_matrix[0][2] = buffer4[6];
	if(buffer4[5] == 'D') 
		adjacency_matrix[0][3] = buffer4[6];
	}//end of dest[5] != NULL
    }
    else if(buffer4[0] == 'B')
    {
	//adjacency_matrix[1][1] = '0';
	if(buffer4[1] == 'A') 
		adjacency_matrix[1][0] = buffer4[2];
	if(buffer4[1] == 'C') 
		adjacency_matrix[1][2] = buffer4[2];
	if(buffer4[1] == 'D') 
		adjacency_matrix[1][3] = buffer4[2];
	if(buffer4[3] != ('A' || 'C' || 'D')) {
	if(buffer4[3] == 'A') 
		adjacency_matrix[1][0] = buffer4[4];
	if(buffer4[3] == 'C') 
		adjacency_matrix[1][2] = buffer4[4];
	if(buffer4[3] == 'D') 
		adjacency_matrix[1][3] = buffer4[4];
	}//end of dest[3] != NULL
	if(buffer4[5] != ('A' || 'C' || 'D')) {
	if(buffer4[5] == 'A') 
		adjacency_matrix[1][0] = buffer4[6];
	if(buffer4[5] == 'C') 
		adjacency_matrix[1][2] = buffer4[6];
	if(buffer4[5] == 'D') 
		adjacency_matrix[1][3] = buffer4[6];
	}//end of dest[5] != NULL

    }
    else if(buffer4[0] == 'C')
    {
	//adjacency_matrix[2][2] = '0';
	if(buffer4[1] == 'A') 
		adjacency_matrix[2][0] = buffer4[2];
	if(buffer4[1] == 'B') 
		adjacency_matrix[2][1] = buffer4[2];
	if(buffer4[1] == 'D') 
		adjacency_matrix[2][3] = buffer4[2];
	if(buffer4[3] != ('A' || 'B' || 'D')) {
	if(buffer4[3] == 'A') 
		adjacency_matrix[2][0] = buffer4[4];
	if(buffer4[3] == 'B') 
		adjacency_matrix[2][1] = buffer4[4];
	if(buffer4[3] == 'D') 
		adjacency_matrix[2][3] = buffer4[4];
	}//end of dest[3] != NULL
	if(buffer4[5] != ('A' || 'B' || 'D')) {
	if(buffer4[5] == 'A') 
		adjacency_matrix[2][0] = buffer4[6];
	if(buffer4[5] == 'B') 
		adjacency_matrix[2][1] = buffer4[6];
	if(buffer4[5] == 'D') 
		adjacency_matrix[2][3] = buffer4[6];
	}//end of dest[5] != NULL

    }
    else if(buffer4[0] == 'D')
    {
	//adjacency_matrix[3][3] ='0';
	if(buffer4[1] == 'A') 
		adjacency_matrix[3][0] = buffer4[2];
	if(buffer4[1] == 'B') 
		adjacency_matrix[3][1] = buffer4[2];
	if(buffer4[1] == 'C') 
		adjacency_matrix[3][2] = buffer4[2];
	if(buffer4[3] != ('A' || 'B' || 'C')) {
	if(buffer4[3] == 'A') 
		adjacency_matrix[3][0] = buffer4[4];
	if(buffer4[3] == 'B') 
		adjacency_matrix[3][1] = buffer4[4];
	if(buffer4[3] == 'C') 
		adjacency_matrix[3][2] = buffer4[4];
	}//end of dest[3] != NULL
	if(buffer4[5] != ('A' || 'B' || 'C')) {
	if(buffer4[5] == 'A') 
		adjacency_matrix[3][0] = buffer4[6];
	if(buffer4[5] == 'B') 
		adjacency_matrix[3][1] = buffer4[6];
	if(buffer4[5] == 'C') 
		adjacency_matrix[3][2] = buffer4[6];
	}//end of dest[5] != NULL

    }
// ----------------------------------------------------------------------------------------------
    printf("\n");
    printf("-----------Adjacency Matrix-----------\n");
    printf("\tA \tB \tC \tD \n");

    int w,e;
    for(w = 0; w < 4; w++)
    {
	if(w == 0) printf("A\t");
	else if (w == 1) printf("B\t");
	else if (w == 2) printf("C\t");
	else if (w == 3) printf("D\t");
	for(e = 0; e < 4; e++)
	{
		printf("%d\t", adjacency_matrix[w][e] -'0');

	}
	printf("\n");
    }
    printf("--------------------------------------\n");
    char udp_buffer[1025] = "";
    //calculate edge/cost topology
    for(w = 0; w < 4; w++)
    {
	for(e = 0; e < 4; e++)
	{
		if(adjacency_matrix[w][e] != '0')
		{
			if(w == 0)
			{
				if(e == 1) //add AB 
				{ 
					int len = strlen(udp_buffer);	
     					udp_buffer[len] = 'A';
    					udp_buffer[len+1] = 'B';
					udp_buffer[len+2] = adjacency_matrix[w][e];
					udp_buffer[len+3] = '\0';
				}
				else if (e == 2) //add AC
				{ 
					int len = strlen(udp_buffer);	
     					udp_buffer[len] = 'A';
    					udp_buffer[len+1] = 'C';
					udp_buffer[len+2] = adjacency_matrix[w][e];
					udp_buffer[len+3] = '\0';
				}	
				else if (e == 3) //add AD
				{ 
					int len = strlen(udp_buffer);	
     					udp_buffer[len] = 'A';
    					udp_buffer[len+1] = 'D';
					udp_buffer[len+2] = adjacency_matrix[w][e];
					udp_buffer[len+3] = '\0';
				}	
			}
			else if (w == 1)
			{		
				// AB/BA would have been added above		
				if (e == 2) //add BC
 				{
					int len = strlen(udp_buffer);	
     					udp_buffer[len] = 'B';
    					udp_buffer[len+1] = 'C';
					udp_buffer[len+2] = adjacency_matrix[w][e];
					udp_buffer[len+3] = '\0';
				}
				else if (e == 3) //add BD
				{ 
					int len = strlen(udp_buffer);	
     					udp_buffer[len] = 'B';
    					udp_buffer[len+1] = 'D';
					udp_buffer[len+2] = adjacency_matrix[w][e];
					udp_buffer[len+3] = '\0';
				}

			}
			else if (w == 2)
			{
				// AC/CA, BC/CB would have been added above
				if (e == 4) //add CD
				{ 
					int len = strlen(udp_buffer);	
     					udp_buffer[len] = 'C';
    					udp_buffer[len+1] = 'D';
					udp_buffer[len+2] = adjacency_matrix[w][e];
					udp_buffer[len+3] = '\0';
				}
			}
		}
	}
    }
    printf("\n");
    printf("--------------------------------------------------------------\n\n");
    sleep(2);
//-------------------------------------- UDP TALKER TO A ---------------------------------------------------
// UDP Talker code drived from beej datagram example

    int sock_udp;		//sockfd
    struct addrinfo udp_hints, *udpinfo, *q;	//hints //*servinfo //*p
    int rcv;	//rv
    int num_udp_bytes;	//numbytes
    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_UNSPEC;
    udp_hints.ai_socktype = SOCK_DGRAM;
    if ((rcv = getaddrinfo(localhost, SERVERA, &udp_hints, &udpinfo)) != 0) {	//SERVERPORT --> SERVERAserverA, serverB, serverC, serverD
 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rcv));

    return 1;
    }

    // loop through all the results and make a socket
    for(q = udpinfo; q != NULL; q = q->ai_next) {
 	if ((sock_udp = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1) {
 		perror("Client Talker: socket");
 		continue;
 	}
    	break;
    }

    if (q == NULL) {
    	fprintf(stderr, "Client Talker: failed to create socket\n");
    	return 2;
    }

    if ((num_udp_bytes = sendto(sock_udp, udp_buffer, strlen(udp_buffer), 0, q->ai_addr, q->ai_addrlen)) == -1) {
 	perror("Client Talker: sendto");
 	exit(1);
    }
    sleep(2);
    freeaddrinfo(udpinfo);

    printf("The Client has sent the network topology to the network topology");
    printf(" of Server A with UDP port number %d and IP address ", SERVERA);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
	    printf(" as follows: \n\n");
    printf("\nEdge------Cost\n");  
    int counta;
    for(counta = 0; counta < strlen(udp_buffer); counta += 3)
    {
	printf(" %c%c\t  %d\n", udp_buffer[counta], udp_buffer[counta+1], udp_buffer[counta+2] - '0');
    }
    printf("\nFor this connection with Server A, The Client has UDP port number %d and IP address ", sock_udp);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\n\n"); 
    printf("--------------------------------------------------------------\n\n");
    close(sock_udp);

//-------------------------------------- UDP TALKER TO B ---------------------------------------------------
// UDP Talker code drived from beej datagram example
    int sock_udpB;		//sockfd
    struct addrinfo udpB_hints, *udpBinfo, *o;	//hints //*servinfo //*p
    int brcv;	//rv
    int num_udpB_bytes;	//numbytes
    memset(&udpB_hints, 0, sizeof udpB_hints);
    udpB_hints.ai_family = AF_UNSPEC;
    udpB_hints.ai_socktype = SOCK_DGRAM;

    if ((brcv = getaddrinfo(localhost, SERVERB, &udpB_hints, &udpBinfo)) != 0) {//SERVERPORT --> SERVERAserverA, serverB, serverC, serverD
 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(brcv));

    return 1;
    }

    // loop through all the results and make a socket
    for(o = udpBinfo; o != NULL; o = o->ai_next) {
 	if ((sock_udpB = socket(o->ai_family, o->ai_socktype, o->ai_protocol)) == -1) {
 		perror("Client Talker: socket");
 		continue;
 	}
    	break;
    }

    if (o == NULL) {
    	fprintf(stderr, "Client Talker: failed to create socket\n");
    	return 2;
    }

    if ((num_udpB_bytes = sendto(sock_udpB, udp_buffer, strlen(udp_buffer), 0, o->ai_addr, o->ai_addrlen)) == -1) {
 	perror("Client Talker: sendto");
 	exit(1);
    }
    sleep(2);
    freeaddrinfo(udpBinfo);

    printf("The Client has sent the network topology to the network topology");
    printf(" of Server B with UDP port number %d and IP address ", SERVERB);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf(" as follows:\n\n"); 
    printf("\nEdge------Cost\n");  
    for(counta = 0; counta < strlen(udp_buffer); counta += 3)
    {
	printf(" %c%c\t  %d\n", udp_buffer[counta], udp_buffer[counta+1], udp_buffer[counta+2] - '0');
    }
    printf("\nFor this connection with Server B, The Client has UDP port number %d and IP address ", sock_udpB);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\n\n"); 
    printf("--------------------------------------------------------------\n\n");
    close(sock_udpB);
//-------------------------------------- UDP TALKER TO C ---------------------------------------------------
// UDP Talker code drived from beej datagram example
    int sock_udpC;		//sockfd
    struct addrinfo udpC_hints, *udpCinfo, *mm;	//hints //*servinfo //*p
    int Crcv;	//rv
    int num_udpC_bytes;	//numbytes
    memset(&udpC_hints, 0, sizeof udpC_hints);
    udpC_hints.ai_family = AF_UNSPEC;
    udpC_hints.ai_socktype = SOCK_DGRAM;

    if ((Crcv = getaddrinfo(localhost, SERVERC, &udpC_hints, &udpCinfo)) != 0) {//SERVERPORT --> SERVERAserverA, serverB, serverC, serverD
 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(Crcv));

    return 1;
    }

    // loop through all the results and make a socket
    for(mm = udpCinfo; mm != NULL; mm = mm->ai_next) {
 	if ((sock_udpC = socket(mm->ai_family, mm->ai_socktype, mm->ai_protocol)) == -1) {
 		perror("Client Talker: socket");
 		continue;
 	}
    	break;
    }

    if (mm == NULL) {
    	fprintf(stderr, "Client Talker: failed to create socket\n");
    	return 2;
    }

    if ((num_udpC_bytes = sendto(sock_udpC, udp_buffer, strlen(udp_buffer), 0, mm->ai_addr, mm->ai_addrlen)) == -1) {
 	perror("Client Talker: sendto");
 	exit(1);
    }
    sleep(2);
    freeaddrinfo(udpCinfo);

    printf("The Client has sent the network topology to the network topology");
    printf(" of Server C with UDP port number %d and IP address ", SERVERC);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf(" as follows:\n\n"); 
    printf("\nEdge------Cost\n");  
    for(counta = 0; counta < strlen(udp_buffer); counta += 3)
    {
	printf(" %c%c\t  %d\n", udp_buffer[counta], udp_buffer[counta+1], udp_buffer[counta+2] - '0');
    }
    printf("\nFor this connection with Server C, The Client has UDP port number %d and IP address ", sock_udpC);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\n\n"); 
    printf("--------------------------------------------------------------\n\n");
    close(sock_udpC);
//-------------------------------------- UDP TALKER TO D ---------------------------------------------------
// UDP Talker code drived from beej datagram example
    int sock_udpD;		//sockfd
    struct addrinfo udpD_hints, *udpDinfo, *EE;	//hints //*servinfo //*p
    int rcvD;	//rv
    int num_udpD_bytes;	//numbytes

    memset(&udpD_hints, 0, sizeof udpD_hints);
    udpD_hints.ai_family = AF_UNSPEC;
    udpD_hints.ai_socktype = SOCK_DGRAM;

    if ((rcvD = getaddrinfo(localhost, SERVERD, &udpD_hints, &udpDinfo)) != 0) {//SERVERPORT --> SERVERAserverA, serverB, serverC, serverD
 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rcvD));

    return 1;
    }

    // loop through all the results and make a socket
    for(EE = udpDinfo; EE != NULL; EE = EE->ai_next) {
 	if ((sock_udpD = socket(EE->ai_family, EE->ai_socktype, EE->ai_protocol)) == -1) {
 		perror("Client Talker: socket");
 		continue;
 	}
    	break;
    }

    if (EE == NULL) {
    	fprintf(stderr, "Client Talker: failed to create socket\n");
    	return 2;
    }

    if ((num_udpD_bytes = sendto(sock_udpD, udp_buffer, strlen(udp_buffer), 0, EE->ai_addr, EE->ai_addrlen)) == -1) {
 	perror("Client Talker: sendto");
 	exit(1);
    }
    sleep(2);
    freeaddrinfo(udpDinfo);

    printf("The Client has sent the network topology to the network topology");
    printf(" of Server D with UDP port number %d and IP address ", SERVERA);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf(" as follows:\n\n"); 
    printf("\nEdge------Cost\n");  
    for(counta = 0; counta < strlen(udp_buffer); counta += 3)
    {
	printf(" %c%c\t  %d\n", udp_buffer[counta], udp_buffer[counta+1], udp_buffer[counta+2] - '0');
    }
    printf("\nFor this connection with Server D, The Client has UDP port number %d and IP address ",sock_udpD);
    for(ii = 0; addr_list[ii] != NULL; ii++) {printf("%s ", inet_ntoa(*addr_list[ii]));} 
    printf("\n\n"); 
    printf("--------------------------------------------------------------\n\n");
    close(sock_udpD);

//-------------------------------------- phase 3 ----------------------------------------
//phase 3 algorithm derived from Prim's algorithm

   //to convert char -> int, char - '0'
   //to convert int -> char, int + '0'
    //int visited[10] = {0},min,mincost=0;
    
   /* for(w = 0; w < 4; w++)
    {
	for(e = 0; e < 4; e++)
	{
		if(adjacency_matrix[w][e] == '0')
			adjacency_matrix[w][e] = 99 + '0';

	}

    }
    printf("\n Edge ----- Cost \n");
    visited[0] = 1; 
    int ne = 0;
    int ix, a, b, u, v;
    while(ne < 4)
    {
	for(i = 0, min = 99; i < 4; i++)
		for(j = 0; j < 4; j++)
			if((adjacency_matrix[i][j] -'0') < min)
				if(visited[i] != 0)
				{
					min = adjacency_matrix[i][j] - '0';
					a = u = i;
					b = v = j;
				}
	if(visited[u] == 0 || visited[v] == 0)
	{	
		ne++;
		printf("Edge %d:(%d %d) \t cost:%d", ne , a, b, min);
		mincost += min;
		visited[b] = 1;
	}
	adjacency_matrix[a][b] = adjacency_matrix[b][a] = 99 + '0';
    }
    printf("\n Minimum Cost = %d \n", mincost);
 */
    return 0;
}
