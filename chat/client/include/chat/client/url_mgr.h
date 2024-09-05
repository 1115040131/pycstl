#pragma once

#include <QString>

#include "common/singleton.h"

class UrlMgr : public pyc::Singleton<UrlMgr> {
    friend class pyc::Singleton<UrlMgr>;

public:
    ~UrlMgr() = default;

    const QString& GateUrlPrefix() const { return gate_url_prefix_; }

private:
    UrlMgr();

private:
    QString gate_url_prefix_;
};