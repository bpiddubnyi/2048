PROJECT ?= 2048
CFLAGS = -W -Wall -Werror -std=c99 -O2
LDFLAGS = -lm -lncurses 

default: all

all: $(PROJECT)

%.o: %.c
	@echo "  CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $^

$(PROJECT): main.o 2048.o
	@echo "  LD $@"
	@$(CC) -o $@ $^ $(LDFLAGS) 

clean:
	rm -rf $(PROJECT) *.o

.PHONY: default clean all
