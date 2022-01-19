
enum ECDType {ECD_num, ECD_reg, ECD_tag, ECD_empty };
class ECD {
public:
    enum ECDType type;
    MCTag tag;
    long long num;
    enum reg reg;

    ECD() {
        this->type = ECD_empty;
    }

    ECD(MCTag tag) {
        this->type = ECD_tag;
        this->tag = tag;
    }

    ECD(long long num) {
        this->type = ECD_num;
        this->num = num;
    }

    ECD (enum reg reg) {
        this->type = ECD_reg;
        this->reg = reg;
    }
};

struct ECE {
    enum MCinstr instr;
    vector<MCTag> tags;
};

map<long long, bool[6]> regsUsed(vector<MCE*> midcode) {
    map<long long, bool[6]> regusage;

    for (MCE* m: midcode) {
        long long cfID = m->cfID;
        if (regusage.count(cfID)==0) {
            bool nt[6];
            for (int i=0;i<6;i++) {
                regusage[cfID][i] = true;
            }
        }
        for (int i=0;i<6;i++) {
            if (!m->registry[i+2]) regusage[cfID][i]=false;
        }
    }

    return regusage;
}

class VarUsage {

};

map<char*,VarUsage> m;

void printregsUsed(map<long long,bool[6]> m) {
    for (auto const& e: m) {
        cout<<e.first<<": ";
        for(int i=0;i<6;i++) {
            if (e.second[i]) cout<<regName[i+2]<<" ";
        }
        cout<<endl;
    }
}