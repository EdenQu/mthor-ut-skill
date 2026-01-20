//
//  {ClassName}Tests.swift
//  {ModuleName}-Unit-Tests
//
//  Unit tests for {ClassName}
//

@testable import {ModuleName}
import UnitTestCommon
import XCTest

/// Unit tests for {ClassName}
final class {ClassName}Tests: XCBaseTestCase {
    // MARK: - Properties
    
    private var sut: {ClassName}!
    private var mock{Dependency}: Mock{Dependency}!
    
    // MARK: - Setup & Teardown
    
    override func setUp() {
        super.setUp()
        mock{Dependency} = Mock{Dependency}()
        sut = {ClassName}(dependency: mock{Dependency})
    }
    
    override func tearDown() {
        sut = nil
        mock{Dependency} = nil
        super.tearDown()
    }
    
    // MARK: - {MethodName} Tests
    
    /// Test {methodName} with valid input returns expected result
    func test{MethodName}_WithValidInput_ReturnsExpectedResult() {
        // Arrange
        let input = "test_input"
        let expectedOutput = "expected_output"
        
        // Act
        let result = sut.{methodName}(input)
        
        // Assert
        XCTAssertEqual(result, expectedOutput)
    }
    
    /// Test {methodName} with nil input returns nil
    func test{MethodName}_WithNilInput_ReturnsNil() {
        // Arrange
        let input: String? = nil
        
        // Act
        let result = sut.{methodName}(input)
        
        // Assert
        XCTAssertNil(result)
    }
    
    /// Test {methodName} with empty input returns empty result
    func test{MethodName}_WithEmptyInput_ReturnsEmpty() {
        // Arrange
        let input = ""
        
        // Act
        let result = sut.{methodName}(input)
        
        // Assert
        XCTAssertTrue(result.isEmpty)
    }
    
    /// Test {methodName} when error occurs throws expected error
    func test{MethodName}_WhenErrorOccurs_ThrowsExpectedError() {
        // Arrange
        mock{Dependency}.shouldThrowError = true
        
        // Act & Assert
        XCTAssertThrowsError(try sut.{methodName}()) { error in
            XCTAssertTrue(error is {ExpectedErrorType})
        }
    }
    
    // MARK: - Async Tests
    
    /// Test async {methodName} completes successfully
    func test{MethodName}Async_WithValidInput_CompletesSuccessfully() {
        // Arrange
        let expectation = expectation(description: "{methodName} completes")
        var capturedResult: {ResultType}?
        
        // Act
        sut.{methodName}Async { result in
            capturedResult = result
            expectation.fulfill()
        }
        
        // Assert
        wait(for: [expectation], timeout: 1.0)
        XCTAssertNotNil(capturedResult)
    }
    
    /// Test async {methodName} handles failure correctly
    func test{MethodName}Async_WhenFails_ReturnsError() {
        // Arrange
        let expectation = expectation(description: "{methodName} fails")
        mock{Dependency}.shouldFail = true
        var capturedError: Error?
        
        // Act
        sut.{methodName}Async { result in
            if case .failure(let error) = result {
                capturedError = error
            }
            expectation.fulfill()
        }
        
        // Assert
        wait(for: [expectation], timeout: 1.0)
        XCTAssertNotNil(capturedError)
    }
    
    // MARK: - Dependency Interaction Tests
    
    /// Test {methodName} calls dependency with correct parameters
    func test{MethodName}_CallsDependencyWithCorrectParameters() {
        // Arrange
        let expectedParam = "test_param"
        
        // Act
        sut.{methodName}(param: expectedParam)
        
        // Assert
        XCTAssertTrue(mock{Dependency}.{dependencyMethod}Called)
        XCTAssertEqual(mock{Dependency}.lastParam, expectedParam)
    }
}

// MARK: - Mock Classes

class Mock{Dependency}: {DependencyProtocol} {
    var shouldThrowError = false
    var shouldFail = false
    var {dependencyMethod}Called = false
    var lastParam: String?
    
    func {dependencyMethod}(param: String) {
        {dependencyMethod}Called = true
        lastParam = param
    }
}
