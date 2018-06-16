#include "storage.h"

Storage::Storage()
{
    add_table("A");
    add_table("B");
}

bool Storage::insert(const std::string& table, int id, const std::string& name)
{
    auto rc = names[table].emplace(id, name);
    return rc.second;
}

void Storage::add_table(const char* name)
{
    tables.emplace_back(records_t());
    names.emplace(name, tables.back());
}
