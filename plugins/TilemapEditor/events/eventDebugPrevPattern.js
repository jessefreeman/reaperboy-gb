export const id = "EVENT_DEBUG_PREV_PATTERN";
export const name = "Debug: Previous Pattern";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative } = helpers;
  _callNative("vm_debug_prev_pattern");
};
