//Szymon Pilawka 254649
/** Plik definiuje strukturę przechowującą zmienne z programu */

#include <string.h>
enum valInfoType {NUM,ELEM,TELEM,TELEMID};

struct valinfo {
    enum valInfoType type;
    long long num;
    char* varName;
    long long index;
    char* indexName;
    long long line;
};

typedef struct valinfo valinfo;

vector<valinfo*> valInfos = {};

//różne "konstruktory"

valinfo* makeValinfoNum(long long value, long long line){
    valinfo* vi = new valinfo();
    vi->type = NUM;
    vi->num = value;
    vi->line = line;

    valInfos.push_back(vi);
    return vi;
}

valinfo* makeValinfoElem(char* name, long long line){
    valinfo* vi = new valinfo();
    vi->type = ELEM;
    vi->varName = name;
    vi->line = line;

    valInfos.push_back(vi);
    return vi;
}

valinfo* makeValinfoTElem(char* name, long long index, long long line){
    valinfo* vi = new valinfo();
    vi->type = TELEM;
    vi->varName = name;
    vi->index = index;
    vi->line = line;

    valInfos.push_back(vi);
    return vi;
}

valinfo* makeValinfoTElemID(char* name, char* index, long long line){
    valinfo* vi = new valinfo();
    vi->type = TELEMID;
    vi->varName = name;
    vi->indexName = index;
    vi->line = line;
    
    valInfos.push_back(vi);
    return vi;
}

bool isTheSameVal(valinfo* v1, valinfo* v2) {
    if (v1->type != v2->type) return false;

    switch (v1->type) {
        case NUM:
            if (v1->num == v2->num) return true;
            return false;
        case ELEM:
            if (strcmp(v1->varName,v2->varName)==0) return true;
            return false;
        case TELEM:
            if (strcmp(v1->varName,v2->varName)==0 && v1->index == v2->index) return true;
            return false;
        case TELEMID:
            if (strcmp(v1->varName,v2->varName)==0 && strcmp(v1->indexName,v2->indexName)==0) return true;
            return false;
    }
    
    return false;
}
 
void printVal(valinfo* v) {
    switch(v->type) {
        case NUM: cout<<v->num; break;
        case ELEM: cout<<v->varName; break;
        case TELEM: cout<< v->varName<<"["<<v->index<<"]"; break;
        case TELEMID: cout<<v->varName<<"["<<v->indexName<<"]"; break;
    }
}