#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QUrl>

#include "app/Application.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/images/logo.png")));

    QCoreApplication::setOrganizationName(QStringLiteral("Idiomind"));
    QCoreApplication::setApplicationName(QStringLiteral("idiomind"));
    QCoreApplication::setApplicationVersion(idiomind::Application::appVersion());

    idiomind::Application application;

    // Expone el composition root como singleton "App" del módulo QML Idiomind.
    qmlRegisterSingletonInstance("Idiomind", 1, 0, "App", &application);

    // Registrar explícitamente el módulo QML Idiomind.
    extern void qml_register_types_Idiomind();
    qml_register_types_Idiomind();

    QQmlApplicationEngine engine;

    QObject::connect(&engine,
                     &QQmlApplicationEngine::objectCreationFailed,
                     &app,
                     [](const QUrl& objectUrl) {
                         qCritical() << "QML load failed:" << objectUrl;
                         QCoreApplication::exit(1);
                     },
                     Qt::QueuedConnection);

    // Cargar módulo QML Idiomind.
    engine.loadFromModule("Idiomind", "Main");

    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
