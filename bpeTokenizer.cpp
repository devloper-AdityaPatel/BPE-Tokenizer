#include "Models.cpp"
#include "MemoryPool.cpp"
#include "Serializer.cpp"
#include "saveutility.cpp"
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include <iostream>
using namespace std;

using uint_32 = uint32_t;
using uint_64 = uint64_t;
using uint_8 = uint8_t;


class BytePairTokenizer
{

    const uint_32 VocabSize;
    const uint_32 MemoryPoolSize;
    uint_32 NextTokenID = 256;
    vector<pair<uint_32, uint_32>> ChildTokenIdArray;
    vector<uint_32> TokenIdToRadixIndexMap;
    vector<string> TokenIdBytes;

private:
    uint_32 getTokenDll(string &text, MemoryPool<TokenNode> &tokenListPool)
    {

        uint_32 Head = NULL_IDX;
        uint_32 iter = Head;
        for (unsigned char c : text)
        {

            if (Head == NULL_IDX)
            {

                Head = iter = tokenListPool.allocate(TokenNode{static_cast<uint_32>(c), 0, NULL_IDX, NULL_IDX});
            }
            else
            {

                uint_32 NodeIndex = tokenListPool.allocate(TokenNode{static_cast<uint_32>(c), 0, NULL_IDX, NULL_IDX});

                TokenNode &prevNode = tokenListPool.get(iter);
                TokenNode &currNode = tokenListPool.get(NodeIndex);
                prevNode.NextNodeIndex = NodeIndex;
                currNode.PrevNodeIndex = iter;

                iter = NodeIndex;
            }
        }

        return Head;
    }

    uint_64 GetTokenPairHash(uint_32 leftTokenId, uint_32 rightTokenId)
    {

        return (uint_64(leftTokenId) << 32) | (uint_64(rightTokenId));
    }

    pair<uint_32, uint_32> GetTokenPair(uint_64 hash)
    {

        uint_32 leftTokenId = static_cast<uint_32>(hash >> 32);

        uint_32 rightTokenId = static_cast<uint_32>(hash);

        return {leftTokenId, rightTokenId};
    }

    TokenTreeNode MakeTokenTreeNode()
    {

        TokenTreeNode newNode;
        newNode.TokenID = NULL_IDX;
        newNode.IsEndOfWord = false;
        newNode.NodeString = "";

        for (int i = 0; i < 256; i++)
        {
            newNode.ChildIdx[i] = NULL_IDX;
        }

        return newNode;
    }

    void RadixInsert(
        uint_32 CurrentNodeIndex,
        string &suffix,
        uint_32 &suffixIndex,
        uint_32 tokenId,
        MemoryPool<TokenTreeNode> &tokenRadixTreePool)
    {

        TokenTreeNode &CurrentNode =
            tokenRadixTreePool.get(CurrentNodeIndex);

        int i = 0;

        //-------------------------------------------------------
        // Match current node string
        //-------------------------------------------------------

        for (
            i = 0;
            i < CurrentNode.NodeString.length() &&
            suffixIndex < suffix.length();
            i++, suffixIndex++)
        {

            if (
                CurrentNode.NodeString[i] !=
                suffix[suffixIndex])
            {
                break;
            }
        }

        //-------------------------------------------------------
        // FULL NODE MATCH
        //-------------------------------------------------------

        if (i == CurrentNode.NodeString.length())
        {

            //---------------------------------------------------
            // Entire suffix consumed
            //---------------------------------------------------

            if (suffixIndex == suffix.length())
            {

                CurrentNode.IsEndOfWord = true;
                CurrentNode.TokenID = tokenId;
                TokenIdToRadixIndexMap[tokenId] = CurrentNodeIndex;

                return;
            }

            //---------------------------------------------------
            // Need to go deeper
            //---------------------------------------------------

            unsigned char nextChar =
                (unsigned char)suffix[suffixIndex];

            //---------------------------------------------------
            // Child does not exist
            //---------------------------------------------------

            if (CurrentNode.ChildIdx[nextChar] == NULL_IDX)
            {

                uint_32 NewNodeIndex =
                    tokenRadixTreePool.allocate(MakeTokenTreeNode());

                TokenTreeNode &NewNode =
                    tokenRadixTreePool.get(NewNodeIndex);

                NewNode.NodeString =
                    suffix.substr(suffixIndex);

                NewNode.IsEndOfWord = true;
                NewNode.TokenID = tokenId;
                TokenIdToRadixIndexMap[tokenId] = NewNodeIndex;
                CurrentNode.ChildIdx[nextChar] = NewNodeIndex;

                return;
            }

            //---------------------------------------------------
            // Recursive insert
            //---------------------------------------------------

            RadixInsert(
                CurrentNode.ChildIdx[nextChar],
                suffix,
                suffixIndex,
                tokenId,
                tokenRadixTreePool);

            return;
        }

        //-------------------------------------------------------
        // PARTIAL MATCH -> SPLIT NODE
        //-------------------------------------------------------

        uint_32 SplitNodeIndex =
            tokenRadixTreePool.allocate(MakeTokenTreeNode());

        TokenTreeNode &SplitNode =
            tokenRadixTreePool.get(SplitNodeIndex);

        //-------------------------------------------------------
        // Move old children
        //-------------------------------------------------------

        for (int j = 0; j < 256; j++)
        {

            SplitNode.ChildIdx[j] =
                CurrentNode.ChildIdx[j];

            CurrentNode.ChildIdx[j] =
                NULL_IDX;
        }

        //-------------------------------------------------------
        // Old suffix becomes split node
        //-------------------------------------------------------

        SplitNode.NodeString =
            CurrentNode.NodeString.substr(i);

        SplitNode.IsEndOfWord =
            CurrentNode.IsEndOfWord;

        SplitNode.TokenID =
            CurrentNode.TokenID;
        TokenIdToRadixIndexMap[SplitNode.TokenID] = SplitNodeIndex;
        //-------------------------------------------------------
        // Shrink current node
        //-------------------------------------------------------

        CurrentNode.NodeString =
            CurrentNode.NodeString.substr(0, i);

        CurrentNode.IsEndOfWord = false;

        //-------------------------------------------------------
        // Attach split node
        //-------------------------------------------------------

        unsigned char oldChar =
            (unsigned char)SplitNode.NodeString[0];

        CurrentNode.ChildIdx[oldChar] =
            SplitNodeIndex;

        //-------------------------------------------------------
        // Incoming string fully consumed
        //-------------------------------------------------------

        if (suffixIndex == suffix.length())
        {

            CurrentNode.IsEndOfWord = true;
            CurrentNode.TokenID = tokenId;
            TokenIdToRadixIndexMap[tokenId] = CurrentNodeIndex;
            return;
        }

        //-------------------------------------------------------
        // Create new branch
        //-------------------------------------------------------

        uint_32 NewNodeIndex =
            tokenRadixTreePool.allocate(MakeTokenTreeNode());

        TokenTreeNode &NewNode =
            tokenRadixTreePool.get(NewNodeIndex);

        NewNode.NodeString =
            suffix.substr(suffixIndex);

        NewNode.IsEndOfWord = true;
        NewNode.TokenID = tokenId;
        TokenIdToRadixIndexMap[tokenId] = NewNodeIndex;
        unsigned char newChar =
            (unsigned char)suffix[suffixIndex];

        CurrentNode.ChildIdx[newChar] =
            NewNodeIndex;
    }
    void InsertIntoTokenTree(uint_32 parentNodeIndex, string &suffix, uint_32 tokenId, MemoryPool<TokenTreeNode> &tokenRadixTreePool)
    {

        if (suffix.length() <= 0)
            return;

        TokenTreeNode &parentNode = tokenRadixTreePool.get(parentNodeIndex);

        uint_8 nextChildIndex = static_cast<uint_8>(suffix[0]);

        if (parentNode.ChildIdx[nextChildIndex] == NULL_IDX)
        {

            uint_32 newNodeIndex = tokenRadixTreePool.allocate(MakeTokenTreeNode());

            TokenTreeNode &newNode = tokenRadixTreePool.get(newNodeIndex);

            newNode.TokenID = tokenId;
            newNode.NodeString = suffix;
            newNode.IsEndOfWord = true;
            TokenIdToRadixIndexMap[tokenId] = newNodeIndex;

            parentNode.ChildIdx[nextChildIndex] = newNodeIndex;
            return;
        }

        uint_32 suffixIndex = 0;
        RadixInsert(parentNode.ChildIdx[nextChildIndex],suffix,suffixIndex,tokenId,tokenRadixTreePool);
    }


    uint_32 GetTokenFromRadixTree(uint_32 CurrentNodeIndex, string & suffix , int suffixIndex,MemoryPool<TokenTreeNode> &tokenRadixTreePool){



        TokenTreeNode & curretNode = tokenRadixTreePool.get(CurrentNodeIndex);


        int i = 0 ;

        for(i = 0 ; i < curretNode.NodeString.length() && suffixIndex < suffix.length(); i++ , suffixIndex++){

            if(curretNode.NodeString[i] != suffix[suffixIndex]) return NULL_IDX;
        }

        if(suffixIndex == suffix.length()){

            if(i == curretNode.NodeString.length()){

                return curretNode.TokenID;
            }
            return NULL_IDX;
        }

        if(i < curretNode.NodeString.length()){

            return NULL_IDX;
        }

        unsigned char nextChar = (unsigned char)suffix[suffixIndex];

        if(curretNode.ChildIdx[nextChar] == NULL_IDX) return NULL_IDX;

        return GetTokenFromRadixTree(curretNode.ChildIdx[nextChar],suffix,suffixIndex,tokenRadixTreePool);
    }   

public:
    BytePairTokenizer(uint_32 VocabSize, uint_32 MemoryPoolSize) : VocabSize(VocabSize), MemoryPoolSize(MemoryPoolSize)
    {
        ChildTokenIdArray.resize(VocabSize);
    }

    bool train(vector<string> &textCorpus)
    {

        MemoryPool<TokenNode> tokenListPool;
        MemoryPool<TokenTreeNode> tokenRadixTreePool;

        tokenListPool.reserve(MemoryPoolSize);
        tokenRadixTreePool.reserve(4 * VocabSize);
        TokenIdToRadixIndexMap.resize(VocabSize);
        TokenIdBytes.resize(VocabSize);
        unordered_map<string, StringMetaData> TextMap;

        unordered_map<uint_64, TokenPairData> TokenMap;

        priority_queue<pair<uint_32, uint_64>> TokenPQ;

        uint_32 TokenTreeRootIndex = tokenRadixTreePool.allocate(MakeTokenTreeNode());

        for (int TokenId = 0; TokenId < 256; TokenId++)
        {
            string basetoken(1, static_cast<char>(TokenId));
            TokenIdBytes[TokenId] = basetoken;
            InsertIntoTokenTree(TokenTreeRootIndex,basetoken,TokenId,tokenRadixTreePool);
        }
        for (string &text : textCorpus)
        {

            if (text == "")
                continue;

            auto textNode = TextMap.find(text);

            if (textNode != TextMap.end())
            {
                textNode->second.count++;
            }
            else
            {
                uint_32 head = getTokenDll(text, tokenListPool);
                StringMetaData metaData{1, head};
                TextMap[text] = metaData;
            }
        }

        for (auto &[text, metadata] : TextMap)
        {

            uint_32 iter = metadata.DllHead;

            while (iter != NULL_IDX)
            {

                TokenNode &leftTokenNode = tokenListPool.get(iter);
                leftTokenNode.Count = metadata.count;

                if (leftTokenNode.NextNodeIndex == NULL_IDX)
                    break;

                TokenNode &rightTokenNode = tokenListPool.get(leftTokenNode.NextNodeIndex);

                uint_64 TokenPairHash = GetTokenPairHash(leftTokenNode.TokenId, rightTokenNode.TokenId);

                auto tokenNode = TokenMap.find(TokenPairHash);

                if (tokenNode == TokenMap.end())
                {

                    TokenPairData data{metadata.count, {make_pair(iter, metadata.count)}};
                    TokenMap[TokenPairHash] = data;
                }
                else
                {

                    TokenPairData &data = TokenMap[TokenPairHash];
                    data.count += metadata.count;
                    data.TokensIndex.push_back(make_pair(iter, metadata.count));
                }

                iter = leftTokenNode.NextNodeIndex;
            }
        }

        for (auto &[tokenPairHash, data] : TokenMap)
        {

            TokenPQ.push({data.count, tokenPairHash});
        }

        // Everything till here is one time

        while (NextTokenID < VocabSize && !TokenPQ.empty())
        {

            auto [TokenPairFreq, TokenPairHash] = TokenPQ.top();

            if (TokenPairFreq != TokenMap[TokenPairHash].count)
            {

                TokenPQ.pop();
                TokenPQ.push({TokenMap[TokenPairHash].count, TokenPairHash});
            }
            else if (TokenPairFreq == 0)
            {

                TokenPQ.pop();
            }
            else
            {
                auto [leftTokenID, rightTokenID] = GetTokenPair(TokenPairHash);

                string NewTokenStr = TokenIdBytes[leftTokenID];

                for(unsigned char c : TokenIdBytes[rightTokenID]){

                    NewTokenStr.push_back(c);
                }

                uint_32 isNewTokenPresent = GetTokenFromRadixTree(TokenTreeRootIndex,NewTokenStr,0,tokenRadixTreePool);
                
                uint_32 NewTokenID;
                if(isNewTokenPresent != NULL_IDX){
                   
                    NewTokenID = isNewTokenPresent;
                }
                else{
                    NewTokenID= NextTokenID++;

                }



                
                // Radix Tree Operation Pending
                
                if(isNewTokenPresent == NULL_IDX){
                    
                    ChildTokenIdArray[NewTokenID] = {leftTokenID, rightTokenID};
                    InsertIntoTokenTree(TokenIdToRadixIndexMap[leftTokenID],TokenIdBytes[rightTokenID],NewTokenID,tokenRadixTreePool);
                    TokenIdBytes[NewTokenID] = NewTokenStr;
                }

                TokenPairData &pairData = TokenMap[TokenPairHash];

                auto &PairLocations = pairData.TokensIndex;

                unordered_set<uint_64> NewTokenIdHashes;

                for (auto [TokenIndex, TokenCount] : PairLocations)
                {

                    TokenNode &LeftTokenNode = tokenListPool.get(TokenIndex);

                    if (LeftTokenNode.NextNodeIndex == NULL_IDX)
                    {
                        continue;
                    }
                    TokenNode &rightTokenNode = tokenListPool.get(LeftTokenNode.NextNodeIndex);

                    if (LeftTokenNode.TokenId != leftTokenID || rightTokenNode.TokenId != rightTokenID)
                    {

                        continue; // It's a stale pointer! Skip it safely in O(1) time.
                    }

                    if (LeftTokenNode.PrevNodeIndex != NULL_IDX)
                    {

                        TokenNode &PrevTokenNode = tokenListPool.get(LeftTokenNode.PrevNodeIndex);

                        uint_64 Old_Prev_Left_TokenPairHash = GetTokenPairHash(PrevTokenNode.TokenId, leftTokenID);

                        uint_64 New_Prev_Left_TokenPairHash = GetTokenPairHash(PrevTokenNode.TokenId, NewTokenID);

                        TokenMap[Old_Prev_Left_TokenPairHash].count -= PrevTokenNode.Count;

                        auto New_prev_left_TokenDataNode = TokenMap.find(New_Prev_Left_TokenPairHash);

                        if (New_prev_left_TokenDataNode == TokenMap.end())
                        {

                            TokenPairData data{PrevTokenNode.Count, {make_pair(LeftTokenNode.PrevNodeIndex, PrevTokenNode.Count)}};
                            TokenMap[New_Prev_Left_TokenPairHash] = data;
                        }
                        else
                        {

                            TokenPairData &data = TokenMap[New_Prev_Left_TokenPairHash];
                            data.count += PrevTokenNode.Count;
                            data.TokensIndex.push_back(make_pair(LeftTokenNode.PrevNodeIndex, PrevTokenNode.Count));
                        }

                        NewTokenIdHashes.insert(New_Prev_Left_TokenPairHash);
                    }

                    if (rightTokenNode.NextNodeIndex != NULL_IDX)
                    {

                        TokenNode &NextTokenNode = tokenListPool.get(rightTokenNode.NextNodeIndex);

                        uint_64 Old_right_next_TokenPairHash = GetTokenPairHash(rightTokenNode.TokenId, NextTokenNode.TokenId);

                        uint_64 New_left_next_TokenPairHash = GetTokenPairHash(NewTokenID, NextTokenNode.TokenId);

                        TokenMap[Old_right_next_TokenPairHash].count -= NextTokenNode.Count;

                        auto New_left_next_TokenDataNode = TokenMap.find(New_left_next_TokenPairHash);

                        if (New_left_next_TokenDataNode == TokenMap.end())
                        {

                            TokenPairData data{rightTokenNode.Count, {make_pair(rightTokenNode.PrevNodeIndex, rightTokenNode.Count)}};
                            TokenMap[New_left_next_TokenPairHash] = data;
                        }
                        else
                        {

                            TokenPairData &data = TokenMap[New_left_next_TokenPairHash];
                            data.count += rightTokenNode.Count;
                            data.TokensIndex.push_back(make_pair(rightTokenNode.PrevNodeIndex, rightTokenNode.Count));
                        }

                        NewTokenIdHashes.insert(New_left_next_TokenPairHash);
                    }

                    LeftTokenNode.NextNodeIndex = rightTokenNode.NextNodeIndex;

                    if (rightTokenNode.NextNodeIndex != NULL_IDX)
                    {

                        TokenNode &NextTokenNode = tokenListPool.get(rightTokenNode.NextNodeIndex);

                        NextTokenNode.PrevNodeIndex = rightTokenNode.PrevNodeIndex;
                    }

                    LeftTokenNode.TokenId = NewTokenID;
                    LeftTokenNode.Count = rightTokenNode.Count;
                }

                for (auto &UTokenPairHash : NewTokenIdHashes)
                {

                    TokenPQ.push({TokenMap[UTokenPairHash].count, UTokenPairHash});
                }

                TokenPQ.pop();
            }
        }

        vector<uint8_t> serializedBuffer = BufferSerializer::SerializeToBuffer(NextTokenID, TokenIdBytes, tokenRadixTreePool);
        DiskManager::SaveBuffer(serializedBuffer,"learnedVocab.bin");

        cout << "Successfully serialized to in-memory buffer of size: " << serializedBuffer.size() << " bytes." << endl;

        for(int i = 0 ; i < TokenIdBytes.size(); i++){

            cout << "Token ID : " << i << " => " << TokenIdBytes[i] << endl;
        }
        return true; 
    }
};
