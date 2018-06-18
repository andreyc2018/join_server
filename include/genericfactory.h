#pragma once

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <boost/algorithm/string.hpp>

template <typename B>
class Creator
{
    public:
        Creator(const std::string& key);
        virtual B* create() = 0;
};

template <typename T, typename B>
class CreatorImpl : public Creator<B>
{
    public:
        CreatorImpl(const std::string& key) : Creator<B>(key) {}
        virtual B* create() { return new T; }
};

template <typename B>
class Factory
{
    public:
        Factory() = delete;

        using ClassCreator = Creator<B>;
        using Registry = std::map<std::string, ClassCreator*>;
        using Names = std::vector<std::string>;

        static B* create(const std::string& key)
        {
            auto it = getTable().find(key);
            assert(it != getTable().end());
            return it->second->create();
        }

        static void registerRequestClass(const std::string& key,
                                         ClassCreator* creator)
        {
            getTable()[key] = creator;
        }

        static Names listNames()
        {
            Names names;
            for (auto const& record: getTable()) {
                names.push_back(record.first);
            }
            return names;
        }

    private:
        static Registry& getTable()
        {
            static Registry table;
            return table;
        }
};

template<typename B>
Creator<B>::Creator(const std::string& key)
{
    Factory<B>::registerRequestClass(key, this);
}

#define REGISTER(classname, base)       \
    public:                             \
        static const std::string registryKey; \
    private:                            \
        static const CreatorImpl<classname, base> creator

#define REGISTER_IMPL(classname, base)     \
    const std::string classname::registryKey = # classname; \
    const CreatorImpl<classname, base> classname::creator(# classname)

#define REGISTER_IMPL_UPPER(classname, base)     \
    const std::string classname::registryKey = boost::to_upper_copy<std::string>(# classname); \
    const CreatorImpl<classname, base> classname::creator(boost::to_upper_copy<std::string>(# classname))

#define REGISTER_IMPL_KEY(key, classname, base)     \
    const std::string classname::registryKey = # key; \
    const CreatorImpl<classname, base> classname::creator(# key)
