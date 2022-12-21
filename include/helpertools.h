#ifndef HELPERTOOLS_H
#define HELPERTOOLS_H


#include <iostream>
#include <rocksdb/db.h>
#include <memory>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>

namespace helpertools {

    std::vector<std::string> split(std::string inp, const std::string &delim);

    class KVStore {
    public:
        KVStore() {
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
        explicit WebTools(std::shared_ptr<helpertools::KVStore> &kvs)
            : kvs{kvs} {};
        http_result http_get(const std::string &url);

    private:
        std::shared_ptr<helpertools::KVStore> kvs;
        std::string cache_location{"/tmp/rocksdb_data"};
    };


    class SqliteDB {
    public:
        explicit SqliteDB(const std::string &db_location)
            : db_location{db_location} {
                  db = std::make_shared<SQLite::Database>(db_location,SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
              };

        void runsql(const std::string& sql);
    private:
        const std::string db_location;
        std::shared_ptr<SQLite::Database> db;
    };


}  // namespace helpertools

#endif  // HELPERTOOLS_H