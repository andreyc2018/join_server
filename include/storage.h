#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

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

class Storage
{
    public:
        using table_t = std::set<Record>;
        using tables_t = std::vector<table_t>;
        using result_table_t = std::set<ResultRecord>;
        using names_t = std::map<std::string, table_t&>;
        using keys_table_t = std::vector<int>;
        using keys_tables_t = std::vector<keys_table_t>;

        Storage();

        size_t n_tables() const { return tables_.size(); }

        bool insert(const std::string& table, int id, const std::string& name);
        void truncate(const std::string& table);
        void intersection();
        void symmetric_difference();

    private:
        tables_t tables_;
        names_t names_;

        void add_table(const char* name);
        std::tuple<std::string, bool> find_name(table_t& data, int key);
        void extract_keys(keys_tables_t& table_keys);
        void fill_result(keys_table_t& keys_ab, result_table_t& result);
};
