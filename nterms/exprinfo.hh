//Szymon Pilawka 254649
/** Plik zawiera definicji encji przechowującej informacje o danym wyrażeniu z programu */
/** Rodzaje wyrażeń */
enum exprInfoTypes {e_SOLO,e_MINUS,e_PLUS,e_TIMES,e_DIV,e_MOD};

struct exprinfo {
    enum exprInfoTypes type;
    valinfo* v1;
    valinfo* v2;
};

typedef struct exprinfo exprinfo;

vector<exprinfo*> exprInfos = {};

exprinfo* createExprInfo(valinfo* v1, enum exprInfoTypes type, valinfo* v2) {
    exprinfo* e = new exprinfo();
    e->v1 = v1;
    e->v2 = v2;
    e->type = type;

    exprInfos.push_back(e);
    return e;
}