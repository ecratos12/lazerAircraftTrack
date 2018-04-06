#include "Radar.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdlib.h>

// Golosiiv (id=1824)
const double LOCAL_LAT = 50.;
const double LOCAL_LON = 30.;
const double LOCAL_ALT = 300.;
const double R = 6371000;

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
    double alt = strtod(msg.params[11].c_str(), NULL);
    double deltaAngleSq = (lat - LOCAL_LAT)*(lat - LOCAL_LAT) +
                          (lon - LOCAL_LON)*(lon - LOCAL_LON)*cos(lat)*cos(lat); // TODO : less calc-s
    return (alt - LOCAL_ALT)*(alt - LOCAL_ALT) - deltaAngleSq*(R + alt)*(alt - LOCAL_ALT) /
            deltaAngleSq*(R + alt)*(R + alt);
}

// ----------------INTERFACE

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
    double alt = strtod(msg.params[11].c_str(), NULL);

    ACPoint data_from_msg{};
    data_from_msg.grSpeedKmPH = strtod(msg.params[12].c_str(), NULL);
    data_from_msg.azGrad = RAD_TO_GRAD(atan2(  (lon - LOCAL_LON)*cos(lat) ,
                                                    (lat - LOCAL_LAT) + (lon - LOCAL_LON)*(lon - LOCAL_LON)*sin(LOCAL_LAT)*cos(lat)/2
                                            ));
    double tgH = tanH(msg);
    double tgA = tanA(msg);
    data_from_msg.elGrad = RAD_TO_GRAD(atan(tgH));
    data_from_msg.distanceMeters = (R + alt)*(lon - LOCAL_LON)*cos(lat)*sqrt((1 + tgA*tgA)*(1 + tgH*tgH)) / tgA;

    std::cout << "LOGS: A: " << data_from_msg.azGrad << " , H: " << data_from_msg.elGrad << std::endl;

    return data_from_msg;
}

void Radar::_readAirData(SearchService &service)
{
    std::cout << "Radar::_readAirData" << std::endl;
    if (! service.getCache().empty() ) {
        auto it = service.getCache().begin();
        do {
            int aircrft_id = it->first;
            ACPoint aircrft_info = convertRawMessage(it->second);

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

        } while (it++ != service.getCache().end());
        std::cout << "Radar::_readAirData readed cache" << std::endl;
    }
    std::cout << "Radar::_readAirData proceeded" << std::endl;

    t.expires_from_now(boost::posix_time::seconds(RADAR_UPDATE_DELAY_SEC));
//    t.async_wait(boost::bind(&Radar::_readAirData, this, _1)(searchService));
    t.async_wait([&](const boost::system::error_code& error) {
        if (!error) {
            _readAirData(service);
        }
    });
    io_service.run();
}



