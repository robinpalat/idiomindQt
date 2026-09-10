#pragma once

#include <QObject>
#include <QString>

#include "core/config/AppPaths.h"
#include "core/config/Settings.h"
#include "ui/controllers/TopicProxy.h"

namespace idiomind {

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject* parent = nullptr);

    static QString appName();
    static QString appVersion();

    Q_INVOKABLE QString name() const;
    Q_INVOKABLE QString version() const;
    Q_INVOKABLE QString pathsSummary() const;

    // F6-B.1: TopicProxy para QML.
    Q_PROPERTY(ui::TopicProxy* topicProxy READ topicProxy CONSTANT)
    ui::TopicProxy* topicProxy() const { return m_topicProxy; }

private:
    core::AppPaths m_paths;
    core::Settings m_settings;
    ui::TopicProxy *m_topicProxy;
};

} // namespace idiomind