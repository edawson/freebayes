#include "Allele.h"
#include "multichoose.h"
#include "TryCatch.h"


int Allele::referenceOffset(void) const {
    /*cout << readID << " offset checked " << referencePosition - position << " against position " << position 
        << " allele length " << length << " str length " << referenceSequence.size() << " qstr size " << qualityString.size() << endl;
        */
    return *currentReferencePosition - position;
}

// quality at a given reference position
const short Allele::currentQuality(void) const {
    switch (this->type) {
        case ALLELE_REFERENCE:
            TRY {
            return baseQualities.at(referenceOffset());
            } CATCH;
            break;
        case ALLELE_INSERTION:
        case ALLELE_DELETION:
        case ALLELE_SNP:
            return quality;
            break;
    }
}

const long double Allele::lncurrentQuality(void) const {
    return phred2ln(currentQuality());
}

string Allele::typeStr(void) {

    string t;

    switch (this->type) {
        case ALLELE_GENOTYPE:
            t = "genotype";
            break;
        case ALLELE_REFERENCE:
            t = "reference";
            break;
        case ALLELE_MISMATCH:
            t = "mismatch";
            break;
        case ALLELE_SNP:
            t = "snp";
            break;
        case ALLELE_INSERTION:
            t = "insertion";
            break;
        case ALLELE_DELETION:
            t = "deletion";
            break;
        case ALLELE_CNV:
            t = "cnv";
            break;
        default:
            t = "unknown";
            break;
    }

    return t;

}

inline
const string Allele::base(void) const { // the base of this allele

    if (genotypeAllele)
        return alternateSequence;

    switch (this->type) {
        case ALLELE_GENOTYPE:
            return alternateSequence;
            break;
        case ALLELE_REFERENCE:
            return alternateSequence.substr(referenceOffset(), 1);
            break;
        //case ALLELE_MISMATCH:
            //break;
        case ALLELE_SNP:
            return alternateSequence;
            break;
        case ALLELE_INSERTION:
            return "I" + length; // unclear what to do here
            break;
        case ALLELE_DELETION:
            return "D" + length;
            break;
        default:
            break;
    }

}

const bool Allele::masked(void) const {

    // guard against uninitialized indelMask
    if (indelMask.size() == 0)
        return false;

    if (genotypeAllele)
        return false;

    switch (this->type) {
        case ALLELE_GENOTYPE:
            return false;
            break;
        case ALLELE_REFERENCE:
            return indelMask.at(referenceOffset());
            break;
        case ALLELE_SNP:
            return indelMask.at(0);
            break;
        case ALLELE_INSERTION: // XXX presently these are masked by default...
            return true;
            break;
        case ALLELE_DELETION:
            return true;
            break;
        default:
            break;
    }

}

string stringForAllele(Allele &allele) {

    stringstream out;
    if (!allele.genotypeAllele) {
        out 
            << allele.sampleID << "\t"
            << allele.readID << "\t"
            << allele.typeStr() << "\t" 
            << allele.position << "\t"
            << allele.length << "\t"
            << (allele.strand == STRAND_FORWARD ? "+" : "-") << "\t"
            << allele.referenceSequence << "\t"
            << allele.alternateSequence << "\t"
            << allele.qualityString << "\t"
            << allele.quality << "\t" << endl;
    } else {
        out << allele.typeStr() << "\t"
            << allele.length << "\t"
            << allele.alternateSequence
            << endl;
    }

    return out.str();
}

string stringForAlleles(vector<Allele> &alleles) {
    stringstream out;
    for (vector<Allele>::iterator allele = alleles.begin(); allele != alleles.end(); allele++) {
        out << stringForAllele(*allele) << endl;
    }
    return out.str();
}

string json(vector<Allele*> &alleles) {
    stringstream out;
    vector<Allele*>::iterator a = alleles.begin();
    out << "[" << json(**a); ++a;
    for (; a != alleles.end(); ++a)
        out << "," << json(**a);
    out << "]";
    return out.str();
}

string json(Allele*& allele) { return json(*allele); }

string json(Allele& allele) {
    int referenceOffset = allele.referenceOffset();
    stringstream out;
    if (!allele.genotypeAllele) {
        out << "{\"id\":\"" << allele.readID << "\""
            << ",\"type\":\"" << allele.typeStr() << "\""
            << ",\"length\":" << ((allele.type == ALLELE_REFERENCE) ? 1 : allele.length)
            << ",\"position\":" << allele.position 
            << ",\"strand\":\"" << (allele.strand == STRAND_FORWARD ? "+" : "-") << "\"";
        if (allele.type == ALLELE_REFERENCE ) {
            out << ",\"base\":\"" << allele.alternateSequence.at(referenceOffset) << "\""
                //<< ",\"reference\":\"" << allele.referenceSequence.at(referenceOffset) << "\""
                << ",\"quality\":" << allele.currentQuality();
        } else {
            out << ",\"base\":\"" << allele.alternateSequence << "\""
                //<< ",\"reference\":\"" << allele.referenceSequence << "\""
                << ",\"quality\":" << allele.quality;
        }
        out << "}";

    } else {
        out << "{\"type\":\"" << allele.typeStr() << "\"";
        switch (allele.type) {
            case ALLELE_REFERENCE:
                out << "}";
                break;
            default:
                out << "\",\"length\":" << allele.length 
                    << ",\"alt\":\"" << allele.alternateSequence << "\"}";
                break;
        }
    }
    return out.str();
}

ostream &operator<<(ostream &out, vector<Allele*> &alleles) {
    vector<Allele*>::iterator a = alleles.begin();
    out << **a++;
    while (a != alleles.end())
        out << "|" << **a++;
    return out;
}

ostream &operator<<(ostream &out, vector<Allele> &alleles) {
    vector<Allele>::iterator a = alleles.begin();
    out << *a++;
    while (a != alleles.end())
        out << "|" << *a++;
    return out;
}

ostream &operator<<(ostream &out, list<Allele*> &alleles) {
    list<Allele*>::iterator a = alleles.begin();
    out << **a++;
    while (a != alleles.end())
        out << "|" << **a++;
    return out;
}

ostream &operator<<(ostream &out, Allele* &allele) {
    out << *allele;
    return out;
}

ostream &operator<<(ostream &out, Allele &allele) {

    if (!allele.genotypeAllele) {
        // << &allele << ":" 
        out << allele.readID 
            << ":" << allele.typeStr() 
            << ":" << allele.length 
            << ":" << allele.position 
            << ":" << (allele.strand == STRAND_FORWARD ? "+" : "-")
            << ":" << allele.alternateSequence
            //<< ":" << allele.referenceSequence
            << ":" << allele.quality;
    } else {
        out << allele.typeStr() 
            << ":" << allele.length 
            << ":" << (string) allele.alternateSequence;
    }
    return out;
}

bool operator<(const Allele &a, const Allele &b) {
    return a.base() < b.base();
}

// alleles are equal if they represent the same reference-relative variation or
// sequence, which we encode as a string and compare here
bool operator==(const Allele &a, const Allele &b) {
    return a.base() == b.base();
}

bool operator!=(const Allele& a, const Allele& b) {
    return ! (a == b);
}

bool Allele::equivalent(Allele &b) {

    if (type != b.type) {
        return false;
    } else {
        switch (type) {
            case ALLELE_SNP:
                if (alternateSequence == b.alternateSequence)
                    return true;
                break;
            case ALLELE_DELETION:
                if (length == b.length)
                    return true;
                break;
            case ALLELE_INSERTION:
                if (length == b.length 
                    && alternateSequence == b.alternateSequence)
                    return true;
                break;
            case ALLELE_REFERENCE:
                if (base() == b.base())
                    return true;
                break;
            default:
                break;
        }
    }

    return false;
}

// counts alleles which satisfy operator==
map<Allele, int> countAlleles(vector<Allele*>& alleles) {
    map<Allele, int> counts;
    for (vector<Allele*>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele& allele = **a;
        map<Allele, int>::iterator f = counts.find(allele);
        if (f == counts.end()) {
            counts[allele] = 1;
        } else {
            counts[allele] += 1;
        }
    }
    return counts;
}

// counts alleles which satisfy operator==
map<string, int> countAllelesString(vector<Allele*>& alleles) {
    map<string, int> counts;
    for (vector<Allele*>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele& thisAllele = **a;
        const string& allele = thisAllele.base();
        map<string, int>::iterator f = counts.find(allele);
        if (f == counts.end()) {
            counts[allele] = 1;
        } else {
            counts[allele] += 1;
        }
    }
    return counts;
}

map<string, int> countAllelesString(vector<Allele>& alleles) {
    map<string, int> counts;
    for (vector<Allele>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele& thisAllele = *a;
        const string& allele = thisAllele.base();
        map<string, int>::iterator f = counts.find(allele);
        if (f == counts.end()) {
            counts[allele] = 1;
        } else {
            counts[allele] += 1;
        }
    }
    return counts;
}


map<Allele, int> countAlleles(vector<Allele>& alleles) {
    map<Allele, int> counts;
    for (vector<Allele>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele& allele = *a;
        map<Allele, int>::iterator f = counts.find(allele);
        if (f == counts.end()) {
            counts[allele] = 1;
        } else {
            counts[allele] += 1;
        }
    }
    return counts;
}

map<Allele, int> countAlleles(list<Allele*>& alleles) {
    map<Allele, int> counts;
    for (list<Allele*>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele& allele = **a;
        map<Allele, int>::iterator f = counts.find(allele);
        if (f == counts.end()) {
            counts[allele] = 1;
        } else {
            counts[allele] += 1;
        }
    }
    return counts;
}


map<string, vector<Allele*> > groupAllelesBySample(list<Allele*>& alleles) {
    map<string, vector<Allele*> > groups;
    for (list<Allele*>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele*& allele = *a;
        groups[allele->sampleID].push_back(allele);
    }
    return groups;
}

vector<Allele> uniqueAlleles(list<Allele*>& alleles) {
    vector<Allele> uniques;
    map<Allele, int> counts = countAlleles(alleles);
    for (map<Allele, int>::iterator c = counts.begin(); c != counts.end(); ++c) {
        uniques.push_back(c->first);
    }
    return uniques;
}

void groupAllelesBySample(list<Allele*>& alleles, map<string, vector<Allele*> >& groups) {
    for (list<Allele*>::iterator a = alleles.begin(); a != alleles.end(); ++a) {
        Allele*& allele = *a;
        groups[allele->sampleID].push_back(allele);
    }
}

vector<vector<Allele*> >  groupAlleles(list<Allele*> &alleles, bool (*fncompare)(Allele* &a, Allele* &b)) {
    vector<vector<Allele*> > groups;
    for (list<Allele*>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele*> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if ((*fncompare)(*oa, ag->front())) {
                ag->push_back(*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele*> trueAlleleGroup;
            trueAlleleGroup.push_back(*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

vector<vector<Allele*> >  groupAlleles(list<Allele> &alleles, bool (*fncompare)(Allele &a, Allele &b)) {
    vector<vector<Allele*> > groups;
    for (list<Allele>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele*> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if ((*fncompare)(*oa, *ag->front())) {
                ag->push_back(&*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele*> trueAlleleGroup;
            trueAlleleGroup.push_back(&*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

vector<vector<Allele*> >  groupAlleles(vector<Allele*> &alleles, bool (*fncompare)(Allele &a, Allele &b)) {
    vector<vector<Allele*> > groups;
    for (vector<Allele*>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele*> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if ((*fncompare)(**oa, *ag->front())) {
                ag->push_back(*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele*> trueAlleleGroup;
            trueAlleleGroup.push_back(*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

vector<vector<Allele*> >  groupAlleles(vector<Allele> &alleles, bool (*fncompare)(Allele &a, Allele &b)) {
    vector<vector<Allele*> > groups;
    for (vector<Allele>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele*> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if ((*fncompare)(*oa, *ag->front())) {
                ag->push_back(&*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele*> trueAlleleGroup;
            trueAlleleGroup.push_back(&*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

vector<vector<Allele> >  groupAlleles_copy(list<Allele> &alleles, bool (*fncompare)(Allele &a, Allele &b)) {
    vector<vector<Allele> > groups;
    for (list<Allele>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if ((*fncompare)(*oa, ag->front())) {
                ag->push_back(*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele> trueAlleleGroup;
            trueAlleleGroup.push_back(*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

vector<vector<Allele> >  groupAlleles_copy(vector<Allele> &alleles, bool (*fncompare)(Allele &a, Allele &b)) {
    vector<vector<Allele> > groups;
    for (vector<Allele>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if ((*fncompare)(*oa, ag->front())) {
                ag->push_back(*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele> trueAlleleGroup;
            trueAlleleGroup.push_back(*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

vector<vector<Allele> >  groupAlleles_copy(vector<Allele> &alleles) {
    vector<vector<Allele> > groups;
    for (vector<Allele>::iterator oa = alleles.begin(); oa != alleles.end(); ++oa) {
        bool unique = true;
        for (vector<vector<Allele> >::iterator ag = groups.begin(); ag != groups.end(); ++ag) {
            if (*oa == ag->front()) {
                ag->push_back(*oa);
                unique = false;
                break;
            }
        }
        if (unique) {
            vector<Allele> trueAlleleGroup;
            trueAlleleGroup.push_back(*oa);
            groups.push_back(trueAlleleGroup);
        }
    }
    return groups;
}

bool Allele::sameSample(Allele &other) { return this->sampleID == other.sampleID; }

bool allelesSameType(Allele* &a, Allele* &b) { return a->type == b->type; }

bool allelesEquivalent(Allele* &a, Allele* &b) { return a->equivalent(*b); }

bool allelesSameSample(Allele* &a, Allele* &b) { return a->sampleID == b->sampleID; }

bool allelesSameType(Allele &a, Allele &b) { return a.type == b.type; }

bool allelesEquivalent(Allele &a, Allele &b) { return a.equivalent(b); }

bool allelesSameSample(Allele &a, Allele &b) { return a.sampleID == b.sampleID; }

bool allelesEqual(Allele &a, Allele &b) { return a == b; }

vector<Allele> genotypeAllelesFromAlleleGroups(vector<vector<Allele*> > &groups) {

    vector<Allele> results;
    for (vector<vector<Allele*> >::iterator g = groups.begin(); g != groups.end(); ++g)
        results.push_back(genotypeAllele(*g->front()));
    return results;

}

vector<Allele> genotypeAllelesFromAlleles(vector<Allele*> &alleles) {

    vector<Allele> results;
    for (vector<Allele*>::iterator a = alleles.begin(); a != alleles.end(); ++a)
        results.push_back(genotypeAllele(**a));
    return results;

}

vector<Allele> genotypeAllelesFromAlleleGroups(vector<vector<Allele> > &groups) {

    vector<Allele> results;
    for (vector<vector<Allele> >::iterator g = groups.begin(); g != groups.end(); ++g)
        results.push_back(genotypeAllele(g->front()));
    return results;

}

vector<Allele> genotypeAllelesFromAlleles(vector<Allele> &alleles) {

    vector<Allele> results;
    for (vector<Allele>::iterator a = alleles.begin(); a != alleles.end(); ++a)
        results.push_back(genotypeAllele(*a));
    return results;

}

Allele genotypeAllele(Allele &a) {
    return Allele(a.type, a.alternateSequence, a.length, a.position);
}

Allele genotypeAllele(AlleleType type, string alt, unsigned int len) {
    return Allele(type, alt, len);
}

/*
vector<Allele> uniqueAlleleObservations(vector<vector<Allele> > &alleleObservations) {
}
*/


void* AlleleFreeList::NewAllele() {
    if (_p != NULL) {
        void* mem = _p;
        _p = _p->pNext();
        return mem;
    } else {
        return ::new char [sizeof (Allele)];
    }
}

void AlleleFreeList::Recycle(void* mem) {
    Allele* allele = static_cast<Allele*> (mem);
    allele->_pNext = _p;
    _p = allele;
    ++_allocs;
}

AlleleFreeList::~AlleleFreeList() {
    Purge();
}

void AlleleFreeList::Purge() {
    while (_p != NULL) {
        char * mem = reinterpret_cast<char *> (_p);
        _p = _p->pNext();
        ::delete [] mem;
    }
}

bool sufficientAlternateObservations(map<string, vector<Allele*> >& sampleGroups, int mincount, float minfraction) {

    for (map<string, vector<Allele*> >::iterator sample = sampleGroups.begin();
            sample != sampleGroups.end(); ++sample) {

        vector<Allele*>& observedAlleles = sample->second;
        int alternateCount = 0;
        for (vector<Allele*>::iterator a = observedAlleles.begin(); a != observedAlleles.end(); ++a) {
            if ((*a)->type != ALLELE_REFERENCE)
                ++alternateCount;
        }
        if (alternateCount >= mincount && (float) alternateCount / (float) observedAlleles.size() >= minfraction)
            return true;
    
    }
    return false;

}

vector<bool> allowedAlleleTypesVector(vector<AlleleType>& allowedEnumeratedTypes) {
    vector<bool> allowedTypes;// (numberOfPossibleAlleleTypes, false);
    for (int i = 0; i < numberOfPossibleAlleleTypes; ++i) {
        bool allowed = false;
        for (vector<AlleleType>::iterator t = allowedEnumeratedTypes.begin(); t != allowedEnumeratedTypes.end(); ++t) {
            if (i == *t) {
                allowed = true;
                break;
            }
        }
        allowedTypes.push_back(allowed);
    }
    return allowedTypes;
}

void filterAlleles(list<Allele*>& alleles, vector<bool>& allowedTypes) {

    for (list<Allele*>::iterator allele = alleles.begin(); allele != alleles.end(); ++allele) {
        bool allowed = false;
        if (!allowedTypes.at((*allele)->type))
            *allele = NULL;
    }
    alleles.erase(remove(alleles.begin(), alleles.end(), (Allele*)NULL), alleles.end());

}

// removes alleles which are indelmasked at position
void removeIndelMaskedAlleles(list<Allele*>& alleles, long unsigned int position) {

    for (list<Allele*>::iterator allele = alleles.begin(); allele != alleles.end(); ++allele) {
        //cerr << *allele << " " << (*allele)->indelMask.size() << " " << (*allele)->referenceOffset() << endl;
        if ((*allele)->masked()) {
            *allele = NULL;
        }
    }
    alleles.erase(remove(alleles.begin(), alleles.end(), (Allele*)NULL), alleles.end());

}
