#include "MainWindow.h"
#include "gui/StyleSheet.h"
#include "gui/NodeGraphView.h"
#include "gui/scene/NodeGraphScene.h"
#include "gui/panels/CodePreviewPanel.h"
#include "gui/panels/ConsolePanel.h"
#include "gui/panels/NodeLibraryPanel.h"
#include "gui/panels/VariablesPanel.h"
#include "compiler/CompilerBackend.h"
#include "plugin/PluginManager.h"
#include "core/CodeGenerator.h"
#include "core/Serialization.h"
#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsView>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("NodeCode Studio");
  setStyleSheet(getSleekQSS());
  resize(1400, 850);

  m_project = new Project();
  m_undoStack = new QUndoStack(this);
  m_compiler = new LocalCompiler(this);

  // Scene + View
  m_scene = new NodeGraphScene(m_project, m_undoStack, this);
  m_view = new NodeGraphView(m_scene, this);
  setCentralWidget(m_view);

  // Load plugins/node definitions
  m_pluginManager = new PluginManager(m_project, this);
  m_pluginManager->scanAndLoad();

  setupMenuBar();
  setupToolBar();
  setupDocks();
  setupConnections();

  // Spawn default Start + End nodes
  spawnDefaultNodes();

  // Auto-save every 60 seconds
  m_autoSaveTimer = new QTimer(this);
  m_autoSaveTimer->setInterval(60000);
  connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
  m_autoSaveTimer->start();

  updateTitle();
}

MainWindow::~MainWindow() { delete m_project; }

void MainWindow::setupMenuBar() {
  auto *fileMenu = menuBar()->addMenu("&File");

  auto *actNew = fileMenu->addAction("New Project", this,
                                     &MainWindow::onNewProject);
  actNew->setShortcut(QKeySequence::New);

  auto *actOpen =
      fileMenu->addAction("Open...", this, &MainWindow::onOpenProject);
  actOpen->setShortcut(QKeySequence::Open);

  auto *actSave =
      fileMenu->addAction("Save", this, &MainWindow::onSaveProject);
  actSave->setShortcut(QKeySequence::Save);

  fileMenu->addAction("Save As...", this, &MainWindow::onSaveProjectAs);

  fileMenu->addSeparator();

  // Recent files
  auto *recentMenu = fileMenu->addMenu("Recent Files");
  for (const auto &path : m_project->recentFiles()) {
    recentMenu->addAction(path, [this, path]() { openRecentFile(path); });
  }

  fileMenu->addSeparator();
  fileMenu->addAction("Load Node Definitions...", this,
                      &MainWindow::onLoadDefs);

  fileMenu->addSeparator();
  fileMenu->addAction("Exit", qApp, &QApplication::quit);

  auto *editMenu = menuBar()->addMenu("&Edit");
  auto *actUndo = editMenu->addAction("Undo", m_undoStack, &QUndoStack::undo);
  actUndo->setShortcut(QKeySequence::Undo);
  auto *actRedo = editMenu->addAction("Redo", m_undoStack, &QUndoStack::redo);
  actRedo->setShortcut(QKeySequence::Redo);

  editMenu->addSeparator();
  editMenu->addAction("Copy", m_scene, &NodeGraphScene::copySelected);
  editMenu->addAction("Paste", [this]() { m_scene->paste(QPointF(0, 0)); });

  auto *viewMenu = menuBar()->addMenu("&View");
  viewMenu->addAction("Zoom to Fit", this, &MainWindow::onZoomToFit);
  viewMenu->addAction("Auto Layout", this, &MainWindow::onAutoLayout);

  auto *buildMenu = menuBar()->addMenu("&Build");
  buildMenu->addAction("Export Code...", this, &MainWindow::onExportCode);

  auto *actRun =
      buildMenu->addAction("Compile && Run", this, &MainWindow::onCompileAndRun);
  actRun->setShortcut(QKeySequence(Qt::Key_F5));

  auto *actStop =
      buildMenu->addAction("Stop", this, &MainWindow::onStopExecution);
  actStop->setShortcut(QKeySequence(Qt::Key_F6));

  auto *toolMenu = menuBar()->addMenu("&Tools");
  toolMenu->addAction("Select Mode", [this]() {
    m_scene->setToolMode(ToolMode::Select);
  });
  toolMenu->addAction("Knife Mode", [this]() {
    m_scene->setToolMode(ToolMode::Knife);
  });
}

void MainWindow::setupToolBar() {
  auto *toolbar = addToolBar("Main");
  toolbar->setMovable(false);
  toolbar->setIconSize(QSize(20, 20));

  auto *btnSelect = new QPushButton("Select", this);
  btnSelect->setCheckable(true);
  btnSelect->setChecked(true);
  toolbar->addWidget(btnSelect);

  auto *btnKnife = new QPushButton("Knife", this);
  btnKnife->setCheckable(true);
  toolbar->addWidget(btnKnife);

  connect(btnSelect, &QPushButton::clicked, [this, btnSelect, btnKnife]() {
    m_scene->setToolMode(ToolMode::Select);
    btnSelect->setChecked(true);
    btnKnife->setChecked(false);
  });
  connect(btnKnife, &QPushButton::clicked, [this, btnSelect, btnKnife]() {
    m_scene->setToolMode(ToolMode::Knife);
    btnKnife->setChecked(true);
    btnSelect->setChecked(false);
  });

  toolbar->addSeparator();

  auto *btnRun = new QPushButton("Run", this);
  btnRun->setStyleSheet(
      "QPushButton { background-color: #2e7d32; } QPushButton:hover { "
      "background-color: #43a047; }");
  connect(btnRun, &QPushButton::clicked, this, &MainWindow::onCompileAndRun);
  toolbar->addWidget(btnRun);

  auto *btnStop = new QPushButton("Stop", this);
  btnStop->setStyleSheet(
      "QPushButton { background-color: #c62828; } QPushButton:hover { "
      "background-color: #e53935; }");
  connect(btnStop, &QPushButton::clicked, this, &MainWindow::onStopExecution);
  toolbar->addWidget(btnStop);
}

void MainWindow::setupDocks() {
  // Left: language selector + node library
  auto *leftDock = new QDockWidget("Language", this);
  m_languageList = new QListWidget(leftDock);
  m_languageList->addItems({"C++", "Python", "JavaScript"});
  m_languageList->setCurrentRow(0);
  leftDock->setWidget(m_languageList);
  addDockWidget(Qt::LeftDockWidgetArea, leftDock);

  m_nodeLibrary = new NodeLibraryPanel(m_project, this);
  addDockWidget(Qt::LeftDockWidgetArea, m_nodeLibrary);

  m_variables = new VariablesPanel(m_project, this);
  addDockWidget(Qt::LeftDockWidgetArea, m_variables);

  // Right: code preview
  m_codePreview = new CodePreviewPanel(m_project, this);
  addDockWidget(Qt::RightDockWidgetArea, m_codePreview);

  // Bottom: console
  m_console = new ConsolePanel(this);
  addDockWidget(Qt::BottomDockWidgetArea, m_console);
}

void MainWindow::setupConnections() {
  // Language selection
  connect(m_languageList, &QListWidget::currentTextChanged, this,
          &MainWindow::onLanguageChanged);

  // Node library → add node to scene
  connect(m_nodeLibrary, &NodeLibraryPanel::nodeRequested, this,
          [this](const NodeDef &def, const QPointF &pos) {
            QPointF scenePos = pos.isNull()
                                   ? m_view->mapToScene(m_view->viewport()->rect().center())
                                   : pos;
            Node *node = m_project->graph()->addNode(def.type, def.name, scenePos);
            node->setLanguage(def.language);
            for (const auto &in : def.inputs)
              node->addInput(in.name, in.type, in.defaultValue, in.allowInline);
            for (const auto &out : def.outputs)
              node->addOutput(out.name, out.type, out.defaultValue, out.allowInline);
            node->setHasValue(def.hasValue);
            node->setValue(def.value);
            node->setCodeTemplate(def.code);
            if (auto *item = m_scene->getNodeItem(node->id()))
              item->rebuildPorts();
          });

  // Variables → navigate to node
  connect(m_variables, &VariablesPanel::variableClicked, this,
          [this](const QUuid &nodeId) {
            if (auto *item = m_scene->getNodeItem(nodeId)) {
              m_view->centerOn(item);
              m_scene->clearSelection();
              item->setSelected(true);
            }
          });

  // Compiler output
  connect(m_compiler, &CompilerBackend::outputReady, m_console,
          &ConsolePanel::appendOutput);
  connect(m_compiler, &CompilerBackend::errorReady, m_console,
          &ConsolePanel::appendError);
  connect(m_compiler, &CompilerBackend::finished, this,
          [this](int exitCode) {
            m_console->appendOutput(
                exitCode == 0 ? "\n--- Finished successfully ---"
                              : "\n--- Exited with code " +
                                    QString::number(exitCode) + " ---");
          });

  // Graph changed → update links
  connect(m_project->graph(), &NodeGraph::graphChanged, m_scene,
          &NodeGraphScene::updateLinks);
}

void MainWindow::onNewProject() {
  m_project->graph()->clear();
  m_project->setCurrentFilePath("");
  spawnDefaultNodes();
  updateTitle();
}

void MainWindow::spawnDefaultNodes() {
  auto defs = m_project->nodeDefinitions();
  QString lang = m_project->language();

  // Find the Start and End defs for current language
  QString startType = lang + ".Start";
  QString endType = lang + ".End";

  auto spawnDef = [&](const QString &type, const QPointF &pos) {
    if (!defs.contains(type)) return;
    const auto &def = defs[type];
    Node *node = m_project->graph()->addNode(def.type, def.name, pos);
    node->setLanguage(def.language);
    for (const auto &in : def.inputs)
      node->addInput(in.name, in.type, in.defaultValue, in.allowInline);
    for (const auto &out : def.outputs)
      node->addOutput(out.name, out.type, out.defaultValue, out.allowInline);
    node->setHasValue(def.hasValue);
    node->setValue(def.value);
    node->setCodeTemplate(def.code);
    if (auto *item = m_scene->getNodeItem(node->id()))
      item->rebuildPorts();
  };

  spawnDef(startType, QPointF(-150, 0));
  spawnDef(endType, QPointF(150, 0));
}

void MainWindow::onOpenProject() {
  QString filename = QFileDialog::getOpenFileName(
      this, "Open Node Tree", "", "NodeCode Trees (*.nodetree);;All Files (*)");
  if (filename.isEmpty())
    return;

  m_project->graph()->clear();
  if (Serialization::loadJson(m_project, filename)) {
    m_project->setCurrentFilePath(filename);
    m_project->addRecentFile(filename);
    updateTitle();
  } else {
    QMessageBox::warning(this, "Error", "Failed to load project.");
  }
}

void MainWindow::onSaveProject() {
  if (m_project->currentFilePath().isEmpty()) {
    onSaveProjectAs();
    return;
  }
  Serialization::saveJson(m_project, m_project->currentFilePath());
  updateTitle();
}

void MainWindow::onSaveProjectAs() {
  QString filename = QFileDialog::getSaveFileName(
      this, "Save Node Tree", "", "NodeCode Trees (*.nodetree)");
  if (filename.isEmpty())
    return;
  if (!filename.endsWith(".nodetree"))
    filename += ".nodetree";
  if (Serialization::saveJson(m_project, filename)) {
    m_project->setCurrentFilePath(filename);
    m_project->addRecentFile(filename);
    updateTitle();
  }
}

void MainWindow::onExportCode() {
  QString code = CodeGenerator::generate(m_project);
  QString ext = "cpp";
  if (m_project->language() == "Python")
    ext = "py";
  else if (m_project->language() == "JavaScript")
    ext = "js";

  QString filename = QFileDialog::getSaveFileName(
      this, "Export Code", "",
      "Source Files (*." + ext + ");;All Files (*)");
  if (filename.isEmpty())
    return;

  QFile f(filename);
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(code.toUtf8());
  }
}

void MainWindow::onCompileAndRun() {
  m_console->clear();
  m_console->appendOutput("--- Compiling and running (" +
                          m_project->language() + ") ---\n");
  QString code = CodeGenerator::generate(m_project);
  m_compiler->run(code, m_project->language());
}

void MainWindow::onStopExecution() { m_compiler->stop(); }

void MainWindow::onLoadDefs() {
  QString filename = QFileDialog::getOpenFileName(
      this, "Load Node Definitions", "",
      "Node Files (*.node *.json);;All Files (*)");
  if (filename.isEmpty())
    return;
  if (m_project->loadNodeDefinitions(filename)) {
    m_nodeLibrary->refresh();
  } else {
    QMessageBox::warning(this, "Error",
                         "Failed to load node definitions.");
  }
}

void MainWindow::onZoomToFit() { m_scene->zoomToFit(m_view); }

void MainWindow::onAutoLayout() { m_scene->autoLayout(); }

void MainWindow::onLanguageChanged(const QString &lang) {
  m_project->setLanguage(lang);
  m_nodeLibrary->refresh();
}

void MainWindow::updateTitle() {
  QString title = "NodeCode Studio";
  if (!m_project->currentFilePath().isEmpty()) {
    QFileInfo fi(m_project->currentFilePath());
    title += " - " + fi.fileName();
  }
  setWindowTitle(title);
}

void MainWindow::updateSidebarLanguages() {
  for (int i = 0; i < m_languageList->count(); ++i) {
    if (m_languageList->item(i)->text() == m_project->language()) {
      m_languageList->setCurrentRow(i);
      break;
    }
  }
}

void MainWindow::autoSave() {
  if (!m_project->currentFilePath().isEmpty()) {
    QString backupPath = m_project->currentFilePath() + ".autosave";
    Serialization::saveJson(m_project, backupPath);
  }
}

void MainWindow::openRecentFile(const QString &path) {
  m_project->graph()->clear();
  if (Serialization::loadJson(m_project, path)) {
    m_project->setCurrentFilePath(path);
    updateTitle();
  }
}
