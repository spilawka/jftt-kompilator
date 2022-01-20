//Szymon Pilawka
#include<string.h>

/* Typ wyliczeniowy przechowuje informacje jaką komende przechowuje struktura */
//root oznacza "korzeń" - 
enum comInfoType {c_IF,c_IFELSE,c_WHILE,c_REPEAT,c_FORTO
,c_FORDOWNTO,c_ASSIGN,c_READ,c_WRITE,c_ROOT};

string comNames[] = {"if","ifelse","while","repeat","for","fordown","assign","read","write","root"};

struct comvar {
    char* name;
    valinfo* from;
    valinfo* to;
};

struct comvar* makeComvar(char* name, valinfo* from, valinfo* to) {
    struct comvar* c = new struct comvar;
    c->name = name;
    c->from = from;
    c->to = to;
    return c;
}

struct cominfo {
    enum comInfoType type;
    long long ID;
    long long line;
    long long sep;

    struct cominfo* parent;
    struct cominfo* next;
    struct cominfo** children;

    struct comvar* ifvar;
    condinfo* ci;
    exprinfo* ei;
    valinfo* vi;
};
typedef struct cominfo cominfo;

vector<cominfo*> comInfos = {};
map<long long,cominfo*> cfParents;

cominfo* genComInfo(enum comInfoType t, long long ID, long long line) {
    cominfo* ci = new cominfo;
    ci->type = t;
    ci->parent = 0;
    ci->line = line;
    ci->children = new cominfo*;
    *(ci->children) = 0;
    ci->ID = ID;

    comInfos.push_back(ci);
    if (ID != 0) cfParents[ID] = ci;
    return ci;
}

cominfo* getClosestLoopParent(long long ID) {
    cominfo* p = cfParents[ID];

    while(p!=0 && !(p->type == c_WHILE || p->type == c_REPEAT || p->type == c_FORTO || p->type == c_FORDOWNTO)) {
        p = p->parent;
    }

    return p;
}

cominfo* insertComInfoData(cominfo* com,struct comvar* ifvar, condinfo* ci, exprinfo* ei, valinfo* vi) {
    com->ifvar = ifvar;
    com->ci = ci;
    com->ei = ei;
    com->vi = vi;
    return com;
}

bool isChildOf(cominfo* c, long long ID) {
    cominfo* p = c->parent;
    while (c!=0) {
        if (c->ID == ID) return true;
        c = c->parent;
    }
    return false;
}

void checkIfChildrenHaveSymbol(cominfo* par,char* sym, long long line) {
    cominfo* ch = *(par->children);

    while (ch!=0) {
        checkIfChildrenHaveSymbol(ch,sym,line);

        if (ch->type == c_FORDOWNTO || ch->type == c_FORTO) {
            if (strcmp(ch->ifvar->name,sym) == 0) {
                yyerrorline("Zmienna "+string(sym)+" już została zadeklarowana w pętli.",line);
            }
        }

        ch = ch->next;
    }
}

bool checkIfParentsHaveSymbol(cominfo* c,char* sym) {
    cominfo* p = c->parent;

    while (p!=0) {
        if (p->type == c_FORDOWNTO || p->type == c_FORTO) {
            if (strcmp(p->ifvar->name,sym) == 0)
                return true;
        }
        p = p->parent;
    }
    return false;
}

void insertChildren(cominfo* parent, cominfo** children) {
    if (*(parent->children)==0)
        *(parent->children) = *children;
    else {
        cominfo* t = *(parent->children);
        while (t->next != 0) t = t->next;

        t->next = *children;
    }

    cominfo* ptr = *children;
    while (ptr != 0) {
        ptr->parent = parent;
        ptr = ptr->next;
    }
}

long long getChildrenLenght(cominfo* parent) {
    cominfo* ptr = *(parent->children);
    long long nb = 0;
    while (ptr != 0) {
        nb++;
        ptr = ptr->next;
    }
    return nb;
}

void printChildren(cominfo* parent, int tabn) {
    cominfo* ptr = *(parent->children);
    while (ptr != 0) {
        for(int i=0;i<tabn;i++) cout<<"  ";
        
        cout<<comNames[ptr->type];
        cout<<"["<<ptr->ID<<"]";
        cout<<endl;
        
        printChildren(ptr,tabn+1);
        
        ptr = ptr->next;
    }
}