const id = "EVENT_TEST_VERIFY_VARIABLE";
const groups = ["EVENT_GROUP_TEST"];
const name = "Test: Verify Variable";

const fields = [
    {
        key: "variable",
        label: "Variable",
        type: "variable",
        defaultValue: "LAST_VARIABLE"
    },
    {
        key: "expected_value",
        label: "Expected Value",
        type: "number",
        min: 0,
        max: 255,
        defaultValue: 0
    },
    {
        key: "true",
        label: "On Pass",
        type: "events"
    },
    {
        key: "false",
        label: "On Fail",
        type: "events"
    }
];

const compile = (input, helpers) => {
    const { ifVariableValue } = helpers;
    
    ifVariableValue(
        input.variable,
        ".EQ",
        input.expected_value || 0,
        input.true || [],
        input.false || []
    );
};

module.exports = {
    id,
    name,
    groups,
    fields,
    compile
};
