# Lua to C++ Porting Guide for Amalgam

This comprehensive guide will walk you through porting Lua scripts (like uber2.lua) into the Amalgam C++ codebase. This guide is based on the successful porting of uber2.lua and covers all the quirks, fixes, and SDK integration points you'll encounter.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Understanding the Amalgam Architecture](#understanding-the-amalgam-architecture)
3. [Initial Analysis](#initial-analysis)
4. [Setting Up the Feature Structure](#setting-up-the-feature-structure)
5. [Common API Mappings](#common-api-mappings)
6. [SDK Integration Points](#sdk-integration-points)
7. [Drawing and Rendering](#drawing-and-rendering)
8. [Project Integration](#project-integration)
9. [Common Issues and Fixes](#common-issues-and-fixes)
10. [Testing and Debugging](#testing-and-debugging)

## Prerequisites

Before starting, ensure you have:
- Basic understanding of C++ and the Amalgam codebase structure
- The Lua script you want to port
- Visual Studio with the Amalgam solution loaded
- Understanding of TF2 game mechanics related to your script

## Understanding the Amalgam Architecture

### Project Structure
```
Amalgam/
├── src/
│   ├── Features/           # All cheat features
│   │   ├── Visuals/       # Visual features (ESP, Chams, etc.)
│   │   ├── Aimbot/        # Aimbot features
│   │   ├── Misc/          # Miscellaneous features
│   │   └── ...
│   ├── SDK/               # TF2 SDK definitions and helpers
│   │   ├── Definitions/   # Class definitions, interfaces
│   │   ├── Helpers/       # Drawing, entities, utilities
│   │   └── Vars.h         # Configuration variables
│   ├── Hooks/             # Game function hooks
│   └── Utils/             # Utility classes
```

### Feature Integration Pattern
All features follow this pattern:
1. **Header file** (`FeatureName.h`) - Class definition with `ADD_FEATURE` macro
2. **Implementation file** (`FeatureName.cpp`) - Feature logic
3. **Hook integration** - Added to appropriate hook (usually `IEngineVGui_Paint.cpp`)
4. **Project file integration** - Added to `Amalgam.vcxproj`

## Initial Analysis

### 1. Understand the Lua Script Purpose
Before porting, analyze:
- **What game data does it access?** (entities, netvars, etc.)
- **What does it display?** (text, boxes, colors)
- **When does it run?** (every frame, on events, etc.)
- **What are the key algorithms?** (calculations, state tracking)

### 2. Identify TF2 Game Concepts
Map Lua concepts to TF2 SDK equivalents:
- **Entities** → `CBaseEntity`, `CTFPlayer`, `CTFWeaponBase`
- **Classes** → `TF_CLASS_MEDIC`, `TF_CLASS_SOLDIER`, etc.
- **Teams** → `TF_TEAM_RED`, `TF_TEAM_BLUE`
- **Weapons** → Weapon IDs, netvars, properties

### 3. Categorize the Feature
Determine where your feature belongs:
- **Visuals** - ESP, trackers, overlays, HUD elements
- **Aimbot** - Targeting, auto-actions
- **Misc** - Game modifications, utilities

## Setting Up the Feature Structure

### 1. Create Feature Directory
```
src/Features/Visuals/YourFeature/
├── YourFeature.h
└── YourFeature.cpp
```

### 2. Basic Header Template
```cpp
#pragma once
#include "../../../SDK/SDK.h"
#include <map>  // Add STL includes as needed

// Data structures for your feature
struct YourDataStruct
{
    std::string SomeData;
    float SomeValue;
    bool SomeFlag;
};

class CYourFeature
{
private:
    // Configuration constants
    static constexpr bool ENABLE_FEATURE = true;
    static constexpr float SOME_THRESHOLD = 50.0f;
    
    // State tracking variables
    std::map<int, float> m_StateTracker;
    std::vector<YourDataStruct> m_DataCache;
    
    // Helper functions
    void ProcessData();
    void DrawElements();
    
public:
    void Draw();  // Main entry point
};

ADD_FEATURE(CYourFeature, YourFeature)
```

### 3. Basic Implementation Template
```cpp
#include "YourFeature.h"
#include "../../../SDK/SDK.h"

void CYourFeature::Draw()
{
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Your feature logic here
    ProcessData();
    DrawElements();
}

void CYourFeature::ProcessData()
{
    // Process game data
}

void CYourFeature::DrawElements()
{
    // Draw visual elements
}
```

## Common API Mappings

### Lua → C++ Entity Access
| Lua Concept | C++ Equivalent | Example |
|-------------|----------------|---------|
| `entities.FindByClass("CTFPlayer")` | `H::Entities.GetGroup(EGroupType::PLAYERS_ALL)` | Get all players |
| `entity:GetClassNum()` | `pPlayer->m_iClass()` | Get player class |
| `entity:GetTeamNumber()` | `pEntity->m_iTeamNum()` | Get team number |
| `entity:IsAlive()` | `pPlayer->IsAlive()` | Check if alive |
| `entity:GetIndex()` | `pEntity->entindex()` | Get entity index |

### Lua → C++ Weapon Access
| Lua Concept | C++ Equivalent | Example |
|-------------|----------------|---------|
| `entity:GetWeaponFromSlot(1)` | `pPlayer->GetWeaponFromSlot(SLOT_SECONDARY)` | Get secondary weapon |
| `weapon:GetItemDefIndex()` | `pWeapon->m_iItemDefinitionIndex()` | Get item definition index |
| `medigun.m_flChargeLevel` | `pMedigun->As<CWeaponMedigun>()->m_flChargeLevel()` | Get uber charge |

### Lua → C++ Constants
| Lua Constant | C++ Constant | Notes |
|--------------|--------------|-------|
| `CLASS_MEDIC` | `TF_CLASS_MEDIC` | Player classes |
| Team numbers (2, 3) | `TF_TEAM_RED`, `TF_TEAM_BLUE` | Team constants |
| `SLOT_SECONDARY` | `SLOT_SECONDARY` | Weapon slots |

## SDK Integration Points

### Essential SDK Headers
The main SDK header includes everything:
```cpp
#include "../../../SDK/SDK.h"
```

### Key SDK Components

#### 1. Entity Management (`H::Entities`)
```cpp
// Get local player
auto pLocal = H::Entities.GetLocal();

// Get all players
for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
{
    auto pPlayer = pEntity->As<CTFPlayer>();
}
```

#### 2. Drawing System (`H::Draw`)
```cpp
// Colors
Color_t red = {255, 0, 0, 255};

// Basic shapes
H::Draw.FillRect(x, y, width, height, color);
H::Draw.LineRect(x, y, width, height, color);

// Text rendering
H::Draw.String(H::Fonts.GetFont(FONT_ESP), x, y, color, ALIGN_CENTER, "Text");
```

#### 3. Global Interfaces (`I::`)
```cpp
// Game globals
float currentTime = I::GlobalVars->curtime;

// Screen size
int screenW, screenH;
I::MatSystemSurface->GetScreenSize(screenW, screenH);

// UI state
if (I::EngineVGui->IsGameUIVisible())
    return;
```

### Entity Class Hierarchy
```
CBaseEntity
├── CBasePlayer
│   └── CTFPlayer          // Players
├── CBaseCombatWeapon
│   └── CTFWeaponBase      // Weapons
│       └── CWeaponMedigun // Medigun specifically
└── CBaseObject            // Buildings
```

### Netvar Access Pattern
```cpp
// Access netvars using the () operator
int teamNum = pEntity->m_iTeamNum();
float charge = pMedigun->m_flChargeLevel();
bool isAlive = pPlayer->IsAlive();  // Some have helper methods
```

## Drawing and Rendering

### Font System
Available fonts:
```cpp
FONT_ESP        // Standard ESP font
FONT_INDICATORS // Smaller indicator font
```

### Text Alignment
```cpp
ALIGN_TOPLEFT    ALIGN_TOP       ALIGN_TOPRIGHT
ALIGN_LEFT       ALIGN_CENTER    ALIGN_RIGHT
ALIGN_BOTTOMLEFT ALIGN_BOTTOM    ALIGN_BOTTOMRIGHT
```

### Color System
```cpp
// RGBA format
Color_t color = {red, green, blue, alpha};  // 0-255 values

// Predefined colors (check Vars::Colors for more)
Vars::Colors::Health.Value
Vars::Colors::TeamRed.Value
Vars::Colors::TeamBlu.Value
```

### Common Drawing Patterns
```cpp
void DrawInfoBox(int x, int y, const std::vector<std::string>& lines)
{
    // Background
    int boxHeight = 25 + (lines.size() * 15);
    H::Draw.FillRect(x-5, y-5, 200, boxHeight, {0, 0, 0, 100});
    
    // Text lines
    for (size_t i = 0; i < lines.size(); ++i)
    {
        int yPos = y + (i * 15);
        H::Draw.String(H::Fonts.GetFont(FONT_ESP), x, yPos, 
                      {255, 255, 255, 255}, ALIGN_TOPLEFT, lines[i].c_str());
    }
}
```

## Project Integration

### 1. Add to Visual Studio Project
Edit `Amalgam.vcxproj`:

**Add source file:**
```xml
<ClCompile Include="src\Features\Visuals\YourFeature\YourFeature.cpp" />
```

**Add header file:**
```xml
<ClInclude Include="src\Features\Visuals\YourFeature\YourFeature.h" />
```

### 2. Hook Integration
Add to `src/Hooks/IEngineVGui_Paint.cpp`:

**Include header:**
```cpp
#include "../Features/Visuals/YourFeature/YourFeature.h"
```

**Add to drawing loop:**
```cpp
if (auto pLocal = H::Entities.GetLocal())
{
    // ... other features ...
    F::YourFeature.Draw();
    // ... more features ...
}
```

### 3. Feature Registration
The `ADD_FEATURE` macro automatically registers your feature. The naming convention:
- Class: `CYourFeature`
- Instance: `F::YourFeature`
- Macro: `ADD_FEATURE(CYourFeature, YourFeature)`

## Common Issues and Fixes

### 1. Missing STL Headers
**Problem:** `std::map`, `std::vector` not recognized
**Solution:** Add includes to header
```cpp
#include <map>
#include <vector>
#include <string>
```

### 2. Weapon ID vs Item Definition Index
**Problem:** `GetWeaponID()` returns weapon class ID, not item definition index
**Solution:** Use netvar for item definition index to distinguish weapon types
```cpp
// Wrong - All mediguns return same ID (50)
int weaponID = pWeapon->GetWeaponID();

// Correct - Distinguishes between weapon types (35=KRITZ, 411=QUICKFIX, etc.)
int itemDefIndex = pWeapon->m_iItemDefinitionIndex();
```

### 3. Player Name Access
**Problem:** `GetName()` method doesn't exist on entities
**Solution:** Use PlayerInfo and PlayerUtils for proper name access
```cpp
// Wrong
std::string name = pPlayer->GetName();

// Correct
PlayerInfo_t pi{};
if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi))
    std::string name = F::PlayerUtils.GetPlayerName(pPlayer->entindex(), pi.name);
else
    std::string name = "Player " + std::to_string(pPlayer->entindex());

// Don't forget to include PlayerUtils
#include "../../Players/PlayerUtils.h"
```

### 4. Drawing Function Issues
**Problem:** `FillRectangle()`, `FONT_MENU` not found
**Solution:** Use correct Amalgam drawing API
```cpp
// Wrong
H::Draw.FillRectangle(...)
// Correct
H::Draw.FillRect(...)

// Wrong
FONT_MENU
// Correct
H::Fonts.GetFont(FONT_ESP)
```

### 5. Team/Class Constants
**Problem:** `CLASS_MEDIC`, team numbers as raw integers
**Solution:** Use TF2 constants
```cpp
// Wrong
if (playerClass == 5)  // Raw number
// Correct
if (pPlayer->m_iClass() == TF_CLASS_MEDIC)

// Wrong
if (team == 2)
// Correct
if (pEntity->m_iTeamNum() == TF_TEAM_RED)
```

### 6. Netvar Access
**Problem:** Direct member access failing
**Solution:** Use netvar accessor methods
```cpp
// Wrong (direct access)
pMedigun->m_flChargeLevel
// Correct (netvar method)
pMedigun->m_flChargeLevel()
```

### 7. Entity Casting
**Problem:** Method not available on base entity
**Solution:** Cast to specific type
```cpp
// Get entity as specific type
auto pPlayer = pEntity->As<CTFPlayer>();
auto pMedigun = pWeapon->As<CWeaponMedigun>();
```

### 8. Debugging Unknown Values
**Problem:** Need to identify what values are actually being returned
**Solution:** Use console output for debugging
```cpp
// Add temporary debug output to see actual values
I::CVar->ConsolePrintf("DEBUG: Weapon ID = %d\n", pWeapon->GetWeaponID());
I::CVar->ConsolePrintf("DEBUG: Item Def Index = %d\n", pWeapon->m_iItemDefinitionIndex());

// View results in game console (~ key)
// Remove debug output once you identify the correct values
```

## Advanced Patterns

### 1. State Tracking
```cpp
class CYourFeature
{
private:
    std::map<int, float> m_PreviousValues;
    std::map<int, std::string> m_EntityStates;
    
public:
    void UpdateState(int entityIndex, float newValue)
    {
        float oldValue = m_PreviousValues[entityIndex];
        if (newValue != oldValue)
        {
            // Handle state change
            m_EntityStates[entityIndex] = "CHANGED";
        }
        m_PreviousValues[entityIndex] = newValue;
    }
};
```

### 2. Complex Calculations
```cpp
// Timer-based calculations
float timeDelta = I::GlobalVars->curtime - m_LastUpdateTime;
if (timeDelta > UPDATE_INTERVAL)
{
    // Perform expensive calculations
    m_LastUpdateTime = I::GlobalVars->curtime;
}

// Rate calculations (like uber build rates)
float buildRate = baseRate * (isOverhealed ? OVERHEAL_PENALTY : 1.0f);
float projectedUber = currentUber + (buildRate * timeDelta);
```

### 3. Multi-Entity Tracking
```cpp
void CYourFeature::ProcessEntities()
{
    std::vector<EntityData> teamData[4];  // Support up to 4 teams
    
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || !ShouldTrackEntity(pPlayer))
            continue;
            
        EntityData data;
        data.Entity = pPlayer;
        data.Value = GetRelevantValue(pPlayer);
        
        int team = pPlayer->m_iTeamNum();
        if (team >= 0 && team < 4)
            teamData[team].push_back(data);
    }
    
    // Process team-specific data
    AnalyzeTeamData(teamData[TF_TEAM_RED], teamData[TF_TEAM_BLUE]);
}
```

## Testing and Debugging

### 1. Debug Output
```cpp
// Console output for debugging
#ifdef _DEBUG
    I::CVar->ConsolePrintf("Debug: Value = %f\n", someValue);
#endif
```

### 2. Visual Debugging
```cpp
void DrawDebugInfo()
{
    if (!Vars::Debug::Info.Value)
        return;
        
    H::Draw.String(H::Fonts.GetFont(FONT_ESP), 10, 10, {255, 255, 0, 255}, 
                  ALIGN_TOPLEFT, std::format("Debug: {}", debugValue).c_str());
}
```

### 3. Safe Entity Access
```cpp
bool IsValidEntity(CBaseEntity* pEntity)
{
    return pEntity && !pEntity->IsDormant() && pEntity->IsAlive();
}

// Always check before accessing
if (IsValidEntity(pPlayer))
{
    // Safe to access pPlayer methods
}
```

## Best Practices

### 1. Performance Considerations
- Cache expensive calculations
- Limit entity iteration frequency
- Use early returns to avoid unnecessary processing
- Only update when values actually change

### 2. Code Organization
- Keep drawing and logic separate
- Use const for read-only data
- Follow Amalgam naming conventions
- Comment complex algorithms

### 3. Configuration
- Use static constexpr for compile-time constants
- Make features easily configurable
- Follow existing Vars:: patterns if adding menu options

### 4. Error Handling
- Always check for null pointers
- Validate entity states before access
- Handle edge cases (no entities, game state changes)

## Example: Complete Feature Port

Here's how the uber2.lua was successfully ported following this guide:

1. **Analysis**: Identified it tracked medic uber charges and displayed advantage calculations
2. **Structure**: Created `UberTracker.h/cpp` in `Features/Visuals/`
3. **API Mapping**: 
   - `entities.FindByClass("CTFPlayer")` → `H::Entities.GetGroup(EGroupType::PLAYERS_ALL)`
   - `entity:GetClassNum() == CLASS_MEDIC` → `pPlayer->m_iClass() == TF_CLASS_MEDIC`
   - `medigun.m_flChargeLevel` → `pMedigun->m_flChargeLevel()`
4. **Drawing Integration**: Used `H::Draw.FillRect()` and `H::Draw.String()`
5. **Project Integration**: Added to `.vcxproj` and `IEngineVGui_Paint.cpp`
6. **Fixes Applied**: 
   - Added `#include <map>` for STL containers
   - Changed `GetWeaponID()` → `m_iItemDefinitionIndex()` for proper weapon type detection
   - Added PlayerUtils include for proper name access
   - Used correct TF2 constants

The result was a perfectly integrated feature that provided all the original Lua functionality natively in C++.

## Example: Complete HealthBarESP Port

Following this guide, the esp.lua health bar system was successfully ported as a standalone always-on feature:

### Analysis Phase
- **Purpose**: Health bar ESP with overheal support for TF2 players
- **Key Features**: Medic mode, visibility checking, distance filtering, health-based alpha
- **Category**: Visuals feature (always-on like UberTracker)

### Implementation Structure
```cpp
// Header: src/Features/Visuals/HealthBarESP/HealthBarESP.h
class CHealthBarESP
{
private:
    static constexpr int ALPHA = 70;
    static constexpr int BAR_HEIGHT = 10;
    static constexpr int BAR_WIDTH = 90;
    static constexpr float MAX_DISTANCE_SQR = 3500.0f * 3500.0f;
    static constexpr bool MEDIC_MODE = true;
    static constexpr Color_t OVERHEAL_COLOR = {71, 166, 255, 255};
    
public:
    void Draw();
};
ADD_FEATURE(CHealthBarESP, HealthBarESP);
```

### Key API Mappings Applied
- `entities.FindByClass("CTFPlayer")` → `H::Entities.GetGroup(EGroupType::PLAYERS_ALL)`
- `entity:GetClassNum() == CLASS_MEDIC` → `pPlayer->m_iClass() == TF_CLASS_MEDIC`
- `entity:InCond(CLOAK_COND)` → `pPlayer->InCond(TF_COND_STEALTHED)`
- `client.WorldToScreen()` → `SDK::W2S()`
- `draw.FilledRect()` → `H::Draw.FillRect()`

### Fixes Applied During Port
1. **Trace Structure**: Fixed `trace.flFraction` → `trace.fraction`
2. **Visibility Improvements**: Changed `MASK_SHOT` → `MASK_VISIBLE` to reduce flickering
3. **Alpha Logic**: Corrected health-based visibility formula for proper brightening on damage
4. **No Wobble Mode**: Removed dynamic bounding box calculations, kept fixed positioning only

### Integration Steps
1. **Project Files**: Added to `Amalgam.vcxproj` (both .h and .cpp)
2. **Hook Integration**: Added include and `F::HealthBarESP.Draw()` to `IEngineVGui_Paint.cpp`
3. **Always-On Design**: No Vars checks, follows UberTracker pattern

### Final Result
- Always-enabled health bars with no menu toggles
- Medic mode: shows teammates when playing medic, enemies otherwise  
- Health-responsive visibility: dimmer when healthy, brighter when wounded
- Fixed 90px width bars positioned 30px below player center
- No flickering during combat, stable visibility checking

## Example: CritHeals Medic Assistance Port

Another successful port following this guide was the critheals.lua medic assistance system:

### Analysis Phase
- **Purpose**: Visual indicators for crit heal eligibility and uber build rate warnings
- **Key Features**: Triangle indicators, damage tracking, uber build penalty warnings
- **Category**: Medic-specific visuals feature (always-on when playing medic)

### Implementation Highlights
```cpp
// Always-on medic assistance with event integration
class CCritHeals
{
private:
    static constexpr float CRIT_HEAL_TIME = 10.0f;
    static constexpr float UBER_PENALTY_THRESHOLD = 1.425f;
    std::unordered_map<int, float> m_LastDamageTimes;
    
public:
    void Draw();
    void OnPlayerHurt(int victimIndex);  // Event-driven damage tracking
};
```

### Key Integration Points
- **Event system integration**: Added `player_hurt` event handling to track damage times
- **Class-specific activation**: Only runs when playing medic (`pLocal->m_iClass() == TF_CLASS_MEDIC`)
- **Visual positioning**: Aligned uber warnings with UberTracker display area
- **Entity casting**: Proper `CBaseEntity` → `CTFPlayer`/`CTFWeaponBase` casting

### Fixes Applied During Port
1. **Entity type safety**: Fixed `CBaseEntity->IsAlive()` → `CTFPlayer->IsAlive()` casting
2. **Event integration**: Added to existing `player_hurt` event dispatcher in `Events.cpp`
3. **Visual consistency**: Positioned warnings to align with UberTracker layout
4. **Visibility improvements**: Changed `MASK_SHOT_HULL` → `MASK_VISIBLE` to reduce flickering
5. **Performance optimization**: Used damage event tracking instead of continuous health monitoring

### Final Result
- Triangle indicators above players eligible for crit heals (10+ seconds since damage)
- "Reduced Uber Build Rate!" warning when healing overhealed players (142.5%+ health)
- Integrated with existing event system for efficient damage tracking
- Seamless visual integration with UberTracker positioning
- No flickering during combat, stable triangle indicators

This example demonstrates event-driven features and proper medic-specific functionality integration.

## Example: PlayerTrails System Port

Another successful integration following this guide was the trails.lua movement tracking system:

### Analysis Phase
- **Purpose**: Visual movement trails for enemy players showing their recent paths
- **Key Features**: Colored trails, visibility-based display, fade-out effects, distance filtering
- **Category**: Always-on visuals feature for enemy tracking

### Implementation Highlights
```cpp
// Always-on enemy trail tracking with performance optimizations
class CPlayerTrails
{
private:
    static constexpr int TRAIL_LENGTH = 19;
    static constexpr float FADE_TIME = 7.0f;
    static constexpr float MAX_TRAIL_DISTANCE = 1850.0f;
    std::unordered_map<std::string, PlayerTrailData> m_PlayerData;
    
public:
    void Draw();  // Main drawing loop with visibility and distance checks
};
```

### Key Integration Points
- **Entity iteration**: Used `H::Entities.GetGroup(EGroupType::PLAYERS_ALL)` for efficient player access
- **Visibility system**: Integrated with existing trace filtering using `MASK_VISIBLE`
- **Color generation**: Unique trail colors per player using SteamID-based hashing
- **Performance optimization**: Cached visibility checks, distance filtering, cleanup routines

### Fixes Applied During Port
1. **API corrections**: Fixed `trace.entity` → `trace.m_pEnt` for proper entity access
2. **Type safety**: Corrected `W2S` function calls from `Vec2` → `Vec3` parameters
3. **Lifestate tracking**: Simplified respawn detection using `IsAlive()` instead of raw lifestate values
4. **ViewOffset access**: Fixed player view offset access using proper casting `pLocal->As<CTFPlayer>()`
5. **Color narrowing**: Used `byte()` cast for proper RGBA color component conversion

### Final Result
- Always-enabled colored trails behind enemy players only
- 19-position trails with 7-second fade time and 1850-unit distance limit
- Visibility-responsive display: trails show during and briefly after losing sight
- Performance-optimized with cached checks and periodic cleanup
- Unique colors per player for easy identification during combat
- Seamless integration with existing visual systems

This example demonstrates comprehensive entity tracking, performance optimization patterns, and proper visual effect integration.

## Example: StickyESP Integration

The final integration following this guide was the sticky.lua stickybomb ESP system:

### Analysis Phase
- **Purpose**: Always-on ESP for enemy stickybombs with visibility-based coloring and performance optimization
- **Key Features**: 2D/3D box ESP, enemy-only filtering, distance filtering, visibility checks, caching systems
- **Category**: Always-on visual ESP feature that leverages existing native systems

### Implementation Highlights
```cpp
// Stickybomb ESP with native system integration
class CStickyESP
{
private:
    static constexpr bool ENEMY_ONLY = true;
    static constexpr float MAX_DISTANCE = 2800.0f;
    static constexpr Color_t BOX_COLOR_VISIBLE = {0, 255, 0, 255};   // Green
    static constexpr Color_t BOX_COLOR_INVISIBLE = {255, 0, 0, 255}; // Red
    
public:
    void Draw();  // Mirrors Lua stickybomb_esp() function exactly
};
```

### Key Integration Points
- **Native entity groups**: Used `H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES)` instead of `entities.FindByClass()`
- **Existing chams system**: Leveraged native projectile chams (no custom implementation needed)
- **Native drawing API**: Used `H::Draw.Line()` instead of `draw.Line()` for box rendering
- **SDK trace system**: Integrated with existing `SDK::Trace()` and `MASK_SHOT` constants

### Fixes Applied During Port
1. **Entity filtering**: Used `pSticky->m_iType() == 1` netvar instead of `GetPropInt("m_iType")`
2. **Class identification**: Used `ETFClassID::CTFGrenadePipebombProjectile` instead of string class names
3. **Distance calculation**: Used squared distance comparison for performance (avoiding sqrt)
4. **API consistency**: Used `pEntity->m_iTeamNum()` instead of `GetTeamNumber()`
5. **Native integration**: No custom chams - existing projectile chams system handles stickybombs automatically

### Final Result
- Always-enabled ESP for enemy stickybombs only (2800 unit range)
- 2D box ESP (20px boxes) with visibility-based green/red coloring
- Optional 3D bounding box ESP using entity bounds calculation
- Direct processing every frame (no caching) for stable, flicker-free rendering
- Seamless integration with existing projectile chams system (user-controllable)
- Exact mirror of Lua functionality using native drawing and entity systems
- Clean, minimal implementation focused purely on Lua script behavior

### Key Lessons Learned
1. **Avoid over-optimization**: Initial implementation had complex caching systems that caused flickering
2. **Mirror Lua exactly**: Best results came from direct 1:1 translation of Lua logic
3. **Follow existing patterns**: HealthBarESP's direct processing approach worked perfectly
4. **Keep it simple**: Removed all caching, timers, and performance optimizations for stable rendering
5. **Native drawing**: Using `H::Draw.Line()` directly matches Lua `draw.Line()` behavior

This example demonstrates that the best ports often prioritize exact behavior over performance optimization.

## Example: FocusFire Focus Target Tracking

The latest successful integration was the focusfire.lua multi-targeting detection system:

### Analysis Phase
- **Purpose**: Visual tracking system for enemies being targeted by multiple teammates
- **Key Features**: Event-driven damage tracking, visual box ESP, automatic cleanup, team coordination assistance
- **Category**: Always-on tactical visuals feature for team coordination

### Implementation Highlights
```cpp
// Multi-targeting tracker with event integration
class CFocusFire
{
private:
    static constexpr int MIN_ATTACKERS = 2;
    static constexpr float TRACKER_TIME_WINDOW = 4.5f;
    static constexpr Color_t BOX_COLOR = {255, 0, 0, 190}; // Red corners
    std::unordered_map<int, TargetInfo> m_TargetData;
    
public:
    void Draw();  // Box ESP drawing
    void OnPlayerHurt(int victimIndex, int attackerIndex);  // Event tracking
};
```

### Key Integration Points
- **Event-driven tracking**: Connected to `player_hurt` events in `Events.cpp` for real-time damage tracking
- **Visual coordination**: Red corner boxes highlight enemies being focused by 2+ teammates
- **Performance optimization**: Time-windowed tracking (4.5s) with automatic cleanup routines
- **Team filtering**: Only highlights enemy players, ignores friendly fire

### API Mappings Applied
- **Event handling**: `callbacks.Register("FireGameEvent")` → `Events.cpp` integration
- **Multi-targeting logic**: `targetData[entIndex].attackers` → `std::unordered_map<int, TargetInfo>`
- **Box drawing**: `draw.Line()` corner system → `H::Draw.Line()` with thickness
- **Entity filtering**: `entity:GetTeamNumber()` → `pPlayer->m_iTeamNum()`

### Integration Steps
1. **Project structure**: Added to `Features/Visuals/FocusFire/` following established patterns
2. **Event integration**: Added `OnPlayerHurt` call to existing `player_hurt` event handler
3. **Drawing integration**: Added `F::FocusFire.Draw()` to main drawing loop in `IEngineVGui_Paint.cpp`
4. **Always-on design**: No configuration options, automatic activation like UberTracker

### Final Result
- Always-enabled focus fire detection for tactical team coordination
- Red corner boxes appear around enemies being shot by 2+ teammates within 4.5 seconds
- Event-driven performance with automatic target cleanup and memory management
- Seamless integration with existing visual systems and drawing pipeline
- Real-time tactical awareness for coordinated team pushes

### Key Lessons Learned
1. **Event integration**: Player damage events provide reliable real-time tracking data
2. **Time-windowed tracking**: 4.5-second windows effectively capture coordinated attacks
3. **Visual clarity**: Red corner boxes provide clear focus target indication without screen clutter
4. **Memory management**: Automatic cleanup prevents memory leaks during long gameplay sessions
5. **Team coordination**: Visual feedback enhances team tactics without being distracting

This example demonstrates successful event-driven feature integration with real-time tactical applications.

## Conclusion

This guide provides the foundation for porting any Lua script to Amalgam's C++ codebase. The key is understanding the API mappings, following the established patterns, and methodically fixing compilation issues. Each successful port makes future ports easier as you build familiarity with the SDK and common patterns.

**Best Practices Learned from All Examples:**
1. **Start simple**: Direct 1:1 Lua translation without optimization
2. **Follow existing patterns**: HealthBarESP, UberTracker, PlayerTrails approaches
3. **Avoid premature optimization**: Caching and performance optimizations can cause flickering
4. **Use native systems**: Leverage existing ESP, chams, and drawing systems when possible
5. **Test iteratively**: Build, test, fix issues, repeat until behavior matches exactly

Remember: when in doubt, look at existing similar features in the codebase. The ESP system is particularly good reference for entity access and drawing patterns. For always-on features, use the UberTracker as a reference implementation.