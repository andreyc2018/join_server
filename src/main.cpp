#include "logger.h"
#include "server.h"
#include "storage.h"

#include <iostream>
#include <string>
#include <thread>

/**
 * @brief Terminates io service
 */
class SignalHandler
{
    public:
        SignalHandler(asio::io_service& io_service)
            : io_service_(io_service) {}

        void operator()(const std::error_code&, int signum)
        {
            gLogger->debug("End of service requested by user. {}.", signum);
            io_service_.stop();
        }

    private:
        asio::io_service& io_service_;
};

int main(int argc, char const** argv)
{
    try {
        if (argc < 2) {
            std::cout << "usage: "
                      << std::string(argv[0]).substr(std::string(argv[0]).rfind("/") + 1)
                      << " <port> [d]\n"
                         "where:\n"
                         "  port - tcp port for incomming connections\n"
                         "  d - print debug info\n"
                         "\nUse Ctrl-C to stop the service.\n";
            exit(1);
        }

        if (argc > 2) {
            gLogger->set_level(spdlog::level::debug);
        }

        asio::io_service io_service;

        asio::signal_set signals(io_service, SIGINT, SIGTERM);
        SignalHandler sh(io_service);
        signals.async_wait(sh);

        Storage db;
        Server s(io_service, std::atoi(argv[1]), db);

        io_service.run();

        std::cout << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
