#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/dstr.h>
#include <vector>
#include <string>

// Include StreamDestination definition
#include "stream-destination.h"

#define PLUGIN_NAME "obs-multistream"
#define PLUGIN_VERSION "1.0.0"

// Forward declarations
class MultistreamDock;
class MultistreamOutput;

// Main plugin class
class MultistreamPlugin {
public:
    static MultistreamPlugin* GetInstance();
    
    bool Initialize();
    void Shutdown();
    void Cleanup(); // Public cleanup method for proper singleton destruction
    
    // Stream management
    void AddDestination(const StreamDestination& dest);
    void RemoveDestination(size_t index);
    void UpdateDestination(size_t index, const StreamDestination& dest);
    const std::vector<StreamDestination>& GetDestinations() const;
    
    // Streaming control
    void StartStreaming();
    void StopStreaming();
    bool IsStreaming() const;
    
    // Configuration
    void SaveSettings();
    void LoadSettings();
    
private:
    MultistreamPlugin();
    ~MultistreamPlugin();
    
    static MultistreamPlugin* instance;
    
    std::vector<StreamDestination> destinations;
    std::vector<MultistreamOutput*> outputs;
    MultistreamDock* dock;
    
    bool isStreaming;
    
    // Event handlers
    static void OnMainStreamingStarted(enum obs_frontend_event event, void* data);
    static void OnMainStreamingStopped(enum obs_frontend_event event, void* data);
};

// Helper functions
const char* GetPluginVersion();
const char* GetPluginName();

// Module exports
extern "C" {
    EXPORT bool obs_module_load(void);
    EXPORT void obs_module_unload(void);
    EXPORT MODULE_EXPORT const char* obs_module_name(void);
    EXPORT MODULE_EXPORT const char* obs_module_description(void);
} 