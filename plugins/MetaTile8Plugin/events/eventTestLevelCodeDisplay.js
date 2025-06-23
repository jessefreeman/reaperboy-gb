export const id = "EVENT_TEST_LEVEL_CODE_DISPLAY";
export const name = "Test Level Code Display";
export const groups = ["Tile Editor"];

export const autoLabel = () => {
  return `Test Level Code Display`;
};

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment(
    "Test: Display sample level code pattern (0000 0000 0000 / 0000)"
  );

  _callNative("vm_test_level_code_display");
};
