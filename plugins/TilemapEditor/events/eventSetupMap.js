export const id = "EVENT_SETUP_MAP";
export const name = "Setup Map Actors";
export const groups = ["TilemapEditor"];

export const fields = [
  {
    key: "output",
    label: "Variable",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
  { key: "exit", label: "Exit Actor", type: "actor", defaultValue: "1" },
  { key: "e1", label: "Enemy 1", type: "actor", defaultValue: "2" },
  { key: "e2", label: "Enemy 2", type: "actor", defaultValue: "3" },
  { key: "e3", label: "Enemy 3", type: "actor", defaultValue: "4" },
  { key: "e4", label: "Enemy 4", type: "actor", defaultValue: "5" },
  { key: "e5", label: "Enemy 5", type: "actor", defaultValue: "6" },
  { key: "e6", label: "Enemy 6", type: "actor", defaultValue: "7" },
];

export const compile = (input, helpers) => {
  const {
    actorPushById,
    _callNative,
    _stackPop,
    _addComment,
    getVariableAlias,
    _stackPushConst,
  } = helpers;

  _addComment("↪ Setup map: player, exit, enemies");
  // push in reverse order for pop() in C
  actorPushById(input.e6);
  actorPushById(input.e5);
  actorPushById(input.e4);
  actorPushById(input.e3);
  actorPushById(input.e2);
  actorPushById(input.e1);
  actorPushById(input.exit);

  const variableAlias = getVariableAlias(input.output);
  _stackPushConst(variableAlias);

  _callNative("vm_setup_map");
  _stackPop(8);
};
