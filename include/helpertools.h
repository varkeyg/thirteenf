#ifndef HELPERTOOLS_H
#define HELPERTOOLS_H


#include <iostream>
#include <rocksdb/db.h>


namespace helpertools {


    class KVStore {
    public:
        KVStore(){
            options.create_if_missing = true;
            rocksdb::Status status    = rocksdb::DB::Open(options, cache_location, &db);
            assert(status.ok());
        };
        KVStore(std::string &cache_location)
            : KVStore() {
            this->cache_location = cache_location;
            // options.create_if_missing = true;
            // rocksdb::Status status    = rocksdb::DB::Open(options, cache_location, &db);
            // assert(status.ok());
        };
        ~KVStore();
        rocksdb::Status put(const std::string &key, const std::string &value);
        std::string get(const std::string &key);

    private:
        std::string cache_location{"/tmp/rocksdb_data"};
        rocksdb::DB *db;
        rocksdb::Options options;
        //const std::string cache_location;
    };




    struct http_result {
        std::string response;
        std::string error;
    };




    class WebTools {
    public:
        WebTools(const std::string &cache_location)
            : cache_location{cache_location} {};
        http_result httpGet(const std::string &url);

    private:
        std::string cache_location{"/tmp/rocksdb_data"};
    };

}  // namespace helpertools

#endif  // HELPERTOOLS_H