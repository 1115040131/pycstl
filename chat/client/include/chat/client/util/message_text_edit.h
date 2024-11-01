#pragma once

#include <QTextEdit>

#include "chat/client/define.h"

class MessageTextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit MessageTextEdit(QWidget* parent = nullptr);

    ~MessageTextEdit() = default;

    std::vector<MsgInfo> getMsgList();

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

    virtual void insertFromMimeData(const QMimeData* source) override;

private:
    /// @brief 判断文件是否为图片
    bool isImage(const QString& url);

    /// @brief 根据 Url 插入文件
    void insertFileFromUrl(const QStringList& urls);

    /// @brief 插入图片
    void insertImage(const QString& url);

    /// @brief 插入文件
    void insertTextFile(const QString& url);

    /// @brief 解析出 url
    QStringList getUrl(const QString& text);

    /// @brief 根据 url 获取图片
    QPixmap getFileIconPixmap(const QString& url);

    /// @brief 文件大小转换到可读字符串
    QString getFileSize(qint64 size);

signals:
    void send();

private:
    std::vector<MsgInfo> msg_list_;
};