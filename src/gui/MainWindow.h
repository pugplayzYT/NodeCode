#pragma once
#include "core/Project.h"
#include <QMainWindow>
#include <QUndoStack>

class NodeGraphScene;
class QGraphicsView;
class QListWidget;
class CodePreviewPanel;
class ConsolePanel;
class NodeLibraryPanel;
class VariablesPanel;
class CompilerBackend;
class PluginManager;
class QTimer;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void onNewProject();
  void onOpenProject();
  void onSaveProject();
  void onSaveProjectAs();
  void onExportCode();
  void onCompileAndRun();
  void onStopExecution();
  void onLoadDefs();
  void onZoomToFit();
  void onAutoLayout();
  void onLanguageChanged(const QString &lang);

private:
  void setupMenuBar();
  void setupToolBar();
  void setupDocks();
  void setupConnections();
  void updateTitle();
  void updateSidebarLanguages();
  void autoSave();
  void openRecentFile(const QString &path);
  void spawnDefaultNodes();

  Project *m_project;
  QGraphicsView *m_view;
  NodeGraphScene *m_scene;
  QUndoStack *m_undoStack;

  // Panels
  CodePreviewPanel *m_codePreview;
  ConsolePanel *m_console;
  NodeLibraryPanel *m_nodeLibrary;
  VariablesPanel *m_variables;
  QListWidget *m_languageList;

  // Backend
  CompilerBackend *m_compiler;
  PluginManager *m_pluginManager;

  // Auto-save
  QTimer *m_autoSaveTimer;
};
