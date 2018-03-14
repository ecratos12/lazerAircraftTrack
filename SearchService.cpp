#include "SearchService.h"
#include <ctime>

#define AIRCRAFT_DATA_EXPIRATION_DELAY_SEC  30

SearchService::SearchService(boost::asio::io_service &io_service)
    : t(io_service)
{}

SearchService::~SearchService()
{}

void SearchService::startTracking()
{
    service.run();
}

void SearchService::stopTracking()
{
    service.stop();
    t.cancel();
}

void SearchService::read(std::stringstream &ss)
{
    std::string param;
    SBS1_message msg;
    while (std::getline(ss, param, ',')) {
        msg.params.push_back(param);
        std::cout << param << ", ";
    }
    std::cout << std::endl;
    int aircraftId = (int)strtol(msg.params[3].c_str(), NULL, 16);

    if (msg.isValid())
    {
        if (msg.params[6]!="" && msg.params[7]!="") {
            time_t rtime;
            time(&rtime);
            struct tm * timeinfo;
            timeinfo = localtime(&rtime);

            char buf[64];
            strftime(buf, sizeof(buf), "%Y/%m/%d", timeinfo);
            msg.params[6] += std::string(buf);
            strftime(buf, sizeof(buf), "%H/%M/%S", timeinfo);
            msg.params[7] += std::string(buf);
        }
        // Insert only with a new unique key.
        // One message per aircraft
        std::pair<std::map<int, SBS1_message>::iterator,bool> ret = cache.insert(std::pair<int, SBS1_message>(aircraftId, msg));
        if (ret.second==false)
        {
            cache.erase(aircraftId);
            cache.insert(std::pair<int, SBS1_message>(aircraftId, msg));
        }
    }
}

std::map<int, SBS1_message> SearchService::getCache()
{
    return cache;
}

bool SearchService::cleanupCache()
{
    std::map<int, SBS1_message> new_data;
    std::map<int, SBS1_message>::iterator it = cache.begin();
    do {
        int aircrft_id = it->first;
        SBS1_message msg = it->second;

        // check the age of the most actual message
        std::string msg_dt_str = msg.params[6] + " " + msg.params[7];
        boost::posix_time::ptime time_msg_gen = boost::posix_time::time_from_string(msg_dt_str);
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - time_msg_gen;

        // add non-expired message to new data
        if (td < boost::posix_time::seconds(AIRCRAFT_DATA_EXPIRATION_DELAY_SEC))
            new_data.insert(std::pair<int, SBS1_message>(aircrft_id, msg));
    } while (it++ != cache.end());

    cache = new_data;

    t.expires_from_now(boost::posix_time::seconds(AIRCRAFT_DATA_EXPIRATION_DELAY_SEC/2));
    t.async_wait(boost::bind(&SearchService::cleanupCache, this));
    service.run();
}
