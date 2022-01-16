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
  extern int yylineno;

  #include "vmInstructions.hh"
  #include "sym.hh"
  #include "valinfo.hh"
  #include "exprinfo.hh"
  #include "condinfo.hh"
  #include "cominfo.hh"
  #include "codeGen.hh"
%}

%locations

%union {
  char* pid;
  long long nb;
  valinfo* vi;
  cominfo* ci;
  cominfo** cip;
  exprinfo* ei;
  condinfo* cdi;
}
%token <pid> pidentifier
%token <nb> num
%nterm <vi> identifier
%nterm <vi> value
%nterm <ci> command
%nterm <cip> commands
%nterm <ei> expression
%nterm <cdi> condition

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
    cominfo* root = genComInfo(c_ROOT,cfID++);
    insertChildren(root,$4);
    printChildren(root,0);
  }
| BEG commands END {
    printSymbols();
  }
;

declarations:
  declarations ',' pidentifier 
    { if (!putSymbol($3,GLOBAL)) yyerrorline("Zmienna istnieje!",yylineno); }
| declarations ',' pidentifier '[' num ':' num ']'
    { if($5>$7) yyerrorline("Błędny zakres tablicy "+string($3),yylineno); if (!putSymbolTable($3,$5,$7,GLOBAL)) yyerrorline("Talica istnieje!",yylineno); }
| pidentifier 
    { if (!putSymbol($1,GLOBAL)) yyerrorline("Zmienna istnieje!",yylineno); }
| pidentifier '[' num ':' num ']' 
    { if($3>$5) yyerrorline("Błędny zakres tablicy "+string($1),yylineno); if (!putSymbolTable($1,$3,$5,GLOBAL)) yyerrorline("Tablica istnieje!",yylineno); }
;

commands:
  commands command {
    cominfo* t = *$1;
    while (t->next != 0) t=t->next;
    t->next = $2;
    $$ = $1;
  }
| command {
    $$ = new cominfo*;
    $1->next = 0;
    *$$ = $1;
  }
;

command:
  identifier ASSIGN expression ';' {
    $$ = genComInfo(c_ASSIGN,0);
    insertComInfoData($$,0,0,$3,$1);
  }
| IF condition THEN commands ELSE commands ENDIF {
    $$ = genComInfo(c_IFELSE,cfID++);
    insertComInfoData($$,0,$2,0,0);
    insertChildren($$,$4);
    insertChildren($$,$6);
  }
| IF condition THEN commands ENDIF {
    $$ = genComInfo(c_IF,cfID++);
    insertComInfoData($$,0,$2,0,0);
    insertChildren($$,$4);
  }
| WHILE condition DO commands ENDWHILE {
    $$ = genComInfo(c_WHILE,cfID++);
    insertComInfoData($$,0,$2,0,0);
    insertChildren($$,$4);
  }
| REPEAT commands UNTIL condition ';' {
    $$ = genComInfo(c_REPEAT,cfID++);
    insertComInfoData($$,0,$4,0,0);
    insertChildren($$,$2);
  }
| FOR pidentifier FROM value TO value DO commands ENDFOR {
    $$ = genComInfo(c_FORTO,cfID++);
    insertComInfoData($$,makeComvar($2,$4,$6),0,0,0);
    insertChildren($$,$8);
  }
| FOR pidentifier FROM value DOWNTO value DO commands ENDFOR {
    $$ = genComInfo(c_FORDOWNTO,cfID++);
    insertComInfoData($$,makeComvar($2,$4,$6),0,0,0);
    insertChildren($$,$8);
  }
| READ identifier ';' {
    $$ = genComInfo(c_READ,0);
    insertComInfoData($$,0,0,0,$2);
  }
| WRITE value ';' {
    $$ = genComInfo(c_WRITE,0);
    insertComInfoData($$,0,0,0,$2);
  }
;

expression:
  value {
    $$ = createExprInfo($1,e_SOLO,0);
  }
| value PLUS value {
    $$ = createExprInfo($1,e_PLUS,$3);
  }
| value MINUS value {
    $$ = createExprInfo($1,e_MINUS,$3);
  }
| value TIMES value {
    $$ = createExprInfo($1,e_TIMES,$3);
  }
| value DIV value {
    $$ = createExprInfo($1,e_DIV,$3);
  }
| value MOD value {
    $$ = createExprInfo($1,e_MOD,$3);
  }
;

condition: 
  value EQ value {
    $$ = createCondInfo($1,c_EQ,$3);
  }
| value NEQ value {
    $$ = createCondInfo($1,c_NEQ,$3);
  }
| value LE value {
    $$ = createCondInfo($1,c_LE,$3);
  }
| value GE value {
    $$ = createCondInfo($1,c_GE,$3);
  }
| value LEQ value {
    $$ = createCondInfo($1,c_LEQ,$3);
  }
| value GEQ value {
    $$ = createCondInfo($1,c_GEQ,$3);
  }
;

value:
  num {
    $$ = makeValinfoNum($1,yylineno);
  }
| identifier {
    $$ = $1;
  }

identifier:
  pidentifier {
    $$ = makeValinfoElem($1,yylineno);

  }
| pidentifier '[' pidentifier ']' {
    $$ = makeValinfoTElemID($1,$3,yylineno);

  }
| pidentifier '[' num ']' {
    $$ = makeValinfoTElem($1,$3,yylineno);

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
