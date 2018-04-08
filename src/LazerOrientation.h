#ifndef LAZERORIENTATION_H
#define LAZERORIENTATION_H

#define LAZER_ORIENTATION_UPDATE_DELAY_SEC 1

#include "SearchService.h"
#include <ctime>


struct SATPoint
{
    double azGrad;
    double elGrad;
    struct tm timeStamp;
};

typedef std::map<std::string, SATPoint> SATMap;
typedef std::vector<SATPoint> SATData;

class EphData
{
public:
    EphData();
    ~EphData();

    void fill();
    SATMap getCurrent();


protected:
    // workaround of empty current satellites sky map
    // if satellite isn't available, set: az = 0, el = -90
    bool _isAvailable_perSat;
    std::map<std::string, SATData> _data;
};

class LazerOrientation
{
public:
    LazerOrientation();
    ~LazerOrientation();

    // supposing laser oriented straight to target satellite
    // az, el
    std::pair<double, double> get(std::string& satString);

protected:
    void _setFromServo();
    void _setFromEph(std::string& satString);

    double azGrad;
    double elGrad;
    EphData ephData;
};

#endif // LAZERORIENTATION_H
