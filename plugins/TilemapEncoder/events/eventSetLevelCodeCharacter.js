const id = "EVENT_SET_LEVEL_CODE_CHARACTER";
const groups = ["EVENT_GROUP_MISC"];
const name = "Set Level Code Character";

const fields = [
  {
    key: "charIndex",
    label: "Character Index",
    description: "Character position (0-23) to modify",
    type: "union",
    types: ["number", "variable"],
    defaultType: "number",
    defaultValue: {
      number: 0,
      variable: "LAST_VARIABLE",
    },
  },
  {
    key: "value",
    label: "Character Value",
    description: "New value for the character (0-40 range)",
    type: "union",
    types: ["number", "variable"],
    defaultType: "number",
    defaultValue: {
      number: 0,
      variable: "LAST_VARIABLE",
    },
  },
  {
    key: "description",
    type: "label",
    defaultValue: "Sets a specific character in the stored level code. Characters 0-15: platforms (0-34), Character 16: player (0-40), Characters 17-21: enemies (0-40), Characters 22-23: enemy masks (0-31)."
  }
];

const compile = (input, helpers) => {
  const { _callNative, _setConst, getVariableAlias } = helpers;
  
  // Set character index (ARG0)
  if (input.charIndex.type === "number") {
    _setConst(".ARG0", input.charIndex.value);
  } else {
    const variableAlias = getVariableAlias(input.charIndex.value);
    _setConst(".ARG0", variableAlias);
  }
  
  // Set character value (ARG1)
  if (input.value.type === "number") {
    _setConst(".ARG1", input.value.value);
  } else {
    const variableAlias = getVariableAlias(input.value.value);
    _setConst(".ARG1", variableAlias);
  }
  
  _callNative("vm_set_level_code_character");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
