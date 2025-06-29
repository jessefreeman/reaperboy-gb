export const id = "EVENT_SETUP_PAINT_ACTORS";
export const name = "Setup Paint Actors";
export const groups = ["TilemapEditor"];

export const fields = [
  { key: "player", label: "Player Actor", type: "actor", defaultValue: "0" },
  { key: "exit", label: "Exit Actor", type: "actor", defaultValue: "1" },
  { key: "e1", label: "Enemy 1", type: "actor", defaultValue: "2" },
  { key: "e2", label: "Enemy 2", type: "actor", defaultValue: "3" },
  { key: "e3", label: "Enemy 3", type: "actor", defaultValue: "4" },
  { key: "e4", label: "Enemy 4", type: "actor", defaultValue: "5" },
  { key: "e5", label: "Enemy 5", type: "actor", defaultValue: "6" },
];

export const compile = (input, helpers) => {
  const { actorPushById, _callNative, _stackPop, _addComment } = helpers;

  _addComment("↪ Setup paint actors: player, exit, enemies");
  // push in reverse order for pop() in C
  actorPushById(input.e5);
  actorPushById(input.e4);
  actorPushById(input.e3);
  actorPushById(input.e2);
  actorPushById(input.e1);
  actorPushById(input.exit);
  actorPushById(input.player);

  _callNative("vm_setup_paint_actors");
  _stackPop(7);
};
