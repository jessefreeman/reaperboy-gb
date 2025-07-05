export const id = "EVENT_CYCLE_CHARACTER";
export const name = "Cycle Character";
export const groups = ["TilemapEncoder"];

export const autoLabel = (fetchArg) => {
  return `Cycle character at (${fetchArg("x")}, ${fetchArg("y")})`;
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
];

export const compile = (input, helpers) => {
  const {
    _callNative,
    _stackPush,
    _stackPop,
    _addComment,
    _declareLocal,
    variableSetToScriptValue,
  } = helpers;

  const tmp0 = _declareLocal("tmp0", 1, true);
  const tmp1 = _declareLocal("tmp1", 1, true);

  variableSetToScriptValue(tmp0, input.x);
  variableSetToScriptValue(tmp1, input.y);

  _addComment("Cycle character");

  _stackPush(tmp1);
  _stackPush(tmp0);

  _callNative("vm_cycle_character");
  _stackPop(2);
};
