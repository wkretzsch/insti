#ifndef _SNP_H
#define _SNP_H 1

#include <string>

class snp {
public:
    unsigned pos;
    std::string ref, alt;

    snp(int pos, std::string ref, std::string alt)
    {
        this->pos = pos;
        this->ref = ref;
        this->alt = alt;
    }

    bool strand(std::string & r, std::string & a)
    {
        if (ref == r && alt == a) return true;

        return false;
    }
};

#endif
