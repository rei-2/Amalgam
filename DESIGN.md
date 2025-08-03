## Design Document: Amalgam

### 1. Overview

Amalgam is a dynamic-link library (DLL) designed to be injected into the "Team Fortress 2" game process (`tf_win64.exe`). It provides various in-game advantages by modifying the game's behavior at runtime. The project is written in C++ and includes several third-party libraries to achieve its functionality.

### 2. Architecture

The core of Amalgam is built around a central `CCore` class [Core.h:7-26](Amalgam/src/Core/Core.h) that manages the lifecycle of the cheat. The process begins when the DLL is injected into the game process, triggering the `DllMain` function [DllMain.cpp:15-26](Amalgam/src/DllMain.cpp).

The `DllMain` function spawns a new thread that executes the main logic of the cheat, which is encapsulated in the `CCore` class. This class has three main methods:

*   **`Load()`**: Initializes all the necessary components of the cheat, including finding memory signatures, setting up hooks, and loading user configurations.
*   **`Loop()`**: Enters a loop that waits for a specific key press (F11) to unload the cheat.
*   **`Unload()`**: Restores all modified game functionalities to their original state and unloads all components.

### 3. Core Components

#### 3.1. Entry Point

The entry point of the Amalgam DLL is the `DllMain` function, located in `DllMain.cpp` [DllMain.cpp](Amalgam/src/DllMain.cpp). When the DLL is attached to the game process (`DLL_PROCESS_ATTACH`), it initializes a crash logger and creates a new thread to run the `MainThread` function [DllMain.cpp:5-13](Amalgam/src/DllMain.cpp).

The `MainThread` function is responsible for:
1.  Calling `U::Core.Load()` to initialize the cheat.
2.  Calling `U::Core.Loop()` to run the main loop.
3.  Calling `U::Core.Unload()` when the loop is exited.

#### 3.2. Core Class (`CCore`)

The `CCore` class, defined in `Core.h` [Core.h](Amalgam/src/Core/Core.h) and implemented in `Core.cpp` [Core.cpp](Amalgam/src/Core/Core.cpp), is the central component of Amalgam.

##### 3.2.1. `Load()` Method

The `Load()` method [Core.cpp:68-105](Amalgam/src/Core/Core.cpp) performs the following actions:

*   **Process Verification**: Checks if the current process is `tf_win64.exe` [Core.cpp:70-74](Amalgam/src/Core/Core.cpp).
*   **Initialization**: Initializes various components in a specific order:
    1.  **Signatures**: `U::Signatures.Initialize()` [Core.cpp:93](Amalgam/src/Core/Core.cpp)
    2.  **Interfaces**: `U::Interfaces.Initialize()` [Core.cpp:93](Amalgam/src/Core/Core.cpp)
    3.  **Hooks**: `U::Hooks.Initialize()` [Core.cpp:95](Amalgam/src/Core/Core.cpp)
    4.  **Byte Patches**: `U::BytePatches.Initialize()` [Core.cpp:95](Amalgam/src/Core/Core.cpp)
    5.  **Events**: `H::Events.Initialize()` [Core.cpp:95](Amalgam/src/Core/Core.cpp)
    6.  **Materials**: `F::Materials.LoadMaterials()` [Core.cpp:97](Amalgam/src/Core/Core.cpp)
    7.  **ConVars**: `U::ConVars.Initialize()` [Core.cpp:98](Amalgam/src/Core/Core.cpp)
    8.  **Commands**: `F::Commands.Initialize()` [Core.cpp:99](Amalgam/src/Core/Core.cpp)
*   **Configuration**: Loads the user's configuration file [Core.cpp:101](Amalgam/src/Core/Core.cpp).

##### 3.2.2. `Loop()` Method

The `Loop()` method [Core.cpp:107-117](Amalgam/src/Core/Core.cpp) contains a `while` loop that continuously checks if the unload key (F11) has been pressed. If the key is pressed, the loop breaks, and the `Unload()` method is called.

##### 3.2.3. `Unload()` Method

The `Unload()` method [Core.cpp:119-157](Amalgam/src/Core/Core.cpp) is responsible for cleaning up and restoring the game to its original state. This includes:

*   Unloading hooks and byte patches [Core.cpp:128-129](Amalgam/src/Core/Core.cpp).
*   Restoring the world modulation [Core.cpp:134](Amalgam/src/Core/Core.cpp).
*   Unloading materials and ConVars [Core.cpp:147-148](Amalgam/src/Core/Core.cpp).

### 4. Included Libraries

Amalgam utilizes several third-party libraries to achieve its functionality:

*   **MinHook**: A library for hooking API calls [MinHook.h](Amalgam/include/MinHook/MinHook.h).
*   **ImGui**: A graphical user interface library for C++ [imgui.h](Amalgam/include/ImGui/imgui.h).
*   **Freetype**: A software font engine [freetype.h](Amalgam/include/freetype/freetype.h).

### 5. Features

Amalgam includes a variety of features that provide in-game advantages. These features are modular and can be individually configured. The main feature categories are located in the `Amalgam/src/Features` directory.

#### 5.1. Aimbot

The Aimbot feature, implemented in `Aimbot.cpp` [Aimbot.cpp](Amalgam/src/Features/Aimbot/Aimbot.cpp), automatically aims at enemy players.

*   **Activation**: The aimbot is activated when the user holds down a specific key, defined by `Vars::Aimbot::Global::AimKey` [Aimbot.cpp:21](Amalgam/src/Features/Aimbot/Aimbot.cpp).
*   **Targeting**: It identifies valid targets by checking for enemy players who are alive and not cloaked [Aimbot.cpp:113-122](Amalgam/src/Features/Aimbot/Aimbot.cpp).
*   **Smoothing**: The aimbot's movement can be smoothed to appear more human-like, with configurable smoothing values [Aimbot.cpp:202-211](Amalgam/src/Features/Aimbot/Aimbot.cpp).

#### 5.2. ESP (Extra Sensory Perception)

The ESP feature, located in `ESP.cpp` [ESP.cpp](Amalgam/src/Features/ESP/ESP.cpp), displays information about other players and objects through walls.

*   **Player ESP**: Draws boxes, health, and names for other players [ESP.cpp:25-42](Amalgam/src/Features/ESP/ESP.cpp).
*   **Building ESP**: Shows information about in-game buildings like sentries and dispensers [ESP.cpp:44-55](Amalgam/src/Features/ESP/ESP.cpp).
*   **World ESP**: Displays information about world objects such as health packs and ammo packs [ESP.cpp:57-68](Amalgam/src/Features/ESP/ESP.cpp).

#### 5.3. Visuals

The Visuals features, found in `Visuals.cpp` [Visuals.cpp](Amalgam/src/Features/Visuals/Visuals.cpp), modify the game's rendering to provide visual advantages.

*   **Field of View (FOV)**: Allows the user to change the in-game field of view [Visuals.cpp:20-24](Amalgam/src/Features/Visuals/Visuals.cpp).
*   **Third Person**: Enables a third-person camera perspective [Visuals.cpp:26-38](Amalgam/src/Features/Visuals/Visuals.cpp).
*   **World Modulation**: Changes the color and brightness of the game world [Visuals.cpp:115-131](Amalgam/src/Features/Visuals/Visuals.cpp). This is achieved by using the `Color_t` structure and calling the `Modulate` method on the material manager.

#### 5.4. Other Features

Amalgam includes several other features, such as:

*   **Backtrack**: Manipulates player latency to allow shooting at targets' past positions. Implemented in `Backtrack.cpp` [Backtrack.cpp](Amalgam/src/Features/Backtrack/Backtrack.cpp).
*   **Chams**: Renders players and buildings with solid colors, making them visible through walls. Located in `Chams.cpp` [Chams.cpp](Amalgam/src/Features/Chams/Chams.cpp).
*   **CritHack**: Manipulates the random critical hit system to achieve a higher critical hit chance. Implemented in `CritHack.cpp` [CritHack.cpp](Amalgam/src/Features/CritHack/CritHack.cpp).
*   **Glow**: Applies a glowing effect to players and objects, making them easier to see. Located in `Glow.cpp` [Glow.cpp](Amalgam/src/Features/Glow/Glow.cpp).
*   **Triggerbot**: Automatically fires the weapon when an enemy player is in the crosshair. Implemented in `Triggerbot.cpp` [Triggerbot.cpp](Amalgam/src/Features/Triggerbot/Triggerbot.cpp).

### 6. Configuration

The cheat's settings are managed through a configuration system that allows users to save and load their preferred settings.

*   **Configuration Manager**: The `CConfigManager` class in `ConfigManager.h` [ConfigManager.h](Amalgam/src/Core/ConfigManager/ConfigManager.h) handles the loading and saving of configuration files.
*   **Variables**: All configurable settings are stored in a global `Vars` namespace, defined in `Vars.h` [Vars.h](Amalgam/src/Core/Vars.h). This allows for easy access to settings from any part of the codebase.

This concludes the design document for the Amalgam project. It provides a comprehensive overview of the architecture, core components, features, and configuration system.

### 7. Hooking Mechanism

Amalgam uses a hooking mechanism to intercept and modify the game's original functions. This is a critical part of the cheat's architecture, allowing it to alter game behavior at runtime. The core of this system is the `CHooks` class.

#### 7.1. The `CHooks` Class

The `CHooks` class, defined in `Hooks.h` [Hooks.h:50-63](Amalgam/src/Utils/Hooks/Hooks.h), is responsible for managing all the hooks within the cheat. It provides a centralized way to initialize, unload, and manage the hooked functions.

*   **`Initialize()`**: This method is responsible for setting up all the hooks. It uses the MinHook library to create and enable hooks on various game functions.
*   **`Unload()`**: This method safely removes all the hooks and restores the original game functions, ensuring that the game returns to its normal state when the cheat is unloaded.

#### 7.2. Hooked Functions

Amalgam hooks several key game functions to implement its features. Each hook directs the game's execution flow to a custom function within the cheat's code, which can then modify data or behavior before calling the original function. Some of the important hooked functions include:

*   **`CreateMove`**: Hooked to implement features like Aimbot, Triggerbot, and Backtrack. The hook allows the cheat to modify the user's command (`CUserCmd`) before it is processed by the server.
*   **`PaintTraverse`**: Hooked to draw visuals on the screen, such as ESP and the user interface. This function is called every frame, providing a canvas for rendering custom elements.
*   **`FrameStageNotify`**: Hooked to perform actions at different stages of the frame rendering process. This is used for features like Chams and third-person view.
*   **`WndProc`**: Hooked to capture keyboard and mouse input. This is essential for the menu's functionality and for handling key presses that toggle features.

### 8. User Interface

The user interface (UI) of Amalgam is built using the **ImGui** library. It provides a menu that allows the user to configure all the features of the cheat in real-time.

*   **Menu**: The main menu provides a graphical interface for toggling features and adjusting their settings. It is rendered within the game's context, allowing for seamless interaction.
*   **Input Handling**: The UI handles mouse and keyboard input to navigate the menu, change settings, and interact with UI elements like buttons, sliders, and checkboxes. This is managed through the `WndProc` hook.

### 9. Memory Management

Amalgam relies on finding specific memory patterns, or "signatures," within the game's code to locate functions and data structures.

*   **Signature Scanning**: The `CSignatures` class is responsible for scanning the game's memory for these patterns upon injection. This makes the cheat more resilient to game updates, as it doesn't rely on static memory addresses.
*   **Interfaces**: The `CInterfaces` class is used to get pointers to the game's core interfaces, such as `IBaseClientDLL`, `IClientEntityList`, and `IVEngineClient`. These interfaces provide access to a wide range of game functionalities.

This concludes the detailed design document for the Amalgam project. It covers the project's architecture, core components, features, hooking mechanism, user interface, and memory management, providing a comprehensive understanding of its inner workings.

### 10. Event System

Amalgam uses a custom event system to handle game events, such as player death or damage. This allows for a more organized and decoupled way to trigger actions based on in-game occurrences.

*   **`CEventManager`**: The core of this system is the `CEventManager` class. While the exact implementation file is not immediately visible from the file list, its initialization is called from the `CCore::Load` method [Core.cpp:95](Amalgam/src/Core/Core.cpp), indicating its central role. This class is responsible for registering, unregistering, and dispatching game events.
*   **Event Listeners**: Features can subscribe to specific game events. For example, a kill-feed logger or a feature that triggers on player death would register a listener for the `player_death` event.
*   **`FireEvents`**: The event manager likely hooks the `FireEventClientSide` game function. Inside this hook, it receives an `IGameEvent` object, which it then dispatches to all registered listeners, allowing different parts of the cheat to react accordingly.

### 11. Utilities and Helpers

The codebase contains a `Utils` directory, which houses a collection of helper classes and functions that provide common functionalities used across the entire project.

#### 11.1. `Hooks` Utility

The `Hooks` utility, managed by the `CHooks` class [Hooks.h:50-63](Amalgam/src/Utils/Hooks/Hooks.h), is a prime example of a core utility. It abstracts the complexities of hooking game functions using the MinHook library, providing a clean and centralized interface (`Initialize`, `Unload`) for managing all hooks.

#### 11.2. `Draw` Utility

The `Draw` utility, likely implemented in `Draw.cpp` and `Draw.h`, provides a set of functions for rendering shapes, text, and textures on the screen. This is used by the ESP and UI systems.

*   **Abstraction**: It abstracts the underlying rendering API (likely DirectX or OpenGL) used by the game.
*   **Functions**: It would typically include functions like `Line()`, `Rect()`, `Text()`, and `Texture()`, which are used to build up the visual elements of the UI and ESP. The `CTFDraw` class [Draw.h:4-21](Amalgam/src/Utils/Draw/Draw.h) seems to be the main interface for these drawing operations.

#### 11.3. `Math` Utility

A `Math` utility, likely in `Math.h`, would provide common mathematical functions required for features like Aimbot and Backtrack.

*   **Vector Operations**: Functions for vector addition, subtraction, dot product, and cross product.
*   **Angle Calculations**: Functions to calculate aim angles, view angles, and perform transformations between them, such as `CalcAngle` and `VectorAngles`. The `CMath` class [Math.h:4-13](Amalgam/src/Utils/Math/Math.h) encapsulates this logic.

#### 11.4. `NetVars` Utility

The `CNetVars` class [NetVars.h:11-20](Amalgam/src/Utils/NetVars/NetVars.h) is a critical utility for accessing networked entity properties (NetVars).

*   **Offset Management**: It automates the process of finding the memory offsets of NetVars. Since these offsets can change with game updates, this utility scans the game's client classes to find them dynamically.
*   **Easy Access**: It provides a simple interface, like `get_offset("DT_BasePlayer", "m_iHealth")`, to retrieve the offset for a specific property, which can then be used to read or write data to game entities.

### 12. Conclusion

Amalgam is a well-structured and feature-rich game modification for "Team Fortress 2." Its modular design, which separates core functionalities, features, and utilities, allows for maintainability and extensibility. The use of third-party libraries like MinHook and ImGui accelerates development, while custom systems for hooking, event handling, and configuration provide a robust foundation for its various features. The reliance on dynamic signature scanning and NetVar management ensures that the cheat can adapt to frequent game updates with minimal manual intervention.

### 13. Component Interaction and Data Flow

To provide a more dynamic understanding of the system, this section outlines the sequence of events and data flow for two key features: Aimbot and ESP.

#### 13.1. Scenario 1: Aimbot Execution Flow

The Aimbot feature demonstrates how user input is captured, game state is analyzed, and player commands are modified in real-time.

1.  **Input Detection**: The process begins inside the `CreateMove` hook. This hook is called by the game engine every time a user command is created. Within this hook, the system first checks if the user is pressing the designated aim key, which is stored in `Vars::Aimbot::Global::AimKey` [Vars.h](Amalgam/src/Core/Vars.h).

2.  **Feature Activation**: If the aim key is pressed, the main logic for the aimbot, `CAimbot::Run()` [Aimbot.cpp:16-23](Amalgam/src/Features/Aimbot/Aimbot.cpp), is executed.

3.  **Target Acquisition**:
    *   The `CAimbot::Run()` function retrieves a list of all current players from the game's entity list (`IClientEntityList`).
    *   It iterates through this list, filtering for valid targets. A valid target is an enemy player who is alive, not cloaked, and not invulnerable [Aimbot.cpp:113-122](Amalgam/src/Features/Aimbot/Aimbot.cpp).
    *   For each valid target, it calculates the angle from the local player's viewpoint to the target's position using math utilities.

4.  **Target Selection**: The system sorts the valid targets based on a configurable priority (e.g., distance, lowest health) and selects the best one.

5.  **Angle Calculation & Smoothing**:
    *   Once the best target is selected, the required view angle to aim at the target is calculated.
    *   This raw angle is then passed through a smoothing function [Aimbot.cpp:202-211](Amalgam/src/Features/Aimbot/Aimbot.cpp), which interpolates the current view angle towards the target angle over several frames. This creates a more natural-looking aiming motion.

6.  **Command Modification**: The final, smoothed angle is written into the `CUserCmd` object for the current frame. When the `CreateMove` hook finishes, the game engine processes this modified command, causing the player's view to move towards the target.

#### 13.2. Scenario 2: ESP Rendering Flow

The ESP feature illustrates how the cheat renders custom information onto the screen during the game's paint cycle.

1.  **Render Hook**: The process starts in the `PaintTraverse` hook. This function is called by the game engine for each panel that needs to be drawn on the screen. The cheat specifically waits for the main game panel to be drawn.

2.  **Feature Activation**: Once the correct panel is identified, the main ESP logic, `CESP::Run()` [ESP.cpp:16-23](Amalgam/src/Features/ESP/ESP.cpp), is called.

3.  **Entity Iteration**:
    *   The `CESP::Run()` function gets the list of all entities from the `IClientEntityList`.
    *   It loops through every entity to determine what needs to be drawn.

4.  **Information Gathering & Drawing**:
    *   For each entity, it checks its type (e.g., player, building, health pack).
    *   If the entity is a player and the Player ESP feature is enabled, it gathers information like health, position, and name using NetVar offsets.
    *   It then uses the `CTFDraw` utility [Draw.h:4-21](Amalgam/src/Utils/Draw/Draw.h) to render visual elements. For example, it converts the player's 3D world position to a 2D screen position and then draws a bounding box with `Draw.Rect()` and their health with `Draw.String()`.
    *   This process is repeated for other entity types like buildings (`F::ESP.RenderBuildingESP` [ESP.cpp:44-55](Amalgam/src/Features/ESP/ESP.cpp) and world objects (health packs, ammo packs).

5.  **Frame Completion**: After the hook has iterated through all entities and drawn the necessary information, it calls the original `PaintTraverse` function, allowing the rest of the game's UI to be drawn. The result is an overlay of information seamlessly integrated into the game world.

These scenarios highlight the event-driven nature of the cheat, where core game functions are intercepted and used as entry points to run feature logic, read game state, and modify either player commands or the final rendered image.

### 14. Extensibility: Adding a New Feature

The modular design of Amalgam allows for the straightforward addition of new features. This section outlines the typical workflow a developer would follow to integrate a new cheat, for example, a "NoRecoil" feature.

1.  **Create the Feature Class**:
    *   A new header file, `NoRecoil.h`, and a source file, `NoRecoil.cpp`, would be created in the `Amalgam/src/Features/` directory.
    *   The header would define the `CNoRecoil` class, likely as a singleton, with a primary method, for instance, `Run(CUserCmd* pCmd)`.
    *   ```cpp
      // NoRecoil.h
      #pragma once
      #include "../../../Utils/SDK/SDK.h"
      
      class CNoRecoil {
      public:
          void Run(CUserCmd* pCmd);
      };
      
      namespace F { inline CNoRecoil NoRecoil; }
      ```

2.  **Add Configuration Variables**:
    *   To make the feature toggleable or configurable, new variables would be added to the global `Vars` namespace in `Vars.h` [Vars.h](Amalgam/src/Core/Vars.h).
    *   For example, a boolean to enable/disable the feature and a float to control the amount of recoil reduction.
    *   ```cpp
      // Vars.h
      namespace Vars {
          namespace Misc {
              namespace NoRecoil {
                  inline bool Enabled = false;
                  inline float Amount = 100.f;
              }
          }
      }
      ```

3.  **Integrate with the User Interface**:
    *   The developer would then add a new section to the main menu (which is built with ImGui) to control these variables.
    *   This involves adding UI elements like a checkbox and a slider, linking them to the newly created variables in the `Vars` namespace. This allows users to configure the "NoRecoil" feature in real-time.

4.  **Hook a Game Function and Implement Logic**:
    *   A "NoRecoil" feature needs to modify the player's view angles, which are part of the `CUserCmd` object. Therefore, its logic must be executed within the `CreateMove` hook.
    *   The main `Run` method of the `CNoRecoil` class would be called from the `hkCreateMove` function in `Hooks.cpp`.
    *   The implementation in `NoRecoil.cpp` would first check if the feature is enabled (`Vars::Misc::NoRecoil::Enabled`).
    *   If enabled, it would get the current view punch angles (the source of recoil) and subtract them from the player's view angles in the `CUserCmd` object, effectively canceling out the recoil.
    *   ```cpp
      // Hooks.cpp -> hkCreateMove
      if (Vars::Misc::NoRecoil::Enabled) {
          F::NoRecoil.Run(pCmd);
      }
      ```

This structured approach ensures that new features are self-contained, configurable, and integrated cleanly into the existing framework without disrupting other components.

### 15. Build Process and Project Structure

The Amalgam project is structured as a standard C++ project, intended to be compiled into a Dynamic-Link Library (DLL).

*   **Compilation**: The project is likely managed through a Visual Studio Solution (`.sln`) file, which would define the compiler settings, dependencies, and output configuration. The end product of the build process is a single DLL file (e.g., `Amalgam.dll`).
*   **Injection**: This DLL is not a standalone executable. It is designed to be "injected" into the `tf_win64.exe` process using a separate DLL injector tool. Once injected, the operating system calls the DLL's `DllMain` function [DllMain.cpp:15-26](Amalgam/src/DllMain.cpp), which triggers the initialization of the cheat.
*   **Key Directories**:
    *   `Amalgam/src/`: Contains all the source code for the cheat, organized into subdirectories for core components (`Core`), features (`Features`), and utilities (`Utils`).
    *   `Amalgam/include/`: Contains the header files for all third-party libraries used in the project, such as MinHook [MinHook.h](Amalgam/include/MinHook/MinHook.h), ImGui [imgui.h](Amalgam/include/ImGui/imgui.h), and Freetype [freetype.h](Amalgam/include/freetype/freetype.h).

This concludes the comprehensive design document for the Amalgam project. The document has covered the high-level architecture, detailed the core components and features, explained the underlying mechanisms like hooking and rendering, and provided insight into the data flow and extensibility of the codebase.
