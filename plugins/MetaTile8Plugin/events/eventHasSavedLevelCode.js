export const id = "EVENT_HAS_SAVED_LEVEL_CODE";
export const name = "Check Saved Level Code";
export const groups = ["MetaTile8Plugin"];

export const fields = [
  {
    key: "variable",
    type: "variable",
    label: "Store Result In",
    defaultValue: "LAST_VARIABLE",
  },
];

export const compile = (input, helpers) => {
  const { _callNative, _setVariable, getVariableAlias } = helpers;

  const variableAlias = getVariableAlias(input.variable);
  _callNative("vm_has_saved_level_code");
  _setVariable(variableAlias, ".ARG0");
};
