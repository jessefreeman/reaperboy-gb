const id = "EVENT_GET_PREDEFINED_LEVEL_COUNT";
const groups = ["EVENT_GROUP_MISC"];
const name = "Get Predefined Level Count";

const fields = [
  {
    key: "variable",
    label: "Variable",
    description: "Variable to store the number of available predefined levels",
    type: "variable",
    defaultValue: "LAST_VARIABLE"
  },
  {
    key: "description",
    type: "label",
    defaultValue: "Gets the total number of predefined levels built into your game. Use this to create level selection menus or validate level indices."
  }
];

const compile = (input, helpers) => {
  const { _callNative, _setConst, getVariableAlias } = helpers;
  
  const variableAlias = getVariableAlias(input.variable);
  
  _callNative("vm_get_predefined_level_count");
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
