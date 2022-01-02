num -?[0-9]+
pid [_a-z]+

%{
    #include "parser.tab.c"
    void yyerror(char*);
    #include <stdio.h>
    #include <stdlib.h>
    int lines = 0;
%}

%x KOM

%%
[ \t] ;

\n { lines++; }

"\(" {
    BEGIN(KOM);
}

<KOM>("\("([^"\)"])*"\)") {
    yyerror("Zagnieżdżony komentarz!");
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
    printf("Nierozpoznawalny token: %s, Linia %d",yytext,lines);
}

%%

int yywrap(void) {
    return 1;
}