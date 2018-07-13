#pragma once

#include "storage.h"
#include "resultprinter.h"
#include <memory>

using result_t = std::tuple<result_table_t, bool>;

class IProcessor
{
    public:
        IProcessor(IStorage& storage) : storage_(storage) {}
        virtual ~IProcessor() {}

        virtual ResultPrinterUPtr execute(const std::string& command) = 0;

    protected:
        IStorage& storage_;
};

using ProcessorUPtr = std::unique_ptr<IProcessor>;

class Processor : public IProcessor
{
    public:
        Processor(IStorage& storage);

        virtual ResultPrinterUPtr execute(const std::string& command);
};
