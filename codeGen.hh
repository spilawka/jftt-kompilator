#include <stack>

struct reference {
    bool isFinal;
    bool isRegistry;
    enum reg Registry;
    long long memoryLoc;
};

vector<pair<enum instr, struct reference>> midCode;

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

    while (!code.empty()) {
        pair<enum instr, enum reg> elem = code.top();
        struct reference r;
        r.isFinal = true;
        r.isRegistry = true;
        r.Registry = elem.second;
        
        midCode.push_back(make_pair(elem.first,r));

        code.pop();
    }
}