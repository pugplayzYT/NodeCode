#include "ConsolePanel.h"

ConsolePanel::ConsolePanel(QWidget *parent)
    : QDockWidget("Console", parent) {
  auto *container = new QWidget(this);
  auto *layout = new QVBoxLayout(container);
  layout->setContentsMargins(4, 4, 4, 4);

  m_output = new QTextEdit(container);
  m_output->setReadOnly(true);
  m_output->setFont(QFont("monospace", 11));
  layout->addWidget(m_output);

  auto *btnLayout = new QHBoxLayout();
  auto *clearBtn = new QPushButton("Clear", container);
  connect(clearBtn, &QPushButton::clicked, this, &ConsolePanel::clear);
  btnLayout->addStretch();
  btnLayout->addWidget(clearBtn);
  layout->addLayout(btnLayout);

  setWidget(container);
}

void ConsolePanel::appendOutput(const QString &text) {
  m_output->setTextColor(QColor(160, 240, 160));
  m_output->append(text);
}

void ConsolePanel::appendError(const QString &text) {
  m_output->setTextColor(QColor(244, 67, 54));
  m_output->append(text);
}

void ConsolePanel::clear() {
  m_output->clear();
}
