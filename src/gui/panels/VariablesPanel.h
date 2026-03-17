#pragma once
#include "core/Project.h"
#include <QDockWidget>
#include <QListWidget>

class VariablesPanel : public QDockWidget {
  Q_OBJECT
public:
  VariablesPanel(Project *project, QWidget *parent = nullptr);

public slots:
  void refresh();

signals:
  void variableClicked(const QUuid &nodeId);

private:
  Project *m_project;
  QListWidget *m_list;
  QMap<QListWidgetItem *, QUuid> m_itemToNodeId;
};
