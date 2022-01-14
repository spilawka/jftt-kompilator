%{
  #include <stdio.h>
  #include <string>
  #include <stdbool.h>
  #include <map>
  #include <iostream>
  #include <utility>
  #include <vector>

  using namespace std;

  #include "vmInstructions.hh"
  #include "sym.hh"
  #include "codeGen.hh"

  int yylex(void);
  void yyerror(string);
  void yyerrorline(string,long long);

  long long globalID = 0;

  struct cfinfo {
    long long ID;
  };

  struct varinfo {
    bool constant;
    bool init;
    string name;
    bool isTableElem;
    long long index;
  };

  typedef struct varinfo* vip;

  vector<vip> varInfos;

  vip makeVarInfo(bool constant, long long value, char* name) {
    vip s = new struct varinfo;
    s->constant = constant;
    s->init = false;
    s->name = string(name);

    varInfos.push_back(s);
    return s;
  }
%}

%union {
  char* pid;
  long long nb;
  long long line;
  vip myvip;
}
%token <pid> pidentifier
%token <nb> num
%nterm <myvip> identifier
%nterm <myvip> value

%token VAR BEG END
%token PLUS MINUS TIMES DIV MOD
%token EQ NEQ LE GE LEQ GEQ
%token ASSIGN READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE ENDWHILE REPEAT UNTIL FOR ENDFOR DO
%token FROM TO DOWNTO

%%
program:
  VAR declarations BEG commands END {printSymbols();}
| BEG commands END {printSymbols();}
;

declarations:
  declarations ',' pidentifier 
    { if (!putSymbol($3)) yyerrorline("Zmienna istnieje!",yylval.line); }
| declarations ',' pidentifier '[' num ':' num ']'
    { if($5>$7) yyerrorline("Błędny zakres tablicy "+string($3),yylval.line); if (!putSymbolTable($3,$5,$7)) yyerrorline("Zmienna istnieje!",yylval.line); }
| pidentifier 
    { if (!putSymbol($1)) yyerrorline("Zmienna istnieje!",yylval.line); }
| pidentifier '[' num ':' num ']' 
    { if($3>$5) yyerrorline("Błędny zakres tablicy "+string($1),yylval.line); if (!putSymbolTable($1,$3,$5)) yyerrorline("Zmienna istnieje!",yylval.line); }
;

commands:
  commands command {}
| command {}
;

command:
  identifier ASSIGN expression ';' {
    $1->init = true;
  }
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
  value {
    if (!$1->init) yyerrorline("Zmienna "+$1->name+" nie została zainicjowana!",yylval.line);
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
    $$ = makeVarInfo(true,$1,0);
  }
| identifier {
    $$ = $1;
  }

identifier:
  pidentifier {
    if (!hasSymbol($1)) yyerrorline("Zmienna nie została zadeklarowana!",yylval.line);
    $$ = makeVarInfo(false,0,$1);
  }
| pidentifier '[' pidentifier ']' {
    if (!hasSymbol($1)) yyerrorline("Zmienna nie została zadeklarowana!",yylval.line);
    $$ = makeVarInfo(false,0,$1);
  }
| pidentifier '[' num ']' {
    if (!hasSymbol($1)) yyerrorline("Zmienna nie została zadeklarowana!",yylval.line);
    if (!isInBounds(getSymbol($1),$3)) yyerrorline("Wyjście poza zakres tablicy!",yylval.line);
    $$ = makeVarInfo(false,0,$1);
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
