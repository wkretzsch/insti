#include "kNN.hpp"
#include <utility>

using namespace std;

// initialization for kNN clustering
void KNN::init(const vector< uint64_t > & vuHaplotypes,
                  unsigned uNumWordsPerHap, unsigned uNumSites,
                  double dFreqCutoff)
{

    cerr << "[KNN]: Initialization start\n";
    assert(m_bInitialized == false);
    assert(uNumWordsPerHap > 0);

    assert(uNumSites > 0);
    m_uNumSites = uNumSites;

    m_dFreqCutoff = dFreqCutoff;

    // dereferencing the vector pointer
    assert(vuHaplotypes.size() % uNumWordsPerHap == 0);
    m_uNumHaps = vuHaplotypes.size() / uNumWordsPerHap;
    assert(m_uNumHaps % 2 == 0);
    m_uNumWordsPerHap = uNumWordsPerHap;

    if (UsingScaffold())
        CalculateVarAfs(vuHaplotypes);

    CutDownHaps();
    ClusterHaps(vuHaplotypes);

    m_bInitialized = true;
    cerr << "[KNN]: Initialization complete\n";

}

unsigned KNN::SampleHap(unsigned uInd, gsl_rng *rng)
{

    // this should be out of bounds if left unchanged
    unsigned uPropHap = m_uNumHaps;

    if (UsingScaffold())
        uPropHap = m_vvuNeighbors[uInd][gsl_rng_uniform_int(rng,
                                        m_vvuNeighbors[uInd].size())];
    else {
        cout << "kNN without a scaffold is not implemented yet" << endl;
        exit(4);
    }

    // make sure the return value is sensible
    assert(floor(uPropHap / 2) != uInd);
    return uPropHap;

}

// returns the neighbors of haplotype uHapNum
struct dist {
    unsigned idx;
    unsigned distance;
};

struct by_dist {
    bool operator()(dist const &a, dist const &b)
    {
        return a.distance < b.distance;
    }
};

void KNN::ClusterHaps(const vector< uint64_t > & vuHaplotypes)
{

    if (UsingScaffold()) {
        m_vvuNeighbors.clear();
        unsigned uNumCommonSites = m_vuCommonSiteNums.size();

        for (unsigned uSampNum = 0; uSampNum < m_uNumHaps / 2; uSampNum ++) {

            cerr << "[KNN::ClusterHaps]: Finding " << m_uNumClusters <<
                 " nearest neighbors for sample " << uSampNum << "/" << m_uNumHaps / 2 - 1 <<
                 "\r";

            // assign only the common sites of each hap to new haps
            Haplotype hHapA(uNumCommonSites);
            Haplotype hHapB(uNumCommonSites);
            AssignHap(hHapA, vuHaplotypes.data() + uSampNum * 2 * m_uNumWordsPerHap);
            AssignHap(hHapB, vuHaplotypes.data() + (uSampNum * 2 + 1) * m_uNumWordsPerHap);

            // calculate neighbors of this sample
            // hap number then distance
            vector < dist > vpDists;

            for (unsigned uHapNum = 0; uHapNum < m_uNumHaps; uHapNum ++) {
                if (floor(uHapNum / 2) == uSampNum) continue;

                Haplotype hCompHap(uNumCommonSites);
                AssignHap(hCompHap, vuHaplotypes.data() + uHapNum * m_uNumWordsPerHap);
                dist dTemp;
                dTemp.idx = uHapNum;
                dTemp.distance =  min(hHapA.HammingDist(hCompHap),
                                      hHapB.HammingDist(hCompHap));
                vpDists.push_back(dTemp);
            }

            //sort the haplotypes by distance
            sort(vpDists.begin(), vpDists.end(), by_dist());

            // keep only the k samples with smallest distance
            vector< unsigned > vuDists(m_uNumClusters);

            for (unsigned idx = 0; idx != m_uNumClusters; ++idx)
                vuDists[idx] = vpDists[idx].idx;

            // put the closest k in m_vvuNeighbors
            m_vvuNeighbors.push_back(vuDists);
        }


    } else {
        cout << "kNN without a scaffold is not implemented yet" << endl;
        exit(4);
    }

    cerr << "[KNN::ClusterHaps]: Finding " << m_uNumClusters <<
         " nearest neighbors for sample " << m_uNumHaps / 2 - 1 << "/" << m_uNumHaps / 2
         - 1 << "\n";

}

void KNN::AssignHap(Haplotype &hHap, const uint64_t *oa)
{
    unsigned uSiteNum = 0;

    for (auto iHapNum : m_vuCommonSiteNums) {
        hHap.Set(uSiteNum, hHap.TestSite(iHapNum, oa));
        uSiteNum++;
    }
}


void KNN::CutDownHaps()
{

    for (unsigned uPosNum = 0; uPosNum < m_uNumSites; uPosNum ++) {
        if (m_vdVarAfs[uPosNum] >= m_dFreqCutoff)
            m_vuCommonSiteNums.push_back(uPosNum);
    }
}

/*
  calculate the variant allele frequency from scaffold for each position and store it in m_vdVarAfs
*/
void KNN::CalculateVarAfs(const vector < uint64_t > & vuScaffoldHaps)
{

    m_vdVarAfs.resize(m_uNumSites);
    Haplotype hTestHap(m_uNumSites);

    //    cerr << "num words per hap: " << m_uNumWordsPerHap << endl;
    for (unsigned uPosNum = 0; uPosNum < m_uNumSites; uPosNum ++) {

        unsigned uAltAllNum = 0;

        for (unsigned uHapNum = 0; uHapNum < m_uNumHaps; uHapNum ++) {
            if (hTestHap.TestSite(uPosNum,
                                  vuScaffoldHaps.data() + uHapNum * m_uNumWordsPerHap))
                uAltAllNum ++;
        }

        //        cerr << "alternate allele count: " << uAltAllNum << endl;

        m_vdVarAfs[uPosNum] = static_cast<double>(uAltAllNum) / m_uNumHaps;
    }

    /*    for(unsigned i = 0; i < m_uNumHaps; i++){
            cerr << "hap num: " << i << ": ";
            for (unsigned j = 0; j < m_uNumSites; j ++){
                cerr << hTestHap.TestSite(j, vuScaffoldHaps.data() + i * m_uNumWordsPerHap);
            }
            cerr << endl;
        }
    */
}




