NAME = pool

LIBPATH = lib
TARGET_SO = $(LIBPATH)/lib$(NAME).so
TARGET_ST = $(LIBPATH)/lib$(NAME).a

CC ?= gcc
SRC = src/pool.c
OBJ = $(SRC:.c=.o)
FPIC_OBJ = $(SRC:.c=.fpic.o)
CFLAGS ?= -Wall -Wextra
LDFLAGS ?=
DEBUG ?= 0
VALGRIND ?=0

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -O0 -ggdb3
else
	CFLAGS += -DNDEBUG -O3
endif

all: $(TARGET_SO) $(TARGET_ST)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

#static
$(TARGET_ST) : $(OBJ)
	@mkdir -p $(LIBPATH)
	$(AR) rcs $@ $^

#shared
$(TARGET_SO) : $(FPIC_OBJ)
	@mkdir -p $(LIBPATH)
	$(CC) -shared $(LDFLAGS) -o $@ $<

%.fpic.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

test: $(TARGET_SO) $(TARGET_ST)
	@make -C test

.PHONY: all clean
clean :
	@rm -f $(OBJ) $(FPIC_OBJ) $(TARGET_SO) $(TARGET_ST)
	@rm -df $(LIBPATH)
	@make -C test clean
