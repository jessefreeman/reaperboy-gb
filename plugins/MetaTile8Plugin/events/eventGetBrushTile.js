export const id = "EVENT_GET_BRUSH_TILE";
export const name = "Get Brush Tile";
export const groups = ["Tile Editor"];

export const fields = [
  { key: "x",       label: "X",       type: "value",    defaultValue: { type: "number", value: 0 } },
  { key: "y",       label: "Y",       type: "value",    defaultValue: { type: "number", value: 0 } },
  { key: "output",  label: "Variable",type: "variable", defaultValue: "LAST_VARIABLE" },
];

export const compile = (input, helpers) => {
  const {
    _declareLocal,
    variableSetToScriptValue,
    _stackPush,
    _stackPushConst,
    _callNative,
    _stackPop,
    _addComment,
    getVariableAlias,
  } = helpers;

  const tx = _declareLocal("tx", 1, true);
  const ty = _declareLocal("ty", 1, true);
  variableSetToScriptValue(tx, input.x);
  variableSetToScriptValue(ty, input.y);

  const varAlias = getVariableAlias(input.output);

  _addComment("Get brush tile at (X,Y)");
  _stackPush(tx);         // → FN_ARG0 (x)
  _stackPush(ty);         // → FN_ARG1 (y)
  _stackPushConst(varAlias);  // → FN_ARG2 (script variable index)
  _callNative("get_brush_tile");
  _stackPop(3);
};
