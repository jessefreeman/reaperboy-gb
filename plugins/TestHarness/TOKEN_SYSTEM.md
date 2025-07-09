# Test Harness - Token-Based Results

## 🎯 How the Token System Works

The test harness now uses customizable message templates with tokens that get replaced with actual test results.

## 📋 Available Tokens

### **Start Test Event:**
- `{TEST_NAME}` - The name of your test suite

### **Verify Variable Event:**
- `{TEST_NAME}` - The test description
- `{RESULT}` - "PASS" or "FAIL"
- `{EXPECTED}` - The expected value
- `{ACTUAL}` - The actual variable value

### **End Test Event:**
- `{PASSED}` - Number of passed tests (runtime replacement)
- `{FAILED}` - Number of failed tests (runtime replacement)
- `{TOTAL}` - Total number of tests (runtime replacement)

## 🎮 Example Test Script

```
1. Test: Start Test Suite
   - test_name: "Health System Test"
   - start_message: "🧪 Testing: {TEST_NAME}"
   - debug_enabled: ✓

2. Store Variable: V_PLAYER_HEALTH = 3

3. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 3
   - test_name: "Initial Health"
   - message_template: "✅ {TEST_NAME}: {RESULT} (Expected: {EXPECTED}, Got: {ACTUAL})"

4. Store Variable: V_PLAYER_HEALTH = 2

5. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 3
   - test_name: "Health After Damage"
   - message_template: "❌ {TEST_NAME}: {RESULT} - Expected {EXPECTED} but got {ACTUAL}"

6. Test: End Test Suite
   - show_results: ✓
   - completion_message: "🏁 Tests completed! {PASSED} passed, {FAILED} failed ({TOTAL} total)"
```

## 📊 Expected Output

```
🧪 Testing: Health System Test
✅ Initial Health: PASS (Expected: 3, Got: 3)
❌ Health After Damage: FAIL - Expected 3 but got 2
🏁 Tests completed! 1 passed, 1 failed (2 total)
```

## 🎨 Message Template Examples

### **Simple Templates:**
```
"Test {TEST_NAME}: {RESULT}"
"{RESULT}: {TEST_NAME}"
"Result: {RESULT}"
```

### **Detailed Templates:**
```
"Test '{TEST_NAME}' {RESULT} - Expected: {EXPECTED}, Actual: {ACTUAL}"
"✅ PASS: {TEST_NAME}" / "❌ FAIL: {TEST_NAME} (got {ACTUAL}, wanted {EXPECTED})"
"[{RESULT}] {TEST_NAME}: {ACTUAL} vs {EXPECTED}"
```

### **Fun Templates:**
```
"🧪 Testing {TEST_NAME}... {RESULT}! 🎉"
"⚡ {TEST_NAME} → {RESULT} (expected {EXPECTED}, got {ACTUAL})"
"🎯 Target: {EXPECTED}, Hit: {ACTUAL} = {RESULT}"
```

### **Summary Templates:**
```
"Tests finished! {PASSED} passed, {FAILED} failed"
"Results: {PASSED}/{TOTAL} passed"
"Score: {PASSED} out of {TOTAL} tests passed"
"Suite complete - {FAILED} failures out of {TOTAL} tests"
```

## 🔧 How It Works

1. **You configure** the message template with tokens
2. **GB Studio evaluates** the variable comparison
3. **The event replaces** tokens with actual values (compile-time for static tokens, runtime for dynamic tokens)
4. **You see** the customized result message

### **Token Replacement Types:**

**Compile-Time Replacement:**
- `{TEST_NAME}`, `{RESULT}`, `{EXPECTED}` - replaced when plugin compiles the event scripts

**Runtime Replacement:**
- `{ACTUAL}` - uses GB Studio's variable system (`$VARIABLE_NAME$`)  
- `{PASSED}`, `{FAILED}`, `{TOTAL}` - calls C functions via temporary variables

This gives you complete control over how test results are displayed while getting real-time test data!
