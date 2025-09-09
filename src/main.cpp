#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLibraryInfo>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

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
