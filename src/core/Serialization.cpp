#include "Serialization.h"
#include "Project.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace Serialization {

bool saveJson(Project *project, const QString &filename) {
  nlohmann::json j;
  j["version"] = 2;
  j["language"] = project->language().toStdString();

  auto &jnodes = j["nodes"];
  jnodes = nlohmann::json::array();
  for (const auto *node : project->graph()->nodes()) {
    nlohmann::json jn;
    jn["id"] = node->id().toString().toStdString();
    jn["type"] = node->type().toStdString();
    jn["name"] = node->name().toStdString();
    jn["x"] = node->position().x();
    jn["y"] = node->position().y();
    jn["hasValue"] = node->hasValue();
    jn["value"] = node->value().toStdString();
    jn["language"] = node->language().toStdString();
    jn["collapsed"] = node->collapsed();
    jn["breakpoint"] = node->hasBreakpoint();

    auto &jinputs = jn["inputs"];
    jinputs = nlohmann::json::array();
    for (const auto &p : node->inputs()) {
      jinputs.push_back({
        {"name", p.name.toStdString()},
        {"value", p.value.toStdString()},
        {"displayName", p.displayName.toStdString()},
        {"allowInline", p.allowInline}
      });
    }
    auto &joutputs = jn["outputs"];
    joutputs = nlohmann::json::array();
    for (const auto &p : node->outputs()) {
      joutputs.push_back({
        {"name", p.name.toStdString()},
        {"displayName", p.displayName.toStdString()},
        {"allowInline", p.allowInline}
      });
    }
    jnodes.push_back(jn);
  }

  auto &jlinks = j["links"];
  jlinks = nlohmann::json::array();
  for (const auto *link : project->graph()->links()) {
    // Find port names for stable references
    Node *srcNode = project->graph()->getNode(link->sourceNodeId);
    Node *tgtNode = project->graph()->getNode(link->targetNodeId);
    QString srcPortName, tgtPortName;
    if (srcNode) {
      for (const auto &p : srcNode->outputs())
        if (p.id == link->sourcePortId) { srcPortName = p.name; break; }
    }
    if (tgtNode) {
      for (const auto &p : tgtNode->inputs())
        if (p.id == link->targetPortId) { tgtPortName = p.name; break; }
    }

    jlinks.push_back({
      {"sourceNode", link->sourceNodeId.toString().toStdString()},
      {"sourcePort", srcPortName.toStdString()},
      {"targetNode", link->targetNodeId.toString().toStdString()},
      {"targetPort", tgtPortName.toStdString()}
    });
  }

  std::ofstream file(filename.toStdString());
  if (!file.is_open()) return false;
  file << j.dump(2);
  return true;
}

bool loadJson(Project *project, const QString &filename) {
  std::ifstream file(filename.toStdString());
  if (!file.is_open()) return false;

  try {
    nlohmann::json j;
    file >> j;

    project->graph()->clear();
    project->setLanguage(QString::fromStdString(j.value("language", "C++")));

    QMap<QString, Node*> idToNode; // saved ID string -> new Node*

    for (const auto &jn : j["nodes"]) {
      QString savedId = QString::fromStdString(jn.value("id", ""));
      QString type = QString::fromStdString(jn.value("type", ""));
      QString name = QString::fromStdString(jn.value("name", ""));
      QPointF pos(jn.value("x", 0.0), jn.value("y", 0.0));

      Node *node = project->graph()->addNode(type, name, pos);
      node->setHasValue(jn.value("hasValue", false));
      node->setValue(QString::fromStdString(jn.value("value", "")));
      node->setLanguage(QString::fromStdString(jn.value("language", "")));
      node->setCollapsed(jn.value("collapsed", false));
      node->setBreakpoint(jn.value("breakpoint", false));

      // Re-apply node definition
      auto defs = project->nodeDefinitions();
      if (defs.contains(type)) {
        const auto &def = defs[type];
        for (const auto &pd : def.inputs)
          node->addInput(pd.name, pd.type, pd.defaultValue, pd.allowInline);
        for (const auto &pd : def.outputs)
          node->addOutput(pd.name, pd.type, pd.defaultValue, pd.allowInline);
        node->setCodeTemplate(def.code);
      }

      // Restore port customizations
      if (jn.contains("inputs")) {
        int idx = 0;
        for (const auto &jp : jn["inputs"]) {
          if (idx < node->inputs().size()) {
            node->inputs()[idx].value = QString::fromStdString(jp.value("value", ""));
            node->inputs()[idx].displayName = QString::fromStdString(jp.value("displayName", ""));
            node->inputs()[idx].allowInline = jp.value("allowInline", true);
          }
          idx++;
        }
      }
      if (jn.contains("outputs")) {
        int idx = 0;
        for (const auto &jp : jn["outputs"]) {
          if (idx < node->outputs().size()) {
            node->outputs()[idx].displayName = QString::fromStdString(jp.value("displayName", ""));
            node->outputs()[idx].allowInline = jp.value("allowInline", false);
          }
          idx++;
        }
      }

      idToNode[savedId] = node;
    }

    // Restore links using port names (stable across sessions)
    for (const auto &jl : j["links"]) {
      QString srcNodeId = QString::fromStdString(jl.value("sourceNode", ""));
      QString srcPortName = QString::fromStdString(jl.value("sourcePort", ""));
      QString tgtNodeId = QString::fromStdString(jl.value("targetNode", ""));
      QString tgtPortName = QString::fromStdString(jl.value("targetPort", ""));

      Node *srcNode = idToNode.value(srcNodeId);
      Node *tgtNode = idToNode.value(tgtNodeId);
      if (!srcNode || !tgtNode) continue;

      QUuid srcPortId, tgtPortId;
      for (const auto &p : srcNode->outputs())
        if (p.name == srcPortName) { srcPortId = p.id; break; }
      for (const auto &p : tgtNode->inputs())
        if (p.name == tgtPortName) { tgtPortId = p.id; break; }

      if (!srcPortId.isNull() && !tgtPortId.isNull())
        project->graph()->addLink(srcNode->id(), srcPortId, tgtNode->id(), tgtPortId);
    }

  } catch (...) {
    return false;
  }
  return true;
}

} // namespace Serialization
