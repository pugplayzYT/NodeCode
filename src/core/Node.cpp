#include "Node.h"

Node::Node(const QString &type, const QString &name)
    : m_id(QUuid::createUuid()), m_type(type), m_name(name) {}

void Node::addInput(const QString &name, const QString &dataType,
                    const QString &defaultValue, bool allowInline) {
  m_inputs.append({QUuid::createUuid(), PortType::Input, name, dataType,
                   defaultValue, allowInline});
}
void Node::addOutput(const QString &name, const QString &dataType,
                     const QString &defaultValue, bool allowInline) {
  m_outputs.append({QUuid::createUuid(), PortType::Output, name, dataType,
                    defaultValue, allowInline});
}

void Node::setPortValue(const QUuid &portId, const QString &val) {
  for (auto &p : m_inputs) {
    if (p.id == portId) { p.value = val; return; }
  }
  for (auto &p : m_outputs) {
    if (p.id == portId) { p.value = val; return; }
  }
}

void Node::setPortDisplayName(const QUuid &portId, const QString &displayName) {
  for (auto &p : m_inputs) {
    if (p.id == portId) { p.displayName = displayName; return; }
  }
  for (auto &p : m_outputs) {
    if (p.id == portId) { p.displayName = displayName; return; }
  }
}

void Node::setPortAllowInline(const QUuid &portId, bool allow) {
  for (auto &p : m_inputs) {
    if (p.id == portId) { p.allowInline = allow; return; }
  }
  for (auto &p : m_outputs) {
    if (p.id == portId) { p.allowInline = allow; return; }
  }
}
