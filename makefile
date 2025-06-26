
SRC_FILES = ./src/main.c ./src/tokenizer.c ./src/AST.c ./src/graph.c
run: build
	./main

build:
	gcc -Wall -g -o main $(SRC_FILES)

output: link_output
	./output
link_output: assemble_output
	ld -o output output.o
assemble_output:
	as -o output.o output.asm

