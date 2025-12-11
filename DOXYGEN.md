# Doxygen Documentation - Mystic-Type Graphical Client

## Overview

The Mystic-Type Graphical Client includes comprehensive Doxygen documentation covering all classes, methods, and systems.

## Generating Documentation

### Prerequisites

Make sure Doxygen is installed on your system:

```bash
# Ubuntu/Debian
sudo apt-get install doxygen graphviz

# macOS (with Homebrew)
brew install doxygen graphviz

# Or download from https://www.doxygen.nl/download.html
```

### Generate HTML Documentation

From the project root directory:

```bash
cd /home/loic/EPITECH/rtype/combine-server-client/Mystic-Type
doxygen Doxyfile
```

The documentation will be generated in the `docs/html/` directory.

### View Documentation

Open the generated documentation in your browser:

```bash
# Open the main page
open docs/html/index.html

# Or with a different browser
firefox docs/html/index.html
google-chrome docs/html/index.html
```

## Documentation Structure

The documentation is organized into sections:

### Main Components
- **GraphicClient**: Entry point for the application
- **SceneManager**: Manages scene transitions and lifecycle
- **Scene Classes**: MenuScene, GameScene, LobbyScene, etc.

### Graphics System
- **Raylib Wrapper**: Window management and drawing
- **AnimatedSprite**: Sprite animation system
- **Rendering Systems**: Sprite and rectangle rendering

### Game Logic
- **ECS Architecture**: Entity-Component-System core
- **Components**: Position, Velocity, Sprite, Rectangle
- **Systems**: Input, Movement, Rendering

### Network System
- **NetworkClient**: Server communication
- **Socket Classes**: TCP/UDP socket abstraction
- **Game State**: Synchronized game state

## Documentation Comments

All classes and methods are documented using Doxygen-style comments:

```cpp
/**
 * @class MyClass
 * @brief Short description of the class
 * 
 * Longer description explaining what the class does,
 * how it's used, and important details.
 */
class MyClass {
public:
    /**
     * @brief Short description of the method
     * @param param1 Description of first parameter
     * @param param2 Description of second parameter
     * @return Description of return value
     * 
     * Longer explanation of what the method does.
     */
    int myMethod(int param1, float param2);
};
```

### Documentation Tags

Common Doxygen tags used:

- `@class`: Marks the start of a class documentation
- `@brief`: One-line summary
- `@param`: Parameter description
- `@return`: Return value description
- `@note`: Important note
- `@warning`: Warning about potential issues
- `@see`: Reference to related documentation
- `@code/@endcode`: Code examples
- `@file`: File documentation
- `@mainpage`: Main documentation page
- `@section`: Section heading

## Building with Graphs

The Doxyfile is configured to generate class diagrams and dependency graphs using GraphViz.

These diagrams show:
- Class hierarchies
- Component relationships
- System dependencies
- Data flow

## Custom Documentation

### Main Page
The main documentation page is defined in `DOXYGEN_MAIN.hpp` and includes:
- Introduction
- Architecture overview
- Scene system documentation
- ECS system documentation
- Network communication details
- Performance considerations
- Extension guide

### Scene System Documentation
Detailed documentation on:
- Creating new scenes
- Scene lifecycle
- Scene transitions
- Event handling

### API Reference
Complete API reference with:
- Class hierarchies
- Method signatures
- Parameter descriptions
- Return values
- Usage examples

## Documentation Best Practices

When adding new code:

1. **Document Classes**: Add `@class` and `@brief` comments
2. **Document Methods**: Explain parameters, return values, and behavior
3. **Document Complex Logic**: Use `@code` blocks to show usage
4. **Add Examples**: Include example code where helpful
5. **Cross-Reference**: Use `@see` to link related topics
6. **Keep Updated**: Update docs when code changes

## Accessing Documentation

### Local Access
After generating, open in browser:
```
file:///path/to/docs/html/index.html
```

### Online Access (Future)
If hosted online, access at:
```
https://mystic-type.example.com/docs
```

## Customizing Documentation

To customize the documentation output:

1. Edit the `Doxyfile` in the project root
2. Modify settings like:
   - `PROJECT_NAME`: Project title
   - `HTML_COLORSTYLE_HUE`: Color scheme
   - `GENERATE_LATEX`: Enable/disable LaTeX output
   - `HAVE_DOT`: Enable/disable GraphViz diagrams

3. Regenerate:
```bash
doxygen Doxyfile
```

## Troubleshooting

### Missing Dependencies
If you see warnings about GraphViz:
```bash
# Install GraphViz
sudo apt-get install graphviz

# Or check if dot is available
which dot
```

### Slow Generation
For large projects, generation can take time:
- Disable graph generation: `HAVE_DOT = NO`
- Reduce graph depth: `MAX_DOT_GRAPH_DEPTH = 2`
- Limit documentation: `EXTRACT_ALL = NO`

### Build System Integration
To generate documentation as part of the build:

Add to `CMakeLists.txt`:
```cmake
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif()
```

Then build docs:
```bash
cd build
cmake ..
make doc
```

## References

- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Doxygen Commands](https://www.doxygen.nl/manual/commands.html)
- [GraphViz](https://graphviz.org/)

## Version

- **Doxygen**: 1.9.0+
- **GraphViz**: 2.40+
- **Generated**: 2025-12-11
