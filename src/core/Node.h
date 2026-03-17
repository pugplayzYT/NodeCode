#pragma once
#include "NodePort.h"
#include <QList>
#include <QPointF>
#include <QString>
#include <QUuid>

class Node {
public:
  Node(const QString &type, const QString &name);
  QUuid id() const { return m_id; }
  QString type() const { return m_type; }
  QString name() const { return m_name; }
  void setName(const QString &name) { m_name = name; }

  QPointF position() const { return m_pos; }
  void setPosition(const QPointF &pos) { m_pos = pos; }

  void addInput(const QString &name, const QString &dataType,
                const QString &defaultValue = "", bool allowInline = true);
  void addOutput(const QString &name, const QString &dataType,
                 const QString &defaultValue = "", bool allowInline = true);

  QList<NodePort> &inputs() { return m_inputs; }
  QList<NodePort> &outputs() { return m_outputs; }
  const QList<NodePort> &inputs() const { return m_inputs; }
  const QList<NodePort> &outputs() const { return m_outputs; }

  QString codeTemplate() const { return m_codeTemplate; }
  void setCodeTemplate(const QString &tpl) { m_codeTemplate = tpl; }

  bool hasValue() const { return m_hasValue; }
  void setHasValue(bool h) { m_hasValue = h; }
  QString value() const { return m_value; }
  void setValue(const QString &val) { m_value = val; }

  QString language() const { return m_language; }
  void setLanguage(const QString &lang) { m_language = lang; }

  void setPortValue(const QUuid &portId, const QString &val);
  void setPortDisplayName(const QUuid &portId, const QString &displayName);
  void setPortAllowInline(const QUuid &portId, bool allow);

  bool collapsed() const { return m_collapsed; }
  void setCollapsed(bool c) { m_collapsed = c; }

  bool hasBreakpoint() const { return m_breakpoint; }
  void setBreakpoint(bool b) { m_breakpoint = b; }

  // Dead code tracking
  bool isDeadCode() const { return m_isDeadCode; }
  void setDeadCode(bool dead) { m_isDeadCode = dead; }

private:
  QUuid m_id;
  QString m_type;
  QString m_name;
  QPointF m_pos;
  QList<NodePort> m_inputs;
  QList<NodePort> m_outputs;
  QString m_codeTemplate;
  bool m_hasValue = false;
  QString m_value;
  QString m_language;
  bool m_collapsed = false;
  bool m_breakpoint = false;
  bool m_isDeadCode = false;
};