//#include <iostream>
//#include <boost/array.hpp>
//#include <boost/asio.hpp>
//#include <fstream>
//#include <sstream>
#include "SearchService.h"
#include "Radar.h"

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
    strcpy(radarStartCmd, "../dump1090-original/dump1090 --metric --raw --net-sbs-port ");
    strcat(radarStartCmd, argv[2]);
    system(radarStartCmd);

    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string(argv[1]), atoi(argv[2]));
    ip::tcp::socket sock(service);
    boost::asio::connect(sock, &ep);

    SearchService search_service(service);
    search_service.startTracking();

    Radar radar(service);
    radar.connectToSearchService(search_service);

    for (;;)
    {
      service.run();
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
    
  }
  // handle any exceptions that may have been thrown.
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;

}
