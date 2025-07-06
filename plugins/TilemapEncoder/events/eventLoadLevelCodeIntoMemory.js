const id = "EVENT_LOAD_LEVEL_CODE_INTO_MEMORY";
const groups = ["EVENT_GROUP_MISC"];
const name = "Load Level Code Into Memory";

const fields = [
  {
    key: "levelCode",
    label: "Level Code",
    description: "24-character level code as individual character values (0-40 range)",
    type: "textarea",
    placeholder: "Enter level code as comma-separated values: 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,10,0,0,0,0,0,0,0",
    defaultValue: "1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,10,0,0,0,0,0,0,0"
  },
  {
    key: "description",
    type: "label",
    defaultValue: "Loads a level code into memory variables. Format: 24 comma-separated numbers (0-40). First 16 are platforms, 17th is player position, last 7 are enemy data."
  }
];

const compile = (input, helpers) => {
  const { _callNative, _setConst } = helpers;
  
  // Parse the level code string into individual values
  const levelCodeStr = input.levelCode || "1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,10,0,0,0,0,0,0,0";
  const levelCodeArray = levelCodeStr.split(',').map(s => parseInt(s.trim()) || 0);
  
  // Ensure we have exactly 24 values
  while (levelCodeArray.length < 24) {
    levelCodeArray.push(0);
  }
  levelCodeArray.splice(24); // Trim to exactly 24
  
  // Clear existing level code
  _callNative("vm_clear_level_code_string");
  
  // Set each character value
  for (let i = 0; i < 24; i++) {
    const value = Math.max(0, Math.min(40, levelCodeArray[i])); // Clamp to 0-40
    _setConst(".ARG0", i);
    _setConst(".ARG1", value);
    _callNative("vm_set_level_code_character");
  }
};

module.exports = {
  id,
  name,
  groups,
  fields,
  compile,
  waitUntilAfterInitFade: true,
};
