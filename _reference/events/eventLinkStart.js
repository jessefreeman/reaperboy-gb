const l10n = require("../helpers/l10n").default;

const id = "EVENT_LINK_HOST";
const subGroups = {
  EVENT_GROUP_MISC: "EVENT_GROUP_MULTIPLAYER",
};

const fields = [
  {
    label: l10n("FIELD_LINK_HOST"),
  },
];

const compile = (input, helpers) => {
  const { linkHost } = helpers;
  linkHost();
};

module.exports = {
  id,
  subGroups,
  fields,
  compile,
};
