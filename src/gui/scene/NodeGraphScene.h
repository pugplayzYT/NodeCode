#pragma once
#include "core/Project.h"
#include "gui/items/NodeCommentItem.h"
#include "gui/items/NodeGraphicsItem.h"
#include "gui/items/NodeLinkGraphicsItem.h"
#include <QGraphicsScene>
#include <QUndoStack>

enum class ToolMode { Select, Knife };

class NodeGraphScene : public QGraphicsScene {
  Q_OBJECT
public:
  NodeGraphScene(Project *project, QUndoStack *undoStack,
                 QObject *parent = nullptr);
  NodeGraphicsItem *getNodeItem(const QUuid &id) const {
    return m_nodeItems.value(id, nullptr);
  }
  Project *getProject() const { return m_project; }
  QUndoStack *undoStack() const { return m_undoStack; }

  void createTempLink(NodePortGraphicsItem *port);
  void updateTempLink(const QPointF &endPos);
  void finishTempLink(NodePortGraphicsItem *destPort);

  void setToolMode(ToolMode mode) { m_toolMode = mode; }
  ToolMode toolMode() const { return m_toolMode; }

  void addComment(const QPointF &pos);
  void zoomToFit(class QGraphicsView *view);
  void autoLayout();
  void copySelected();
  void paste(const QPointF &pos);

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

public slots:
  void onNodeAdded(Node *node);
  void onNodeRemoved(const QUuid &id);
  void onLinkAdded(NodeLink *link);
  void onLinkRemoved(const QUuid &id);
  void updateLinks();
  void highlightErrors(const QList<QUuid> &errorNodes);

private:
  Project *m_project;
  QUndoStack *m_undoStack;
  QMap<QUuid, NodeGraphicsItem *> m_nodeItems;
  QMap<QUuid, NodeLinkGraphicsItem *> m_linkItems;
  QList<NodeCommentItem *> m_comments;

  QGraphicsPathItem *m_tempLink = nullptr;
  QPointF m_tempLinkStart;
  NodePortGraphicsItem *m_tempLinkStartPort = nullptr;

  ToolMode m_toolMode = ToolMode::Select;
  QGraphicsPathItem *m_slashItem = nullptr;
  QPointF m_slashStart;

  QString m_clipboard;
};