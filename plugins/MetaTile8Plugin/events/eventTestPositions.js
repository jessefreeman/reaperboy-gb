export const id = "EVENT_TEST_POSITIONS";
export const name = "Test Actor Positions";
export const groups = ["Map", "Debug"];

export const fields = [
  { key: "player", label: "Player", type: "actor", defaultValue: "0" },
  { key: "exit", label: "Exit", type: "actor", defaultValue: "1" },
  { key: "e1", label: "Enemy 1", type: "actor", defaultValue: "2" },
  { key: "e2", label: "Enemy 2", type: "actor", defaultValue: "3" },
  { key: "e3", label: "Enemy 3", type: "actor", defaultValue: "4" },
  { key: "e4", label: "Enemy 4", type: "actor", defaultValue: "5" },
  { key: "e5", label: "Enemy 5", type: "actor", defaultValue: "6" },
  { key: "e6", label: "Enemy 6", type: "actor", defaultValue: "7" },
];

export const compile = (input, helpers) => {
  const { actorPushById, _callNative, _stackPop, _addComment } = helpers;

  _addComment("→ Test actor positions in a row");
  // push in reverse order (so JS pop() in C sees player at FN_ARG0, exit at FN_ARG1, etc)
  actorPushById(input.e6);
  actorPushById(input.e5);
  actorPushById(input.e4);
  actorPushById(input.e3);
  actorPushById(input.e2);
  actorPushById(input.e1);
  actorPushById(input.exit);
  actorPushById(input.player);

  _callNative("vm_test_positions");
  _stackPop(8);
};
