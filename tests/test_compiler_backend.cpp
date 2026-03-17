#include "compiler/CompilerBackend.h"
#include <QSignalSpy>
#include <QTimer>
#include <gtest/gtest.h>

// ── LocalCompiler Construction ──────────────────────────────────────────────

TEST(CompilerBackendTest, ConstructWithoutCrash) {
  LocalCompiler compiler;
  // Should not crash on construction
  SUCCEED();
}

TEST(CompilerBackendTest, StopWithoutRunDoesNotCrash) {
  LocalCompiler compiler;
  compiler.stop();
  SUCCEED();
}

TEST(CompilerBackendTest, StopMultipleTimesDoesNotCrash) {
  LocalCompiler compiler;
  compiler.stop();
  compiler.stop();
  compiler.stop();
  SUCCEED();
}

// ── Run C++ code ────────────────────────────────────────────────────────────

TEST(CompilerBackendTest, RunCppHelloWorld) {
  LocalCompiler compiler;
  QSignalSpy outputSpy(&compiler, &CompilerBackend::outputReady);
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  QString code = R"(
#include <iostream>
int main() {
    std::cout << "hello_from_test" << std::endl;
    return 0;
}
)";
  compiler.run(code, "C++");

  // Wait for process to finish (up to 15 seconds for compilation)
  ASSERT_TRUE(finishedSpy.wait(15000));
  EXPECT_EQ(finishedSpy[0][0].toInt(), 0);

  // Check output
  QString allOutput;
  for (const auto &call : outputSpy) {
    allOutput += call[0].toString();
  }
  EXPECT_TRUE(allOutput.contains("hello_from_test"));
}

TEST(CompilerBackendTest, RunCppCompileError) {
  LocalCompiler compiler;
  QSignalSpy errorSpy(&compiler, &CompilerBackend::errorReady);
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  QString code = "this is not valid c++ code!!!";
  compiler.run(code, "C++");

  ASSERT_TRUE(finishedSpy.wait(15000));
  // Should have non-zero exit code
  EXPECT_NE(finishedSpy[0][0].toInt(), 0);

  // Should have error output
  QString allErrors;
  for (const auto &call : errorSpy) {
    allErrors += call[0].toString();
  }
  EXPECT_FALSE(allErrors.isEmpty());
}

// ── Run Python code ─────────────────────────────────────────────────────────

TEST(CompilerBackendTest, RunPythonHelloWorld) {
  LocalCompiler compiler;
  QSignalSpy outputSpy(&compiler, &CompilerBackend::outputReady);
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  compiler.run("print('hello_python_test')", "Python");

  ASSERT_TRUE(finishedSpy.wait(10000));
  EXPECT_EQ(finishedSpy[0][0].toInt(), 0);

  QString allOutput;
  for (const auto &call : outputSpy) {
    allOutput += call[0].toString();
  }
  EXPECT_TRUE(allOutput.contains("hello_python_test"));
}

TEST(CompilerBackendTest, RunPythonSyntaxError) {
  LocalCompiler compiler;
  QSignalSpy errorSpy(&compiler, &CompilerBackend::errorReady);
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  compiler.run("def (((broken", "Python");

  ASSERT_TRUE(finishedSpy.wait(10000));
  EXPECT_NE(finishedSpy[0][0].toInt(), 0);
}

// ── Run JavaScript code ─────────────────────────────────────────────────────

TEST(CompilerBackendTest, RunJavaScriptHelloWorld) {
  LocalCompiler compiler;
  QSignalSpy outputSpy(&compiler, &CompilerBackend::outputReady);
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  compiler.run("console.log('hello_js_test');", "JavaScript");

  ASSERT_TRUE(finishedSpy.wait(10000));
  EXPECT_EQ(finishedSpy[0][0].toInt(), 0);

  QString allOutput;
  for (const auto &call : outputSpy) {
    allOutput += call[0].toString();
  }
  EXPECT_TRUE(allOutput.contains("hello_js_test"));
}

TEST(CompilerBackendTest, RunJavaScriptError) {
  LocalCompiler compiler;
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  compiler.run("throw new Error('test_error');", "JavaScript");

  ASSERT_TRUE(finishedSpy.wait(10000));
  EXPECT_NE(finishedSpy[0][0].toInt(), 0);
}

// ── Unsupported Language ────────────────────────────────────────────────────

TEST(CompilerBackendTest, UnsupportedLanguageEmitsError) {
  LocalCompiler compiler;
  QSignalSpy errorSpy(&compiler, &CompilerBackend::errorReady);
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  compiler.run("code", "Rust");

  // Should emit error and finished synchronously for unsupported language
  ASSERT_EQ(errorSpy.count(), 1);
  EXPECT_TRUE(errorSpy[0][0].toString().contains("Unsupported"));
  ASSERT_EQ(finishedSpy.count(), 1);
  EXPECT_EQ(finishedSpy[0][0].toInt(), 1);
}

// ── Stop running process ────────────────────────────────────────────────────

TEST(CompilerBackendTest, StopRunningProcess) {
  LocalCompiler compiler;
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  // Start a long-running process
  compiler.run("import time; time.sleep(60)", "Python");

  // Give it a moment to start, then stop
  QTimer::singleShot(500, [&compiler]() { compiler.stop(); });

  // Should finish quickly after stop
  ASSERT_TRUE(finishedSpy.wait(5000));
}

// ── Run replaces previous process ───────────────────────────────────────────

TEST(CompilerBackendTest, RunReplacePreviousProcess) {
  LocalCompiler compiler;
  QSignalSpy finishedSpy(&compiler, &CompilerBackend::finished);

  // Start a long-running process
  compiler.run("import time; time.sleep(60)", "Python");

  // Immediately run another one (should stop the first)
  compiler.run("print('replaced')", "Python");

  ASSERT_TRUE(finishedSpy.wait(10000));
  // At least one finished signal should fire
  EXPECT_GE(finishedSpy.count(), 1);
}
