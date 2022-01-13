%{
  #include <stdio.h>
  #include <string>
  #include <iostream>
  #include "sym.hh"

  using namespace std;

  int yylex(void);
  void yyerror(char*);
  void yyerrorline(string,int);
%}

%union {
  char* pid;
  long long nb;
  int line;
}
%token <pid> pidentifier
%token <nb> num

%token VAR BEG END
%token PLUS MINUS TIMES DIV MOD
%token EQ NEQ LE GE LEQ GEQ
%token ASSIGN READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE ENDWHILE REPEAT UNTIL FOR ENDFOR DO
%token FROM TO DOWNTO

%%
program:
  VAR declarations BEG commands END {printSymbols(); }
| BEG commands END { printSymbols(); }
;

declarations:
  declarations ',' pidentifier {if (getSymbol($3)==0) putSymbol($3); else yyerrorline("Zmienna istnieje!",yylval.line);}
| declarations ',' pidentifier '[' num ':' num ']' {if (getSymbol($3)==0) putSymbolTable($3,$5,$7); else yyerrorline("Zmienna istnieje!",yylval.line); }
| pidentifier {if (getSymbol($1)==0) putSymbol($1); else yyerrorline("Zmienna istnieje!",yylval.line); }
| pidentifier '[' num ':' num ']' {if (getSymbol($1)==0) putSymbolTable($1,$3,$5); else yyerrorline("Zmienna istnieje!",yylval.line); }
;

commands:
  commands command {}
| command {}
;

command:
  identifier ASSIGN expression ';' {}
| IF condition THEN commands ELSE commands ENDIF {}
| IF condition THEN commands ENDIF {}
| WHILE condition DO commands ENDWHILE {}
| REPEAT commands UNTIL condition ';' {}
| FOR pidentifier FROM value TO value DO commands ENDFOR {}
| FOR pidentifier FROM value DOWNTO value DO commands ENDFOR {}
| READ identifier ';' {}
| WRITE value ';' {}
;

expression:
  value {}
| value PLUS value {}
| value MINUS value {}
| value TIMES value {}
| value DIV value {}
| value MOD value {}
;

condition: 
  value EQ value {}
| value NEQ value {}
| value LE value {}
| value GE value {}
| value LEQ value {}
| value GEQ value {}
;

value:
  num {}
| identifier {}

identifier:
  pidentifier {}
| pidentifier '[' pidentifier ']' {}
| pidentifier '[' num ']' {}
%%

void yyerrorline(string e, int i) {
    cerr<<"Linia "<<i<<" "<<e<<endl;
}

void yyerror(char* s) {
    printf("Błąd: %s\n",s);
}

int main() {
  yyparse();
}
