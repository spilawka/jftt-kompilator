#include <stack>
#include <algorithm>
#include "mcInstructions.hh"

enum MCTagTypes {t_START,t_END,t_CODE1,t_CODE2};
struct MCTag {
    enum MCTagTypes type;
    long long ID;
};
typedef struct MCTag MCTag;

enum MCVarTypes {t_VAL,t_TAG,t_REG};
struct MCVar {
    enum MCVarTypes type;
    valinfo* val;
    enum reg reg;
    MCTag tag;
};
typedef struct MCVar MCVar;

class MCEntry {
public:
    vector<MCTag> tags;
    enum MCinstr ins;
    MCVar* var;
    bool registry[8];
    valinfo* registryVals[2];

    MCEntry() {
        this->tags = {};
        this->var = new MCVar();
    }

    /** Konstruktor dla stałej liczby*/
    /** Konstruktor dla zmiennej*/
    MCEntry(enum MCinstr i, valinfo* v): MCEntry(){
        this->ins = i;
        this->var->type = t_VAL;
        this->var->val = v;
    }
    /** Konstruktor dla rejestru*/
    MCEntry(enum MCinstr i, enum reg reg): MCEntry(){
        this->ins = i;
        this->var->type = t_REG;
        this->var->reg = reg;
    }
    /** Konstruktor dla tagu (odniesienia)*/
    MCEntry(enum MCinstr i, enum MCTagTypes tt, long long ID): MCEntry(){
        this->ins = i;
        this->var->type = t_TAG;
        this->var->tag.type = tt;
        this->var->tag.ID = ID;
    }

    ~MCEntry() {
        this->tags.clear();
        delete(var);
    }

    void addTag(MCTag t) {
        this->tags.push_back(t);
    }

    bool hasTag(MCTag t) {
        for(auto it = tags.begin(); it != tags.end(); it++){
            if (it->type == t.type && it->ID == t.ID) return true;
        }
        return false;
    }

    bool hasTags() {
        return tags.size() != 0;
    }
};
typedef class MCEntry MCEntry;

vector<MCEntry*> midCode;
/** Określenie czy dane rejestry są dostępne */
bool mcRegistry[8] = {false,false,true,true,true,true,true,true};
/** konkretne wartości przechowywane w rejestrach A i B */
valinfo* mcRegistryVals[2] = {0,0};

bool isTagQueued = false;
MCTag queuedTag;

void queueTag(MCTag tag) {
    isTagQueued = true;
    queuedTag = tag;
}

void modifyMCReg(MCEntry* e) {
    valinfo* t;
    switch (e->ins) {
        case mREAD: case mSAVE: case mLDA:
            mcRegistryVals[A] = e->var->val;
        break;
        case mLDB:
            mcRegistryVals[A] = 0;
            mcRegistryVals[B] = e->var->val;
        break;
        case mSWAP:
            if (e->var->type == t_REG) {
                if (e->var->reg == B) {
                    t = mcRegistryVals[A];
                    mcRegistryVals[A] = mcRegistryVals[B];
                    mcRegistryVals[B] = t;
                }
                else {
                    mcRegistryVals[A] = 0;
                }
            }
        break;
        case mADD: case mSUB: case mSHIFT:
            mcRegistryVals[A] = 0;
        break;
        case mINC: case mDEC:
            if (e->var->type == t_REG && e->var->reg<=B) {
                mcRegistryVals[e->var->reg] = 0;
            }
        break;
        case mTIMES: case mDIV: case mMOD:
            mcRegistryVals[A] = 0;
            mcRegistryVals[B] = 0;
        break;
    }
}

void MCInsert(MCEntry* newEntry) {
    if (isTagQueued) {
        newEntry->addTag(queuedTag);
        isTagQueued = false;
    }

    if (newEntry->hasTags()) {
        mcRegistryVals[A]=0; mcRegistryVals[B]=0;
    }

    modifyMCReg(newEntry);

    for (int i=0;i<8;i++) newEntry->registry[i] = mcRegistry[i];
    for (int i=0;i<2;i++) newEntry->registryVals[i] = mcRegistryVals[i];
    
    midCode.push_back(newEntry);
}

void genValue (valinfo* v, enum reg targetReg) {
    //zmienna znajdzie się w rejestrze
    if (targetReg <= 1) mcRegistryVals[targetReg] = v;

    switch (v->type) {
        case NUM: case ELEM:
            MCInsert(new MCEntry(getLoadReg(targetReg),v)); break;
        default:
            mcRegistry[C] = false;
            MCInsert(new MCEntry(getLoadReg(targetReg),v));
            mcRegistry[C] = true; break;
    }
    
    
}

void genSaveValue (valinfo* v) {
    switch (v->type) {
        case NUM:
            yyerrorline("Nie można zapisać stałej!",0); break;
        case ELEM:
            MCInsert(new MCEntry(mSAVE,v)); break; 
        default:
            mcRegistry[C] = false;
            MCInsert(new MCEntry(mSAVE,v)); 
            mcRegistry[C] = true; break;
    }
}

void genCondition(condinfo* c, long long id, enum MCTagTypes conttag, enum MCTagTypes exittag) {
    genValue(c->v2,B);
    genValue(c->v1,A);

    //wartość A staje się bez znaczenia
    MCInsert(new MCEntry(mSUB,B));

    MCTag t;
    switch(c->type) {
        case c_EQ:
            MCInsert(new MCEntry(mJZERO,conttag,id));
            MCInsert(new MCEntry(mJUMP,exittag,id));
        break;
        case c_NEQ:
            MCInsert(new MCEntry(mJZERO,exittag,id));
        break;
        case c_LE:
            MCInsert(new MCEntry(mJNEG,conttag,id));
            MCInsert(new MCEntry(mJUMP,exittag,id));
        break;
        case c_LEQ:
            MCInsert(new MCEntry(mJPOS,exittag,id));
        break;
        case c_GE:
            MCInsert(new MCEntry(mJPOS,conttag,id));
            MCInsert(new MCEntry(mJUMP,exittag,id));
        break;
        case c_GEQ:
            MCInsert(new MCEntry(mJNEG,exittag,id));
        break;
    }

    t = {conttag,id}; queueTag(t);
}

long long pwrOfTwo(long long n) {
    if (n==0) return -1;
    long long p = 0;
    
    while (n!=1) {
        if (n%2!=0) return -1;
        p++;
        n = n/2;
    }
    return p;
}

void genTimes(exprinfo* e) {
    valinfo* v1 = e->v1;
    valinfo* v2 = e->v2;
    bool negFlag = false;

    if ((v1->type == NUM && v1->num == 0) || (v2->type == NUM && v2->num == 0)) {
        MCInsert(new MCEntry(mRESET,A));
        return;
    }

    if (v2->type == NUM) {
        long long n = v2->num;
        if (n<0)  {
            negFlag = true;
            n=-n;
        }
        long long p=pwrOfTwo(n);
        if (p!=-1) {
            valinfo* v = makeValinfoNum(p,0);
            genValue(v,B);
            genValue(v1,A);

            MCInsert(new MCEntry(mSHIFT,B));

            if (negFlag) {
                MCInsert(new MCEntry(mSWAP,B));
                MCInsert(new MCEntry(mRESET,A));
                MCInsert(new MCEntry(mSUB,B));
            }
            return;
        }
    }

    negFlag = false;
    if (v1->type == NUM) {
        long long n = v1->num;
        if (n<0)  {
            negFlag = true;
            n=-n;
        }
        long long p=pwrOfTwo(n);
        if (p!=-1) {
            genValue(makeValinfoNum(p,0),B);
            genValue(v2,A);

            MCInsert(new MCEntry(mSHIFT,B));

            if (negFlag) {
                MCInsert(new MCEntry(mSWAP,B));
                MCInsert(new MCEntry(mRESET,A));
                MCInsert(new MCEntry(mSUB,B));
            }
            return;
        }
    }

    negFlag = false;
    if (v1->type == NUM || v2->type == NUM) {
        long long n = 0;
        bool usev1 = false;
        if (v1->type == NUM) {
            if (v1->num < 0) {
                n = -(v1->num);
                negFlag = true;
            }
            else {
                n = v1->num;
            }
            usev1 = true;
        }

        if (v2->type == NUM) {
            long long n2;
            if (v2->num < 0) {
                n2 = -(v2->num);
                if (n2<n) {
                    n = n2;
                    usev1 = false;
                    negFlag = true;
                }
            }
            else {
                n2 = v2->num;
                if (n2<n) {
                    n = n2;
                    usev1 = false;
                    negFlag = true;
                }
            }
        }
        //A - wynik, B - min(v1,v2), C - 1, D - max(v1,v2)
        if (n<=15) {
            //zajmij rejestry
            mcRegistry[C] = false;
            mcRegistry[D] = false;
            //c=1
            MCInsert(new MCEntry(mRESET,C));
            MCInsert(new MCEntry(mINC,C));

            //
            if(usev1) {
                MCInsert(new MCEntry(mLDB,v1));
                MCInsert(new MCEntry(mLDD,v2));
            }
            else {
                MCInsert(new MCEntry(mLDB,v2));
                MCInsert(new MCEntry(mLDD,v1));
            }
            MCInsert(new MCEntry(mRESET,A));
            
            //pętla
            MCInsert(new MCEntry(mSWAP,B));
            MCInsert(new MCEntry(mJZERO,makeValinfoNum(5,0)));
            MCInsert(new MCEntry(mDEC,A));
            MCInsert(new MCEntry(mSWAP,B));
            MCInsert(new MCEntry(mADD,D));
            MCInsert(new MCEntry(mJUMP,makeValinfoNum(-5,0)));
            //odpowiedź znajduje się w B
            if (negFlag) {
                MCInsert(new MCEntry(mRESET,A));
                MCInsert(new MCEntry(mSUB,B));
            }
            else {
                MCInsert(new MCEntry(mSWAP,B));
            }

            //zwolnij rejestry
            mcRegistry[C] = true;
            mcRegistry[D] = true;

            return;
        }
    }

    genValue(e->v2,B);
    genValue(e->v1,A);
    mcRegistry[C] = false;
    mcRegistry[D] = false;
    mcRegistry[E] = false;
    mcRegistry[F] = false;
    MCInsert(new MCEntry(mTIMES,B));
    mcRegistry[C] = true;
    mcRegistry[D] = true;
    mcRegistry[E] = true;
    mcRegistry[F] = true;
}

void genDiv(exprinfo* e) {
    genValue(e->v2,B);
    genValue(e->v1,A);
    MCInsert(new MCEntry(mDIV,B));
}

void genMod(exprinfo* e) {
    genValue(e->v2,B);
    genValue(e->v1,A);
    MCInsert(new MCEntry(mMOD,B));
}

void genExpression(exprinfo* e) {
    switch (e->type) {
        case e_SOLO: genValue(e->v1,A); break;
        case e_PLUS:
            genValue(e->v2,B);
            genValue(e->v1,A);
            MCInsert(new MCEntry(mADD,B));
            break;
        case e_MINUS:
            //zero
            if (isTheSameVal(e->v1,e->v2)) {
                MCInsert(new MCEntry(mRESET,A));
            }
            else {
                genValue(e->v2,B);
                genValue(e->v1,A);
                MCInsert(new MCEntry(mSUB,B));
                break;
            }
        case e_TIMES:
            genTimes(e); break;
        case e_DIV:
            genDiv(e); break;
        case e_MOD:
            genMod(e); break;
    }
}

void genCommand(cominfo* c) {
    MCTag t;
    cominfo* ch;
    long long s;

    switch (c->type) {
        case c_IF:
            genCondition(c->ci,c->ID,t_CODE1,t_END);

            // gen commands
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch);
                ch = ch->next;
            }

            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_IFELSE:
            genCondition(c->ci,c->ID,t_CODE1,t_CODE2);

            // gen commands 1
            ch = *(c->children);
            s = c->sep;
            while (ch!=0 && s>0) {
                genCommand(ch);
                ch = ch->next;
                s--;
            }

            MCInsert(new MCEntry(mJUMP,t_END,c->ID));
            t = {t_CODE2,c->ID}; queueTag(t);

            // gen commands 2
            while (ch!=0) {
                genCommand(ch);
                ch = ch->next;
            }

            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_WHILE:
            t = {t_START,c->ID}; queueTag(t);
            genCondition(c->ci,c->ID,t_CODE1,t_END);

            // gen
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch);
                ch = ch->next;
            }

            MCInsert(new MCEntry(mJUMP,t_START,c->ID));
            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_REPEAT:
            MCInsert(new MCEntry(mJUMP,t_CODE1,c->ID));
            t = {t_START,c->ID}; queueTag(t);
            genCondition(c->ci,c->ID,t_CODE1,t_END);

            //generate
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch);
                ch = ch->next;
            }

            MCInsert(new MCEntry(mJUMP,t_START,c->ID));
            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_FORTO:
            genValue(c->ifvar->from,A);
            MCInsert(new MCEntry(mJUMP,makeValinfoNum(2,c->line)));
            t = {t_START,c->ID}; queueTag(t);
            genValue(c->vi,A);
            MCInsert(new MCEntry(mSWAP,B));
            genValue(c->ifvar->to,A);
            MCInsert(new MCEntry(mSUB,B));
            MCInsert(new MCEntry(mJNEG,t_END,c->ID));
            MCInsert(new MCEntry(mSWAP,B));
            MCInsert(new MCEntry(mINC,A));
            genSaveValue(c->vi);
            
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch);
                ch = ch->next;
            }

            MCInsert(new MCEntry(mJUMP,t_START,c->ID));
            t = {t_END,c->ID}; queueTag(t);

        break;
        case c_FORDOWNTO:
            genValue(c->ifvar->from,A);
            MCInsert(new MCEntry(mJUMP,makeValinfoNum(2,c->line)));
            t = {t_START,c->ID}; queueTag(t);
            genValue(c->vi,A);
            MCInsert(new MCEntry(mSWAP,B));
            genValue(c->ifvar->to,A);
            MCInsert(new MCEntry(mSUB,B));
            MCInsert(new MCEntry(mJPOS,t_END,c->ID));
            MCInsert(new MCEntry(mSWAP,B));
            MCInsert(new MCEntry(mDEC,A));
            genSaveValue(c->vi);
            
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch);
                ch = ch->next;
            }

            MCInsert(new MCEntry(mJUMP,t_START,c->ID));
            t = {t_END,c->ID}; queueTag(t);

        break;
        case c_ASSIGN:
            genExpression(c->ei);
            genSaveValue(c->vi);
        break;
        case c_READ:
            MCInsert(new MCEntry(mREAD,A));
            genSaveValue(c->vi);
        break;
        case c_WRITE:
            genValue(c->vi,A);
            MCInsert(new MCEntry(mWRITE,A));
        break;
    }
}

void printMCEntry(MCEntry* mc) {
    for (auto v: mc->tags) {
        cout<<"<";
        switch (v.type){
            case t_START: cout<<"s-"; break;
            case t_CODE1: cout<<"c-"; break;
            case t_CODE2: cout<<"z-"; break;
            case t_END: cout<<"e-"; break;
        }
        cout<<v.ID<<"> ";
    }

    cout<<MCinstrName[mc->ins]<<" ";
    
    switch (mc->var->type) {
        case t_VAL:
            printVal(mc->var->val);
        break;
        case t_TAG: 
            cout<<"<";
            switch (mc->var->tag.type){
                case t_START: cout<<"s-"; break;
                case t_CODE1: cout<<"c-"; break;
                case t_CODE2: cout<<"z-"; break;
                case t_END: cout<<"e-"; break;
            }
            cout<<mc->var->tag.ID<<">";
        break;
        case t_REG: cout<<":"<<regName[mc->var->reg]<<":"; break;
    }

    cout<<"\t\ta:";
    if (mc->registryVals[A]!=0) printVal(mc->registryVals[A]);
    cout<<" b:";
    if (mc->registryVals[B]!=0) printVal(mc->registryVals[B]);

    cout<<endl;
}



typedef stack<pair<enum instr, enum reg>> insstack;
/** Funkcja zwraca kod do generowania danej liczby od podstaw w kodzie maszynowym 
    @return stos LILO z kodem instrukcji oraz modyfikowanym rejestrem

    REJESTRY: A,B
*/
void genNumber(long long num) {
    //stack z kodem maszynowym
    insstack code = {};

    //małe liczby (abs(n)<10)
    if (num == 0) {
        code.push(make_pair(RESET,A));
    }
    else if (num > 0 && num < 10) {
        for (int i=0; i<num; i++)
            code.push(make_pair(INC,A));
        code.push(make_pair(RESET,A));
    }
    else if (num > -10 && num < 0) {
        for (int i=0; i>num; i--) 
            code.push(make_pair(DEC,A));
        code.push(make_pair(RESET,A));
    }
    else {
        //liczby ujemne - weź odwrotność i odejmij pod sam koniec
        if (num<0) {
            num = -num;
            code.push(make_pair(SUB,B));
            code.push(make_pair(RESET,A));
            code.push(make_pair(SWAP,B));
        }

        //ilość występowania prostych wymnożeń razy dwa
        long long conseq2 = 0;
        while (num>0) {
            long long modulo = num%2;

            if (modulo == 0) {
                conseq2++;
            }
            else {
                if (conseq2==1) {
                    code.push(make_pair(SHIFT,B));
                }
                // dla więcej operacji optymalizacja - bardziej
                // opłaca się zwiększać b niż wykonywać SHIFTy
                else if (conseq2>1) {
                    code.push(make_pair(INC,B));
                    code.push(make_pair(RESET,B));
                    code.push(make_pair(SHIFT,B));
                    for(int i=1;i<conseq2;i++){
                        code.push(make_pair(INC,B));
                    }
                }
                conseq2=0;

                code.push(make_pair(INC,A));
                if (num!=1)
                    code.push(make_pair(SHIFT,B));
            }

            num = num/2;
        }

        //reset rejestrów
        code.push(make_pair(INC,B));
        code.push(make_pair(RESET,B));
        code.push(make_pair(RESET,A));
    }
    /*
    while (!code.empty()) {
        pair<enum instr, enum reg> elem = code.top();
        struct reference r;
        r.isFinal = true;
        r.isRegistry = true;
        r.Registry = elem.second;
        
        midCode.push_back(make_pair(elem.first,r));

        code.pop();
    }
    */
}