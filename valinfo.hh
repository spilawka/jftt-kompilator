enum valInfoType {NUM,ELEM,TELEM,TELEMID};

struct valinfo {
    enum valInfoType type;
    long long num;
    char* varName;
    long long index;
    char* indexName;
    long long line;
    bool init;
};

typedef struct valinfo valinfo;

vector<valinfo*> valInfos = {};

valinfo* makeValinfoNum(long long value, long long line){
    valinfo* vi = new valinfo();
    vi->type = NUM;
    vi->num = value;
    vi->line = line;
    vi->init = false;

    valInfos.push_back(vi);
    return vi;
}

valinfo* makeValinfoElem(char* name, long long line){
    valinfo* vi = new valinfo();
    vi->type = ELEM;
    vi->varName = name;
    vi->line = line;
    vi->init = false;

    valInfos.push_back(vi);
    return vi;
}

valinfo* makeValinfoTElem(char* name, long long index, long long line){
    valinfo* vi = new valinfo();
    vi->type = TELEM;
    vi->varName = name;
    vi->index = index;
    vi->line = line;
    vi->init = false;

    valInfos.push_back(vi);
    return vi;
}

valinfo* makeValinfoTElemID(char* name, char* index, long long line){
    valinfo* vi = new valinfo();
    vi->type = TELEMID;
    vi->varName = name;
    vi->indexName = index;
    vi->line = line;
    vi->init = false;
    
    valInfos.push_back(vi);
    return vi;
}
 
