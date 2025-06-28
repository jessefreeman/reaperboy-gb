export const id = "EVENT_DISABLE_CODE_ENTRY_MODE";
export const name = "Disable Code Entry Mode";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("↪ Disable code entry mode: return to normal validation");
  _callNative("vm_disable_code_entry_mode");
};
