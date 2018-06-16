#include <functional> // std::bind
#include <iostream>
#include <asio.hpp>

const auto noop = std::bind([] {});

int main()
{
    using asio::ip::tcp;
    asio::io_service io_service;

    // Create all I/O objects.
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 0));
    tcp::socket socket1(io_service);
    tcp::socket socket2(io_service);

    // Connect sockets.
    acceptor.async_accept(socket1, noop);
    socket2.async_connect(acceptor.local_endpoint(), noop);
    io_service.run();
    io_service.reset();

    const std::string delimiter = "\r\n\r\n";

    // Write two commands from socket1 to socket2.
    asio::write(socket1, asio::buffer("cmd1" + delimiter));
    asio::write(socket1, asio::buffer("cmd2" + delimiter));

    // Read a single command from socket2.
    asio::streambuf streambuf;
    asio::async_read_until(socket2, streambuf, delimiter,
                           [delimiter, &streambuf](
                               const std::error_code& error_code,
                               std::size_t bytes_transferred)
    {
        // Verify streambuf contains more data beyond the delimiter. (e.g.
        // async_read_until read beyond the delimiter)
        assert(streambuf.size() > bytes_transferred);

        // Extract up to the first delimiter.
        std::string command {
            buffers_begin(streambuf.data()),
            buffers_begin(streambuf.data()) + bytes_transferred
            - delimiter.size()
        };

        // Consume through the first delimiter so that subsequent async_read_until
        // will not reiterate over the same data.
        streambuf.consume(bytes_transferred);

        assert(command == "cmd1");
        std::cout << "received command: " << command << "\n"
                  << "streambuf contains " << streambuf.size() << " bytes."
                  << " ec = " << error_code
                  << std::endl;
    });
    io_service.run();
}
