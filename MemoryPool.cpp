#include <vector>
#include <cstdint>


using namespace std;
using uint_32 = uint32_t;
using uint_64 = uint64_t;

const uint_32 NULL_IDX = 0xFFFFFFFF;


template <typename T>
struct MemoryPool{

    vector<T> pool;


    void reserve(size_t size){

        pool.reserve(size);
    }

    // size_t allocate(uint_32 tokenId){

    //     size_t newIdx = static_cast<size_t>(pool.size());
    //     pool.push_back(T{tokenId,0, NULL_IDX, NULL_IDX});
    //     return newIdx;

    // }

    size_t allocate(T obj){
        size_t newIdx = static_cast<size_t>(pool.size());

        pool.push_back(obj);

        return newIdx;
    }

    T& get(size_t index){

        return pool[index];

    }

    void clear() {
        pool.clear();
    }
};