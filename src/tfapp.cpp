//
// Created by Geo Varkey on 12/18/22.
//
#include <iostream>
#include "helpertools.h"
#include "thirteenf.h"

int main(int argc, char *argv[]) {
    std::string cache_location{"tmp/rocksdb_data"};
    std::string from_date = "2022/08/10";
    std::string to_date   = "2022/08/15";
    thirteenf::RuntimeContext rt{cache_location, from_date, to_date, "1067983,93751"};

    thirteenf::Index ix{rt};
    rt.index = ix.get_index();
    //rt.index->print();
    thirteenf::Holdings h{rt};
    h.process();
    rt.holdings = h.get_holdings();
    std::cout << rt.holdings->cik.size() << std::endl;
    return 0;
};