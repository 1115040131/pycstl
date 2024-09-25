#include "chat/client/url_mgr.h"

#include <QSettings>

UrlMgr::UrlMgr() {
    QSettings settings("chat/client/config/config.ini", QSettings::IniFormat);
    auto gate_host = settings.value("GateServer/host").toString();
    auto gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix_ = "http://" + gate_host + ":" + gate_port;
}
