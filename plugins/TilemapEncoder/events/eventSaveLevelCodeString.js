const id = "EVENT_SAVE_LEVEL_CODE_STRING";
const groups = ["EVENT_GROUP_MISC"];
const name = "Save Level Code";

const fields = [
  {
    key: "description",
    type: "label",
    defaultValue: "Saves the current level design as a 24-character level code to memory. The code persists across scene reloads."
  }
];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  
  _callNative("vm_save_level_code_string");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
