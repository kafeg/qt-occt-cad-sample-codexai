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
#include <CreateCylinderCommand.h>
#include <CreateCylinderDialog.h>
// model/viewer headers for sync helpers
#include <Feature.h>
#include <BoxFeature.h>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <AIS_Shape.hxx>
#include <CylinderFeature.h>
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
    QAction* actAddCyl = new QAction(file);
    actAddCyl->setText("Add Cylinder");
    file->addAction(actAddCyl);
    connect(actAddCyl, &QAction::triggered, [this]() { addCylinder(); });
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

  QAction* actAddCyl = new QAction("Add Cylinder", tb);
  connect(actAddCyl, &QAction::triggered, [this]() { addCylinder(); });
  tb->addAction(actAddCyl);


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

void MainWindow::addCylinder()
{
  TabPage* page = currentPage(); if (!page) return;
  CreateCylinderDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted) return;

  CreateCylinderCommand cmd(dlg.radius(), dlg.height());
  cmd.execute(page->doc());
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
  if (toUpdate)
  {
    // Ensure OCCT view invalidates and redraws after content changes
    myViewer->Context()->UpdateCurrentViewer();
    myViewer->View()->Invalidate();
    myViewer->update();
  }
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
  // Boxes
  Handle(BoxFeature) b1 = new BoxFeature(dx, dy, dz);
  Handle(BoxFeature) b2 = new BoxFeature(dx, dy, dz);
  Handle(BoxFeature) b3 = new BoxFeature(dx, dy, dz);
  page->doc().addFeature(b1);
  page->doc().addFeature(b2);
  page->doc().addFeature(b3);
  // Cylinders placed nearby along +Y
  const double cr = 10.0;
  const double ch = dz;
  Handle(CylinderFeature) c1 = new CylinderFeature(cr, ch);
  Handle(CylinderFeature) c2 = new CylinderFeature(cr, ch);
  Handle(CylinderFeature) c3 = new CylinderFeature(cr, ch);
  page->doc().addFeature(c1);
  page->doc().addFeature(c2);
  page->doc().addFeature(c3);
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
  const double yoff = dy + gap;
  if (page->featureToBody().Contains(c1))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(page->featureToBody().FindFromKey(c1));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(0.0, yoff, 0.0)); s->SetLocalTransformation(tr); page->viewer()->Context()->Redisplay(s, false); }
  }
  if (page->featureToBody().Contains(c2))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(page->featureToBody().FindFromKey(c2));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(dx + gap, yoff, 0.0)); s->SetLocalTransformation(tr); page->viewer()->Context()->Redisplay(s, false); }
  }
  if (page->featureToBody().Contains(c3))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(page->featureToBody().FindFromKey(c3));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(2.0 * (dx + gap), yoff, 0.0)); s->SetLocalTransformation(tr); page->viewer()->Context()->Redisplay(s, false); }
  }
  // Force viewer to refresh immediately after redisplay
  page->viewer()->Context()->UpdateCurrentViewer();
  page->viewer()->View()->Invalidate();
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
