export const id = "EVENT_MOVE_ACTOR_TO_TEST";
export const name = "Move Actor to Test";
export const groups = ["Actor"];

export const fields = [
  {
    key: "actor",
    label: "Actor",
    type: "actor",
    defaultValue: "0",
  },
];

export const compile = (input, helpers) => {
  const { actorSetActive, actorPushById, _callNative, _stackPop, _addComment } =
    helpers;

  _addComment("Move actor to hardcoded position (5,10)");
  actorSetActive(input.actor); // Sets actor 0 to selected actor
  actorPushById(input.actor); // Push selected actor index
  _callNative("vm_move_actor_to_test");
  _stackPop(1);
};
