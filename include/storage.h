#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <tuple>
#include <stdexcept>
#include <initializer_list>

struct Field
{
    enum class type_e { INTEGER, STRING };

    type_e type;
    std::string value;

    Field(int v) : type(type_e::INTEGER), value(std::to_string(v)) {}
    Field(std::string v) : type(type_e::STRING), value(v) {}
    Field(const Field& f) : type(f.type), value(f.value) {}

    Field& operator=(const Field& f) {
        type = f.type;
        value = f.value;
        return *this;
    }

    friend bool operator==(int l, const Field& r) {
        if (r.type != type_e::INTEGER)
            throw std::logic_error("Unable to compare diferent types");
        return l == std::stoi(r.value);
    }

    friend bool operator==(const Field& l, int r) {
        return r == l;
    }

    friend bool operator<(int l, const Field& r) {
        if (r.type != type_e::INTEGER)
            throw std::logic_error("Unable to compare diferent types");
        return l < std::stoi(r.value);
    }

    friend bool operator<(const Field& l, int r) {
        return r < l;
    }

    friend bool operator<(const std::string& l, const Field& r) {
        if (r.type != type_e::STRING)
            throw std::logic_error("Unable to compare diferent types");
        return l < r.value;
    }

    friend bool operator<(const Field& l, const std::string& r) {
        return r < l;
    }

    friend bool operator<(const Field& l, const Field& r) {
        return l.value < r;
    }
};

struct Record
{
    size_t index_field = 0;
    std::vector<Field> fields;
    std::vector<std::string> names;

    Record(std::initializer_list<Field> l) : fields(l) {
        if (fields.size() < 1) {
            throw std::logic_error("Record must have at least one field.");
        }
    }

    friend bool operator<(const Record& l, const Record& r) {
        return l.fields[l.index_field] < r.fields[r.index_field];
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
        bool truncate(const std::string& table);
        result_table_t intersection();
        result_table_t symmetric_difference();

    private:
        tables_t tables_;
        names_t names_;

        void add_table(const char* name);
        std::tuple<std::string, bool> find_name(table_t& data, int key);
        void extract_keys(keys_tables_t& table_keys);
        void fill_result(keys_table_t& keys_ab, result_table_t& result);
};
