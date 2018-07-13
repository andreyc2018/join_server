#pragma once

#include <string>
#include <memory>

class IResultPrinter
{
    public:
        IResultPrinter(const std::string& n) : name_(n) {}
        virtual ~IResultPrinter() {};

        virtual std::string print() const = 0;

        const std::string& name() const { return name_; }

    private:
        const std::string name_;
};

using ResultPrinterUPtr = std::unique_ptr<IResultPrinter>;

class InsertPrinter : public IResultPrinter
{
    public:
        InsertPrinter() : IResultPrinter(__func__) {}
        std::string print() const override
        {
            return "InsertPrinter";
        }
};

class TruncatePrinter : public IResultPrinter
{
    public:
        TruncatePrinter() : IResultPrinter(__func__) {}
        std::string print() const override
        {
            return "TruncatePrinter";
        }
};

class IntersectionPrinter : public IResultPrinter
{
    public:
        IntersectionPrinter() : IResultPrinter(__func__) {}
        std::string print() const override
        {
            return "IntersectionPrinter";
        }
};

class SymmetricDifferencePrinter : public IResultPrinter
{
    public:
        SymmetricDifferencePrinter() : IResultPrinter(__func__) {}
        std::string print() const override
        {
            return "SymmetricDifferencePrinter";
        }
};

class UnknownPrinter : public IResultPrinter
{
    public:
        UnknownPrinter() : IResultPrinter(__func__) {}
        std::string print() const override
        {
            return "UnknownPrinter";
        }
};
