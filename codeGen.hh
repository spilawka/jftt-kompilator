#include <stack>



typedef stack<pair<enum instr, string>> insstack;

/** Funkcja zwraca kod do generowania danej liczby od podstaw w kodzie maszynowym 
    @return stos LILO z kodem instrukcji oraz modyfikowanym rejestrem
*/
insstack genNumber(long long num) {
    //stack z kodem maszynowym
    insstack code = {};

    //małe liczby (abs(n)<10)
    if (num == 0) {
        code.push(make_pair(RESET,"a"));
        return code;
    }
    else if (num > 0 && num < 10) {
        for (int i=0; i<num; i++)
            code.push(make_pair(INC,"a"));
        code.push(make_pair(RESET,"a"));
        return code;
    }
    else if (num > -10 && num < 0) {
        for (int i=0; i>num; i--) 
            code.push(make_pair(DEC,"a"));
        code.push(make_pair(RESET,"a"));
        return code;
    }

    //liczby ujemne - weź odwrotność i odejmij pod sam koniec
    if (num<0) {
        num = -num;
        code.push(make_pair(SUB,"b"));
        code.push(make_pair(RESET,"a"));
        code.push(make_pair(SWAP,"b"));
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
                code.push(make_pair(SHIFT,"b"));
            }
            // dla więcej operacji optymalizacja - bardziej
            // opłaca się zwiększać b niż wykonywać SHIFTy
            else if (conseq2>1) {
                code.push(make_pair(INC,"b"));
                code.push(make_pair(RESET,"b"));
                code.push(make_pair(SHIFT,"b"));
                for(int i=1;i<conseq2;i++){
                    code.push(make_pair(INC,"b"));
                }
            }
            conseq2=0;

            code.push(make_pair(INC,"a"));
            if (num!=1)
                code.push(make_pair(SHIFT,"b"));
        }

        num = num/2;
    }

    //reset rejestrów
    code.push(make_pair(INC,"b"));
    code.push(make_pair(RESET,"b"));
    code.push(make_pair(RESET,"a"));

    return code;
}

void printCode(long long num) {
    insstack code = genNumber(num);

    while (!code.empty()) {
        pair<enum instr, string> elem = code.top();
        cout<<instrName[elem.first]<<" "<<elem.second<<endl;
        code.pop();
    }

    cout<<"PUT"<<endl;
    cout<<"HALT"<<endl;
}