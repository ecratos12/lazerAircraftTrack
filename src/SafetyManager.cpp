#include "SafetyManager.h"

#include <cmath>
const double PI = 3.14159265358979323846;

//-----------------CALCULATIONS

const double cosCriticalAngle = cos(CRITICAL_ANGLE_GRAD * PI / 180.);

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
        :t(service)
{}

SafetyManager::~SafetyManager() = default;

void SafetyManager::attach(Radar& radar, LazerOrientation& lazerOrientation, std::string& satName)
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
//     TODO : GUI (big red buttons, staff..)
//    SFML window
    std::ofstream statusFile("current_status");
    statusFile << (status==Status::Ok ? "Ok" : "Danger") << std::endl;
    statusFile.close();
}

void SafetyManager::_monitoring(Radar& radar, LazerOrientation& lazerOrientation, std::string& satName)
{
    std::cout << "SafetyManager::_monitoring" << std::endl;
    auto laserGazePos = lazerOrientation.get(satName);
    ACMap acGazePos = radar.getCache();

    status = Status::Ok;
    for (auto const& acPos : acGazePos) {
        if (cosLAZER_AIRCRAFT(laserGazePos, std::make_pair(acPos.second.azGrad, acPos.second.elGrad))
            > cosCriticalAngle) {
            status = Status::Danger;
//            TURN-OFF LASER COMMAND
            break;
        }
    }

    showStatus();

    // plotter addition
//#ifdef PLOT_ENABLED
    std::ofstream csvForPlot("current_positions.csv");

    SATMap satMap = lazerOrientation.getCurrentSatMap();
    for (auto const& satPoint : satMap) {
        if (satPoint.second.elGrad > 0)
            csvForPlot << "SAT," << satPoint.first << "," << satPoint.second.azGrad << "," << satPoint.second.elGrad << std::endl;
    }
    for (auto const& acPoint : acGazePos) {
        csvForPlot << "AIR," << acPoint.first << "," << acPoint.second.azGrad << "," << acPoint.second.elGrad << std::endl;
    }
    csvForPlot.close();

//#endif

    t.expires_from_now(boost::posix_time::seconds(LAZER_ORIENTATION_UPDATE_DELAY_SEC));
    t.async_wait([&](const boost::system::error_code& error) {
        if (!error) {
            _monitoring(radar, lazerOrientation, satName);
        }
    });
    io_service.run();
}