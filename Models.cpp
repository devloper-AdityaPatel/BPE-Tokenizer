#include <cstdint>
#include <vector>
#include <string>
using namespace std;
using uint_32 = uint32_t;
using uint_64 = uint64_t;



struct TokenNode
{
    uint_32 TokenId;
    uint_32 Count;
    uint_32 PrevNodeIndex;
    uint_32 NextNodeIndex;
};


struct StringMetaData{

    uint_32 count;
    uint_32 DllHead;
};

struct TokenPairData{

    uint_32 count;
    vector<pair<uint_32,uint_32>> TokensIndex; //{occurrence index in memory pool , local count of node}
};


struct TokenTreeNode{
    uint_32 TokenID;
    bool IsEndOfWord;
    string NodeString;
    uint_32 ChildIdx[256];
};
