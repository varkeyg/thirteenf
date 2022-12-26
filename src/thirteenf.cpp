
#include "thirteenf.h"
#include <boost/date_time.hpp>
#include <thread>
#include <tinyxml2.h>
#include <regex>

using namespace std;
using namespace boost::gregorian;

void thirteenf::RuntimeConfig::init() {
    auto values     = helpertools::split(run_params.cik_filter, ",");
    cik_filter_set  = std::set<std::string>(values.begin(), values.end());
    auto forms      = helpertools::split(run_params.form_filter, ",");
    form_filter_set = std::set<std::string>(forms.begin(), forms.end());
};



bool thirteenf::RuntimeConfig::include_cik(std::string cik) {
    if (run_params.cik_filter.empty()) {
        return true;
    } else {
        if (cik_filter_set.contains(cik)) {
            return true;
        } else {
            return false;
        }
    }
};



bool thirteenf::RuntimeConfig::include_form(std::string form_name) {
    if (run_params.form_filter.empty()) {
        return true;
    } else {
        if (form_filter_set.contains(form_name)) {
            return true;
        } else {
            return false;
        }
    }
};



void thirteenf::EdgarIndex::load_index_urls() {
    vecstr index_urls;
    std::set<std::string> weekends{"6", "0"};
    auto process_date = from_string(rtc.run_params.from_date);
    auto end_date     = from_string(rtc.run_params.to_date);
    auto get_quarter  = [](auto process_date) { return static_cast<short>(ceil(static_cast<float>(process_date.month()) / 3)); };
    while (process_date <= end_date) {
        if (!(weekends.contains(to_string(process_date.day_of_week())))) {
            std::string url{rtc.run_params.sec_site + "edgar/daily-index/" + to_string(process_date.year()) + "/QTR" +
                            to_string(get_quarter(process_date)) + "/master." + to_iso_string(process_date) + ".idx"};
            index_urls.push_back(url);
        }
        process_date = process_date + days(1);
    }
    spdlog::info("Generated index urls for days: {0}", index_urls.size());
    rtc.index_urls = std::make_unique<vecstr>(index_urls);
};




void thirteenf::EdgarIndex::load_index() {
    size_t cntr = 0;
    for (auto &url : *rtc.index_urls) {
        try {
            auto x     = rtc.wt->http_get(url);
            auto lines = helpertools::split(x.response, "\n");
            for (auto &line : lines) {
                auto fields = helpertools::split(line, "|");
                strmap record;
                if (fields.size() == 5 and rtc.include_cik(fields.at(0)) and rtc.include_form(fields.at(2))) {
                    record["cik"]        = fields.at(0);
                    record["holder"]     = fields.at(1);
                    record["form_type"]  = fields.at(2);
                    record["date_filed"] = fields.at(3);
                    record["file_name"]  = fields.at(4);
                    rtc.index->push_back(record);
                }
            }
            // spdlog::info("Loaded {0} filing index entries for {1}", rtc.index->size(), helpertools::split(url, ".").at(3));
        } catch (const std::exception &e) {
            spdlog::error(e.what() + std::string(" Error processing url ") + url);
        };
        cntr++;
        if (cntr % 30 == 0) {
            spdlog::info("Extracted filings for {0} days", cntr);
        }
    }
    spdlog::info("Extracted filings for {0} days", cntr);
};




void thirteenf::EdgarHoldings::load_holdings() {
    load_cusip_ticker_map();
    vector<thread> workers;
    for (int i = 0; i < rtc.run_params.num_threads; ++i) {
        workers.emplace_back(thread(&thirteenf::EdgarHoldings::load_holdings_worker, this));
    }
    spdlog::info("Starting {0} threads to process filings", workers.size());
    for (auto &t : workers) {
        t.join();
    }
};


// Running in a thread
void thirteenf::EdgarHoldings::load_holdings_worker() {
    while (true) {
        strmap filing;
        // Isolate the scope for taking lock
        {
            const lock_guard<mutex> lock(index_mutex);
            if (current_index_id < rtc.index->size()) {
                filing = rtc.index->at(current_index_id);
                current_index_id++;
            } else {
                break;
            }
        }
        try {
            rel_type records;
            auto url = rtc.run_params.sec_site + filing["file_name"];
            auto x   = rtc.wt->http_get(url);
            strmap header;
            get_header_info(header, x.response);
            clean_tags(x.response);
            auto start   = x.response.find("<informationTable");
            auto length  = (x.response.find("</informationTable>") - start) + (string{"</informationTable>"}.size());
            auto xml_str = x.response.substr(start, length);

            tinyxml2::XMLDocument doc;
            doc.Parse(xml_str.c_str());
            if (!doc.Error()) {
                auto infoTable = doc.FirstChildElement("informationTable")->FirstChildElement("infoTable");
                while (infoTable != nullptr) {
                    strmap rec;
                    rec["holder_cik"]     = filing["cik"];
                    rec["holder_name"]    = filing["holder"];
                    rec["form"]           = filing["form_type"];
                    rec["date_filed"]     = filing["date_filed"];
                    rec["effective_date"] = header["effective_date"];
                    rec["period_date"]    = header["period_date"];
                    rec["holding_name"]   = infoTable->FirstChildElement("nameOfIssuer")->GetText();
                    rec["sec_type"]       = infoTable->FirstChildElement("titleOfClass")->GetText();
                    rec["cusip"]          = infoTable->FirstChildElement("cusip")->GetText();
                    rec["sym"]            = rtc.kvs->get(rec["cusip"]);
                    rec["market_value"]   = std::to_string(atol(infoTable->FirstChildElement("value")->GetText()) * 1000);
                    auto shprn            = infoTable->FirstChildElement("shrsOrPrnAmt");
                    rec["quantity"]       = std::to_string(atol(shprn->FirstChildElement("sshPrnamt")->GetText()));
                    rec["quantity_type"]  = shprn->FirstChildElement("sshPrnamtType")->GetText();
                    if (infoTable->FirstChildElement("putCall") != nullptr) {
                        rec["put_call"] = infoTable->FirstChildElement("putCall")->GetText();
                    } else {
                        rec["put_call"] = "";
                    }
                    records.push_back(rec);
                    infoTable = infoTable->NextSiblingElement();
                }
            }
            const lock_guard<mutex> lock(holdings_mutex);
            rtc.holdings->insert(rtc.holdings->end(), records.begin(), records.end());
            spdlog::info("Extracted {0} records from filing", records.size());
        } catch (const std::exception &e) {
            spdlog::error(e.what());
            spdlog::error(helpertools::map2str(filing));
        }
    }
};




void thirteenf::EdgarHoldings::clean_tags(std::string &inp) {
    std::vector<std::string> tags{"ns1:", "ns1 :", "ns1 :", "ns1", "ns2:", "N1:", "n1:", "n1:", "ns4:", "eis:", ":"};
    for (auto &tag : tags) {
        boost::replace_all(inp, tag, "");
    }
}



void thirteenf::EdgarHoldings::get_header_info(strmap &header, const std::string &filing) {
    auto edate        = "EFFECTIVENESS DATE:";
    auto pdate        = "CONFORMED PERIOD OF REPORT:";
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



void thirteenf::EdgarHoldings::load_cusip_ticker_map() {
    size_t cntr = 0;
    std::string line;
    ifstream f(rtc.run_params.cns_fails_data);
    while (std::getline(f, line)) {
        auto fields = helpertools::split(line, "|");
        if (fields.size() == 6) {
            rtc.kvs->put(fields.at(1), fields.at(2));
        }
        cntr++;
    }
    spdlog::info("Loaded cusip ticker map: {0} records", cntr);
};