Szymon Pilawka
Projekt kompilatora z użyciem flex, bison, c++

Zależności:
 te same co maszyna wirtualna

Kompilacja:

    make

    make clean - usuń niepotrzebne pliki

Użycie:

    ./kompilator plik_wejściowy plik_wyjściowy

Pliki:

    Makefile - kompilacja kodu

    parser.y - parser

    lekser.l - lekser

    /nterms - struktury do zapamiętywania informacji z kodu wejściowego, używane w parserze
        valinfo.hh - zmienne/stałe
        exprinfo.hh - wyrażenia
        condinfo.hh - warunki
        cominfo.hh - polecenia

    /codegen - pliki odpowiedzialne za generowanie kodu pośredniego i wyjściowego
        VerifySymbols.hh - sprawdź poprawność użycia symboli w kodzie wejściowym
        MidCodeInstructions.hh - instrukcje kodu pośredniego
        MidCodeGenerator.hh - generator kodu pośredniego
        ControlSymbols.hh - alokacja zmiennych
        MidCodeToMR.hh - przekształć kod pośredni na wyjściowy
    
    /frag - fragmenty kodu maszynowego do konkretnych operacji
        div2clean.mr - kod dla operacji dzielenia oraz modulo
        mult2clean.mr - kod dla mnożenia
        * - nieużywany kod
    
        