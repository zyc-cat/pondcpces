CC	= g++

FLAGS	= -DUNIX
MFLAG   =
ICFLAGS = -g
CFLAGS  = $(ICFLAGS) $(MFLAG) $(XCFLAGS)

SRC	= rddlclient.cc strxml.cc observation.cc action_group.cc action.cc fluent.cc problem.cc
OBJ	= $(SRC:.cc=.o)
EXEC = rddlclient

all: $(SRC) $(EXEC) manual_client manual_server

clean:
	rm -f $(OBJ) manual_client.o manual_server.o $(EXEC) manual_client manual_server

manual_client: manual_client.o
	$(CC) manual_client.o -o $@

manual_client.cc.o:
	$(CC) $(CFLAGS) -I. -c manual_client.cc -o $@

manual_server: manual_server.o
	$(CC) manual_server.o -o $@

manual_server.cc.o:
	$(CC) $(CFLAGS) -I. -c manual_server.cc -o $@

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@

.cc.o:
	$(CC) $(CFLAGS) -I. -c $< -o $@
