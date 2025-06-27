#pragma once

#include <string>

// Stream destination structure
struct StreamDestination {
    std::string name;
    std::string url;
    std::string key;
    bool enabled;
    bool useMainEncoder;
    int bitrate;
    
    StreamDestination() : enabled(false), useMainEncoder(true), bitrate(2500) {}
}; 