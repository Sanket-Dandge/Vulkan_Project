#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include "helpers/logger.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_GRAY    "\033[90m"       // VERBOSE
#define COLOR_BLUE    "\033[34m"
#define COLOR_ORANGE  "\033[38;5;208m" // WARNING (256-color orange)
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RED     "\033[31m"       // ERROR

#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE2    "\033[38;5;39m"     // brighter cyan-blue
#define COLOR_BOLD    "\033[1m"
#define COLOR_DIM     "\033[2m"


#define LOG(fmt, ...) \
    std::printf(COLOR_GREEN fmt "\n" COLOR_RESET, ##__VA_ARGS__)

using debugLevel = enum DebugLevel : uint8_t {
    VERBOSE, WARNING, ERROR
};

#define DEBUGLOG(level, fmt, ...)                                      \
    do {                                                               \
        const char* _color = COLOR_RESET;                              \
        const char* _label = "";                                       \
        switch (level) {                                               \
            case VERBOSE: _color = COLOR_BLUE;   _label = "[VERBOSE] "; break; \
            case WARNING: _color = COLOR_YELLOW; _label = "[WARNING] "; break; \
            case ERROR:   _color = COLOR_RED;    _label = "[ERROR]   "; break; \
        }                                                              \
        std::printf("%s%s" fmt "%s", _color, _label, ##__VA_ARGS__, COLOR_RESET); \
    } while (0)

#define LOG_INIT(objName) \
    std::printf( \
        COLOR_BOLD COLOR_BLUE2 "[INIT] " COLOR_RESET "Created %s...\n", \
        objName);


#define LOG_DSTY(objName) \
    std::printf( \
        COLOR_BOLD COLOR_ORANGE "[DESTROY] " COLOR_RESET "Destroyed %s...\n", \
        objName);


#define LOAD_INSTANCE_EXTENSION(name) \
    name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);

#define LOAD_DEVICE_EXTENSION(name) \
    name##_fp = (PFN_##name)vkGetDeviceProcAddr(device, #name);

inline void load_VK_EXTENSIONS(VkInstance instance,
                               PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                               VkDevice device,
                               PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr)
{}
