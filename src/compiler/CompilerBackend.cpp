#include "CompilerBackend.h"
#include <QDir>
#include <QTemporaryDir>
#include <QStandardPaths>

LocalCompiler::LocalCompiler(QObject *parent) : CompilerBackend(parent) {}

void LocalCompiler::run(const QString &code, const QString &language) {
  stop();

  QTemporaryDir tempDir;
  if (!tempDir.isValid()) {
    emit errorReady("Failed to create temp directory");
    emit finished(1);
    return;
  }
  tempDir.setAutoRemove(false);
  m_tempDir = tempDir.path();

  QString sourceFile, command;
  if (language == "C++") {
    sourceFile = m_tempDir + "/main.cpp";
    QFile f(sourceFile);
    f.open(QIODevice::WriteOnly);
    f.write(code.toUtf8());
    f.close();
    command = "cd " + m_tempDir + " && g++ -std=c++17 -o output main.cpp && ./output";
  } else if (language == "Python") {
    sourceFile = m_tempDir + "/main.py";
    QFile f(sourceFile);
    f.open(QIODevice::WriteOnly);
    f.write(code.toUtf8());
    f.close();
    command = "python3 " + sourceFile;
  } else if (language == "JavaScript") {
    sourceFile = m_tempDir + "/main.js";
    QFile f(sourceFile);
    f.open(QIODevice::WriteOnly);
    f.write(code.toUtf8());
    f.close();
    command = "node " + sourceFile;
  } else {
    emit errorReady("Unsupported language: " + language);
    emit finished(1);
    return;
  }

  m_process = new QProcess(this);
  connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
    emit outputReady(QString::fromUtf8(m_process->readAllStandardOutput()));
  });
  connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
    emit errorReady(QString::fromUtf8(m_process->readAllStandardError()));
  });
  connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, [this](int code, QProcess::ExitStatus) {
    emit finished(code);
    // Cleanup temp
    QDir(m_tempDir).removeRecursively();
  });

  m_process->start("/bin/bash", {"-c", command});
}

void LocalCompiler::stop() {
  if (m_process && m_process->state() != QProcess::NotRunning) {
    m_process->kill();
    m_process->waitForFinished(1000);
  }
  if (m_process) {
    delete m_process;
    m_process = nullptr;
  }
}
