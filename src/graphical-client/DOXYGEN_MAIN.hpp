/**
 * @file main_doxygen.hpp
 * @brief Mystic-Type Graphical Client - Main Documentation
 * 
 * @mainpage Mystic-Type Graphical Client Documentation
 * 
 * @section intro Introduction
 * 
 * The Mystic-Type Graphical Client is a multiplayer real-time strategy game
 * client built with C++ using the following technologies:
 * - **Raylib**: Graphics and window management
 * - **ECS Architecture**: Entity-Component-System for game object management
 * - **Network Communication**: TCP/UDP sockets for server communication
 * 
 * @section architecture Architecture Overview
 * 
 * The client follows a modular architecture with clear separation of concerns:
 * 
 * @subsection main_components Main Components
 * 
 * - **GraphicClient**: Entry point that coordinates all systems
 * - **SceneManager**: Manages scene transitions and lifecycle
 * - **IScene**: Abstract interface for all game scenes
 * - **GameScene**: Main gameplay implementation with ECS
 * - **MenuScene**: Main menu with play button
 * - **LobbyScene**: Player lobby for game coordination
 * 
 * @subsection graphics_layer Graphics Layer
 * 
 * - **Raylib**: Wrapper around Raylib for window management
 * - **AnimatedSprite**: Handles sprite animation and rendering
 * - **SpriteRenderSystem**: ECS system for rendering sprites
 * - **RectangleRenderSystem**: ECS system for rendering rectangles
 * 
 * @subsection network_layer Network Layer
 * 
 * - **NetworkClient**: Handles server communication
 * - **ASocket/ISocket**: Socket abstraction for network communication
 * - **Packet**: Packet serialization and deserialization
 * - **GameState**: Maintains synchronized game state
 * 
 * @subsection ecs_layer ECS Layer
 * 
 * - **ECS**: Entity Component System core
 * - **Entity**: Unique identifier for game objects
 * - **Component**: Base class for all components (Position, Velocity, Sprite, etc.)
 * - **System**: Processes entities with specific component combinations
 * 
 * @section dataflow Data Flow
 * 
 * @code
 * GraphicClient
 *     ├─ SceneManager
 *     │   ├─ GameScene
 *     │   │   ├─ ECS (Entities, Components, Systems)
 *     │   │   └─ NetworkClient (Sync)
 *     │   ├─ MenuScene
 *     │   └─ LobbyScene
 *     ├─ Raylib::Window (Rendering)
 *     └─ NetworkClient (Server Communication)
 * @endcode
 * 
 * @section scene_system Scene System
 * 
 * The scene system manages different game states:
 * 
 * @subsection scene_lifecycle Scene Lifecycle
 * 
 * 1. **Registration**: Scenes are created and registered with SceneManager
 * 2. **Transition**: changeScene() triggers onExit() on old scene, onEnter() on new
 * 3. **Update**: Each frame, the active scene's update() is called
 * 4. **Render**: Each frame, the active scene's render() is called
 * 
 * @subsection creating_scenes Creating New Scenes
 * 
 * To create a new scene:
 * 
 * 1. Create a class inheriting from IScene
 * 2. Implement pure virtual methods (onEnter, onExit, update, render)
 * 3. Override setSceneManager() to store the manager reference
 * 4. Register the scene in GraphicClient::init()
 * 5. Call setSceneManager() to enable scene transitions
 * 
 * Example:
 * @code
 * class MyScene : public IScene {
 * public:
 *     MyScene(Raylib::Window& window, NetworkClient& net);
 *     
 *     void onEnter() override;
 *     void onExit() override;
 *     void update(float dt) override;
 *     void render(float dt) override;
 *     void setSceneManager(SceneManager* mgr) override { _sceneManager = mgr; }
 * 
 * private:
 *     SceneManager* _sceneManager;
 *     Raylib::Window& _window;
 *     NetworkClient& _net;
 * };
 * @endcode
 * 
 * @section gameplay_system Gameplay System (ECS)
 * 
 * The GameScene uses an Entity-Component-System architecture for flexible
 * game object management:
 * 
 * @subsection entities Entities
 * Entities are represented by unique 32-bit IDs and contain:
 * - Position component: x, y coordinates
 * - Velocity component: movement speed
 * - Sprite component: animated sprite rendering
 * - Rectangle component: simple shape rendering
 * 
 * @subsection systems Systems
 * Systems process entities with specific component combinations:
 * - **InputSystem**: Reads player input and updates velocity
 * - **MovementSystem**: Updates position based on velocity
 * - **SpriteRenderSystem**: Renders animated sprites
 * - **RectangleRenderSystem**: Renders rectangle shapes
 * 
 * @subsection synchronization Network Synchronization
 * 
 * GameScene synchronizes with the server:
 * - **syncEntities()**: Updates player positions from server snapshots
 * - **syncBullets()**: Manages bullet lifecycle
 * - **syncMonsters()**: Updates monster positions and states
 * 
 * @section network_communication Network Communication
 * 
 * The client communicates with the server using:
 * 
 * @subsection protocols Protocols
 * - **TCP**: Initial connection and handshake
 * - **UDP**: Real-time game state updates (snapshots)
 * 
 * @subsection message_flow Message Flow
 * 
 * 1. **Connection**: TCP handshake with server
 * 2. **Initial State**: Receive player list
 * 3. **Game Loop**:
 *    - Send: Player input (position, actions)
 *    - Receive: Game snapshots (players, bullets, monsters)
 * 
 * @section input_handling Input Handling
 * 
 * The InputSystem processes keyboard input:
 * - **Arrow Keys/WASD**: Move player
 * - **Space/Ctrl**: Fire weapon
 * - **ESC**: Menu/Pause
 * 
 * @section rendering_pipeline Rendering Pipeline
 * 
 * Each frame:
 * 1. Window begins drawing
 * 2. Clear background
 * 3. Draw game area boundaries
 * 4. Render all player sprites
 * 5. Render all bullets
 * 6. Render all monsters
 * 7. Window ends drawing
 * 
 * @section performance_considerations Performance Considerations
 * 
 * - **Entity Pooling**: Entities are reused rather than created/destroyed
 * - **Component Arrays**: Contiguous memory for cache efficiency
 * - **Network Smoothing**: Position interpolation for smooth movement
 * - **Sprite Caching**: Textures loaded once and reused
 * 
 * @section extending Extending the Client
 * 
 * @subsection adding_components Adding Components
 * 
 * To add a new component:
 * 1. Define the struct in Components.hpp
 * 2. Create a ComponentArray in ECS
 * 3. Use in systems as needed
 * 
 * @subsection adding_systems Adding Systems
 * 
 * To add a new system:
 * 1. Create a system class with update() method
 * 2. Query ECS for entities with required components
 * 3. Process each entity in the update loop
 * 
 * @subsection adding_scenes Adding Scenes
 * 
 * See the @ref creating_scenes section above.
 * 
 * @section compilation Building the Project
 * 
 * The project uses CMake with FetchContent for dependency management:
 * 
 * @code
 * mkdir build
 * cd build
 * cmake ..
 * make
 * ./src/graphical-client/rtype-client
 * @endcode
 * 
 * Dependencies are automatically downloaded and built:
 * - **raylib**: Graphics library
 * - **raylib-src**: Source code
 * 
 * @section dependencies Dependencies
 * 
 * - **C++17** or later
 * - **CMake 3.14** or later
 * - **Linux** (X11) or Windows
 * - **Raylib 5.5** (auto-downloaded)
 * 
 * @section file_structure File Structure
 * 
 * @code
 * src/graphical-client/
 * ├── CMakeLists.txt           # CMake build configuration
 * ├── main.cpp                 # Entry point
 * ├── GraphicClient/
 * │   ├── GraphicClient.hpp    # Main client class
 * │   └── GraphicClient.cpp
 * ├── Scenes/
 * │   ├── IScene.hpp           # Scene interface
 * │   ├── SceneManager.hpp     # Scene manager
 * │   ├── GameScene.hpp        # Main gameplay scene
 * │   ├── MenuScene.hpp        # Menu scene
 * │   └── LobbyScene.hpp       # Lobby scene
 * ├── ECS/
 * │   ├── Core.hpp             # ECS core
 * │   ├── Components.hpp       # Component definitions
 * │   └── System.hpp           # System implementations
 * ├── Graphic/
 * │   ├── Graphic.hpp          # Graphics engine
 * │   └── Graphic.cpp
 * └── Raylib/
 *     ├── Raylib.hpp           # Raylib wrapper
 *     └── Raylib.cpp
 * @endcode
 * 
 * @section design_patterns Design Patterns Used
 * 
 * - **Entity-Component-System**: Flexible game object management
 * - **Strategy Pattern**: Different rendering/input systems
 * - **Template Method**: Scene lifecycle management
 * - **Observer Pattern**: Event-based scene transitions
 * - **Factory Pattern**: Entity creation
 * 
 * @section glossary Glossary
 * 
 * - **Entity**: A game object (player, bullet, monster)
 * - **Component**: Data attached to an entity (position, velocity, sprite)
 * - **System**: Logic that processes entities with specific components
 * - **Scene**: A distinct game state (menu, gameplay, lobby)
 * - **Snapshot**: Server's complete game state at a moment
 * - **ECS**: Entity-Component-System architecture
 * 
 * @section troubleshooting Troubleshooting
 * 
 * @subsection compilation_issues Compilation Issues
 * 
 * - Ensure C++17 or later is available
 * - Check CMake version (3.14+)
 * - Verify all includes are correct
 * 
 * @subsection runtime_issues Runtime Issues
 * 
 * - Check server connection parameters (IP, port)
 * - Verify network connectivity
 * - Check for undefined references (missing scene implementations)
 * 
 * @section future_improvements Future Improvements
 * 
 * - [ ] Add sound/audio system
 * - [ ] Implement particle effects
 * - [ ] Add UI overlay system
 * - [ ] Implement game settings/options
 * - [ ] Add replay system
 * - [ ] Performance profiling tools
 * 
 * @section license License
 * 
 * EPITECH PROJECT, 2025
 * 
 * @author Mystic Organization
 * @version 1.0.0
 * @date 2025-12-11
 */
