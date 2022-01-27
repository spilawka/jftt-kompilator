//Szymon Pilawka 254649
/** Funkcje w nagłówku odpowiadają za przydział rejestrów oraz pamięci dla zmiennych */

#include<queue>

/** Zasięg poszczególnych pętli oraz IFów */
map<long long,pair<long long,long long>> cfRange;
/** Wektory przechowujące lokalizację użyć zmiennej*/
map<string,vector<long long>> varUsage;
/** Zasięg występowania zmiennej */
map<string,pair<long,long>> varRange;

/** Waga występowania zmiennej (zagnieżdżenie w pętlach powoduje większy priorytet) */
typedef pair<string,long long> varprio;
auto cmp = [](varprio p1, varprio p2) { return p1.second < p2.second; };
priority_queue<varprio, vector<varprio>, decltype(cmp)> varPriority(cmp);

/** Zarezerwowane w pamięci miejsce */
long long allocmem = 0;

/** Instrukcja kodu pośredniego*/
struct MCInstr {
    vector<MCTag> tags;
    enum MCinstr instr;
};

/** Rodzaj docelowego argumentu*/
enum destTypes {d_EMPTY, d_1VAR, d_2VAR, d_REG, d_CONST, d_TAG};
/** Docelowy argument */
class MCDest {
public:
    enum destTypes type;
    //zmienne
    string var1;
    string var2;
    //różne operacje
    enum reg reg;
    //przeskoki
    long long n;
    MCTag tag;

    MCDest() {
        this->type = d_EMPTY;
    }

    MCDest(string s) {
        this->type = d_1VAR;
        this->var1 = s;
    }

    MCDest(string s, string s2) {
        this->type = d_2VAR;
        this->var1 = s;
        this->var2 = s2;
    }

    MCDest(enum reg reg) {
        this->type = d_REG;
        this->reg = reg;
    }

    MCDest(long long c){
        this->type = d_CONST;
        this->n = c;
    }

    MCDest(MCTag tag) {
        this->type = d_TAG;
        this->tag = tag;
    }

    void print() {
        switch(type) {
            case d_1VAR: cout<<var1; break;
            case d_2VAR: cout<<var1<<" "<<var2; break;
            case d_REG: cout<<regName[reg]; break;
            case d_CONST: cout<<n; break;
            case d_TAG: printTag(tag); break;
            default: return;
        }
    }
};

/** Kod pośredni z konkretnymi wskaźnikami zmiennych*/
vector<pair<MCInstr,MCDest>> midCodeAllocated;

/** Funkcja dodaje instrukcję oraz argument do midCodeAllocated */
void MCApush(MCInstr mi, MCDest md) {
    midCodeAllocated.push_back(make_pair(mi,md));
}

/** Funkcja przekształca kod pośredni na kod pośredni z alokacjami */
void MCtoMCAllo(vector<MCE*> midcode) {
    valinfo* v;
    for (MCE* m: midcode) {
        struct MCInstr mi = {m->tags,m->ins};
        switch(m->ins) {
            case mREAD: case mWRITE: case mHALT: case mTIMES: case mDIV: case mMOD:
                MCApush(mi,MCDest());
            break;
            case mADD: case mSUB: case mSHIFT: case mSWAP: case mRESET: case mINC: case mDEC:
                MCApush(mi,MCDest(m->var->reg));
            break;
            case mJUMP: case mJPOS: case mJZERO: case mJNEG:
                if (m->var->type == t_TAG) {
                    MCApush(mi, MCDest(m->var->tag));
                }
                else if (m->var->type == t_VAL) {
                    if(m->var->val->type != NUM) yyerror("Krytyczny błąd przy MCA");
                    MCApush(mi, MCDest(m->var->val->num));
                }
            break;
            case mSAVE: case mLD:
                if(m->var->type != t_VAL) yyerror("Krytyczny błąd przy MCA");
                v = m->var->val;
                switch(v->type){
                    case NUM:
                        MCApush(mi, MCDest(to_string(v->num))); break;
                    case ELEM:
                        MCApush(mi, MCDest(string(v->varName))); break;
                    case TELEM:
                        MCApush(mi, MCDest(string(v->varName),to_string(v->index))); break;
                    case TELEMID:
                        MCApush(mi, MCDest(string(v->varName),string(v->indexName))); break;
                }
            break;
            default: yyerror("Krytyczny błąd: brak implementacji MCtoMCA");
        }
    }
}

/** Wyświetl kod pośredni z alokacjami na stdout */
void printMCA() {
    for (pair<MCInstr,MCDest> p: midCodeAllocated) {
        for (auto t: p.first.tags) printTag(t);
        cout<<MCinstrName[p.first.instr]<<" ";
        p.second.print();
        cout<<endl;
    }

}

/** Rodzaj przechowywania zmiennej */
enum memLocType {m_REG, m_MEM, m_CONST};
/** Klasa przechowuje informacje jak dany argument ma zostać przechowany */
class memLoc {
public:
    enum memLocType type;
    enum reg reg;
    long long loc;
    long long offset;

    memLoc() {}

    memLoc(long long n) {
        this->type = m_CONST;
        this->loc = n;
    }

    memLoc(varData* vd) {
        this->type = m_MEM;
        if (vd->isTable) {
            long long size;
            this->loc = allocmem;
            size = vd->end - vd->start + 1;
            offset = vd->start;
            allocmem += size;
        }
        else {
            this->loc = allocmem;
            this->offset = 0;
            allocmem++;
        }
    }

    memLoc(enum reg r) {
        this->type = m_REG;
        this->reg = r;
    }
};

/** Mapa zapamiętująca miejsce przechowywania zmiennej */
map<string,class memLoc> varMemLoc;

/** Wyświetl lokalizację zmiennych */
void printvarLocType() {
    for (auto const&a: varMemLoc) {
        cout<<a.first<<" ";
        switch (a.second.type) {
            case m_CONST: cout<<"= const("<<a.second.loc<<")"; break;
            case m_REG: cout<<"["<<regName[a.second.reg]<<"]"; break;
            case m_MEM: cout<<"["<<a.second.loc<<"]"; break;
        }
        cout<<endl;
    }
}

/** Przydziel miejsce w pamięci bądź w rejestrze danej zmiennej */
void allocateVars(vector<MCE*> midcode) {
    while (!varPriority.empty()) {
        //pobierz zmienną nieprzetworzoną o największym priorytecie
        varprio v = varPriority.top();
        string vname = v.first;
        varPriority.pop();

        if (!hasSymbolString(vname)) {
            //stała
            //TODO - zapisywanie stałych do użytku
            varMemLoc[vname]=memLoc(atoll(vname.c_str()));
        }
        else {
            varData* vd = getSymbolString(v.first);
            if (vd->isTable) {
                varMemLoc[vname]=memLoc(vd);
            }
            else {
                pair<long long,long long> vrng = varRange[vname];
                bool freeRegs[6] = {true,true,true,true,true,true};
                //sprawdź czy rejestry są wolne w tym zakresie
                for (auto it = begin(midcode) + vrng.first - 1; it != begin(midcode) + vrng.second; it++) {
                    for(int i=0;i<6;i++) {
                        if ((*it)->registry[i+2] == false) {
                            freeRegs[i] = false;
                        }
                    }
                }
                enum reg destReg = UNDEF;
                for (int i=5;i>=0;i--) {
                    //znaleziono wolny rejestr
                    if (freeRegs[i]) {
                        destReg = getRegNum(i+2);
                        for (auto it = begin(midcode) + vrng.first - 1; it != begin(midcode) + vrng.second; it++) {
                            (*it)->registry[i+2] = false;
                        }
                        varMemLoc[vname] = memLoc(destReg);
                        break;
                    }
                }
                // nie znaleziono wolnego rejestru
                if (destReg == UNDEF) {
                    varMemLoc[vname] = memLoc(vd);
                }
            }
        }
    }
}

/** Oblicz priorytet zmiennej (zmienne z wyższym priorytetem pierwsze są alokowane) */
void calculateVarPriority(map<string,vector<long long>> vu, vector<MCE*> midcode) {
    for (auto const& v: vu) {
        long long p = 0;
        for (long long l: v.second) {
            p += 1 + midcode.at(l)->priority;
        }
        varPriority.push(make_pair(v.first,p));
    }
}

/** Oblicz zasięg występowania zmiennej*/
void calculateVarRange(map<string,vector<long long>> vu, vector<MCE*> midcode) {
    for (auto const&v: vu) {
        long long min = numeric_limits<long long int>::max();
        long long max = 0;

        for (long long l: v.second) {
            //sprawdz czy zmienna jest w pętli
            cominfo* c = getClosestLoopParent(midcode.at(l)->cfID);
            //jest - dodaj do zakresu 
            if (c!=0) {
                pair<long long,long long> cfrng = cfRange[c->ID];

                if (cfrng.first < min) min = cfrng.first;
                if (cfrng.second > max) max = cfrng.second;
            }
            else {
                if (l < min) min=l;
                if (l > max) max=l;
            }
        }
        varRange[v.first] = make_pair(min,max);
    }

    /*
    for (auto const& e: varRange) {
        cout<<e.first<<" "<<e.second.first<<"-"<<e.second.second<<endl;
    }*/

    //calculateVarPriority(vu,midcode);
}

/** Pobierz występowania zmiennych z kodu pośredniego */
void getVarUsage(vector<MCE*> midcode) {
    long long line = 0;
    for (MCE* m: midcode) {
        line++;
        if (m->ins == mLD || m->ins == mSAVE) {
            MCVar* var = m->var;
            if (var->type == t_VAL) {
                valinfo* val = var->val;
                switch(val->type) {
                    case NUM:
                        varUsage[to_string(val->num)].push_back(line);
                        break;
                    case ELEM:
                        varUsage[val->varName].push_back(line);
                        break;
                    case TELEM:
                        varUsage[val->varName].push_back(line);
                        varUsage[to_string(val->index)].push_back(line);
                        break;
                    case TELEMID:
                        varUsage[val->varName].push_back(line);
                        varUsage[val->indexName].push_back(line);
                        break;
                }
            }
        }
    }

    /*
    for (auto const& e: varUsage) {
        cout<<e.first<<": {";
        for (long long l: e.second) {
            cout<<" "<<l<<",";
        }
        cout<<"}"<<endl;
    }
    */

    //calculateVarRange(varUsage,midcode);
}

/** Wylicz zasięg pętli oraz ifów */
void calculatecfRange(vector<MCE*> midcode) {
    long long line = 0;

    for (MCE* m: midcode) {
        line++;
        long long cfID = m->cfID;
        if (cfRange.count(cfID)==0) {
            cfRange[cfID].first = line;
        }
        else {
            cfRange[cfID].second = line;
        }
    }

    //print
    /*
    for (auto const& e: cfRange) {
        cout<<e.first<<" "<<e.second.first<<"-"<<e.second.second<<endl;
    }*/
}

/** Wygeneruj tablicę z alokacją zmiennych */
void generateSymbolLocationTable(vector<MCE*> midcode) {
    calculatecfRange(midcode);
    getVarUsage(midcode);
    calculateVarRange(varUsage,midcode);
    calculateVarPriority(varUsage,midcode);
    allocateVars(midcode);
    //gotowe
    //printvarLocType();
}

void generateMCA(vector<MCE*> midcode) {
    MCtoMCAllo(midcode);
    //printMCA();
}