#pragma once
#include <QString>

class Project;

namespace Serialization {
  bool saveJson(Project *project, const QString &filename);
  bool loadJson(Project *project, const QString &filename);
}
