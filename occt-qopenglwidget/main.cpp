// Copyright (c) 2025 Kirill Gavrilov

#include "OcctQOpenGLWidgetViewer.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QSurfaceFormat>

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QSlider>
#include <QWidgetAction>
#include <QHBoxLayout>
#include <Standard_WarningsRestore.hxx>

#include <Standard_Version.hxx>
#include <Quantity_Color.hxx>

//! Main application window.
class MyMainWindow : public QMainWindow
{
  OcctQOpenGLWidgetViewer* myViewer = nullptr;

public:
  //! Window constructor.
  MyMainWindow()
  {
    // 3D Viewer widget as a central widget
    myViewer = new OcctQOpenGLWidgetViewer();
    setCentralWidget(myViewer);

    // menu bar
    createMenuBar();

    // toolbar with actions
    createToolBar();

    // no overlay controls; toolbar hosts UI now
  }

private:
  //! Define menu bar with Quit item.
  void createMenuBar()
  {
    QMenuBar* aMenuBar    = new QMenuBar();
    QMenu*    aMenuWindow = aMenuBar->addMenu("&File");
    {
      QAction* anActionSplit = new QAction(aMenuWindow);
      anActionSplit->setText("Split Views");
      aMenuWindow->addAction(anActionSplit);
      connect(anActionSplit, &QAction::triggered, [this]() { splitSubviews(); });
    }
    {
      QAction* anActionQuit = new QAction(aMenuWindow);
      anActionQuit->setText("Quit");
      aMenuWindow->addAction(anActionQuit);
      connect(anActionQuit, &QAction::triggered, [this]() { close(); });
    }
    setMenuBar(aMenuBar);
  }

  //! Top toolbar for quick actions.
  void createToolBar()
  {
    QToolBar* tb = new QToolBar("Main Toolbar", this);
    tb->setMovable(true);

    // Toggle bodies visibility (checkable)
    QAction* actBodies = new QAction("Hide bodies", tb);
    actBodies->setCheckable(true);
    actBodies->setChecked(true);
    connect(actBodies, &QAction::toggled, [this, actBodies](bool on) {
      myViewer->setBodiesVisible(on);
      actBodies->setText(on ? "Hide bodies" : "Show bodies");
    });
    tb->addAction(actBodies);

    // About
    QAction* actAbout = new QAction("About", tb);
    connect(actAbout, &QAction::triggered, [this]() {
      QMessageBox::information(0,
                               "About Sample",
                               QString() + "OCCT 3D Viewer sample embedded into Qt Widgets.\n\n"
                                 + "Open CASCADE Technology v." OCC_VERSION_STRING_EXT "\n"
                                 + "Qt v." QT_VERSION_STR "\n\n" + "OpenGL info:\n" + myViewer->getGlInfo());
    });
    tb->addAction(actAbout);

    // Background slider (as embedded widget)
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

  //! Define controls over 3D viewer.
  void createLayoutOverViewer() {}

  //! Advanced method splitting 3D Viewer into sub-views.
  void splitSubviews()
  {
    if (!myViewer->View()->Subviews().IsEmpty())
    {
      // remove subviews
      myViewer->View()->View()->SetSubviewComposer(false);
      NCollection_Sequence<Handle(V3d_View)> aSubviews = myViewer->View()->Subviews();
      for (const Handle(V3d_View)& aSubviewIter : aSubviews)
        aSubviewIter->Remove();

      myViewer->OnSubviewChanged(myViewer->Context(), nullptr, myViewer->View());
    }
    else
    {
      // create two subviews splitting window horizontally
      myViewer->View()->View()->SetSubviewComposer(true);

      Handle(V3d_View) aSubView1 = new V3d_View(myViewer->Viewer());
      aSubView1->SetImmediateUpdate(false);
      aSubView1->SetWindow(myViewer->View(),
                           Graphic3d_Vec2d(0.5, 1.0),
                           Aspect_TOTP_LEFT_UPPER,
                           Graphic3d_Vec2d(0.0, 0.0));

      Handle(V3d_View) aSubView2 = new V3d_View(myViewer->Viewer());
      aSubView2->SetImmediateUpdate(false);
      aSubView2->SetWindow(myViewer->View(),
                           Graphic3d_Vec2d(0.5, 1.0),
                           Aspect_TOTP_LEFT_UPPER,
                           Graphic3d_Vec2d(0.5, 0.0));

      myViewer->OnSubviewChanged(myViewer->Context(), nullptr, aSubView1);
    }
    myViewer->View()->Invalidate();
    myViewer->update();
  }
};

int main(int theNbArgs, char** theArgVec)
{
  QApplication aQApp(theNbArgs, theArgVec);

  QCoreApplication::setApplicationName("OCCT Qt/QOpenGLWidget Viewer sample");
  QCoreApplication::setOrganizationName("OpenCASCADE");
  QCoreApplication::setApplicationVersion(OCC_VERSION_STRING_EXT);

#ifdef __APPLE__
  // suppress Qt warning "QCocoaGLContext: Falling back to unshared context"
  bool           isCoreProfile = true;
  QSurfaceFormat aGlFormat;
  aGlFormat.setDepthBufferSize(24);
  aGlFormat.setStencilBufferSize(8);
  if (isCoreProfile)
    aGlFormat.setVersion(4, 5);

  aGlFormat.setProfile(isCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
  QSurfaceFormat::setDefaultFormat(aGlFormat);
#endif

  MyMainWindow aMainWindow;
  aMainWindow.resize(aMainWindow.sizeHint());
  aMainWindow.show();
  return aQApp.exec();
}
