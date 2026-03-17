#pragma once
#include "NodePortGraphicsItem.h"
#include "core/Node.h"
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QMap>

class NodeGraphScene;

class NodeGraphicsItem : public QGraphicsItem {
public:
  NodeGraphicsItem(Node *node);
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  void rebuildPorts();
  QPointF getPortPosition(const QUuid &portId) const;
  Node *node() const { return m_node; }

  void setErrorHighlight(bool err) { m_hasError = err; update(); }

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
  Node *m_node;
  QMap<QUuid, NodePortGraphicsItem *> m_ports;
  QMap<QUuid, QGraphicsProxyWidget *> m_portProxies;
  QGraphicsProxyWidget *m_proxy = nullptr;
  QLineEdit *m_editor = nullptr;
  bool m_hasError = false;
};
