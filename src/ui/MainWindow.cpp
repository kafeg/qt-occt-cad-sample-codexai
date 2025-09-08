#include "MainWindow.h"

#include <OcctQOpenGLWidgetViewer.h>

#include <Standard_WarningsDisable.hxx>
#include <QAction>
#include <QTabWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <Standard_WarningsRestore.hxx>

#include <Quantity_Color.hxx>
#include <Standard_Version.hxx>
#include <NCollection_Sequence.hxx>

#include <Document.h>
#include <command/CreateBoxCommand.h>
#include <dialog/CreateBoxDialog.h>
#include <command/CreateCylinderCommand.h>
#include <dialog/CreateCylinderDialog.h>
#include <command/CreateExtrudeCommand.h>
#include <command/CreateSketchCommand.h>
#include <dialog/CreateExtrudeDialog.h>
#include <dialog/CreateSketchDialog.h>
#include <dialog/SettingsDialog.h>
// sketch for extrusion
#include <Sketch.h>
// model/viewer headers for sync helpers
#include <Feature.h>
#include <PlaneFeature.h>
#include "TabPageWidget.h"

MainWindow::MainWindow()
{
  m_tabs = new QTabWidget(this);
  m_tabs->setTabsClosable(true);
  connect(m_tabs, &QTabWidget::tabCloseRequested, [this](int index) {
    QWidget* w = m_tabs->widget(index);
    m_tabs->removeTab(index);
    delete w;
    if (m_tabs->count() == 0) {
      addNewTab();
    }
  });
  setCentralWidget(m_tabs);
  addNewTab();
  createMenuBar();
  createToolBar();
  // React to tab switches to update action states and reconnect viewer signals
  connect(m_tabs, &QTabWidget::currentChanged, [this](int){ connectViewerSignals(); updateActionStates(); });
  connectViewerSignals();
  updateActionStates();
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenuBar()
{
  QMenuBar* mbar = new QMenuBar();
  QMenu*    file = mbar->addMenu("&File");
  {
    QAction* newTab = new QAction(file);
    newTab->setText("New Tab");
    newTab->setObjectName("actionNewTab");
    newTab->setShortcuts({ QKeySequence(QStringLiteral("Ctrl+T")), QKeySequence(QStringLiteral("Meta+T")) });
    file->addAction(newTab);
    connect(newTab, &QAction::triggered, [this]() { addNewTab(); });
  }
  {
    QAction* closeTab = new QAction(file);
    closeTab->setText("Close Tab");
    closeTab->setObjectName("actionCloseTab");
    closeTab->setShortcuts({ QKeySequence(QStringLiteral("Ctrl+W")), QKeySequence(QStringLiteral("Meta+W")) });
    file->addAction(closeTab);
    connect(closeTab, &QAction::triggered, [this]() {
      const int idx = m_tabs->currentIndex();
      if (idx >= 0) emit m_tabs->tabCloseRequested(idx);
    });
  }
  {
    QAction* split = new QAction(file);
    split->setText("Split Views");
    file->addAction(split);
    // Toggle split view: enable/disable subview composer and manage subviews
    connect(split, &QAction::triggered, [this]() {
      TabPageWidget* page = currentPage(); if (!page) return;
      auto* viewer = page->viewer();
      if (!viewer->View()->Subviews().IsEmpty())
      {
        // Disable composer and remove subviews
        viewer->View()->View()->SetSubviewComposer(false);
        NCollection_Sequence<Handle(V3d_View)> aSubviews = viewer->View()->Subviews();
        for (const Handle(V3d_View)& aSubviewIter : aSubviews) aSubviewIter->Remove();
        viewer->OnSubviewChanged(viewer->Context(), nullptr, viewer->View());
      }
      else
      {
        // Enable composer and create two subviews side-by-side
        viewer->View()->View()->SetSubviewComposer(true);
        Handle(V3d_View) aSubView1 = new V3d_View(viewer->Viewer());
        aSubView1->SetImmediateUpdate(false);
        aSubView1->SetWindow(viewer->View(), Graphic3d_Vec2d(0.5, 1.0), Aspect_TOTP_LEFT_UPPER, Graphic3d_Vec2d(0.0, 0.0));
        Handle(V3d_View) aSubView2 = new V3d_View(viewer->Viewer());
        aSubView2->SetImmediateUpdate(false);
        aSubView2->SetWindow(viewer->View(), Graphic3d_Vec2d(0.5, 1.0), Aspect_TOTP_LEFT_UPPER, Graphic3d_Vec2d(0.5, 0.0));
        viewer->OnSubviewChanged(viewer->Context(), nullptr, aSubView1);
      }
      viewer->View()->Invalidate();
      viewer->update();
    });
  }
  {
    QAction* actAddBox = new QAction(file);
    actAddBox->setText("Add Box");
    file->addAction(actAddBox);
    connect(actAddBox, &QAction::triggered, [this]() { addBox(); });
  }
  {
    QAction* actAddCyl = new QAction(file);
    actAddCyl->setText("Add Cylinder");
    file->addAction(actAddCyl);
    connect(actAddCyl, &QAction::triggered, [this]() { addCylinder(); });
  }
  {
    QAction* actAddExtrude = new QAction(file);
    actAddExtrude->setText("Add Extrude");
    file->addAction(actAddExtrude);
    connect(actAddExtrude, &QAction::triggered, [this]() { addExtrude(); });
  }
  {
    QAction* actNewSketch = new QAction(file);
    actNewSketch->setText("New Sketch");
    file->addAction(actNewSketch);
    connect(actNewSketch, &QAction::triggered, [this]() { addSketch(); });
    // Complete Sketch action right next to New Sketch
    m_actCompleteSketch = new QAction(file);
    m_actCompleteSketch->setText("Complete Sketch");
    m_actCompleteSketch->setEnabled(false);
    file->addAction(m_actCompleteSketch);
    connect(m_actCompleteSketch, &QAction::triggered, [this]() {
      TabPageWidget* p = currentPage(); if (!p) return; auto* v = p->viewer(); if (!v) return;
      if (v->hasActiveSketchEdit()) v->clearSketchEditMode(true);
      updateActionStates();
    });
  }
  {
    QAction* actMove = new QAction(file);
    actMove->setText("Move");
    file->addAction(actMove);
    connect(actMove, &QAction::triggered, [this]() {
      TabPageWidget* p = currentPage(); if (!p) return;
      auto* v = p->viewer();
      if (v && v->isManipulatorActive()) p->confirmMove(); else p->activateMove();
      updateActionStates();
    });
    m_actCancelMove = new QAction(file);
    m_actCancelMove->setText("Cancel Move");
    m_actCancelMove->setEnabled(false);
    file->addAction(m_actCancelMove);
    connect(m_actCancelMove, &QAction::triggered, [this]() { TabPageWidget* p = currentPage(); if (p) p->cancelMove(); updateActionStates(); });
  }
  {
    QAction* quit = new QAction(file);
    quit->setText("Quit");
    file->addAction(quit);
    connect(quit, &QAction::triggered, [this]() { close(); });
  }
  // Settings menu
  QMenu* settingsMenu = mbar->addMenu("&Settings");
  {
    QAction* actSettings = new QAction(settingsMenu);
    actSettings->setText("Preferences...");
    settingsMenu->addAction(actSettings);
    connect(actSettings, &QAction::triggered, [this]() {
      SettingsDialog dlg(this);
      dlg.exec();
      // Panels auto-refresh via AppSettings signal
    });
  }
  setMenuBar(mbar);
}

void MainWindow::createToolBar()
{
  QToolBar* tb = new QToolBar("Main Toolbar", this);
  tb->setMovable(true);

  QAction* actAddBox = new QAction("Add Box", tb);
  connect(actAddBox, &QAction::triggered, [this]() { addBox(); });
  tb->addAction(actAddBox);

  QAction* actAddCyl = new QAction("Add Cylinder", tb);
  connect(actAddCyl, &QAction::triggered, [this]() { addCylinder(); });
  tb->addAction(actAddCyl);

  QAction* actAddExtrude = new QAction("Add Extrude", tb);
  connect(actAddExtrude, &QAction::triggered, [this]() { addExtrude(); });
  tb->addAction(actAddExtrude);

  QAction* actNewSketch = new QAction("New Sketch", tb);
  connect(actNewSketch, &QAction::triggered, [this]() { addSketch(); });
  tb->addAction(actNewSketch);
  // Complete Sketch next to New Sketch (reuse same QAction in menu and toolbar)
  if (!m_actCompleteSketch)
  {
    m_actCompleteSketch = new QAction("Complete Sketch", tb);
    m_actCompleteSketch->setEnabled(false);
    connect(m_actCompleteSketch, &QAction::triggered, [this]() {
      TabPageWidget* p = currentPage(); if (!p) return; auto* v = p->viewer(); if (!v) return;
      if (v->hasActiveSketchEdit()) v->clearSketchEditMode(true);
      updateActionStates();
    });
  }
  tb->addAction(m_actCompleteSketch);

  QAction* actMove = new QAction("Move", tb);
  connect(actMove, &QAction::triggered, [this]() {
    TabPageWidget* p = currentPage(); if (!p) return; auto* v = p->viewer();
    if (v && v->isManipulatorActive()) p->confirmMove(); else p->activateMove();
    updateActionStates();
  });
  tb->addAction(actMove);
  if (!m_actCancelMove)
  {
    m_actCancelMove = new QAction("Cancel Move", tb);
    m_actCancelMove->setEnabled(false);
    connect(m_actCancelMove, &QAction::triggered, [this]() { TabPageWidget* p = currentPage(); if (p) p->cancelMove(); updateActionStates(); });
  }
  tb->addAction(m_actCancelMove);

  QAction* actAbout = new QAction("About", tb);
  connect(actAbout, &QAction::triggered, [this]() {
    TabPageWidget* page = currentPage(); if (!page) return;
    auto* viewer = page->viewer();
    QMessageBox::information(0,
                             "About",
                             QString() + "Parametric CAD skeleton using OCCT + Qt6\n\n"
                               + "Open CASCADE Technology v." OCC_VERSION_STRING_EXT "\n"
                               + "Qt v." QT_VERSION_STR "\n\n" + "OpenGL info:\n" + viewer->getGlInfo());
  });
  tb->addAction(actAbout);
  addToolBar(Qt::TopToolBarArea, tb);
}

void MainWindow::updateActionStates()
{
  TabPageWidget* p = currentPage();
  OcctQOpenGLWidgetViewer* v = p ? p->viewer() : nullptr;
  const bool sketchActive = v && v->hasActiveSketchEdit();
  const bool moveActive   = v && v->isManipulatorActive();
  if (m_actCompleteSketch) m_actCompleteSketch->setEnabled(sketchActive);
  if (m_actCancelMove)     m_actCancelMove->setEnabled(moveActive);
}

void MainWindow::connectViewerSignals()
{
  TabPageWidget* p = currentPage(); if (!p) return;
  OcctQOpenGLWidgetViewer* v = p->viewer(); if (!v) return;
  // Disconnect previous connections by using QObject::connect with context 'this' and auto disconnection is fine on viewer destruction; no stored handles used here
  // Update on manipulator finish and sketch edit mode changes
  connect(v, &OcctQOpenGLWidgetViewer::manipulatorFinished, this, [this](const gp_Trsf&){ updateActionStates(); });
  connect(v, &OcctQOpenGLWidgetViewer::selectionChanged, this, [this](){ updateActionStates(); });
  connect(v, &OcctQOpenGLWidgetViewer::sketchEditModeChanged, this, [this](bool){ updateActionStates(); });
}

void MainWindow::addBox()
{
  TabPageWidget* page = currentPage(); if (!page) return;
  CreateBoxDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted) return;

  CreateBoxCommand cmd(dlg.dx(), dlg.dy(), dlg.dz());
  cmd.execute(page->doc());
  page->syncViewerFromDoc(true);
  page->refreshFeatureList();
}

void MainWindow::addCylinder()
{
  TabPageWidget* page = currentPage(); if (!page) return;
  CreateCylinderDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted) return;

  CreateCylinderCommand cmd(dlg.radius(), dlg.height());
  cmd.execute(page->doc());
  page->syncViewerFromDoc(true);
  page->refreshFeatureList();
}

void MainWindow::addExtrude()
{
  TabPageWidget* page = currentPage(); if (!page) return;

  // Ensure there is at least one sketch to pick; if none, create a sample rectangle sketch
  if (page->doc().sketches().empty())
  {
    auto sk = std::make_shared<Sketch>();
    const double w = 20.0, h = 10.0;
    auto c1 = sk->addLine(gp_Pnt2d(0.0, 0.0), gp_Pnt2d(w, 0.0));
    auto c2 = sk->addLine(gp_Pnt2d(w, 0.0), gp_Pnt2d(w, h));
    auto c3 = sk->addLine(gp_Pnt2d(w, h), gp_Pnt2d(0.0, h));
    auto c4 = sk->addLine(gp_Pnt2d(0.0, h), gp_Pnt2d(0.0, 0.0));
    sk->addCoincident({c1, 1}, {c2, 0});
    sk->addCoincident({c2, 1}, {c3, 0});
    sk->addCoincident({c3, 1}, {c4, 0});
    sk->addCoincident({c4, 1}, {c1, 0});
    sk->solveConstraints();
    page->doc().addSketch(sk);
  }

  // Build names for selection from the document's registered sketches
  const auto sketches = page->doc().sketches();
  QStringList names;
  for (int i = 0; i < static_cast<int>(sketches.size()); ++i)
    names << QString("Sketch %1").arg(i + 1);

  CreateExtrudeDialog dlg(this);
  dlg.setSketchNames(names);
  if (dlg.exec() != QDialog::Accepted) return;

  int idx = dlg.selectedSketchIndex();
  if (idx < 0 || idx >= static_cast<int>(sketches.size())) return;

  auto sketch = sketches.at(static_cast<std::size_t>(idx));
  CreateExtrudeCommand cmd(sketch, dlg.distance());
  cmd.execute(page->doc());
  page->syncViewerFromDoc(true);
  page->refreshFeatureList();
}

void MainWindow::addSketch()
{
  TabPageWidget* page = currentPage(); if (!page) return;
  auto& doc = page->doc();

  // Build plane names from document planes
  const auto& planes = doc.planes();
  if (planes.IsEmpty()) return;

  QStringList names;
  for (NCollection_Sequence<Handle(PlaneFeature)>::Iterator it(planes); it.More(); it.Next())
  {
    const Handle(PlaneFeature)& pf = it.Value();
    if (!pf.IsNull() && !pf->name().IsEmpty())
      names << QString::fromLatin1(pf->name().ToCString());
    else
      names << QStringLiteral("Plane");
  }

  CreateSketchDialog dlg(this);
  dlg.setPlaneNames(names);
  if (dlg.exec() != QDialog::Accepted) return;

  const int idx = dlg.selectedPlaneIndex();
  if (idx < 0 || idx >= planes.Size()) return;

  Handle(PlaneFeature) plane = planes.Value(idx + 1); // NCollection_Sequence is 1-based
  CreateSketchCommand cmd(plane);
  cmd.execute(doc);
  // Sync viewer and set overlay edit mode for the created sketch
  page->syncViewerFromDoc(true);
  if (auto sk = cmd.createdSketch())
  {
    page->viewer()->setSketchEditMode(sk->id(), true, true);
  }
  page->refreshFeatureList();
  updateActionStates();
}



void MainWindow::syncViewerFromDoc(bool toUpdate)
{
  TabPageWidget* page = currentPage(); if (!page) return;
  page->syncViewerFromDoc(toUpdate);
}

void MainWindow::addNewTab()
{
  auto* page = new TabPageWidget(this);
  int idx = m_tabs->addTab(page, QString("Untitled %1").arg(m_tabs->count() + 1));
  m_tabs->setCurrentIndex(idx);
  // Initialize empty history list
  page->refreshFeatureList();
}

TabPageWidget* MainWindow::currentPage() const
{
  return qobject_cast<TabPageWidget*>(m_tabs ? m_tabs->currentWidget() : nullptr);
}
