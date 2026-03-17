#pragma once
#include <QObject>
#include <QProcess>

class CompilerBackend : public QObject {
  Q_OBJECT
public:
  CompilerBackend(QObject *parent = nullptr) : QObject(parent) {}
  virtual void run(const QString &code, const QString &language) = 0;
  virtual void stop() = 0;

signals:
  void outputReady(const QString &text);
  void errorReady(const QString &text);
  void finished(int exitCode);
};

class LocalCompiler : public CompilerBackend {
  Q_OBJECT
public:
  LocalCompiler(QObject *parent = nullptr);
  void run(const QString &code, const QString &language) override;
  void stop() override;

private:
  QProcess *m_process = nullptr;
  QString m_tempDir;
};
