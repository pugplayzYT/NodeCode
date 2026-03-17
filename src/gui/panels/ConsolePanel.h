#pragma once
#include <QDockWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

class ConsolePanel : public QDockWidget {
  Q_OBJECT
public:
  ConsolePanel(QWidget *parent = nullptr);

public slots:
  void appendOutput(const QString &text);
  void appendError(const QString &text);
  void clear();

private:
  QTextEdit *m_output;
};
