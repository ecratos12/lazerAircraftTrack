#ifndef RADAR_H
#define RADAR_H

#define RADAR_UPDATE_DELAY_SEC  2

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

        ACMap getCache();

    protected:
        void _readAirData(SearchService&);
        ACPoint convertRawMessage(SBS1_message&);

        boost::asio::io_service io_service;
        boost::asio::deadline_timer t;
        ACMap cache;
};

#endif // RADAR_H
