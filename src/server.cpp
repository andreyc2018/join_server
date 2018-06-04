#include "server.h"
#include "logger.h"
#include <spdlog/fmt/ostr.h>

using asio::ip::tcp;

Session::Session(tcp::socket socket)
    : socket_(std::move(socket))
{
}

Session::~Session()
{
    gLogger->debug("eos");
}

void Session::start()
{
    do_read();
}

void Session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(data_, max_length),
                            [this, self](std::error_code ec,
                            std::size_t length)
    {
        if (!ec) {
            gLogger->debug("read input: {} {} {}",
                           std::this_thread::get_id(),
                           static_cast<const void*>(this), length);
            do_read();
        }
    });
}

void Session::do_write(std::size_t length)
{
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(data_, length),
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
