#include <string.h>
#include <stdbool.h>
#include <string>
#include <stdio.h>

using namespace std;

struct symbol {
    string name;
    bool isTable;
    long long start;
    long long end;
    struct symbol* next;
};

struct symbol* table = 0;

struct symbol* putSymbol(char* symname) {
    //nowy symbol
    struct symbol* s = new struct symbol();
    s->name = string(symname);
    s->isTable = false;
    s->next = table;
    table = s;
    return s;
}

struct symbol* putSymbolTable(char* symname, long long start, long long end) {
    struct symbol* s = new struct symbol();
    s->name = string(symname);
    s->isTable = true;
    s->start = start;
    s->end = end;
    s->next = table;
    table = s;
    return s;
}

struct symbol* getSymbol(char* symname) {
    struct symbol* s = table;
    while(s != 0) {
        if (s->name == symname)
            return s;
        
        s = s->next;
    }
    return 0;
}

bool isInBounds(struct symbol* tn, long long id) {
    if (!tn->isTable) return false;

    if (id < tn->start || id > tn->end) return false;
    return true;
}

void printSymbols() {
    struct symbol* s = table;
    printf("Symbols:\n");
    while (s!=0) {
        if (!s->isTable) {
            cout<<' '<<s->name;
        }
        else {
            cout<<' '<<s->name<<'['<<s->start<<':'<<s->end<<']';
        }
        s = s->next;
    }
}