LEX := lekser.l
BISON := parser.y
program := kompilator

all:
	flex ${LEX}
	bison ${BISON}
	gcc -lfl lex.yy.c -o ${program}