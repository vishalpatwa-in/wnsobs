# OBS Studio Multistream Plugin

A native OBS Studio plugin for Windows 11 that enables simultaneous streaming to multiple RTMP endpoints and YouTube channels.

## Features

- 🔀 **Multiple RTMP Destinations**: Stream to multiple platforms simultaneously
- 📺 **YouTube Support**: Multiple YouTube stream support with manual RTMP configuration
- ⚡ **Shared Encoding**: Option to use main stream encoder for efficiency
- 🎛️ **Custom Encoding**: Individual encoder settings per destination
- 🖥️ **Native Integration**: Uses OBS built-in property dialogs (no Qt dependencies)
- 💾 **Persistent Configuration**: JSON-based settings storage

## Technical Requirements

- **Operating System**: Windows 11 (64-bit)
- **OBS Studio**: Latest version with plugin support
- **Development**: Visual Studio 2022 (for building from source)

## Installation

### Option 1: Pre-built DLL (Recommended)

1. Download the latest `obs-multistream.dll` from the releases
2. Copy `obs-multistream.dll` to your OBS Studio plugins directory:
   ```
   C:\Program Files\obs-studio\obs-plugins\64bit\
   ```
3. Restart OBS Studio
4. The "Multistream" dock should appear in the Docks menu

### Option 2: Build from Source

#### Prerequisites

1. **Visual Studio 2022** with C++ development tools
2. **OBS Studio** installed to default location (`C:\Program Files\obs-studio\`)

#### Build Steps

1. Clone or download this repository
2. Open `obs-multistream.sln` in Visual Studio 2022
3. Set configuration to **Release** and platform to **x64**
4. Build the solution (Build → Build Solution)
5. The compiled DLL will be in `bin\x64\Release\obs-multistream.dll`
6. Copy the DLL to OBS plugins directory as above

## Usage

### Setting Up Stream Destinations

1. **Open the Multistream Dock**:
   - In OBS Studio, go to View → Docks → Multistream
   - The Multistream dock will appear with configuration options

2. **Add a New Destination**:
   - Click "Add Destination" in the dock
   - Choose from preset platforms or enter custom RTMP details:
     - **Name**: Friendly name for the destination
     - **RTMP URL**: Server URL (e.g., `rtmp://live.twitch.tv/live/`)
     - **Stream Key**: Your platform-specific stream key
     - **Enabled**: Check to include in multistream
     - **Use Main Encoder**: Use shared encoding for efficiency
     - **Bitrate**: Custom bitrate (if not using main encoder)

3. **Supported Preset Platforms**:
   - **Twitch**: `rtmp://live.twitch.tv/live/`
   - **YouTube**: `rtmp://a.rtmp.youtube.com/live2/`
   - **Facebook**: `rtmps://live-api-s.facebook.com:443/rtmp/`
   - **TikTok**: `rtmp://push.tiktokcdn.com/live/`

### Starting Multistream

1. **Automatic Mode** (Recommended):
   - Configure your destinations in the dock
   - Start your normal OBS stream
   - The plugin automatically starts multistreaming to all enabled destinations

2. **Manual Mode**:
   - Use the "Start Multistream" button in the dock
   - Control multistreaming independently of main OBS stream

### Managing Destinations

- **Edit**: Select a destination and click "Edit Selected"
- **Remove**: Select a destination and click "Remove Selected"
- **Enable/Disable**: Toggle destinations without deleting them
- **Refresh**: Update the dock display and status

## Configuration

### Encoder Settings

- **Shared Encoding**: Uses the same encoder as your main OBS stream (recommended for performance)
- **Custom Encoding**: Creates separate encoders with individual bitrate settings

### Settings Storage

Configuration is automatically saved to:
```
%APPDATA%\obs-studio\obs-multistream.json
```

## Troubleshooting

### Common Issues

1. **Plugin not loading**:
   - Ensure DLL is in correct plugins directory
   - Check OBS Studio version compatibility
   - Verify Windows 11 x64 architecture

2. **Stream connection failures**:
   - Verify RTMP URLs and stream keys
   - Check network connectivity
   - Ensure platforms support simultaneous streams

3. **Performance issues**:
   - Use shared encoding when possible
   - Reduce bitrates for additional streams
   - Monitor CPU usage in OBS

### Logging

Plugin logs appear in OBS Studio logs with `[obs-multistream]` or `[multistream]` prefixes. Enable logging in OBS Help → Log Files.

## Development

### Architecture

- **MultistreamPlugin**: Main singleton managing plugin lifecycle
- **MultistreamOutput**: Individual RTMP output management  
- **MultistreamDock**: UI integration using OBS property dialogs
- **SharedEncoderManager**: Encoder sharing and custom encoder creation

### Key Features

- Event-driven architecture with OBS streaming lifecycle integration
- Win32 dialog helpers instead of Qt dependencies
- JSON configuration persistence
- Proper OBS module exports and lifecycle management

### Building

The project uses Visual Studio project files (.sln/.vcxproj) without CMake:

```bash
# Open in Visual Studio 2022
obs-multistream.sln

# Or build from command line
msbuild obs-multistream.sln /p:Configuration=Release /p:Platform=x64
```

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]

## Support

[Add support contact information here] 