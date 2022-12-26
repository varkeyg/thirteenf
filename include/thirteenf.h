#ifndef THIRTEENF_H
#define THIRTEENF_H
#include <iostream>
#include <vector>
#include <unordered_map>
#include "helpertools.h"
#include <iomanip>
#include <sstream>
#include <memory>
#include <mutex>
#include <tsl/ordered_map.h>
#include "nlohmann/json.hpp"
#include <fstream>
#include <set>

using json = nlohmann::json;

namespace thirteenf {

    using vecstr     = std::vector<std::string>;
    using strmap     = tsl::ordered_map<std::string, std::string>;
    using index_urls = std::unique_ptr<vecstr>;
    using rel_type   = std::vector<strmap>;
    using relation   = std::unique_ptr<rel_type>;




    struct run_params_t {
        std::string from_date;
        std::string to_date;
        std::string kvstore_cache;
        std::string cns_fails_data;
        std::string cik_filter;
        std::string form_filter;
        std::string sec_site;
        std::string sqldb;
        int num_threads;
    };



    class RuntimeConfig {
    public:
        RuntimeConfig(const std::string &config_file) {
            std::ifstream f(config_file);
            auto config_json          = json::parse(f);
            run_params.from_date      = config_json["from_date"];
            run_params.to_date        = config_json["to_date"];
            run_params.sec_site       = config_json["sec_site"];
            run_params.cik_filter     = config_json["cik_filter"];
            run_params.form_filter    = config_json["form_filter"];
            run_params.kvstore_cache  = config_json["kvstore_cache"];
            run_params.sqldb          = config_json["sqldb"];
            run_params.cns_fails_data = config_json["cns_fails_data"];
            run_params.num_threads    = std::stoi(std::string{config_json["num_threads"]});
            kvs                       = std::make_unique<helpertools::KVStore>(run_params.kvstore_cache);
            wt                        = std::make_unique<helpertools::WebTools>(kvs);
            sqldb                     = std::make_unique<helpertools::SqliteDB>(run_params.sqldb);
            index                     = std::make_unique<rel_type>();
            holdings                  = std::make_unique<rel_type>();
            init();
        };
        run_params_t run_params;
        index_urls index_urls;
        relation index;
        relation holdings;
        bool include_cik(std::string cik);
        bool include_form(std::string form_name);
        std::unique_ptr<helpertools::KVStore> kvs;
        std::unique_ptr<helpertools::WebTools> wt;
        std::unique_ptr<helpertools::SqliteDB> sqldb;

    private:
        void init();
        std::set<std::string> cik_filter_set;
        std::set<std::string> form_filter_set;
    };




    class EdgarIndex {
    public:
        explicit EdgarIndex(RuntimeConfig &rtc) : rtc{rtc} {};
        void load_index_urls();
        void load_index();

    private:
        RuntimeConfig &rtc;
    };




    class EdgarHoldings {
    public:
        explicit EdgarHoldings(RuntimeConfig &rtc) : rtc{rtc} {};
        void load_holdings();

    private:
        RuntimeConfig &rtc;
        int current_index_id = 0;
        std::mutex index_mutex;
        std::mutex holdings_mutex;
        void load_holdings_worker();
        void clean_tags(std::string &inp);
        void get_header_info(strmap &header, const std::string &filing);
        void load_cusip_ticker_map();
    };


}  // namespace thirteenf

#endif  // THIRTEENF_H
