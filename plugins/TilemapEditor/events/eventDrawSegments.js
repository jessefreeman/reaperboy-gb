export const id = "EVENT_DRAW_SEGMENTS";
export const name = "Draw Segment IDs";
export const groups = ["TilemapEditor"];

export const autoLabel = () => {
  return `Draw Segments`;
};

export const fields = [
  {
    key: "patternVar",
    label: "Pattern Index Var",
    type: "variable",
    defaultValue: "0",
  },
  {
    key: "variantVar",
    label: "Variant Var",
    type: "variable",
    defaultValue: "1",
  },
];

export const compile = (input, helpers) => {
  const { _callNative, _stackPushVariable, _stackPop, _addComment } = helpers;

  _addComment("Debug: Draw segment ID tiles");

  _stackPushVariable(input.patternVar); // FN_ARG0
  _stackPushVariable(input.variantVar); // FN_ARG1

  _callNative("vm_draw_segment_ids");
  _stackPop(2);
};
