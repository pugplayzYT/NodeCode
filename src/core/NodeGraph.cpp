#include "NodeGraph.h"

NodeGraph::NodeGraph(QObject *parent) : QObject(parent) {}

Node *NodeGraph::addNode(const QString &type, const QString &name,
                         const QPointF &pos) {
  Node *node = new Node(type, name);
  node->setPosition(pos);
  m_nodes.insert(node->id(), node);
  emit nodeAdded(node);
  emit graphChanged();
  return node;
}

void NodeGraph::removeNode(const QUuid &id, bool emitGraphChanged) {
  if (m_nodes.contains(id)) {
    QList<QUuid> linksToRemove;
    for (auto *link : m_links) {
      if (link->sourceNodeId == id || link->targetNodeId == id)
        linksToRemove.append(link->id);
    }
    for (const auto &lid : linksToRemove)
      removeLink(lid, emitGraphChanged);
    Node *node = m_nodes.take(id);
    emit nodeRemoved(id);
    if (emitGraphChanged)
      emit graphChanged();
    delete node;
  }
}

Node *NodeGraph::getNode(const QUuid &id) const {
  return m_nodes.value(id, nullptr);
}

NodeLink *NodeGraph::addLink(const QUuid &srcNode, const QUuid &srcPort,
                             const QUuid &tgtNode, const QUuid &tgtPort) {
  NodeLink *link =
      new NodeLink{QUuid::createUuid(), srcNode, srcPort, tgtNode, tgtPort};
  m_links.insert(link->id, link);
  emit linkAdded(link);
  emit graphChanged();
  return link;
}

void NodeGraph::removeLink(const QUuid &id, bool emitGraphChanged) {
  if (m_links.contains(id)) {
    NodeLink *link = m_links.take(id);
    emit linkRemoved(id);
    if (emitGraphChanged)
      emit graphChanged();
    delete link;
  }
}

void NodeGraph::clear() {
  auto nodeIds = m_nodes.keys();
  for (const auto &id : nodeIds)
    removeNode(id, false);
  emit graphChanged();
}
