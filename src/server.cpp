#include "server.h"
#include "logger.h"
#include "storage.h"
#include "processor.h"
#include <spdlog/fmt/ostr.h>
#include <iostream>

using asio::ip::tcp;

Session::Session(tcp::socket socket, IStorage& storage)
    : socket_(std::move(socket))
    , processor(std::make_unique<Processor>(storage))
{
}

Session::~Session()
{
    gLogger->debug("END: session = {}",
                   static_cast<void*>(this));
}

void Session::prompt()
{
    reply_ = "> ";
    do_write();
}

void Session::start()
{
    gLogger->debug("START: session = {}",
                   static_cast<void*>(this));

    prompt();
    do_read();
}

void Session::do_read()
{
//    auto self(shared_from_this());

    const std::string delimiter = "\n";

    gLogger->debug("before read_until streambuf contains {} bytes.",
                   streambuf_.size());
    asio::async_read_until(socket_, streambuf_, delimiter,
                           [delimiter, this](const std::error_code& error_code,
                                                   std::size_t bytes_transferred)
    {
        gLogger->debug("session = {} streambuf contains {} bytes. "
                       "bytes transferred = {}",
                       static_cast<void*>(this), this->streambuf_.size(),
                       bytes_transferred);

        if (bytes_transferred > 1) {
            // Extract up to the first delimiter.
            std::string command {
                buffers_begin(this->streambuf_.data()),
                        buffers_begin(this->streambuf_.data())
                        + bytes_transferred - delimiter.size()
            };

            // Consume through the first delimiter so that subsequent async_read_until
            // will not reiterate over the same data.
            this->streambuf_.consume(bytes_transferred);

            ResultPrinterUPtr result = this->processor->execute(command);
            this->reply_ = result->print();
            this->reply_.append("\n");
            this->do_write();

            gLogger->debug("  received command: {},"
                           " streambuf contains {} bytes. ec = {}",
                           command, this->streambuf_.size(), error_code);
            this->prompt();
        }
        else {
            this->streambuf_.consume(bytes_transferred);
        }
    });
    gLogger->debug("after read_until streambuf contains {} bytes.",
                   streambuf_.size());
}

void Session::do_write()
{
//    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(reply_, reply_.size()),
                      [this](std::error_code ec,
                      std::size_t length)
    {
        if (!ec) {
            this->reply_.clear();
            gLogger->debug("write output: session = {} length = {}",
                           static_cast<void*>(this), length);
            do_read();
        }
    });
}

Server::Server(asio::io_service& io_service, short port, IStorage& storage)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    , socket_(io_service)
    , storage_(storage)
{
    do_accept();
}

void Server::do_accept()
{
    acceptor_.async_accept(socket_,
                           [this](std::error_code ec)
    {
        if (!ec) {
            gLogger->debug("accepted new connection: server = {}",
                           static_cast<void*>(this));
            session_.push_back(std::make_shared<Session>(std::move(socket_), storage_));
            session_.back()->start();
        }

        do_accept();
    });
}
