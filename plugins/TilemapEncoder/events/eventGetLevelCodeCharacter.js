const id = "EVENT_GET_LEVEL_CODE_CHARACTER";
const groups = ["EVENT_GROUP_MISC"];
const name = "Get Level Code Character";

const fields = [
  {
    key: "charIndex",
    label: "Character Index",
    description: "Character position (0-23) to read",
    type: "union",
    types: ["number", "variable"],
    defaultType: "number",
    defaultValue: {
      number: 0,
      variable: "LAST_VARIABLE",
    },
  },
  {
    key: "variable",
    label: "Result Variable",
    description: "Variable to store the character value",
    type: "variable",
    defaultValue: "LAST_VARIABLE"
  },
  {
    key: "description",
    type: "label",
    defaultValue: "Gets a specific character from the stored level code. Use this to read individual level code components or export level codes."
  }
];

const compile = (input, helpers) => {
  const { _callNative, _setConst, getVariableAlias } = helpers;
  
  const resultVariableAlias = getVariableAlias(input.variable);
  
  // Set character index (ARG0)
  if (input.charIndex.type === "number") {
    _setConst(".ARG0", input.charIndex.value);
  } else {
    const variableAlias = getVariableAlias(input.charIndex.value);
    _setConst(".ARG0", variableAlias);
  }
  
  // Set result variable pointer (ARG1)
  _setConst(".ARG1", resultVariableAlias);
  
  _callNative("vm_get_level_code_character");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
