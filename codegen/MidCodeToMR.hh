#include <fstream>
#include <stdlib.h>

struct EE {
    vector <MCTag> tags;
    enum instr inst;
};

vector<pair<struct EE,class ECD*>> outputCode;
vector<MCTag> queuedTags = {};

void OCinsert(pair<enum instr,class ECD*> p) {
    struct EE newe = {{},p.first};
    if (!queuedTags.empty()) {
        newe.tags = queuedTags;
        queuedTags = {};
    }
    outputCode.push_back(make_pair(newe,p.second));
}

typedef stack<pair<enum instr, enum reg>> insstack;
/** Funkcja zwraca kod do generowania danej liczby od podstaw w kodzie maszynowym 

    REJESTRY: A,C
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
            code.push(make_pair(SUB,C));
            code.push(make_pair(RESET,A));
            code.push(make_pair(SWAP,C));
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
                    code.push(make_pair(SHIFT,C));
                }
                // dla więcej operacji optymalizacja - bardziej
                // opłaca się zwiększać b niż wykonywać SHIFTy
                else if (conseq2>1) {
                    code.push(make_pair(INC,C));
                    code.push(make_pair(RESET,C));
                    code.push(make_pair(SHIFT,C));
                    for(int i=1;i<conseq2;i++){
                        code.push(make_pair(INC,C));
                    }
                }
                conseq2=0;

                code.push(make_pair(INC,A));
                if (num!=1)
                    code.push(make_pair(SHIFT,C));
            }

            num = num/2;
        }

        //reset rejestrów
        code.push(make_pair(INC,C));
        code.push(make_pair(RESET,C));
        code.push(make_pair(RESET,A));
    }
    
    while (!code.empty()) {
        pair<enum instr, enum reg> elem = code.top();
        class ECD* loc = new class ECD(elem.second);
        OCinsert(make_pair(elem.first,loc));

        code.pop();
    }

   code = {};
}

void injectFilesCode(string filename) {
    ifstream f(filename);
    
    string line;
    string del = " ";
    while (getline(f,line)) {
        size_t pos = line.find(del);
        string ins = line.substr(0,pos);
        line.erase(0,pos+1);

        line = line.substr(0,line.find(del));
        enum reg r = getReg(line);
        if (r == UNDEF) {
            OCinsert(make_pair(getEnum(ins),new ECD(atoll(line.c_str()))));
        }
        else {
            OCinsert(make_pair(getEnum(ins),new ECD(r)));
        }
    }
}

void interpretIns(pair<struct ECE, class ECD*> p) {
    queuedTags = p.first.tags;
    enum ECDType t;
    switch (p.first.instr) {
        case mREAD:
            OCinsert(make_pair(GET,new ECD())); break;
        case mWRITE:
            OCinsert(make_pair(PUT,new ECD())); break;
        case mSAVE:
            t = p.second->type;
            if (t == ECD_reg) {
                OCinsert(make_pair(SWAP,p.second));
            }
            else if (t== ECD_num) {
                OCinsert(make_pair(SWAP,new ECD(B)));
                genNumber(p.second->num);
                OCinsert(make_pair(SWAP,new ECD(B)));
                OCinsert(make_pair(STORE,new ECD(B)));
            }
            break;
        case mADD:
            OCinsert(make_pair(ADD,p.second)); break;
        case mSUB:
            OCinsert(make_pair(SUB,p.second)); break;
        case mSHIFT:
            OCinsert(make_pair(SHIFT,p.second)); break;
        case mSWAP:
            OCinsert(make_pair(SWAP,p.second)); break;
        case mRESET:
            OCinsert(make_pair(RESET,p.second)); break;
        case mINC:
            OCinsert(make_pair(INC,p.second)); break;
        case mDEC:
            OCinsert(make_pair(DEC,p.second)); break;
        case mJUMP:
            OCinsert(make_pair(JUMP,p.second)); break;
        case mJPOS:
            OCinsert(make_pair(JPOS,p.second)); break;
        case mJZERO:
            OCinsert(make_pair(JZERO,p.second)); break;
        case mJNEG:
            OCinsert(make_pair(JNEG,p.second)); break;
        case mLD:
            t = p.second->type;
            if (t == ECD_reg) {
                OCinsert(make_pair(RESET,new ECD(A)));
                OCinsert(make_pair(ADD,p.second));
            }
            else if (t == ECD_num) {
                genNumber(p.second->num);
                OCinsert(make_pair(SWAP,new ECD(C)));
                OCinsert(make_pair(LOAD,new ECD(C)));
            }
        case mTIMES:
            injectFilesCode("frag/mult2clean.mr");
            break;
        case mDIV:
            injectFilesCode("frag/div2clean.mr");
            break;
        case mMOD:
            injectFilesCode("frag/div2clean.mr");
            OCinsert(make_pair(SWAP,new ECD(B)));
            break;
        case mHALT:
            OCinsert(make_pair(HALT,new ECD())); break;
        break;
    }
}

void printOutputLine(pair<struct EE,class ECD*> p) {
    if (!p.first.tags.empty()) {
        for (auto v: p.first.tags) {
            cout<<"<";
            switch (v.type){
                case t_START: cout<<"s-"; break;
                case t_CODE1: cout<<"c-"; break;
                case t_CODE2: cout<<"z-"; break;
                case t_END: cout<<"e-"; break;
            }
            cout<<v.ID<<"> ";
        }
    }

    cout<<instrName[p.first.inst]<<" ";

    switch (p.second->type) {
        case ECD_num:
            cout<<p.second->num; break;
        case ECD_reg:
            cout<<regName[p.second->reg]; break;
        case ECD_tag:
            cout<<"<";
            switch (p.second->tag.type){
                case t_START: cout<<"s-"; break;
                case t_CODE1: cout<<"c-"; break;
                case t_CODE2: cout<<"z-"; break;
                case t_END: cout<<"e-"; break;
            }
            cout<<p.second->tag.ID<<">";
            break;
        default:
        break;
    }
    cout<<endl; 
}

void printOutputLines() {
    for (auto l: outputCode) {
        printOutputLine(l);
    }
}