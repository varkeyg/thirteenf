#ifndef HELPERTOOLS_H
#define HELPERTOOLS_H


#include <iostream>
#include <rocksdb/db.h>
#include <memory>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>
#include "spdlog/spdlog.h"
#include <tsl/ordered_map.h>


namespace helpertools {
    using strmap   = tsl::ordered_map<std::string, std::string>;
    using rel_type = std::vector<strmap>;
    using relation = std::unique_ptr<rel_type>;


    std::vector<std::string> split(std::string inp, const std::string &delim);

    std::string map2str(tsl::ordered_map<std::string, std::string>& m);


    class KVStore {
    public:
        KVStore() {
            //std::cout << "Initiliazing cache_location to " << cache_location << std::endl;
            options.create_if_missing = true;
            rocksdb::Status status    = rocksdb::DB::Open(options, cache_location, &db);
            
            assert(status.ok());
        };
        explicit KVStore(const std::string &cache_location)
            : KVStore() {
            this->cache_location = cache_location;
        };
        ~KVStore();
        rocksdb::Status put(const std::string &key, const std::string &value);
        std::string get(const std::string &key);

    private:
        std::string cache_location{"/tmp/rocksdb_data"};
        rocksdb::DB *db{};
        rocksdb::Options options;
    };




    struct http_result {
        std::string response;
        std::string error;
        http_result() = default;
        // copy constructor to see if we end up copying and not doing RVO.
        http_result(const http_result &) {
            std::cout << "Copy is made" << std::endl;
        };
    };




    class WebTools {
    public:
        explicit WebTools(std::unique_ptr<helpertools::KVStore> &kvs)
            : kvs{kvs} {};
        http_result http_get(const std::string &url);

    private:
        std::unique_ptr<helpertools::KVStore>& kvs;
        std::string cache_location{"/tmp/rocksdb_data"};
    };


    class SqliteDB {
    public:
        explicit SqliteDB(const std::string &db_location)
            : db_location{db_location} {
                  db = std::make_shared<SQLite::Database>(db_location,SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
              };

        void runsql(const std::string& sql);
        std::string generate_ddl(relation &table, const std::string &table_name);
        std::shared_ptr<SQLite::Statement> runquery(std::string sql);
        void save2db(relation& table, const std::string &table_name);

    private:
        const std::string db_location;
        std::shared_ptr<SQLite::Database> db;
    };


}  // namespace helpertools

#endif  // HELPERTOOLS_H