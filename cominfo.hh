enum comInfoType {c_IF,c_IFELSE,c_WHILE,c_REPEAT,c_FORTO
,c_FORDOWNTO,c_ASSIGN,c_READ,c_WRITE,c_ROOT};

struct cominfo {
    enum comInfoType type;
    struct cominfo* parent;
    struct cominfo* next;

    struct cominfo** children;
    long long ID;
};

vector<struct cominfo*> comInfos = {};

struct cominfo* genComInfo(enum comInfoType t, long long ID) {
    struct cominfo* ci = new struct cominfo;
    ci->type = t;
    ci->parent = 0;
    ci->children = new struct cominfo*;
    *(ci->children) = 0;
    ci->ID = ID;

    return ci;
}

void insertChildren(struct cominfo* parent, struct cominfo** children) {
    if (*(parent->children)==0)
        *(parent->children) = *children;
    else {
        struct cominfo* t = *(parent->children);
        while (t->next != 0) t = t->next;

        t->next = *children;
    }

    struct cominfo* ptr = *children;
    while (ptr != 0) {
        ptr->parent = parent;
        ptr = ptr->next;
    }
}

void printChildren(struct cominfo* parent) {
    struct cominfo* ptr = *(parent->children);
    while (ptr != 0) {
        cout<<ptr->type<<" "<<ptr->ID<<endl;
        ptr = ptr->next;
    }
}