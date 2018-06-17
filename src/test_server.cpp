#include "storage.h"
#include <algorithm>
#include <iterator>
#include <gtest/gtest.h>

TEST(Table_Test, Insert_Records)
{
    std::set<Record> data;

    auto rc = data.emplace(0, "hello");

    EXPECT_TRUE(rc.second);

    rc = data.emplace(0, "world");

    EXPECT_FALSE(rc.second);
    EXPECT_EQ("hello", rc.first->name);

    rc = data.emplace(1, "world");

    EXPECT_TRUE(rc.second);
    EXPECT_EQ("world", rc.first->name);
}

std::tuple<std::string, bool> find_name(std::set<Record>& data, int key)
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

TEST(Table_Test, Set_Operations)
{
    std::set<Record> data_a {
         { 0, "lean" },
         { 1, "sweater" },
         { 2, "frank" },
         { 3, "violation" },
         { 4, "quality" },
         { 5, "precision" }
    } ;

    std::set<Record> data_b {
         { 3, "proposal" },
         { 4, "example" },
         { 5, "lake" },
         { 6, "flour" },
         { 7, "wonder" },
         { 8, "selection" }
    } ;

    std::vector<std::set<Record>> tables;
    tables.push_back(data_a);
    tables.push_back(data_b);

    std::vector<std::vector<int>> table_keys;

    for (auto& t : tables) {
        std::vector<int> keys;
        transform(std::begin(t), std::end(t), back_inserter(keys),
                  [](const Record& rec) { return rec.id; });
        table_keys.push_back(keys);
    }

    std::vector<int> keys_ab;
    std::set_intersection(std::begin(table_keys[0]), std::end(table_keys[0]),
                          std::begin(table_keys[1]), std::end(table_keys[1]),
                          std::back_inserter(keys_ab));

    std::set<ResultRecord> result;

    for (const auto& key : keys_ab) {
        ResultRecord rr(2);
        for (size_t i = 0; i < tables.size(); ++i) {
            bool found;
            std::string name;
            std::tie(name, found) = find_name(tables[i], key);
            if (found) {
                rr.id = key;
                rr.fields[i] = name;
            }
        }
        result.insert(rr);
    }

    for (const auto& r : result) {
        std::cout << r.id << ", " << r.fields[0] << ", " << r.fields[1] << "\n";
    }

    result.clear();
    keys_ab.clear();

    std::set_symmetric_difference(
                std::begin(table_keys[0]), std::end(table_keys[0]),
                std::begin(table_keys[1]), std::end(table_keys[1]),
                std::back_inserter(keys_ab));

    for (const auto& key : keys_ab) {
        ResultRecord rr(2);
        for (size_t i = 0; i < tables.size(); ++i) {
            bool found;
            std::string name;
            std::tie(name, found) = find_name(tables[i], key);
            if (found) {
                rr.id = key;
                rr.fields[i] = name;
            }
        }
        result.insert(rr);
    }

    for (const auto& r : result) {
        std::cout << r.id << ", " << r.fields[0] << ", " << r.fields[1] << "\n";
    }
}

TEST(Storage_Test, Init)
{
    Storage s;
    EXPECT_EQ(2, s.n_tables());

    EXPECT_TRUE(s.insert("A", 0, "name"));
    EXPECT_FALSE(s.insert("A", 0, "another"));

    EXPECT_TRUE(s.insert("B", 0, "name"));
    EXPECT_FALSE(s.insert("B", 0, "another"));

    EXPECT_FALSE(s.insert("C", 0, "another"));
}
