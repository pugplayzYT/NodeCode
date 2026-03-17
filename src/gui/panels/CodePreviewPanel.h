#pragma once
#include "core/Project.h"
#include <QDockWidget>
#include <QTextEdit>
#include <QTimer>

class SyntaxHighlighter;

class CodePreviewPanel : public QDockWidget {
  Q_OBJECT
public:
  CodePreviewPanel(Project *project, QWidget *parent = nullptr);

public slots:
  void scheduleRefresh();

private slots:
  void refresh();

private:
  Project *m_project;
  QTextEdit *m_textEdit;
  QTimer m_debounce;
  SyntaxHighlighter *m_highlighter;
};
