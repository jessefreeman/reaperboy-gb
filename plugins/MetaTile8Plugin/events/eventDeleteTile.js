export const id = "EVENT_DELETE_META_TILE";
export const name = "Delete tile";
export const groups = ["Tile Editor"];

export const autoLabel = () => {
  return `Delete tile`;
};

export const fields = [
  {
    key: "x",
    label: "X",
    type: "value",
    width: "50%",
    defaultValue: {
      type: "number",
      value: 0,
    },
  },
  {
    key: "y",
    label: "Y",
    type: "value",
    width: "50%",
    defaultValue: {
      type: "number",
      value: 0,
    },
  },
  {
    key: "commit",
    label: "Commit render",
    type: "checkbox",
    defaultValue: false,
  },
];

export const compile = (input, helpers) => {
  const {
    _callNative,
    _stackPush,
    _stackPushConst,
    _stackPop,
    _addComment,
    _declareLocal,
    variableSetToScriptValue,
  } = helpers;

  const tmp0 = _declareLocal("tmp0", 1, true);
  const tmp1 = _declareLocal("tmp1", 1, true);

  variableSetToScriptValue(tmp0, input.x);
  variableSetToScriptValue(tmp1, input.y);

  _addComment("Delete metatile");

  _stackPushConst(input.commit ? 1 : 0);
  _stackPush(tmp1);
  _stackPush(tmp0);

  _callNative("vm_delete_tile_at_pos");
  _stackPop(3);
};
