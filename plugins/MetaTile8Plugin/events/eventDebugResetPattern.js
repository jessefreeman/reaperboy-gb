const id = "DEBUG_RESET_PATTERN";
const groups = ["MetaTile8Plugin"];
const name = "Debug: Reset Pattern (Start from 0)";

const fields = [];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  _callNative("vm_debug_reset_pattern");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
};
