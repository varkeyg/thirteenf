
#include "thirteenf.h"
#include <boost/date_time.hpp>
#include <thread>
#include <tinyxml2.h>
#include <regex>

using namespace std;
using namespace boost::gregorian;



void thirteenf::Index::build_index_urls() {
    std::set<std::string> weekends{"6", "0"};
    auto process_date = from_string(rt.get_from_date());
    auto end_date     = from_string(rt.get_to_date());
    auto get_quarter  = [](auto process_date) { return static_cast<short>(ceil(static_cast<float>(process_date.month()) / 3)); };
    while (process_date <= end_date) {
        if (!(weekends.contains(to_string(process_date.day_of_week())))) {
            std::string url{sec_site + "edgar/daily-index/" + to_string(process_date.year()) + "/QTR" +
                            to_string(get_quarter(process_date)) + "/master." + to_iso_string(process_date) + ".idx"};
            index_urls.push_back(url);
        }
        process_date = process_date + days(1);
    }
    spdlog::info("Number of days to process: {0}", index_urls.size());
};




thirteenf::sec_index_ptr thirteenf::Index::get_index() {
    bool add_to_index = false;
    build_index_urls();
    size_t counter = 0;
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
                    if (fields.at(2).rfind("13F-HR", 0) == 0) {
                        sidx.cik.push_back(fields.at(0));
                        sidx.holder.push_back(fields.at(1));
                        sidx.form_type.push_back(fields.at(2));
                        sidx.date_filed.push_back(fields.at(3));
                        sidx.file_name.push_back(fields.at(4));
                    }
                }
            }
        }
        counter++;
    }
    return std::make_shared<thirteenf::sec_index>(sidx);
}



void thirteenf::RuntimeContext::initialize() {
    // LOG(INFO) << "Google logging!";
    spdlog::info("Applying filter for CIK id's: {0} ", cik_string);

    kvs       = std::make_shared<helpertools::KVStore>(cache_location);
    wt        = std::make_shared<helpertools::WebTools>(kvs);
    cik_list  = std::make_shared<std::vector<std::string>>(helpertools::split(cik_string, ","));
    sql_store = std::make_shared<helpertools::SqliteDB>("/tmp/sqlite.db");
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
    // if(!cik_list->empty()){
    //     spdlog::info("Applying filter: {0} for CIK values",cik_list->size());
    // }
    return cik_list;
}




void thirteenf::Holdings::process() {
    load_cusip_ticker_map();
    vector<thread> workers;
    for (int i = 0; i < 1; ++i) {
        workers.emplace_back(thread(&thirteenf::Holdings::process_filings, this));
    }
    spdlog::info("Starting {0}", workers.size(), " threads to process filings");
    for (auto &t : workers) {
        t.join();
    }
}



void thirteenf::Holdings::process_filings() {
    try {
        while (true) {
            std::unordered_map<std::string, std::string> filing;
            {
                const lock_guard<mutex> lock(index_entry_mutex);
                if (index_entry < rt.index->get_max_index()) {
                    filing = rt.index->get_record(index_entry);
                    index_entry++;
                    spdlog::info("Filings Processed: {0}", index_entry);
                } else {
                    break;
                }
            }
            get_holdings_from_filing(filing);
        }
    } catch (const std::exception &e) {
        spdlog::error(e.what());
    };
}



thirteenf::holdings_ptr thirteenf::Holdings::get_holdings() {
    return std::make_shared<thirteenf::holdings_t>(holdings);
}


std::string thirteenf::Holdings::get_holdings_table_ddl() {
    stringstream ss;
    ss << "create table holdings ( \n";
    ss << "holder_cik       text,\n";
    ss << "holder_name      text,\n";
    ss << "form_type        text,\n";
    ss << "date_filed       text,\n";
    ss << "filing_url       text,\n";
    ss << "effective_date   text,\n";
    ss << "period_date      text,\n";
    ss << "holding_name     text,\n";
    ss << "sec_type         text,\n";
    ss << "cusip            text,\n";
    ss << "ticker           text,\n";
    ss << "market_value     integer,\n";
    ss << "quantity         integer,\n";
    ss << "qty_type         text,\n";
    ss << "put_call         text\n)";
    return ss.str();
};



void thirteenf::Holdings::get_holdings_from_filing(std::unordered_map<std::string, std::string> &filing) {
    auto url = std::string{sec_site} + filing["file_name"];
    auto x   = rt.get_webtools()->http_get(url);
    strmap header;
    get_header_info(header, x.response);
    // spdlog::info("Got http response!");
    clean_tags(x.response);
    // std::cout << x.response << std::endl;
    //  std::cout << url << endl;
    auto start   = x.response.find("<informationTable");
    auto length  = (x.response.find("</informationTable>") - start) + (string{"</informationTable>"}.size());
    auto xml_str = x.response.substr(start, length);
    // spdlog::info("After cleaning file size {0}", xml_str.size());

    tinyxml2::XMLDocument doc;
    doc.Parse(xml_str.c_str());
    if (!doc.Error()) {
        auto infoTable = doc.FirstChildElement("informationTable")->FirstChildElement("infoTable");
        while (infoTable != nullptr) {
            const lock_guard<mutex> lock(holdings_mutex);
            holdings.holder_cik.push_back(filing["cik"]);
            holdings.holder_name.push_back(filing["holder"]);
            holdings.form_type.push_back(filing["form_type"]);
            holdings.date_filed.push_back(filing["date_filed"]);
            holdings.filing_url.push_back(url);
            holdings.effective_date.push_back(header["effective_date"]);
            holdings.period_date.push_back(header["period_date"]);
            holdings.holding_name.push_back(infoTable->FirstChildElement("nameOfIssuer")->GetText());
            holdings.sec_type.push_back(infoTable->FirstChildElement("titleOfClass")->GetText());
            auto cusip = infoTable->FirstChildElement("cusip")->GetText();
            holdings.cusip.push_back(cusip);
            holdings.ticker.push_back(rt.get_kvstore()->get(cusip));
            holdings.market_value.push_back(atoi(infoTable->FirstChildElement("value")->GetText()) * 1000);
            auto shprn = infoTable->FirstChildElement("shrsOrPrnAmt");
            holdings.quantity.push_back(atoi(shprn->FirstChildElement("sshPrnamt")->GetText()));
            holdings.qty_type.push_back(shprn->FirstChildElement("sshPrnamtType")->GetText());

            if (infoTable->FirstChildElement("putCall") != nullptr) {
                holdings.put_call.push_back(infoTable->FirstChildElement("putCall")->GetText());
            } else {
                holdings.put_call.push_back("");
            }
            infoTable = infoTable->NextSiblingElement();
        }
    }
    
}




void thirteenf::Holdings::get_header_info(strmap &header, const std::string &filing) {
    auto edate = "EFFECTIVENESS DATE:";
    auto pdate = "CONFORMED PERIOD OF REPORT:";

    auto start        = filing.find("<SEC-HEADER>");
    auto length       = (filing.find("</SEC-HEADER>") - start) + (string{"</SEC-HEADER>"}.size());
    auto header_text  = filing.substr(start, length);
    auto header_lines = helpertools::split(header_text, "\n");
    for (auto &line : header_lines) {
        if (line.find(edate) != string::npos) {
            line                     = regex_replace(line, regex(edate), "");
            line                     = regex_replace(line, regex("\\s+"), "");
            header["effective_date"] = line;
        }
        if (line.find(pdate) != string::npos) {
            line                  = regex_replace(line, regex(pdate), "");
            line                  = regex_replace(line, regex("\\s+"), "");
            header["period_date"] = line;
        }
    }
};


void thirteenf::Holdings::clean_tags(std::string &inp) {
    std::vector<std::string> tags{"ns1:", "ns1 :", "ns1 :", "ns1", "ns2:", "N1:", "n1:", "n1:", "ns4:", "eis:", ":"};
    for (auto &tag : tags) {
        boost::replace_all(inp, tag, "");
    }
}



std::unique_ptr<std::string> thirteenf::Holdings::get_holdings_to_sql_statements() {
    std::stringstream ss;
    std::string prefix = {"insert into holdings values ( "};
    for (size_t i = 0; i < holdings.holder_cik.size(); i++) {
        ss << prefix;
        ss << "'" << holdings.holder_cik.at(i) << "', ";
        ss << "'" << holdings.holder_name.at(i) << "', ";
        ss << "'" << holdings.form_type.at(i) << "', ";
        ss << "'" << holdings.date_filed.at(i) << "', ";
        ss << "'" << holdings.filing_url.at(i) << "', ";
        ss << "'" << holdings.effective_date.at(i) << "', ";
        ss << "'" << holdings.period_date.at(i) << "', ";

        ss << "'" << holdings.holding_name.at(i) << "', ";
        ss << "'" << holdings.sec_type.at(i) << "', ";
        ss << "'" << holdings.cusip.at(i) << "', ";
        ss << "'" << holdings.ticker.at(i) << "', ";
        ss << holdings.market_value.at(i) << ", ";
        ss << holdings.quantity.at(i) << ", ";
        ss << "'" << holdings.qty_type.at(i) << "', ";
        ss << "'" << holdings.put_call.at(i) << "');\n";
    }
    return std::make_unique<std::string>(ss.str());
};



std::string thirteenf::RuntimeContext::get_cns_location() {
    return cns_fails_data;
};



void thirteenf::Holdings::load_cusip_ticker_map() {
    size_t cntr = 0;
    std::string line;
    ifstream f(rt.get_cns_location());
    while (std::getline(f, line)) {
        auto fields = helpertools::split(line, "|");
        if (fields.size() == 6) {
            rt.get_kvstore()->put(fields.at(1), fields.at(2));
        }
        cntr++;
    }
    spdlog::info("Loaded cusip ticker map: {0} records", cntr);
};
