package com.glip.{module}.{package}

import com.glip.BaseRobolectricTest
import io.mockk.every
import io.mockk.slot
import io.mockk.verify
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import kotlin.test.assertEquals
import kotlin.test.assertFalse
import kotlin.test.assertNotNull
import kotlin.test.assertNull
import kotlin.test.assertTrue

/**
 * Unit tests for {ClassName}
 */
@RunWith(RobolectricTestRunner::class)
class {ClassName}Test : BaseRobolectricTest() {

    // MARK: - Properties
    
    private lateinit var sut: {ClassName}
    
    // Use docker() to create mocks - NOT @MockK
    private val mock{Dependency}: {DependencyType} = docker()
    
    // Callback slot for capturing
    private val callbackSlot = slot<{CallbackType}>()

    // MARK: - Setup & Teardown
    
    @Before
    fun setup() {
        // Configure default mock behavior
        every { mock{Dependency}.{method}() } returns {defaultValue}
        
        // Initialize SUT
        sut = {ClassName}(mock{Dependency})
    }

    @After
    fun tearDown() {
        // Cleanup
    }

    // MARK: - {MethodName} Tests

    @Test
    fun test{MethodName}WithValidInputReturnsExpectedResult() {
        // Arrange
        val input = "test_input"
        val expected = "expected_output"
        every { mock{Dependency}.{method}(input) } returns expected

        // Act
        val result = sut.{methodName}(input)

        // Assert
        assertEquals(expected, result)
    }

    @Test
    fun test{MethodName}WithNullInputReturnsNull() {
        // Arrange
        val input: String? = null

        // Act
        val result = sut.{methodName}(input)

        // Assert
        assertNull(result)
    }

    @Test
    fun test{MethodName}WithEmptyInputReturnsEmpty() {
        // Arrange
        val input = ""

        // Act
        val result = sut.{methodName}(input)

        // Assert
        assertTrue(result.isEmpty())
    }

    // MARK: - State Tests

    @Test
    fun test{MethodName}UpdatesStateCorrectly() {
        // Arrange
        val initialState = sut.currentState
        assertFalse(initialState.isActive)

        // Act
        sut.{methodName}()

        // Assert
        val newState = sut.currentState
        assertTrue(newState.isActive)
    }

    // MARK: - Callback Tests

    @Test
    fun test{MethodName}InvokesCallbackWithResult() {
        // Arrange
        every { mock{Dependency}.{method}(capture(callbackSlot)) } answers {
            callbackSlot.captured.onResult("success")
        }
        var capturedResult: String? = null

        // Act
        sut.{methodName} { result ->
            capturedResult = result
        }

        // Assert
        assertNotNull(capturedResult)
        assertEquals("success", capturedResult)
    }

    // MARK: - Dependency Interaction Tests

    @Test
    fun test{MethodName}CallsDependencyWithCorrectParameters() {
        // Arrange
        val expectedParam = "test_param"

        // Act
        sut.{methodName}(expectedParam)

        // Assert
        verify(exactly = 1) { mock{Dependency}.{dependencyMethod}(expectedParam) }
    }

    @Test
    fun test{MethodName}DoesNotCallDependencyWhenConditionFalse() {
        // Arrange
        every { mock{Dependency}.shouldProceed } returns false

        // Act
        sut.{methodName}()

        // Assert
        verify(exactly = 0) { mock{Dependency}.{dependencyMethod}(any()) }
    }

    // MARK: - Error Handling Tests

    @Test
    fun test{MethodName}HandlesExceptionGracefully() {
        // Arrange
        every { mock{Dependency}.{method}() } throws RuntimeException("Test error")

        // Act
        val result = sut.{methodName}()

        // Assert
        assertNull(result)
        // Or verify error state
        assertTrue(sut.hasError)
    }

    // MARK: - Multiple Sessions Tests

    @Test
    fun testMultipleSessionsHaveIndependentStates() {
        // Arrange
        val sessionId1 = "session-1"
        val sessionId2 = "session-2"

        // Act
        sut.{methodName}(sessionId1, value1)
        sut.{methodName}(sessionId2, value2)

        // Assert
        val result1 = sut.getState(sessionId1)
        val result2 = sut.getState(sessionId2)
        
        assertNotNull(result1)
        assertNotNull(result2)
        assertEquals(value1, result1)
        assertEquals(value2, result2)
    }
}
