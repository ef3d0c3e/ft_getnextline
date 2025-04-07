NAME := libgnl.a
CC := gcc
CFLAGS := -Wall -Wextra -pedantic -O2
IFLAGS :=
LFLAGS :=

# Objects
SOURCES := \
	src/get_next_line.c \
	src/get_next_line_utils.c

OBJECTS := $(addprefix objs/,$(SOURCES:.c=.o))

SOURCES_TEST := $(wildcard tests/*.c)
OBJECTS_TEST := $(addprefix objs/,$(SOURCES_TEST:.c=.o))

objs/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

# Library
$(NAME): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

.PHONY: all
all: $(NAME)

.PHONY: clean
clean:
	$(RM) -r objs

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)
	$(RM) libftprintf.so
	$(RM) printf-tests

.PHONY: re
re: fclean all
