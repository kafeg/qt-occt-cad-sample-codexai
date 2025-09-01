#pragma once

#include "AbstractCommand.h"
#include <DocumentItem.h>

// Command: add a MoveFeature to the document with given source and transform params
class MoveCommand : public AbstractCommand
{
public:
  MoveCommand(DocumentItem::Id sourceId,
              double tx, double ty, double tz,
              double rxDeg = 0.0, double ryDeg = 0.0, double rzDeg = 0.0)
    : m_sourceId(sourceId), m_tx(tx), m_ty(ty), m_tz(tz), m_rx(rxDeg), m_ry(ryDeg), m_rz(rzDeg) {}

  void execute(Document& doc) override;

private:
  DocumentItem::Id m_sourceId{0};
  double m_tx{0}, m_ty{0}, m_tz{0};
  double m_rx{0}, m_ry{0}, m_rz{0};
};

