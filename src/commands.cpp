#include "commands.h"

Command::Command(const std::string& command_name, IStorage& storage)
    : name_(command_name)
    , valid_(false)
    , storage_(storage)
{
}
