#pragma once

#include "storage.h"

using result_t = std::tuple<result_table_t, bool>;
class IProcessor
{
    public:
        virtual ~IProcessor() {}

        virtual result_t execute(const std::string& command) = 0;
};

class Processor : public IProcessor
{
    public:
        Processor();

        virtual result_t execute(const std::string& command);
};
