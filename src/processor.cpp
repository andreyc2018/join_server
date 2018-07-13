#include "processor.h"
#include "commands.h"

Processor::Processor(IStorage& storage)
    : IProcessor(storage)
{

}

ResultPrinterUPtr Processor::execute(const std::string& command)
{
    CommandUPtr cmd = CommandFactory::create(command, storage_);
    return cmd->run();
}
