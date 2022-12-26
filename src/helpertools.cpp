#include "helpertools.h"
#include <curl/curl.h>
#include <sstream>
#include <boost/algorithm/string.hpp>

using namespace std;


std::vector<std::string> helpertools::split(std::string inp, const std::string &delim) {
    std::vector<std::string> entries;
    if (inp.empty()) {
        return entries;
    }
    boost::split(entries, inp, boost::is_any_of(delim));
    return entries;
};



std::string helpertools::map2str(tsl::ordered_map<std::string, std::string> &m) {
    stringstream ss;
    for (auto &entry : m) {
        ss << setw(15) << entry.first << " " << entry.second << "\n";
    }
    return ss.str();
};




rocksdb::Status helpertools::KVStore::put(const std::string &key, const std::string &value) {
    auto status = db->Put(rocksdb::WriteOptions(), key, value);
    return status;
}




std::string helpertools::KVStore::get(const std::string &key) {
    std::string value;
    rocksdb::Status s;
    s = db->Get(rocksdb::ReadOptions(), key, &value);
    return s.IsNotFound() ? string{""} : value;
}


helpertools::KVStore::~KVStore() {
    db->Close();
    delete (db);
}


size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
};


helpertools::http_result helpertools::WebTools::http_get(const std::string &url) {
    helpertools::http_result hr;
    hr.response = kvs->get(url);
    hr.error    = "";
    if (!hr.response.empty()) {
        // spdlog::info("Using cached filing data for url {0}: ", url);
        return hr;
    }
    // else{
    //     spdlog::info("cache not found for url {0}: ", url);
    // }
    std::string response;
    std::string error_message;
    long http_code = 0;
    auto curl      = curl_easy_init();

    if (curl) {
        CURLcode res;
        char error[CURL_ERROR_SIZE];
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &hr.response);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
        curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
        curl_easy_setopt(curl,
                         CURLOPT_USERAGENT,
                         "Mozilla/5.0 (Macintosh; Intel Mac OS X 11_3_1) "
                         "AppleWebKit/537.36 (KHTML, "
                         "like Gecko) Chrome/90.0.4430.93 Safari/537.36 GEO "
                         "NOBODY nobody@gmail.com ");
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);
        curl     = nullptr;
        hr.error = error;

        stringstream ss;
        if (res != CURLE_OK or http_code != 200) {
            if (res != CURLE_OK) {
                ss << "Error: making call: " << url << " curl error: " << res << "\nError Message: " << hr.response << hr.error << endl;
            } else {
                ss << "Error: in url: " << url << "..  http error code: " << http_code
                   << "\nError Message: " << hr.response + ":" + hr.error << endl;
            }
            hr.error    = ss.str();
            hr.response = "";
        }
    }
    if (hr.error.empty()) {
        kvs->put(url, hr.response);  // Cache the data
    }
    return hr;
};



void helpertools::SqliteDB::runsql(const std::string &sql) {
    SQLite::Transaction transaction(*db);
    db->exec(sql);
    transaction.commit();
}


std::shared_ptr<SQLite::Statement> helpertools::SqliteDB::runquery(std::string sql) {
    auto query = std::make_shared<SQLite::Statement>(*db, sql);
    return query;
};


void helpertools::SqliteDB::save2db(relation &table, const std::string &table_name) {
    stringstream ss;
    ss << "create table " << table_name << "\n(\n";
    if (table->empty()) {
        return;
    } else {
        auto record = table->at(0);
        for (auto &field : record) {
            ss << "   " << setw(15) << left << field.first << "text,\n";
        }
        ss << "   " << setw(15) << left << "load_time"
           << "text";
        ss << "\n)";
    }
    runsql("drop table if exists " + table_name);
    runsql(ss.str()); //create table
    ss.str(std::string()); // clear the contents
    size_t cntr         = 0;
    std::string prefix  = "insert into " + table_name + " values (";
    std::string ins_sql = "";
    for (auto &rec : *table) {
        ins_sql = prefix;
        for (auto &field : rec) {
            auto val = field.second;
            boost::replace_all(val, "'", "");
            ins_sql  = ins_sql + "'" + val + "'" + ",";
        }
        ins_sql = ins_sql +  "datetime('now','localtime') );";
        ss << ins_sql;
        cntr++;
        if (cntr % 1000 == 0){
            runsql(ss.str());
            ss.str(std::string());
            spdlog::info("Records inserted to table {0} : {1}", table_name, cntr);
        }
    }
    runsql(ss.str());
    spdlog::info("Records inserted to table {0} : {1}", table_name, cntr);
};