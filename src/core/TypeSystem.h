#pragma once
#include <QColor>
#include <QString>

namespace TypeSystem {

inline QColor portColor(const QString &dataType) {
  if (dataType == "Exec")    return QColor(220, 220, 220);  // white-ish
  if (dataType == "String")  return QColor(76, 175, 80);    // green
  if (dataType == "Number")  return QColor(33, 150, 243);   // blue
  if (dataType == "Boolean") return QColor(244, 67, 54);    // red
  if (dataType == "Any")     return QColor(158, 158, 158);  // gray
  return QColor(158, 158, 158);
}

inline QColor linkColor(const QString &dataType) {
  if (dataType == "Exec") return QColor(220, 220, 220);
  return portColor(dataType);
}

inline bool areCompatible(const QString &source, const QString &target) {
  if (source == target) return true;
  if (source == "Any" || target == "Any") return true;
  return false;
}

} // namespace TypeSystem
