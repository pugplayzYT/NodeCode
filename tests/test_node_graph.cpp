#include "core/NodeGraph.h"
#include <QSignalSpy>
#include <gtest/gtest.h>

// ── Add / Get Node ──────────────────────────────────────────────────────────

TEST(NodeGraphTest, AddNodeReturnsValidPointer) {
  NodeGraph graph;
  Node *node = graph.addNode("MathAdd", "Add", QPointF(0, 0));
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->type(), "MathAdd");
  EXPECT_EQ(node->name(), "Add");
}

TEST(NodeGraphTest, AddNodeSetsPosition) {
  NodeGraph graph;
  Node *node = graph.addNode("T", "N", QPointF(100, 200));
  EXPECT_DOUBLE_EQ(node->position().x(), 100.0);
  EXPECT_DOUBLE_EQ(node->position().y(), 200.0);
}

TEST(NodeGraphTest, AddNodeAppearsInNodesMap) {
  NodeGraph graph;
  Node *node = graph.addNode("T", "N", QPointF(0, 0));
  EXPECT_EQ(graph.nodes().size(), 1);
  EXPECT_TRUE(graph.nodes().contains(node->id()));
}

TEST(NodeGraphTest, AddMultipleNodes) {
  NodeGraph graph;
  graph.addNode("T1", "A", QPointF(0, 0));
  graph.addNode("T2", "B", QPointF(10, 10));
  graph.addNode("T3", "C", QPointF(20, 20));
  EXPECT_EQ(graph.nodes().size(), 3);
}

TEST(NodeGraphTest, GetNodeById) {
  NodeGraph graph;
  Node *node = graph.addNode("T", "N", QPointF(0, 0));
  Node *found = graph.getNode(node->id());
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(found->id(), node->id());
  EXPECT_EQ(found->name(), "N");
}

TEST(NodeGraphTest, GetNodeInvalidIdReturnsNull) {
  NodeGraph graph;
  graph.addNode("T", "N", QPointF(0, 0));
  Node *found = graph.getNode(QUuid::createUuid());
  EXPECT_EQ(found, nullptr);
}

// ── Remove Node ─────────────────────────────────────────────────────────────

TEST(NodeGraphTest, RemoveNode) {
  NodeGraph graph;
  Node *node = graph.addNode("T", "N", QPointF(0, 0));
  QUuid id = node->id();
  graph.removeNode(id);
  EXPECT_EQ(graph.nodes().size(), 0);
  EXPECT_EQ(graph.getNode(id), nullptr);
}

TEST(NodeGraphTest, RemoveNodeInvalidIdDoesNothing) {
  NodeGraph graph;
  graph.addNode("T", "N", QPointF(0, 0));
  graph.removeNode(QUuid::createUuid());
  EXPECT_EQ(graph.nodes().size(), 1);
}

TEST(NodeGraphTest, RemoveNodeCascadesLinks) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");

  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);
  EXPECT_EQ(graph.links().size(), 1);

  graph.removeNode(a->id());
  EXPECT_EQ(graph.links().size(), 0);
  EXPECT_EQ(graph.nodes().size(), 1);
}

TEST(NodeGraphTest, RemoveNodeCascadesOnlyConnectedLinks) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  Node *c = graph.addNode("T", "C", QPointF(200, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");
  b->addOutput("Out", "Number");
  c->addInput("In", "Number");

  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);
  graph.addLink(b->id(), b->outputs()[0].id, c->id(), c->inputs()[0].id);
  EXPECT_EQ(graph.links().size(), 2);

  // Remove middle node: both links should be removed
  graph.removeNode(b->id());
  EXPECT_EQ(graph.links().size(), 0);
  EXPECT_EQ(graph.nodes().size(), 2);
}

// ── Links ───────────────────────────────────────────────────────────────────

TEST(NodeGraphTest, AddLink) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");

  NodeLink *link = graph.addLink(a->id(), a->outputs()[0].id,
                                  b->id(), b->inputs()[0].id);
  ASSERT_NE(link, nullptr);
  EXPECT_FALSE(link->id.isNull());
  EXPECT_EQ(link->sourceNodeId, a->id());
  EXPECT_EQ(link->targetNodeId, b->id());
  EXPECT_EQ(link->sourcePortId, a->outputs()[0].id);
  EXPECT_EQ(link->targetPortId, b->inputs()[0].id);
  EXPECT_EQ(graph.links().size(), 1);
}

TEST(NodeGraphTest, AddMultipleLinks) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  Node *c = graph.addNode("T", "C", QPointF(200, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");
  b->addOutput("Out2", "Number");
  c->addInput("In2", "Number");

  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);
  graph.addLink(b->id(), b->outputs()[0].id, c->id(), c->inputs()[0].id);
  EXPECT_EQ(graph.links().size(), 2);
}

TEST(NodeGraphTest, RemoveLink) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");

  NodeLink *link = graph.addLink(a->id(), a->outputs()[0].id,
                                  b->id(), b->inputs()[0].id);
  QUuid linkId = link->id;
  graph.removeLink(linkId);
  EXPECT_EQ(graph.links().size(), 0);
}

TEST(NodeGraphTest, RemoveLinkInvalidIdDoesNothing) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");
  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);
  graph.removeLink(QUuid::createUuid());
  EXPECT_EQ(graph.links().size(), 1);
}

// ── Clear ───────────────────────────────────────────────────────────────────

TEST(NodeGraphTest, ClearEmptyGraph) {
  NodeGraph graph;
  graph.clear();
  EXPECT_EQ(graph.nodes().size(), 0);
  EXPECT_EQ(graph.links().size(), 0);
}

TEST(NodeGraphTest, ClearRemovesNodesAndLinks) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");
  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);

  graph.clear();
  EXPECT_EQ(graph.nodes().size(), 0);
  EXPECT_EQ(graph.links().size(), 0);
}

// ── Signals ─────────────────────────────────────────────────────────────────

TEST(NodeGraphTest, SignalNodeAdded) {
  NodeGraph graph;
  QSignalSpy spy(&graph, &NodeGraph::nodeAdded);
  graph.addNode("T", "N", QPointF(0, 0));
  EXPECT_EQ(spy.count(), 1);
}

TEST(NodeGraphTest, SignalNodeRemoved) {
  NodeGraph graph;
  Node *node = graph.addNode("T", "N", QPointF(0, 0));
  QSignalSpy spy(&graph, &NodeGraph::nodeRemoved);
  graph.removeNode(node->id());
  EXPECT_EQ(spy.count(), 1);
}

TEST(NodeGraphTest, SignalLinkAdded) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");

  QSignalSpy spy(&graph, &NodeGraph::linkAdded);
  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);
  EXPECT_EQ(spy.count(), 1);
}

TEST(NodeGraphTest, SignalLinkRemoved) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");
  NodeLink *link = graph.addLink(a->id(), a->outputs()[0].id,
                                  b->id(), b->inputs()[0].id);

  QSignalSpy spy(&graph, &NodeGraph::linkRemoved);
  graph.removeLink(link->id);
  EXPECT_EQ(spy.count(), 1);
}

TEST(NodeGraphTest, SignalGraphChangedOnAddNode) {
  NodeGraph graph;
  QSignalSpy spy(&graph, &NodeGraph::graphChanged);
  graph.addNode("T", "N", QPointF(0, 0));
  EXPECT_EQ(spy.count(), 1);
}

TEST(NodeGraphTest, SignalGraphChangedOnRemoveNode) {
  NodeGraph graph;
  Node *node = graph.addNode("T", "N", QPointF(0, 0));
  QSignalSpy spy(&graph, &NodeGraph::graphChanged);
  graph.removeNode(node->id());
  EXPECT_GE(spy.count(), 1);
}

TEST(NodeGraphTest, SignalGraphChangedOnAddLink) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");

  QSignalSpy spy(&graph, &NodeGraph::graphChanged);
  graph.addLink(a->id(), a->outputs()[0].id, b->id(), b->inputs()[0].id);
  EXPECT_EQ(spy.count(), 1);
}

TEST(NodeGraphTest, SignalGraphChangedOnRemoveLink) {
  NodeGraph graph;
  Node *a = graph.addNode("T", "A", QPointF(0, 0));
  Node *b = graph.addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");
  NodeLink *link = graph.addLink(a->id(), a->outputs()[0].id,
                                  b->id(), b->inputs()[0].id);

  QSignalSpy spy(&graph, &NodeGraph::graphChanged);
  graph.removeLink(link->id);
  EXPECT_GE(spy.count(), 1);
}

TEST(NodeGraphTest, NoSignalOnRemoveInvalidNode) {
  NodeGraph graph;
  graph.addNode("T", "N", QPointF(0, 0));
  QSignalSpy spy(&graph, &NodeGraph::nodeRemoved);
  graph.removeNode(QUuid::createUuid());
  EXPECT_EQ(spy.count(), 0);
}

TEST(NodeGraphTest, NoSignalOnRemoveInvalidLink) {
  NodeGraph graph;
  QSignalSpy spy(&graph, &NodeGraph::linkRemoved);
  graph.removeLink(QUuid::createUuid());
  EXPECT_EQ(spy.count(), 0);
}
