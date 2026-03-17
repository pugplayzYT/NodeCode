#include "core/Node.h"
#include <gtest/gtest.h>

// ── Construction & Basic Properties ──────────────────────────────────────────

TEST(NodeTest, ConstructorSetsTypeAndName) {
  Node node("MathAdd", "Add");
  EXPECT_EQ(node.type(), "MathAdd");
  EXPECT_EQ(node.name(), "Add");
}

TEST(NodeTest, ConstructorAssignsUniqueId) {
  Node a("T", "A");
  Node b("T", "B");
  EXPECT_NE(a.id(), b.id());
  EXPECT_FALSE(a.id().isNull());
  EXPECT_FALSE(b.id().isNull());
}

TEST(NodeTest, SetName) {
  Node node("T", "Original");
  node.setName("Renamed");
  EXPECT_EQ(node.name(), "Renamed");
}

TEST(NodeTest, SetNameEmpty) {
  Node node("T", "Name");
  node.setName("");
  EXPECT_EQ(node.name(), "");
}

TEST(NodeTest, DefaultPositionIsOrigin) {
  Node node("T", "N");
  EXPECT_EQ(node.position().x(), 0.0);
  EXPECT_EQ(node.position().y(), 0.0);
}

TEST(NodeTest, SetPosition) {
  Node node("T", "N");
  node.setPosition(QPointF(10.5, 20.0));
  EXPECT_DOUBLE_EQ(node.position().x(), 10.5);
  EXPECT_DOUBLE_EQ(node.position().y(), 20.0);
}

TEST(NodeTest, SetPositionNegative) {
  Node node("T", "N");
  node.setPosition(QPointF(-100.5, -200.75));
  EXPECT_DOUBLE_EQ(node.position().x(), -100.5);
  EXPECT_DOUBLE_EQ(node.position().y(), -200.75);
}

// ── Port Management ─────────────────────────────────────────────────────────

TEST(NodeTest, AddInputPort) {
  Node node("T", "N");
  node.addInput("A", "Number", "0");
  ASSERT_EQ(node.inputs().size(), 1);
  EXPECT_EQ(node.inputs()[0].name, "A");
  EXPECT_EQ(node.inputs()[0].dataType, "Number");
  EXPECT_EQ(node.inputs()[0].value, "0");
  EXPECT_EQ(node.inputs()[0].type, PortType::Input);
  EXPECT_FALSE(node.inputs()[0].id.isNull());
}

TEST(NodeTest, AddOutputPort) {
  Node node("T", "N");
  node.addOutput("Result", "Number");
  ASSERT_EQ(node.outputs().size(), 1);
  EXPECT_EQ(node.outputs()[0].name, "Result");
  EXPECT_EQ(node.outputs()[0].dataType, "Number");
  EXPECT_EQ(node.outputs()[0].type, PortType::Output);
  EXPECT_FALSE(node.outputs()[0].id.isNull());
}

TEST(NodeTest, AddMultipleInputs) {
  Node node("T", "N");
  node.addInput("A", "Number", "0");
  node.addInput("B", "Number", "0");
  node.addInput("C", "String", "hello");
  EXPECT_EQ(node.inputs().size(), 3);
  EXPECT_EQ(node.inputs()[0].name, "A");
  EXPECT_EQ(node.inputs()[1].name, "B");
  EXPECT_EQ(node.inputs()[2].name, "C");
}

TEST(NodeTest, AddMultipleOutputs) {
  Node node("T", "N");
  node.addOutput("R1", "Number");
  node.addOutput("R2", "String");
  EXPECT_EQ(node.outputs().size(), 2);
}

TEST(NodeTest, InputAndOutputCountsAreIndependent) {
  Node node("T", "N");
  node.addInput("A", "Number");
  node.addInput("B", "Number");
  node.addOutput("R", "Number");
  EXPECT_EQ(node.inputs().size(), 2);
  EXPECT_EQ(node.outputs().size(), 1);
}

TEST(NodeTest, PortDefaultValueEmpty) {
  Node node("T", "N");
  node.addInput("A", "Number");
  EXPECT_EQ(node.inputs()[0].value, "");
}

TEST(NodeTest, PortAllowInlineDefault) {
  Node node("T", "N");
  node.addInput("A", "Number");
  EXPECT_TRUE(node.inputs()[0].allowInline);
}

TEST(NodeTest, PortAllowInlineExplicitFalse) {
  Node node("T", "N");
  node.addInput("A", "Number", "", false);
  EXPECT_FALSE(node.inputs()[0].allowInline);
}

TEST(NodeTest, OutputPortAllowInline) {
  Node node("T", "N");
  node.addOutput("R", "Number", "", false);
  EXPECT_FALSE(node.outputs()[0].allowInline);
}

TEST(NodeTest, EachPortGetsUniqueId) {
  Node node("T", "N");
  node.addInput("A", "Number");
  node.addInput("B", "Number");
  node.addOutput("R", "Number");
  EXPECT_NE(node.inputs()[0].id, node.inputs()[1].id);
  EXPECT_NE(node.inputs()[0].id, node.outputs()[0].id);
}

// ── Port Customization ──────────────────────────────────────────────────────

TEST(NodeTest, SetPortValueOnInput) {
  Node node("T", "N");
  node.addInput("A", "Number", "0");
  QUuid portId = node.inputs()[0].id;
  node.setPortValue(portId, "42");
  EXPECT_EQ(node.inputs()[0].value, "42");
}

TEST(NodeTest, SetPortValueOnOutput) {
  Node node("T", "N");
  node.addOutput("R", "Number", "0");
  QUuid portId = node.outputs()[0].id;
  node.setPortValue(portId, "99");
  EXPECT_EQ(node.outputs()[0].value, "99");
}

TEST(NodeTest, SetPortValueInvalidIdDoesNothing) {
  Node node("T", "N");
  node.addInput("A", "Number", "0");
  node.setPortValue(QUuid::createUuid(), "999");
  EXPECT_EQ(node.inputs()[0].value, "0");
}

TEST(NodeTest, SetPortDisplayNameOnInput) {
  Node node("T", "N");
  node.addInput("A", "Number");
  QUuid portId = node.inputs()[0].id;
  node.setPortDisplayName(portId, "Input A");
  EXPECT_EQ(node.inputs()[0].displayName, "Input A");
}

TEST(NodeTest, SetPortDisplayNameOnOutput) {
  Node node("T", "N");
  node.addOutput("R", "Number");
  QUuid portId = node.outputs()[0].id;
  node.setPortDisplayName(portId, "Result Output");
  EXPECT_EQ(node.outputs()[0].displayName, "Result Output");
}

TEST(NodeTest, SetPortDisplayNameInvalidIdDoesNothing) {
  Node node("T", "N");
  node.addInput("A", "Number");
  node.setPortDisplayName(QUuid::createUuid(), "X");
  EXPECT_EQ(node.inputs()[0].displayName, "");
}

TEST(NodeTest, SetPortAllowInlineOnInput) {
  Node node("T", "N");
  node.addInput("A", "Number", "", true);
  QUuid portId = node.inputs()[0].id;
  node.setPortAllowInline(portId, false);
  EXPECT_FALSE(node.inputs()[0].allowInline);
}

TEST(NodeTest, SetPortAllowInlineOnOutput) {
  Node node("T", "N");
  node.addOutput("R", "Number", "", true);
  QUuid portId = node.outputs()[0].id;
  node.setPortAllowInline(portId, false);
  EXPECT_FALSE(node.outputs()[0].allowInline);
}

TEST(NodeTest, SetPortAllowInlineInvalidIdDoesNothing) {
  Node node("T", "N");
  node.addInput("A", "Number", "", true);
  node.setPortAllowInline(QUuid::createUuid(), false);
  EXPECT_TRUE(node.inputs()[0].allowInline);
}

// ── NodePort::label() ───────────────────────────────────────────────────────

TEST(NodePortTest, LabelReturnsNameWhenNoDisplayName) {
  NodePort port{QUuid::createUuid(), PortType::Input, "MyPort", "Number", "", true, ""};
  EXPECT_EQ(port.label(), "MyPort");
}

TEST(NodePortTest, LabelReturnsDisplayNameWhenSet) {
  NodePort port{QUuid::createUuid(), PortType::Input, "MyPort", "Number", "", true, "Custom Label"};
  EXPECT_EQ(port.label(), "Custom Label");
}

// ── Code Template ───────────────────────────────────────────────────────────

TEST(NodeTest, CodeTemplateDefaultEmpty) {
  Node node("T", "N");
  EXPECT_EQ(node.codeTemplate(), "");
}

TEST(NodeTest, SetCodeTemplate) {
  Node node("T", "N");
  node.setCodeTemplate("int {Result} = {A} + {B};");
  EXPECT_EQ(node.codeTemplate(), "int {Result} = {A} + {B};");
}

// ── HasValue / Value ────────────────────────────────────────────────────────

TEST(NodeTest, HasValueDefaultFalse) {
  Node node("T", "N");
  EXPECT_FALSE(node.hasValue());
}

TEST(NodeTest, SetHasValue) {
  Node node("T", "N");
  node.setHasValue(true);
  EXPECT_TRUE(node.hasValue());
}

TEST(NodeTest, ValueDefaultEmpty) {
  Node node("T", "N");
  EXPECT_EQ(node.value(), "");
}

TEST(NodeTest, SetValue) {
  Node node("T", "N");
  node.setValue("Hello World");
  EXPECT_EQ(node.value(), "Hello World");
}

// ── Language ────────────────────────────────────────────────────────────────

TEST(NodeTest, LanguageDefaultEmpty) {
  Node node("T", "N");
  EXPECT_EQ(node.language(), "");
}

TEST(NodeTest, SetLanguage) {
  Node node("T", "N");
  node.setLanguage("Python");
  EXPECT_EQ(node.language(), "Python");
}

// ── Collapsed ───────────────────────────────────────────────────────────────

TEST(NodeTest, CollapsedDefaultFalse) {
  Node node("T", "N");
  EXPECT_FALSE(node.collapsed());
}

TEST(NodeTest, SetCollapsed) {
  Node node("T", "N");
  node.setCollapsed(true);
  EXPECT_TRUE(node.collapsed());
}

TEST(NodeTest, ToggleCollapsed) {
  Node node("T", "N");
  node.setCollapsed(true);
  EXPECT_TRUE(node.collapsed());
  node.setCollapsed(false);
  EXPECT_FALSE(node.collapsed());
}

// ── Breakpoint ──────────────────────────────────────────────────────────────

TEST(NodeTest, BreakpointDefaultFalse) {
  Node node("T", "N");
  EXPECT_FALSE(node.hasBreakpoint());
}

TEST(NodeTest, SetBreakpoint) {
  Node node("T", "N");
  node.setBreakpoint(true);
  EXPECT_TRUE(node.hasBreakpoint());
}

TEST(NodeTest, ToggleBreakpoint) {
  Node node("T", "N");
  node.setBreakpoint(true);
  EXPECT_TRUE(node.hasBreakpoint());
  node.setBreakpoint(false);
  EXPECT_FALSE(node.hasBreakpoint());
}
