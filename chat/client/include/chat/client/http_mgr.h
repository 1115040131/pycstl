#pragma once

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

#include "chat/client/define.h"
#include "common/singleton.h"

class HttpMgr : public QObject, public pyc::Singleton<HttpMgr> {
    Q_OBJECT
    friend class pyc::Singleton<HttpMgr>;

public:
    ~HttpMgr();

private slots:
    void slot_http_finish(ReqId req_id, const QString& res, ErrorCode err, Module mod);

signals:
    void sig_http_finish(ReqId req_id, const QString& res, ErrorCode err, Module mod);
    void sig_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

private:
    HttpMgr();

    void PostHttpRequest(const QUrl& url, const QJsonObject& json, ReqId req_id, Module mod);

private:
    QNetworkAccessManager manager_;
};