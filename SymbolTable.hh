enum Range {GLOBAL,LOCAL};

struct varData {
    bool isTable;
    long long start;
    long long end;
    bool init;
    enum Range range;
};

typedef struct varData varData;

map<string,varData*> varTable = {};

bool hasSymbol(char* symname){
    return varTable.count(string(symname))==1;
}

bool hasSymbolString(string symname){
    return varTable.count(symname)==1;
}

bool putSymbol(char* symname, enum Range r)  {
    string str(symname);

    if (hasSymbol(symname)) return false;

    //nowy symbol
    varData* s = new varData;
    s->isTable = false;
    s->range = r;
    s->init = false;
    varTable[str] = s;
    return true;
}

bool putSymbolTable(char* symname, long long start, long long end, enum Range r) {
    string str(symname);

    auto find = varTable.find(str);
    if (find != varTable.end()) return false;

    //nowy symbol
    varData* s = new varData;
    s->isTable = true;
    s->start = start;
    s->end = end;
    s->range = r;

    varTable[str] = s;
    return true;
}

varData* getSymbol(char* symname) {
    return varTable[string(symname)];
}

varData* getSymbolString(string symname) {
    return varTable[symname];
}

bool isInBounds(varData* vd, long long id) {
    if (!vd->isTable) return false;

    if (id < vd->start || id > vd->end) return false;
    return true;
}

void printSymbols() {
    for (auto const& s: varTable) {
        if (s.second->isTable) cout<<s.first<<"["<<s.second->start<<":"<<s.second->end<<"] ";
        else cout<<s.first<<" ";
    }
    cout<<endl;
}