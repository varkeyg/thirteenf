#include "helpertools.h"
#include <gtest/gtest.h>
using namespace std;


TEST(KVStore, test001){
    
    helpertools::KVStore kvs;
    std::string key{"K1"};
    kvs.put(key, "Soumya");
    auto val = kvs.get(key);
    std::cout << "value is " << val << endl;
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
};  
 
