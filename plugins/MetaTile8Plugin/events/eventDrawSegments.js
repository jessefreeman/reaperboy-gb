export const id = "EVENT_DRAW_SEGMENTS";
export const name = "Draw Segment IDs";
export const groups = ["Tile Editor"];

export const fields = [];

export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;

  _addComment("Debug: Draw segment ID tiles");
  _callNative("vm_draw_segment_ids");
};
