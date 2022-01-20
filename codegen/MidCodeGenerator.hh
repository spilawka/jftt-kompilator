#include <stack>
#include <algorithm>
#include "MidCodeInstructions.hh"

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

long long globalPriority = 0;
long long globalcfID = cfID;
enum comInfoType globalcftype;

class MCE {
public:
    vector<MCTag> tags;
    enum MCinstr ins;
    MCVar* var;
    bool registry[8];
    valinfo* registryVals[2];
    long long priority;
    long long cfID;
    enum comInfoType cftype;

    MCE() {
        this->tags = {};
        this->var = new MCVar();
        this->priority = globalPriority;
        this->cfID = globalcfID;
        this->cftype = globalcftype;
    }

    /** Konstruktor dla stałej liczby*/
    /** Konstruktor dla zmiennej*/
    MCE(enum MCinstr i, valinfo* v): MCE(){
        this->ins = i;
        this->var->type = t_VAL;
        this->var->val = v;
    }
    /** Konstruktor dla rejestru*/
    MCE(enum MCinstr i, enum reg reg): MCE(){
        this->ins = i;
        this->var->type = t_REG;
        this->var->reg = reg;
    }
    /** Konstruktor dla tagu (odniesienia)*/
    MCE(enum MCinstr i, enum MCTagTypes tt, long long ID): MCE(){
        this->ins = i;
        this->var->type = t_TAG;
        this->var->tag.type = tt;
        this->var->tag.ID = ID;
    }

    ~MCE() {
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
typedef class MCE MCE;

vector<MCE*> midCode;
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

void modifyMCReg(MCE* e) {
    valinfo* t;
    switch (e->ins) {
        case mREAD: case mLD:
            mcRegistryVals[A] = e->var->val;
        break;
        case mSAVE:
            mcRegistryVals[A] = 0;
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

void MCI(MCE* newEntry) {
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
            mcRegistry[C] = false;
            MCI(new MCE(mLD,v));
            if (targetReg != A)  MCI(new MCE(mSWAP,targetReg));
            mcRegistry[C] = true; break;
        default:
            mcRegistry[C] = false;
            mcRegistry[D] = false;
            MCI(new MCE(mLD,v));
            if (targetReg != A) MCI(new MCE(mSWAP,targetReg));
            mcRegistry[C] = true;
            mcRegistry[D] = true; break;
    }
}

void genSaveValue (valinfo* v) {
    switch (v->type) {
        case NUM:
            yyerrorline("Nie można zapisać stałej!",0); break;
        case ELEM:
            mcRegistry[C] = false;
            MCI(new MCE(mSAVE,v)); break;
            mcRegistry[C] = true;
        default:
            mcRegistry[C] = false;
            mcRegistry[D] = false;
            MCI(new MCE(mSAVE,v)); 
            mcRegistry[C] = true;
            mcRegistry[D] = true;
        break;
    }
}

void genMinus(exprinfo* e) {
    long long val;
    //zero
    if (isTheSameVal(e->v1,e->v2)) {
        MCI(new MCE(mRESET,A));
    }
    else if (e->v2->type == NUM) {
        val = e->v2->num;
        if (val == 0) {
            genValue(e->v1,A);
        }
        else if (val<=10 && val>0) {
            genValue(e->v1,A);
            for (int i=0; i<val; i++) {
                MCI(new MCE(mDEC,A));
            }
        }
        else if (val>=-10 && val<0) {
            genValue(e->v1,A);
            for (int i=0; i<val; i++) {
                MCI(new MCE(mINC,A));
            }
        }
    }
    else {
        genValue(e->v2,B);
        genValue(e->v1,A);
        MCI(new MCE(mSUB,B));
    }
}

void genCondition(condinfo* c, long long id, enum MCTagTypes conttag, enum MCTagTypes exittag) {
    genMinus(createExprInfo(c->v2,e_MINUS,c->v1));

    MCTag t;
    switch(c->type) {
        case c_EQ:
            MCI(new MCE(mJZERO,conttag,id));
            MCI(new MCE(mJUMP,exittag,id));
        break;
        case c_NEQ:
            MCI(new MCE(mJZERO,exittag,id));
        break;
        case c_LE:
            MCI(new MCE(mJNEG,conttag,id));
            MCI(new MCE(mJUMP,exittag,id));
        break;
        case c_LEQ:
            MCI(new MCE(mJPOS,exittag,id));
        break;
        case c_GE:
            MCI(new MCE(mJPOS,conttag,id));
            MCI(new MCE(mJUMP,exittag,id));
        break;
        case c_GEQ:
            MCI(new MCE(mJNEG,exittag,id));
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
        MCI(new MCE(mRESET,A));
        return;
    }

    if (v2->type == NUM) {
        long long n = v2->num;
        if (n<0)  {
            negFlag = true;
            n=-n;
        }
        long long p=pwrOfTwo(n);
        if (p==0) {
            if (negFlag) {
                genValue(v1,B);
                MCI(new MCE(mSUB,B));
            }
            else {
                genValue(v1,A);
            }
            return;
        }
        else if (p!=-1) {
            valinfo* v = makeValinfoNum(p,0);
            genValue(v,B);
            genValue(v1,A);

            MCI(new MCE(mSHIFT,B));

            if (negFlag) {
                MCI(new MCE(mSWAP,B));
                MCI(new MCE(mRESET,A));
                MCI(new MCE(mSUB,B));
            }
            return;
        }
    }

    negFlag = false;
    if (v1->type == NUM) {
        long long n = v1->num;
        if (n<0)  {
            negFlag = !negFlag;
            n=-n;
        }
        long long p=pwrOfTwo(n);
        if (p!=-1) {
            genValue(makeValinfoNum(p,0),B);
            genValue(v2,A);

            MCI(new MCE(mSHIFT,B));

            if (negFlag) {
                MCI(new MCE(mSWAP,B));
                MCI(new MCE(mRESET,A));
                MCI(new MCE(mSUB,B));
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
                negFlag = !negFlag;
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
                    negFlag = !negFlag;
                }
            }
            else {
                n2 = v2->num;
                if (n2<n) {
                    n = n2;
                    usev1 = false;
                }
            }
        }
        //A - wynik, B - min(v1,v2), C - 1, D - max(v1,v2)
        if (n<=15) {
            //
            if(usev1) {
                genValue(v1,B);
                genValue(v2,D);
            }
            else {
                genValue(v2,B);
                genValue(v1,D);
            }
            //zajmij rejestry
            mcRegistry[C] = false;
            mcRegistry[D] = false;
            //c=1
            MCI(new MCE(mRESET,C));
            MCI(new MCE(mINC,C));
            MCI(new MCE(mRESET,A));
            //pętla
            MCI(new MCE(mSWAP,B));
            MCI(new MCE(mJZERO,makeValinfoNum(5,0)));
            MCI(new MCE(mDEC,A));
            MCI(new MCE(mSWAP,B));
            MCI(new MCE(mADD,D));
            MCI(new MCE(mJUMP,makeValinfoNum(-5,0)));
            //odpowiedź znajduje się w B
            if (negFlag) {
                MCI(new MCE(mRESET,A));
                MCI(new MCE(mSUB,B));
            }
            else {
                MCI(new MCE(mSWAP,B));
            }

            //zwolnij rejestry
            mcRegistry[C] = true;
            mcRegistry[D] = true;

            return;
        }
    }

    genValue(e->v2,F);
    genValue(e->v1,E);
    mcRegistry[C] = false;
    mcRegistry[D] = false;
    mcRegistry[E] = false;
    mcRegistry[F] = false;
    mcRegistry[G] = false;
    MCI(new MCE(mTIMES,B));
    mcRegistry[C] = true;
    mcRegistry[D] = true;
    mcRegistry[E] = true;
    mcRegistry[F] = true;
    mcRegistry[G] = true;
}

void genDiv(exprinfo* e) {
    valinfo* v1 = e->v1;
    valinfo* v2 = e->v2;

    if ((v1->type == NUM && v1->num == 0) || (v2->type == NUM && v2->num == 0)) {
        MCI(new MCE(mRESET,A));
        return;
    }

    if (v2->type == NUM && v2->num>0) {
        long long n = v2->num;
        long long p = pwrOfTwo(n);
        if (p!=-1) {
            genValue(makeValinfoNum(p,0),B);
            genValue(v1,A);
            MCI(new MCE(mSHIFT,B));
        }
        return;
    }
    genValue(e->v2,F);
    genValue(e->v1,E);
    mcRegistry[C] = false;
    mcRegistry[D] = false;
    mcRegistry[E] = false;
    mcRegistry[F] = false;
    mcRegistry[G] = false;
    MCI(new MCE(mDIV,B));
    mcRegistry[C] = true;
    mcRegistry[D] = true;
    mcRegistry[E] = true;
    mcRegistry[F] = true;
    mcRegistry[G] = true;
}

void genMod(exprinfo* e) {
    valinfo* v1 = e->v1;
    valinfo* v2 = e->v2;

    if ((v1->type == NUM && v1->num == 0) || (v2->type == NUM && v2->num == 0)) {
        MCI(new MCE(mRESET,A));
        return;
    }

    if (v2->type == NUM && (v2->num == 1 || v2->num == -1)) {
        MCI(new MCE(mRESET,A));
        return;
    }

    if (v2->type == NUM && v2->num > 0) {
        long long p = pwrOfTwo(v2->num);
        if (p!=-1) {
            
            genValue(makeValinfoNum(p,0),B);
            genValue(v1,A);
            mcRegistry[C] = false;
            MCI(new MCE(mRESET,C));
            MCI(new MCE(mSWAP,C));
            MCI(new MCE(mRESET,A));
            MCI(new MCE(mADD,C));

            mcRegistry[C] = true;
        }
    }

    
    genValue(e->v2,B);
    genValue(e->v1,A);
    mcRegistry[C] = false;
    mcRegistry[D] = false;
    mcRegistry[E] = false;
    mcRegistry[F] = false;
    mcRegistry[G] = false;
    MCI(new MCE(mDIV,B));
    MCI(new MCE(mSWAP,B));
    mcRegistry[C] = true;
    mcRegistry[D] = true;
    mcRegistry[E] = true;
    mcRegistry[F] = true;
    mcRegistry[G] = true;
}

void genExpression(exprinfo* e) {
    long long val;
    switch (e->type) {
        case e_SOLO: genValue(e->v1,A); break;
        case e_PLUS:

            if (e->v2->type == NUM) {
                val = e->v2->num;
                if (val == 0) {
                    genValue(e->v1,A);
                }
                else if (val<=10 && val>0) {
                    genValue(e->v1,A);
                    for (int i=0; i<val; i++) {
                        MCI(new MCE(mINC,A));
                    }
                }
                else if (val>=-10 && val<0) {
                    genValue(e->v1,A);
                    for (int i=0; i<val; i++) {
                        MCI(new MCE(mDEC,A));
                    }
                }
            }
            else {
                genValue(e->v2,B);
                genValue(e->v1,A);
                MCI(new MCE(mADD,B));
            }
            break;
        case e_MINUS:
            genMinus(e);
            break;
        case e_TIMES:
            genTimes(e); break;
        case e_DIV:
            genDiv(e); break;
        case e_MOD:
            genMod(e); break;
    }
}

void genCommand(cominfo* c, long long priority) {
    MCTag t;
    cominfo* ch;
    long long s;
    long long prevID;
    enum comInfoType prevcftype;

    globalPriority = priority;
    prevID = globalcfID;
    prevcftype = globalcftype;

    switch (c->type) {
        case c_IF:
            globalcfID = c->ID;
            globalcftype = c->type;
            genCondition(c->ci,c->ID,t_CODE1,t_END);

            // gen commands
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch,priority);
                ch = ch->next;
            }

            globalcfID  = prevID;
            globalcftype = prevcftype;
            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_IFELSE:
            globalcfID = c->ID;
            globalcftype = c->type;
            genCondition(c->ci,c->ID,t_CODE1,t_CODE2);

            // gen commands 1
            ch = *(c->children);
            s = c->sep;
            while (ch!=0 && s>0) {
                genCommand(ch,priority);
                ch = ch->next;
                s--;
            }
            
            MCI(new MCE(mJUMP,t_END,c->ID));
            t = {t_CODE2,c->ID}; queueTag(t);

            // gen commands 2
            while (ch!=0) {
                genCommand(ch,priority);
                ch = ch->next;
            }

            globalcftype = prevcftype;
            globalcfID  = prevID;
            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_WHILE:
            globalcfID = c->ID;
            globalcftype = c->type;
            globalPriority++;

            t = {t_START,c->ID}; queueTag(t);
            genCondition(c->ci,c->ID,t_CODE1,t_END);

            // gen
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch,priority+1);
                ch = ch->next;
            }

            MCI(new MCE(mJUMP,t_START,c->ID));
            globalcfID  = prevID;
            globalcftype = prevcftype;
            globalPriority = priority;
            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_REPEAT:
            globalcfID = c->ID;
            globalcftype = c->type;
            globalPriority++;

            MCI(new MCE(mJUMP,t_CODE1,c->ID));
            t = {t_START,c->ID}; queueTag(t);
            genCondition(c->ci,c->ID,t_CODE1,t_END);

            //generate
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch,priority+1);
                ch = ch->next;
            }
            
            MCI(new MCE(mJUMP,t_START,c->ID));
            globalPriority = priority;
            globalcftype = prevcftype;
            globalcfID  = prevID;
            t = {t_END,c->ID}; queueTag(t);
            break;
        case c_FORTO:
            genValue(c->ifvar->from,A);

            globalcfID = c->ID;
            globalcftype = c->type;
            globalPriority++;
            MCI(new MCE(mJUMP,t_CODE1,c->ID));

            t = {t_START,c->ID}; queueTag(t);
            genValue(c->vi,A);
            MCI(new MCE(mINC,A));
            t = {t_CODE1, c->ID}; queueTag(t);
            MCI(new MCE(mSWAP,B));
            genValue(c->ifvar->to,A);
            MCI(new MCE(mSUB,B));
            MCI(new MCE(mJNEG,t_END,c->ID));
            MCI(new MCE(mSWAP,B));
            genSaveValue(c->vi);
            
            
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch,priority+1);
                ch = ch->next;
            }
            
            MCI(new MCE(mJUMP,t_START,c->ID));
            globalPriority = priority;
            globalcftype = prevcftype;
            globalcfID  = prevID;
            t = {t_END,c->ID}; queueTag(t);

        break;
        case c_FORDOWNTO:
            genValue(c->ifvar->from,A);

            globalcfID = c->ID;
            globalcftype = c->type;
            globalPriority++;
            MCI(new MCE(mJUMP,t_CODE1,c->ID));
            
            t = {t_START,c->ID}; queueTag(t);
            genValue(c->vi,A);
            MCI(new MCE(mDEC,A));
            t = {t_CODE1, c->ID}; queueTag(t);
            MCI(new MCE(mSWAP,B));
            genValue(c->ifvar->to,A);
            MCI(new MCE(mSUB,B));
            MCI(new MCE(mJPOS,t_END,c->ID));
            MCI(new MCE(mSWAP,B));
            
            genSaveValue(c->vi);
            
            ch = *(c->children);
            while (ch!=0) {
                genCommand(ch,priority+1);
                ch = ch->next;
            }
            
            MCI(new MCE(mJUMP,t_START,c->ID));
            globalPriority = priority;
            globalcftype = prevcftype;
            globalcfID  = prevID;
            t = {t_END,c->ID}; queueTag(t);
        break;
        case c_ASSIGN:
            genExpression(c->ei);
            genSaveValue(c->vi);
        break;
        case c_READ:
            MCI(new MCE(mREAD,A));
            genSaveValue(c->vi);
        break;
        case c_WRITE:
            genValue(c->vi,A);
            MCI(new MCE(mWRITE,A));
        break;
    }
}

void printTag(MCTag t) {
    cout<<"(";
    switch (t.type){
        case t_START: cout<<"s-"; break;
        case t_CODE1: cout<<"c-"; break;
        case t_CODE2: cout<<"z-"; break;
        case t_END: cout<<"e-"; break;
    }
    cout<<t.ID<<") ";
}

void printMCE(MCE* mc) {
    cout<<"["<<comNames[mc->cftype]<<"] ";

    for (auto v: mc->tags) {
        printTag(v);
    }

    cout<<MCinstrName[mc->ins]<<" ";
    
    switch (mc->var->type) {
        case t_VAL:
            printVal(mc->var->val);
        break;
        case t_TAG: 
            printTag(mc->var->tag);
        break;
        case t_REG: cout<<":"<<regName[mc->var->reg]<<":"; break;
    }

    cout<<"\t";

    for (int i=2;i<8;i++) {
        if (mc->registry[i]) cout<<regName[i]<<" ";
    }

    cout<<endl;
}



