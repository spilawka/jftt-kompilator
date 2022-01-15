%{
  #include <stdio.h>
  #include <string>
  #include <stdbool.h>
  #include <map>
  #include <iostream>
  #include <utility>
  #include <vector>

  using namespace std;
  int yylex(void);
  void yyerror(string);
  void yyerrorline(string,long long);
  long long cfID = 1;
  long long condID = 1;

  #include "vmInstructions.hh"
  #include "sym.hh"
  #include "valinfo.hh"
  #include "cominfo.hh"
  #include "codeGen.hh"
%}

%union {
  char* pid;
  long long nb;
  long long line;
  struct valinfo vi;
  struct cominfo* ci;
  struct cominfo** cip;
}
%token <pid> pidentifier
%token <nb> num
%nterm <vi> identifier
%nterm <vi> value
%nterm <ci> command
%nterm <cip> commands

%token VAR BEG END
%token PLUS MINUS TIMES DIV MOD
%token EQ NEQ LE GE LEQ GEQ
%token ASSIGN READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE ENDWHILE REPEAT UNTIL FOR ENDFOR DO
%token FROM TO DOWNTO

%%
program:
  VAR declarations BEG commands END {
    printSymbols();
    struct cominfo* root = genComInfo(c_ROOT,cfID++);
    insertChildren(root,$4);
    printChildren(root);
  }
| BEG commands END {
    printSymbols();
  }
;

declarations:
  declarations ',' pidentifier 
    { if (!putSymbol($3,GLOBAL)) yyerrorline("Zmienna istnieje!",yylval.line); }
| declarations ',' pidentifier '[' num ':' num ']'
    { if($5>$7) yyerrorline("Błędny zakres tablicy "+string($3),yylval.line); if (!putSymbolTable($3,$5,$7,GLOBAL)) yyerrorline("Talica istnieje!",yylval.line); }
| pidentifier 
    { if (!putSymbol($1,GLOBAL)) yyerrorline("Zmienna istnieje!",yylval.line); }
| pidentifier '[' num ':' num ']' 
    { if($3>$5) yyerrorline("Błędny zakres tablicy "+string($1),yylval.line); if (!putSymbolTable($1,$3,$5,GLOBAL)) yyerrorline("Tablica istnieje!",yylval.line); }
;

commands:
  commands command {
    $2->next = *$$;
    *$$ = $2;
  }
| command {
    $$ = new struct cominfo*;
    $1->next = 0;
    *$$ = $1;
  }
;

command:
  identifier ASSIGN expression ';' {
    $$ = genComInfo(c_ASSIGN,0);
  }
| IF condition THEN commands ELSE commands ENDIF {
    $$ = genComInfo(c_IFELSE,cfID++);
    insertChildren($$,$4);
    insertChildren($$,$6);
  }
| IF condition THEN commands ENDIF {
    $$ = genComInfo(c_IF,cfID++);
    insertChildren($$,$4);
  }
| WHILE condition DO commands ENDWHILE {
    $$ = genComInfo(c_WHILE,cfID++);
    insertChildren($$,$4);
  }
| REPEAT commands UNTIL condition ';' {
    $$ = genComInfo(c_REPEAT,cfID++);
    insertChildren($$,$2);
  }
| FOR pidentifier FROM value TO value DO commands ENDFOR {
    $$ = genComInfo(c_FORTO,cfID++);
    insertChildren($$,$8);
  }
| FOR pidentifier FROM value DOWNTO value DO commands ENDFOR {
    $$ = genComInfo(c_FORDOWNTO,cfID++);
    insertChildren($$,$8);
  }
| READ identifier ';' {
    $$ = genComInfo(c_READ,0);
  }
| WRITE value ';' {
    $$ = genComInfo(c_WRITE,0);
  }
;

expression:
  value {

  }
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
  num {
    $$ = makeValinfoNum($1);
  }
| identifier {
    $$ = $1;
  }

identifier:
  pidentifier {
    //if (!hasSymbol($1)) yyerrorline("Zmienna "+string($1)+" nie została zadeklarowana!",yylval.line);
    $$ = makeValinfoElem($1);
  }
| pidentifier '[' pidentifier ']' {
    //if (!hasSymbol($1)) yyerrorline("Tablica "+string($1)+" nie została zadeklarowana!",yylval.line);
    //if (!hasSymbol($3)) yyerrorline("Zmienna "+string($3)+" nie została zadeklarowana!",yylval.line);
    $$ = makeValinfoTElemID($1,$3);
  }
| pidentifier '[' num ']' {
    //if (!hasSymbol($1)) yyerrorline("Tablica "+string($1)+" nie została zadeklarowana!",yylval.line);
    //if (!isInBounds(getSymbol($1),$3)) yyerrorline("Wyjście poza zakres tablicy: "+string($1),yylval.line);
    $$ = makeValinfoTElem($1,$3);
  }
%%

void yyerrorline(string e, long long i) {
    cerr<<"["<<i<<"]Error: "<<e<<endl;
    exit(-1);
}

void yyerror(string s) {
    cerr<<"Błąd: "<<s<<endl;
    exit(-1);
}

int main() {
  yyparse();
}
