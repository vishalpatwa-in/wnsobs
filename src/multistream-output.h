#pragma once

#include <obs.h>
#include <obs-output.h>
#include <string>

// Include StreamDestination definition
#include "stream-destination.h"

// Class for managing individual RTMP outputs
class MultistreamOutput {
public:
    MultistreamOutput();
    ~MultistreamOutput();
    
    // Initialize with a stream destination
    bool Initialize(const StreamDestination& destination);
    
    // Start/stop streaming
    bool Start();
    void Stop();
    
    // Status checking
    bool IsActive() const;
    bool IsConnecting() const;
    bool IsReconnecting() const;
    
    // Get stream information
    const StreamDestination& GetDestination() const;
    std::string GetStatusString() const;
    
    // Statistics
    uint64_t GetTotalBytes() const;
    int GetDroppedFrames() const;
    float GetCongestion() const;
    
private:
    // OBS output management
    bool CreateOutput();
    bool CreateEncoder();
    bool SetupService();
    
    // Event handlers
    static void OnStarted(void* data, calldata_t* cd);
    static void OnStopped(void* data, calldata_t* cd);
    static void OnReconnecting(void* data, calldata_t* cd);
    static void OnReconnected(void* data, calldata_t* cd);
    
    // Connect signal handlers
    void ConnectSignalHandlers();
    void DisconnectSignalHandlers();
    
    StreamDestination destination;
    
    obs_output_t* output;
    obs_encoder_t* videoEncoder;
    obs_encoder_t* audioEncoder;
    obs_service_t* service;
    
    bool isInitialized;
    bool isActive;
    
    // Status tracking
    bool isConnecting;
    bool isReconnecting;
    std::string lastError;
};

// Helper class for managing shared encoders
class SharedEncoderManager {
public:
    static SharedEncoderManager* GetInstance();
    
    // Get shared encoders from main output
    obs_encoder_t* GetSharedVideoEncoder();
    obs_encoder_t* GetSharedAudioEncoder();
    
    // Create custom encoder with specified bitrate
    obs_encoder_t* CreateCustomVideoEncoder(int bitrate);
    obs_encoder_t* CreateCustomAudioEncoder(int bitrate);
    
    // Release custom encoders
    void ReleaseCustomEncoder(obs_encoder_t* encoder);
    
private:
    SharedEncoderManager();
    ~SharedEncoderManager();
    
    static SharedEncoderManager* instance;
    
    // Encoder settings helpers
    obs_data_t* CreateVideoEncoderSettings(int bitrate);
    obs_data_t* CreateAudioEncoderSettings(int bitrate);
};

// RTMP service helper
class RTMPService {
public:
    static obs_service_t* CreateService(const std::string& url, const std::string& key);
    static void ReleaseService(obs_service_t* service);
    
private:
    static obs_data_t* CreateServiceSettings(const std::string& url, const std::string& key);
}; 