const id = "DEBUG_PREV_PATTERN";
const groups = ["MetaTile8Plugin"];
const name = "Debug: Previous Pattern";

const fields = [];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  _callNative("vm_debug_prev_pattern");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
};
