
#include "thirteenf.h"
#include <boost/date_time.hpp>
#include <thread>
#include <tinyxml2.h>


using namespace std;
using namespace boost::gregorian;



void thirteenf::Index::build_index_urls() {
    auto process_date = from_string(rt.get_from_date());
    auto end_date     = from_string(rt.get_to_date());
    auto get_quarter  = [](auto process_date) { return static_cast<short>(ceil(static_cast<float>(process_date.month()) / 3)); };
    while (process_date <= end_date) {
        std::string url{sec_site + "edgar/daily-index/" + to_string(process_date.year()) + "/QTR" + to_string(get_quarter(process_date)) +
                        "/master." + to_iso_string(process_date) + ".idx"};
        index_urls.push_back(url);
        process_date = process_date + days(1);
    }
};




thirteenf::sec_index_ptr thirteenf::Index::get_index() {
    bool add_to_index = false;
    build_index_urls();
    for (auto &index_url : index_urls) {
        auto x     = rt.get_webtools()->http_get(index_url);
        auto lines = helpertools::split(x.response, "\n");
        for (auto &line : lines) {
            auto fields = helpertools::split(line, "|");
            if (fields.size() == 5 and fields.at(0) != "CIK") {
                add_to_index = rt.get_cik_list()->empty();
                if (!rt.get_cik_list()->empty()) {
                    add_to_index =
                        std::find(rt.get_cik_list()->begin(), rt.get_cik_list()->end(), fields.at(0)) != rt.get_cik_list()->end();
                }
                if (add_to_index) {
                    sidx.cik.push_back(fields.at(0));
                    sidx.holder.push_back(fields.at(1));
                    sidx.form_type.push_back(fields.at(2));
                    sidx.date_filed.push_back(fields.at(3));
                    sidx.file_name.push_back(fields.at(4));
                }
            }
        }
    }
    return std::make_shared<thirteenf::sec_index>(sidx);
}



void thirteenf::RuntimeContext::initialize() {
    kvs      = std::make_shared<helpertools::KVStore>(cache_location);
    wt       = std::make_shared<helpertools::WebTools>(kvs);
    cik_list = std::make_shared<std::vector<std::string>>(helpertools::split(cik_string, ","));
}



std::shared_ptr<helpertools::KVStore> thirteenf::RuntimeContext::get_kvstore() {
    return kvs;
}


std::shared_ptr<helpertools::WebTools> thirteenf::RuntimeContext::get_webtools() {
    return wt;
}
std::string thirteenf::RuntimeContext::get_from_date() {
    return from_date;
}
std::string thirteenf::RuntimeContext::get_to_date() {
    return to_date;
}
std::shared_ptr<std::vector<std::string>> thirteenf::RuntimeContext::get_cik_list() {
    return cik_list;
}




void thirteenf::Holdings::process() {
    vector<thread> workers;
    for (int i = 0; i < 5; ++i) {
        workers.emplace_back(thread(&thirteenf::Holdings::process_filings, this));
    }
    for (auto &t : workers) {
        t.join();
    }
}



void thirteenf::Holdings::process_filings() {
    while (true) {
        std::unordered_map<std::string, std::string> filing;
        {
            const lock_guard<mutex> lock(index_entry_mutex);
            if (index_entry < rt.index->get_max_index()) {
                filing = rt.index->get_record(index_entry);
                index_entry++;
            } else {
                break;
            }
        }

        get_holdings_from_filing(filing);
    }
}



thirteenf::holdings_ptr thirteenf::Holdings::get_holdings() {
    return std::make_shared<thirteenf::holdings_t>(holdings);
}




void thirteenf::Holdings::get_holdings_from_filing(std::unordered_map<std::string, std::string> &filing) {
    auto url = std::string{sec_site} + filing["file_name"];

    auto x       = rt.get_webtools()->http_get(url);
    auto start   = x.response.find("<informationTable");
    auto length  = (x.response.find("</informationTable>") - start) + (string{"</informationTable>"}.size());
    auto xml_str = x.response.substr(start, length);
    tinyxml2::XMLDocument doc;
    doc.Parse(xml_str.c_str());
    if (!doc.Error()) {
        auto infoTable = doc.FirstChildElement("informationTable")->FirstChildElement("infoTable");
        while (infoTable != nullptr) {
            cout << infoTable->FirstChildElement("nameOfIssuer")->GetText() << endl;
            infoTable = infoTable->NextSiblingElement();
        }
    }


    const lock_guard<mutex> lock(holdings_mutex);
    // holdings.cik.push_back(cik);
}
