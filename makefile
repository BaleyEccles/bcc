SRC_FILES = ./src/main.c ./src/tokenizer.c ./src/AST.c ./src/graph.c ./src/assembly.c ./src/preprocessor.c

build:
	gcc -Wall -g -o bcc $(SRC_FILES)
mem_debug:
	gcc -fsanitize=address -Wall -g -o bcc $(SRC_FILES)
test: 
	./test.sh
