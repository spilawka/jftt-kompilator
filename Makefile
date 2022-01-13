LEX := lekser.l
BISON := parser.y
program := kompilator

all:
	bison -d ${BISON} -o parser.tab.c
	g++ -c parser.tab.c -o parser.o
	flex ${LEX}
	g++ -c lex.yy.c -o lexer.o
	g++ parser.o lexer.o -o ${program}
