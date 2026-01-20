# ut_path Command Prompt

## Trigger Pattern
```
ut_path --path=<directory_path> [--platform=<ios|android|cpp|all>]
```

## Examples
```
ut_path --path=phone/ios/Phone/Service
ut_path --path=video/android/module-video/src/main/java/com/glip/video/meeting
ut_path --path=phone/cpp/core-phone/src/cc_lt --platform=cpp
ut_path path=common/ios/RCCopilotSDK/ChatBot/Core
```

## Execution Steps

### 1. Parse Parameters
- Extract `--path` parameter (required)
- Extract `--platform` parameter (default: `all`, auto-detect from path)

### 2. Validate Path
```bash
if [ ! -d "$PATH" ]; then
    echo "Error: Directory not found: $PATH"
    exit 1
fi
```

### 3. Scan Source Files

```bash
find "$PATH" -type f \( \
    -name "*.swift" \
    -o -name "*.kt" \
    -o -name "*.java" \
    -o -name "*.cpp" \
    -o -name "*.hpp" \
\) | sort
```

### 4. Filter Source Files

Apply the same filtering rules as `ut_changed`:
- Exclude test files
- Exclude mock files
- Exclude UI-only files

### 5. Display Analysis Results

Output format:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔍 UT Path Analysis
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📂 Target path: ${PATH}

📊 Source Files Found:
   Total: X files
   Platform: iOS/Android/C++

📁 Files to Test:
   1. path/to/File1.swift → Tests/File1Tests.swift [CREATE/UPDATE]
   2. path/to/File2.swift → Tests/File2Tests.swift [CREATE/UPDATE]
   ...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### 6. Generate Unit Tests

Follow the standard UT generation workflow for each file.

### 7. Build and Run Tests

Apply platform-specific build and test commands.

### 8. Output Coverage Report

Display coverage results for all generated tests.

## Reference Rules
- iOS: `.cursor/rules/unit-test/ios-unit-test.mdc`
- Android: `.cursor/rules/unit-test/android-unit-test.mdc`
- C++: `.cursor/rules/unit-test/cpp-unit-test.mdc`
