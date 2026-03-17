#include "NodeCommentItem.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QPainter>

NodeCommentItem::NodeCommentItem(const QString &title, const QColor &color)
    : m_id(QUuid::createUuid()), m_title(title), m_color(color),
      m_size(300, 200) {
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  setZValue(-10);
}

QRectF NodeCommentItem::boundingRect() const {
  return QRectF(0, 0, m_size.width(), m_size.height());
}

void NodeCommentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
  painter->setBrush(m_color);
  painter->setPen(QPen(m_color.darker(150), isSelected() ? 2.0 : 1.0));
  painter->drawRoundedRect(boundingRect(), 8, 8);

  painter->setPen(Qt::white);
  QFont f = painter->font();
  f.setBold(true);
  f.setPointSize(11);
  painter->setFont(f);
  painter->drawText(QRectF(10, 5, m_size.width() - 20, 30), Qt::AlignLeft | Qt::AlignVCenter, m_title);

  // Resize handle
  painter->setBrush(QColor(255, 255, 255, 60));
  painter->setPen(Qt::NoPen);
  painter->drawRect(QRectF(m_size.width() - 12, m_size.height() - 12, 10, 10));
}

void NodeCommentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  bool ok;
  QString newTitle = QInputDialog::getText(nullptr, "Edit Comment", "Title:",
                                           QLineEdit::Normal, m_title, &ok);
  if (ok && !newTitle.isEmpty()) {
    m_title = newTitle;
    update();
  }
  event->accept();
}

void NodeCommentItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  QPointF local = event->pos();
  if (local.x() > m_size.width() - 15 && local.y() > m_size.height() - 15) {
    m_resizing = true;
    m_resizeStart = event->scenePos();
    event->accept();
    return;
  }
  QGraphicsItem::mousePressEvent(event);
}

void NodeCommentItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_resizing) {
    QPointF delta = event->scenePos() - m_resizeStart;
    m_resizeStart = event->scenePos();
    prepareGeometryChange();
    m_size.setWidth(qMax(100.0, m_size.width() + delta.x()));
    m_size.setHeight(qMax(60.0, m_size.height() + delta.y()));
    update();
    event->accept();
    return;
  }
  QGraphicsItem::mouseMoveEvent(event);
}

void NodeCommentItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  m_resizing = false;
  QGraphicsItem::mouseReleaseEvent(event);
}
