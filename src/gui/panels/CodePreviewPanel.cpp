#include "CodePreviewPanel.h"
#include "core/CodeGenerator.h"
#include "gui/widgets/SyntaxHighlighter.h"

CodePreviewPanel::CodePreviewPanel(Project *project, QWidget *parent)
    : QDockWidget("Live Code Preview", parent), m_project(project) {
  m_textEdit = new QTextEdit(this);
  m_textEdit->setReadOnly(true);
  m_textEdit->setFont(QFont("monospace", 11));
  setWidget(m_textEdit);

  m_highlighter = new SyntaxHighlighter(project->language(), m_textEdit->document());

  m_debounce.setSingleShot(true);
  m_debounce.setInterval(300);
  connect(&m_debounce, &QTimer::timeout, this, &CodePreviewPanel::refresh);

  connect(project->graph(), &NodeGraph::graphChanged, this, &CodePreviewPanel::scheduleRefresh);
}

void CodePreviewPanel::scheduleRefresh() {
  m_debounce.start();
}

void CodePreviewPanel::refresh() {
  m_highlighter->setLanguage(m_project->language());
  QString code = CodeGenerator::generate(m_project);
  m_textEdit->setPlainText(code);
}
