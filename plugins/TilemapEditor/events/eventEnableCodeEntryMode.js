export const id = "EVENT_ENABLE_CODE_ENTRY_MODE";
export const name = "Enable Code Entry Mode";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("↪ Enable code entry mode: allow unrestricted pattern drawing");
  _callNative("vm_enable_code_entry_mode");
};
