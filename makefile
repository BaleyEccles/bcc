
SRC_FILES = ./src/main.c ./src/tokenizer.c ./src/AST.c ./src/graph.c
run: build
	./main

build:
	gcc -Wall -g -o main $(SRC_FILES)
