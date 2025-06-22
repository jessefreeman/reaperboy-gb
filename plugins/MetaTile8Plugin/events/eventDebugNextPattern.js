const id = "DEBUG_NEXT_PATTERN";
const groups = ["MetaTile8Plugin"];
const name = "Debug: Next Pattern";

const fields = [];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  _callNative("vm_debug_next_pattern");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
};
