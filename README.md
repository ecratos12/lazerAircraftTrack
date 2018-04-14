# lazerAircraftTrack

This is in-sky safety system using ADS-B radar receiver for SLR (Satellite Laser Ranging) station. It is asyncronous service which bases on processing SBS1 BaseStation format data as aircraft data from some socket, as well as processing ephemeris data as data of target satellites.

[SBS1 format](http://woodair.net/sbs/Article/Barebones42_Socket_Data.htm)
Ephemeris data format : 
file: `SATDDMM.STA` (SAT - sattelite name, STA - station name)
header: `DD MM YYYY SAT STA xxxxxxx`
content: `HH MM SS Azimuth Elevation Distance x`

## Build source with command
`g++ -std=c++11 src/*.cpp -o bin/binary-name -lboost_system -lpthread  -lboost_date_time`

( use of boost `1.65.1` http://www.boost.org/ )

## ADS-B decoder

https://github.com/ecratos12/dump1090

## Usage

`bin/binary-name <server> <port> <sattelite name>`
