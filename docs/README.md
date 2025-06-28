# TilemapEditor Plugin Documentation

## Overview

The TilemapEditor plugin provides a comprehensive level editing system for GB Studio, featuring a lossless level code system, advanced platform painting with validation, and seamless save/load functionality.

## Documentation Index

### Core Systems

#### [Level Code System](level-code-system.md)

Complete documentation of the level code encoding, storage, and persistence system.

**Key Topics:**

- 5-bit platform pattern encoding (21 patterns)
- Variable-based persistence across 6 global variables
- 24-character display system
- VM wrapper functions and GB Studio integration
- Usage examples and setup requirements

#### [Platform Paint System](platform-paint-system.md)

Comprehensive guide to the platform painting, validation, and management system.

**Key Topics:**

- Platform creation and validation rules (2-8 tile limits)
- Auto-completion and merging logic
- Brush preview system
- Pattern application with manual paint simulation
- Integration with level code updates

#### [Code Entry Mode](code-entry-mode.md)

Documentation for the code entry mode system that allows unrestricted pattern drawing with automatic cleanup.

**Key Topics:**

- Unrestricted drawing during code entry
- Automatic cleanup on exit
- Available events and functions
- Usage workflows and examples
- Rule enforcement after cleanup

> **Note**: This system appears to be partially implemented. Event files exist but core functions may need verification.

### Technical Details

#### [Performance Optimizations](performance-optimizations.md)

Detailed coverage of all performance improvements and critical bug fixes.

**Key Topics:**

- Level code display flicker elimination
- Selective update system implementation
- Paint logic synchronization fixes
- Platform validation improvements
- Code refactoring and cleanup results

#### [Plugin Architecture](plugin-architecture.md)

Complete overview of the plugin structure, migration history, and integration details.

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
