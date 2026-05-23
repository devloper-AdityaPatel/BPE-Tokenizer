#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

using namespace std;

class BufferSerializer {

public:

    static vector<uint8_t> SerializeToBuffer(uint_32 nextTokenID,const vector<string>& tokenIdBytes,MemoryPool<TokenTreeNode>& tokenRadixTreePool
    ) {

        vector<uint8_t> buffer;

        auto writeBytes = [&](const void* data, size_t size) {

            const uint8_t* ptr = (const uint8_t*)data;

            buffer.insert(buffer.end(), ptr, ptr + size);
        };

        writeBytes(&nextTokenID, sizeof(nextTokenID));

        uint_32 vocabSize = (uint_32)tokenIdBytes.size();

        writeBytes(&vocabSize, sizeof(vocabSize));

        for (uint_32 i = 0; i < vocabSize; i++) {

            uint_32 strLength = (uint_32)tokenIdBytes[i].length();

            writeBytes(&strLength, sizeof(strLength));

            if (strLength > 0) {
                writeBytes(tokenIdBytes[i].data(), strLength);
            }
        }


        uint_32 treeSize = (uint_32)tokenRadixTreePool.pool.size();

        writeBytes(&treeSize, sizeof(treeSize));

        for (uint_32 i = 0; i < treeSize; i++) {

            TokenTreeNode& node = tokenRadixTreePool.get(i);

            writeBytes(&node.TokenID, sizeof(node.TokenID));

            writeBytes(&node.IsEndOfWord, sizeof(node.IsEndOfWord));

            uint_32 nodeStringLength = (uint_32)node.NodeString.length();

            writeBytes(&nodeStringLength, sizeof(nodeStringLength));

            if (nodeStringLength > 0) {
                writeBytes(node.NodeString.data(), nodeStringLength);
            }

            writeBytes(node.ChildIdx, sizeof(node.ChildIdx));
        }

        return buffer;
    }



    static bool DeserializeFromBuffer(const vector<uint8_t>& buffer,uint_32& nextTokenID,vector<string>& tokenIdBytes,MemoryPool<TokenTreeNode>& tokenRadixTreePool
    ) {

        size_t offset = 0;

        auto readBytes = [&](void* dest, size_t size) -> bool {

            if (offset + size > buffer.size()) {
                return false;
            }

            memcpy(dest, buffer.data() + offset, size);

            offset += size;

            return true;
        };

        if (!readBytes(&nextTokenID, sizeof(nextTokenID))) {
            return false;
        }

        uint_32 vocabSize;

        if (!readBytes(&vocabSize, sizeof(vocabSize))) {
            return false;
        }

        tokenIdBytes.resize(vocabSize);

        for (uint_32 i = 0; i < vocabSize; i++) {

            uint_32 strLength;

            if (!readBytes(&strLength, sizeof(strLength))) {
                return false;
            }

            if (strLength > 0) {

                tokenIdBytes[i].resize(strLength);

                if (!readBytes(&tokenIdBytes[i][0], strLength)) {
                    return false;
                }
            }
        }




        uint_32 treeSize;

        if (!readBytes(&treeSize, sizeof(treeSize))) {
            return false;
        }

        tokenRadixTreePool.clear();

        tokenRadixTreePool.reserve(treeSize);

        for (uint_32 i = 0; i < treeSize; i++) {

            TokenTreeNode node;

            if (!readBytes(&node.TokenID, sizeof(node.TokenID))) {
                return false;
            }

            if (!readBytes(&node.IsEndOfWord, sizeof(node.IsEndOfWord))) {
                return false;
            }


            uint_32 nodeStringLength;

            if (!readBytes(&nodeStringLength, sizeof(nodeStringLength))) {
                return false;
            }

            if (nodeStringLength > 0) {

                node.NodeString.resize(nodeStringLength);

                if (!readBytes(&node.NodeString[0], nodeStringLength)) {
                    return false;
                }
            }


            if (!readBytes(node.ChildIdx, sizeof(node.ChildIdx))) {
                return false;
            }

            tokenRadixTreePool.allocate(node);
        }

        return true;
    }
};