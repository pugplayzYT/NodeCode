#pragma once
#include "core/Project.h"
#include <QDockWidget>
#include <QLineEdit>
#include <QTreeWidget>

class NodeLibraryPanel : public QDockWidget {
  Q_OBJECT
public:
  NodeLibraryPanel(Project *project, QWidget *parent = nullptr);
  void refresh();

signals:
  void nodeRequested(const NodeDef &def, const QPointF &pos);

private slots:
  void onFilterChanged(const QString &text);

private:
  Project *m_project;
  QLineEdit *m_search;
  QTreeWidget *m_tree;
  QMap<QTreeWidgetItem *, QString> m_itemToType;
};
