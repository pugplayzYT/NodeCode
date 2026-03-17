#pragma once
#include <QString>
#include <QUuid>

enum class PortType { Input, Output };

struct NodePort {
  QUuid id;
  PortType type;
  QString name;
  QString dataType;
  QString value;
  bool allowInline = true;
  QString displayName;
  QString label() const { return displayName.isEmpty() ? name : displayName; }
};
