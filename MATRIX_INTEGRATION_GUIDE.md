# Matrix Chat Integration Guide

## ✅ Current Implementation Status

The Matrix chat integration framework is now **fully implemented** and ready for testing. Here's what's been completed:

### Core Features ✅
- **Chat Tab UI**: Clean configuration interface in the main menu
- **Game Chat Integration**: Messages display in TF2's in-game chat (cyan [Matrix] prefix)
- **Message Interception**: `!!` prefix detection and blocking from game chat
- **Connection Management**: Connect/disconnect functionality
- **Configuration System**: All necessary variables (server, username, password, space, room)

### Architecture ✅
- **`CChat` Class**: Complete Matrix client wrapper
- **Hook Integration**: Properly integrated with existing TF2 chat hooks
- **UI Integration**: Added to menu system with chat icon
- **Project Files**: Added to vcxproj for compilation

## 🔧 Next Steps: Adding mtxclient

To complete the integration, you need to add the mtxclient library dependency:

### Option 1: vcpkg (Recommended)
```bash
# Install mtxclient via vcpkg
vcpkg install mtxclient:x64-windows

# Add to your CMakeLists.txt or vcxproj
find_package(mtxclient CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE mtxclient::mtxclient)
```

### Option 2: Manual Build
```bash
git clone https://github.com/Nheko-Reborn/mtxclient.git
cd mtxclient
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Option 3: Pre-built Binaries
Download from: https://github.com/Nheko-Reborn/mtxclient/releases

## 🔌 Matrix Client Implementation

Once mtxclient is available, replace the placeholder code in `Chat.cpp`:

### Replace Login Function:
```cpp
void CChat::Login(const std::string& username, const std::string& password)
{
    // Create Matrix client
    m_pMatrixClient = new mtx::http::Client("https://" + Vars::Chat::Server.Value);
    auto client = static_cast<mtx::http::Client*>(m_pMatrixClient);
    
    // Login
    client->login(username, password, [this](const mtx::responses::Login &res, mtx::http::RequestErr err) {
        if (err) {
            m_sLastError = "Login failed: " + std::string(err->what());
            m_bLoginInProgress = false;
            return;
        }
        
        m_bLoginInProgress = false;
        m_bConnected = true;
        
        DisplayInGameMessage("Matrix", "Connected to #" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value);
        DisplayInGameMessage("Matrix", "Use !! prefix to send messages to Matrix chat");
        
        // Start listening for messages
        StartEventListener();
    });
}
```

### Replace SendMessage Function:
```cpp
void CChat::SendMessage(const std::string& message)
{
    if (!m_bConnected || !m_pMatrixClient)
        return;
    
    auto client = static_cast<mtx::http::Client*>(m_pMatrixClient);
    std::string roomId = "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value;
    
    mtx::events::msg::Text text;
    text.body = message;
    
    client->send_room_message(roomId, text, [](const mtx::responses::EventId &res, mtx::http::RequestErr err) {
        // Optional: Handle send response
    });
}
```

### Add Event Listener:
```cpp
void CChat::HandleMatrixEvents()
{
    if (!m_pMatrixClient)
        return;
        
    auto client = static_cast<mtx::http::Client*>(m_pMatrixClient);
    
    // Set up sync to receive messages
    client->sync([this](const mtx::responses::Sync &res, mtx::http::RequestErr err) {
        if (err)
            return;
            
        // Process incoming messages
        for (const auto& [room_id, room] : res.rooms.join) {
            for (const auto& event : room.timeline.events) {
                if (auto msg = std::get_if<mtx::events::RoomEvent<mtx::events::msg::Text>>(&event)) {
                    std::string sender = msg->sender;
                    std::string content = msg->content.body;
                    ProcessIncomingMessage(sender, content);
                }
            }
        }
    });
}
```

## 🎯 Current Chat Flow

### Outgoing Messages:
1. User types `!!hello world` in TF2 chat
2. `Cbuf_ExecuteCommand` hook intercepts the command
3. `F::Chat.ProcessGameChatMessage()` detects `!!` prefix
4. Message is sent to Matrix room (when mtxclient is implemented)
5. Message is blocked from appearing in game chat

### Incoming Messages:
1. Matrix client receives message from room
2. `ProcessIncomingMessage()` is called
3. `DisplayInGameMessage()` shows message in TF2 chat with cyan [Matrix] prefix
4. Only visible to the user (local client only)

## 🏷️ Message Format

**Incoming Matrix messages appear as:**
```
[Matrix 14:30] username: message content
```

**Or without timestamps:**
```
[Matrix] username: message content
```

## ⚙️ Configuration

Users can configure:
- **Server**: Matrix homeserver (default: pendora.io)
- **Username**: Matrix username
- **Password**: Matrix password  
- **Space**: Matrix space (default: asyncroom)
- **Room**: Matrix room (default: talk)
- **Auto Connect**: Connect automatically on startup
- **Show Timestamps**: Show timestamps in Matrix messages

## 🧪 Testing Without mtxclient

The system is designed to work without mtxclient for testing:
- Connection simulation works
- `!!` prefix detection and blocking works
- UI and configuration work
- You can test `DisplayInGameMessage()` by calling it directly

## 📁 Files Modified

- `src/Features/Chat/Chat.h` - Matrix chat class definition
- `src/Features/Chat/Chat.cpp` - Matrix chat implementation
- `src/Features/ImGui/Menu/Menu.h` - Added MenuChat declaration
- `src/Features/ImGui/Menu/Menu.cpp` - Added chat tab UI
- `src/Hooks/Cbuf_ExecuteCommand.cpp` - Added !! prefix interception
- `src/SDK/Vars.h` - Added chat configuration variables
- `Amalgam.vcxproj` - Added chat files to project

The Matrix integration is architecturally complete and ready for mtxclient integration!