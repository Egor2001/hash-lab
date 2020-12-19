// #include "ChainHashTable.h"
// #include "OpenLinearAddrHashTable.h"
// #include "OpenQuadroAddrHashTable.h"
// #include "OpenDoubleAddrHashTable.h"
#include "CuckooHashTable.h"
 
#include <iostream>
#include <fstream>
#include <string>
 
int main()
{
    CCuckooHashTable<std::string, std::string> ht;
    // COpenDoubleAddrHashTable<std::string, std::string> ht;
    // COpenQuadroAddrHashTable<std::string, std::string> ht;
    // COpenLinearAddrHashTable<std::string, std::string> ht;
    // CChainHashTable<std::string, std::string> ht;
 
    std::ifstream stream_in("map.in");
    std::ofstream stream_out("map.out");
 
    std::string cmd, x, y;
    std::string none_str = "none";
    while (stream_in)
    {
        cmd.clear();
        stream_in >> cmd;
 
        if (cmd == "put")
        {
            stream_in >> x >> y;
            ht.insert(x, y);
        }
        else if (cmd == "delete")
        {
            stream_in >> x;
            ht.erase(x);
        }
        else if (cmd == "get")
        {
            stream_in >> x;
            auto opt = ht.find(x);
 
            stream_out << opt.value_or(none_str).get() << '\n';
        }
    }
 
    return 0;
}
