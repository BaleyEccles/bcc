
link: assemble
	ld -o if_statement if_statement.o

assemble: clean
	as -o if_statement.o if_statement.asm

clean: if_statement.o if_statement
	rm if_statement.o if_statement
