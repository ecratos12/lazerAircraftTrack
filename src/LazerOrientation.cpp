#include "LazerOrientation.h"
#include <boost/filesystem.hpp>
#include <fstream>

using namespace boost::filesystem;

EphData::EphData() = default;
EphData::~EphData() = default;

void EphData::fill()
{
    path p("./ephdata");
    directory_iterator eod;

    for (directory_iterator dir_it(p); dir_it != eod; ++dir_it)
    {
        std::string fname = dir_it->path().string();
        std::ifstream in(fname);
        if (fname.substr(10,4) == "menu") {
            in.close();        
            continue;
        }       
        std::string tmp;
        std::getline(in, tmp);

        int d,m,y; char sat_name[3];
        sscanf(tmp.c_str(), "%i %i %i %s", &d, &m, &y, sat_name);
        SATData satData;

        // due to tm struct compatibility
        m--;
        y = y - 1900;

        do {
            std::getline(in, tmp);
            time_t now;
            time(&now);
        
            SATPoint point{0,0,localtime(&now)};
            sscanf(tmp.c_str(), "%i %i %i %lf %lf",
                   &point.time->tm_hour,
                   &point.time->tm_min,
                   &point.time->tm_sec,
                   &point.azGrad,
                   &point.elGrad);
            point.time->tm_mday = d;
            point.time->tm_mon = m;
            point.time->tm_year = y;
            satData.push_back(point);
        } while (!tmp.empty());

        auto iter = _data.find(std::string(sat_name));
        if (iter != _data.end()) {
            iter->second.insert(iter->second.end(), satData.begin(), satData.end());
        } else {
            _data.emplace(std::string(sat_name), satData);
        }

        in.close();
    }
}

SATMap EphData::getCurrent()
{
    time_t now;
    time(&now);

    SATMap currentPositions;

    auto it = _data.begin();
    do {
        for (auto inner_it = it->second.begin(); inner_it != it->second.end() ; ++inner_it)
        {
            double t_diff = difftime(now, mktime(inner_it->time));
            if ((int)t_diff == 0) {
                currentPositions.emplace(it->first, *inner_it);
            }
            // delete outdated satellite data
            if ((int)t_diff > 0) {
                it->second.erase(inner_it);
            }
        }
    } while (it++ != _data.end());

    return currentPositions;
}

LazerOrientation::LazerOrientation()
{
    std::cout << "Get eph data" << std::endl;
    ephData.fill();
    std::cout << "Got eph data" << std::endl;
}

LazerOrientation::~LazerOrientation() = default;

void LazerOrientation::_setFromServo() {}

void LazerOrientation::_setFromEph(std::string& satString)
{
    SATMap satMap = ephData.getCurrent();
    azGrad = satMap.at(satString).azGrad;
    elGrad = satMap.at(satString).elGrad;
}

std::pair<double, double> LazerOrientation::get(std::string& satString)
{
#ifdef SERVO_DATA_ENABLED
    _setFromServo();
    return std::make_pair(azGrad, elGrad);
#else
    _setFromEph(satString);
    return std::make_pair(azGrad, elGrad);
#endif
}