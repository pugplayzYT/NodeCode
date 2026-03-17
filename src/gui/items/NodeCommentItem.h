#pragma once
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QUuid>

class NodeCommentItem : public QGraphicsItem {
public:
  NodeCommentItem(const QString &title = "Comment", const QColor &color = QColor(44, 83, 158, 60));
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

  QUuid id() const { return m_id; }
  QString title() const { return m_title; }
  void setTitle(const QString &t) { m_title = t; update(); }
  QColor color() const { return m_color; }
  void setColor(const QColor &c) { m_color = c; update(); }
  QSizeF size() const { return m_size; }
  void setSize(const QSizeF &s) { m_size = s; prepareGeometryChange(); update(); }

protected:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
  QUuid m_id;
  QString m_title;
  QColor m_color;
  QSizeF m_size;
  bool m_resizing = false;
  QPointF m_resizeStart;
};
