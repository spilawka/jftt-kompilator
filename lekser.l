num -?[0-9]+
pid [_a-z]+

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string>
    #include "parser.h"

    void yyerror(char*);
    void yyerrorline(std::string, long long);
%}

%option yylineno
%x KOM

%%
[ \t] ;

\n ;

"\(" {
    BEGIN(KOM);
}

<KOM>("\("([^"\)"])*"\)") {
    yyerrorline("Zagnieżdżony komentarz!",yylineno);
}

<KOM>"\)" {
    BEGIN(INITIAL);
}

<KOM>. ;

PLUS { return PLUS; }
MINUS { return MINUS; }
TIMES { return TIMES; }
DIV { return DIV; }
MOD { return MOD; }

EQ { return EQ; }
NEQ { return NEQ; }
LE { return LE; }
GE { return GE; }
LEQ { return LEQ; }
GEQ { return GEQ; }

ASSIGN { return ASSIGN; }
IF { return IF; }
THEN { return THEN; }
ELSE { return ELSE; }
ENDIF { return ENDIF; }

WHILE { return WHILE; }
DO { return DO; }
ENDWHILE { return ENDWHILE; }
REPEAT { return REPEAT; }
FOR { return FOR; }
ENDFOR { return ENDFOR; }

UNTIL { return UNTIL; }
FROM { return FROM; }
TO { return TO; }
DOWNTO { return DOWNTO; }

BEGIN { return BEG; }
END { return END; }
VAR { return VAR; }
WRITE { return WRITE; }
READ { return READ; }

[\[\]:,;] {
    return yytext[0];
}

{num} {
    yylval.nb = atoll(yytext);
    return num;
}

{pid} {
    yylval.pid = strdup(yytext);
    return pidentifier;
}

. {
    std::string s("Nierozpoznawalny token: ");
    s += yytext;
    yyerrorline(s,yylineno);
}

%%

int yywrap(void) {
    return 1;
}