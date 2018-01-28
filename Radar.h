#ifndef RADAR_H
#define RADAR_H

#include "SearchService.h"

// Aircraft radar data.
struct ACData
{
        double azimuthGrad;
        double heightGrad;
        double distanceMeters;
        double grSpeedKmPH;
};

class Radar : public boost::noncopyable
{
    public:
        Radar(boost::asio::io_service&);
        ~Radar();

        void connectToSearchService(SearchService&);
        void stop();

        ACData convertRawMessage(SBS1_message&);
        void readAllCache(SearchService&);

    private:
        boost::asio::io_service io_service;
        boost::asio::deadline_timer t;
        std::map<int, ACData> cache;

};

#endif // RADAR_H
