const id = "EVENT_LOAD_LEVEL_CODE_STRING";
const groups = ["EVENT_GROUP_MISC"];
const name = "Load Level Code";

const fields = [
  {
    key: "description",
    type: "label",
    defaultValue: "Loads a previously saved 24-character level code from memory and rebuilds the level. Use this to restore saved levels after scene reloads."
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
