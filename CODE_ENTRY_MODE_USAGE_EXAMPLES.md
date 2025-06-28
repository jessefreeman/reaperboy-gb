# Code Entry Mode Usage Examples

## Example 1: Basic Code Entry Session

```
// Start code entry session
→ Enable Code Entry Mode

// User can now freely edit patterns
// [User interaction with level code display]
// [User cycles characters and sees immediate feedback]
// [Single-tile platforms and invalid patterns are allowed]

// End code entry session
→ Cleanup Invalid Platforms  // Fix any invalid patterns
→ Disable Code Entry Mode    // Return to normal mode
```

## Example 2: Code Entry with Save/Load

```
// Save current state before editing
→ Save Level Code

// Enter code entry mode
→ Enable Code Entry Mode

// [User edits patterns]

// User decides to cancel - restore previous state
→ Load Level Code
→ Disable Code Entry Mode

// OR user confirms changes
→ Cleanup Invalid Platforms
→ Disable Code Entry Mode
→ Save Level Code  // Save corrected version
```

## Example 3: Code Entry with Validation

```
// Start editing session
→ Enable Code Entry Mode

// [User enters/edits code]

// Clean up and validate
→ Cleanup Invalid Platforms
→ Disable Code Entry Mode

// Check if result is valid (has player, etc.)
→ Validate Level Setup
If validation fails:
  → Show error message
  → [Allow user to re-edit or restore default]
```

## Example 4: Code Import from External Source

```
// When loading a code from string/external source
→ Enable Code Entry Mode

// Apply each character of the imported code
For each character in imported code:
  → Set cursor to position
  → Cycle Character (until correct value)

// Clean up the imported pattern
→ Cleanup Invalid Platforms
→ Disable Code Entry Mode

// Verify the imported level is playable
→ Validate Level Setup
```

## Example 5: Pattern Preview System

```
// Show what a pattern will look like before committing
→ Save Level Code           // Backup current state
→ Enable Code Entry Mode    // Allow unrestricted drawing
→ Apply preview pattern     // Show the pattern
// [Show preview to user]
If user accepts:
  → Cleanup Invalid Platforms
  → Disable Code Entry Mode
Else:
  → Load Level Code         // Restore backup
  → Disable Code Entry Mode
```

## Key Points

1. **Always** call "Cleanup Invalid Platforms" before exiting code entry mode
2. **Always** call "Disable Code Entry Mode" when finished editing
3. Save state before entry if you want to support cancellation
4. Use "Validate Level Setup" after cleanup to ensure playability
5. The cleanup process automatically updates the level code display

## Error Handling

If you forget to disable code entry mode:

- The system will remain in unrestricted mode
- Future pattern edits will not be validated
- Call "Disable Code Entry Mode" to fix

If you forget to cleanup:

- Invalid patterns will remain on the tilemap
- The level may not be playable
- Call "Cleanup Invalid Platforms" to fix
