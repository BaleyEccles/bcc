
link: assemble
	ld -o fibonacci fibonacci.o

assemble: clean
	as -o fibonacci.o fibonacci.asm

clean: fibonacci.o fibonacci
	rm fibonacci.o fibonacci
