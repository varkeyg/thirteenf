#ifndef HELPERTOOLS_H
#define HELPERTOOLS_H


#include <iostream>
#include <rocksdb/db.h>
#include <memory>

namespace helpertools {



    class KVStore {
    public:
        KVStore() {
            options.create_if_missing = true;
            rocksdb::Status status    = rocksdb::DB::Open(options, cache_location, &db);
            assert(status.ok());
        };
        KVStore(std::string &cache_location)
            : KVStore() {
            this->cache_location = cache_location;
        };
        ~KVStore();
        rocksdb::Status put(const std::string &key, const std::string &value);
        std::string get(const std::string &key);

    private:
        std::string cache_location{"/tmp/rocksdb_data"};
        rocksdb::DB *db;
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
        WebTools(std::shared_ptr<helpertools::KVStore> kvs):kvs{kvs}{};
        http_result http_get(const std::string &url);

    private:
        std::shared_ptr<helpertools::KVStore> kvs;
        std::string cache_location{"/tmp/rocksdb_data"};
    };



}  // namespace helpertools

#endif  // HELPERTOOLS_H