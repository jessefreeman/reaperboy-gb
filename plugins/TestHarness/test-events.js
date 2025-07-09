// Test script to validate event syntax
const fs = require('fs');
const path = require('path');

const eventsDir = './events';
const events = ['eventTestStart.js', 'eventTestVerifyVariable.js', 'eventTestEnd.js'];

console.log('Testing event syntax...');

events.forEach(eventFile => {
    try {
        const eventPath = path.join(eventsDir, eventFile);
        if (fs.existsSync(eventPath)) {
            const event = require(eventPath);
            console.log(`✓ ${eventFile} - OK`);
            console.log(`  ID: ${event.id}`);
            console.log(`  Name: ${event.name}`);
            console.log(`  Fields: ${event.fields.length} fields`);
        } else {
            console.log(`✗ ${eventFile} - NOT FOUND`);
        }
    } catch (error) {
        console.log(`✗ ${eventFile} - ERROR: ${error.message}`);
    }
});

console.log('Event syntax test complete!');
