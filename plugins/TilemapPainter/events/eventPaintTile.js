export const id = "EVENT_PAINT_TILE";
export const name = "Paint Tile";
export const groups = ["TilemapEditor"];

export const fields = [
  {
    key: "x",
    label: "X",
    type: "value",
    defaultValue: { type: "number", value: 0 },
  },
  {
    key: "y",
    label: "Y",
    type: "value",
    defaultValue: { type: "number", value: 0 },
  },
];

export const compile = (input, helpers) => {
  const {
    _declareLocal,
    variableSetToScriptValue,
    _stackPush,
    _callNative,
    _stackPop,
    _addComment,
  } = helpers;
  const tx = _declareLocal("tx", 1, true);
  const ty = _declareLocal("ty", 1, true);
  variableSetToScriptValue(tx, input.x);
  variableSetToScriptValue(ty, input.y);

  _addComment("Paint tile at (X,Y)");
  _stackPush(ty); // becomes FN_ARG1 in vm_paint
  _stackPush(tx); // becomes FN_ARG0 in vm_paint

  _callNative("vm_paint"); // must match the "paint" in vm_paint
  _stackPop(2);
};
