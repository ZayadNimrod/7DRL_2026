

EXECUTABLE_PATH = bin/7drl

CFLAGS_DEBUG	= 	-O0 -g $(CFLAGS_COMMON)
CFLAGS_COMMON	= -MD -MP -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-but-set-variable
CFLAGS_RELEASE	= -Os $(CFLAGS_COMMON) -DNDEBUG
CLIBS	= -lncurses

$(shell mkdir -p bin tests)

$(EXECUTABLE_PATH): **.c
	gcc $(CFLAGS_DEBUG) -o $@ main.c $(CLIBS)

build: $(EXECUTABLE_PATH)

run: build
	$(EXECUTABLE_PATH)

clean:
	rm bin/*

tests/engine_test: engine.c engine_test.c
	gcc $(CFLAGS_DEBUG) -o tests/engine_test engine_test.c $(CLIBS)

test: tests/engine_test
