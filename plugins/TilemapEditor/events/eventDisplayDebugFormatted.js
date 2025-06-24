export const id = "EVENT_DISPLAY_DEBUG_FORMATTED";
export const name = "Display Debug Patterns (Formatted)";
export const groups = ["TilemapEditor"];

export const autoLabel = () => {
  return `Display Debug Patterns (Formatted)`;
};

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment(
    "Display debug patterns in formatted layout (4-char blocks with spaces)"
  );

  _callNative("vm_display_debug_patterns_formatted");
};
