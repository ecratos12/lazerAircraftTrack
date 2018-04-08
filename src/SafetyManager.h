#ifndef SAFETYMANAGER_H
#define SAFETYMANAGER_H

#define CRITICAL_ANGLE_GRAD 10.

#include "Radar.h"
#include "LazerOrientation.h"

enum class Status: unsigned int
{
    Ok = 0,
    Warning = 1,
    Danger = 2
};

class SafetyManager
{
public:
    explicit SafetyManager(boost::asio::io_service&);
    ~SafetyManager();

    // TODO : vector of satName's
    void attach(Radar&, LazerOrientation&, std::string& satName);
    void stop();

    void showStatus();

protected:
    void _monitoring(Radar&, LazerOrientation&, std::string& satName);
    boost::asio::io_service io_service;
    boost::asio::deadline_timer t;
    Status status;
};


#endif //SAFETYMANAGER_H
