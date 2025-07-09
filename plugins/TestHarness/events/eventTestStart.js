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
        key: "message",
        label: "Start Message",
        type: "text",
        defaultValue: "Starting: {TEST_NAME}",
        placeholder: "Available tokens: {TEST_NAME}"
    },
    {
        key: "debug_enabled",
        label: "Debug Messages",
        type: "checkbox",
        defaultValue: true
    }
];

const compile = (input, helpers) => {
    const { textDialogue, _addComment, _addNL } = helpers;
    
    _addComment(`Start Test Suite: ${input.test_name || "Test Suite"}`);
    _addNL();
    
    if (input.debug_enabled) {
        const testName = input.test_name || "Test Suite";
        const startMessage = (input.message || "Starting: {TEST_NAME}")
            .replace(/{TEST_NAME}/g, testName);
        
        textDialogue(startMessage);
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
