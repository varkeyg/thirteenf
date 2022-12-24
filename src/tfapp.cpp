//
// Created by Geo Varkey on 12/18/22.
//
#include <iostream>
#include "helpertools.h"
#include "thirteenf.h"
#include "spdlog/spdlog.h"


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Starting Thirteef run");
    std::string cache_location{"tmp/rocksdb_data"};
    std::string from_date      = "2022/08/10";
    std::string to_date        = "2022/08/15";
    std::string cns_fails_data = "../resources/cnsfails.txt";
    //std::string cik_string{"1067983,93751"};
    std::string cik_string;
    thirteenf::RuntimeContext rt{cache_location, from_date, to_date, cik_string, cns_fails_data};

    thirteenf::Index ix{rt};
    rt.index = ix.get_index();
    spdlog::info("Number of filings to process: {0}", rt.index->cik.size());
    thirteenf::Holdings h{rt};
    h.process();
    rt.holdings    = h.get_holdings();
    auto table_ddl = h.get_holdings_table_ddl();

    spdlog::info("Writing to sqlite database {0} ", rt.holdings->cusip.size(), "records");
    rt.sql_store->runsql("drop table if exists holdings");
    rt.sql_store->runsql(table_ddl);
    std::cout << *h.get_holdings_to_sql_statements() << std::endl;
    rt.sql_store->runsql(*h.get_holdings_to_sql_statements());
    spdlog::info("Completed writing to database");
    auto query = rt.sql_store->runquery("select * from holdings limit 10;");
    // while (query->executeStep()) {
    //     std::cout << query->getColumn(0) << std::endl;
    // }

    // std::cout << rt.holdings->holder_cik.size() << std::endl;
    return 0;
};