#include "SearchService.h"
#include <ctime>


SearchService::SearchService(boost::asio::io_service &io_service)
    : t(io_service)
{}

SearchService::~SearchService()
{}

void SearchService::startTracking()
{
    io_service.run();
    cleanupCache();
}

void SearchService::stopTracking()
{
    io_service.stop();
    t.cancel();
}

void SearchService::read(std::stringstream &ss)
{
    std::string param;
    SBS1_message msg;
    while (std::getline(ss, param, ',')) {
        msg.params.push_back(param);
        std::cout << param << ",";
    }
    std::cout << std::endl;

    int aircraftId = (int)strtol(msg.params[4].c_str(), NULL, 16);

    if (msg.isValid())
    {   
        time_t rtime;
        time(&rtime);
        struct tm * timeinfo;
        // stamp message with timeStamp when readed
        timeinfo = localtime(&rtime);

        char buf[64];
        strftime(buf, sizeof(buf), "%Y/%m/%d", timeinfo);
        msg.params[6] += std::string(buf);
        strftime(buf, sizeof(buf), "%H/%M/%S", timeinfo);
        msg.params[7] += std::string(buf);
    
        // Insert only with a new unique key.
        // One message per aircraft
        std::pair<AIRMap::iterator,bool>
                ret = cache.emplace(aircraftId, msg);
        if (!ret.second)
        {
            cache.erase(aircraftId);
            cache.emplace(aircraftId, msg);
        }

        for (int i=0; i<cache[aircraftId].params.size(); i++)
            std::cout << cache[aircraftId].params[i];
        std::cout << std::endl;

    }
}

AIRMap SearchService::getCache()
{
    return cache;
}

bool SearchService::cleanupCache()
{
    std::cout << "SearchService::cleanupCache" << std::endl;
    if (!cache.empty()) {
        AIRMap new_data;
        AIRMap::iterator it = cache.begin();
        do {
            int aircrft_id = it->first;
            SBS1_message msg = it->second;

            // check the age of the most actual message
            std::string msg_dt_str = msg.params[6] + " " + msg.params[7];
            boost::posix_time::ptime time_msg_gen = boost::posix_time::time_from_string(msg_dt_str);
            boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - time_msg_gen;

            // add non-expired message to new data
            if (td < boost::posix_time::seconds(AIRCRAFT_DATA_EXPIRATION_DELAY_SEC))
                new_data.emplace(aircrft_id, msg);
        } while (++it != cache.end());

        cache = new_data;
    }

    t.expires_from_now(boost::posix_time::seconds(AIRCRAFT_DATA_EXPIRATION_DELAY_SEC/2));
    // t.async_wait(boost::bind(&SearchService::cleanupCache, this));
    t.async_wait([&](const boost::system::error_code& error) {
        if (!error)
            cleanupCache();
    });
    io_service.run();
}
