#include "chat/client/http_mgr.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QSettings>
#include <QUrl>

#include "chat/common/method.h"

HttpMgr::HttpMgr() {
    QSettings settings("chat/client/config/config.ini", QSettings::IniFormat);
    auto gate_host = settings.value("GateServer/host").toString();
    auto gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix_ = "http://" + gate_host + ":" + gate_port;

    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

HttpMgr::~HttpMgr() {}

void HttpMgr::PostHttpRequest(Module mod, ReqId req_id, const QJsonObject& json) {
    QUrl url(gate_url_prefix_ + ToUrl(req_id));

    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    QNetworkReply* reply = manager_.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, [this, mod, req_id, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit this->sig_http_finish(mod, req_id, "", ErrorCode::kNetworkError);
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit this->sig_http_finish(mod, req_id, res, ErrorCode::kSuccess);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(Module mod, ReqId req_id, const QString& res, ErrorCode err) {
    if (mod == Module::kRegisterMod) {
        emit sig_reg_mod_finish(req_id, res, err);
    } else if (mod == Module::kResetMod) {
        emit sig_reset_mod_finish(req_id, res, err);
    } else if (mod == Module::kLogin) {
        emit sig_login_mod_finish(req_id, res, err);
    }
}