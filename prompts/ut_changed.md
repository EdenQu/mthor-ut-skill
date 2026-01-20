# ut_changed Command Prompt

## Trigger Pattern
```
ut_changed [--base=<branch>] [--platform=<ios|android|cpp|all>]
```

## Examples
```
ut_changed
ut_changed --base=origin/dev
ut_changed --base=origin/main --platform=ios
ut_changed platform=android base=feature/my-feature
```

## Execution Steps

When this command is triggered, execute the following workflow:

### 1. Parse Parameters
- Extract `--base` parameter (default: `origin/dev`)
- Extract `--platform` parameter (default: `all`)

### 2. Detect File Changes

Run these git commands to find all changed files:

```bash
# Fetch latest remote to ensure accurate comparison
git fetch origin dev 2>/dev/null

# Local uncommitted changes
git status --porcelain

# Changes compared to base branch (use remote branch by default)
# This avoids issues with stale local branches
git diff ${BASE_BRANCH}...HEAD --name-only
# Default: git diff origin/dev...HEAD --name-only
```

### 3. Filter by Platform

Based on `--platform` parameter:
- `ios`: Keep only `*.swift` files
- `android`: Keep only `*.kt`, `*.java` files
- `cpp`: Keep only `*.cpp`, `*.hpp`, `*.h` files
- `all`: Keep all supported files

### 4. Filter Source Files

Exclude test files and non-testable files:
- Exclude: `*Test*.swift`, `*Tests.swift`, `*Test.kt`, `*_test.cpp`
- Exclude: `Mock*.swift`, `Mock*.kt`, `Fake*.*`
- Exclude: `*View.swift`, `*Cell.swift`, `*ViewController.swift`
- Exclude: `*Activity.kt`, `*Fragment.kt`, `*Adapter.kt`

### 5. Determine Coverage Scope

For each source file, check if existing UT exists:

```
┌────────────────────────────────────────────────────┐
│ Coverage Scope for ut_changed                       │
├────────────────────────────────────────────────────┤
│ Has Existing UT?  │  Coverage Scope                │
├───────────────────┼────────────────────────────────┤
│ ✅ Yes            │  ENTIRE file (100% coverage)   │
│ ❌ No             │  ONLY CHANGED methods          │
└────────────────────────────────────────────────────┘
```

**For files WITHOUT existing UT:**
- Use `git diff` to extract only changed/added methods
- Generate tests ONLY for those methods
- This focuses effort on new/modified code

### 6. Display Analysis Results

Output format:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔍 UT Change Analysis
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📂 Base branch: ${BASE_BRANCH}
📂 Current branch: $(git branch --show-current)

📊 Changed Files Summary:
   🍎 iOS (.swift):     X files
   🤖 Android (.kt):    X files
   ⚙️  C++ (.cpp/.hpp):  X files

📁 Source Files to Test:
   1. path/to/File1.swift → Tests/File1Tests.swift [UPDATE - full coverage]
   2. path/to/File2.kt → test/.../File2Test.kt [CREATE - changed methods only]
   ...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### 7. Generate Unit Tests

For each source file, follow the UT generation workflow:
1. Check if test file exists
2. **If exists (UPDATE mode):**
   - Read entire source file
   - Generate tests for ALL methods (100% coverage)
3. **If NOT exists (CREATE mode):**
   - Parse `git diff` to find changed methods only
   - Generate tests ONLY for changed methods
   - Add comment: `// Tests for changed methods only`
4. Find similar tests for reference
5. Ensure 100% coverage within scope

### 8. Compile UT Files

Compile test files until all succeed:
1. Attempt compilation
2. If errors → fix and retry
3. Only proceed when compilation succeeds

### 9. iOS Build Prompt (Conditional)

**Only show if:**
- Platform is iOS AND
- At least one test file was CREATED (new file)

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔔 Action Required - iOS Build
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
New iOS test files created. Dependency installation may be required.

Options:
   1️⃣  make ios_install_with_binary_cache (faster)
   2️⃣  make ios_install_with_source (for new deps)
   3️⃣  Skip - I'll handle manually

⏳ Waiting for your input (1, 2, or 3)...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**⚠️ After user responds: IMMEDIATELY continue to Step 10**

### 10. Run Tests and Report Coverage

Execute tests and output coverage report.

---

## Workflow Completion Rules

1. **NEVER end conversation** after any step
2. **Complete ALL 10 steps** without interruption
3. **Track time** for each step and display progress
4. **Non-blocking prompts** - wait for input, then continue
5. **Show final summary** with total time when workflow completes

## Reference Rules
- iOS: `.cursor/rules/unit-test/ios-unit-test.mdc`
- Android: `.cursor/rules/unit-test/android-unit-test.mdc`
- C++: `.cursor/rules/unit-test/cpp-unit-test.mdc`
- Principles: `.cursor/rules/unit-test/unit-testing-principles.mdc`
