// Lightweight QML viewer wrapper over OCCT using QQuickFramebufferObject
#ifndef _OcctQmlViewer_HeaderFile
#define _OcctQmlViewer_HeaderFile

#include <Standard_WarningsDisable.hxx>
#include <QQuickFramebufferObject>
#include <QMutex>
#include <QSize>
#include <Standard_WarningsRestore.hxx>

#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <TopoDS_Shape.hxx>
#include <NCollection_Sequence.hxx>

#include <memory>
#include <vector>

class Datum;

// QML-facing viewer. Renders into a Qt Quick FBO and manages an OCCT context.
class OcctQmlViewer : public QQuickFramebufferObject, public AIS_ViewController
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

protected: // input handling (forwarded to AIS_ViewController)
  void keyPressEvent(QKeyEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void wheelEvent(QWheelEvent* e) override;
  void hoverMoveEvent(QHoverEvent* e) override;

private:
  // Called by the renderer thread via synchronize() to update UI-visible GL info
  void updateGlInfoFromRenderer(const QString& info);

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
  bool                      m_fitRequested   = false; // request FitAll
  bool                      m_clickSelectPending = false; // single-click selection request
  std::shared_ptr<Datum>    m_datum;
  QString                   m_glInfo;
  // Optional helpers
  Handle(AIS_InteractiveObject) m_grid; // FiniteGrid instance
  std::unique_ptr<class SceneGizmos> m_gizmos;

  // Basic click bookkeeping for selection thresholding
  mutable QPoint m_lastPosPx;        // last mouse pos in device px
  mutable QPoint m_currPosPx;        // current mouse pos in device px
  mutable bool   m_leftDown = false;
  mutable QPoint m_pressPosPx;       // press pos (for drag threshold)
  static constexpr int kDragThresholdPx = 4;

  // Pull and clear input state snapshot for render thread
  // Deprecated fallback input path removed; kept for ABI stability no-op
  void pullInput(bool& rotStart, bool& rotActive, bool& panActive,
                 QPoint& currPos, QPoint& lastPos) const;

public: // internal helper for renderer to pull state
  void takePending(std::vector<PendingShape>& outShapes,
                   bool& outDoClear,
                   bool& outDoReset,
                   double& outResetDist,
                   bool& outDoFitAll,
                   bool& outDoClickSelect,
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
    void handleSingleClickSelection();

  private:
    // OCCT handles
    Handle(V3d_Viewer)             m_viewer;
    Handle(V3d_View)               m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_ViewCube)           m_viewCube;
    Handle(AIS_InteractiveObject)  m_grid;
    std::unique_ptr<class SceneGizmos> m_gizmos;
    NCollection_Sequence<Handle(AIS_Shape)> m_bodies; // track bodies for clearBodies()

    // Cached state from item
    std::vector<PendingShape> m_toAdd;
    bool                      m_doClear = false;
    bool                      m_doReset = false;
    bool                      m_doFitAll = false;
    bool                      m_doClickSelect = false;
    double                    m_resetDistance = 1.2;
    std::shared_ptr<Datum>    m_datum;

    // GL info string (updated after first context bind)
    QString m_glInfo;
    OcctQmlViewer* m_owner = nullptr; // back-ref for event flushing
  };

  // Grant renderer access to private helpers
  friend class RendererImpl;
};

#endif // _OcctQmlViewer_HeaderFile
