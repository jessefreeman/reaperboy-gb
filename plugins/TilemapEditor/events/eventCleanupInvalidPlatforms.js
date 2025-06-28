export const id = "EVENT_CLEANUP_INVALID_PLATFORMS";
export const name = "Cleanup Invalid Platforms";
export const groups = ["TilemapEditor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment(
    "↪ Cleanup invalid platforms: fix single tiles and enforce 2-8 tile rule"
  );
  _callNative("vm_cleanup_invalid_platforms");
};
