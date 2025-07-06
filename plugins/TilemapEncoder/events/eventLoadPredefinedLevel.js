const id = "EVENT_LOAD_PREDEFINED_LEVEL";
const groups = ["EVENT_GROUP_MISC"];
const name = "Load Predefined Level";

const fields = [
  {
    key: "levelIndex",
    label: "Level Index",
    description: "Index of the predefined level to load (0, 1, 2, etc.)",
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
    defaultValue: "Loads a predefined level by index. These levels are built into your game and can be accessed instantly. Level 0 = tutorial, Level 1 = challenge, etc."
  }
];

const compile = (input, helpers) => {
  const { _callNative, _setConst, getVariableAlias } = helpers;
  
  if (input.levelIndex.type === "number") {
    _setConst(".ARG0", input.levelIndex.value);
  } else {
    const variableAlias = getVariableAlias(input.levelIndex.value);
    _setConst(".ARG0", variableAlias);
  }
  
  _callNative("vm_load_predefined_level");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
