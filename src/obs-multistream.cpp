#include "obs-multistream.h"
#include "multistream-dock.h"
#include "multistream-output.h"
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/platform.h>

// Static instance
MultistreamPlugin* MultistreamPlugin::instance = nullptr;

// Module globals
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-multistream", "en-US")

// ============================================================================
// Plugin Implementation
// ============================================================================

MultistreamPlugin::MultistreamPlugin() 
    : dock(nullptr), isStreaming(false) {
}

MultistreamPlugin::~MultistreamPlugin() {
    Shutdown();
}

MultistreamPlugin* MultistreamPlugin::GetInstance() {
    if (!instance) {
        instance = new MultistreamPlugin();
    }
    return instance;
}

bool MultistreamPlugin::Initialize() {
    blog(LOG_INFO, "[%s] Initializing plugin v%s", PLUGIN_NAME, PLUGIN_VERSION);
    
    // Load settings
    LoadSettings();
    
    // Create dock
    dock = new MultistreamDock();
    if (!dock->Initialize()) {
        blog(LOG_ERROR, "[%s] Failed to initialize dock", PLUGIN_NAME);
        return false;
    }
    
    // Register event handlers
    obs_frontend_add_event_callback(
        [](enum obs_frontend_event event, void* data) {
            MultistreamPlugin* plugin = static_cast<MultistreamPlugin*>(data);
            switch (event) {
            case OBS_FRONTEND_EVENT_STREAMING_STARTED:
                plugin->OnMainStreamingStarted(event, data);
                break;
            case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
                plugin->OnMainStreamingStopped(event, data);
                break;
            default:
                break;
            }
        }, this);
    
    blog(LOG_INFO, "[%s] Plugin initialized successfully", PLUGIN_NAME);
    return true;
}

void MultistreamPlugin::Shutdown() {
    blog(LOG_INFO, "[%s] Shutting down plugin", PLUGIN_NAME);
    
    // Stop streaming if active
    StopStreaming();
    
    // Remove event callbacks
    obs_frontend_remove_event_callback(OnMainStreamingStarted, nullptr);
    obs_frontend_remove_event_callback(OnMainStreamingStopped, nullptr);
    
    // Clean up dock
    if (dock) {
        delete dock;
        dock = nullptr;
    }
}

void MultistreamPlugin::Cleanup() {
    // Perform shutdown operations
    Shutdown();
    
    // Reset the singleton instance
    delete this;
    instance = nullptr;
}

void MultistreamPlugin::AddDestination(const StreamDestination& dest) {
    destinations.push_back(dest);
    SaveSettings();
}

void MultistreamPlugin::RemoveDestination(size_t index) {
    if (index < destinations.size()) {
        destinations.erase(destinations.begin() + index);
        SaveSettings();
    }
}

void MultistreamPlugin::UpdateDestination(size_t index, const StreamDestination& dest) {
    if (index < destinations.size()) {
        destinations[index] = dest;
        SaveSettings();
    }
}

const std::vector<StreamDestination>& MultistreamPlugin::GetDestinations() const {
    return destinations;
}

void MultistreamPlugin::StartStreaming() {
    if (isStreaming) {
        blog(LOG_WARNING, "[%s] Streaming already active", PLUGIN_NAME);
        return;
    }
    
    blog(LOG_INFO, "[%s] Starting multistream to %zu destinations", 
         PLUGIN_NAME, destinations.size());
    
    // Create outputs for each enabled destination
    for (const auto& dest : destinations) {
        if (!dest.enabled) continue;
        
        MultistreamOutput* output = new MultistreamOutput();
        if (output->Initialize(dest)) {
            outputs.push_back(output);
            output->Start();
        } else {
            delete output;
            blog(LOG_ERROR, "[%s] Failed to initialize output for %s", 
                 PLUGIN_NAME, dest.name.c_str());
        }
    }
    
    isStreaming = true;
}

void MultistreamPlugin::StopStreaming() {
    if (!isStreaming) return;
    
    blog(LOG_INFO, "[%s] Stopping multistream", PLUGIN_NAME);
    
    // Stop and clean up all outputs
    for (auto* output : outputs) {
        output->Stop();
        delete output;
    }
    outputs.clear();
    
    isStreaming = false;
}

bool MultistreamPlugin::IsStreaming() const {
    return isStreaming;
}

void MultistreamPlugin::SaveSettings() {
    // Get OBS config path
    char* configPath = obs_frontend_get_global_config_path();
    if (!configPath) return;
    
    // Create config file path
    std::string configFilePath = std::string(configPath) + "/obs-multistream.json";
    bfree(configPath);
    
    // Create JSON data
    obs_data_t* data = obs_data_create();
    obs_data_array_t* destArray = obs_data_array_create();
    
    for (const auto& dest : destinations) {
        obs_data_t* destData = obs_data_create();
        obs_data_set_string(destData, "name", dest.name.c_str());
        obs_data_set_string(destData, "url", dest.url.c_str());
        obs_data_set_string(destData, "key", dest.key.c_str());
        obs_data_set_bool(destData, "enabled", dest.enabled);
        obs_data_set_bool(destData, "useMainEncoder", dest.useMainEncoder);
        obs_data_set_int(destData, "bitrate", dest.bitrate);
        
        obs_data_array_push_back(destArray, destData);
        obs_data_release(destData);
    }
    
    obs_data_set_array(data, "destinations", destArray);
    
    // Save to file
    obs_data_save_json_safe(data, configFilePath.c_str(), "tmp", "bak");
    
    obs_data_array_release(destArray);
    obs_data_release(data);
    
    blog(LOG_INFO, "[%s] Settings saved to %s", PLUGIN_NAME, configFilePath.c_str());
}

void MultistreamPlugin::LoadSettings() {
    // Get OBS config path
    char* configPath = obs_frontend_get_global_config_path();
    if (!configPath) return;
    
    // Create config file path
    std::string configFilePath = std::string(configPath) + "/obs-multistream.json";
    bfree(configPath);
    
    // Load from file
    obs_data_t* data = obs_data_create_from_json_file(configFilePath.c_str());
    if (!data) {
        blog(LOG_INFO, "[%s] No existing config file found", PLUGIN_NAME);
        return;
    }
    
    destinations.clear();
    
    obs_data_array_t* destArray = obs_data_get_array(data, "destinations");
    size_t count = obs_data_array_count(destArray);
    
    for (size_t i = 0; i < count; i++) {
        obs_data_t* destData = obs_data_array_item(destArray, i);
        
        StreamDestination dest;
        dest.name = obs_data_get_string(destData, "name");
        dest.url = obs_data_get_string(destData, "url");
        dest.key = obs_data_get_string(destData, "key");
        dest.enabled = obs_data_get_bool(destData, "enabled");
        dest.useMainEncoder = obs_data_get_bool(destData, "useMainEncoder");
        dest.bitrate = (int)obs_data_get_int(destData, "bitrate");
        
        destinations.push_back(dest);
        obs_data_release(destData);
    }
    
    obs_data_array_release(destArray);
    obs_data_release(data);
    
    blog(LOG_INFO, "[%s] Loaded %zu destinations from config", 
         PLUGIN_NAME, destinations.size());
}

void MultistreamPlugin::OnMainStreamingStarted(enum obs_frontend_event event, void* data) {
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(data);
    
    // Automatically start multistream when main streaming starts
    MultistreamPlugin::GetInstance()->StartStreaming();
}

void MultistreamPlugin::OnMainStreamingStopped(enum obs_frontend_event event, void* data) {
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(data);
    
    // Automatically stop multistream when main streaming stops
    MultistreamPlugin::GetInstance()->StopStreaming();
}

// ============================================================================
// Helper Functions
// ============================================================================

const char* GetPluginVersion() {
    return PLUGIN_VERSION;
}

const char* GetPluginName() {
    return PLUGIN_NAME;
}

// ============================================================================
// Module Exports
// ============================================================================

extern "C" {

EXPORT bool obs_module_load(void) {
    blog(LOG_INFO, "[%s] Loading module", PLUGIN_NAME);
    
    MultistreamPlugin* plugin = MultistreamPlugin::GetInstance();
    if (!plugin->Initialize()) {
        blog(LOG_ERROR, "[%s] Failed to initialize plugin", PLUGIN_NAME);
        return false;
    }
    
    return true;
}

EXPORT void obs_module_unload(void) {
    blog(LOG_INFO, "[%s] Unloading module", PLUGIN_NAME);
    
    MultistreamPlugin* plugin = MultistreamPlugin::GetInstance();
    if (plugin) {
        // Use public cleanup method instead of accessing private members
        plugin->Cleanup();
    }
}

EXPORT MODULE_EXPORT const char* obs_module_name(void) {
    return "OBS Multistream Plugin";
}

EXPORT MODULE_EXPORT const char* obs_module_description(void) {
    return "Plugin for streaming to multiple RTMP destinations and YouTube channels simultaneously";
}

} 