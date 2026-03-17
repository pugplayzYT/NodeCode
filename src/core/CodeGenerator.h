#pragma once
#include <QString>
#include <QList>
#include <QUuid>

class Project;

struct GenerationResult {
  QString code;
  QList<QUuid> errorNodes;
  QStringList errors;
};

class CodeGenerator {
public:
  static QString generate(Project *project);
  static GenerationResult generateWithErrors(Project *project);
  static QString generateSingleNode(class Node *node);
};
