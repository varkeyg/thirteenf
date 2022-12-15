#include "helpertools.h"
#include <gtest/gtest.h>
#include <memory>

using namespace std;


class TestInfra : public ::testing::Test {
protected:
    std::string cache_location{"tmp/rocksdb_data"};
    std::shared_ptr<helpertools::KVStore> kvs = std::make_shared<helpertools::KVStore>(cache_location);
    helpertools::WebTools wt{kvs};
};



TEST_F(TestInfra, test001) {
    std::string key{"K1"};
    kvs->put(key, "Soumya");
    auto val = kvs->get(key);
    EXPECT_EQ("Soumya", val);
};




TEST_F(TestInfra, test002) {
    std::string index_url = string{
        "https://www.sec.gov/Archives/edgar/daily-index/2022/"
        "QTR4/master.20221003.idx"};
    auto x = wt.http_get(index_url);
    EXPECT_EQ(426030, x.response.length());
};


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
};
