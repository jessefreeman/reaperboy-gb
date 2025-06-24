export const id = "EVENT_DEBUG_NEXT_PATTERN";
export const name = "Debug: Next Pattern";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative } = helpers;
  _callNative("vm_debug_next_pattern");
};
