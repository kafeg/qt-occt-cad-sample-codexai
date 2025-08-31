#include "MainWindow.h"

#include <OcctQOpenGLWidgetViewer.h>

#include <Standard_WarningsDisable.hxx>
#include <QAction>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSlider>
#include <QToolBar>
#include <QWidgetAction>
#include <Standard_WarningsRestore.hxx>

#include <Quantity_Color.hxx>
#include <Standard_Version.hxx>

#include <Document.h>
#include <CreateBoxCommand.h>
#include <CreateBoxDialog.h>
// model/viewer headers for sync helpers
#include <Feature.h>
#include <BoxFeature.h>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <AIS_Shape.hxx>
#include "TabPage.h"

MainWindow::MainWindow()
{
  myTabs = new QTabWidget(this);
  myTabs->setTabsClosable(true);
  connect(myTabs, &QTabWidget::tabCloseRequested, [this](int index) {
    QWidget* w = myTabs->widget(index);
    myTabs->removeTab(index);
    delete w;
    if (myTabs->count() == 0) {
      addNewTab();
    }
  });
  setCentralWidget(myTabs);
  addNewTab();
  createMenuBar();
  createToolBar();
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
      const int idx = myTabs->currentIndex();
      if (idx >= 0) emit myTabs->tabCloseRequested(idx);
    });
  }
  {
    QAction* split = new QAction(file);
    split->setText("Split Views");
    file->addAction(split);
    connect(split, &QAction::triggered, [this]() {
      TabPage* page = currentPage(); if (!page) return;
      auto* myViewer = page->viewer();
      if (!myViewer->View()->Subviews().IsEmpty())
      {
        myViewer->View()->View()->SetSubviewComposer(false);
        NCollection_Sequence<Handle(V3d_View)> aSubviews = myViewer->View()->Subviews();
        for (const Handle(V3d_View)& aSubviewIter : aSubviews) aSubviewIter->Remove();
        myViewer->OnSubviewChanged(myViewer->Context(), nullptr, myViewer->View());
      }
      else
      {
        myViewer->View()->View()->SetSubviewComposer(true);
        Handle(V3d_View) aSubView1 = new V3d_View(myViewer->Viewer());
        aSubView1->SetImmediateUpdate(false);
        aSubView1->SetWindow(myViewer->View(), Graphic3d_Vec2d(0.5, 1.0), Aspect_TOTP_LEFT_UPPER, Graphic3d_Vec2d(0.0, 0.0));
        Handle(V3d_View) aSubView2 = new V3d_View(myViewer->Viewer());
        aSubView2->SetImmediateUpdate(false);
        aSubView2->SetWindow(myViewer->View(), Graphic3d_Vec2d(0.5, 1.0), Aspect_TOTP_LEFT_UPPER, Graphic3d_Vec2d(0.5, 0.0));
        myViewer->OnSubviewChanged(myViewer->Context(), nullptr, aSubView1);
      }
      myViewer->View()->Invalidate();
      myViewer->update();
    });
  }
  {
    QAction* actAddBox = new QAction(file);
    actAddBox->setText("Add Box");
    file->addAction(actAddBox);
    connect(actAddBox, &QAction::triggered, [this]() { addBox(); });
  }
  {
    QAction* quit = new QAction(file);
    quit->setText("Quit");
    file->addAction(quit);
    connect(quit, &QAction::triggered, [this]() { close(); });
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

  QAction* actDelete = new QAction("Delete Selected", tb);
  actDelete->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
  connect(actDelete, &QAction::triggered, [this]() { deleteSelected(); });
  tb->addAction(actDelete);

  QAction* actAbout = new QAction("About", tb);
  connect(actAbout, &QAction::triggered, [this]() {
    TabPage* page = currentPage(); if (!page) return;
    auto* myViewer = page->viewer();
    QMessageBox::information(0,
                             "About",
                             QString() + "Parametric CAD skeleton using OCCT + Qt6\n\n"
                               + "Open CASCADE Technology v." OCC_VERSION_STRING_EXT "\n"
                               + "Qt v." QT_VERSION_STR "\n\n" + "OpenGL info:\n" + myViewer->getGlInfo());
  });
  tb->addAction(actAbout);

  QWidget* bgBox = new QWidget(tb);
  QHBoxLayout* bgLay = new QHBoxLayout(bgBox);
  bgLay->setContentsMargins(6, 0, 6, 0);
  QLabel* lbl = new QLabel("Background", bgBox);
  bgLay->addWidget(lbl);
  QSlider* slider = new QSlider(Qt::Horizontal, bgBox);
  slider->setRange(0, 255);
  slider->setSingleStep(1);
  slider->setPageStep(15);
  slider->setTickInterval(15);
  slider->setTickPosition(QSlider::NoTicks);
  slider->setValue(64);
  slider->setFixedWidth(200);
  bgLay->addWidget(slider);
  connect(slider, &QSlider::valueChanged, [this](int theValue) {
    TabPage* page = currentPage(); if (!page) return;
    auto* myViewer = page->viewer();
    const float          aVal = theValue / 255.0f;
    const Quantity_Color aColor(aVal, aVal, aVal, Quantity_TOC_sRGB);
    const Quantity_Color aBottom(0.40f, 0.40f, 0.40f, Quantity_TOC_sRGB);
    for (const Handle(V3d_View)& aSubviewIter : myViewer->View()->Subviews())
    {
      aSubviewIter->SetBgGradientColors(aColor, aBottom, Aspect_GradientFillMethod_Elliptical);
      aSubviewIter->Invalidate();
    }
    myViewer->View()->SetBgGradientColors(aColor, aBottom, Aspect_GradientFillMethod_Elliptical);
    myViewer->View()->Invalidate();
    myViewer->update();
  });
  QWidgetAction* bgAction = new QWidgetAction(tb);
  bgAction->setDefaultWidget(bgBox);
  tb->addAction(bgAction);

  addToolBar(Qt::TopToolBarArea, tb);

  // Test toolbar block
  QToolBar* ttest = new QToolBar("Test", this);
  ttest->setMovable(true);
  QAction* actClearAll = new QAction("Clear All", ttest);
  connect(actClearAll, &QAction::triggered, [this]() { clearAll(); });
  ttest->addAction(actClearAll);
  QAction* actAddSample = new QAction("Add Sample", ttest);
  connect(actAddSample, &QAction::triggered, [this]() { addSample(); });
  ttest->addAction(actAddSample);
  addToolBar(Qt::TopToolBarArea, ttest);
}

void MainWindow::addBox()
{
  TabPage* page = currentPage(); if (!page) return;
  CreateBoxDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted) return;

  CreateBoxCommand cmd(dlg.dx(), dlg.dy(), dlg.dz());
  cmd.execute(page->doc());
  syncViewerFromDoc(true);
}

void MainWindow::deleteSelected()
{
  TabPage* page = currentPage(); if (!page) return;
  // Collect all selected bodies; if nothing is selected but something is detected, select detected and remove it
  NCollection_Sequence<Handle(AIS_Shape)> selected;
  for (Handle(AIS_Shape) s = page->viewer()->selectedShape(); !s.IsNull(); s.Nullify())
  {
    // selectedShape() returns first one; iterate full list manually
    for (page->viewer()->Context()->InitSelected(); page->viewer()->Context()->MoreSelected(); page->viewer()->Context()->NextSelected())
    {
      Handle(AIS_Shape) one = Handle(AIS_Shape)::DownCast(page->viewer()->Context()->SelectedInteractive());
      if (!one.IsNull()) selected.Append(one);
    }
    break;
  }
  if (selected.IsEmpty())
  {
    // Try to take detected and select it programmatically
    Handle(AIS_Shape) det = page->viewer()->lastDetectedShape();
    if (!det.IsNull())
    {
      page->viewer()->Context()->ClearSelected(false);
      page->viewer()->Context()->AddOrRemoveSelected(det, false); // select detected
      selected.Append(det);
    }
  }
  if (selected.IsEmpty()) return; // nothing to remove

  // Remove all corresponding features (no duplicates)
  NCollection_Sequence<Handle(Feature)> toRemove;
  for (NCollection_Sequence<Handle(AIS_Shape)>::Iterator it(selected); it.More(); it.Next())
  {
    const Handle(AIS_Shape)& s = it.Value(); if (s.IsNull()) continue;
    if (page->bodyToFeature().Contains(s))
    {
      Handle(Feature) f = Handle(Feature)::DownCast(page->bodyToFeature().FindFromKey(s));
      bool dup = false; for (NCollection_Sequence<Handle(Feature)>::Iterator jt(toRemove); jt.More(); jt.Next()) { if (jt.Value() == f) { dup = true; break; } }
      if (!dup) toRemove.Append(f);
    }
  }
  if (toRemove.IsEmpty()) return;
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(toRemove); it.More(); it.Next())
  {
    page->doc().removeFeature(it.Value());
  }
  page->doc().recompute();
  page->viewer()->Context()->ClearSelected(false);
  syncViewerFromDoc(true);
}

void MainWindow::syncViewerFromDoc(bool toUpdate)
{
  TabPage* page = currentPage(); if (!page) return;
  auto* myViewer = page->viewer();
  page->featureToBody().Clear();
  page->bodyToFeature().Clear();
  myViewer->clearBodies(false);
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(page->doc().features()); it.More(); it.Next())
  {
    const Handle(Feature)& f = it.Value(); if (f.IsNull()) continue;
    Handle(AIS_Shape) body = myViewer->addShape(f->shape(), AIS_Shaded, 0, false);
    page->featureToBody().Add(f, body);
    page->bodyToFeature().Add(body, f);
  }
  if (toUpdate) myViewer->update();
}

void MainWindow::clearAll()
{
  TabPage* page = currentPage(); if (!page) return;
  page->doc().clear();
  page->viewer()->clearBodies(true);
}

void MainWindow::addSample()
{
  TabPage* page = currentPage(); if (!page) return;
  const double dx = 20.0, dy = 20.0, dz = 20.0;
  const double gap = 5.0;
  Handle(BoxFeature) b1 = new BoxFeature(dx, dy, dz);
  Handle(BoxFeature) b2 = new BoxFeature(dx, dy, dz);
  Handle(BoxFeature) b3 = new BoxFeature(dx, dy, dz);
  page->doc().addFeature(b1);
  page->doc().addFeature(b2);
  page->doc().addFeature(b3);
  page->doc().recompute();
  // Sync without update to apply local transforms first
  syncViewerFromDoc(false);
  // Position shapes independently in viewer (test-only)
  if (page->featureToBody().Contains(b1))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(page->featureToBody().FindFromKey(b1));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(0.0, 0.0, 0.0)); s->SetLocalTransformation(tr); page->viewer()->Context()->Redisplay(s, false); }
  }
  if (page->featureToBody().Contains(b2))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(page->featureToBody().FindFromKey(b2));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(dx + gap, 0.0, 0.0)); s->SetLocalTransformation(tr); page->viewer()->Context()->Redisplay(s, false); }
  }
  if (page->featureToBody().Contains(b3))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(page->featureToBody().FindFromKey(b3));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(2.0 * (dx + gap), 0.0, 0.0)); s->SetLocalTransformation(tr); page->viewer()->Context()->Redisplay(s, false); }
  }
  page->viewer()->update();
}

void MainWindow::addNewTab()
{
  auto* page = new TabPage(this);
  int idx = myTabs->addTab(page, QString("Untitled %1").arg(myTabs->count() + 1));
  myTabs->setCurrentIndex(idx);
}

TabPage* MainWindow::currentPage() const
{
  return qobject_cast<TabPage*>(myTabs ? myTabs->currentWidget() : nullptr);
}
