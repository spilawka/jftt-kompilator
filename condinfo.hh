enum condInfoTypes {c_EQ,c_NEQ,c_LE,c_GE,c_LEQ,c_GEQ};

struct condinfo {
    enum condInfoTypes type;
    valinfo* v1;
    valinfo* v2;
};

typedef struct condinfo condinfo;

vector<condinfo*> condInfos = {};

condinfo* createCondInfo(valinfo* v1, enum condInfoTypes type, valinfo* v2) {
    condinfo* c = new condinfo();
    c->type = type;
    c->v1 = v1;
    c->v2 = v2;

    condInfos.push_back(c);
    return c;
}

void invertCondInfo(condinfo* c) {
    struct valinfo* temp = c->v1;
    c->v1 = c->v2;
    c->v2 = temp;

    switch (c->type) {
        case c_LE: c->type=c_GE; break;
        case c_GE: c->type=c_LE; break;
        case c_LEQ: c->type=c_GEQ; break;
        case c_GEQ: c->type=c_LEQ; break;
        default: break;
    }
}

