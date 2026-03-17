#pragma once
#include "core/NodePort.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>

class NodeGraphScene;
class NodeGraphicsItem;

class NodePortGraphicsItem : public QGraphicsEllipseItem {
public:
  NodePortGraphicsItem(QGraphicsItem *parent, const NodePort &port, QUuid id,
                       QUuid nodeId);
  QUuid portId() const { return m_portId; }
  QUuid nodeId() const { return m_nodeId; }
  PortType portType() const { return m_type; }
  QString dataType() const { return m_dataType; }

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
  QUuid m_portId;
  QUuid m_nodeId;
  PortType m_type;
  QString m_dataType;
};
