%{
    int yylex(void);
    void yyerror(char*,int);
%}

%define api.value.type {long long}
%token pidentifier
%token num

%%
program:
  "VAR" declarations "BEGIN" commands "END"
| "BEGIN" commands "END"
;

declarations:
  declarations ',' pidentifier
| declarations ',' pidentifier '[' num ':' num ']'
| pidentifier
| pidentifier '[' num ':' num ']'
;

commands:
  commands command
| command
;

command:
  identifier "ASSIGN" expression;
| "IF" condition "THEN" commands "ELSE" commands "ENDIF"
| "IF" condition "THEN" commands "ENDIF"
| "WHILE" condition "DO" commands "ENDWHILE"
| "REPEAT" commands "UNTIL" condition
| "FOR" pidentifier "FROM" value "TO" value "DO" commands "ENDFOR"
| "FOR" pidentifier "FROM" value "DOWNTO" value "DO" commands "ENDFOR"
| "READ" identifier
| "WRITE" value
;

expression:
  value
| value "PLUS" value
| value "MINUS" value
| value "TIMES" value
| value "DIV" value
| value "MOD" value
;

condition: 
  value "EQ" value
| value "NEQ" value
| value "LE" value
| value "GE" value
| value "LEQ" value
| value "GEQ" value
;

value:
  num
| identifier

identifier:
  pidentifier
| pidentifier '[' pidentifier ']'
| pidentifier '[' num ']'
%%