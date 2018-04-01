#ifndef RADAR_H
#define RADAR_H

#include "SearchService.h"

// Aircraft radar data
struct ACPoint
{
        double azGrad;
        double elGrad;
        double distanceMeters;
        double grSpeedKmPH;
};

typedef std::map<int, ACPoint> ACMap;

class Radar : public boost::noncopyable
{
    public:
        explicit Radar(boost::asio::io_service&);
        ~Radar();

        void attach(SearchService&);
        void stop();

        ACPoint convertRawMessage(SBS1_message&);
        void readAllCache(SearchService&);

    private:
        boost::asio::io_service io_service;
        boost::asio::deadline_timer t;
        ACMap cache;

};

#endif // RADAR_H
