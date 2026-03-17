#include "NodeGraphScene.h"
#include "core/CodeGenerator.h"
#include "core/TypeSystem.h"
#include <QApplication>
#include <QClipboard>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QQueue>
#include <QSet>
#include <fstream>
#include <nlohmann/json.hpp>

NodeGraphScene::NodeGraphScene(Project *project, QUndoStack *undoStack,
                               QObject *parent)
    : QGraphicsScene(parent), m_project(project), m_undoStack(undoStack) {
  connect(project->graph(), &NodeGraph::nodeAdded, this,
          &NodeGraphScene::onNodeAdded);
  connect(project->graph(), &NodeGraph::nodeRemoved, this,
          &NodeGraphScene::onNodeRemoved);
  connect(project->graph(), &NodeGraph::linkAdded, this,
          &NodeGraphScene::onLinkAdded);
  connect(project->graph(), &NodeGraph::linkRemoved, this,
          &NodeGraphScene::onLinkRemoved);
}

void NodeGraphScene::drawBackground(QPainter *painter, const QRectF &rect) {
  QColor c1(30, 30, 36);
  QColor c2(38, 38, 45);
  painter->fillRect(rect, c1);

  qreal gridSize = 30;
  QPen pen(c2, 1.0);
  painter->setPen(pen);

  qreal left = int(rect.left()) - (int(rect.left()) % int(gridSize));
  qreal top = int(rect.top()) - (int(rect.top()) % int(gridSize));

  QVector<QLineF> lines;
  for (qreal x = left; x < rect.right(); x += gridSize)
    lines.append(QLineF(x, rect.top(), x, rect.bottom()));
  for (qreal y = top; y < rect.bottom(); y += gridSize)
    lines.append(QLineF(rect.left(), y, rect.right(), y));
  painter->drawLines(lines.data(), lines.size());

  QPen majorPen(QColor(45, 45, 52), 1.5);
  painter->setPen(majorPen);
  QVector<QLineF> majorLines;
  qreal majorSize = gridSize * 5;
  qreal majorLeft = int(rect.left()) - (int(rect.left()) % int(majorSize));
  qreal majorTop = int(rect.top()) - (int(rect.top()) % int(majorSize));
  for (qreal x = majorLeft; x < rect.right(); x += majorSize)
    majorLines.append(QLineF(x, rect.top(), x, rect.bottom()));
  for (qreal y = majorTop; y < rect.bottom(); y += majorSize)
    majorLines.append(QLineF(rect.left(), y, rect.right(), y));
  painter->drawLines(majorLines.data(), majorLines.size());
}

void NodeGraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (m_toolMode == ToolMode::Knife && event->button() == Qt::LeftButton) {
    m_slashStart = event->scenePos();
    m_slashItem = new QGraphicsPathItem();
    m_slashItem->setPen(QPen(QColor(255, 50, 50), 3, Qt::DashLine));
    m_slashItem->setZValue(100);
    addItem(m_slashItem);
    event->accept();
    return;
  }
  QGraphicsScene::mousePressEvent(event);
}

void NodeGraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_toolMode == ToolMode::Knife && m_slashItem) {
    QPainterPath path(m_slashStart);
    path.lineTo(event->scenePos());
    m_slashItem->setPath(path);
    QSet<QUuid> linksToRemove;
    for (auto *item : items(path)) {
      if (auto *linkItem = dynamic_cast<NodeLinkGraphicsItem *>(item))
        linksToRemove.insert(linkItem->linkId());
    }
    for (const QUuid &id : linksToRemove)
      m_project->graph()->removeLink(id);
    event->accept();
    return;
  }
  QGraphicsScene::mouseMoveEvent(event);
}

void NodeGraphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  if (m_toolMode == ToolMode::Knife && m_slashItem) {
    removeItem(m_slashItem);
    delete m_slashItem;
    m_slashItem = nullptr;
    event->accept();
    return;
  }
  QGraphicsScene::mouseReleaseEvent(event);
}

void NodeGraphScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  QMenu menu;
  auto nodeDefs = m_project->nodeDefinitions();
  QMap<QString, QMenu *> categoryMenus;

  for (const auto &def : nodeDefs) {
    if (!def.language.isEmpty() && def.language != m_project->language())
      continue;

    QString category = "General";
    QStringList parts = def.type.split('.');
    if (parts.size() >= 3) {
      category = parts[1];
      if (category == "Function")
        category = "Functions";
      else if (category == "Variable")
        category = "Variables";
      else if (category == "Thread")
        category = "Threading";
      else if (category == "Mutex")
        category = "Threading";
      else if (category == "Cast")
        category = "Casting";
      else if (category == "Fetch")
        category = "Network";
    }

    if (!categoryMenus.contains(category)) {
      categoryMenus[category] = menu.addMenu(category);
    }

    auto *action = categoryMenus[category]->addAction(def.name);
    connect(action, &QAction::triggered, [this, event, def]() {
      Node *node =
          m_project->graph()->addNode(def.type, def.name, event->scenePos());
      node->setLanguage(def.language);
      for (const auto &in : def.inputs)
        node->addInput(in.name, in.type, in.defaultValue, in.allowInline);
      for (const auto &out : def.outputs)
        node->addOutput(out.name, out.type, out.defaultValue, out.allowInline);
      node->setHasValue(def.hasValue);
      node->setValue(def.value);
      node->setCodeTemplate(def.code);
      if (auto *item = m_nodeItems.value(node->id()))
        item->rebuildPorts();
    });
  }

  menu.addSeparator();

  auto *actComment = menu.addAction("Add Comment");
  connect(actComment, &QAction::triggered,
          [this, event]() { addComment(event->scenePos()); });

  menu.addSeparator();

  auto *actDelete = menu.addAction("Delete Selected");
  connect(actDelete, &QAction::triggered, [this]() {
    for (auto *item : selectedItems()) {
      if (auto *p = dynamic_cast<NodeGraphicsItem *>(item))
        m_project->graph()->removeNode(p->node()->id());
      else if (auto *c = dynamic_cast<NodeCommentItem *>(item)) {
        m_comments.removeAll(c);
        removeItem(c);
        delete c;
      }
    }
  });

  menu.exec(event->screenPos());
}

void NodeGraphScene::keyPressEvent(QKeyEvent *event) {
  // Guard check: If a proxy widget (like QLineEdit) has focus, let it handle
  // the event natively. Otherwise, the scene eats the backspace and starves the
  // text box.
  if (focusItem() && focusItem()->isWidget()) {
    QGraphicsScene::keyPressEvent(event);
    return;
  }

  if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
    for (auto *item : selectedItems()) {
      if (auto *p = dynamic_cast<NodeGraphicsItem *>(item))
        m_project->graph()->removeNode(p->node()->id());
      else if (auto *c = dynamic_cast<NodeCommentItem *>(item)) {
        m_comments.removeAll(c);
        removeItem(c);
        delete c;
      }
    }
    event->accept();
  } else if (event->matches(QKeySequence::Copy)) {
    copySelected();
    event->accept();
  } else if (event->matches(QKeySequence::Paste)) {
    paste(QPointF(0, 0));
    event->accept();
  } else if (event->matches(QKeySequence::Undo)) {
    if (m_undoStack)
      m_undoStack->undo();
    event->accept();
  } else if (event->matches(QKeySequence::Redo)) {
    if (m_undoStack)
      m_undoStack->redo();
    event->accept();
  } else {
    QGraphicsScene::keyPressEvent(event);
  }
}

void NodeGraphScene::createTempLink(NodePortGraphicsItem *port) {
  m_tempLinkStartPort = port;
  if (!m_tempLinkStartPort ||
      m_tempLinkStartPort->portType() != PortType::Output)
    return;
  m_tempLink = new QGraphicsPathItem();
  QPen pen(TypeSystem::portColor(port->dataType()), 2.5, Qt::DashLine);
  m_tempLink->setPen(pen);
  m_tempLink->setZValue(-1);
  m_tempLink->setAcceptedMouseButtons(Qt::NoButton);
  addItem(m_tempLink);
  m_tempLinkStart = m_tempLinkStartPort->scenePos();
}

void NodeGraphScene::updateTempLink(const QPointF &endPos) {
  if (!m_tempLink)
    return;
  QPainterPath path(m_tempLinkStart);
  qreal dist = qAbs(endPos.x() - m_tempLinkStart.x()) * 0.5;
  dist = qMax(dist, 50.0);
  path.cubicTo(m_tempLinkStart.x() + dist, m_tempLinkStart.y(),
               endPos.x() - dist, endPos.y(), endPos.x(), endPos.y());
  m_tempLink->setPath(path);
}

void NodeGraphScene::finishTempLink(NodePortGraphicsItem *destPort) {
  if (m_tempLink) {
    removeItem(m_tempLink);
    delete m_tempLink;
    m_tempLink = nullptr;
  }
  if (destPort && m_tempLinkStartPort) {
    if (destPort->portType() == PortType::Input &&
        m_tempLinkStartPort->portType() == PortType::Output) {
      if (!TypeSystem::areCompatible(m_tempLinkStartPort->dataType(),
                                     destPort->dataType())) {
        m_tempLinkStartPort = nullptr;
        return;
      }
      if (m_tempLinkStartPort->nodeId() == destPort->nodeId()) {
        m_tempLinkStartPort = nullptr;
        return;
      }

      if (m_tempLinkStartPort->dataType() == "Exec") {
        QList<QUuid> toRemove;
        for (const auto *link : m_project->graph()->links()) {
          if (link->sourceNodeId == m_tempLinkStartPort->nodeId() &&
              link->sourcePortId == m_tempLinkStartPort->portId())
            toRemove.append(link->id);
        }
        for (const auto &id : toRemove)
          m_project->graph()->removeLink(id);
      }

      {
        QList<QUuid> toRemove;
        for (const auto *link : m_project->graph()->links()) {
          if (link->targetNodeId == destPort->nodeId() &&
              link->targetPortId == destPort->portId())
            toRemove.append(link->id);
        }
        for (const auto &id : toRemove)
          m_project->graph()->removeLink(id);
      }
      m_project->graph()->addLink(m_tempLinkStartPort->nodeId(),
                                  m_tempLinkStartPort->portId(),
                                  destPort->nodeId(), destPort->portId());
    }
  }
  m_tempLinkStartPort = nullptr;
}

void NodeGraphScene::onNodeAdded(Node *node) {
  auto *item = new NodeGraphicsItem(node);
  item->setOpacity(node->isDeadCode() ? 0.35 : 1.0);
  addItem(item);
  m_nodeItems[node->id()] = item;
}

void NodeGraphScene::onNodeRemoved(const QUuid &id) {
  if (m_nodeItems.contains(id)) {
    auto *item = m_nodeItems.take(id);
    removeItem(item);
    delete item;
  }
}

void NodeGraphScene::onLinkAdded(NodeLink *link) {
  auto *item = new NodeLinkGraphicsItem(link, this);
  addItem(item);
  m_linkItems[link->id] = item;
  item->updatePath();
  if (auto *tgt = m_nodeItems.value(link->targetNodeId))
    tgt->rebuildPorts();
}

void NodeGraphScene::onLinkRemoved(const QUuid &id) {
  if (m_linkItems.contains(id)) {
    auto *item = m_linkItems.take(id);
    QUuid targetNodeId = item->link()->targetNodeId;
    removeItem(item);
    delete item;
    if (auto *tgt = m_nodeItems.value(targetNodeId))
      tgt->rebuildPorts();
  }
}

void NodeGraphScene::updateLinks() {
  for (auto *item : m_linkItems)
    item->updatePath();

  // --- Real-Time Dead Code Analysis (Reachability Graph) ---
  QSet<QUuid> liveNodes;
  QQueue<QUuid> execQueue;

  for (auto *node : m_project->graph()->nodes()) {
    bool hasExecIn = false;
    bool hasExecOut = false;
    for (const auto &in : node->inputs()) {
      if (in.dataType == "Exec") {
        for (const auto *link : m_project->graph()->links()) {
          if (link->targetNodeId == node->id() && link->targetPortId == in.id) {
            hasExecIn = true;
            break;
          }
        }
      }
    }
    for (const auto &out : node->outputs()) {
      if (out.dataType == "Exec")
        hasExecOut = true;
    }

    if (node->type().endsWith(".Start") || (!hasExecIn && hasExecOut)) {
      execQueue.enqueue(node->id());
      liveNodes.insert(node->id());
    }
  }

  while (!execQueue.isEmpty()) {
    QUuid currentId = execQueue.dequeue();
    Node *current = m_project->graph()->getNode(currentId);
    if (!current)
      continue;

    for (const auto &out : current->outputs()) {
      if (out.dataType == "Exec") {
        for (const auto *link : m_project->graph()->links()) {
          if (link->sourceNodeId == currentId && link->sourcePortId == out.id) {
            if (!liveNodes.contains(link->targetNodeId)) {
              liveNodes.insert(link->targetNodeId);
              execQueue.enqueue(link->targetNodeId);
            }
          }
        }
      }
    }
  }

  QQueue<QUuid> dataQueue;
  for (const QUuid &id : liveNodes) {
    dataQueue.enqueue(id);
  }

  while (!dataQueue.isEmpty()) {
    QUuid currentId = dataQueue.dequeue();
    Node *current = m_project->graph()->getNode(currentId);
    if (!current)
      continue;

    for (const auto &in : current->inputs()) {
      if (in.dataType != "Exec") {
        for (const auto *link : m_project->graph()->links()) {
          if (link->targetNodeId == currentId && link->targetPortId == in.id) {
            if (!liveNodes.contains(link->sourceNodeId)) {
              liveNodes.insert(link->sourceNodeId);
              dataQueue.enqueue(link->sourceNodeId);
            }
          }
        }
      }
    }
  }

  for (auto *node : m_project->graph()->nodes()) {
    bool isDead = !liveNodes.contains(node->id());
    if (node->isDeadCode() != isDead) {
      node->setDeadCode(isDead);
      if (auto *item = m_nodeItems.value(node->id())) {
        item->setOpacity(isDead ? 0.35 : 1.0);
        item->update();
      }
    }
  }
}

void NodeGraphScene::addComment(const QPointF &pos) {
  auto *comment = new NodeCommentItem("Comment");
  comment->setPos(pos);
  addItem(comment);
  m_comments.append(comment);
}

void NodeGraphScene::zoomToFit(QGraphicsView *view) {
  if (!view)
    return;
  QRectF bounds = itemsBoundingRect().adjusted(-50, -50, 50, 50);
  if (bounds.isValid())
    view->fitInView(bounds, Qt::KeepAspectRatio);
}

void NodeGraphScene::autoLayout() {
  QMap<QUuid, int> depth;
  QList<Node *> startNodes;

  for (auto *node : m_project->graph()->nodes()) {
    bool hasExecIn = false;
    for (const auto &in : node->inputs()) {
      if (in.dataType == "Exec") {
        for (const auto *link : m_project->graph()->links()) {
          if (link->targetNodeId == node->id() && link->targetPortId == in.id) {
            hasExecIn = true;
            break;
          }
        }
      }
      if (hasExecIn)
        break;
    }
    if (!hasExecIn)
      startNodes.append(node);
  }

  QQueue<QPair<Node *, int>> queue;
  for (auto *n : startNodes)
    queue.enqueue({n, 0});

  while (!queue.isEmpty()) {
    auto [node, d] = queue.dequeue();
    if (depth.contains(node->id()) && depth[node->id()] >= d)
      continue;
    depth[node->id()] = d;

    for (const auto &out : node->outputs()) {
      for (const auto *link : m_project->graph()->links()) {
        if (link->sourceNodeId == node->id() && link->sourcePortId == out.id) {
          Node *next = m_project->graph()->getNode(link->targetNodeId);
          if (next)
            queue.enqueue({next, d + 1});
        }
      }
    }
  }

  QMap<int, QList<Node *>> layers;
  for (auto it = depth.begin(); it != depth.end(); ++it)
    layers[it.value()].append(m_project->graph()->getNode(it.key()));

  qreal x = 0;
  for (auto it = layers.begin(); it != layers.end(); ++it) {
    qreal y = 0;
    for (auto *node : it.value()) {
      node->setPosition(QPointF(x, y));
      if (auto *item = m_nodeItems.value(node->id())) {
        item->setPos(QPointF(x, y));
        y += item->boundingRect().height() + 40;
      } else {
        y += 120;
      }
    }
    x += 280;
  }
  updateLinks();
}

void NodeGraphScene::copySelected() {
  nlohmann::json j;
  j["nodes"] = nlohmann::json::array();
  QSet<QUuid> selectedNodeIds;

  for (auto *item : selectedItems()) {
    if (auto *nodeItem = dynamic_cast<NodeGraphicsItem *>(item)) {
      selectedNodeIds.insert(nodeItem->node()->id());
      auto *node = nodeItem->node();
      nlohmann::json jn;
      jn["type"] = node->type().toStdString();
      jn["name"] = node->name().toStdString();
      jn["x"] = node->position().x();
      jn["y"] = node->position().y();
      jn["hasValue"] = node->hasValue();
      jn["value"] = node->value().toStdString();
      jn["language"] = node->language().toStdString();
      jn["id"] = node->id().toString().toStdString();
      j["nodes"].push_back(jn);
    }
  }

  j["links"] = nlohmann::json::array();
  for (const auto *link : m_project->graph()->links()) {
    if (selectedNodeIds.contains(link->sourceNodeId) &&
        selectedNodeIds.contains(link->targetNodeId)) {
      Node *srcNode = m_project->graph()->getNode(link->sourceNodeId);
      Node *tgtNode = m_project->graph()->getNode(link->targetNodeId);
      QString srcPortName, tgtPortName;
      if (srcNode)
        for (const auto &p : srcNode->outputs())
          if (p.id == link->sourcePortId) {
            srcPortName = p.name;
            break;
          }
      if (tgtNode)
        for (const auto &p : tgtNode->inputs())
          if (p.id == link->targetPortId) {
            tgtPortName = p.name;
            break;
          }

      j["links"].push_back(
          {{"sourceNode", link->sourceNodeId.toString().toStdString()},
           {"sourcePort", srcPortName.toStdString()},
           {"targetNode", link->targetNodeId.toString().toStdString()},
           {"targetPort", tgtPortName.toStdString()}});
    }
  }

  m_clipboard = QString::fromStdString(j.dump());
  QApplication::clipboard()->setText(m_clipboard);
}

void NodeGraphScene::paste(const QPointF &offset) {
  QString data = QApplication::clipboard()->text();
  if (data.isEmpty())
    return;

  try {
    nlohmann::json j = nlohmann::json::parse(data.toStdString());
    QMap<QString, Node *> idMap;
    auto defs = m_project->nodeDefinitions();

    for (const auto &jn : j["nodes"]) {
      QString type = QString::fromStdString(jn.value("type", ""));
      QString name = QString::fromStdString(jn.value("name", ""));
      QPointF pos(jn.value("x", 0.0) + 30, jn.value("y", 0.0) + 30);
      QString savedId = QString::fromStdString(jn.value("id", ""));

      Node *node = m_project->graph()->addNode(type, name, pos);
      node->setHasValue(jn.value("hasValue", false));
      node->setValue(QString::fromStdString(jn.value("value", "")));
      node->setLanguage(QString::fromStdString(jn.value("language", "")));

      if (defs.contains(type)) {
        const auto &def = defs[type];
        for (const auto &pd : def.inputs)
          node->addInput(pd.name, pd.type, pd.defaultValue, pd.allowInline);
        for (const auto &pd : def.outputs)
          node->addOutput(pd.name, pd.type, pd.defaultValue, pd.allowInline);
        node->setCodeTemplate(def.code);
      }
      if (auto *item = m_nodeItems.value(node->id()))
        item->rebuildPorts();

      idMap[savedId] = node;
    }

    for (const auto &jl : j["links"]) {
      QString srcId = QString::fromStdString(jl.value("sourceNode", ""));
      QString tgtId = QString::fromStdString(jl.value("targetNode", ""));
      QString srcPort = QString::fromStdString(jl.value("sourcePort", ""));
      QString tgtPort = QString::fromStdString(jl.value("targetPort", ""));

      Node *srcNode = idMap.value(srcId);
      Node *tgtNode = idMap.value(tgtId);
      if (!srcNode || !tgtNode)
        continue;

      QUuid srcPortId, tgtPortId;
      for (const auto &p : srcNode->outputs())
        if (p.name == srcPort) {
          srcPortId = p.id;
          break;
        }
      for (const auto &p : tgtNode->inputs())
        if (p.name == tgtPort) {
          tgtPortId = p.id;
          break;
        }

      if (!srcPortId.isNull() && !tgtPortId.isNull())
        m_project->graph()->addLink(srcNode->id(), srcPortId, tgtNode->id(),
                                    tgtPortId);
    }
  } catch (...) {
  }
}

void NodeGraphScene::highlightErrors(const QList<QUuid> &errorNodes) {
  for (auto *item : m_nodeItems)
    item->setErrorHighlight(false);
  for (const auto &id : errorNodes) {
    if (auto *item = m_nodeItems.value(id))
      item->setErrorHighlight(true);
  }
}