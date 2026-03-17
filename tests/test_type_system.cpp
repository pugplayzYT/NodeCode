#include "core/TypeSystem.h"
#include <gtest/gtest.h>

// ── Port Colors ─────────────────────────────────────────────────────────────

TEST(TypeSystemTest, ExecPortColor) {
  QColor c = TypeSystem::portColor("Exec");
  EXPECT_EQ(c, QColor(220, 220, 220));
}

TEST(TypeSystemTest, StringPortColor) {
  QColor c = TypeSystem::portColor("String");
  EXPECT_EQ(c, QColor(76, 175, 80));
}

TEST(TypeSystemTest, NumberPortColor) {
  QColor c = TypeSystem::portColor("Number");
  EXPECT_EQ(c, QColor(33, 150, 243));
}

TEST(TypeSystemTest, BooleanPortColor) {
  QColor c = TypeSystem::portColor("Boolean");
  EXPECT_EQ(c, QColor(244, 67, 54));
}

TEST(TypeSystemTest, AnyPortColor) {
  QColor c = TypeSystem::portColor("Any");
  EXPECT_EQ(c, QColor(158, 158, 158));
}

TEST(TypeSystemTest, UnknownTypeReturnsGray) {
  QColor c = TypeSystem::portColor("SomeRandomType");
  EXPECT_EQ(c, QColor(158, 158, 158));
}

TEST(TypeSystemTest, EmptyTypeReturnsGray) {
  QColor c = TypeSystem::portColor("");
  EXPECT_EQ(c, QColor(158, 158, 158));
}

// ── Link Colors ─────────────────────────────────────────────────────────────

TEST(TypeSystemTest, ExecLinkColor) {
  QColor c = TypeSystem::linkColor("Exec");
  EXPECT_EQ(c, QColor(220, 220, 220));
}

TEST(TypeSystemTest, NonExecLinkColorMatchesPortColor) {
  EXPECT_EQ(TypeSystem::linkColor("String"), TypeSystem::portColor("String"));
  EXPECT_EQ(TypeSystem::linkColor("Number"), TypeSystem::portColor("Number"));
  EXPECT_EQ(TypeSystem::linkColor("Boolean"), TypeSystem::portColor("Boolean"));
  EXPECT_EQ(TypeSystem::linkColor("Any"), TypeSystem::portColor("Any"));
}

// ── Type Compatibility ──────────────────────────────────────────────────────

TEST(TypeSystemTest, SameTypesAreCompatible) {
  EXPECT_TRUE(TypeSystem::areCompatible("Number", "Number"));
  EXPECT_TRUE(TypeSystem::areCompatible("String", "String"));
  EXPECT_TRUE(TypeSystem::areCompatible("Boolean", "Boolean"));
  EXPECT_TRUE(TypeSystem::areCompatible("Exec", "Exec"));
}

TEST(TypeSystemTest, AnySourceIsCompatibleWithAll) {
  EXPECT_TRUE(TypeSystem::areCompatible("Any", "Number"));
  EXPECT_TRUE(TypeSystem::areCompatible("Any", "String"));
  EXPECT_TRUE(TypeSystem::areCompatible("Any", "Boolean"));
  EXPECT_TRUE(TypeSystem::areCompatible("Any", "Exec"));
}

TEST(TypeSystemTest, AnyTargetIsCompatibleWithAll) {
  EXPECT_TRUE(TypeSystem::areCompatible("Number", "Any"));
  EXPECT_TRUE(TypeSystem::areCompatible("String", "Any"));
  EXPECT_TRUE(TypeSystem::areCompatible("Boolean", "Any"));
  EXPECT_TRUE(TypeSystem::areCompatible("Exec", "Any"));
}

TEST(TypeSystemTest, AnyToAnyIsCompatible) {
  EXPECT_TRUE(TypeSystem::areCompatible("Any", "Any"));
}

TEST(TypeSystemTest, DifferentTypesAreIncompatible) {
  EXPECT_FALSE(TypeSystem::areCompatible("Number", "String"));
  EXPECT_FALSE(TypeSystem::areCompatible("String", "Boolean"));
  EXPECT_FALSE(TypeSystem::areCompatible("Boolean", "Number"));
  EXPECT_FALSE(TypeSystem::areCompatible("Exec", "String"));
  EXPECT_FALSE(TypeSystem::areCompatible("Number", "Exec"));
}

TEST(TypeSystemTest, CaseSensitivity) {
  // Type names are case-sensitive
  EXPECT_FALSE(TypeSystem::areCompatible("number", "Number"));
  EXPECT_FALSE(TypeSystem::areCompatible("string", "String"));
}
