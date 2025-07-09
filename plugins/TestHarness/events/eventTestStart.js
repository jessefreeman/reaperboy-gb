const id = "EVENT_TEST_START";
const groups = ["EVENT_GROUP_TEST"];
const name = "Test: Start Test Suite";

const fields = [
    {
        key: "test_name",
        label: "Test Name",
        type: "text",
        defaultValue: "Test Suite",
        placeholder: "Enter test name..."
    },
    {
        key: "debug_enabled",
        label: "Debug Messages",
        type: "checkbox",
        defaultValue: true
    }
];

const compile = (input, helpers) => {
    const { appendRaw, _addComment, _addNL, textDialogue } = helpers;
    
    _addComment(`Start Test Suite: ${input.test_name || "Test Suite"}`);
    _addNL();
    
    // Initialize test harness
    appendRaw(`test_harness_init();`);
    _addNL();
    
    // Start test execution
    appendRaw(`test_start_execution();`);
    _addNL();
    
    // Show start message if debug enabled
    if (input.debug_enabled) {
        textDialogue(`Starting tests: ${input.test_name || "Test Suite"}`);
        _addNL();
    }
};

module.exports = {
    id,
    name,
    groups,
    fields,
    compile
};
