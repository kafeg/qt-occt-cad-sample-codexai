#include "MainWindow.h"

#include <OcctQOpenGLWidgetViewer.h>

#include <Standard_WarningsDisable.hxx>
#include <QAction>
#include <QHBoxLayout>
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

MainWindow::MainWindow()
{
  myViewer = new OcctQOpenGLWidgetViewer();
  myDoc = new Document();
  setCentralWidget(myViewer);
  createMenuBar();
  createToolBar();
}

void MainWindow::createMenuBar()
{
  QMenuBar* mbar = new QMenuBar();
  QMenu*    file = mbar->addMenu("&File");
  {
    QAction* split = new QAction(file);
    split->setText("Split Views");
    file->addAction(split);
    connect(split, &QAction::triggered, [this]() {
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
  connect(actDelete, &QAction::triggered, [this]() { deleteSelected(); });
  tb->addAction(actDelete);

  QAction* actAbout = new QAction("About", tb);
  connect(actAbout, &QAction::triggered, [this]() {
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
  CreateBoxDialog dlg(this);
  if (dlg.exec() != QDialog::Accepted) return;

  CreateBoxCommand cmd(dlg.dx(), dlg.dy(), dlg.dz());
  cmd.execute(*myDoc);
  syncViewerFromDoc(true);
}

void MainWindow::deleteSelected()
{
  // Delete selected feature (if any)
  Handle(AIS_Shape) sel = myViewer->selectedShape();
  if (!sel.IsNull() && myBodyToFeature.Contains(sel))
  {
    Handle(Feature) f = Handle(Feature)::DownCast(myBodyToFeature.FindFromKey(sel));
    myDoc->removeFeature(f);
    myDoc->recompute();
    syncViewerFromDoc(true);
    return;
  }
}

void MainWindow::syncViewerFromDoc(bool toUpdate)
{
  myFeatureToBody.Clear();
  myBodyToFeature.Clear();
  myViewer->clearBodies(false);
  for (NCollection_Sequence<Handle(Feature)>::Iterator it(myDoc->features()); it.More(); it.Next())
  {
    const Handle(Feature)& f = it.Value(); if (f.IsNull()) continue;
    Handle(AIS_Shape) body = myViewer->addShape(f->shape(), AIS_Shaded, 0, false);
    myFeatureToBody.Add(f, body);
    myBodyToFeature.Add(body, f);
  }
  if (toUpdate) myViewer->update();
}

void MainWindow::clearAll()
{
  myDoc->clear();
  myViewer->clearBodies(true);
}

void MainWindow::addSample()
{
  const double dx = 20.0, dy = 20.0, dz = 20.0;
  const double gap = 5.0;
  Handle(BoxFeature) b1 = new BoxFeature(dx, dy, dz);
  Handle(BoxFeature) b2 = new BoxFeature(dx, dy, dz);
  Handle(BoxFeature) b3 = new BoxFeature(dx, dy, dz);
  myDoc->addFeature(b1);
  myDoc->addFeature(b2);
  myDoc->addFeature(b3);
  myDoc->recompute();
  // Sync without update to apply local transforms first
  syncViewerFromDoc(false);
  // Position shapes independently in viewer (test-only)
  if (myFeatureToBody.Contains(b1))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(myFeatureToBody.FindFromKey(b1));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(0.0, 0.0, 0.0)); s->SetLocalTransformation(tr); myViewer->Context()->Redisplay(s, false); }
  }
  if (myFeatureToBody.Contains(b2))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(myFeatureToBody.FindFromKey(b2));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(dx + gap, 0.0, 0.0)); s->SetLocalTransformation(tr); myViewer->Context()->Redisplay(s, false); }
  }
  if (myFeatureToBody.Contains(b3))
  {
    Handle(AIS_Shape) s = Handle(AIS_Shape)::DownCast(myFeatureToBody.FindFromKey(b3));
    if (!s.IsNull()) { gp_Trsf tr; tr.SetTranslation(gp_Vec(2.0 * (dx + gap), 0.0, 0.0)); s->SetLocalTransformation(tr); myViewer->Context()->Redisplay(s, false); }
  }
  myViewer->update();
}
