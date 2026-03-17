#pragma once
#include <QList>
#include <QString>
#include <QStringList>

struct PortDef {
  QString name;
  QString type;
  QString defaultValue;
  bool allowInline = true;
};

struct NodeDef {
  QString type;
  QString name;
  QList<PortDef> inputs;
  QList<PortDef> outputs;
  QString code;
  bool hasValue = false;
  QString value;
  QString language;
  QStringList requires;
};
