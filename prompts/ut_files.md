# ut_files Command Prompt

## Trigger Pattern
```
ut_files --files="<file1,file2,...>" [--platform=<ios|android|cpp|all>]
```

## Examples
```
ut_files --files="UserService.swift,AuthManager.swift"
ut_files --files="PhoneService.kt,CallManager.kt" --platform=android
ut_files files="live_transcript_service.cpp"
ut_files --files="phone/ios/Phone/Service/TelephonyService.swift,phone/ios/Phone/Service/PageService.swift"
```

## Execution Steps

### 1. Parse Parameters
- Extract `--files` parameter (required)
- Split by comma to get file list
- Extract `--platform` parameter (default: auto-detect)

### 2. Resolve File Paths

For each file:
```bash
# If relative path, search in workspace
if [ ! -f "$file" ]; then
    found=$(find . -name "$file" -type f | head -1)
    if [ -n "$found" ]; then
        file="$found"
    else
        echo "Warning: File not found: $file"
    fi
fi
```

### 3. Validate Files

```bash
for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ Found: $file"
    else
        echo "❌ Not found: $file"
    fi
done
```

### 4. Display Analysis Results

Output format:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔍 UT Files Analysis
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 Specified Files: X

📁 Files to Test:
   ✅ path/to/File1.swift → Tests/File1Tests.swift [CREATE]
   ✅ path/to/File2.swift → Tests/File2Tests.swift [UPDATE]
   ❌ File3.swift (not found)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### 5. Generate Unit Tests

For each valid file:
1. Check if test file exists → CREATE or UPDATE mode
2. Read and analyze source file
3. Find similar tests for patterns
4. Generate test cases with 100% coverage

### 6. Handle Existing Tests

If test file exists:
- Read existing test methods
- Identify new methods needing tests
- Update outdated tests
- Skip already covered scenarios

### 7. Build and Run

Apply platform-specific build prompts and test execution.

### 8. Output Coverage Report

Display individual and aggregate coverage results.

## Reference Rules
- iOS: `.cursor/rules/unit-test/ios-unit-test.mdc`
- Android: `.cursor/rules/unit-test/android-unit-test.mdc`
- C++: `.cursor/rules/unit-test/cpp-unit-test.mdc`
