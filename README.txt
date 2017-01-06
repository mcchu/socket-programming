a) Michael Chu
b) 5708356882

c) In this assignment, we are tasked with utilizing socket programming to transfer neighboring information from each server to a client, who
will then send all the information gathered to create a topology of the network. The servers will send their information to the 
client via TCP, and the client will send it back to each server via UDP. Ater that, we attempt to analyze the minimum spanning tree of 
the network. In that extent, I have derived a lot of structures and basic code from beej, and implemented it to make sure that the code
works well and transmits TCP/UDP dynamically (if needed).

d)  I have 5 code files: client.c, serverA.c ,serverB.c ,serverC.c ,serverD.c.

client.c is the client server which opens up a "TCP listener" to wait for any TCP packets. After receiving from 4 servers, 
it will process the information into an adjacency matrix, then send the topology back to each server via UDP. Then, it'll calculate
the minimum-spanning tree and display it on the terminal.

serverA.c, ServerB.c, serverC.c, serverD.c are all similar to each other. As soon as they are executed, they send their neighboring
data, which is extrapolated from a textfile, to whoever is listening for TCP packets (which is Client). Then, the servers goes into
"listening" for UDP packets mode, where it waits for client. Once it receives the UDP packet, it will display the results on the terminal.

e) To run the program, please follow the sequence below:

make
./client.out
on another terminal: ./serverA.out serverA.txt
on another terminal: ./serverB.out serverB.txt
on another terminal: ./serverC.out serverC.txt
on another terminal: ./serverD.out serverD.txt

f) The format of the messages will be like those shown in the assignment.
Servers:
- Booting Up message
- Receiving File message
- Sending neighbor information to client message
- Receiving topology information from client message

Client:
- Booting up message
- Receiving message from server A
- Receiving message from server B
- Receiving message from server C
- Receiving message from server D
- Adjacency Matrix
- Sending message to server A
- Sending message to server B
- Sending message to server C
- Sending message to server D
- Minimum spanning tree output

g) Idiosyncrasies:
Sometimes, if you execute serverA last, the UDP packets might not receive it. The slight error happens occasionally, and is 
quite uncommon, but does appear. This only affects serverA however, so if you execute the server commands in any order, with 
serverA not last, there will be no error.

h) Reused Code:
The creation of sockets and the networking architecture was used by beej. Also, the minimum spanning tree algorithm 
is derived from the greedy algorithm of Prims algorithm. Each area has been commented appropriately in the code.

