OBJS = client serverA serverB serverC serverD
CC = gcc

all: client serverA serverB serverC serverD

client:
	 $(CC) client.c -o client.out -lsocket -lnsl -lresolv

serverA:
	 $(CC) serverA.c -o serverA.out -lsocket -lnsl -lresolv

serverB:
	 $(CC) serverB.c -o serverB.out -lsocket -lnsl -lresolv

serverC:
	 $(CC) serverC.c -o serverC.out -lsocket -lnsl -lresolv
	 
serverD:
	 $(CC) serverD.c -o serverD.out -lsocket -lnsl -lresolv

clean:
	 rm -rf $(OBJS)
