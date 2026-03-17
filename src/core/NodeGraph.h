#pragma once
#include "Node.h"
#include "NodeLink.h"
#include <QMap>
#include <QObject>

class NodeGraph : public QObject {
  Q_OBJECT
public:
  NodeGraph(QObject *parent = nullptr);
  Node *addNode(const QString &type, const QString &name, const QPointF &pos);
  void removeNode(const QUuid &id, bool emitGraphChanged = true);
  Node *getNode(const QUuid &id) const;
  const QMap<QUuid, Node *> &nodes() const { return m_nodes; }

  NodeLink *addLink(const QUuid &srcNode, const QUuid &srcPort,
                    const QUuid &tgtNode, const QUuid &tgtPort);
  void removeLink(const QUuid &id, bool emitGraphChanged = true);
  const QMap<QUuid, NodeLink *> &links() const { return m_links; }

  void clear();

signals:
  void nodeAdded(Node *node);
  void nodeRemoved(const QUuid &id);
  void linkAdded(NodeLink *link);
  void linkRemoved(const QUuid &id);
  void graphChanged();

private:
  QMap<QUuid, Node *> m_nodes;
  QMap<QUuid, NodeLink *> m_links;
};
