#include "LazerOrientation.h"
#include <boost/filesystem.hpp>
#include <fstream>

using namespace boost::filesystem;

//---------------------------CLASS EPHDATA IMPLEMENTATION--------------------------------------------

EphData::EphData() = default;
EphData::~EphData() = default;

void EphData::fill()
{
#ifdef __linux__
    path p("./ephdata");
#endif
#ifdef _WIN32
    path p(".\\ephdata");
#endif

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

        while (std::getline(in, tmp)) {
            if (tmp.empty())
                continue;

            time_t now;
            time(&now);
#ifdef __linux__
            struct tm now_tm;
            gmtime_r(&now, &now_tm);
            SATPoint point{0,0,now_tm};
#endif
#ifdef _WIN32
            struct tm* now_tm;
            now_tm = localtime(&now);
            SATPoint point{0,0,*now_tm};
#endif
            sscanf(tmp.c_str(), "%i %i %i %lf %lf",
                   &point.timeStamp.tm_hour,
                   &point.timeStamp.tm_min,
                   &point.timeStamp.tm_sec,
                   &point.azGrad,
                   &point.elGrad);
            point.timeStamp.tm_mday = d;
            point.timeStamp.tm_mon = m;
            point.timeStamp.tm_year = y;

            satData.push_back(point);

//            char buffer[80];
//            strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",&satData.back().timeStamp);
//            std::string str(buffer);
//            std::cout << str << std::endl;
        }

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

#ifdef __linux__
    struct tm now_tm;
    gmtime_r(&now, &now_tm);
    const SATPoint UNAVAILABLE_SAT_POSITION = {
        0.
        , -90.
        , now_tm
    };
#endif
#ifdef _WIN32
    struct tm* now_tm;
    now_tm = localtime(&now);
    const SATPoint UNAVAILABLE_SAT_POSITION = {
        0.
        , -90.
        , *now_tm
    };
#endif

    SATMap currentPositions;

    std::cout << " EphData::getCurrent -- Found current SATPoint for ";
    for (auto &it : _data)
    {
        _isAvailable_perSat = false;
        for (auto inner_it = it.second.begin(); inner_it != it.second.end() ; ++inner_it)
        {
            #ifdef __linux__
            double t_diff = difftime(mktime(&now_tm), mktime(&inner_it->timeStamp));
            #endif
            #ifdef _WIN32
            double t_diff = difftime(mktime(now_tm), mktime(&inner_it->timeStamp));
            #endif
            if ((int)t_diff == 0) {
                currentPositions.emplace(it.first, *inner_it);
                _isAvailable_perSat = true;
                std::cout << it.first << " ; ";
            }
//            delete outdated satellite data
            if (!it.second.empty()) {
                if ((int)t_diff > 0) {
                    if (it.second.size() != 1) {
                        it.second.erase(inner_it);
                        --inner_it;
                    }
                }
            }
        }
        if (!_isAvailable_perSat) {
            currentPositions.emplace(it.first, UNAVAILABLE_SAT_POSITION);
        //    std::cout << "EphData::getCurrent -- Set unavailable SAT " << it.first << std::endl;
        }
    }
    std::cout << std::endl;
//    std::cout << "EphData::getCurrent -- Got currentPositions" << std::endl;
    return currentPositions;
}

//---------------------------CLASS LAZERORIENTATION IMPLEMENTATION--------------------------------------------

LazerOrientation::LazerOrientation()
{
    ephData.fill();
    std::cout << "Got eph data" << std::endl;
}

LazerOrientation::~LazerOrientation() = default;

void LazerOrientation::_setFromServo() {}

void LazerOrientation::_setFromEph(std::string& satString)
{
    SATMap satMap = ephData.getCurrent();
    // try {
        azGrad = satMap.at(satString).azGrad;
        elGrad = satMap.at(satString).elGrad;
    // } catch (const std::out_of_range& e) {
    //     azGrad = 0;
    //     elGrad = 0;
    //     std::cout << "WARNING!!! NO SUCH SAT_NAME FOUND IN EPHEMERIS BASE !!!" << std::endl;
    //     std::cout << "-------------LAZER: AZ = 0, EL = 0------------" << std::endl;
    // }
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

SATMap LazerOrientation::getCurrentSatMap()
{
    return ephData.getCurrent();
}