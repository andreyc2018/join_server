#include "async.h"
#include "logger.h"
#include <asio.hpp>

#include <iostream>
#include <string>
#include <thread>

using asio::ip::tcp;

/**
 * @brief Process the data
 */
class session
    : public std::enable_shared_from_this<session>
{
    public:
        session(tcp::socket socket, async::handle_t h)
            : socket_(std::move(socket))
            , h_(h)
        {
        }

        ~session()
        {
//            std::cout << "eos\n";
            async::disconnect(h_);
        }

        void start()
        {
            do_read();
        }

    private:
        void do_read()
        {
            auto self(shared_from_this());
            socket_.async_read_some(asio::buffer(data_, max_length),
                                    [this, self](std::error_code ec, std::size_t length)
            {
                if (!ec) {
//                    std::cout << std::this_thread::get_id()
//                              << " " << (void*)this
//                              << " " << length << "\n";
                    async::receive(h_, data_, length);
                    do_read();
                }
            });
        }

//        void do_write(std::size_t length)
//        {
//            auto self(shared_from_this());
//            asio::async_write(socket_, asio::buffer(data_, length),
//                                     [this, self](std::error_code ec, std::size_t /*length*/)
//            {
//                if (!ec) {
//                    do_read();
//                }
//            });
//        }

        tcp::socket socket_;
        async::handle_t h_;

        enum { max_length = 1024 };

        char data_[max_length];
};

/**
 * @brief Accepts incomming connections
 */
class server
{
    public:
        server(asio::io_service& io_service, short port, size_t bulk)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
            , socket_(io_service)
            , bulk_(bulk)
        {
            do_accept();
        }

    private:
        void do_accept()
        {
            acceptor_.async_accept(socket_,
                                   [this](std::error_code ec)
            {
                if (!ec) {
                    async::handle_t h = async::connect(bulk_);
                    std::make_shared<session>(std::move(socket_), h)->start();
                }

                do_accept();
            });
        }

        tcp::acceptor acceptor_;
        tcp::socket socket_;
        size_t bulk_;
};

/**
 * @brief Terminates io service
 */
class SignalHandler
{
    public:
        SignalHandler(asio::io_service& io_service)
            : io_service_(io_service) {}

        void operator()(const std::error_code&, int)
        {
            io_service_.stop();
        }

    private:
        asio::io_service& io_service_;
};

int main(int argc, char const** argv)
{
    try {
        if (argc < 3) {
            std::cout << "usage: "
                      << std::string(argv[0]).substr(std::string(argv[0]).rfind("/") + 1)
                      << " <port> <N>\n"
                         "where:\n"
                         "  port - tcp port for incomming connections\n"
                         "  N    - command block size\n"
                         "\nUse Ctrl-C to stop the service.\n";
            exit(1);
        }

        if (argc > 3) {
            gLogger->set_level(spdlog::level::debug);
        }

        asio::io_service io_service;

        asio::signal_set signals(io_service, SIGINT, SIGTERM);
        SignalHandler sh(io_service);
        signals.async_wait(sh);

        server s(io_service, std::atoi(argv[1]), std::atoi(argv[2]));

        io_service.run();

        std::cout << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

