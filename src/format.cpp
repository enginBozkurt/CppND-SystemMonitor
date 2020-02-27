#include <string>
#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    int hour = seconds / 3600;
    short minute = (seconds % 3600) / 60;
    short second = (seconds % 3600) % 60;
    char buffer[8];
    sprintf(buffer,"%.2d:%.2d:%.2d",hour,minute,second);
    string elapsed_time(buffer);
    return elapsed_time;
}