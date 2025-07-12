# Amalgam - Competitive Fork

[![GitHub Repo stars](https://img.shields.io/github/stars/coffeegrind123/Amalgam-Comp)](/../../stargazers)
[![Matrix](https://img.shields.io/matrix/amalgam-comp:matrix.org?server_fqdn=matrix.org&logo=element&label=matrix)](https://matrix.to/#/#amalgam-comp:matrix.org)
[![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/coffeegrind123/Amalgam-Comp/msbuild.yml?branch=master)](/../../actions)
[![GitHub commit activity (branch)](https://img.shields.io/github/commit-activity/m/coffeegrind123/Amalgam-Comp)](/../../commits/)

A competitive-focused fork featuring enhanced information gathering and advantage tracking systems.

<p align="center">
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64Release.zip">
    <img src=".github/assets/download.png" alt="Download Button" width="400" height="auto" align="center">
  </a>
  <br/><br/>
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64ReleaseAVX2.zip">
    <img src=".github/assets/download_avx2.png" alt="Download AVX2" width="auto" height="auto">
  </a>
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64ReleaseFreetype.zip">
    <img src=".github/assets/freetype.png" alt="Download Freetype" width="auto" height="auto">
  </a>
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64ReleaseFreetypeAVX2.zip">
    <img src=".github/assets/freetype_avx2.png" alt="Download Freetype AVX2" width="auto" height="auto">
  </a>
</p>

###### AVX2 may be faster than SSE2 though not all CPUs support it (`Steam > Help > System Information > Processor Information > AVX2`). Freetype uses freetype as the text rasterizer and includes some custom fonts, which results in better looking text but larger DLL sizes.
###### If nightly.link is down, you can still download through [github](https://github.com/coffeegrind123/Amalgam-Comp/actions) with an account.

## Added Features

### Competitive Information Systems
All features are fully configurable through the COMP tab menu with individual toggles and customization options.

| Feature | Category | Description |
|---------|----------|-------------|
| **UberTracker** | Medical | Comprehensive uber advantage tracking with detailed medic information, weapon types, and advantage calculations |
| **HealthBarESP** | Visual | Health bars for visible-only players. Includes medic mode, health-responsive visibility, and overheal display |
| **CritHeals Indicator** | Medical | Triangle indicators above players eligible for crit heals (medic-only) with uber build rate warnings |
| **PlayerTrails** | Movement | Colored movement trails for enemy players showing recent paths with visibility-based display and fade-out |
| **SentryESP** | Buildings | Advanced sentry ESP with aim lines, targeting detection, chams, level indicators, and color-coded threat system |
| **StickyESP** | Explosives | Stickybomb ESP with 2D/3D boxes, visibility-based coloring, distance filtering, and chams support |
| **FocusFire** | Combat | Multi-targeting detection highlighting enemies focused by multiple teammates (2+ attackers within 4.5s) |
| **PylonESP** | Visual | Vertical pylon indicators above enemy medics behind walls with segmented alpha fade (800+ unit range) |
| **AmmoTracker** | Resources | Supply respawn ESP with visual countdown timers, pie charts, timer text scaling, and distance filtering |
| **MarkSpot** | Communication | Team coordinate marking via Matrix integration. Press E to create shared visual markers with team |
| **SplashRadius** | Combat | Enhanced splash damage visualization with filled polygons showing rocket/pipebomb damage radius |
| **CraterCheck** | Movement | Fall damage prediction showing red (lethal) or green (safe) landing zones using TF2's exact damage formula |
| **EnemyCam** | Spectating | Picture-in-picture camera showing enemy perspectives with multiple targeting modes and configurable window |
| **StickyCam** | Spectating | Demoman sticky bomb security camera with manual/auto modes, enemy tracking, and damage radius chams |
| **OffScreenIndicators** | Visual | Steam avatar-based indicators for off-screen enemies with names, classes, health, and distance info |
| **SpectateAll** | Spectating | Advanced spectating system with enemy cam, free movement (WASDQE), and first/third person toggle |
| **ScoreboardRevealer** | Information | Enhanced scoreboard revealing hidden enemy player information for strategic analysis |
| **FlatTextures** | Visual | Reduces visual clutter by flattening map textures for improved enemy visibility |
| **Match HUD Enhancement** | Interface | Enemy health and class information displayed on match scoreboard/HUD for instant assessment |
| **Safe Bunnyhop** | Movement | Advanced movement assistance with configurable success rate and safety mechanisms |
| **Disable Freezecam** | Spectating | Disables freezecam delay after death for instant spectating and immediate tactical awareness |
| **No Hats** | Visual | Removes all cosmetic items and hats from all players for improved visibility and reduced clutter |
| **Hider ESP** | Visual | Highlights stationary enemy players with orange boxes after 5.5 seconds of no movement for camping detection |

### Communication & Coordination

- **Matrix Chat Integration** - Full Matrix client with end-to-end encryption built directly into TF2
  - **Real E2E Encryption** - Complete libolm integration with Megolm group encryption and Olm device-to-device encryption
  - **Multi-User Support** - Device key management, session key sharing, and cross-device message decryption
  - **TF2 Chat Display** - Messages appear directly in TF2 chat with proper color formatting and usernames
  - **Send with !! Prefix** - Type `!!message` in TF2 chat to send to Matrix (blocked from game chat when connected, passes through when not connected)
  - **Optional Timestamps** - Configurable timestamp display in [HH:MM:SS] format
  - **Production Ready** - Thread-safe background sync with Matrix homeservers
  - **Auto-Discovery** - Automatic account creation, room discovery, and encryption initialization

  Configure via CHAT tab: set server, username, password, space, and room. Default room changed from 'chat' to 'talk'. Messages display as: `[Matrix] @username: message`

## Development Setup

### Quick Start
1. **Run the automated setup:**
   ```cmd
   setup.bat
   ```
2. **Open the project:**
   - Open `Amalgam.sln` in Visual Studio 2022
   - Select configuration: `Release` and platform: `x64`
   - Build the solution

For detailed setup instructions, see [DEVELOPMENT_SETUP.md](DEVELOPMENT_SETUP.md).

### Lua Script Porting
For porting Lua scripts to C++, see the comprehensive [LUA_TO_CPP_PORTING_GUIDE.md](LUA_TO_CPP_PORTING_GUIDE.md) with detailed examples, API mappings, and common fixes for successful integration.

## Installation

**AmalgamLoader.exe** (included with downloads) is the recommended injection tool. It's automatically built and includes all necessary dependencies.

Alternative: [Xenos](https://github.com/DarthTon/Xenos/releases) also works if you prefer a separate injector.
