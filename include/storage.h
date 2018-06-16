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
        using records_t = std::set<Record>;
        using table_t = std::vector<records_t>;
        using names_t = std::map<std::string, records_t&>;
        Storage();

        size_t n_tables() const { return tables.size(); }

        bool insert(const std::string& table, int id, const std::string& name);

    private:
        table_t tables;
        names_t names;

        void add_table(const char* name);
};
