#include "server.h"
#include "logger.h"
#include <spdlog/fmt/ostr.h>
#include <iostream>

using asio::ip::tcp;

Session::Session(tcp::socket socket)
    : socket_(std::move(socket))
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

            if (command == "long") {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                self->reply_.clear();
                self->reply_.resize(10240, '.');
                self->do_write();
            }

            gLogger->debug("  received command: {},"
                           " streambuf contains {} bytes. ec = {}",
                           command, self->streambuf_.size(), error_code);
            self->prompt();
        }
    });
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

Server::Server(asio::io_service& io_service, short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    , socket_(io_service)
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
            std::make_shared<Session>(std::move(socket_))->start();
        }

        do_accept();
    });
}
