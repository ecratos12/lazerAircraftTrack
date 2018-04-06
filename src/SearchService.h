#ifndef SEARCH_SERVICE_H
#define SEARCH_SERVICE_H

#define AIRCRAFT_DATA_EXPIRATION_DELAY_SEC  30

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/date_time.hpp> // TODO: <ctime>
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>


//class SBS1_message
//{
//    SBS1_message()
//        : altitude(0)
//        , gr_speed(0)
//        , lat(0)
//        , lon(0)
//    {}

//    char* msg_type; 0
//    char* trm_type; 1
//    unsigned int session_id; 2

//    unsigned int aircraft_id; 3

//    char* hex_craft_id; 4
//    unsigned int flight_id; 5
//    boost::date_time::date date_gen; 6
//    boost::posix_time::time_duration time_gen; 7
//    boost::date_time::date date_log; 8
//    boost::posix_time::time_duration time_log; 9
//    char* callsign; 10

//    double altitude; // m 11
//    double gr_speed; // km/h 12
//    double track; // grad 13
//    double lat; // grad 14
//    double lon; // grad 15

//    double vert_rate; 16
//    char* squawk; 17
//    bool alert; 18
//    bool emergency; 19
//    bool spi; 20
//    bool is_on_ground; 21

//    bool is_informative()
//    {
//        return ( altitude!=0 && gr_speed!=0 && lat!=0 && lon!=0 );
//    }
//};


struct SBS1_message
{
    std::vector<std::string> params;
    bool isValid(){
        return (!params[4].empty() /*&& params[6]!="" && params[7]!=""*/
                && !params[11].empty() && !params[14].empty() && !params[15].empty());
    }
};

typedef std::map<int, SBS1_message> AIRMap;

class SearchService: boost::noncopyable
{
    public:
        explicit SearchService(boost::asio::io_service&);
        ~SearchService();

        void startTracking();
        void stopTracking();
        void read(std::stringstream&);

        AIRMap getCache();

    private:
        boost::asio::io_service io_service;
        boost::asio::deadline_timer t;
        AIRMap cache;
        bool cleanupCache();
};

#endif // SEARCH_SERVICE_H
