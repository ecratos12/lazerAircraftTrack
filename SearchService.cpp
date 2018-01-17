#include "SearchService.h"

//static const double STATION_LAT = 50.0;
//static const double STATION_LON = 30.0;

const int AIRCRAFT_DATA_EXPIRATION_DELAY = 30;

SearchService::SearchService(boost::asio::io_service &io_service)
    : t(io_service)
{}

SearchService::~SearchService()
{

}

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
    while (std::getline(ss, param, ','))
    {
        msg.params.push_back(param);
        std::cout << param.c_str() << std::endl;
    }
    int aircraftId = std::stoi(msg.params[3]);

    if (msg.isValid())
    {
        // see std::map::insert docs. Insert only with a new unique key.
        std::pair<std::map<int, SBS1_message>::iterator,bool> ret = data.insert(std::pair<int, SBS1_message>(aircraftId, msg));
        if (ret.second==false)
        {
            data.erase(aircraftId);
            data.insert(std::pair<int, SBS1_message>(aircraftId, msg));
        }
    }
}

bool SearchService::resetData()
{
    std::map<int, SBS1_message> new_data;
    std::map<int, SBS1_message>::iterator it = data.begin();
    while (it != data.end())
    {
        int aircrft_id = it->first;
        SBS1_message msg = it->second;

        // check the age of the most actual message
        std::string msg_dt_str = msg.params[6] + " " + msg.params[7];
        boost::posix_time::ptime time_msg_gen = boost::posix_time::time_from_string(msg_dt_str);
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - time_msg_gen;

        // add non-expired messages to new data
        if (td < boost::posix_time::seconds(AIRCRAFT_DATA_EXPIRATION_DELAY))
            new_data.insert(std::pair<int, SBS1_message>(aircrft_id, msg));
        it++;
    }
    new_data = data;

    t.expires_from_now(boost::posix_time::seconds(AIRCRAFT_DATA_EXPIRATION_DELAY/2));
    t.async_wait(boost::bind(&SearchService::resetData, this));
    service.run();
}
