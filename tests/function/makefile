
link: assemble
	ld -o function function.o

assemble: clean
	as -o function.o function.asm

clean: function.o function
	rm function.o function
