const id = "EVENT_RESTORE_LEVEL_FROM_MEMORY";
const groups = ["EVENT_GROUP_MISC"];
const name = "Restore Level from Memory";

const fields = [
  {
    key: "description",
    type: "label",
    defaultValue: "Restores the level layout from whatever is currently stored in C memory. Use this on scene init to restore the level after editing."
  }
];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  
  _callNative("vm_restore_level_from_memory");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
