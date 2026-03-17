#pragma once
#include "core/NodeLink.h"
#include <QGraphicsPathItem>
#include <QTimer>

class NodeGraphScene;

class NodeLinkGraphicsItem : public QObject, public QGraphicsPathItem {
  Q_OBJECT
public:
  NodeLinkGraphicsItem(NodeLink *link, NodeGraphScene *scene);
  void updatePath();
  QUuid linkId() const { return m_link ? m_link->id : QUuid(); }
  NodeLink *link() const { return m_link; }

private:
  NodeLink *m_link;
  NodeGraphScene *m_scene;
  QTimer m_animTimer;
  qreal m_dashOffset = 0;
  bool m_isExec = false;
};
