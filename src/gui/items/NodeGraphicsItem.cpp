#include "NodeGraphicsItem.h"
#include "gui/scene/NodeGraphScene.h"
#include "core/TypeSystem.h"
#include "core/CodeGenerator.h"
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsScene>
#include <QInputDialog>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>

static constexpr int kHeaderH = 28;
static constexpr int kPortStartY = 38;
static constexpr int kPortSpacing = 24;
static constexpr int kInlineEditorWidth = 80;
static constexpr int kPortTextX = 14;
static constexpr int kEditorGap = 4;
static constexpr qreal kGridSize = 30.0;

NodeGraphicsItem::NodeGraphicsItem(Node *node)
    : m_node(node), m_editor(nullptr), m_proxy(nullptr) {
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  setPos(node->position());
  setAcceptHoverEvents(true);
  setToolTip(CodeGenerator::generateSingleNode(node));

  rebuildPorts();

  auto *shadow = new QGraphicsDropShadowEffect();
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 4);
  shadow->setColor(QColor(0, 0, 0, 120));
  setGraphicsEffect(shadow);
}

void NodeGraphicsItem::rebuildPorts() {
  for (auto *p : m_ports) {
    if (p->scene()) p->scene()->removeItem(p);
    delete p;
  }
  m_ports.clear();

  // If collapsed, skip port creation
  if (m_node->collapsed()) {
    prepareGeometryChange();
    update();
    return;
  }

  QFont portFont;
  portFont.setPointSize(8);
  QFontMetrics portFm(portFont);

  qreal y = kPortStartY;
  for (const auto &in : m_node->inputs()) {
    auto *portItem = new NodePortGraphicsItem(this, in, in.id, m_node->id());
    portItem->setPos(0, y);
    m_ports[in.id] = portItem;

    if (in.allowInline) {
      if (!m_portProxies.contains(in.id)) {
        auto *editor = new QLineEdit();
        editor->setFixedWidth(kInlineEditorWidth);
        editor->setStyleSheet(
            "background-color: #1a1a1d; color: #e0e0e0; border: 1px solid #3b3b4a; "
            "border-radius: 3px; font-size: 10px; padding: 1px;");
        editor->setText(in.value);
        QObject::connect(editor, &QLineEdit::textChanged,
                         [this, id = in.id](const QString &text) {
                           m_node->setPortValue(id, text);
                           if (auto *s = dynamic_cast<NodeGraphScene *>(scene()))
                             emit s->getProject()->graph()->graphChanged();
                         });
        auto *proxy = new QGraphicsProxyWidget(this);
        proxy->setWidget(editor);
        m_portProxies[in.id] = proxy;
      }

      int labelW = portFm.horizontalAdvance(in.label());
      qreal editorX = kPortTextX + labelW + kEditorGap;
      m_portProxies[in.id]->setPos(editorX, y - 10);

      bool connected = false;
      if (auto *s = dynamic_cast<NodeGraphScene *>(scene())) {
        for (auto *link : s->getProject()->graph()->links()) {
          if (link->targetNodeId == m_node->id() && link->targetPortId == in.id) {
            connected = true;
            break;
          }
        }
      }
      m_portProxies[in.id]->setVisible(!connected);
    } else {
      if (m_portProxies.contains(in.id)) {
        auto *proxy = m_portProxies.take(in.id);
        delete proxy;
      }
    }
    y += kPortSpacing;
  }

  y = kPortStartY;
  for (const auto &out : m_node->outputs()) {
    auto *portItem = new NodePortGraphicsItem(this, out, out.id, m_node->id());
    portItem->setPos(boundingRect().width(), y);
    m_ports[out.id] = portItem;
    y += kPortSpacing;
  }

  if (m_node->hasValue()) {
    if (!m_editor) {
      m_editor = new QLineEdit();
      m_editor->setStyleSheet(
          "background-color: #1a1a1d; color: white; border: 1px solid #3b3b4a; "
          "border-radius: 3px; padding: 2px; font-size: 10px;");
      m_editor->setText(m_node->value());
      QObject::connect(m_editor, &QLineEdit::textChanged,
                       [this](const QString &text) {
                         m_node->setValue(text);
                         if (auto *s = dynamic_cast<NodeGraphScene *>(scene()))
                           emit s->getProject()->graph()->graphChanged();
                       });
      m_proxy = new QGraphicsProxyWidget(this);
      m_proxy->setWidget(m_editor);
      m_proxy->setZValue(1);
    }
    int maxPorts = qMax(m_node->inputs().size(), m_node->outputs().size());
    int startY = kPortStartY + maxPorts * kPortSpacing + 5;
    m_proxy->setGeometry(QRectF(8, startY, boundingRect().width() - 16, 22));
  } else if (m_proxy) {
    delete m_proxy;
    m_proxy = nullptr;
    m_editor = nullptr;
  }
  prepareGeometryChange();
  update();
}

QRectF NodeGraphicsItem::boundingRect() const {
  if (m_node->collapsed())
    return QRectF(0, 0, 120, kHeaderH);

  int maxPorts = qMax(m_node->inputs().size(), m_node->outputs().size());
  int h = qMax(50, kPortStartY + maxPorts * kPortSpacing + 10);
  if (m_node->hasValue()) h += 32;

  QFont titleFont;
  titleFont.setBold(true);
  titleFont.setPointSize(9);
  QFontMetrics titleFm(titleFont);
  int titleW = titleFm.horizontalAdvance(m_node->name()) + 28;

  QFont portFont;
  portFont.setPointSize(8);
  QFontMetrics portFm(portFont);

  int leftW = 0;
  for (const auto &in : m_node->inputs()) {
    int pw = kPortTextX + portFm.horizontalAdvance(in.label()) + kEditorGap;
    if (in.allowInline) pw += kInlineEditorWidth + kEditorGap;
    leftW = qMax(leftW, pw);
  }
  int rightW = 0;
  for (const auto &out : m_node->outputs()) {
    rightW = qMax(rightW, portFm.horizontalAdvance(out.label()) + 14);
  }

  int contentW = leftW + 8 + rightW;
  int w = qMax(titleW, contentW);
  w = qMax(w, 120);

  return QRectF(0, 0, w, h);
}

void NodeGraphicsItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget) {
  Q_UNUSED(option);
  Q_UNUSED(widget);

  QPainterPath path;
  path.addRoundedRect(boundingRect(), 6, 6);
  painter->fillPath(path, QColor(43, 43, 48, 240));

  // Border: error=red, selected=blue, normal=dark
  if (m_hasError) {
    painter->setPen(QPen(QColor(244, 67, 54), 2.0));
  } else if (isSelected()) {
    painter->setPen(QPen(QColor(100, 200, 255), 2.0));
  } else {
    painter->setPen(QPen(QColor(25, 25, 30), 1.5));
  }
  painter->drawPath(path);

  // Header
  QPainterPath headerPath;
  headerPath.addRoundedRect(0, 0, boundingRect().width(), kHeaderH, 6, 6);
  QPainterPath bottomSquare;
  bottomSquare.addRect(0, kHeaderH / 2, boundingRect().width(), kHeaderH / 2);
  headerPath = headerPath.united(bottomSquare).intersected(path);

  QLinearGradient bg(0, 0, 0, kHeaderH);
  bg.setColorAt(0, QColor(44, 83, 158));
  bg.setColorAt(1, QColor(29, 58, 115));
  painter->fillPath(headerPath, bg);

  // Breakpoint indicator
  if (m_node->hasBreakpoint()) {
    painter->setBrush(QColor(244, 67, 54));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(10, kHeaderH / 2.0), 5, 5);
  }

  // Collapse indicator
  painter->setPen(QColor(180, 180, 180));
  QFont sf = painter->font();
  sf.setPointSize(8);
  painter->setFont(sf);
  QString collapseIcon = m_node->collapsed() ? "+" : "-";
  painter->drawText(QRectF(boundingRect().width() - 18, 2, 16, kHeaderH - 4),
                    Qt::AlignCenter, collapseIcon);

  // Title
  painter->setPen(Qt::white);
  QFont f = painter->font();
  f.setBold(true);
  f.setPointSize(9);
  painter->setFont(f);
  painter->drawText(QRectF(m_node->hasBreakpoint() ? 18 : 0, 0,
                            boundingRect().width() - 20, kHeaderH),
                    Qt::AlignCenter, m_node->name());

  if (m_node->collapsed()) return;

  // Port labels
  f.setBold(false);
  f.setPointSize(8);
  painter->setFont(f);
  painter->setPen(QColor(210, 210, 210));
  QFontMetrics portFm(f);

  int inY = kPortStartY;
  for (const auto &p : m_node->inputs()) {
    int labelW = portFm.horizontalAdvance(p.label()) + 4;
    painter->drawText(QRectF(kPortTextX, inY - 9, labelW, 18),
                      Qt::AlignLeft | Qt::AlignVCenter, p.label());
    inY += kPortSpacing;
  }
  int outY = kPortStartY;
  for (const auto &p : m_node->outputs()) {
    painter->drawText(QRectF(5, outY - 9, boundingRect().width() - 14, 18),
                      Qt::AlignRight | Qt::AlignVCenter, p.label());
    outY += kPortSpacing;
  }
}

QPointF NodeGraphicsItem::getPortPosition(const QUuid &portId) const {
  if (m_ports.contains(portId))
    return m_ports[portId]->scenePos();
  return scenePos() + QPointF(boundingRect().width() / 2, kHeaderH / 2);
}

QVariant NodeGraphicsItem::itemChange(GraphicsItemChange change,
                                      const QVariant &value) {
  if (change == ItemPositionChange && scene()) {
    QPointF newPos = value.toPointF();
    // Snap to grid
    newPos.setX(qRound(newPos.x() / kGridSize) * kGridSize);
    newPos.setY(qRound(newPos.y() / kGridSize) * kGridSize);
    m_node->setPosition(newPos);
    if (auto *s = dynamic_cast<NodeGraphScene *>(scene()))
      s->updateLinks();
    return newPos;
  }
  return QGraphicsItem::itemChange(change, value);
}

void NodeGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsItem::mouseMoveEvent(event);
  if (auto *s = dynamic_cast<NodeGraphScene *>(scene()))
    s->updateLinks();
}

void NodeGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  // Double-click on collapse button area
  if (event->pos().x() > boundingRect().width() - 20 && event->pos().y() < kHeaderH) {
    m_node->setCollapsed(!m_node->collapsed());
    rebuildPorts();
    if (auto *s = dynamic_cast<NodeGraphScene *>(scene()))
      s->updateLinks();
    event->accept();
    return;
  }
  // Double-click on header to rename
  if (event->pos().y() < kHeaderH) {
    bool ok;
    QString newName = QInputDialog::getText(nullptr, "Rename Node", "Node name:",
                                            QLineEdit::Normal, m_node->name(), &ok);
    if (ok && !newName.isEmpty()) {
      m_node->setName(newName);
      setToolTip(CodeGenerator::generateSingleNode(m_node));
      prepareGeometryChange();
      update();
    }
    event->accept();
    return;
  }
  QGraphicsItem::mouseDoubleClickEvent(event);
}
