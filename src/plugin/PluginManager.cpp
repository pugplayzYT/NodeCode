#include "PluginManager.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <fstream>
#include <nlohmann/json.hpp>

PluginManager::PluginManager(Project *project, QObject *parent)
    : QObject(parent), m_project(project) {
  m_searchPaths << QApplication::applicationDirPath() + "/../nodes"
                << QApplication::applicationDirPath() + "/../plugins"
                << "nodes"
                << "plugins"
                << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins";
}

void PluginManager::scanAndLoad() {
  m_plugins.clear();
  for (const auto &searchPath : m_searchPaths) {
    QDir dir(searchPath);
    if (!dir.exists()) continue;
    for (const QFileInfo &info : dir.entryInfoList({"*.node", "*.json"}, QDir::Files)) {
      if (m_project->loadNodeDefinitions(info.absoluteFilePath())) {
        // Count nodes in file
        PluginInfo pi;
        pi.filename = info.absoluteFilePath();
        try {
          std::ifstream f(info.absoluteFilePath().toStdString());
          nlohmann::json j;
          f >> j;
          pi.nodeCount = j["nodes"].size();
          if (!j["nodes"].empty())
            pi.language = QString::fromStdString(j["nodes"][0].value("language", ""));
        } catch (...) {}
        m_plugins.append(pi);
      }
    }
  }
}
