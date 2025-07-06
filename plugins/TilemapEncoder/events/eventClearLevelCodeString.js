const id = "EVENT_CLEAR_LEVEL_CODE_STRING";
const groups = ["EVENT_GROUP_MISC"];
const name = "Clear Level Code";

const fields = [
  {
    key: "description",
    type: "label",
    defaultValue: "Clears all saved level code data from memory. Use this to reset the level code storage or free up memory."
  }
];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  
  _callNative("vm_clear_level_code_string");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
