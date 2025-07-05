export const id = "EVENT_ENABLE_EDITOR";
export const name = "Enable Editor";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("↪ Enable editor mode: disable actors and prepare editor state");
  _callNative("vm_enable_editor");
};
