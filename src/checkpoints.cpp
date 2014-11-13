// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (  0, uint256("0x00000496d303ae6e6ed9d474639f18b3fdf70166c8d89d1267bbf5fd640e1690"))
        (  1, uint256("0x000002bdf3c3a74682b7cb835e9a431832728ff056d2a859a1e191f3ff71c378"))
        (  50, uint256("0x00000b4d4f7dec7d1fcfa143cdbdeb9397b55d989d5da8a148b43fee07ad63d6"))
        (  100, uint256("0x000003d5654690e6ac39e6d6d3713fccdeb64a8ccb113c1434efdcaebb64f43e"))
        (  1611, uint256("0x0000000007c94b680ac77122eb882a8b45cd0b3d167e24112096c7b01e24bfb3"))
        (  1612, uint256("0x00000000047ec7d9318ecf5c128c15141a76105339098da97e614364fc2a09a9"))
        (  1999, uint256("0x0000000015c1f6fc25899bd13c8111a5255748622d46581c21e50dc2051a23a1"))
        (  2000, uint256("0x0000000000bbd180a7818896df255a09955393fe5432428e17b1cbae572e2a13"))
        (  3010, uint256("0x000000000e099f930eb1da8c7925112f7af6221bd5912dda4e2358eda9ff9964"))
        (  4300, uint256("0x000000016bf6bb1f040cc50578ae2897bd3754a7ec37120e8fe2fcb4dd9c7e6c"))
        (  4512, uint256("0x000000007ad8789e12c23e6e8482c672dacb1d3f2c120fea6d2047dd1055d579"))
        (  11177, uint256("0x000000004327606dee194e90cb1e5fabe9d4e9ce798e50a9c303a75b186cba2a"))
        (  12485, uint256("0x00000000fc6146156e1edcc05017231e0c9262f1ab4a661b669381a54e273d55"))
        (  22650, uint256("0x00000000601668eded5ba43578abff2c166481bff9449abffa339ce9ac8c63e5"))
        (  26000, uint256("0x00000000015da7acf3afcf206db6ad0f7fc1928ff3505f84a96195e6fff2ffec"))
        (  27975, uint256("0x0000000004499157bc9577b8b8902fbeaae418ed805ccd58660a587dbe3215487"))
        (  45001, uint256("0x00000001a1ee4e1dafe94079d0a4fde98d6314d0bb1fad05e8a49b62a2004cde"))
        (  50000, uint256("0x000000005deea64c2353af5c6c75b37033e4ab8da628b24360643be32f51d8ae"))
        (  75000, uint256("0x00000000bcc6345cc5af3e011c86e7ae53825449e19337f0d54aeef2a07ac65c"))

        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1411478807, // * UNIX timestamp of last checkpoint block
        93239,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        480     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = 
        boost::assign::map_list_of
        (   0, uint256("0x"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1396890000,
        3000,
        30
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    uint256 GetLastAvailableCheckpoint() {
        const MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints) {
            const uint256& hash = i.second;
            if(mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
                return(hash);
        }
        return(hashGenesisBlock);
   }

    uint256 GetLatestHardenedCheckpoint()
    {
        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
        return (checkpoints.rbegin()->second);
    }
}
