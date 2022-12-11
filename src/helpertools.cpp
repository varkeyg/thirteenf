#include "helpertools.h"

using namespace std;

rocksdb::Status helpertools::KVStore::put(const std::string &key, const std::string &value) {
    auto status = db->Put(rocksdb::WriteOptions(), key, value);
    return status;
}




std::string helpertools::KVStore::get(const std::string &key) {
    std::string value{""};
    rocksdb::Status s;
    s = db->Get(rocksdb::ReadOptions(), key, &value);
    return s.IsNotFound() ? string{""} : value;
}


helpertools::KVStore::~KVStore() {
    db->Close();
    delete (db);
}
