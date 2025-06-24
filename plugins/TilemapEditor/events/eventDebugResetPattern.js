export const id = "EVENT_DEBUG_RESET_PATTERN";
export const name = "Debug: Reset Pattern (Start from 0)";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative } = helpers;
  _callNative("vm_debug_reset_pattern");
};
