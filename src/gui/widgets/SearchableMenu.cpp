#include "SearchableMenu.h"
#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>

SearchableMenu::SearchableMenu(Project *project, QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint),
      m_project(project) {
  setFixedSize(280, 380);
  setStyleSheet(
      "SearchableMenu { background-color: #2b2b36; border: 1px solid #1a1a1d; border-radius: 6px; }"
      "QLineEdit { background-color: #1a1a1d; color: #e0e0e0; border: 1px solid #3b3b4a; "
      "border-radius: 4px; padding: 6px; font-size: 13px; margin: 4px; }"
      "QTreeWidget { background-color: #2b2b36; color: #e0e0e0; border: none; font-size: 12px; }"
      "QTreeWidget::item { padding: 3px 6px; }"
      "QTreeWidget::item:selected { background-color: #2c539e; }"
      "QTreeWidget::item:hover { background-color: #3b3b4a; }");

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(2);

  m_search = new QLineEdit(this);
  m_search->setPlaceholderText("Search nodes...");
  layout->addWidget(m_search);

  m_tree = new QTreeWidget(this);
  m_tree->setHeaderHidden(true);
  m_tree->setIndentation(16);
  m_tree->setRootIsDecorated(true);
  layout->addWidget(m_tree);

  connect(m_search, &QLineEdit::textChanged, this, &SearchableMenu::onFilterChanged);
  connect(m_tree, &QTreeWidget::itemActivated, this, &SearchableMenu::onItemActivated);
  connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &SearchableMenu::onItemActivated);

  buildTree();
  m_search->setFocus();
}

void SearchableMenu::buildTree() {
  m_tree->clear();
  m_itemToType.clear();
  auto nodeDefs = m_project->nodeDefinitions();
  QMap<QString, QTreeWidgetItem *> categories;

  for (const auto &def : nodeDefs) {
    if (!def.language.isEmpty() && def.language != m_project->language())
      continue;

    QString category = "General";
    QStringList parts = def.type.split('.');
    if (parts.size() >= 3) category = parts[1];

    if (!categories.contains(category)) {
      auto *catItem = new QTreeWidgetItem(m_tree, {category});
      catItem->setFlags(catItem->flags() & ~Qt::ItemIsSelectable);
      QFont f = catItem->font(0);
      f.setBold(true);
      catItem->setFont(0, f);
      categories[category] = catItem;
    }

    auto *item = new QTreeWidgetItem(categories[category], {def.name});
    m_itemToType[item] = def.type;
  }

  // Add utility items
  auto *sep = new QTreeWidgetItem(m_tree, {"---"});
  sep->setFlags(sep->flags() & ~Qt::ItemIsSelectable);
  auto *deleteItem = new QTreeWidgetItem(m_tree, {"Delete Selected"});
  m_itemToType[deleteItem] = "__delete__";
  auto *commentItem = new QTreeWidgetItem(m_tree, {"Add Comment Box"});
  m_itemToType[commentItem] = "__comment__";

  m_tree->expandAll();
}

void SearchableMenu::onFilterChanged(const QString &text) {
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
    // Also match category name
    if (!filter.isEmpty() && cat->text(0).toLower().contains(filter)) {
      cat->setHidden(false);
      for (int j = 0; j < cat->childCount(); ++j)
        cat->child(j)->setHidden(false);
    }
  }
}

void SearchableMenu::onItemActivated(QTreeWidgetItem *item, int) {
  if (!m_itemToType.contains(item)) return;
  QString type = m_itemToType[item];

  if (type == "__delete__") {
    close();
    return;
  }
  if (type == "__comment__") {
    close();
    return;
  }

  auto defs = m_project->nodeDefinitions();
  if (defs.contains(type)) {
    emit nodeSelected(defs[type]);
  }
  close();
}
