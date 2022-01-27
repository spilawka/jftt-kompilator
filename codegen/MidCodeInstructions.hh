/** Instrukcje kodu pośredniego */
enum MCinstr {
    mREAD, mWRITE, mSAVE, mADD, mSUB, mSHIFT, mSWAP, mRESET, mINC, mDEC, mJUMP, mJPOS, mJZERO, mJNEG, mLD, mTIMES, mDIV, mMOD, mHALT
};

string MCinstrName []= {"READ","WRITE","SAVE","ADD","SUB","SHIFT","SWAP","RESET","INC","DEC","JUMP","JPOS","JZERO","JNEG", "LD", "TIMES", "DIV", "MOD", "HALT"};