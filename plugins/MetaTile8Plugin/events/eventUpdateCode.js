export const id = "EVENT_UPDATE_CODE";
export const name = "Update Code Display";
export const groups = ["Tile Editor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("Update platform code display");
  _callNative("vm_update_code");
};
