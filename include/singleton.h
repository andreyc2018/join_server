#pragma once

template <typename T>
class Singleton
{
    public:
        static T& instance()
        {
            static T the_instance;
            return the_instance;
        }

    private:
        Singleton() = default;
        Singleton(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton& operator=(Singleton&&) = delete;
};
