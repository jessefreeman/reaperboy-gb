export const id = "EVENT_EXIT_CODE_ENTRY_WITH_CLEANUP";
export const name = "Exit Code Entry & Cleanup";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("↪ Exit code entry mode and automatically fix invalid platforms");
  _callNative("vm_exit_code_entry_mode_with_cleanup");
};
