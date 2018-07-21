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
    auto self(shared_from_this());

    const std::string delimiter = "\n";

    gLogger->debug("before read_until streambuf contains {} bytes.",
                   self->streambuf_.size());
    asio::async_read_until(socket_, streambuf_, delimiter,
                           [delimiter, self](const std::error_code& error_code,
                                                   std::size_t bytes_transferred)
    {
        gLogger->debug("streambuf contains {} bytes. bytes transferred = {}",
                       self->streambuf_.size(), bytes_transferred);

        if (bytes_transferred > 1) {
            // Extract up to the first delimiter.
            std::string command {
                buffers_begin(self->streambuf_.data()),
                        buffers_begin(self->streambuf_.data()) + bytes_transferred
                        - delimiter.size()
            };

            // Consume through the first delimiter so that subsequent async_read_until
            // will not reiterate over the same data.
            self->streambuf_.consume(bytes_transferred);

            ResultPrinterUPtr result = self->processor->execute(command);
            self->reply_ = result->print();
            self->do_write();

            gLogger->debug("  received command: {},"
                           " streambuf contains {} bytes. ec = {}",
                           command, self->streambuf_.size(), error_code);
            self->prompt();
        }
        else {
            self->streambuf_.consume(bytes_transferred);
        }
    });
    gLogger->debug("after read_until streambuf contains {} bytes.",
                   self->streambuf_.size());
}

void Session::do_write()
{
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(reply_, reply_.size()),
                      [this, self](std::error_code ec,
                      std::size_t length)
    {
        if (!ec) {
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
            std::make_shared<Session>(std::move(socket_), storage_)->start();
        }

        do_accept();
    });
}
