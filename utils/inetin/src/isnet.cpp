
#include <boost/asio.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

constexpr int TIMEOUT = 5;

class NetworkProbe :
    public std::enable_shared_from_this<NetworkProbe>
{
private:
    asio::io_context& io;
    tcp::resolver resolver;
    tcp::socket socket;
    asio::steady_timer timer;

    std::string hostname;
    int& successes;
    bool finished = false;

public:
    NetworkProbe(
        asio::io_context& context,
        const std::string& host,
        int& success_count
    )
        : io(context),
          resolver(context),
          socket(context),
          timer(context),
          hostname(host),
          successes(success_count)
    {}

    void start()
    {
        auto self = shared_from_this();

        timer.expires_after(std::chrono::seconds(TIMEOUT));

        timer.async_wait(
            [self](const boost::system::error_code& ec)
            {
                if (!ec)
                    self->finish(false);
            }
        );

        resolver.async_resolve(
            hostname,
            "443",
            [self](
                const boost::system::error_code& ec,
                tcp::resolver::results_type results
            )
            {
                if (self->finished)
                    return;

                if (ec)
                {
                    self->finish(false);
                    return;
                }

                asio::async_connect(
                    self->socket,
                    results,
                    [self](
                        const boost::system::error_code& ec,
                        const tcp::endpoint&
                    )
                    {
                        if (self->finished)
                            return;

                        self->finish(!ec);
                    }
                );
            }
        );
    }

private:
    void finish(bool success)
    {
        if (finished)
            return;

        finished = true;

        if (success)
            ++successes;

        timer.cancel();
        resolver.cancel();
        socket.cancel();
    }
};

bool CheckNetwork()
{
    const std::vector<std::string> websites =
    {
        "www.google.com",
        "www.cloudflare.com",
        "www.microsoft.com",
        "www.wikipedia.org",
        "www.github.com",
        "www.amazon.com",
        "www.mozilla.org",
        "www.debian.org",
        "www.apple.com",
        "www.kernel.org",
        "www.chatgpt.com",
        "www.stackoverflow.com",
        "www.reddit.com",
        "www.linkedin.com",
        "www.twitter.com"
    };

    try
    {
        asio::io_context io;
        int successes = 0;

        std::vector<std::shared_ptr<NetworkProbe>> probes;

        for (const auto& website : websites)
        {
            auto probe = std::make_shared<NetworkProbe>(
                io,
                website,
                successes
            );

            probes.push_back(probe);
            probe->start();
        }

        io.run();

        return successes > 0;
    }
    catch (...)
    {
        return false;
    }
}

int main()
{
    if (CheckNetwork())
    {
        std::cout << "Network Successful\n";
        return 0;
    }

    std::cout << "Network Failure\n";
    return 1;
}
