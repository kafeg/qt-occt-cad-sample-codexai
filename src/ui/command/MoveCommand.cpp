#include "MoveCommand.h"

#include <Document.h>
#include <MoveFeature.h>

void MoveCommand::execute(Document& doc)
{
  Handle(MoveFeature) mf = new MoveFeature();
  mf->setSourceId(m_sourceId);
  mf->setTranslation(m_tx, m_ty, m_tz);
  mf->setRotation(m_rx, m_ry, m_rz);
  doc.addFeature(mf);
  doc.recompute();
}

