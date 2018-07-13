#include "storage.h"
#include "interpreter.h"
#include "processor.h"
#include "commands.h"
#include <algorithm>
#include <iterator>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

class MockStorage : public IStorage
{
    public:
        MockStorage() {}

        MOCK_CONST_METHOD0(n_tables, size_t());
        MOCK_METHOD3(insert, bool(const std::string&,
                                  int, const std::string&));
        MOCK_METHOD1(truncate, bool(const std::string&));
        MOCK_CONST_METHOD0(intersection, result_table_t());
        MOCK_CONST_METHOD0(symmetric_difference, result_table_t());
};

TEST(CommandFactory, Factory)
{
    MockStorage storage;
    CommandUPtr cmd_1(CommandFactory::create("INSERT", storage));
    CommandUPtr cmd_2(CommandFactory::create("TRUNCATE", storage));

    EXPECT_TRUE(bool(cmd_1));
    std::string result = cmd_1->run()->name();
    EXPECT_EQ("InsertPrinter", result);

    EXPECT_TRUE(bool(cmd_2));
    result = cmd_2->run()->name();
    EXPECT_EQ("TruncatePrinter", result);
}

TEST(Interpreter, Expressions)
{
    expr_t command_kw = std::make_shared<term_t>("INSERT|"
                                                 "TRUNCATE|"
                                                 "INTERSECTION|"
                                                 "SYMMETRIC_DIFFERENCE");
    expr_t table_name_kw = std::make_shared<term_t>("[AB]");
    expr_t id_field_kw = std::make_shared<term_t>("[0-9]+");
    expr_t name_field_kw = std::make_shared<term_t>("[a-z]+");

    EXPECT_TRUE(command_kw->interpret("INSERT"));
    EXPECT_FALSE(command_kw->interpret("insert"));
    EXPECT_TRUE(command_kw->interpret("TRUNCATE"));
    EXPECT_TRUE(command_kw->interpret("INTERSECTION"));
    EXPECT_TRUE(command_kw->interpret("SYMMETRIC_DIFFERENCE"));
    EXPECT_FALSE(command_kw->interpret("SYMMETRIC"));

    EXPECT_TRUE(table_name_kw->interpret("A"));
    EXPECT_TRUE(table_name_kw->interpret("B"));
    EXPECT_FALSE(table_name_kw->interpret("randomname"));

    EXPECT_FALSE(id_field_kw->interpret("A"));
    EXPECT_TRUE(id_field_kw->interpret("088"));

    EXPECT_FALSE(name_field_kw->interpret("{088}"));
    EXPECT_TRUE(name_field_kw->interpret("wonder"));
}
