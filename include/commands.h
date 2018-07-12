#pragma once

#include <string>
#include <memory>

class IStorage;

class Command
{
    public:
        Command(const std::string& command_name, IStorage& storage);
        virtual ~Command() {}

        virtual int run() = 0;

        const std::string& name() const { return name_; }

    private:
        const std::string name_;

    protected:
        bool valid_;
        IStorage& storage_;
};

using CommandUPtr = std::unique_ptr<Command>;

class Insert : public Command
{
    public:
        Insert(IStorage& storage)
            : Command("Insert", storage) { valid_ = true; }

        int run() override
        {
            return 1;
        }

        void setTable(const std::string& table) { table_ = table; }
        void setId(int id) { id_ = id; }
        void setValue(const std::string& value) { value_ = value; }

    private:
        std::string table_;
        int id_;
        std::string value_;
};

class Truncate : public Command
{
    public:
        Truncate(IStorage& storage)
            : Command("Truncate", storage) { valid_ = true; }

        int run() override
        {
            return 2;
        }
};

class Intersection : public Command
{
    public:
        Intersection(IStorage& storage)
            : Command("Intersection", storage) { valid_ = true; }

        int run() override
        {
            return 3;
        }
};

class Symmetric_Difference : public Command
{
    public:
        Symmetric_Difference(IStorage& storage)
            : Command("Symmetric_Difference", storage) { valid_ = true; }

        int run() override
        {
            return 4;
        }
};

class Unknown : public Command
{
    public:
        Unknown(IStorage& storage)
            : Command("Unknown", storage) {}

        int run() override
        {
            return 3;
        }
};

class CommandFactory
{
    public:
        static CommandUPtr create(const std::string& cmd_str,
                                  IStorage& storage)
        {
            if (cmd_str == "INSERT") {
                return std::make_unique<Insert>(storage);
            }
            else if (cmd_str == "TRUNCATE") {
                return std::make_unique<Truncate>(storage);
            }
            return std::make_unique<Unknown>(storage);
        }
};
