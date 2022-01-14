CFLAGS = -std=c++17 -O3
LEX = lekser.l
BISON = parser.y
program = kompilator

all:
	bison -d ${BISON} -o parser.tab.c
	g++ ${CFLAGS} -c parser.tab.c -o parser.o
	flex ${LEX}
	g++ ${CFLAGS} -c lex.yy.c -o lexer.o
	g++ ${CFLAGS} parser.o lexer.o -o ${program}
