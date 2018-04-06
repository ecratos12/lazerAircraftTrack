//#include <iostream>
//#include <boost/array.hpp>
//#include <boost/asio.hpp>
//#include <fstream>
//#include <sstream>

#include "SearchService.h"
#include "Radar.h"
#include "SafetyManager.h"

using namespace boost::asio;

int main(int argc, char* argv[])
{
  try
  {
    // should specify the server - the 2nd argument, and port - the 3rd argument
    if (argc != 3)
    {
      std::cerr << "Usage: <program-name> <server> <port> " << std::endl;
      return 1;
    }

    char radarStartCmd[256];

    // using third-party ads-b decoder
    // send SBS formated data on specific port
#ifdef _WIN32
    strcpy(radarStartCmd, "start ");  // run in background
#endif
    strcpy(radarStartCmd, "nohup ./bin/dump1090 --metric --raw --net --net-sbs-port ");
    strcat(radarStartCmd, argv[2]);
#ifdef __linux__
    strcat(radarStartCmd, " & >/dev/null 2>&1"); // silently run in background
#endif
    system(radarStartCmd);
    sleep(2); // Decoder initialization

    io_service service;

    // make connection

    ip::tcp::endpoint ep(ip::address::from_string(argv[1]), atoi(argv[2]));
    ip::tcp::socket sock(service);
    sock.open(ep.protocol());
    sock.set_option(ip::tcp::socket::reuse_address(true));
    sock.connect(ep);

    // start services

    SearchService search_service(service);
    search_service.startTracking();

    Radar radar(service);
    radar.attach(search_service);
    std::cout << "Radar attached" << std::endl;

    LazerOrientation lazerOrientation;

    // std::string some_string_sat_name("sat_name");
    // SafetyManager safetyManager(service);
    // safetyManager.attach(radar, lazerOrientation, some_string_sat_name);

    service.run();

    for (;;)
    {
        // this buffer prevents self-overflow
        boost::array<char, 128> buf;
        boost::system::error_code ec;

        size_t len = sock.read_some(boost::asio::buffer(buf), ec);

        if (ec == boost::asio::error::eof)
            break;
        else if (ec)
            throw boost::system::system_error(ec);

        std::stringstream ss;
        ss.write(buf.data(), len);
        search_service.read(ss);
    }
    std::cout << "Closing connection" << std::endl;

    sock.close();
    // safetyManager.stop();
    radar.stop();
    search_service.stopTracking();
  }
  // handle any exceptions that may have been thrown.
  catch (std::exception& e)
  {
    std::cerr << "Terminated by error : " << e.what() << std::endl;
  }

  return 0;

}
