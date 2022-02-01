
/** Eliminacja przeskoków o 1*/
void eliminateJumpOne() {
    for (auto i = midCode.begin(); i < midCode.end()-1; i++ ) {
        if (i[0]->ins == mJUMP && i[0]->var->type == t_TAG) {
            MCTag t0 = i[0]->var->tag;
            vector<MCTag> tags1 = i[1]->tags;

            for (MCTag t: tags1) if (t0.type == t.type && t0.ID == t.ID) {
                //mamy przeskok o 1
                i[1]->tags.insert(i[0]->tags.end(), i[0]->tags.begin(), i[0]->tags.end());

                i = midCode.erase(i);
                break;
            }
        }
    }
}

void optimizeMC() {
    eliminateJumpOne();
}
