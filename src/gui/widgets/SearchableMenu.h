#pragma once
#include "core/NodeDef.h"
#include "core/Project.h"
#include <QLineEdit>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

class SearchableMenu : public QWidget {
  Q_OBJECT
public:
  SearchableMenu(Project *project, QWidget *parent = nullptr);

signals:
  void nodeSelected(const NodeDef &def);

private slots:
  void onFilterChanged(const QString &text);
  void onItemActivated(QTreeWidgetItem *item, int column);

private:
  void buildTree();
  Project *m_project;
  QLineEdit *m_search;
  QTreeWidget *m_tree;
  QMap<QTreeWidgetItem *, QString> m_itemToType; // maps tree items to node def types
};
