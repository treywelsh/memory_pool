BIN = pool_test 

INCLUDE = -I ../src
LIB = -L ../lib
SRC = pool_test.c
OBJ = $(SRC:.c=.o)

CC ?= gcc
CFLAGS ?= -Wall -Wextra
LDFLAGS ?= -lpool

DEBUG ?= 1
VALGRIND ?=1
STATIC ?= 1

#Mandatory flags
override CFLAGS += $(INCLUDE)
override LDFLAGS += $(LIB) $(INCLUDE)

ifeq ($(DEBUG), 1)
	override CFLAGS += -DDEBUG -O0 -ggdb3
else
	override CFLAGS += -DNDEBUG -O3
endif

ifeq ($(STATIC), 1)
	override LDFLAGS += -static
endif

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(SRC) -o $(BIN) $(LDFLAGS)

.PHONY: all clean
clean :
	@rm -f $(OBJ) $(BIN) 
