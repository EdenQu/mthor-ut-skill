# ut_run Command Prompt

## Command Pattern

```regex
^ut_run(\s+.*)?$
```

## Parameters

| Parameter | Required | Default | Description |
|-----------|----------|---------|-------------|
| `--base` | No | `origin/dev` | Base branch for comparison |
| `--platform` | No | `all` | Filter by platform: `ios`, `android`, `cpp`, `all` |

## Workflow Steps

### Step 1: Detect Changed UT Files

```bash
# Get base branch (default: origin/dev)
BASE_BRANCH="${base:-origin/dev}"

# Fetch latest from remote
git fetch origin

# Get committed changes (UT files only)
COMMITTED_UT_FILES=$(git diff ${BASE_BRANCH}...HEAD --name-only | grep -E "Tests\.swift$|Test\.kt$|_test\.cpp$")

# Get uncommitted changes (staged + unstaged)
UNCOMMITTED_UT_FILES=$(git status --porcelain | awk '{print $2}' | grep -E "Tests\.swift$|Test\.kt$|_test\.cpp$")

# Combine and deduplicate
ALL_UT_FILES=$(echo -e "${COMMITTED_UT_FILES}\n${UNCOMMITTED_UT_FILES}" | sort -u | grep -v "^$")

echo "Found $(echo "$ALL_UT_FILES" | wc -l | tr -d ' ') changed UT files"
```

### Step 2: Group by Platform

```bash
# iOS test files
IOS_TESTS=$(echo "$ALL_UT_FILES" | grep "Tests\.swift$")

# Android test files
ANDROID_TESTS=$(echo "$ALL_UT_FILES" | grep "Test\.kt$")

# C++ test files
CPP_TESTS=$(echo "$ALL_UT_FILES" | grep "_test\.cpp$")
```

### Step 3: iOS Pre-build (REQUIRED for iOS)

**⚠️ CRITICAL: Must wait for `make ios_install_with_binary_cache` to complete before running any iOS tests.**

```bash
if [ -n "$IOS_TESTS" ]; then
    echo "🍎 iOS tests detected. Running pre-build..."
    
    cd app/ios/Glip
    
    # Run make command and capture output
    echo "Running: make ios_install_with_binary_cache"
    make ios_install_with_binary_cache 2>&1 | tee /tmp/ios_install.log
    
    # Check exit code
    MAKE_EXIT_CODE=$?
    
    if [ $MAKE_EXIT_CODE -eq 0 ]; then
        echo "✅ iOS pre-build completed successfully"
    else
        echo "❌ iOS pre-build failed with exit code: $MAKE_EXIT_CODE"
        echo "Please check /tmp/ios_install.log for details"
        # DO NOT proceed with tests
        exit 1
    fi
fi
```

### Step 4: Run Tests

#### iOS Tests

```bash
for TEST_FILE in $IOS_TESTS; do
    # Extract test class name from file path
    TEST_CLASS=$(basename "$TEST_FILE" .swift)
    
    # Determine test target based on file location
    if [[ "$TEST_FILE" == *"GlipTests/Phone"* ]]; then
        TEST_TARGET="RCPhoneTests"
    elif [[ "$TEST_FILE" == *"GlipTests/AIHub"* ]]; then
        TEST_TARGET="RCAIHubTests"
    else
        TEST_TARGET="GlipTests"
    fi
    
    echo "Running: $TEST_TARGET/$TEST_CLASS"
    
    xcodebuild test \
        -workspace Glip.xcworkspace \
        -scheme $TEST_TARGET \
        -destination 'platform=iOS Simulator,name=iPhone 16' \
        -only-testing:$TEST_TARGET/$TEST_CLASS \
        2>&1 | tee /tmp/ut_${TEST_CLASS}.log
    
    # Parse results
    if grep -q "Test Suite.*passed" /tmp/ut_${TEST_CLASS}.log; then
        PASSED_COUNT=$(grep "Executed.*tests" /tmp/ut_${TEST_CLASS}.log | tail -1 | grep -oE "[0-9]+ tests" | head -1)
        echo "✅ $TEST_CLASS: $PASSED_COUNT passed"
    else
        echo "❌ $TEST_CLASS: FAILED"
        grep "error:" /tmp/ut_${TEST_CLASS}.log | head -5
    fi
done
```

#### Android Tests

```bash
for TEST_FILE in $ANDROID_TESTS; do
    # Extract package and class name
    PACKAGE=$(grep "^package" "$TEST_FILE" | head -1 | sed 's/package //' | tr -d ';')
    TEST_CLASS=$(basename "$TEST_FILE" .kt)
    FULL_TEST_NAME="${PACKAGE}.${TEST_CLASS}"
    
    # Determine module from path
    MODULE=$(echo "$TEST_FILE" | grep -oE "module-[^/]+" | head -1)
    
    echo "Running: $FULL_TEST_NAME"
    
    ./gradlew :${MODULE}:testDebugUnitTest --tests "$FULL_TEST_NAME" 2>&1 | tee /tmp/ut_${TEST_CLASS}.log
    
    if [ $? -eq 0 ]; then
        echo "✅ $TEST_CLASS: passed"
    else
        echo "❌ $TEST_CLASS: FAILED"
    fi
done
```

#### C++ Tests

```bash
for TEST_FILE in $CPP_TESTS; do
    # Extract test name
    TEST_NAME=$(basename "$TEST_FILE" .cpp)
    
    # Determine module from path
    MODULE_PATH=$(dirname "$TEST_FILE" | sed 's|/tests/.*||')
    
    echo "Building and running: $TEST_NAME"
    
    cd "$MODULE_PATH"
    cmake -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --target $TEST_NAME
    
    ./build/tests/$TEST_NAME 2>&1 | tee /tmp/ut_${TEST_NAME}.log
    
    if [ $? -eq 0 ]; then
        echo "✅ $TEST_NAME: passed"
    else
        echo "❌ $TEST_NAME: FAILED"
    fi
done
```

### Step 5: Generate Summary Report

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 ut_run Summary
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Platform   │ Files │ Tests │ Passed │ Failed
───────────┼───────┼───────┼────────┼────────
iOS        │   X   │  XXX  │  XXX   │   X
Android    │   X   │   XX  │   XX   │   X
C++        │   X   │   XX  │   XX   │   X
───────────┼───────┼───────┼────────┼────────
Total      │   X   │  XXX  │  XXX   │   X
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⏱️ Total Time: XXX.Xs
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Progress Display Format

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 ut_run Progress (Step X/5)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[✅] Step 1: Detect Changed UT Files     (X.Xs)
[✅] Step 2: Group by Platform           (X.Xs)
[🔄] Step 3: iOS Pre-build               (in progress...)
     Running: make ios_install_with_binary_cache
     [=====>                    ] 25%
[ ] Step 4: Run Tests
[ ] Step 5: Report Results
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⏱️ Elapsed: XX.Xs
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Error Handling

### iOS Pre-build Failure

If `make ios_install_with_binary_cache` fails:

1. **DO NOT proceed** with iOS tests
2. Display error message with log location
3. Suggest running `make ios_install_with_source` as alternative
4. Continue with Android/C++ tests if any

### Test Failure

If any test fails:

1. Continue running remaining tests
2. Collect all failures
3. Display summary with failed test details
4. Return non-zero exit code

## Key Rules

1. **iOS Pre-build is MANDATORY** - Never run iOS tests without completing `make ios_install_with_binary_cache`
2. **Wait for completion** - Monitor the make command output until it finishes
3. **Handle failures gracefully** - Continue with other platforms if one fails
4. **Track timing** - Record time for each step
5. **No skipping** - Run ALL changed UT files, not just a subset
