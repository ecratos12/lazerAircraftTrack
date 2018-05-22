//#include <iostream>
//#include <boost/array.hpp>
//#include <boost/asio.hpp>
//#include <fstream>
//#include <sstream>

#include "SearchService.h"
#include "Radar.h"
#include "SafetyManager.h"
#include "LazerOrientation.h"

#ifdef _WIN32
#include "windows.h"
#endif

using namespace boost::asio;

io_service service;
SearchService search_service(service);
Radar radar(service);
LazerOrientation lazerOrientation;
SafetyManager safetyManager(service);
ip::tcp::socket sock(service);

boost::array<char, 128> buf;
char buf_w[1] = "";

void on_read(const boost::system::error_code &ec, std::size_t bytes);
void on_write(const boost::system::error_code &ec, std::size_t bytes) 
{
	sock.async_read_some(buffer(buf), on_read);
}
void on_read(const boost::system::error_code &ec, std::size_t bytes) 
{
    if (bytes > 0) {
        std::stringstream ss;
        ss.write(buf.data(), buf.size());
        search_service.read(ss);
    }

	sock.async_write_some(buffer(buf_w,1), on_write);
}
void on_connect(const boost::system::error_code &ec) 
{
    std::cout << "Connecting" << std::endl;
	sock.async_read_some(buffer(buf), on_read);
}

int main(int argc, char* argv[])
{
    // should specify the server, port and sattelite to track
    if (argc < 4)
    {
      std::cerr << "Usage: <program-name> <server> <port> <satellite name>" << std::endl;
      return 1;
    }
    // possiblity for plotting
    if (argc > 4)
    {
        if (std::string(argv[4]) == "--visual" || std::string(argv[4]) == "-v") {
            #define PLOT_ENABLED
            // system("python plotter.py");
        }
    }

    char radarStartCmd[256];
    // using third-party ads-b decoder
    // which sends SBS formated data on specific port
#ifdef _WIN32
    strcpy(radarStartCmd, "start .\\bin\\dump1090.bat");  // run in background
#endif
#ifdef __linux__
    strcpy(radarStartCmd, "nohup bin/dump1090 --metric --raw --net --net-sbs-port ");
    strcat(radarStartCmd, argv[2]);
    strcat(radarStartCmd, " & >/dev/null 2>&1"); // silently run in background
#endif
    system(radarStartCmd);
    sleep(1);

    // make connection

    ip::tcp::endpoint ep(ip::address::from_string(argv[1]), atoi(argv[2]));
    sock.open(ep.protocol());
    sock.set_option(ip::tcp::socket::reuse_address(true));
    sock.async_connect(ep, on_connect);

    // start services

    search_service.startTracking();
    radar.attach(search_service);
    std::cout << "Radar attached" << std::endl;
    std::string sat_name(argv[3]);
    safetyManager.attach(radar, lazerOrientation, sat_name);
    service.run();

    // stop services

    std::cout << "Closing connection" << std::endl;
    sock.close();
    safetyManager.stop();
    radar.stop();
    search_service.stopTracking();
    service.stop();

  return 0;
}
