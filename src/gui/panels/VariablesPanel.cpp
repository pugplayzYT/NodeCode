#include "VariablesPanel.h"
#include <QVBoxLayout>

VariablesPanel::VariablesPanel(Project *project, QWidget *parent)
    : QDockWidget("Variables", parent), m_project(project) {
  m_list = new QListWidget(this);
  setWidget(m_list);

  connect(m_list, &QListWidget::itemClicked, this,
          [this](QListWidgetItem *item) {
    if (m_itemToNodeId.contains(item))
      emit variableClicked(m_itemToNodeId[item]);
  });

  connect(project->graph(), &NodeGraph::graphChanged, this, &VariablesPanel::refresh);
  refresh();
}

void VariablesPanel::refresh() {
  m_list->clear();
  m_itemToNodeId.clear();
  for (auto *node : m_project->graph()->nodes()) {
    if (node->type().contains("Variable") || node->type().contains("Const")) {
      QString label = node->value().isEmpty() ? node->name() : node->value() + " (" + node->name() + ")";
      auto *item = new QListWidgetItem(label, m_list);
      m_itemToNodeId[item] = node->id();
    }
  }
}
