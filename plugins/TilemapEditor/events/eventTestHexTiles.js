export const id = "EVENT_TEST_HEX_TILES";
export const name = "Test Hex Tiles";
export const groups = ["TilemapEditor"];

export const autoLabel = () => {
  return `Test Hex Tiles`;
};

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("Test: Display hex tiles 0-F to verify tile mapping");

  _callNative("vm_test_hex_tiles");
};
