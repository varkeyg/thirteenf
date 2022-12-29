//
// Created by Geo Varkey on 12/18/22.
//
#include <iostream>
#include <vector>
#include "helpertools.h"
#include "thirteenf.h"
#include "spdlog/spdlog.h"
#include <fstream>




using namespace std;



int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);
    std::vector<string> params(argv + 1, argv + argc);

    if (params.empty()) params.push_back("/Users/gvarkey/workspace/thirteenf/resources/thirteenf_config.json");

    if (params.empty()) {
        spdlog::error("config file location was not passed in");
    } else {
        spdlog::info("Reading configfile {0}", params.at(0));
        thirteenf::RuntimeConfig rtc{params.at(0)};
        thirteenf::EdgarIndex ei{rtc};
        ei.load_index_urls();
        ei.load_index();
        spdlog::info("Entries in the index: {0}", rtc.index->size());
        thirteenf::EdgarHoldings eh{rtc};
        eh.load_holdings();
        //spdlog::info("Total Holdings records extracted {0}", rtc.holdings->size());
        //rtc.sqldb->save2db(rtc.holdings, "holdings");
    }
    return 0;
};