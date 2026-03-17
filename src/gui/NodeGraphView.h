#pragma once
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

class NodeGraphView : public QGraphicsView {
  Q_OBJECT
public:
  NodeGraphView(QGraphicsScene *scene, QWidget *parent = nullptr)
      : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setViewportUpdateMode(SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSceneRect(-1e6, -1e6, 2e6, 2e6);
  }

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::MiddleButton) {
      m_panning = true;
      m_lastPanPos = event->position().toPoint();
      setCursor(Qt::ClosedHandCursor);
      event->accept();
      return;
    }
    QGraphicsView::mousePressEvent(event);
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (m_panning) {
      QPoint delta = event->position().toPoint() - m_lastPanPos;
      m_lastPanPos = event->position().toPoint();
      horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
      verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
      event->accept();
      return;
    }
    QGraphicsView::mouseMoveEvent(event);
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    if (event->button() == Qt::MiddleButton) {
      m_panning = false;
      setCursor(Qt::ArrowCursor);
      event->accept();
      return;
    }
    QGraphicsView::mouseReleaseEvent(event);
  }

  void wheelEvent(QWheelEvent *event) override {
    double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
    event->accept();
  }

private:
  bool m_panning = false;
  QPoint m_lastPanPos;
};
