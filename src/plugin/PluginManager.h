#pragma once
#include "core/Project.h"
#include <QObject>
#include <QStringList>

struct PluginInfo {
  QString filename;
  QString language;
  int nodeCount = 0;
  bool enabled = true;
};

class PluginManager : public QObject {
  Q_OBJECT
public:
  PluginManager(Project *project, QObject *parent = nullptr);
  void scanAndLoad();
  QList<PluginInfo> plugins() const { return m_plugins; }

private:
  Project *m_project;
  QList<PluginInfo> m_plugins;
  QStringList m_searchPaths;
};
