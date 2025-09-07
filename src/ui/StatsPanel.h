#pragma once

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <QTimer>
#include <Standard_WarningsRestore.hxx>

class QLabel;
class OcctQOpenGLWidgetViewer;

// Simple side panel to display viewer statistics (FPS, triangles) and camera zoom
class StatsPanel : public QWidget
{
  Q_OBJECT
public:
  explicit StatsPanel(QWidget* parent = nullptr);

  void attachViewer(OcctQOpenGLWidgetViewer* v);

private slots:
  void refresh();

private:
  QLabel* m_lblFps = nullptr;
  QLabel* m_lblTriangles = nullptr;
  QLabel* m_lblZoom = nullptr;
  QTimer  m_timer;
  OcctQOpenGLWidgetViewer* m_viewer = nullptr;
};
