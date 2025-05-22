#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

#define DRAM_SIZE (64*1024*1024)
#define CACHE_SIZE (64*1024)

enum cacheResType {MISS=0, HIT=1};

unsigned int m_w = 0xABABAB55;
unsigned int m_z = 0x05080902;

unsigned int rand_() {
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

unsigned int memGen1() {
    static unsigned int addr = 0;
    return (addr++) % (DRAM_SIZE);
}

unsigned int memGen2() {
    static unsigned int addr = 0;
    return rand_() % (24 * 1024);
}

unsigned int memGen3() {
    return rand_() % (DRAM_SIZE);
}

unsigned int memGen4() {
    static unsigned int addr = 0;
    return (addr++) % (4 * 1024);
}

unsigned int memGen5() {
    static unsigned int addr = 0;
    return (addr++) % (1024 * 64);
}

unsigned int memGen6() {
    static unsigned int addr = 0;
    return (addr += 32) % (64 * 4 * 1024);
}

struct CacheLine {
    bool valid;
    bool dirty;
    unsigned int tag;
    unsigned int lastUsed;
    vector<unsigned char> data;

    CacheLine(int lineSize) : valid(false), dirty(false), tag(0), lastUsed(0) {
        data.resize(lineSize);
    }
};

struct CacheSet {
    vector<CacheLine> lines;

    CacheSet(int ways, int lineSize) {
        for (int i = 0; i < ways; i++) {
            lines.push_back(CacheLine(lineSize));
        }
    }
};

class Cache {
private:
    int cacheSize;
    int lineSize;
    int numWays;
    int numSets;

    int indexBits;
    int offsetBits;

    vector<CacheSet> sets;
    unsigned int accessCount;
    unsigned int hitCount;

public:
    Cache(int size, int line, int ways) {
        cacheSize = size;
        lineSize = line;
        numWays = ways;
        numSets = cacheSize / (lineSize * numWays);

        offsetBits = log2(lineSize);
        indexBits = log2(numSets);

        for (int i = 0; i < numSets; i++) {
            sets.push_back(CacheSet(numWays, lineSize));
        }

        accessCount = 0;
        hitCount = 0;
    }

    void parseAddress(unsigned int addr, unsigned int &tag, unsigned int &index, unsigned int &offset) {
        offset = addr & ((1 << offsetBits) - 1);
        index = (addr >> offsetBits) & ((1 << indexBits) - 1);
        tag = addr >> (offsetBits + indexBits);
    }

    cacheResType access(unsigned int addr) {
        accessCount++;

        unsigned int tag, index, offset;
        parseAddress(addr, tag, index, offset);

        for (int i = 0; i < numWays; i++) {
            if (sets[index].lines[i].valid && sets[index].lines[i].tag == tag) {
                hitCount++;
                sets[index].lines[i].lastUsed = accessCount;
                return HIT;
            }
        }

        int replaceIndex = 0;
        unsigned int lruValue = sets[index].lines[0].lastUsed;

        for (int i = 1; i < numWays; i++) {
            if (!sets[index].lines[i].valid) {
                replaceIndex = i;
                break;
            }
            if (sets[index].lines[i].lastUsed < lruValue) {
                lruValue = sets[index].lines[i].lastUsed;
                replaceIndex = i;
            }
        }

        sets[index].lines[replaceIndex].valid = true;
        sets[index].lines[replaceIndex].tag = tag;
        sets[index].lines[replaceIndex].lastUsed = accessCount;

        return MISS;
    }

    double getHitRatio() {
        return (double)hitCount / accessCount;
    }

    void resetStats() {
        accessCount = 0;
        hitCount = 0;
    }

    void printConfig() {
        cout << "Cache Size: " << cacheSize << " bytes" << endl;
        cout << "Line Size: " << lineSize << " bytes" << endl;
        cout << "Associativity: " << numWays << "-way" << endl;
        cout << "Number of Sets: " << numSets << endl;
        cout << "Index Bits: " << indexBits << endl;
        cout << "Offset Bits: " << offsetBits << endl;
    }
};

typedef unsigned int (*MemGenFunc)();

void experimentVaryLineSize(MemGenFunc memGen, int ways) {
    const int NUM_ITERATIONS = 1000000;
    const int lineSizes[] = {16, 32, 64, 128};

    cout << "\nExperiment: Fixed " << ways << " ways, varying line size" << endl;
    cout << "Line Size\tHit Ratio" << endl;

    for (int lineSize : lineSizes) {
        Cache cache(CACHE_SIZE, lineSize, ways);

        for (int i = 0; i < NUM_ITERATIONS; i++) {
            unsigned int addr = memGen();
            cache.access(addr);
        }

        cout << lineSize << "\t\t" << fixed << setprecision(6) << cache.getHitRatio() << endl;
    }
}

void experimentVaryWays(MemGenFunc memGen, int lineSize) {
    const int NUM_ITERATIONS = 1000000;
    const int waysOptions[] = {1, 2, 4, 8, 16};

    cout << "\nExperiment: Fixed line size " << lineSize << " bytes, varying ways" << endl;
    cout << "Ways\tHit Ratio" << endl;

    for (int ways : waysOptions) {
        Cache cache(CACHE_SIZE, lineSize, ways);

        for (int i = 0; i < NUM_ITERATIONS; i++) {
            unsigned int addr = memGen();
            cache.access(addr);
        }

        cout << ways << "\t" << fixed << setprecision(6) << cache.getHitRatio() << endl;
    }
}

void validateCache() {
    cout << "Validating cache functionality..." << endl;

    Cache testCache(1024, 64, 2);

    unsigned int addr1 = 0;
    unsigned int addr2 = 64;
    unsigned int addr3 = 128;

    cacheResType res1 = testCache.access(addr1);
    cout << "First access to addr " << addr1 << ": " << (res1 == HIT ? "HIT" : "MISS") << endl;

    cacheResType res2 = testCache.access(addr1);
    cout << "Second access to addr " << addr1 << ": " << (res2 == HIT ? "HIT" : "MISS") << endl;

    cacheResType res3 = testCache.access(addr2);
    cout << "First access to addr " << addr2 << ": " << (res3 == HIT ? "HIT" : "MISS") << endl;

    cacheResType res4 = testCache.access(addr3);
    cout << "First access to addr " << addr3 << ": " << (res4 == HIT ? "HIT" : "MISS") << endl;

    cacheResType res5 = testCache.access(addr1);
    cout << "Third access to addr " << addr1 << ": " << (res5 == HIT ? "HIT" : "MISS") << endl;

    cout << "Validation complete. Hit ratio: " << testCache.getHitRatio() << endl;
}

int main() {
    cout << "Cache Simulator Project" << endl;
    cout << "=======================" << endl;

    validateCache();

    MemGenFunc memGens[] = {memGen1, memGen2, memGen3, memGen4, memGen5, memGen6};
    const char* genNames[] = {"memGen1", "memGen2", "memGen3", "memGen4", "memGen5", "memGen6"};

    for (int i = 0; i < 6; i++) {
        cout << "\n\nRunning experiments with " << genNames[i] << endl;
        cout << "----------------------------------------" << endl;

        experimentVaryLineSize(memGens[i], 4);
        experimentVaryWays(memGens[i], 64);
    }

    return 0;
}