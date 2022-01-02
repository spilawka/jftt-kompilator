%{
  #include <stdio.h>
  int yylex(void);
  void yyerror(char*);
%}

%union {
  char* pid;
  long long nb;
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
  VAR declarations BEG commands END {}
| BEG commands END {}
;

declarations:
  declarations ',' pidentifier
| declarations ',' pidentifier '[' num ':' num ']'
| pidentifier { printf("zmienna: %s",$1); }
| pidentifier '[' num ':' num ']'
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

void yyerror(char* s) {
    printf("Błąd: %s\n",s);
}

int main() {
  yyparse();
}
