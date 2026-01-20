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

## ⚠️ Critical Workflow Rules

1. **ALL 6 STEPS MUST EXECUTE** - Never skip any step
2. **COVERAGE IS MANDATORY** - Always collect and report branch + code coverage
3. **NEVER END EARLY** - Complete entire workflow even if tests fail
4. **TRACK TIMING** - Record time for each step

## Workflow Steps (6 Steps Total)

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
    
    # Run from project root where Makefile is located
    cd /path/to/project/root
    
    # Run make command and capture output
    echo "Running: make ios_install_with_binary_cache"
    make ios_install_with_binary_cache 2>&1 | tee /tmp/ios_install.log
    
    # Check exit code
    MAKE_EXIT_CODE=$?
    
    if [ $MAKE_EXIT_CODE -eq 0 ]; then
        echo "✅ iOS pre-build completed successfully"
    else
        echo "⚠️ iOS pre-build failed with exit code: $MAKE_EXIT_CODE"
        echo "Attempting with existing build..."
        # Continue with existing build - may work if dependencies already installed
    fi
fi
```

### Step 4: Run Tests WITH Code Coverage

**⚠️ CRITICAL: MUST enable code coverage when running tests**

#### iOS Tests (with Coverage)

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
    
    # CRITICAL: Use -enableCodeCoverage YES and -resultBundlePath for coverage
    xcodebuild test \
        -workspace Glip.xcworkspace \
        -scheme $TEST_TARGET \
        -destination 'platform=iOS Simulator,name=iPhone 16' \
        -only-testing:$TEST_TARGET/$TEST_CLASS \
        -enableCodeCoverage YES \
        -resultBundlePath /tmp/ut_${TEST_CLASS}_result.xcresult \
        2>&1 | tee /tmp/ut_${TEST_CLASS}.log
    
    # Parse test results
    if grep -q "Test Suite.*passed" /tmp/ut_${TEST_CLASS}.log; then
        PASSED_COUNT=$(grep "Executed.*tests" /tmp/ut_${TEST_CLASS}.log | tail -1 | grep -oE "[0-9]+ tests" | head -1)
        echo "✅ $TEST_CLASS: $PASSED_COUNT passed"
    else
        echo "❌ $TEST_CLASS: FAILED"
        grep "error:" /tmp/ut_${TEST_CLASS}.log | head -5
    fi
done
```

#### Android Tests (with Coverage)

```bash
for TEST_FILE in $ANDROID_TESTS; do
    # Extract package and class name
    PACKAGE=$(grep "^package" "$TEST_FILE" | head -1 | sed 's/package //' | tr -d ';')
    TEST_CLASS=$(basename "$TEST_FILE" .kt)
    FULL_TEST_NAME="${PACKAGE}.${TEST_CLASS}"
    
    # Determine module from path
    MODULE=$(echo "$TEST_FILE" | grep -oE "module-[^/]+" | head -1)
    
    echo "Running: $FULL_TEST_NAME"
    
    # Enable JaCoCo coverage
    ./gradlew :${MODULE}:testDebugUnitTest \
        --tests "$FULL_TEST_NAME" \
        jacocoTestReport \
        2>&1 | tee /tmp/ut_${TEST_CLASS}.log
    
    if [ $? -eq 0 ]; then
        echo "✅ $TEST_CLASS: passed"
    else
        echo "❌ $TEST_CLASS: FAILED"
    fi
done
```

#### C++ Tests (with Coverage)

```bash
for TEST_FILE in $CPP_TESTS; do
    # Extract test name
    TEST_NAME=$(basename "$TEST_FILE" .cpp)
    
    # Determine module from path
    MODULE_PATH=$(dirname "$TEST_FILE" | sed 's|/tests/.*||')
    
    echo "Building and running: $TEST_NAME"
    
    cd "$MODULE_PATH"
    # Build with coverage flags
    cmake -B build -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="--coverage" \
        -DCMAKE_C_FLAGS="--coverage"
    cmake --build build --target $TEST_NAME
    
    ./build/tests/$TEST_NAME 2>&1 | tee /tmp/ut_${TEST_NAME}.log
    
    # Generate coverage report
    gcov -r $(find build -name "*.gcda") 2>/dev/null
    
    if [ $? -eq 0 ]; then
        echo "✅ $TEST_NAME: passed"
    else
        echo "❌ $TEST_NAME: FAILED"
    fi
done
```

### Step 5: Collect Coverage Data

**⚠️ CRITICAL: This step MUST execute - coverage data is REQUIRED**

#### iOS Coverage Collection

```bash
# Extract coverage from xcresult bundle
for RESULT_BUNDLE in /tmp/ut_*_result.xcresult; do
    if [ -d "$RESULT_BUNDLE" ]; then
        TEST_NAME=$(basename "$RESULT_BUNDLE" _result.xcresult)
        
        # Get coverage report using xcrun xccov
        xcrun xccov view --report "$RESULT_BUNDLE" > /tmp/${TEST_NAME}_coverage.txt 2>/dev/null
        
        # Extract file-level coverage for the source file
        # Look for the source file that matches the test class
        SOURCE_FILE=$(echo "$TEST_NAME" | sed 's/Tests$//')
        
        # Parse coverage data
        LINE_COVERAGE=$(xcrun xccov view --report "$RESULT_BUNDLE" 2>/dev/null | \
            grep -A5 "${SOURCE_FILE}" | \
            grep -oE "[0-9]+\.[0-9]+%" | head -1)
        
        # Get branch coverage (from detailed report)
        xcrun xccov view --report --json "$RESULT_BUNDLE" 2>/dev/null > /tmp/${TEST_NAME}_coverage.json
        
        # Parse JSON for branch coverage
        BRANCH_COVERAGE=$(cat /tmp/${TEST_NAME}_coverage.json | \
            grep -oE '"branchCoverage":[0-9.]+' | head -1 | \
            sed 's/"branchCoverage"://' | \
            awk '{printf "%.1f%%", $1 * 100}')
        
        echo "Coverage for ${SOURCE_FILE}: Line=${LINE_COVERAGE:-N/A}, Branch=${BRANCH_COVERAGE:-N/A}"
    fi
done
```

#### Alternative iOS Coverage (using llvm-cov)

```bash
# If xcresult doesn't provide detailed coverage, use llvm-cov
PROFDATA=$(find ~/Library/Developer/Xcode/DerivedData -name "*.profdata" -newer /tmp/ut_*.log | head -1)
BINARY=$(find ~/Library/Developer/Xcode/DerivedData -name "*.xctest" -type d -newer /tmp/ut_*.log | head -1)

if [ -n "$PROFDATA" ] && [ -n "$BINARY" ]; then
    xcrun llvm-cov report "${BINARY}/Contents/MacOS/*" -instr-profile="$PROFDATA" \
        --ignore-filename-regex=".*/Tests/.*" 2>/dev/null | tee /tmp/llvm_coverage.txt
fi
```

#### Android Coverage Collection

```bash
# JaCoCo report location
JACOCO_REPORT=$(find . -path "**/reports/jacoco/**/jacocoTestReport.xml" | head -1)

if [ -f "$JACOCO_REPORT" ]; then
    # Parse JaCoCo XML for coverage
    LINE_COVERAGE=$(grep -oE 'LINE[^>]*covered="[0-9]+"' "$JACOCO_REPORT" | \
        awk -F'"' '{c+=$2; m+=$4} END {printf "%.1f%%", (c/(c+m))*100}')
    
    BRANCH_COVERAGE=$(grep -oE 'BRANCH[^>]*covered="[0-9]+"' "$JACOCO_REPORT" | \
        awk -F'"' '{c+=$2; m+=$4} END {printf "%.1f%%", (c/(c+m))*100}')
    
    echo "Android Coverage: Line=${LINE_COVERAGE}, Branch=${BRANCH_COVERAGE}"
fi
```

#### C++ Coverage Collection

```bash
# Use gcov/lcov for C++ coverage
lcov --capture --directory build --output-file /tmp/coverage.info 2>/dev/null
lcov --remove /tmp/coverage.info '/usr/*' --output-file /tmp/coverage_filtered.info 2>/dev/null

# Generate summary
LINE_COVERAGE=$(lcov --summary /tmp/coverage_filtered.info 2>&1 | grep "lines" | grep -oE "[0-9.]+%")
BRANCH_COVERAGE=$(lcov --summary /tmp/coverage_filtered.info 2>&1 | grep "branches" | grep -oE "[0-9.]+%")

echo "C++ Coverage: Line=${LINE_COVERAGE:-N/A}, Branch=${BRANCH_COVERAGE:-N/A}"
```

### Step 6: Generate Summary Report WITH Coverage

**⚠️ CRITICAL: Report MUST include both Branch Coverage and Code Coverage**

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 ut_run Summary
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Platform   │ Files │ Tests │ Passed │ Failed │ Code Coverage │ Branch Coverage
───────────┼───────┼───────┼────────┼────────┼───────────────┼────────────────
iOS        │   X   │  XXX  │  XXX   │   X    │   XX.X%       │    XX.X%
Android    │   X   │   XX  │   XX   │   X    │   XX.X%       │    XX.X%
C++        │   X   │   XX  │   XX   │   X    │   XX.X%       │    XX.X%
───────────┼───────┼───────┼────────┼────────┼───────────────┼────────────────
Total      │   X   │  XXX  │  XXX   │   X    │   XX.X%       │    XX.X%
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⏱️ Total Time: XXX.Xs
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📈 Coverage Details by File:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Test Class                    │ Source File             │ Line    │ Branch
──────────────────────────────┼─────────────────────────┼─────────┼────────
ConferenceInteractorTests     │ ConferenceInteractor    │  XX.X%  │  XX.X%
UserServiceTest               │ UserService             │  XX.X%  │  XX.X%
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

✅ Coverage Target: 100% Line, 100% Branch
📊 Status: [PASS/FAIL based on coverage]
```

## Progress Display Format (6 Steps)

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 ut_run Progress (Step X/6)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[✅] Step 1: Detect Changed UT Files     (X.Xs)
[✅] Step 2: Group by Platform           (X.Xs)
[✅] Step 3: iOS Pre-build               (X.Xs)
[🔄] Step 4: Run Tests with Coverage     (in progress...)
     Running: ConferenceInteractorTests
[ ] Step 5: Collect Coverage Data
[ ] Step 6: Generate Report
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⏱️ Elapsed: XX.Xs
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Error Handling

### iOS Pre-build Failure

If `make ios_install_with_binary_cache` fails:

1. **Log the error** but DO NOT stop completely
2. Display error message with log location
3. Suggest running `make ios_install_with_source` as alternative
4. **TRY to continue** with existing build (may work if dependencies installed)
5. Continue with Android/C++ tests if any

### Test Failure

If any test fails:

1. **Continue running** remaining tests
2. **Still collect coverage** for passed tests
3. Display summary with failed test details
4. Return non-zero exit code at end

### Coverage Collection Failure

If coverage data unavailable:

1. **Report "N/A"** for coverage metrics
2. **DO NOT skip** the coverage step
3. Document reason for missing coverage
4. **Still complete** Step 6 (Generate Report)

## Key Rules

1. **ALL 6 STEPS MUST EXECUTE** - Never skip any step, even on failures
2. **iOS Pre-build is MANDATORY** - Attempt make command, continue with existing if fails
3. **COVERAGE IS REQUIRED** - Enable coverage flags, collect data, report results
4. **Branch + Code Coverage** - Both metrics MUST appear in final report
5. **Track timing** - Record time for each step
6. **No skipping** - Run ALL changed UT files, complete ALL workflow steps
7. **Never end early** - Always generate final report with whatever data is available
