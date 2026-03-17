#include "core/Project.h"
#include <QTemporaryDir>
#include <gtest/gtest.h>

// ── Default State ───────────────────────────────────────────────────────────

TEST(ProjectTest, DefaultLanguageIsCpp) {
  Project p;
  EXPECT_EQ(p.language(), "C++");
}

TEST(ProjectTest, GraphIsNotNull) {
  Project p;
  EXPECT_NE(p.graph(), nullptr);
}

TEST(ProjectTest, GraphStartsEmpty) {
  Project p;
  EXPECT_EQ(p.graph()->nodes().size(), 0);
  EXPECT_EQ(p.graph()->links().size(), 0);
}

TEST(ProjectTest, DefaultFilePathEmpty) {
  Project p;
  EXPECT_EQ(p.currentFilePath(), "");
}

TEST(ProjectTest, DefaultNodeDefsEmpty) {
  Project p;
  EXPECT_TRUE(p.nodeDefinitions().isEmpty());
}

// ── Language ────────────────────────────────────────────────────────────────

TEST(ProjectTest, SetLanguagePython) {
  Project p;
  p.setLanguage("Python");
  EXPECT_EQ(p.language(), "Python");
}

TEST(ProjectTest, SetLanguageJavaScript) {
  Project p;
  p.setLanguage("JavaScript");
  EXPECT_EQ(p.language(), "JavaScript");
}

TEST(ProjectTest, SetLanguageSwitchBack) {
  Project p;
  p.setLanguage("Python");
  p.setLanguage("C++");
  EXPECT_EQ(p.language(), "C++");
}

// ── Current File Path ───────────────────────────────────────────────────────

TEST(ProjectTest, SetCurrentFilePath) {
  Project p;
  p.setCurrentFilePath("/tmp/test.nodetree");
  EXPECT_EQ(p.currentFilePath(), "/tmp/test.nodetree");
}

// ── Save / Load Node Tree ───────────────────────────────────────────────────

TEST(ProjectTest, SaveNodeTreeSetsCurrentPath) {
  QTemporaryDir tempDir;
  Project p;
  p.graph()->addNode("T", "N", QPointF(0, 0));
  QString path = tempDir.path() + "/test.nodetree";

  ASSERT_TRUE(p.saveNodeTree(path));
  EXPECT_EQ(p.currentFilePath(), path);
}

TEST(ProjectTest, LoadNodeTreeSetsCurrentPath) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/test.nodetree";

  Project p1;
  p1.graph()->addNode("T", "N", QPointF(0, 0));
  ASSERT_TRUE(p1.saveNodeTree(path));

  Project p2;
  ASSERT_TRUE(p2.loadNodeTree(path));
  EXPECT_EQ(p2.currentFilePath(), path);
}

TEST(ProjectTest, SaveLoadRoundTrip) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/roundtrip.nodetree";

  Project p1;
  p1.setLanguage("Python");
  p1.graph()->addNode("T1", "Alpha", QPointF(10, 20));
  p1.graph()->addNode("T2", "Beta", QPointF(30, 40));
  ASSERT_TRUE(p1.saveNodeTree(path));

  Project p2;
  ASSERT_TRUE(p2.loadNodeTree(path));
  EXPECT_EQ(p2.language(), "Python");
  EXPECT_EQ(p2.graph()->nodes().size(), 2);
}

TEST(ProjectTest, SaveToInvalidPathFails) {
  Project p;
  EXPECT_FALSE(p.saveNodeTree("/nonexistent/dir/fail.nodetree"));
}

TEST(ProjectTest, LoadNonexistentFileFails) {
  Project p;
  EXPECT_FALSE(p.loadNodeTree("/nonexistent/file.nodetree"));
}

// ── Load Node Definitions ───────────────────────────────────────────────────

TEST(ProjectTest, LoadNodeDefinitionsFromFile) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/defs.node";

  // Write a valid node definitions file
  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write(R"({
    "nodes": [
      {
        "type": "Math.Add",
        "name": "Add",
        "inputs": [
          {"name": "A", "type": "Number", "value": "0"},
          {"name": "B", "type": "Number", "value": "0"}
        ],
        "outputs": [
          {"name": "Result", "type": "Number"}
        ],
        "code": "int {Result} = {A} + {B};",
        "language": "C++"
      },
      {
        "type": "Math.Subtract",
        "name": "Subtract",
        "inputs": [
          {"name": "A", "type": "Number", "value": "0"},
          {"name": "B", "type": "Number", "value": "0"}
        ],
        "outputs": [
          {"name": "Result", "type": "Number"}
        ],
        "code": "int {Result} = {A} - {B};",
        "language": "C++"
      }
    ]
  })");
  f.close();

  Project p;
  ASSERT_TRUE(p.loadNodeDefinitions(path));

  auto defs = p.nodeDefinitions();
  EXPECT_EQ(defs.size(), 2);
  EXPECT_TRUE(defs.contains("Math.Add"));
  EXPECT_TRUE(defs.contains("Math.Subtract"));

  // Verify the Add definition
  auto addDef = defs["Math.Add"];
  EXPECT_EQ(addDef.name, "Add");
  EXPECT_EQ(addDef.inputs.size(), 2);
  EXPECT_EQ(addDef.outputs.size(), 1);
  EXPECT_EQ(addDef.inputs[0].name, "A");
  EXPECT_EQ(addDef.inputs[0].type, "Number");
  EXPECT_EQ(addDef.inputs[0].defaultValue, "0");
  EXPECT_EQ(addDef.outputs[0].name, "Result");
  EXPECT_EQ(addDef.code, "int {Result} = {A} + {B};");
  EXPECT_EQ(addDef.language, "C++");
}

TEST(ProjectTest, LoadNodeDefinitionsWithRequires) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/defs_req.node";

  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write(R"({
    "nodes": [
      {
        "type": "IO.Print",
        "name": "Print",
        "inputs": [
          {"name": "Text", "type": "String"}
        ],
        "outputs": [],
        "code": "std::cout << {Text};",
        "language": "C++",
        "requires": ["<iostream>"]
      }
    ]
  })");
  f.close();

  Project p;
  ASSERT_TRUE(p.loadNodeDefinitions(path));
  auto def = p.nodeDefinitions()["IO.Print"];
  EXPECT_EQ(def.requires.size(), 1);
  EXPECT_EQ(def.requires[0], "<iostream>");
}

TEST(ProjectTest, LoadNodeDefinitionsWithHasValue) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/defs_val.node";

  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write(R"({
    "nodes": [
      {
        "type": "Data.Constant",
        "name": "Constant",
        "inputs": [],
        "outputs": [{"name": "Value", "type": "Number"}],
        "code": "auto {Value} = {VALUE};",
        "hasValue": true,
        "value": "0",
        "language": "C++"
      }
    ]
  })");
  f.close();

  Project p;
  ASSERT_TRUE(p.loadNodeDefinitions(path));
  auto def = p.nodeDefinitions()["Data.Constant"];
  EXPECT_TRUE(def.hasValue);
  EXPECT_EQ(def.value, "0");
}

TEST(ProjectTest, LoadNodeDefinitionsWithAllowInline) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/defs_inline.node";

  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write(R"({
    "nodes": [
      {
        "type": "Test.Node",
        "name": "TestNode",
        "inputs": [
          {"name": "A", "type": "Number", "allowInline": false}
        ],
        "outputs": [],
        "code": "test({A});",
        "language": "C++"
      }
    ]
  })");
  f.close();

  Project p;
  ASSERT_TRUE(p.loadNodeDefinitions(path));
  auto def = p.nodeDefinitions()["Test.Node"];
  EXPECT_FALSE(def.inputs[0].allowInline);
}

TEST(ProjectTest, LoadNodeDefinitionsInvalidFileFails) {
  Project p;
  EXPECT_FALSE(p.loadNodeDefinitions("/nonexistent/defs.node"));
}

TEST(ProjectTest, LoadNodeDefinitionsInvalidJsonFails) {
  QTemporaryDir tempDir;
  QString path = tempDir.path() + "/bad.node";
  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write("not json at all");
  f.close();

  Project p;
  EXPECT_FALSE(p.loadNodeDefinitions(path));
}

TEST(ProjectTest, LoadNodeDefinitionsAccumulates) {
  QTemporaryDir tempDir;

  // First file
  QString path1 = tempDir.path() + "/defs1.node";
  QFile f1(path1);
  f1.open(QIODevice::WriteOnly);
  f1.write(R"({"nodes": [{"type": "A.Node", "name": "A"}]})");
  f1.close();

  // Second file
  QString path2 = tempDir.path() + "/defs2.node";
  QFile f2(path2);
  f2.open(QIODevice::WriteOnly);
  f2.write(R"({"nodes": [{"type": "B.Node", "name": "B"}]})");
  f2.close();

  Project p;
  ASSERT_TRUE(p.loadNodeDefinitions(path1));
  ASSERT_TRUE(p.loadNodeDefinitions(path2));
  EXPECT_EQ(p.nodeDefinitions().size(), 2);
  EXPECT_TRUE(p.nodeDefinitions().contains("A.Node"));
  EXPECT_TRUE(p.nodeDefinitions().contains("B.Node"));
}

// ── Recent Files ────────────────────────────────────────────────────────────

TEST(ProjectTest, AddRecentFileAppearsInList) {
  Project p;
  p.addRecentFile("/tmp/test_recent_unique_12345.nodetree");
  QStringList recent = p.recentFiles();
  EXPECT_TRUE(recent.contains("/tmp/test_recent_unique_12345.nodetree"));
}

TEST(ProjectTest, RecentFilesMostRecentFirst) {
  Project p;
  p.addRecentFile("/tmp/recent_a_unique.nodetree");
  p.addRecentFile("/tmp/recent_b_unique.nodetree");
  QStringList recent = p.recentFiles();
  ASSERT_GE(recent.size(), 2);
  // Most recently added should be first
  int idxA = recent.indexOf("/tmp/recent_a_unique.nodetree");
  int idxB = recent.indexOf("/tmp/recent_b_unique.nodetree");
  EXPECT_LT(idxB, idxA);
}

TEST(ProjectTest, RecentFilesNoDuplicates) {
  Project p;
  p.addRecentFile("/tmp/recent_dedup_unique.nodetree");
  p.addRecentFile("/tmp/recent_dedup_unique.nodetree");
  QStringList recent = p.recentFiles();
  int count = recent.count("/tmp/recent_dedup_unique.nodetree");
  EXPECT_EQ(count, 1);
}

TEST(ProjectTest, RecentFilesMaxTen) {
  Project p;
  for (int i = 0; i < 15; i++) {
    p.addRecentFile("/tmp/recent_max_" + QString::number(i) + "_unique.nodetree");
  }
  QStringList recent = p.recentFiles();
  EXPECT_LE(recent.size(), 10);
}
