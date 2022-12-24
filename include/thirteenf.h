#ifndef THIRTEENF_H
#define THIRTEENF_H
#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include "helpertools.h"
#include <iomanip>
#include <sstream>
#include <memory>
#include <mutex>


//Test comment

namespace thirteenf {

    using vecstr = std::vector<std::string>;
    using vecint = std::vector<uint32_t>;
    using strmap = std::unordered_map<std::string, std::string>;

    struct sec_index {
        std::vector<std::string> cik;
        std::vector<std::string> holder;
        std::vector<std::string> form_type;
        std::vector<std::string> date_filed;
        std::vector<std::string> file_name;
        std::unordered_map<std::string, std::string> get_record(size_t index) {
            std::unordered_map<std::string, std::string> record;
            record["cik"]        = cik.at(index);
            record["holder"]     = holder.at(index);
            record["form_type"]  = form_type.at(index);
            record["date_filed"] = date_filed.at(index);
            record["file_name"]  = file_name.at(index);
            return record;
        };

        [[nodiscard]] size_t get_max_index() const {
            return cik.size();
        };
        void print() {
            std::stringstream ss;
            std::cout << std::setw(10) << std::left << "CIK"
                      << " | " << std::setw(30) << "HOLDER"
                      << " | " << std::setw(7) << "FORM"
                      << " | "
                      << "DATE" << std::endl;
            std::cout << std::string(64, '-') << std::endl;
            for (size_t i = 0; i < get_max_index(); i++) {
                std::cout << std::setw(10) << std::left << cik.at(i) << " | " << std::setw(30) << holder.at(i) << " | " << std::setw(7)
                          << form_type.at(i) << " | " << date_filed.at(i) << std::endl;
            }
        }
    };


    // Final dataset.
    struct holdings_t {
        vecstr holder_cik;
        vecstr holder_name;
        vecstr form_type;
        vecstr date_filed;
        vecstr filing_url;
        vecstr effective_date;
        vecstr period_date;
        vecstr holding_name;
        vecstr sec_type;
        vecstr cusip;
        vecstr ticker;
        vecint market_value;
        vecint quantity;
        vecstr qty_type;
        vecstr put_call;
    };



    using sec_index_ptr = std::shared_ptr<sec_index>;
    using holdings_ptr  = std::shared_ptr<holdings_t>;




    class RuntimeContext {
    public:
        RuntimeContext(const std::string &cache_location,
                       const std::string &from_date,
                       const std::string &to_date,
                       const std::string &cik_string,
                       const std::string &cns_fails_data)
            : cache_location{cache_location},
              from_date{from_date},
              to_date{to_date},
              cik_string{cik_string},
              cns_fails_data{cns_fails_data} {
            initialize();
        };
        std::shared_ptr<helpertools::KVStore> get_kvstore();
        std::shared_ptr<helpertools::WebTools> get_webtools();
        std::string get_from_date();
        std::string get_to_date();
        std::shared_ptr<std::vector<std::string>> get_cik_list();
        sec_index_ptr index;
        holdings_ptr holdings;
        std::shared_ptr<helpertools::SqliteDB> sql_store;
        std::string get_cns_location();

    private:
        void initialize();
        const std::string &cache_location;
        std::shared_ptr<helpertools::KVStore> kvs;
        std::shared_ptr<helpertools::WebTools> wt;

        std::shared_ptr<std::vector<std::string>> cik_list;
        

    protected:
        const std::string &from_date;
        const std::string &to_date;
        const std::string &cik_string;
        const std::string &cns_fails_data;
    };




    class Index {
    public:
        explicit Index(RuntimeContext &rt)
            : rt{rt} {};
        sec_index_ptr get_index();

    private:
        RuntimeContext rt;
        std::string sec_site = "https://www.sec.gov/Archives/";
        std::set<std::string> form_list{"13F-HR", "13F-HR/A"};
        sec_index sidx;
        std::vector<std::string> index_urls;
        void build_index_urls();
    };




    class Holdings {
    public:
        explicit Holdings(RuntimeContext &rt)
            : rt{rt} {};
        void process();
        holdings_ptr get_holdings();
        std::unique_ptr<std::string> get_holdings_to_sql_statements();
        std::string get_holdings_table_ddl();

    private:
        std::string sec_site = "https://www.sec.gov/Archives/";
        int index_entry      = 0;
        std::mutex index_entry_mutex;
        holdings_t holdings;
        std::mutex holdings_mutex;
        RuntimeContext rt;
        void process_filings();
        void get_holdings_from_filing(std::unordered_map<std::string, std::string> &filing);
        void get_header_info(strmap &header, const std::string &filing);
        void clean_tags(std::string &inp);
        const std::string cusip_sym = "https://www.sec.gov/files/data/fails-deliver-data/cnsfails202211b.zip";
        void load_cusip_ticker_map();
    };



}  // namespace thirteenf

#endif  // THIRTEENF_H
