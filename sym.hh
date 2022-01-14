struct varData {
    bool isTable;
    long long start;
    long long end;
};

typedef struct varData varData;

map<string,varData> varTable = {};

bool hasSymbol(char* symname){
    return varTable.count(string(symname))==1;
}

bool putSymbol(char* symname) {
    string str(symname);

    if (hasSymbol(symname)) return false;

    //nowy symbol
    varData s;
    s.isTable = false;
    varTable[str] = s;
    return true;
}

bool putSymbolTable(char* symname, long long start, long long end) {
    string str(symname);

    auto find = varTable.find(str);
    if (find != varTable.end()) return false;

    //nowy symbol
    varData s;
    s.isTable = true;
    s.start = start;
    s.end = end;

    varTable[str] = s;
    return true;
}

varData getSymbol(char* symname) {
    return varTable[string(symname)];
}

bool isInBounds(varData vd, long long id) {
    if (!vd.isTable) return false;

    if (id < vd.start || id > vd.end) return false;
    return true;
}

void printSymbols() {
    for (auto const& s: varTable) {
        if (s.second.isTable) cout<<s.first<<"["<<s.second.start<<":"<<s.second.end<<"] ";
        else cout<<s.first<<" ";
    }
}