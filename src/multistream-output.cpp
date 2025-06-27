#include "multistream-output.h"
#include "obs-multistream.h"
#include <obs-frontend-api.h>
#include <util/platform.h>
#include <util/dstr.h>

// Static instance for SharedEncoderManager
SharedEncoderManager* SharedEncoderManager::instance = nullptr;

// ============================================================================
// MultistreamOutput Implementation
// ============================================================================

MultistreamOutput::MultistreamOutput() 
    : output(nullptr), videoEncoder(nullptr), audioEncoder(nullptr), 
      service(nullptr), isInitialized(false), isActive(false),
      isConnecting(false), isReconnecting(false) {
}

MultistreamOutput::~MultistreamOutput() {
    Stop();
    
    if (service) {
        obs_service_release(service);
        service = nullptr;
    }
    
    if (output) {
        obs_output_release(output);
        output = nullptr;
    }
    
    // Note: We don't release encoders here as they might be shared
    // The SharedEncoderManager handles encoder lifecycle
}

bool MultistreamOutput::Initialize(const StreamDestination& dest) {
    destination = dest;
    
    if (!CreateOutput()) {
        blog(LOG_ERROR, "[multistream] Failed to create output for %s", dest.name.c_str());
        return false;
    }
    
    if (!CreateEncoder()) {
        blog(LOG_ERROR, "[multistream] Failed to create encoder for %s", dest.name.c_str());
        return false;
    }
    
    if (!SetupService()) {
        blog(LOG_ERROR, "[multistream] Failed to setup service for %s", dest.name.c_str());
        return false;
    }
    
    ConnectSignalHandlers();
    isInitialized = true;
    
    blog(LOG_INFO, "[multistream] Initialized output for %s", dest.name.c_str());
    return true;
}

bool MultistreamOutput::CreateOutput() {
    output = obs_output_create("rtmp_output", destination.name.c_str(), nullptr, nullptr);
    if (!output) {
        blog(LOG_ERROR, "[multistream] Failed to create RTMP output");
        return false;
    }
    
    return true;
}

bool MultistreamOutput::CreateEncoder() {
    SharedEncoderManager* manager = SharedEncoderManager::GetInstance();
    
    if (destination.useMainEncoder) {
        // Use shared encoders from main output
        videoEncoder = manager->GetSharedVideoEncoder();
        audioEncoder = manager->GetSharedAudioEncoder();
        
        if (!videoEncoder || !audioEncoder) {
            blog(LOG_ERROR, "[multistream] Failed to get shared encoders");
            return false;
        }
    } else {
        // Create custom encoders with specific bitrate
        videoEncoder = manager->CreateCustomVideoEncoder(destination.bitrate);
        audioEncoder = manager->CreateCustomAudioEncoder(destination.bitrate);
        
        if (!videoEncoder || !audioEncoder) {
            blog(LOG_ERROR, "[multistream] Failed to create custom encoders");
            return false;
        }
    }
    
    return true;
}

bool MultistreamOutput::SetupService() {
    service = RTMPService::CreateService(destination.url, destination.key);
    if (!service) {
        blog(LOG_ERROR, "[multistream] Failed to create RTMP service");
        return false;
    }
    
    obs_output_set_service(output, service);
    return true;
}

bool MultistreamOutput::Start() {
    if (!isInitialized) {
        blog(LOG_ERROR, "[multistream] Cannot start uninitialized output");
        return false;
    }
    
    if (isActive) {
        blog(LOG_WARNING, "[multistream] Output already active for %s", destination.name.c_str());
        return true;
    }
    
    // Set encoders
    obs_output_set_video_encoder(output, videoEncoder);
    obs_output_set_audio_encoder(output, audioEncoder, 0);
    
    // Start the output
    bool result = obs_output_start(output);
    if (result) {
        isActive = true;
        isConnecting = true;
        blog(LOG_INFO, "[multistream] Started streaming to %s", destination.name.c_str());
    } else {
        blog(LOG_ERROR, "[multistream] Failed to start streaming to %s", destination.name.c_str());
    }
    
    return result;
}

void MultistreamOutput::Stop() {
    if (!isActive) return;
    
    DisconnectSignalHandlers();
    
    if (output && obs_output_active(output)) {
        obs_output_stop(output);
    }
    
    isActive = false;
    isConnecting = false;
    isReconnecting = false;
    
    blog(LOG_INFO, "[multistream] Stopped streaming to %s", destination.name.c_str());
}

bool MultistreamOutput::IsActive() const {
    return isActive && output && obs_output_active(output);
}

bool MultistreamOutput::IsConnecting() const {
    return isConnecting;
}

bool MultistreamOutput::IsReconnecting() const {
    return isReconnecting;
}

const StreamDestination& MultistreamOutput::GetDestination() const {
    return destination;
}

std::string MultistreamOutput::GetStatusString() const {
    if (!isActive) return "Inactive";
    if (isReconnecting) return "Reconnecting";
    if (isConnecting) return "Connecting";
    if (!lastError.empty()) return "Error: " + lastError;
    return "Streaming";
}

uint64_t MultistreamOutput::GetTotalBytes() const {
    if (!output) return 0;
    return obs_output_get_total_bytes(output);
}

int MultistreamOutput::GetDroppedFrames() const {
    if (!output) return 0;
    return obs_output_get_frames_dropped(output);
}

float MultistreamOutput::GetCongestion() const {
    if (!output) return 0.0f;
    return obs_output_get_congestion(output);
}

void MultistreamOutput::ConnectSignalHandlers() {
    if (!output) return;
    
    signal_handler_t* handler = obs_output_get_signal_handler(output);
    if (handler) {
        signal_handler_connect(handler, "start", OnStarted, this);
        signal_handler_connect(handler, "stop", OnStopped, this);
        signal_handler_connect(handler, "reconnect", OnReconnecting, this);
        signal_handler_connect(handler, "reconnect_success", OnReconnected, this);
    }
}

void MultistreamOutput::DisconnectSignalHandlers() {
    if (!output) return;
    
    signal_handler_t* handler = obs_output_get_signal_handler(output);
    if (handler) {
        signal_handler_disconnect(handler, "start", OnStarted, this);
        signal_handler_disconnect(handler, "stop", OnStopped, this);
        signal_handler_disconnect(handler, "reconnect", OnReconnecting, this);
        signal_handler_disconnect(handler, "reconnect_success", OnReconnected, this);
    }
}

void MultistreamOutput::OnStarted(void* data, calldata_t* cd) {
    MultistreamOutput* output = static_cast<MultistreamOutput*>(data);
    output->isConnecting = false;
    blog(LOG_INFO, "[multistream] Stream started for %s", output->destination.name.c_str());
}

void MultistreamOutput::OnStopped(void* data, calldata_t* cd) {
    MultistreamOutput* output = static_cast<MultistreamOutput*>(data);
    output->isActive = false;
    output->isConnecting = false;
    output->isReconnecting = false;
    
    int code = (int)calldata_int(cd, "code");
    if (code != OBS_OUTPUT_SUCCESS) {
        const char* error = calldata_string(cd, "error");
        output->lastError = error ? error : "Unknown error";
        blog(LOG_ERROR, "[multistream] Stream stopped with error for %s: %s", 
             output->destination.name.c_str(), output->lastError.c_str());
    } else {
        output->lastError.clear();
        blog(LOG_INFO, "[multistream] Stream stopped for %s", output->destination.name.c_str());
    }
}

void MultistreamOutput::OnReconnecting(void* data, calldata_t* cd) {
    MultistreamOutput* output = static_cast<MultistreamOutput*>(data);
    output->isReconnecting = true;
    blog(LOG_INFO, "[multistream] Reconnecting stream for %s", output->destination.name.c_str());
}

void MultistreamOutput::OnReconnected(void* data, calldata_t* cd) {
    MultistreamOutput* output = static_cast<MultistreamOutput*>(data);
    output->isReconnecting = false;
    blog(LOG_INFO, "[multistream] Reconnected stream for %s", output->destination.name.c_str());
}

// ============================================================================
// SharedEncoderManager Implementation
// ============================================================================

SharedEncoderManager::SharedEncoderManager() {
}

SharedEncoderManager::~SharedEncoderManager() {
}

SharedEncoderManager* SharedEncoderManager::GetInstance() {
    if (!instance) {
        instance = new SharedEncoderManager();
    }
    return instance;
}

obs_encoder_t* SharedEncoderManager::GetSharedVideoEncoder() {
    // Get the main streaming output's video encoder
    obs_output_t* streamOutput = obs_frontend_get_streaming_output();
    if (!streamOutput) {
        blog(LOG_ERROR, "[multistream] No main streaming output available");
        return nullptr;
    }
    
    obs_encoder_t* encoder = obs_output_get_video_encoder(streamOutput);
    obs_output_release(streamOutput);
    
    return encoder;
}

obs_encoder_t* SharedEncoderManager::GetSharedAudioEncoder() {
    // Get the main streaming output's audio encoder
    obs_output_t* streamOutput = obs_frontend_get_streaming_output();
    if (!streamOutput) {
        blog(LOG_ERROR, "[multistream] No main streaming output available");
        return nullptr;
    }
    
    obs_encoder_t* encoder = obs_output_get_audio_encoder(streamOutput, 0);
    obs_output_release(streamOutput);
    
    return encoder;
}

obs_encoder_t* SharedEncoderManager::CreateCustomVideoEncoder(int bitrate) {
    obs_data_t* settings = CreateVideoEncoderSettings(bitrate);
    obs_encoder_t* encoder = obs_video_encoder_create("obs_x264", "multistream_video", settings, nullptr);
    obs_data_release(settings);
    
    if (!encoder) {
        blog(LOG_ERROR, "[multistream] Failed to create custom video encoder");
        return nullptr;
    }
    
    // Set video info from main output
    obs_video_info ovi;
    if (obs_get_video_info(&ovi)) {
        obs_encoder_set_video(encoder, obs_get_video());
    }
    
    return encoder;
}

obs_encoder_t* SharedEncoderManager::CreateCustomAudioEncoder(int bitrate) {
    obs_data_t* settings = CreateAudioEncoderSettings(bitrate);
    obs_encoder_t* encoder = obs_audio_encoder_create("ffmpeg_aac", "multistream_audio", settings, 0, nullptr);
    obs_data_release(settings);
    
    if (!encoder) {
        blog(LOG_ERROR, "[multistream] Failed to create custom audio encoder");
        return nullptr;
    }
    
    // Set audio info from main output
    obs_audio_info oai;
    if (obs_get_audio_info(&oai)) {
        obs_encoder_set_audio(encoder, obs_get_audio());
    }
    
    return encoder;
}

void SharedEncoderManager::ReleaseCustomEncoder(obs_encoder_t* encoder) {
    if (encoder) {
        obs_encoder_release(encoder);
    }
}

obs_data_t* SharedEncoderManager::CreateVideoEncoderSettings(int bitrate) {
    obs_data_t* settings = obs_data_create();
    
    obs_data_set_int(settings, "bitrate", bitrate);
    obs_data_set_string(settings, "rate_control", "CBR");
    obs_data_set_int(settings, "keyint_sec", 2);
    obs_data_set_string(settings, "preset", "veryfast");
    obs_data_set_string(settings, "profile", "baseline");
    obs_data_set_string(settings, "tune", "zerolatency");
    
    return settings;
}

obs_data_t* SharedEncoderManager::CreateAudioEncoderSettings(int bitrate) {
    obs_data_t* settings = obs_data_create();
    
    // Calculate audio bitrate (typically much lower than video)
    int audioBitrate = std::min(320, std::max(64, bitrate / 10));
    obs_data_set_int(settings, "bitrate", audioBitrate);
    
    return settings;
}

// ============================================================================
// RTMPService Implementation
// ============================================================================

obs_service_t* RTMPService::CreateService(const std::string& url, const std::string& key) {
    obs_data_t* settings = CreateServiceSettings(url, key);
    obs_service_t* service = obs_service_create("rtmp_custom", "multistream_service", settings, nullptr);
    obs_data_release(settings);
    
    if (!service) {
        blog(LOG_ERROR, "[multistream] Failed to create RTMP service for %s", url.c_str());
    }
    
    return service;
}

void RTMPService::ReleaseService(obs_service_t* service) {
    if (service) {
        obs_service_release(service);
    }
}

obs_data_t* RTMPService::CreateServiceSettings(const std::string& url, const std::string& key) {
    obs_data_t* settings = obs_data_create();
    
    obs_data_set_string(settings, "server", url.c_str());
    obs_data_set_string(settings, "key", key.c_str());
    
    return settings;
} 