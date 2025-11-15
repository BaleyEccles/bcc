SRC_FILES = ./src/main.c ./src/tokenizer.c ./src/AST.c ./src/graph.c ./src/assembly.c ./src/preprocessor.c

build:
	gcc -Wall -Wpedantic -Wextra -Wno-unused-parameter -g -o bcc $(SRC_FILES)
mem_debug:
	gcc -fsanitize=address -Wall -Wpedantic -Wextra -Wno-unused-parameter -g -o bcc $(SRC_FILES)
test:
	./test.sh
