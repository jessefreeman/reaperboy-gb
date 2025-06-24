# Level Code Persistence Guide

## Overview

The MetaTile8Plugin now supports saving and loading level codes to maintain persistence between game sessions and level reloads.

## Setup Required in GB Studio

### For Variable Storage (Recommended) - OPTIMIZED VERSION

Create these **4 Global Variables** in your GB Studio project:

1. `level_code_part_1` - Platform patterns 0-7 (packed)
2. `level_code_part_2` - Platform patterns 8-15 (packed)
3. `level_code_part_3` - Platform patterns 16-19 + enemy summary (packed)
4. `level_code_part_4` - Enemy data + player position (packed)

**Important:** Update the variable IDs in `code_gen.c` to match your actual variable indices:

```c
#define VAR_LEVEL_CODE_PART_1 0   // Set to your actual variable ID
#define VAR_LEVEL_CODE_PART_2 1   // Set to your actual variable ID
#define VAR_LEVEL_CODE_PART_3 2   // Set to your actual variable ID
#define VAR_LEVEL_CODE_PART_4 3   // Set to your actual variable ID
```

### For SRAM Storage (Advanced)

- Ensure your ROM has SRAM/battery save support enabled
- No additional setup required in GB Studio
- Preserves full enemy position precision

## Available Events

### Save Level Code

- **Event:** "Save Level Code"
- **Purpose:** Saves the current level design to persistent storage
- **Options:** Variables (recommended) or SRAM
- **Usage:** Call this after the player finishes editing a level

### Load Level Code

- **Event:** "Load Level Code"
- **Purpose:** Loads a previously saved level design
- **Options:** Variables (recommended) or SRAM
- **Usage:** Call this when entering the level editor to restore previous work

### Check Saved Level Code

- **Event:** "Check Saved Level Code"
- **Purpose:** Checks if a saved level exists
- **Returns:** 1 if saved data exists, 0 if not
- **Usage:** Show "Continue Editing" vs "New Level" options

## Example Usage Flow

### Level Editor Entry Script:

```
1. Check Saved Level Code → Store in $has_saved_data
2. If $has_saved_data = 1:
   - Show dialog: "Continue previous level?"
   - If Yes: Load Level Code
   - If No: Initialize new level
3. Display Complete Level Code
```

### Level Editor Exit Script:

```
1. Save Level Code
2. Show "Level saved!" message
```

### Level Test/Play Script:

```
1. Load Level Code (to ensure latest changes)
2. Generate level from loaded code
3. Start gameplay
```

## Storage Comparison

| Feature          | Variables (Optimized)       | SRAM                  |
| ---------------- | --------------------------- | --------------------- |
| Variables Needed | 4                           | 0                     |
| Setup Complexity | Easy                        | Easy                  |
| Data Persistence | Until game reset            | Permanent (battery)   |
| Storage Capacity | Platform + basic enemy data | Full precision        |
| Corruption Risk  | Low                         | Medium                |
| Enemy Precision  | Basic (simplified)          | Full                  |
| Best For         | Simple level persistence    | Complex level editing |

## Technical Notes

- **Variable storage** now uses only 4 variables with heavy compression
- **Platform patterns** reduced from 21 possible values to 4 for variable storage
- **Enemy positions** stored with reduced precision in variable mode
- **SRAM storage** preserves full precision and includes checksums
- For full feature support, SRAM storage is recommended

## Data Precision Trade-offs

### Variable Storage:

- ✅ Platform structure preserved
- ✅ Player position preserved
- ⚠️ Platform patterns limited to 4 types (compressed)
- ⚠️ Enemy positions have reduced precision
- ✅ Only needs 4 GB Studio variables

### SRAM Storage:

- ✅ Full 21 platform patterns supported
- ✅ Exact enemy positions preserved
- ✅ Enemy types and directions preserved
- ✅ No variable limit constraints

## Troubleshooting

- **"Data not loading":** Check variable IDs match your GB Studio project
- **"Corruption detected":** Use Check Saved Level Code event before loading
- **"SRAM not working":** Ensure your ROM build includes battery save support
