#include "NodeLibraryPanel.h"
#include <QHeaderView>
#include <QMimeData>
#include <QDrag>
#include <QVBoxLayout>

NodeLibraryPanel::NodeLibraryPanel(Project *project, QWidget *parent)
    : QDockWidget("Node Library", parent), m_project(project) {
  auto *container = new QWidget(this);
  auto *layout = new QVBoxLayout(container);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  m_search = new QLineEdit(container);
  m_search->setPlaceholderText("Search nodes...");
  layout->addWidget(m_search);

  m_tree = new QTreeWidget(container);
  m_tree->setHeaderHidden(true);
  m_tree->setDragEnabled(true);
  m_tree->setIndentation(16);
  layout->addWidget(m_tree);

  setWidget(container);

  connect(m_search, &QLineEdit::textChanged, this, &NodeLibraryPanel::onFilterChanged);
  connect(m_tree, &QTreeWidget::itemDoubleClicked, this,
          [this](QTreeWidgetItem *item, int) {
    if (!m_itemToType.contains(item)) return;
    auto defs = m_project->nodeDefinitions();
    QString type = m_itemToType[item];
    if (defs.contains(type))
      emit nodeRequested(defs[type], QPointF(0, 0));
  });

  refresh();
}

void NodeLibraryPanel::refresh() {
  m_tree->clear();
  m_itemToType.clear();
  auto defs = m_project->nodeDefinitions();
  QMap<QString, QTreeWidgetItem *> categories;

  for (const auto &def : defs) {
    if (!def.language.isEmpty() && def.language != m_project->language())
      continue;

    QString category = "General";
    QStringList parts = def.type.split('.');
    if (parts.size() >= 3) category = parts[1];

    if (!categories.contains(category)) {
      auto *catItem = new QTreeWidgetItem(m_tree, {category});
      QFont f = catItem->font(0);
      f.setBold(true);
      catItem->setFont(0, f);
      categories[category] = catItem;
    }

    auto *item = new QTreeWidgetItem(categories[category], {def.name});
    m_itemToType[item] = def.type;
  }
  m_tree->expandAll();
}

void NodeLibraryPanel::onFilterChanged(const QString &text) {
  QString filter = text.trimmed().toLower();
  for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
    auto *cat = m_tree->topLevelItem(i);
    bool anyVisible = false;
    for (int j = 0; j < cat->childCount(); ++j) {
      auto *child = cat->child(j);
      bool match = filter.isEmpty() || child->text(0).toLower().contains(filter);
      child->setHidden(!match);
      if (match) anyVisible = true;
    }
    cat->setHidden(!anyVisible && !filter.isEmpty());
  }
}
