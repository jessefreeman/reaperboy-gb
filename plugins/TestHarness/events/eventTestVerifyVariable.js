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
        key: "test_name",
        label: "Test Description",
        type: "text",
        defaultValue: "Variable Test",
        placeholder: "Enter test description..."
    },
    {
        key: "delay",
        label: "Delay (frames)",
        type: "number",
        min: 0,
        max: 255,
        defaultValue: 30
    }
];

const compile = (input, helpers) => {
    const { appendRaw, _addComment, _declareLocal, _addNL, textDialogue, ifVariableValue } = helpers;
    
    _addComment(`Test: Verify Variable - ${input.test_name || "Variable Test"}`);
    _addNL();
    
    // Add delay if specified
    if (input.delay && input.delay > 0) {
        appendRaw(`wait_frames(${input.delay});`);
        _addNL();
    }
    
    // Get the variable value and verify it
    const variableName = input.variable || "LAST_VARIABLE";
    const expectedValue = input.expected_value || 0;
    const testName = input.test_name || "Variable Test";
    
    // Declare a local variable to store the test result
    _declareLocal("test_result");
    
    // Call the test function and store result
    appendRaw(`test_result = test_verify_variable(${variableName}, ${expectedValue}, "${testName}");`);
    _addNL();
    
    // Show result based on test outcome
    appendRaw(`if (test_result == 1) {`);
    _addNL();
    textDialogue(`PASS: ${testName}`);
    appendRaw(`} else {`);
    _addNL();
    textDialogue(`FAIL: ${testName} (E:${expectedValue} A:\" + ${variableName} + \")`);
    appendRaw(`}`);
    _addNL();
};

module.exports = {
    id,
    name,
    groups,
    fields,
    compile
};
