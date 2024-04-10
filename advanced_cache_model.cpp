#include<iostream>
#include<algorithm>
#include<list>
#include<set>
#include<vector>
#include<unordered_map>

using namespace std;

template<typename Address, typename Value>
class CacheModel
{
private:
    struct CacheBlock
    {
        Address tag;
        vector<Value> block;
        CacheBlock* prev;
        CacheBlock* next;
        CacheBlock(Address t, int blockSize) : tag(t), block(blockSize), prev(nullptr), next(nullptr) {}
    };

    int capacity;
    int associativity;
    int blockSize;
    int cacheHits;
    int cacheMisses;
    vector<CacheBlock*> set_heads;
    vector<CacheBlock*> set_tails;
    vector<unordered_map<Address, CacheBlock*> > set_maps;

    int getSetIndex(Address address) {
        return address % set_heads.size();
    }

    Address getTag(Address address) {
        return address / set_heads.size();
    }

public:
    CacheModel(int capacity, int associativity, int blockSize)
        : capacity(capacity), associativity(associativity), blockSize(blockSize), cacheHits(0), cacheMisses(0) {
        int numSets = capacity / (associativity * blockSize);
        set_heads.resize(numSets, nullptr);
        set_tails.resize(numSets, nullptr);
        set_maps.resize(numSets);
    }

    vector<Value> getBlock(Address address) {
        int index = getSetIndex(address);
        Address tag = getTag(address);
        unordered_map<Address, CacheBlock*>& cacheMap = set_maps[index];
        if (cacheMap.find(tag) != cacheMap.end()) {
            CacheBlock* line = cacheMap[tag];
            cacheHits++;
            if (line != set_heads[index]) {
                if (line->next) line->next->prev = line->prev;
                if (line->prev) line->prev->next = line->next;
                if (line == set_tails[index]) set_tails[index] = line->prev;
                line->next = set_heads[index];
                if (set_heads[index]) set_heads[index]->prev = line;
                set_heads[index] = line;
            }
            return line->block;
        } else {
            cacheMisses++;
            return vector<Value>();
        }
    }

    void writeBlock(Address address, vector<Value>& values) {
        int index = getSetIndex(address);
        Address tag = getTag(address);
        unordered_map<Address, CacheBlock*>& cacheMap = set_maps[index];
        if (cacheMap.find(tag) != cacheMap.end()) {
            CacheBlock* line = cacheMap[tag];
            line->block = values;
            if (line != set_heads[index]) {
                if (line->next) line->next->prev = line->prev;
                if (line->prev) line->prev->next = line->next;
                if (line == set_tails[index]) set_tails[index] = line->prev;
                line->next = set_heads[index];
                if (set_heads[index]) set_heads[index]->prev = line;
                set_heads[index] = line;
            }
        } 
        
        else {
            cacheMisses++;
            CacheBlock* newLine = new CacheBlock(tag, blockSize);
            newLine->block = values;
            if (!set_heads[index]) {
                set_heads[index] = newLine;
                set_tails[index] = newLine;
            } else {
                newLine->next = set_heads[index];
                set_heads[index]->prev = newLine;
                set_heads[index] = newLine;
            }
            cacheMap[tag] = newLine;
            if (cacheMap.size() > (size_t)associativity) {
                CacheBlock* lastLine = set_tails[index];
                set_tails[index] = lastLine->prev;
                if (set_tails[index]) set_tails[index]->next = nullptr;
                cacheMap.erase(lastLine->tag);
                delete lastLine;
            }
        }
    }

    ~CacheModel() {
        for (size_t i = 0; i < set_heads.size(); ++i) {
            CacheBlock* head = set_heads[i];
            while (head) {
                CacheBlock* next = head->next;
                delete head;
                head = next;
            }
        }
    }
};

int main() {
    CacheModel<int, char> cache(16, 2, 4);

    vector<char> block1;
    block1.push_back('A');
    block1.push_back('B');
    block1.push_back('C');
    block1.push_back('D');

    cache.writeBlock(5, block1);

    vector<char> retrievedBlock1 = cache.getBlock(5);
    for (size_t i = 0; i < retrievedBlock1.size(); ++i) {
        cout << retrievedBlock1[i] << " ";
    }
    cout << endl;

    return 0;
}
