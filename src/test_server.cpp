#include "storage.h"
#include "genericfactory.h"
#include "interpreter.h"
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

struct Param
{
    Param(const std::string& n, const std::string v)
        : name(n), value(v) {}
    std::string name;
    std::string value;
};

class Command
{
    public:
        using params_t = std::vector<Param>;
        Command(const std::string& class_name);
        virtual ~Command() {}

        virtual int run() = 0;

        const std::string& name() const { return name_; }

        void add_param(const std::string& name, const std::string value)
        {
            params.emplace_back(name, value);
        }

    private:
        const std::string name_;
        params_t params;
};

Command::Command(const std::string& class_name)
    : name_(class_name)
{
}

using CommandUPtr = std::unique_ptr<Command>;
using CommandFactory = Factory<Command>;

class Insert : public Command
{
        REGISTER(Insert, Command);
    public:
        Insert() : Command("Insert") {}

        int run() override
        {
            return 1;
        }
};

REGISTER_IMPL_UPPER(Insert, Command);

class Truncate : public Command
{
        REGISTER(Truncate, Command);
    public:
        Truncate() : Command("Truncate") {}

        int run() override
        {
            return 2;
        }
};

REGISTER_IMPL_UPPER(Truncate, Command);

class Intersection : public Command
{
        REGISTER(Intersection, Command);
    public:
        Intersection() : Command("Intersection") {}

        int run() override
        {
            return 3;
        }
};

REGISTER_IMPL_UPPER(Intersection, Command);

class Symmetric_Difference : public Command
{
        REGISTER(Symmetric_Difference, Command);
    public:
        Symmetric_Difference() : Command("Symmetric_Difference") {}

        int run() override
        {
            return 4;
        }
};

REGISTER_IMPL_UPPER(Symmetric_Difference, Command);

//class CommandStringParser
//{
//    public:
//        CommandUPtr parse(const std::string& cmd)
//        {
//        }
//};

TEST(CommandFactory, List_Registered_Success)
{
    CommandFactory::Names list = CommandFactory::listNames();
    std::set<std::string> expected { "INSERT", "TRUNCATE",
                                     "INTERSECTION", "SYMMETRIC_DIFFERENCE" };

    for (const auto& n : list) {
        std::cout << "got name: " << n << "\n";
        auto found = expected.find(n);
        EXPECT_TRUE(found != expected.end());
    }
}

TEST(CommandFactory, Factory)
{
    CommandUPtr cmd_1(CommandFactory::create("INSERT"));
    CommandUPtr cmd_2(CommandFactory::create("TRUNCATE"));

    EXPECT_TRUE(bool(cmd_1));
    EXPECT_EQ(1, cmd_1->run());

    EXPECT_TRUE(bool(cmd_2));
    EXPECT_EQ(2, cmd_2->run());
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
