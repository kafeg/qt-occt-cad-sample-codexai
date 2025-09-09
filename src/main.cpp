#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLibraryInfo>
#include <QScreen>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    // Exit gracefully if no screens are available (headless environment)
    if (QGuiApplication::screens().isEmpty()) {
        qCritical("No screens available. Are you running headless?");
        return 2;
    }

    QQmlApplicationEngine engine;
    // Help the engine find Qt's QML modules when running from the build tree
    engine.addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));
    const QUrl url(QStringLiteral("qrc:/ui/qml/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
