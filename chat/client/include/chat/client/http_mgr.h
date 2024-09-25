#pragma once

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

#include "chat/client/define.h"
#include "common/singleton.h"

class HttpMgr : public QObject, public pyc::Singleton<HttpMgr> {
    Q_OBJECT
    friend class pyc::Singleton<HttpMgr>;

private:
    HttpMgr();

public:
    ~HttpMgr();

    void PostHttpRequest(Module mod, ReqId req_id, const QJsonObject& json);

private slots:
    void slot_http_finish(Module mod, ReqId req_id, const QString& res, ErrorCode err);

signals:
    void sig_http_finish(Module mod, ReqId req_id, const QString& res, ErrorCode err);
    void sig_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err);
    void sig_reset_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

private:
    QNetworkAccessManager manager_;
    QString gate_url_prefix_;
};