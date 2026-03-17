#pragma once
#include "NodeDef.h"
#include "NodeGraph.h"
#include <QMap>
#include <QString>
#include <QSettings>

class Project {
public:
  Project();
  void setLanguage(const QString &lang) { m_language = lang; }
  QString language() const { return m_language; }

  NodeGraph *graph() { return &m_graph; }

  bool loadNodeTree(const QString &filename);
  bool saveNodeTree(const QString &filename);

  bool loadNodeDefinitions(const QString &filename);
  QMap<QString, NodeDef> nodeDefinitions() const { return m_nodeDefs; }

  // Recent files
  QStringList recentFiles() const;
  void addRecentFile(const QString &path);

  QString currentFilePath() const { return m_currentFile; }
  void setCurrentFilePath(const QString &path) { m_currentFile = path; }

private:
  QString m_language;
  NodeGraph m_graph;
  QMap<QString, NodeDef> m_nodeDefs;
  QString m_currentFile;
};
