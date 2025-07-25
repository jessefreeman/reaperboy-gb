export const id = "EVENT_GET_BRUSH_TILE";
export const name = "Get Brush Tile At Position";
export const groups = ["TilemapEditor"];

export const autoLabel = (fetchArg) => {
  return `Get brush tile at position`;
};

export const fields = [
  {
    key: `x`,
    label: "X",
    type: "value",
    width: "50%",
    defaultValue: {
      type: "number",
      value: 0,
    },
  },
  {
    key: `y`,
    label: "Y",
    type: "value",
    width: "50%",
    defaultValue: {
      type: "number",
      value: 0,
    },
  },
  {
    key: "output",
    label: "Variable",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
];

export const compile = (input, helpers) => {
  const {
    _callNative,
    _stackPushConst,
    _stackPush,
    _stackPop,
    _addComment,
    _declareLocal,
    variableSetToScriptValue,
    getVariableAlias,
  } = helpers;

  const tmp0 = _declareLocal("tmp0", 1, true);
  const tmp1 = _declareLocal("tmp1", 1, true);

  variableSetToScriptValue(tmp0, input.x);
  variableSetToScriptValue(tmp1, input.y);

  const variableAlias = getVariableAlias(input.output);

  _addComment("Get tile at position");

  _stackPushConst(variableAlias);
  _stackPush(tmp1);
  _stackPush(tmp0);

  _callNative("vm_get_brush_tile_pos");
  _stackPop(3);
};
