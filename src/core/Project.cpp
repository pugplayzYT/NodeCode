#include "Project.h"
#include "Serialization.h"
#include <QFile>
#include <QSettings>
#include <fstream>
#include <nlohmann/json.hpp>

Project::Project() : m_language("C++") {}

bool Project::saveNodeTree(const QString &filename) {
  bool ok = Serialization::saveJson(this, filename);
  if (ok) {
    m_currentFile = filename;
    addRecentFile(filename);
  }
  return ok;
}

bool Project::loadNodeTree(const QString &filename) {
  bool ok = Serialization::loadJson(this, filename);
  if (ok) {
    m_currentFile = filename;
    addRecentFile(filename);
  }
  return ok;
}

bool Project::loadNodeDefinitions(const QString &filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;
  try {
    QByteArray data = file.readAll();
    nlohmann::json j = nlohmann::json::parse(data.toStdString());
    for (const auto &obj : j["nodes"]) {
      NodeDef def;
      def.type = QString::fromStdString(obj.value("type", ""));
      def.name = QString::fromStdString(obj.value("name", ""));
      if (obj.contains("inputs")) {
        for (auto &p : obj["inputs"])
          def.inputs.append({QString::fromStdString(p.value("name", "")),
                             QString::fromStdString(p.value("type", "")),
                             QString::fromStdString(p.value("value", "")),
                             p.value("allowInline", true)});
      }
      if (obj.contains("outputs")) {
        for (auto &p : obj["outputs"])
          def.outputs.append({QString::fromStdString(p.value("name", "")),
                              QString::fromStdString(p.value("type", "")),
                              QString::fromStdString(p.value("value", "")),
                              p.value("allowInline", true)});
      }
      def.code = QString::fromStdString(obj.value("code", ""));
      def.hasValue = obj.value("hasValue", false);
      def.value = QString::fromStdString(obj.value("value", ""));
      def.language = QString::fromStdString(obj.value("language", ""));
      if (obj.contains("requires")) {
        for (auto &r : obj["requires"])
        def.
          requires
            .append(QString::fromStdString(r));
      }
      m_nodeDefs[def.type] = def;
    }
  } catch (...) {
    return false;
  }
  return true;
}

QStringList Project::recentFiles() const {
  QSettings settings("NodeCode", "NodeCodeStudio");
  return settings.value("recentFiles").toStringList();
}

void Project::addRecentFile(const QString &path) {
  QSettings settings("NodeCode", "NodeCodeStudio");
  QStringList files = settings.value("recentFiles").toStringList();
  files.removeAll(path);
  files.prepend(path);
  while (files.size() > 10)
    files.removeLast();
  settings.setValue("recentFiles", files);
}
