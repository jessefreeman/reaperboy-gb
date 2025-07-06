const id = "EVENT_INIT_TILEMAP_EDITOR_FROM_MEMORY";
const groups = ["EVENT_GROUP_MISC"];
const name = "Initialize Tilemap Editor from Memory";

const fields = [
  {
    key: "description",
    type: "label",
    defaultValue: "Initializes the tilemap editor from memory, ensuring proper player positioning and level restoration. Use this on tilemap editor scene init."
  }
];

const compile = (input, helpers) => {
  const { _callNative } = helpers;
  
  _callNative("vm_init_tilemap_editor_from_memory");
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
