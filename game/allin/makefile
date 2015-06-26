#

SUB_OBJS1=main.o card.o strategy.o analyse.o msg_reader.o player.o

SUB_OBJS2=msg_reader.o

SUB_OBJS=$(SUB_OBJS1)

CPPFLAGS=-I../source

#LDFLAGS=-L../libs -labc -ldef

LDFLAGS=-lstdc++ -lpthread

APP_NAME=game

all:$(SUB_OBJS)
	gcc -o $(APP_NAME) $(SUB_OBJS) $(LDFLAGS)

clean:
	rm -f $(APP_NAME)
	rm -f $(SUB_OBJS)
	rm -f test
test:
	gcc -DDEBUG -g -o test msg_reader.cpp

%.o: %.cpp \
	g++ -Wall -g -fpic $(CPPFLAGS) -c $^ -o $@;



