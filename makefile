SRCS=$(wildcard *.c)
NAME=vince12

all:
	gcc $(SRCS) -o $(NAME) -O2