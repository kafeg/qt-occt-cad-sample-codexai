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

MainWindow::MainWindow()
{
  myViewer = new OcctQOpenGLWidgetViewer();
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

  QAction* actBodies = new QAction("Hide bodies", tb);
  actBodies->setCheckable(true);
  actBodies->setChecked(true);
  connect(actBodies, &QAction::toggled, [this, actBodies](bool on) {
    myViewer->setBodiesVisible(on);
    actBodies->setText(on ? "Hide bodies" : "Show bodies");
  });
  tb->addAction(actBodies);

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
}

