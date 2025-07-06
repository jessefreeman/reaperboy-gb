const id = "EVENT_RELOAD_LEVEL_FROM_MEMORY";
const groups = ["EVENT_GROUP_MISC"];
const name = "Reload Level from Memory";

const fields = [
  {
    key: "description",
    type: "label",
    defaultValue: "Reloads the level from the current variables (memory). Use this to restore a level that was previously stored in variables."
  }
];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  
  _callNative("vm_load_level_code_string");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
