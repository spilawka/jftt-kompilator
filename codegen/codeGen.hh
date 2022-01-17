#include <stack>
#include <algorithm>
#include "mcInstructions.hh"

enum MCTagTypes {t_START,t_END,t_CODE1,t_CODE2};
struct MCTag {
    enum MCTagTypes type;
    long long ID;
};
typedef struct MCTag MCTag;

enum MCVarTypes {t_VAR,t_TAG,t_CONST,t_REG};
struct MCVar {
    enum MCVarTypes type;
    char* name;
    long long num;
    enum reg reg;
    MCTag tag;
};
typedef struct MCVar MCVar;

class MCEntry {
public:
    vector<MCTag> tags;
    enum MCinstr ins;
    MCVar* var;
    MCVar* registry[8];

    MCEntry() {
        this->tags = {};
        this->var = new MCVar();
        for(int i=0;i<8;i++) registry[i]=0;
    }

    /** Konstruktor dla stałej liczby*/
    MCEntry(enum MCinstr i, long long num): MCEntry(){
        this->ins = i;
        this->var->type = t_CONST;
        this->var->num = num;
    }
    /** Konstruktor dla zmiennej*/
    MCEntry(enum MCinstr i, char* name): MCEntry(){
        this->ins = i;
        this->var->type = t_VAR;
        this->var->name = name;
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
};
typedef class MCEntry MCEntry;

vector<MCEntry*> midCode;
MCVar* mcRegistry[8] = {0,0,0,0,0,0,0,0};

bool isTagQueued = false;
MCTag queuedTag;

void queueTag(MCTag tag) {
    isTagQueued = true;
    queuedTag = tag;
}

void MCInsert(MCEntry* newEntry) {
    if (isTagQueued) {
        newEntry->addTag(queuedTag);
        isTagQueued = false;
    }
    
    midCode.push_back(newEntry);

    
}

void genValue (valinfo* v, enum reg targetReg) {
    switch (v->type) {
        case NUM:
            MCInsert(new MCEntry(getLoadReg(targetReg),v->num)); break;
        case ELEM:
            MCInsert(new MCEntry(getLoadReg(targetReg),v->varName)); break;
        case TELEM:
            MCInsert(new MCEntry(mLDA,v->index));
            MCInsert(new MCEntry(mLOADT,v->varName));
            if (targetReg!=A) {
                MCInsert(new MCEntry(mSWAP,targetReg));
            }
            break;
        case TELEMID:
            MCInsert(new MCEntry(mLDA,v->indexName));
            MCInsert(new MCEntry(mLOADT,v->varName));
            if (targetReg!=A) {
                MCInsert(new MCEntry(mSWAP,targetReg));
            } 
            break;
    }
}

void genSaveValue (valinfo* v) {
    switch (v->type) {
        case NUM:
            yyerrorline("Nie można zapisać stałej!",0); break;
        case ELEM:
            MCInsert(new MCEntry(mSAVE,v->varName)); break; 
        case TELEM:
            MCInsert(new MCEntry(mLDB,v->index));
            MCInsert(new MCEntry(mSAVET,v->varName));
            break;
        case TELEMID:
            MCInsert(new MCEntry(mLDB,v->indexName));
            MCInsert(new MCEntry(mSAVET,v->varName));
            break;
    }
}

void genCondition(condinfo* c, long long id, enum MCTagTypes conttag, enum MCTagTypes exittag) {
    genValue(c->v2,B);
    genValue(c->v1,A);
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
        case c_LEQ:
            MCInsert(new MCEntry(mJPOS,exittag,id));
        case c_GE:
            MCInsert(new MCEntry(mJPOS,conttag,id));
            MCInsert(new MCEntry(mJUMP,exittag,id));
        case c_GEQ:
            MCInsert(new MCEntry(mJNEG,exittag,id));
    }

    t = {conttag,id}; queueTag(t);
}

void genTimes(exprinfo* e) {
    genValue(e->v2,B);
    genValue(e->v1,A);
    MCInsert(new MCEntry(mTIMES,B));
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
            genValue(e->v2,B);
            genValue(e->v1,A);
            MCInsert(new MCEntry(mSUB,B));
            break;
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
            break;
        case c_FORDOWNTO:
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
        cout<<"[";
        switch (v.type){
            case t_START: cout<<"s-"; break;
            case t_CODE1: cout<<"c-"; break;
            case t_CODE2: cout<<"z-"; break;
            case t_END: cout<<"e-"; break;
        }
        cout<<v.ID<<"] ";
    }

    cout<<MCinstrName[mc->ins]<<" ";
    
    switch (mc->var->type) {
        case t_VAR: cout<<mc->var->name; break;
        case t_TAG: 
            cout<<"[";
            switch (mc->var->tag.type){
                case t_START: cout<<"s-"; break;
                case t_CODE1: cout<<"c-"; break;
                case t_CODE2: cout<<"z-"; break;
                case t_END: cout<<"e-"; break;
            }
            cout<<mc->var->tag.ID<<"]";
        break;
        case t_CONST: cout<<mc->var->num; break;
        case t_REG: cout<<":"<<regName[mc->var->reg]<<":"; break;
    }
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