#include "StyleSheet.h"

QString getSleekQSS() {
  return R"(
    QMainWindow, QDialog { background-color: #1e1e24; }
    QGraphicsView { border: none; background-color: #1e1e24; }
    QMenuBar { background-color: #151519; color: #e0e0e0; padding: 4px; font-size: 13px; }
    QMenuBar::item:selected { background-color: #2c539e; border-radius: 4px; }
    QMenu { background-color: #2b2b36; color: #e0e0e0; border: 1px solid #1a1a1d; border-radius: 4px; font-size: 12px; }
    QMenu::item { padding: 4px 18px 4px 14px; }
    QMenu::item:selected { background-color: #3b3b4a; }
    QToolBar { background-color: #151519; border: none; spacing: 8px; padding: 8px; }
    QPushButton { background-color: #2c539e; color: #ffffff; border: none; padding: 10px 20px; border-radius: 6px; font-size: 14px; font-weight: bold; }
    QPushButton:hover { background-color: #3a6bca; }
    QPushButton:pressed { background-color: #1d3a73; }
    QLabel { color: #ffffff; }
    QDockWidget { color: #e0e0e0; titlebar-close-icon: none; font-size: 12px; }
    QDockWidget::title { background-color: #151519; padding: 6px; }
    QTreeWidget { background-color: #1a1a1d; color: #e0e0e0; border: 1px solid #3b3b4a; border-radius: 4px; font-size: 12px; }
    QTreeWidget::item { padding: 3px; }
    QTreeWidget::item:selected { background-color: #2c539e; }
    QTreeWidget::item:hover { background-color: #2a2a35; }
    QTabBar::tab { background-color: #1a1a1d; color: #a0a0a0; padding: 8px 16px; border: 1px solid #3b3b4a; }
    QTabBar::tab:selected { background-color: #2c539e; color: white; }
    QTabBar::tab:hover { background-color: #2a2a35; }
    QLineEdit { background-color: #1a1a1d; color: #e0e0e0; border: 1px solid #3b3b4a; border-radius: 3px; padding: 4px; font-size: 12px; }
    QTextEdit { background-color: #1a1a1d; color: #a0f0a0; font-family: monospace; font-size: 13px; border: 1px solid #3b3b4a; }
    QListWidget { background-color: #1a1a1d; border: 1px solid #3b3b4a; border-radius: 4px; padding: 4px; }
    QListWidget::item { padding: 8px; border-radius: 4px; margin-bottom: 2px; color: #a0a0a0; }
    QListWidget::item:selected { background-color: #2c539e; color: white; }
    QScrollBar:vertical { background-color: #1a1a1d; width: 10px; }
    QScrollBar::handle:vertical { background-color: #3b3b4a; border-radius: 5px; min-height: 20px; }
    QScrollBar::handle:vertical:hover { background-color: #4a4a5a; }
    QScrollBar:horizontal { background-color: #1a1a1d; height: 10px; }
    QScrollBar::handle:horizontal { background-color: #3b3b4a; border-radius: 5px; min-width: 20px; }
  )";
}
