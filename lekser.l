num -?[0-9]+
pid [_a-z]+

%{
    //TODO #include tablica znakow
    //void yyerror(char*,int)
    #include <stdio.h>
    #include <stdlib.h>
    int lines = 0;
%}

%x KOM

%%
"( )" |
[ \t] ;

\n { lines++; }

"\( " {
    BEGIN(KOM);
}

<KOM>("\( "([^" \)"])*" \)")|"( )" {
    //yyerror("Zagnieżdżony komentarz!",lines);
    printf("Zagnieżdżony komentarz!\n");
}

<KOM>" \)" {
    BEGIN(INITIAL);
}

<KOM>. ;

PLUS|MINUS|TIMES|DIV|MOD |
EQ|NEQ|LE|GE|LEQ|GEQ |
ASSIGN |
IF|THEN|ELSE|ENDIF |
WHILE|DO|ENDWHILE|REPEAT |
UNTIL|FROM|TO|DOWNTO |
BEGIN|END|VAR {
    printf("kom: %s\n",yytext);
    //return *yytext;
}

[\[\]:,] {
    printf("cf: %s\n",yytext);
    //return *yytext;
}

{num} {
    printf("num: %s\n",yytext);
    //yylval = atoll(yytext);
    //return num;
}

{pid} {
    printf("pid: %s\n",yytext);
    //yylval = yytext;
    //return pidentifier;
}

. {
    printf("Nierozpoznawalny symbol: Linia %d",lines);
}

%%

int yywrap(void) {
    return 1;
}