enum MCinstr {
    mREAD, mWRITE, mSAVE, mADD, mSUB, mSHIFT, mSWAP, mRESET, mINC, mDEC, mJUMP, mJPOS, mJZERO, mJNEG, mLDA, mLDB, mLDC, mLDD, mLDE, mLDF, mLDG, mLDH, mTIMES, mDIV, mMOD, mHALT
};

string MCinstrName []= {"READ","WRITE","SAVE","ADD","SUB","SHIFT","SWAP","RESET","INC","DEC","JUMP","JPOS","JZERO","JNEG", "LDA", "LDB", "LDC", "LDD", "LDE", "LDF", "LDG", "LDH", "TIMES", "DIV", "MOD", "HALT"};
 
enum MCinstr getLoadReg(enum reg r) {
    switch (r) {
        case A: return mLDA;
        case B: return mLDB;
        case C: return mLDC;
        case D: return mLDD;
        case E: return mLDE;
        case F: return mLDF;
        case G: return mLDG;
        case H: return mLDH;
        default: return mLDA;
    }
}