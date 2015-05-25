#

SUB_OBJS=   ../source/CalcCardRate.o \
            ../source/CalcHandStrength.o \
            ../source/CalcRR.o \
            ../source/cardmasks_table.o \
            ../source/ChooseStrategy.o \
            ../source/game.o \
            ../source/Global.o \
            ../source/main.o \
            ../source/MsgProc.o \
            ../source/nbitsandstr_table.o \
            ../source/nbits_table.o \
            ../source/Player.o \
            ../source/SendAction.o \
            ../source/straight_table.o \
            ../source/StrategyHoldCard.o \
            ../source/test_calc_rate.o \
            ../source/topcard_table.o \
            ../source/topfivecards_table.o

CPPFLAGS=-I../source 

#LDFLAGS=-L../libs -labc -ldef

LDFLAGS=-lstdc++

APP_NAME=game

all:$(SUB_OBJS)
	gcc -o ../target/$(APP_NAME) $(SUB_OBJS) $(LDFLAGS)

clean:
	rm -f ../target/$(APP_NAME)
	rm -f $(SUB_OBJS)

%.o: %.cpp \
	g++ $(CPPFLAGS) -c $^ -o $@;

