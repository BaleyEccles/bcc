
link: assemble
	ld -o while_loop while_loop.o

assemble: clean
	as -o while_loop.o while_loop.asm

clean: while_loop.o while_loop
	rm while_loop.o while_loop
