enum valInfoType {NUM,ELEM,TELEM,TELEMID};

struct valinfo {
    enum valInfoType type;
    long long num;
    char* varName;
    long long index;
    char* indexName;
};

struct valinfo makeValinfoNum(long long value){
    struct valinfo vi;
    vi.type = NUM;
    vi.num = value;
    return vi;
}

struct valinfo makeValinfoElem(char* name){
    struct valinfo vi;
    vi.type = ELEM;
    vi.varName = name;
    return vi;
}

struct valinfo makeValinfoTElem(char* name, long long index){

    struct valinfo vi;
    vi.type = TELEM;
    vi.varName = name;
    vi.index = index;
    return vi;
}

struct valinfo makeValinfoTElemID(char* name, char* index){
    struct valinfo vi;
    vi.type = TELEMID;
    vi.varName = name;
    vi.indexName = index;
    return vi;
}
 
