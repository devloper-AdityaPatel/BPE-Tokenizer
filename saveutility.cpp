#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

class DiskManager {
public:


    static bool SaveBuffer(const vector<uint8_t>& buffer, const string& filepath) {
        ofstream out(filepath, ios::binary);
        if (!out) {
            return false;
        }
        
        out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        out.close();
        
        return true;
    }


    static bool LoadBuffer(vector<uint8_t>& buffer, const string& filepath) {
        ifstream in(filepath, ios::binary | ios::ate);
        if (!in) {
            return false;
        }
        
        streamsize fileSize = in.tellg();
        buffer.resize(fileSize);
        
        in.seekg(0, ios::beg);
        in.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        in.close();
        
        return true;
    }
};