const id = "EVENT_TEST_END";
const groups = ["EVENT_GROUP_TEST"];
const name = "Test: End Test Suite";

const fields = [
    {
        key: "show_results",
        label: "Show Final Results",
        type: "checkbox",
        defaultValue: true
    },
    {
        key: "message",
        label: "Completion Message",
        type: "text",
        defaultValue: "Tests completed!",
        placeholder: "Message to show when tests complete"
    }
];

const compile = (input, helpers) => {
    const { textDialogue, _addComment, _addNL } = helpers;
    
    _addComment("End Test Suite");
    _addNL();
    
    if (input.show_results) {
        const completionMessage = input.message || "Tests completed!";
        textDialogue(completionMessage);
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
