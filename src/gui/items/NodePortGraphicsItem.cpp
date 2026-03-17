#include "NodePortGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include "gui/scene/NodeGraphScene.h"
#include "core/TypeSystem.h"
#include "core/Node.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>

NodePortGraphicsItem::NodePortGraphicsItem(QGraphicsItem *parent,
                                           const NodePort &port, QUuid id,
                                           QUuid nodeId)
    : QGraphicsEllipseItem(parent), m_portId(id), m_nodeId(nodeId),
      m_type(port.type), m_dataType(port.dataType) {
  setRect(-6, -6, 12, 12);
  setBrush(QBrush(TypeSystem::portColor(port.dataType)));
  setPen(QPen(Qt::white, 1.5));
  setAcceptHoverEvents(true);
  setToolTip(port.dataType + ": " + port.label());
}

void NodePortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (m_type == PortType::Output) {
    if (auto *scene = dynamic_cast<NodeGraphScene *>(this->scene())) {
      scene->createTempLink(this);
      event->accept();
      return;
    }
  }
  QGraphicsEllipseItem::mousePressEvent(event);
}

void NodePortGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_type == PortType::Output) {
    if (auto *scene = dynamic_cast<NodeGraphScene *>(this->scene())) {
      scene->updateTempLink(event->scenePos());
      event->accept();
      return;
    }
  }
  QGraphicsEllipseItem::mouseMoveEvent(event);
}

void NodePortGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  if (m_type == PortType::Output) {
    if (auto *scene = dynamic_cast<NodeGraphScene *>(this->scene())) {
      QList<QGraphicsItem *> itemsAtDrop = scene->items(event->scenePos());
      NodePortGraphicsItem *destPort = nullptr;
      for (auto *item : itemsAtDrop) {
        destPort = dynamic_cast<NodePortGraphicsItem *>(item);
        if (destPort && destPort->portType() == PortType::Input)
          break;
      }
      scene->finishTempLink(destPort);
      event->accept();
      return;
    }
  }
  QGraphicsEllipseItem::mouseReleaseEvent(event);
}

void NodePortGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  auto *nscene = dynamic_cast<NodeGraphScene *>(scene());
  if (!nscene) return;
  auto *node = nscene->getProject()->graph()->getNode(m_nodeId);
  if (!node) return;

  QString currentLabel;
  bool isInput = false;
  bool currentAllowInline = false;
  for (const auto &p : node->inputs()) {
    if (p.id == m_portId) {
      currentLabel = p.label();
      isInput = true;
      currentAllowInline = p.allowInline;
      break;
    }
  }
  if (!isInput) {
    for (const auto &p : node->outputs()) {
      if (p.id == m_portId) { currentLabel = p.label(); break; }
    }
  }

  QMenu menu;
  auto *renameAction = menu.addAction("Rename Pin");
  QAction *toggleInlineAction = nullptr;
  if (isInput) {
    toggleInlineAction = menu.addAction(
        currentAllowInline ? "Disable Value Editor" : "Enable Value Editor");
  }

  auto *selected = menu.exec(event->screenPos());
  if (!selected) return;

  if (selected == renameAction) {
    bool ok;
    QString newName = QInputDialog::getText(nullptr, "Rename Pin", "Pin name:",
                                            QLineEdit::Normal, currentLabel, &ok);
    if (ok && !newName.isEmpty()) {
      node->setPortDisplayName(m_portId, newName);
      if (auto *nodeItem = dynamic_cast<NodeGraphicsItem *>(parentItem()))
        nodeItem->rebuildPorts();
    }
  } else if (selected == toggleInlineAction && isInput) {
    node->setPortAllowInline(m_portId, !currentAllowInline);
    if (auto *nodeItem = dynamic_cast<NodeGraphicsItem *>(parentItem()))
      nodeItem->rebuildPorts();
  }
  event->accept();
}
