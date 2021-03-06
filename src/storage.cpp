#include "storage.h"
#include <algorithm>
#include <iterator>

Storage::Storage()
{
    add_table("A");
    add_table("B");
}

bool Storage::insert(const std::string& table, int id, const std::string& name)
{
    lock_t lock(m_);
    auto found = names_.find(table);
    if (found != names_.end()) {
        auto rc = found->second.emplace(id, name);
        return rc.second;
    }
    return false;
}

bool Storage::truncate(const std::string& table)
{
    lock_t lock(m_);
    auto found = names_.find(table);
    if (found != names_.end()) {
        found->second.clear();
        return true;
    }
    return false;
}

result_table_t Storage::intersection() const
{
    lock_t lock(m_);
    keys_tables_t table_keys;
    extract_keys(table_keys);

    keys_table_t keys_ab;
    std::set_intersection(std::begin(table_keys[0]), std::end(table_keys[0]),
                          std::begin(table_keys[1]), std::end(table_keys[1]),
                          std::back_inserter(keys_ab));

    result_table_t result;
    fill_result(keys_ab, result);
    return result;
}

result_table_t Storage::symmetric_difference() const
{
    lock_t lock(m_);
    keys_tables_t table_keys;
    extract_keys(table_keys);

    keys_table_t keys_ab;
    std::set_symmetric_difference(
                std::begin(table_keys[0]), std::end(table_keys[0]),
                std::begin(table_keys[1]), std::end(table_keys[1]),
                std::back_inserter(keys_ab));

    result_table_t result;
    fill_result(keys_ab, result);
    return result;
}

void Storage::add_table(const char* name)
{
    tables_.emplace_back(table_t());
    names_.emplace(name, tables_.back());
}

std::tuple<std::string, bool> Storage::find_name(const table_t& data, int key) const
{
    auto found = std::find_if(std::begin(data), std::end(data),
                              [&key](const Record& rec)
    {
        return rec.id == key;
    });
    if (found != std::end(data)) {
        return std::make_tuple(found->name, true);
    }
    return std::make_tuple("", false);
}

void Storage::extract_keys(keys_tables_t& table_keys) const
{
    for (const auto& t : tables_) {
        keys_table_t keys;
        transform(std::begin(t), std::end(t), back_inserter(keys),
                  [](const Record& rec) { return rec.id; });
        table_keys.push_back(keys);
    }
}

void Storage::fill_result(keys_table_t& keys_ab,
                          result_table_t& result) const
{
    for (const auto& key : keys_ab) {
        ResultRecord rr(2);
        for (size_t i = 0; i < tables_.size(); ++i) {
            bool found;
            std::string name;
            std::tie(name, found) = find_name(tables_[i], key);
            if (found) {
                rr.id = key;
                rr.fields[i] = name;
            }
        }
        result.insert(rr);
    }
}
