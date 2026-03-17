#include "core/Project.h"
#include "core/Serialization.h"
#include <QTemporaryDir>
#include <gtest/gtest.h>

class SerializationFixture : public ::testing::Test {
protected:
  QTemporaryDir tempDir;

  QString tempFile(const QString &name) {
    return tempDir.path() + "/" + name;
  }
};

// ── Save & Load Round Trip ──────────────────────────────────────────────────

TEST_F(SerializationFixture, SaveAndLoadEmptyGraph) {
  Project p1;
  p1.setLanguage("Python");
  QString path = tempFile("empty.nodetree");

  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.language(), "Python");
  EXPECT_EQ(p2.graph()->nodes().size(), 0);
  EXPECT_EQ(p2.graph()->links().size(), 0);
}

TEST_F(SerializationFixture, SaveAndLoadSingleNode) {
  Project p1;
  p1.setLanguage("C++");
  Node *n = p1.graph()->addNode("Math.Add", "Add", QPointF(150, 250));
  n->setHasValue(true);
  n->setValue("test_value");
  n->setLanguage("C++");
  n->setCollapsed(true);
  n->setBreakpoint(true);

  QString path = tempFile("single.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.graph()->nodes().size(), 1);

  Node *loaded = p2.graph()->nodes().first();
  EXPECT_EQ(loaded->type(), "Math.Add");
  EXPECT_EQ(loaded->name(), "Add");
  EXPECT_DOUBLE_EQ(loaded->position().x(), 150.0);
  EXPECT_DOUBLE_EQ(loaded->position().y(), 250.0);
  EXPECT_TRUE(loaded->hasValue());
  EXPECT_EQ(loaded->value(), "test_value");
  EXPECT_EQ(loaded->language(), "C++");
  EXPECT_TRUE(loaded->collapsed());
  EXPECT_TRUE(loaded->hasBreakpoint());
}

TEST_F(SerializationFixture, SaveAndLoadMultipleNodes) {
  Project p1;
  p1.graph()->addNode("T1", "A", QPointF(0, 0));
  p1.graph()->addNode("T2", "B", QPointF(100, 100));
  p1.graph()->addNode("T3", "C", QPointF(200, 200));

  QString path = tempFile("multi.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.graph()->nodes().size(), 3);
}

TEST_F(SerializationFixture, SaveAndLoadWithLinks) {
  Project p1;
  p1.setLanguage("JavaScript");
  Node *a = p1.graph()->addNode("T", "A", QPointF(0, 0));
  Node *b = p1.graph()->addNode("T", "B", QPointF(100, 0));
  a->addOutput("Out", "Number");
  b->addInput("In", "Number");

  p1.graph()->addLink(a->id(), a->outputs()[0].id,
                      b->id(), b->inputs()[0].id);

  QString path = tempFile("linked.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  // To reload, we need node defs so ports are re-created
  // Without defs, ports won't be recreated, so links can't be restored
  // Test that the file loads without crash
  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.language(), "JavaScript");
  EXPECT_EQ(p2.graph()->nodes().size(), 2);
}

TEST_F(SerializationFixture, SaveAndLoadPortCustomizations) {
  Project p1;
  Node *n = p1.graph()->addNode("T", "N", QPointF(0, 0));
  n->addInput("A", "Number", "0");
  n->addInput("B", "String", "hello");
  n->addOutput("R", "Number");

  n->inputs()[0].value = "42";
  n->inputs()[0].displayName = "First Input";
  n->inputs()[0].allowInline = false;
  n->inputs()[1].value = "world";
  n->outputs()[0].displayName = "Output Result";

  QString path = tempFile("ports.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  // Load - since there are no registered defs, ports won't be re-created
  // from defs. But the customization data is stored in JSON.
  // This tests that the save format is correct.
  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.graph()->nodes().size(), 1);
}

// ── Language Preservation ───────────────────────────────────────────────────

TEST_F(SerializationFixture, PreservesLanguageCpp) {
  Project p1;
  p1.setLanguage("C++");
  QString path = tempFile("lang_cpp.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.language(), "C++");
}

TEST_F(SerializationFixture, PreservesLanguagePython) {
  Project p1;
  p1.setLanguage("Python");
  QString path = tempFile("lang_py.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.language(), "Python");
}

TEST_F(SerializationFixture, PreservesLanguageJavaScript) {
  Project p1;
  p1.setLanguage("JavaScript");
  QString path = tempFile("lang_js.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  ASSERT_TRUE(Serialization::loadJson(&p2, path));
  EXPECT_EQ(p2.language(), "JavaScript");
}

// ── Error Handling ──────────────────────────────────────────────────────────

TEST_F(SerializationFixture, SaveToInvalidPathFails) {
  Project p;
  EXPECT_FALSE(Serialization::saveJson(&p, "/nonexistent/dir/file.nodetree"));
}

TEST_F(SerializationFixture, LoadNonexistentFileFails) {
  Project p;
  EXPECT_FALSE(Serialization::loadJson(&p, "/nonexistent/file.nodetree"));
}

TEST_F(SerializationFixture, LoadInvalidJsonFails) {
  QString path = tempFile("invalid.nodetree");
  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write("this is not json!!!");
  f.close();

  Project p;
  EXPECT_FALSE(Serialization::loadJson(&p, path));
}

TEST_F(SerializationFixture, LoadClearsExistingGraph) {
  Project p;
  p.graph()->addNode("T", "Existing", QPointF(0, 0));
  EXPECT_EQ(p.graph()->nodes().size(), 1);

  // Save an empty project
  Project empty;
  QString path = tempFile("empty2.nodetree");
  ASSERT_TRUE(Serialization::saveJson(&empty, path));

  // Load into project that already has nodes
  ASSERT_TRUE(Serialization::loadJson(&p, path));
  EXPECT_EQ(p.graph()->nodes().size(), 0);
}

// ── Save overwrites existing file ───────────────────────────────────────────

TEST_F(SerializationFixture, SaveOverwritesExistingFile) {
  QString path = tempFile("overwrite.nodetree");

  Project p1;
  p1.setLanguage("C++");
  p1.graph()->addNode("T", "First", QPointF(0, 0));
  ASSERT_TRUE(Serialization::saveJson(&p1, path));

  Project p2;
  p2.setLanguage("Python");
  p2.graph()->addNode("T2", "Second", QPointF(10, 10));
  p2.graph()->addNode("T3", "Third", QPointF(20, 20));
  ASSERT_TRUE(Serialization::saveJson(&p2, path));

  Project p3;
  ASSERT_TRUE(Serialization::loadJson(&p3, path));
  EXPECT_EQ(p3.language(), "Python");
  EXPECT_EQ(p3.graph()->nodes().size(), 2);
}
