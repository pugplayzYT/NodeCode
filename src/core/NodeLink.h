#pragma once
#include <QUuid>

struct NodeLink {
  QUuid id;
  QUuid sourceNodeId;
  QUuid sourcePortId;
  QUuid targetNodeId;
  QUuid targetPortId;
};
