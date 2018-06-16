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
    gLogger->debug("eos");
}

void Session::prompt()
{
    reply_ = "> ";
    do_write();
}

void Session::start()
{
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
        std::cout << "streambuf contains " << self->streambuf_.size() << " bytes."
                  << " bytes = " << bytes_transferred
                  << std::endl;

        if (bytes_transferred > 0) {
            // Extract up to the first delimiter.
            std::string command {
                buffers_begin(self->streambuf_.data()),
                        buffers_begin(self->streambuf_.data()) + bytes_transferred
                        - delimiter.size()
            };

            // Consume through the first delimiter so that subsequent async_read_until
            // will not reiterate over the same data.
            self->streambuf_.consume(bytes_transferred);

            std::cout << "received command: " << command << "\n"
                      << "streambuf contains " << self->streambuf_.size() << " bytes."
                      << " ec = " << error_code
                      << std::endl;
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
            gLogger->debug("write output: {} {} {}",
                           std::this_thread::get_id(),
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
            gLogger->debug("starting new session: {} {}",
                           std::this_thread::get_id(),
                           static_cast<void*>(this));
            std::make_shared<Session>(std::move(socket_))->start();
        }

        do_accept();
    });
}
