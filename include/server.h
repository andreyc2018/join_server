#pragma once

#include <asio.hpp>

class Session
    : public std::enable_shared_from_this<Session>
{
    public:
        Session(asio::ip::tcp::socket socket);
        ~Session();

        void start();

    private:
        void do_read();

        void do_write(std::size_t length);

        asio::ip::tcp::socket socket_;

        enum { max_length = 1024 };

        char data_[max_length];
};

class Server
{
    public:
        Server(asio::io_service& io_service, short port);

    private:
        void do_accept();

        asio::ip::tcp::acceptor acceptor_;
        asio::ip::tcp::socket socket_;
};
