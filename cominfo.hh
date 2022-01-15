//Szymon Pilawka

/* Typ wyliczeniowy przechowuje informacje jaką komende przechowuje struktura */
//root oznacza "korzeń" - 
enum comInfoType {c_IF,c_IFELSE,c_WHILE,c_REPEAT,c_FORTO
,c_FORDOWNTO,c_ASSIGN,c_READ,c_WRITE,c_ROOT};

struct cominfo {
    enum comInfoType type;
    struct cominfo* parent;
    struct cominfo* next;

    struct cominfo** children;
    long long ID;
    long long line;
};

typedef struct cominfo cominfo;

vector<cominfo*> comInfos = {};

cominfo* genComInfo(enum comInfoType t, long long ID) {
    cominfo* ci = new cominfo;
    ci->type = t;
    ci->parent = 0;
    ci->children = new cominfo*;
    *(ci->children) = 0;
    ci->ID = ID;

    cominfos.push_back(ci);
    return ci;
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

void printChildren(cominfo* parent) {
    cominfo* ptr = *(parent->children);
    while (ptr != 0) {
        cout<<"{"<<ptr->type<<" "<<ptr->ID;
        printChildren(ptr);
        cout<<"}"<<endl;
        ptr = ptr->next;
    }
}