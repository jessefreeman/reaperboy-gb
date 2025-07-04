# Tilemap Editor Documentation

## Overview

The Tilemap Editor plugin provides a comprehensive in-game level editing system for GB Studio platformers, featuring real-time validation, seamless painting, and a compact level code system for persistence.

## Core Documentation

| Document                                                | Description                                                                |
| ------------------------------------------------------- | -------------------------------------------------------------------------- |
| **[Tilemap Editor Overview](tilemap-editor-plugin.md)** | Complete overview of plugin architecture, components, and usage            |
| **[Quick Reference Cheatsheet](cheatsheet.md)**         | At-a-glance reference for events, controls, and rules                      |
| **[Integration Guide](integration-guide.md)**           | Step-by-step instructions for adding the plugin to your project            |
| **[Implementation Details](implementation-details.md)** | Technical guide for developers extending the plugin                        |
| **[Platform Paint System](platform-paint-system.md)**   | Detailed explanation of platform creation, validation, and management      |
| **[Level Code System](level-code-system.md)**           | Technical details on the level encoding, serialization, and display system |
| **[Enemy System](enemy-system.md)**                     | Complete documentation of the enemy placement and management system        |
| **[Code Entry Mode](code-entry-mode.md)**               | Guide to the alternative code-based level editing workflow                 |

## Getting Started

### Quick Setup

1. Add the Tilemap Editor plugin to your GB Studio project
2. Add "Setup Paint Actors" event to your scene
3. Add "Enable Editor" event when triggered by player input
4. Connect paint events to player controls

### Basic Controls

```
D-PAD:        Move cursor
A Button:     Paint at cursor position
B Button:     Delete at cursor position
SELECT:       Cycle between tools (platform, player, enemy)
```

### Loading & Saving

```
Save Level:   Stores current design to variables
Load Level:   Restores previously saved design
Copy Code:    Displays 24-character level code for sharing
```

## Plugin Features

### Level Design

- Interactive platform painting with auto-validation
- Player and enemy placement with position validation
- Real-time visual feedback for valid/invalid actions
- Automatic platform rule enforcement (2-8 tile length)

### Level Code System

- Compact 24-character encoding for level designs
- Variable-based persistence across game sessions
- Smart zone-based updates for performance
- Pattern matching for efficient storage

### Integration

- Compatible with standard GB Studio workflow
- Uses native actor system for visual representation
- Suitable for in-game level editors and debug tools
- Easy to extend with additional tile types

### Additional Resources

#### [Development History](development-history.md)

Chronological record of implementation phases and key decisions.

**Key Topics:**

- Major implementation phases
- Technical challenges and solutions
- Lessons learned during development
- Evolution of the code architecture

### Technical Details

#### [Technical Optimizations](technical-optimizations.md)

Consolidated reference for all technical optimizations and performance improvements.

**Key Topics:**

- Platform pattern system optimization (5-bit patterns)
- Level code display flicker elimination
- Paint logic synchronization fixes
- Platform validation improvements
- Memory usage and performance metrics

#### [Implementation Details](implementation-details.md)

Complete overview of the plugin structure, architecture, and implementation details.

**Key Topics:**

- Current plugin file structure
- Migration from MetaTile8Plugin to TilemapEditor
- Event system architecture
- GB Studio integration requirements
- Future extensibility considerations

#### [Development History](development-history.md)

Implementation notes, key decisions, and lessons learned during development.

**Key Topics:**

- Major implementation phases
- Critical technical challenges and solutions
- Edge cases and how they were handled
- Architecture decisions and rationale
- Code quality improvements

## Quick Start Guide

### For Game Developers

1. **Setup Variables**: Create 6 global variables in GB Studio for level persistence
2. **Update Variable IDs**: Modify the `#define VAR_LEVEL_CODE_PART_X` values in `code_gen.c`
3. **Use Events**: Add TilemapEditor events to your scenes for level editing functionality

### For Plugin Developers

1. **Review Architecture**: Start with [Plugin Architecture](plugin-architecture.md)
2. **Understand Core Systems**: Read [Level Code System](level-code-system.md) and [Platform Paint System](platform-paint-system.md)
3. **Study Implementation**: Check [Development History](development-history.md) for context and decisions

### For Troubleshooting

1. **Performance Issues**: See [Performance Optimizations](performance-optimizations.md)
2. **Integration Problems**: Check [Plugin Architecture](plugin-architecture.md) for requirements
3. **Behavior Issues**: Review [Platform Paint System](platform-paint-system.md) for validation rules

## Key Features Summary

### ✅ Implemented & Tested

- **Lossless Level Code System**: Complete 5-bit encoding with variable persistence
- **Platform Paint Logic**: 8-tile limits, auto-completion, validation
- **Performance Optimizations**: Flicker-free display, selective updates
- **Plugin Architecture**: Clean structure with comprehensive event system

### 🔄 Partially Implemented

- **Code Entry Mode**: Event files exist, core functions may need verification

### 📋 Future Considerations

- **SRAM Storage**: Alternative to variable-based persistence
- **Enhanced Patterns**: Additional platform pattern types
- **Advanced Validation**: More sophisticated rule systems

## Integration Requirements

### GB Studio Setup

- **Variables**: 6 global variables for level persistence
- **Events**: TilemapEditor events added to scenes
- **Bank**: Uses bank 254 for core functions

### Performance

- **Display Updates**: Selective system eliminates flicker
- **Memory Usage**: Efficient 5-bit encoding minimizes storage
- **CPU Usage**: Optimized algorithms with smart caching

### Compatibility

- **GB Studio**: Compatible with standard GB Studio project structure
- **Save/Load**: Integrates with GB Studio save system
- **Extensions**: Modular architecture supports future enhancements

## Contributing

When extending or modifying the plugin:

1. **Follow Architecture**: Maintain the current separation between engine and events
2. **Update Documentation**: Keep docs synchronized with implementation changes
3. **Test Integration**: Verify GB Studio compatibility and performance
4. **Consider Edge Cases**: Review existing edge case handling for consistency

## Support

For issues, questions, or contributions:

- Review the appropriate documentation section
- Check [Development History](development-history.md) for similar issues
- Examine the current implementation in `plugins/TilemapEditor/`
- Consider the integration requirements and plugin architecture

This documentation provides complete coverage of the TilemapEditor plugin system, from high-level architecture to specific implementation details.
