#include "Radar.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define RADAR_UPDATE_DELAY_SEC  2

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
    double lat = GRAD_TO_RAD(std::stod(msg.params[14]));
    double lon = GRAD_TO_RAD(std::stod(msg.params[15]));

    return (lon - LOCAL_LON)*cos(lat) /
            ((lat - LOCAL_LAT) + (lon - LOCAL_LON)*(lon - LOCAL_LON)*sin(LOCAL_LAT)*cos(lat)/2);
}

double tanH(SBS1_message &msg) {
    double lat = GRAD_TO_RAD(std::stod(msg.params[14]));
    double lon = GRAD_TO_RAD(std::stod(msg.params[15]));
    double alt = std::stod(msg.params[11]);
    double deltaAngleSq = (lat - LOCAL_LAT)*(lat - LOCAL_LAT) +
                          (lon - LOCAL_LON)*(lon - LOCAL_LON)*cos(lat)*cos(lat); // TODO : less calc-s
    return (alt - LOCAL_ALT)*(alt - LOCAL_ALT) - deltaAngleSq*(R + alt)*(alt - LOCAL_ALT) /
            deltaAngleSq*(R + alt)*(R + alt);
}

// ----------------INTERFACE

Radar::Radar(boost::asio::io_service &service)
    :t(service)
{}

Radar::~Radar()
{}

void Radar::start()
{
    io_service.run();
}

void Radar::stop()
{
    io_service.stop();
    t.cancel();
}

ACData Radar::convertRawMessage(SBS1_message &msg)
{
    double lat = GRAD_TO_RAD(std::stod(msg.params[14]));
    double lon = GRAD_TO_RAD(std::stod(msg.params[15]));
    double alt = std::stod(msg.params[11]);

    ACData data_from_msg;
    data_from_msg.grSpeedKmPH = std::stod(msg.params[12]);
    data_from_msg.azimuthGrad = RAD_TO_GRAD(atan2(  (lon - LOCAL_LON)*cos(lat) ,
                                                    (lat - LOCAL_LAT) + (lon - LOCAL_LON)*(lon - LOCAL_LON)*sin(LOCAL_LAT)*cos(lat)/2
                                            ));
    double tgH = tanH(msg);
    double tgA = tanA(msg);
    data_from_msg.heightGrad = RAD_TO_GRAD(atan(tgH));
    data_from_msg.distanceMeters = (R + alt)*(lon - LOCAL_LON)*cos(lat)*sqrt((1 + tgA*tgA)*(1 + tgH*tgH)) / tgA;

    std::cout << "LOGS: A: " << data_from_msg.azimuthGrad << " , H: " << data_from_msg.heightGrad << std::endl;

    return data_from_msg;
}

void Radar::readAllCache(SearchService &service)
{
    std::map<int, SBS1_message>::iterator it = service.getCache().begin();
    do {
        int aircrft_id = it->first;
        ACData aircrft_info = convertRawMessage(it->second);

        std::cout << aircrft_id << "  " <<
                     aircrft_info.azimuthGrad << "  " <<
                     aircrft_info.heightGrad << "  " <<
                     aircrft_info.distanceMeters << std::endl;

        std::pair<std::map<int, ACData>::iterator,bool> ret = cache.insert(std::pair<int, ACData>(aircrft_id, aircrft_info));
        if (!ret.second) {
            cache.erase(aircrft_id);
            cache.insert(std::pair<int, ACData>(aircrft_id, aircrft_info));
        }

    } while (it++ != service.getCache().end());

    t.expires_from_now(boost::posix_time::seconds(RADAR_UPDATE_DELAY_SEC));
//    t.async_wait(boost::bind(&Radar::readAllCache, this, _1)(searchService));
    t.async_wait([&](const boost::system::error_code& error) {
        if (!error) {
            readAllCache(service);
        }
    });
}



