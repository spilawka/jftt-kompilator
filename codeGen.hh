#include <stack>
#include "mcInstructions.hh"


void checkVal(cominfo* c, valinfo* v) {
    if (v->type == NUM) return;

    if (v->type == ELEM) {

        if (!hasSymbol(v->varName)) {
            if (!checkIfParentsHaveSymbol(c,v->varName))
                yyerrorline("Zmienna "+string(v->varName)+" niezadeklarowana!",c->line);
        }
        else {
            struct varData* s = getSymbol(v->varName);
            if (s->isTable)
                yyerrorline("Błędne użycie tablicy "+string(v->varName),c->line);
            if (s->init == false) 
                yyerrorline("Zmienna "+string(v->varName)+" niezainicjonowana!",c->line);
        } 
    }

    else if (v->type == TELEM) {
        if (!hasSymbol(v->varName)) {
            yyerrorline("Tablica "+string(v->varName)+" niezadeklarowana!",c->line);
        }
        else {
            if (!getSymbol(v->varName)->isTable)
                yyerrorline("Błędne użycie zmiennej "+string(v->varName),c->line);
            if (!isInBounds(getSymbol(v->varName),v->index))
                yyerrorline("Wyjście poza zakres tablicy!",c->line);
        } 
    }

    else if (v->type == TELEMID) {
        if (!hasSymbol(v->varName)) {
            yyerrorline("Tablica "+string(v->varName)+" niezadeklarowana!",c->line);
        }
        else {
            if (!getSymbol(v->varName)->isTable)
                yyerrorline("Błędne użycie zmiennej "+string(v->varName),c->line);

            if (!hasSymbol(v->indexName)) {
                if (!checkIfParentsHaveSymbol(c,v->indexName))
                    yyerrorline("Zmienna "+string(v->indexName)+" niezadeklarowana!",c->line);
                }
            else {
                struct varData* s = getSymbol(v->indexName);
                if (s->isTable)
                    yyerrorline("Błędne użycie tablicy "+string(v->indexName),c->line);
                if (s->init == false) 
                    yyerrorline("Zmienna "+string(v->indexName)+" niezainicjonowana!",c->line);
            } 
        }
    }
}

void checkExpr(cominfo* c, exprinfo* e) {
    checkVal(c,e->v1);
    if (e->type != e_SOLO) checkVal(c,e->v2);
}

void checkCond(cominfo* c, condinfo* e) {
    checkVal(c,e->v1);
    checkVal(c,e->v2);
}

void checkVariables(cominfo* c) {
    if (c==0) return;
    
    switch (c->type) {
        case c_FORTO: case c_FORDOWNTO:
            checkIfChildrenHaveSymbol(c,c->ifvar->name,c->line);
        break;
        case c_ASSIGN:
            if(!hasSymbol(c->vi->varName)) {
                if (checkIfParentsHaveSymbol(c,c->vi->varName))
                    yyerrorline("Niedozwolona modyfikacja zmiennej "+string(c->vi->varName),c->line);
                else
                    yyerrorline("Niezadeklarowana zmienna "+string(c->vi->varName),c->line);
            }
            else {
                getSymbol(c->vi->varName)->init = true;
            }
            checkVal(c,c->vi);
            checkExpr(c,c->ei);
        break;
        case c_IF: case c_IFELSE: case c_WHILE: case c_REPEAT:
            checkCond(c,c->ci);    
        break;
        case c_READ:
            if(!hasSymbol(c->vi->varName)) {
                yyerrorline("Niezadeklarowana zmienna "+string(c->vi->varName),c->line);
            }
            getSymbol(c->vi->varName)->init = true;
        break;
        case c_WRITE:
            checkVal(c,c->vi);
        break;
    }

    checkVariables(*(c->children));
    checkVariables(c->next);
}

    

typedef stack<pair<enum instr, enum reg>> insstack;
/** Funkcja zwraca kod do generowania danej liczby od podstaw w kodzie maszynowym 
    @return stos LILO z kodem instrukcji oraz modyfikowanym rejestrem

    REJESTRY: A,B
*/
void genNumber(long long num) {
    //stack z kodem maszynowym
    insstack code = {};

    //małe liczby (abs(n)<10)
    if (num == 0) {
        code.push(make_pair(RESET,A));
    }
    else if (num > 0 && num < 10) {
        for (int i=0; i<num; i++)
            code.push(make_pair(INC,A));
        code.push(make_pair(RESET,A));
    }
    else if (num > -10 && num < 0) {
        for (int i=0; i>num; i--) 
            code.push(make_pair(DEC,A));
        code.push(make_pair(RESET,A));
    }
    else {
        //liczby ujemne - weź odwrotność i odejmij pod sam koniec
        if (num<0) {
            num = -num;
            code.push(make_pair(SUB,B));
            code.push(make_pair(RESET,A));
            code.push(make_pair(SWAP,B));
        }

        //ilość występowania prostych wymnożeń razy dwa
        long long conseq2 = 0;
        while (num>0) {
            long long modulo = num%2;

            if (modulo == 0) {
                conseq2++;
            }
            else {
                if (conseq2==1) {
                    code.push(make_pair(SHIFT,B));
                }
                // dla więcej operacji optymalizacja - bardziej
                // opłaca się zwiększać b niż wykonywać SHIFTy
                else if (conseq2>1) {
                    code.push(make_pair(INC,B));
                    code.push(make_pair(RESET,B));
                    code.push(make_pair(SHIFT,B));
                    for(int i=1;i<conseq2;i++){
                        code.push(make_pair(INC,B));
                    }
                }
                conseq2=0;

                code.push(make_pair(INC,A));
                if (num!=1)
                    code.push(make_pair(SHIFT,B));
            }

            num = num/2;
        }

        //reset rejestrów
        code.push(make_pair(INC,B));
        code.push(make_pair(RESET,B));
        code.push(make_pair(RESET,A));
    }
    /*
    while (!code.empty()) {
        pair<enum instr, enum reg> elem = code.top();
        struct reference r;
        r.isFinal = true;
        r.isRegistry = true;
        r.Registry = elem.second;
        
        midCode.push_back(make_pair(elem.first,r));

        code.pop();
    }
    */
}