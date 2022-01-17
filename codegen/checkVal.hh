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

    long long pcfID = cfID;

    checkVariables(*(c->children));

    cfID = pcfID;
    checkVariables(c->next);
} 