#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>

struct Record;
struct ResultRecord;

using table_t = std::set<Record>;
using tables_t = std::vector<table_t>;
using result_table_t = std::set<ResultRecord>;
using names_t = std::map<std::string, table_t&>;
using keys_table_t = std::vector<int>;
using keys_tables_t = std::vector<keys_table_t>;
using lock_t = std::lock_guard<std::mutex>;

struct Record
{
    int id;
    std::string name;

    Record(int key, const char* value) : id(key), name(value) {}
    Record(int key, const std::string& value) : id(key), name(value) {}

    friend bool operator<(const Record& l, const Record& r) {
        return l.id < r.id;
    }
};

struct ResultRecord
{
    int id;
    std::vector<std::string> fields;

    ResultRecord() = delete;
    ResultRecord(size_t n) : id(0), fields(n) {}

    friend bool operator<(const ResultRecord& l, const ResultRecord& r) {
        return l.id < r.id;
    }
};

class IStorage
{
    public:
        virtual ~IStorage() {}

        virtual size_t n_tables() const = 0;

        // INSERT table id name
        virtual bool insert(const std::string& table,
                            int id, const std::string& name) = 0;
        // TRUNCATE table
        virtual bool truncate(const std::string& table) = 0;
        // INTERSECTION
        virtual result_table_t intersection() const = 0;
        // SYMMETRIC_DIFFERENCE
        virtual result_table_t symmetric_difference() const = 0;
};

class Storage : public IStorage
{
    public:
        Storage();

        size_t n_tables() const override { return tables_.size(); }

        bool insert(const std::string& table,
                    int id, const std::string& name) override;
        bool truncate(const std::string& table) override;
        result_table_t intersection() const override;
        result_table_t symmetric_difference() const override;

    private:
        tables_t tables_;
        names_t names_;
        mutable std::mutex m_;

        void add_table(const char* name);
        std::tuple<std::string, bool> find_name(const table_t& data, int key) const;
        void extract_keys(keys_tables_t& table_keys) const;
        void fill_result(keys_table_t& keys_ab, result_table_t& result) const;
};
