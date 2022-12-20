#include "helpertools.h"
#include "thirteenf.h"
#include <gtest/gtest.h>
#include <memory>
#include <boost/date_time.hpp>

using namespace std;


class TestInfra : public ::testing::Test {
protected:
    std::string cache_location{"tmp/rocksdb_data"};
    //std::shared_ptr<helpertools::KVStore> kvs = std::make_shared<helpertools::KVStore>(cache_location);
    //helpertools::WebTools wt{kvs};
    std::string from_date = "2022/08/10";
    std::string to_date   = "2022/08/15";
    thirteenf::RuntimeContext rt{cache_location,from_date, to_date, "1067983,93751"};
};
//
//
//
//TEST_F(TestInfra, test001) {
//    std::string key{"K1"};
//    kvs->put(key, "Value001");
//    auto val = kvs->get(key);
//    EXPECT_EQ("Value001", val);
//};
//
//
//
//
//TEST_F(TestInfra, test002) {
//    std::string index_url = string{"https://www.sec.gov/Archives/edgar/daily-index/2022/QTR4/master.20221003.idx"};
//    auto x                = wt.http_get(index_url);
//    EXPECT_EQ(426030, x.response.length());
//};
//
//
TEST_F(TestInfra, test003) {

    thirteenf::Index ix{rt};
   rt.index = ix.get_index();
    //index->print();
    //rt.add_index(std::move(index));
    rt.index->print();
};

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
};
