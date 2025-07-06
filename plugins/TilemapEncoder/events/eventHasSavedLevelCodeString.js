const id = "EVENT_HAS_SAVED_LEVEL_CODE_STRING";
const groups = ["EVENT_GROUP_MISC"];
const name = "Check for Saved Level Code";

const fields = [
  {
    key: "variable",
    label: "Variable",
    description: "Variable to store result (1 = has saved data, 0 = no saved data)",
    type: "variable",
    defaultValue: "LAST_VARIABLE"
  },
  {
    key: "description",
    type: "label",
    defaultValue: "Checks if a saved level code exists in memory. Returns 1 if data exists, 0 if no saved data found."
  }
];

const compile = (input, helpers) => {
  const { _callNative, _setConst, getVariableAlias } = helpers;
  
  const variableAlias = getVariableAlias(input.variable);
  
  _callNative("vm_has_saved_level_code_string");
  _setConst(variableAlias, ".ARG0");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
