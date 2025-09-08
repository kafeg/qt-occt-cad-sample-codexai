// Lightweight QML viewer wrapper over OCCT using QQuickFramebufferObject
#ifndef _OcctQmlViewer_HeaderFile
#define _OcctQmlViewer_HeaderFile

#include <Standard_WarningsDisable.hxx>
#include <QQuickFramebufferObject>
#include <QMutex>
#include <QSize>
#include <Standard_WarningsRestore.hxx>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <TopoDS_Shape.hxx>
#include <NCollection_Sequence.hxx>

#include <memory>
#include <vector>

class Datum;

// QML-facing viewer. Renders into a Qt Quick FBO and manages an OCCT context.
class OcctQmlViewer : public QQuickFramebufferObject
{
  Q_OBJECT
  Q_PROPERTY(QString glInfo READ glInfo NOTIFY glInfoChanged)

public:
  OcctQmlViewer(QQuickItem* parent = nullptr);
  ~OcctQmlViewer() override;

  // QQuickFramebufferObject
  Renderer* createRenderer() const override;

  // Public API similar to OcctQOpenGLWidgetViewer (subset for now)
public slots:
  void resetViewToOrigin(double distance = 1.2);
  void clearBodies();

public: // C++ API (non-QML) for adding shapes and datum
  Handle(AIS_Shape) addShape(const TopoDS_Shape& theShape,
                             AIS_DisplayMode    theDispMode = AIS_Shaded,
                             Standard_Integer   theDispPriority = 0,
                             bool               theToUpdate = false);
  void setDatum(const std::shared_ptr<Datum>& d);

public:
  const QString& glInfo() const { return m_glInfo; }

signals:
  void selectionChanged();
  void glInfoChanged();

private:
  struct PendingShape
  {
    TopoDS_Shape     shape;
    AIS_DisplayMode  dispMode = AIS_Shaded;
    Standard_Integer dispPrio = 0;
  };

  // State synchronized to the renderer each frame
  mutable QMutex           m_mutex;
  std::vector<PendingShape> m_pendingShapes;
  bool                      m_clearRequested = false;
  bool                      m_resetRequested = false;
  double                    m_resetDistance  = 1.2;
  std::shared_ptr<Datum>    m_datum;
  QString                   m_glInfo;

public: // internal helper for renderer to pull state
  void takePending(std::vector<PendingShape>& outShapes,
                   bool& outDoClear,
                   bool& outDoReset,
                   double& outResetDist,
                   std::shared_ptr<Datum>& outDatum,
                   QString& outGlInfo) const;

public: // Renderer implementation
  class RendererImpl : public QQuickFramebufferObject::Renderer
  {
  public:
    RendererImpl();
    ~RendererImpl() override;

    void render() override;
    void synchronize(QQuickFramebufferObject* item) override;
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;

  private:
    void ensureOcctContext();
    void applyPending();
    void resetViewToOriginInternal(double distance);
    void updateWindowSizeFromFbo();
    void initViewDefaults();

  private:
    // OCCT handles
    Handle(V3d_Viewer)             m_viewer;
    Handle(V3d_View)               m_view;
    Handle(AIS_InteractiveContext) m_context;

    // Cached state from item
    std::vector<PendingShape> m_toAdd;
    bool                      m_doClear = false;
    bool                      m_doReset = false;
    double                    m_resetDistance = 1.2;
    std::shared_ptr<Datum>    m_datum;

    // GL info string (updated after first context bind)
    QString m_glInfo;
  };
};

#endif // _OcctQmlViewer_HeaderFile

