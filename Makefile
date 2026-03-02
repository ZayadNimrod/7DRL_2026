

EXECUTABLE_PATH = bin/7drl

CFLAGS_DEBUG 	 = 	-Og -g $(CFLAGS_COMMON)
CFLAGS_COMMON    = -MD -MP -Wall -Wextra -Werror
CFLAGS_RELEASE   = -Os $(CFLAGS_COMMON) -DNDEBUG


$(EXECUTABLE_PATH):
	gcc $(CFLAGS_DEBUG) -o $@ main.c -lncurses

build: $(EXECUTABLE_PATH)

run: build
	$(EXECUTABLE_PATH)

clean:
	rm bin/*