#include "NodeLinkGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include "core/Node.h"
#include "core/TypeSystem.h"
#include "gui/scene/NodeGraphScene.h"

NodeLinkGraphicsItem::NodeLinkGraphicsItem(NodeLink *link,
                                           NodeGraphScene *scene)
    : m_link(link), m_scene(scene) {
  setZValue(-1);

  // Determine source port type for coloring
  QString dataType = "Any";
  if (scene && link) {
    Node *srcNode = scene->getProject()->graph()->getNode(link->sourceNodeId);
    if (srcNode) {
      for (const auto &p : srcNode->outputs()) {
        if (p.id == link->sourcePortId) {
          dataType = p.dataType;
          break;
        }
      }
    }
  }

  m_isExec = (dataType == "Exec");
  QColor color = TypeSystem::linkColor(dataType);

  if (m_isExec) {
    QPen pen(color, 2.5, Qt::DashLine);
    setPen(pen);
    // Animate dash offset for flow direction
    connect(&m_animTimer, &QTimer::timeout, this, [this, color]() {
      m_dashOffset -= 1.5;
      QPen p(color, 3.0, Qt::DashLine);
      p.setDashOffset(m_dashOffset);
      setPen(p);
      update();
    });
    m_animTimer.start(40);
  } else {
    QPen pen(color, 2.5);
    setPen(pen);
  }
}

void NodeLinkGraphicsItem::updatePath() {
  if (!m_link || !m_scene)
    return;
  auto *srcNode = m_scene->getNodeItem(m_link->sourceNodeId);
  auto *tgtNode = m_scene->getNodeItem(m_link->targetNodeId);
  if (!srcNode || !tgtNode)
    return;

  QPointF p1 = srcNode->getPortPosition(m_link->sourcePortId);
  QPointF p2 = tgtNode->getPortPosition(m_link->targetPortId);

  QPainterPath path(p1);
  qreal dist = qAbs(p2.x() - p1.x()) * 0.5;
  dist = qMax(dist, 50.0);
  path.cubicTo(p1.x() + dist, p1.y(), p2.x() - dist, p2.y(), p2.x(), p2.y());
  setPath(path);
}
