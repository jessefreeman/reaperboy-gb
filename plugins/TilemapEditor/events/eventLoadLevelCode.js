export const id = "EVENT_LOAD_LEVEL_CODE";
export const name = "Load Level Code";
export const groups = ["TilemapEditor"];

export const fields = [
  {
    key: "storage_type",
    type: "select",
    label: "Storage Type",
    defaultValue: "variables",
    options: [
      ["variables", "Variables (Recommended)"],
      ["sram", "SRAM (Advanced)"],
    ],
  },
  {
    key: "variable",
    type: "variable",
    label: "Success Variable",
    defaultValue: "LAST_VARIABLE",
    condition: {
      key: "storage_type",
      eq: "sram",
    },
  },
];

export const compile = (input, helpers) => {
  const { _callNative, _setVariable, getVariableAlias } = helpers;

  if (input.storage_type === "sram") {
    const variableAlias = getVariableAlias(input.variable);
    _callNative("vm_load_level_code_sram");
    _setVariable(variableAlias, ".ARG0");
  } else {
    _callNative("vm_load_level_code");
  }
};
