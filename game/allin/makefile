#

SUB_OBJS1=main.o card.o strategy.o analyse.o msg_reader.o

SUB_OBJS2=msg_reader.o

SUB_OBJS=$(SUB_OBJS2)

CPPFLAGS=-I../source

#LDFLAGS=-L../libs -labc -ldef

LDFLAGS=-lstdc++

APP_NAME=game

all:$(SUB_OBJS)
	gcc -o $(APP_NAME) $(SUB_OBJS) $(LDFLAGS)

clean:
	rm -f $(APP_NAME)
	rm -f $(SUB_OBJS)

%.o: %.cpp \
	g++ $(CPPFLAGS) -c $^ -o $@;

