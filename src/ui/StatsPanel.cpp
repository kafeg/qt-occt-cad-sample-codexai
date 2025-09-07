#include "StatsPanel.h"

#include <OcctQOpenGLWidgetViewer.h>

#include <Standard_WarningsDisable.hxx>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QLocale>
#include <Standard_WarningsRestore.hxx>

StatsPanel::StatsPanel(QWidget* parent)
  : QWidget(parent)
{
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(8, 8, 8, 8);
  lay->setSpacing(6);

  auto* grp = new QGroupBox(QStringLiteral("Viewer Stats"), this);
  auto* v = new QVBoxLayout(grp);
  m_lblFps = new QLabel(QStringLiteral("FPS: —"), grp);
  m_lblTriangles = new QLabel(QStringLiteral("Triangles: —"), grp);
  m_lblZoom = new QLabel(QStringLiteral("Zoom (scale): —"), grp);
  v->addWidget(m_lblFps);
  v->addWidget(m_lblTriangles);
  v->addWidget(m_lblZoom);
  v->addStretch(1);
  lay->addWidget(grp);
  lay->addStretch(1);

  m_timer.setInterval(500);
  connect(&m_timer, &QTimer::timeout, this, &StatsPanel::refresh);
}

void StatsPanel::attachViewer(OcctQOpenGLWidgetViewer* v)
{
  m_viewer = v;
  if (m_viewer) m_timer.start(); else m_timer.stop();
}

void StatsPanel::refresh()
{
  if (!m_viewer) return;
  const double fps = m_viewer->currentFps();
  const int tri = m_viewer->currentTriangles();
  const double sc = m_viewer->currentScale();
  QLocale loc;
  m_lblFps->setText(QStringLiteral("FPS: ") + (fps > 0.0 ? loc.toString(fps, 'f', 1) : QStringLiteral("—")));
  m_lblTriangles->setText(QStringLiteral("Triangles: ") + (tri >= 0 ? loc.toString(tri) : QStringLiteral("—")));
  m_lblZoom->setText(QStringLiteral("Zoom (scale): ") + (sc > 0.0 ? loc.toString(sc, 'f', 3) : QStringLiteral("—")));
}
