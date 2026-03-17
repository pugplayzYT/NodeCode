#include "core/Node.h"
#include <QApplication>
#include <gtest/gtest.h>

TEST(NodeTest, BasicProperties) {
  Node node("MathAdd", "Add");
  EXPECT_EQ(node.type(), "MathAdd");
  EXPECT_EQ(node.name(), "Add");

  node.setName("Addition");
  EXPECT_EQ(node.name(), "Addition");

  node.setPosition(QPointF(10.5, 20.0));
  EXPECT_EQ(node.position().x(), 10.5);
  EXPECT_EQ(node.position().y(), 20.0);
}

TEST(NodeTest, PortsManagement) {
  Node node("MathAdd", "Add");

  node.addInput("A", "number", "0");
  node.addInput("B", "number", "0");
  node.addOutput("Result", "number");

  EXPECT_EQ(node.inputs().size(), 2);
  EXPECT_EQ(node.outputs().size(), 1);

  EXPECT_EQ(node.inputs()[0].name, "A");
  EXPECT_EQ(node.inputs()[0].dataType, "number");
  EXPECT_EQ(node.inputs()[0].value, "0");

  EXPECT_EQ(node.outputs()[0].name, "Result");
}

int main(int argc, char **argv) {
  // Qt requires a QApplication object to be present for some object
  // initializations like QGraphicsItem if we test GUI components later, or
  // certain QObject features.
  QApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
