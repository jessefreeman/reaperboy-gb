export const id = "EVENT_GENERATE_DISPLAY_LEVEL_CODE";
export const name = "Generate & Display Level Code";
export const groups = ["Tile Editor"];

export const autoLabel = () => {
  return `Generate & Display Level Code`;
};

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment(
    "Generate and display complete level code (platforms + player + enemies)"
  );

  _callNative("vm_generate_and_display_level_code");
};
