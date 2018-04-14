#include "Radar.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdlib.h>

// Golosiiv (id=1824)
const double LOCAL_LAT = 50.363127778 * M_PI / 180.;
const double LOCAL_LON = 30.495891667 * M_PI / 180.;
const double LOCAL_ALT = 211.8;
const double R = 6371000.;
const double FOOTS_IN_METER = 100. / 30.48;

// ------------------STUFF

#define GRAD_TO_RAD(x) \
    ( (x) * M_PI / 180. )

#define RAD_TO_GRAD(x) \
    ( (x) / M_PI * 180. )

double tanA(SBS1_message &msg) {
    double lat = GRAD_TO_RAD(strtod(msg.params[14].c_str(), NULL));
    double lon = GRAD_TO_RAD(strtod(msg.params[15].c_str(), NULL));

    return (lon - LOCAL_LON)*cos(lat) /
            ((lat - LOCAL_LAT) + (lon - LOCAL_LON)*(lon - LOCAL_LON)*sin(LOCAL_LAT)*cos(lat)/2);
}

double tanH(SBS1_message &msg) {
    double lat = GRAD_TO_RAD(strtod(msg.params[14].c_str(), NULL));
    double lon = GRAD_TO_RAD(strtod(msg.params[15].c_str(), NULL));
    double alt = strtod(msg.params[11].c_str(), NULL) / FOOTS_IN_METER;
    double deltaAngleSq = (lat - LOCAL_LAT)*(lat - LOCAL_LAT) +
                          (lon - LOCAL_LON)*(lon - LOCAL_LON)*cos(lat)*cos(lat); // TODO : less calc-s
    return ((alt - LOCAL_ALT)*(alt - LOCAL_ALT) - deltaAngleSq*(R + alt)*(alt - LOCAL_ALT)) /
            (deltaAngleSq*(R + alt)*(R + alt));
}

// ----------------CLASS RADAR IMPLEMENTATION

Radar::Radar(boost::asio::io_service &service)
    :t(service)
{}

Radar::~Radar() = default;

void Radar::attach(SearchService &service)
{
    io_service.run();
    _readAirData(service);
}

void Radar::stop()
{
    io_service.stop();
    t.cancel();
}

ACMap Radar::getCache()
{
    return cache;
}

ACPoint Radar::convertRawMessage(SBS1_message &msg)
{
    double lat = GRAD_TO_RAD(strtod(msg.params[14].c_str(), NULL));
    double lon = GRAD_TO_RAD(strtod(msg.params[15].c_str(), NULL));
    double alt = strtod(msg.params[11].c_str(), NULL) / FOOTS_IN_METER;

    ACPoint data_from_msg{0,0,0,0};
    data_from_msg.grSpeedKmPH = strtod(msg.params[12].c_str(), NULL);
    data_from_msg.azGrad = RAD_TO_GRAD(atan2(  (lon - LOCAL_LON)*cos(lat) ,
                                                    (lat - LOCAL_LAT) + (lon - LOCAL_LON)*(lon - LOCAL_LON)*sin(LOCAL_LAT)*cos(lat)/2
                                            ) + M_PI);
    double tgH = tanH(msg);
    double tgA = tanA(msg);
    data_from_msg.elGrad = RAD_TO_GRAD(atan(tgH));
    data_from_msg.distanceMeters = (R + alt)*(LOCAL_LON - lon)*cos(lat)*sqrt((1 + tgA*tgA)*(1 + tgH*tgH)) / tgA;

    std::cout << "LOGS: A: " << data_from_msg.azGrad << " , H: " << data_from_msg.elGrad << std::endl;

    return data_from_msg;
}

void Radar::_readAirData(SearchService &service)
{
    std::cout << "Radar::_readAirData" << std::endl;
    if (! service.getCache().empty() ) {
        auto it = service.getCache().begin();
        for (auto aircraft : service.getCache())
        {
            int aircrft_id = aircraft.first;
            ACPoint aircrft_info = convertRawMessage(aircraft.second);

            std::cout << aircrft_id << "  " <<
                        aircrft_info.azGrad << "  " <<
                        aircrft_info.elGrad << "  " <<
                        aircrft_info.distanceMeters << std::endl;

            std::pair<ACMap::iterator,bool>
                    ret = cache.emplace(aircrft_id, aircrft_info);
            if (!ret.second) {
                cache.erase(aircrft_id);
                cache.emplace(aircrft_id, aircrft_info);
            }
        }
    } else {
        cache.clear();
    }

    t.expires_from_now(boost::posix_time::seconds(RADAR_UPDATE_DELAY_SEC));
//    t.async_wait(boost::bind(&Radar::_readAirData, this, _1)(searchService));
    t.async_wait([&](const boost::system::error_code& error) {
        if (!error) {
            _readAirData(service);
        }
    });
    io_service.run();
}



