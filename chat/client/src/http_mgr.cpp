#include "chat/client/http_mgr.h"

#include <QJsonDocument>
#include <QNetworkReply>

HttpMgr::~HttpMgr() {}

HttpMgr::HttpMgr() { connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish); }

void HttpMgr::PostHttpRequest(const QUrl& url, const QJsonObject& json, ReqId req_id, Module mod) {
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    QNetworkReply* reply = manager_.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, [this, reply, req_id, mod]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit this->sig_http_finish(req_id, "", ErrorCode::kNetworkError, mod);
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit this->sig_http_finish(req_id, res, ErrorCode::kSuccess, mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId req_id, const QString& res, ErrorCode err, Module mod) {
    if (mod == Module::kRegisterMod) {
        emit sig_reg_mod_finish(req_id, res, err);
    }
}