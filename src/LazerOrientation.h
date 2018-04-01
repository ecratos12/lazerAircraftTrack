#ifndef LAZERORIENTATION_H
#define LAZERORIENTATION_H

#include "SearchService.h"
#include <ctime>


struct SATPoint
{
    double azGrad;
    double elGrad;
    struct tm * time;
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
    std::map<std::string, SATData> _data;
};

class LazerOrientation
{
public:
    LazerOrientation();
    ~LazerOrientation();

    // supposing laser oriented straight to target satellite
    std::pair<double, double> get(std::string& satString);

protected:
    void _setFromServo();
    void _setFromEph(std::string& satString);

    double azGrad;
    double elGrad;
    EphData ephData;
};

#endif // LAZERORIENTATION_H
