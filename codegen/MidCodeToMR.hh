//Szymon Pilawka 254649
/*
Plik zawiera funkcje zamieniające kod pośredni określony w pliku MidCodeInstructions.hh
na kod maszynowy MR
*/

#include <fstream>
#include <stdlib.h>

/** Encja przechowująca informację o instrukcji w kodzie wyjściowym */
struct MRIns {
    vector<MCTag> tags;
    enum instr ins;
};

/** Rodzaje argumentów instrukcji wyjściowej. Kolejno: Numer, rejestr, tag, nic */
enum MRRefType {r_NUM,r_REG,r_TAG,r_VOID};
/** Klasa przechowuje informację o argumencie w instrukcji wyjściowej MR */
class MRReference {
public:
    MRRefType type;
    long long num;
    enum reg reg;
    MCTag tag;

    MRReference() {
        this->type = r_VOID;
    }

    MRReference(long long n) {
        this->type = r_NUM;
        this->num = n;
    }

    MRReference(MCTag tag) {
        this->type = r_TAG;
        this->tag = tag;
    }

    MRReference(enum reg reg){
        this->type = r_REG;
        this->reg = reg;
    }

    void exportR(ofstream f) {
        switch(type){
            case r_NUM: f<<num; break;
            case r_REG: f<<regName[reg]; break;
            case r_TAG: yyerror("Nie można exportować tagu!"); break;
            case r_VOID: break;
        }
    }

    void printRef() {
        switch(type){
            case r_NUM: cout<<num; break;
            case r_REG: cout<<regName[reg];
            break;
            case r_TAG: printTag(tag); break;
            case r_VOID: break;
        }
    }
};
typedef class MRReference MRReference;

/** Kod wyjściowy MR z tagami */
vector<pair<struct MRIns, MRReference>> outputCode;
/** Kolejka tagów - używana przy usuwaniu/optymalizacji kodu*/
vector<MCTag> queuedTags = {};

/** Funkcja wstawia instrukcję z argumentem do kolejki oraz dodaje tagi z kolejki */
void OCinsert(enum instr in, MRReference rf) {
    struct MRIns newe = {{},in};
    if (!queuedTags.empty()) {
        newe.tags = queuedTags;
        queuedTags = {};
    }
    outputCode.push_back(make_pair(newe,rf));
}

/** Funkcja wyświetla kod maszynowy na stdout */
void printMRCode() {
    for (auto p: outputCode) {
        for (MCTag t: p.first.tags) printTag(t);
        cout<<instrName[p.first.ins]<<" ";
        p.second.printRef();
        cout<<endl;
    }
}

/** Funkcja eksportuje przechowywany w programie kod maszynowy
 * @param infile plik wejściowy (komentarz na początku pliku)
 * @param outfile plik wyjściowy
 **/
void exportMRCode(char* infile, char* outfile) {
    ofstream f(outfile);
    if (!f) {
        cout<<"Błąd przy eksporcie pliku!"<<endl;
        exit(-1);
    }

    f<<"("<<infile<<")"<<endl;
    for (auto p: outputCode) {
        f<<instrName[p.first.ins]<<" ";
        switch(p.second.type){
            case r_NUM: f<<p.second.num; break;
            case r_REG: f<<regName[p.second.reg]; break;
            case r_TAG: yyerror("Nie można exportować tagu!"); break;
            case r_VOID: break;
        }
        f<<endl;
    }
    f.close();
}

/** Funkcja łączy tagi (oblicza różnicę pomiędzy tagami i przeskokami i zapamiętuje) */
void linkTags() {
    map<pair<MCTagTypes,long long>,long long> tagLoc;

    long long lines = 0;
    for (auto p: outputCode) {
        lines++;
        for (auto t: p.first.tags) {
            tagLoc[make_pair(t.type,t.ID)] = lines;
        }
    }

    lines = 0;
    for (auto p: outputCode) {
        lines++;
        if (p.second.type == r_TAG) {
            MCTag t = p.second.tag;
            long long l = tagLoc[make_pair(t.type,t.ID)];
            if (l==0) yyerror("MCtoMR: Brak tagu " + to_string(t.ID));

            outputCode.at(lines-1) = make_pair(p.first,MRReference(l-lines));
        }
    }
}

/** Stos używany do utworzenia instrukcji generowania numerów */
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
        OCinsert(elem.first,MRReference(elem.second));

        code.pop();
    }

   code = {};
}

/** Funkcja importuje kod maszynowy z pliku
 * Używana dla poleceń DIV,MOD,MULT */
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
            OCinsert(getEnum(ins),MRReference(atoll(line.c_str())));
        }
        else {
            OCinsert(getEnum(ins),MRReference(r));
        }
    }
}

/** Funkcja interpretuje polecenie kodu pośredniego na kod maszynowy z tagami */
void interpretIns(pair<struct MCInstr, class MCDest> p) {
    queuedTags = p.first.tags;
    class MCDest d = p.second;
    class memLoc mL;
    class memLoc mL2;
    long long temp;

    switch (p.first.instr) {
        case mREAD:
            OCinsert(GET, MRReference()); break;
        case mWRITE:
            OCinsert(PUT, MRReference()); break;
        case mSAVE:
            // w A znajduje się wartość do zapisania
            if (d.type == d_1VAR) {
                mL = varMemLoc[d.var1];
                switch (mL.type) {
                    case m_REG:
                        OCinsert(SWAP,MRReference(mL.reg)); break;
                    case m_MEM:
                        OCinsert(SWAP,MRReference(B));
                        genNumber(mL.loc);
                        OCinsert(SWAP,MRReference(B));
                        OCinsert(STORE,MRReference(B));
                        break;
                    case m_CONST:
                        yyerror("MR: Nie można zapisać zmiennej!");
                        break;
                }
            }
            else if (d.type == d_2VAR) {
                //tablica - zawsze m_MEM
                mL = varMemLoc[d.var1];
                mL2 = varMemLoc[d.var2];

                switch(mL2.type) {
                    case m_CONST:
                        //oblicz indeks
                        OCinsert(SWAP,MRReference(B));
                        temp = mL.loc + mL2.loc - mL.offset;
                        genNumber(temp);
                        OCinsert(SWAP,MRReference(B));
                        OCinsert(STORE,MRReference(B));
                        break;
                    case m_REG:
                        OCinsert(SWAP,MRReference(B));
                        temp = mL.loc - mL.offset;
                        genNumber(temp);
                        //dodaj indeks
                        OCinsert(ADD,MRReference(mL2.reg));
                        OCinsert(SWAP,MRReference(B));
                        OCinsert(STORE,MRReference(B));
                        break;
                    case m_MEM:
                        OCinsert(SWAP,MRReference(B));
                        genNumber(mL2.loc);
                        OCinsert(LOAD,MRReference(A));
                        OCinsert(SWAP,MRReference(D));
                        temp = mL.loc - mL.offset;
                        genNumber(temp);
                        //dodaj indeks
                        OCinsert(ADD,MRReference(D));
                        OCinsert(SWAP,MRReference(B));
                        OCinsert(STORE,MRReference(B));
                        break;
                }

            }
            break;
        case mLD:
            // pozostaw B bez zmian
            if (d.type == d_1VAR) {
                mL = varMemLoc[d.var1];
                switch (mL.type) {
                    case m_REG:
                        OCinsert(RESET,MRReference(A));
                        OCinsert(ADD,MRReference(mL.reg)); break;
                    case m_MEM:
                        genNumber(mL.loc);
                        OCinsert(LOAD,MRReference(A));
                        break;
                    case m_CONST:
                        genNumber(mL.loc);
                        break;
                }
            }
            else if (d.type == d_2VAR) {
                //tablica - zawsze m_MEM
                mL = varMemLoc[d.var1];
                mL2 = varMemLoc[d.var2];

                switch(mL2.type) {
                    case m_CONST:
                        //oblicz indeks
                        temp = mL.loc + mL2.loc - mL.offset;
                        genNumber(temp);
                        OCinsert(LOAD,MRReference(A));
                        break;
                    case m_REG:
                        temp = mL.loc - mL.offset;
                        genNumber(temp);
                        //dodaj indeks
                        OCinsert(ADD,MRReference(mL2.reg));
                        OCinsert(LOAD,MRReference(A));
                        break;
                    case m_MEM:
                        genNumber(mL2.loc);
                        OCinsert(LOAD,MRReference(A));
                        OCinsert(SWAP,MRReference(D));
                        temp = mL.loc - mL.offset;
                        genNumber(temp);
                        //dodaj indeks
                        OCinsert(ADD,MRReference(D));
                        OCinsert(LOAD,MRReference(A));
                        break;
                }
            }
            break;
        case mADD:
            OCinsert(ADD, MRReference(d.reg)); break;
        case mSUB:
            OCinsert(SUB, MRReference(d.reg)); break;
        case mSHIFT:
            OCinsert(SHIFT, MRReference(d.reg)); break;
        case mSWAP:
            OCinsert(SWAP, MRReference(d.reg)); break;
        case mRESET:
            OCinsert(RESET, MRReference(d.reg)); break;
        case mINC:
            OCinsert(INC, MRReference(d.reg)); break;
        case mDEC:
            OCinsert(DEC, MRReference(d.reg)); break;
        case mJUMP:
            if (d.type == d_CONST) OCinsert(JUMP, MRReference(d.n)); 
            else if (d.type == d_TAG) OCinsert(JUMP, MRReference(d.tag)); break;
        case mJPOS:
            if (d.type == d_CONST) OCinsert(JPOS, MRReference(d.n)); 
            else if (d.type == d_TAG) OCinsert(JPOS, MRReference(d.tag)); break;
        case mJZERO:
            if (d.type == d_CONST) OCinsert(JZERO, MRReference(d.n)); 
            else if (d.type == d_TAG) OCinsert(JZERO, MRReference(d.tag)); break;
        case mJNEG:
            if (d.type == d_CONST) OCinsert(JNEG, MRReference(d.n)); 
            else if (d.type == d_TAG) OCinsert(JNEG, MRReference(d.tag)); break;
        case mTIMES:
            injectFilesCode("frag/mult2clean.mr");
            break;
        case mDIV:
            injectFilesCode("frag/div2clean.mr");
            break;
        case mMOD:
            injectFilesCode("frag/div2clean.mr");
            break;
        case mHALT:
            OCinsert(HALT, MRReference());
        break;
    }
}

/** Funkcja generuje kod maszynowy używając istniejących funkcji
 * @param MCA Kod pośredni z Argumentami
 **/
void generateMR(vector<pair<MCInstr,MCDest>> MCA) {
    for (auto p: MCA) {
        interpretIns(p);
    }
    //printMRCode();
    linkTags();
    //printMRCode();
}