//
//  {class_name}_test.cpp
//  core-{module}-Unit-Tests
//
//  Unit tests for {ClassName}
//

#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../ut_base/init_glipcore_test.hpp"
#include "../../mock_service/{category}/mock_{dependency}.hpp"

// Access private members for testing
#define private public
#define protected public
#include "{module}/src/{category}/{class_name}.hpp"
#undef private
#undef protected

using namespace glip_mobile;
using namespace ::testing;
using namespace {namespace};

// ============================================================
// Test Fixture
// ============================================================

class {ClassName}Test : public InitGlipCoreTest
{
public:
    void SetUp() override
    {
        InitGlipCoreTest::SetUp();
        
        // Initialize mocks
        m_mock{Dependency} = std::make_shared<Mock{Dependency}>();
        
        // Initialize SUT
        m_sut = std::make_shared<{ClassName}>(getUnifiedFactory());
    }
    
    void TearDown() override
    {
        m_sut.reset();
        m_mock{Dependency}.reset();
        InitGlipCoreTest::TearDown();
    }
    
protected:
    std::shared_ptr<{ClassName}> m_sut;
    std::shared_ptr<Mock{Dependency}> m_mock{Dependency};
};

// ============================================================
// Basic Functionality Tests
// ============================================================

TEST_F({ClassName}Test, test{MethodName}_WithValidInput_ReturnsExpectedResult)
{
    // Arrange
    auto waiter = std::make_shared<Future<bool>>();
    std::string input = "test_input";
    std::string expectedOutput = "expected_output";
    
    EXPECT_CALL(*m_mock{Dependency}, {dependencyMethod}(input))
        .WillOnce(Return(expectedOutput));
    
    // Act
    auto result = m_sut->{methodName}(input);
    
    // Assert
    EXPECT_EQ(result, expectedOutput);
}

TEST_F({ClassName}Test, test{MethodName}_WithEmptyInput_ReturnsEmpty)
{
    // Arrange
    std::string input = "";
    
    // Act
    auto result = m_sut->{methodName}(input);
    
    // Assert
    EXPECT_TRUE(result.empty());
}

TEST_F({ClassName}Test, test{MethodName}_WithNullptr_HandlesGracefully)
{
    // Arrange
    std::string* input = nullptr;
    
    // Act & Assert - Should not crash
    EXPECT_NO_THROW(m_sut->{methodName}(input));
}

// ============================================================
// Async Operation Tests
// ============================================================

TEST_F({ClassName}Test, test{MethodName}Async_CompletesSuccessfully)
{
    // Arrange
    auto waiter = std::make_shared<Future<bool>>();
    std::string capturedResult;
    
    EXPECT_CALL(*m_mock{Dependency}, {dependencyMethod}(_))
        .WillOnce(Invoke([waiter, &capturedResult](const std::string& result) {
            capturedResult = result;
            waiter->setResult(std::make_shared<bool>(true));
        }));
    
    // Act
    m_sut->{methodName}Async("test_param");
    
    // Assert
    auto status = waiter->waitFor(10 * 1000);
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_FALSE(capturedResult.empty());
}

TEST_F({ClassName}Test, test{MethodName}Async_WithCallback_InvokesCallback)
{
    // Arrange
    std::promise<std::pair<bool, std::string>> resultPromise;
    auto resultFuture = resultPromise.get_future();
    
    // Act
    m_sut->{methodName}Async("test_param", 
        [&resultPromise](bool success, const std::string& data) {
            resultPromise.set_value(std::make_pair(success, data));
        });
    
    // Assert
    auto status = resultFuture.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(status, std::future_status::ready);
    
    auto result = resultFuture.get();
    EXPECT_TRUE(result.first);
    EXPECT_FALSE(result.second.empty());
}

// ============================================================
// Error Handling Tests
// ============================================================

TEST_F({ClassName}Test, test{MethodName}_WhenDependencyFails_HandlesError)
{
    // Arrange
    EXPECT_CALL(*m_mock{Dependency}, {dependencyMethod}(_))
        .WillOnce(Throw(std::runtime_error("Test error")));
    
    // Act
    auto result = m_sut->{methodName}("input");
    
    // Assert
    EXPECT_TRUE(result.empty());
    // Or verify error state
    EXPECT_TRUE(m_sut->hasError());
}

// ============================================================
// State Management Tests
// ============================================================

TEST_F({ClassName}Test, test{MethodName}_UpdatesStateCorrectly)
{
    // Arrange
    auto initialState = m_sut->getState();
    EXPECT_EQ(initialState, E{ClassName}State::IDLE);
    
    // Act
    m_sut->{methodName}("test_param");
    
    // Assert
    auto newState = m_sut->getState();
    EXPECT_EQ(newState, E{ClassName}State::ACTIVE);
}

TEST_F({ClassName}Test, testMultipleSessions_HaveIndependentStates)
{
    // Arrange
    std::string sessionId1 = "session-1";
    std::string sessionId2 = "session-2";
    
    // Act
    m_sut->addSession(sessionId1);
    m_sut->addSession(sessionId2);
    m_sut->updateSession(sessionId1, "value1");
    m_sut->updateSession(sessionId2, "value2");
    
    // Assert
    EXPECT_EQ(m_sut->getSessionValue(sessionId1), "value1");
    EXPECT_EQ(m_sut->getSessionValue(sessionId2), "value2");
}

// ============================================================
// Cleanup Tests
// ============================================================

TEST_F({ClassName}Test, testCleanup_ReleasesResources)
{
    // Arrange
    m_sut->{methodName}("setup_data");
    EXPECT_TRUE(m_sut->hasActiveResources());
    
    // Act
    m_sut->cleanup();
    
    // Assert
    EXPECT_FALSE(m_sut->hasActiveResources());
}

// ============================================================
// Mock Class Definition (if not in separate file)
// ============================================================

class Mock{Dependency} : public I{Dependency}
{
public:
    MOCK_METHOD({ReturnType}, {dependencyMethod}, (const std::string& param), (override));
    MOCK_METHOD(void, {asyncMethod}, (const std::function<void(bool, const std::string&)>& callback), (override));
};
