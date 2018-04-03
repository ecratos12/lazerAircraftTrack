#include "SafetyManager.h"

#define _USE_MATH_DEFINES
#include <cmath>

//-----------------CALCULATIONS

const double cosCriticalAngle = cos(M_PI / 18.); // 10 grads

// angle between some aircraft and laser orientation
// az, el
double cosLAZER_AIRCRAFT(std::pair<double, double> lazer, std::pair<double, double> aircraft)
{
    // TODO : less calc-s
    return
            sin(lazer.second)*sin(aircraft.second) +
            cos(lazer.second)*cos(aircraft.second)*cos(lazer.first - aircraft.first);
}

// Thinking geometrically, function builds
// aircraft trajectory using its ADS-B info: heading and course.
// The cosine of angled impact parameter for this trajectory returned
// TODO : implementation
double MAX_PREDICTED_cosLAZER_AIRCRAFT(std::pair<double, double> lazer, ACPoint aircraft)
{

}

//--------------------INTERFACE

SafetyManager::SafetyManager(boost::asio::io_service &service)
        :t(service),
         status(Status::Ok)
{}

SafetyManager::~SafetyManager() = default;

void SafetyManager::attach(Radar& radar, LazerOrientation& lazerOrientation, std::string satName)
{
    io_service.run();
    _monitoring(radar, lazerOrientation, satName);
}

void SafetyManager::stop()
{
    io_service.stop();
    t.cancel();
}

void SafetyManager::showStatus()
{
    // TODO : GUI (big red buttons, staff..)
}

void SafetyManager::_monitoring(Radar& radar, LazerOrientation& lazerOrientation, std::string& satName)
{
    auto laserGazePos = lazerOrientation.get(satName);
    ACMap acGazePos = radar.getCache();

    for (auto const& acPos : acGazePos) {
        if (cosLAZER_AIRCRAFT(laserGazePos, std::make_pair(acPos.second.azGrad, acPos.second.elGrad)) > cosCriticalAngle) {
            status = Status::Danger;
//            TURN-OFF LASER COMMAND
        }
    }


    t.expires_from_now(boost::posix_time::seconds(LAZER_ORIENTATION_UPDATE_DELAY_SEC));
    t.async_wait([&](const boost::system::error_code& error) {
        if (!error) {
            _monitoring(radar, lazerOrientation, satName);
        }
    });
}