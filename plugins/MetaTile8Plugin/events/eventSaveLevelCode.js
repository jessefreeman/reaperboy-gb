export const id = "EVENT_SAVE_LEVEL_CODE";
export const name = "Save Level Code";
export const groups = ["MetaTile8Plugin"];

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
];

export const compile = (input, helpers) => {
  const { _callNative } = helpers;

  if (input.storage_type === "sram") {
    _callNative("vm_save_level_code_sram");
  } else {
    _callNative("vm_save_level_code");
  }
};
