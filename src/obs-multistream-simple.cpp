// Minimal OBS Plugin Test - Should compile successfully
#include <obs-module.h>

// Declare the module
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-multistream", "en-US")

// Minimal plugin functions
extern "C" {

EXPORT bool obs_module_load(void) {
    // Just return true - minimal functionality
    return true;
}

EXPORT void obs_module_unload(void) {
    // Nothing to clean up in minimal version
}

EXPORT const char* obs_module_name(void) {
    return "OBS Multistream Plugin (Test)";
}

EXPORT const char* obs_module_description(void) {
    return "Minimal test version of multistream plugin";
}

} // extern "C" 