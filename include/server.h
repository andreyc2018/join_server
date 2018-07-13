#pragma once

#include "processor.h"
#include <asio.hpp>

class IStorage;
class IProcessor;

class Session
    : public std::enable_shared_from_this<Session>
{
    public:
        Session(asio::ip::tcp::socket socket, IStorage& storage);
        ~Session();

        void start();

    private:
        void do_read();
        void do_write();

        asio::ip::tcp::socket socket_;
        enum { max_length = 8192 };

        /// Buffer for incoming data.
        std::array<char, max_length> buffer_;
        /// The reply to be sent back to the client.
        std::string reply_;
        asio::streambuf streambuf_;
        void prompt();

        ProcessorUPtr processor;
};

class Server
{
    public:
        Server(asio::io_service& io_service,
               short port, IStorage& storage);

    private:
        void do_accept();

        asio::ip::tcp::acceptor acceptor_;
        asio::ip::tcp::socket socket_;
        std::shared_ptr<Session> session_;

        IStorage& storage_;
};
