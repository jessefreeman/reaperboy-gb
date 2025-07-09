const id = "EVENT_TEST_END";
const groups = ["EVENT_GROUP_TEST"];
const name = "Test: End Test Suite";

const fields = [
    {
        key: "show_results",
        label: "Show Final Results",
        type: "checkbox",
        defaultValue: true
    }
];

const compile = (input, helpers) => {
    const { appendRaw, _addComment, _declareLocal, _addNL, textDialogue } = helpers;
    
    _addComment("End Test Suite");
    _addNL();
    
    if (input.show_results) {
        // Get the test results and display them
        _declareLocal("passed_count");
        _declareLocal("failed_count");
        
        appendRaw(`passed_count = test_get_passed_count();`);
        appendRaw(`failed_count = test_get_failed_count();`);
        _addNL();
        
        // Display final results
        textDialogue(`Tests Complete - Passed: \" + passed_count + \" Failed: \" + failed_count`);
        _addNL();
    }
    
    // Stop test execution
    appendRaw(`test_stop_execution();`);
    _addNL();
};

module.exports = {
    id,
    name,
    groups,
    fields,
    compile
};
