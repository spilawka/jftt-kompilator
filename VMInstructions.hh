enum instr {
    GET, PUT, LOAD, STORE, ADD, SUB, SHIFT, SWAP, RESET, INC, DEC, JUMP, JPOS, JZERO, JNEG, HALT
};

string instrName []= {"GET","PUT","LOAD","STORE","ADD","SUB","SHIFT","SWAP","RESET","INC","DEC","JUMP","JPOS","JZERO","JNEG","HALT"};

enum instr getEnum(string s) {
    if (s=="GET") return GET;
    else if (s=="PUT") return PUT;
    else if (s=="LOAD") return LOAD;
    else if (s=="STORE") return STORE;
    else if (s=="ADD") return ADD;
    else if (s=="SUB") return SUB;
    else if (s=="SHIFT") return SHIFT;
    else if (s=="SWAP") return SWAP;
    else if (s=="RESET") return RESET;
    else if (s=="INC") return INC;
    else if (s=="DEC") return DEC;
    else if (s=="JUMP") return JUMP;
    else if (s=="JPOS") return JPOS;
    else if (s=="JZERO") return JZERO;
    else if (s=="JNEG") return JNEG;
    else if (s=="HALT") return HALT;
    return HALT;
}

enum reg {
    A,B,C,D,E,F,G,H,UNDEF
};

string regName[] = {"a","b","c","d","e","f","g","h"};

enum reg getReg(string s) {
    if (s=="a") return A;
    else if (s=="b") return B;
    else if (s=="c") return C;
    else if (s=="d") return D;
    else if (s=="e") return E;
    else if (s=="f") return F;
    else if (s=="g") return G;
    else if (s=="h") return H;
    return UNDEF;
}