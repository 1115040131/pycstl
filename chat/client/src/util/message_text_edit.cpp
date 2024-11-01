#include "chat/client/util/message_text_edit.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>

MessageTextEdit::MessageTextEdit(QWidget* parent) : QTextEdit(parent) { this->setMaximumHeight(60); }

std::vector<MsgInfo> MessageTextEdit::getMsgList() {
    std::vector<MsgInfo> result;

    QString doc = this->document()->toPlainText();
    QString text = "";
    size_t index_url = 0;
    size_t count = msg_list_.size();

    for (auto c : doc) {
        if (c == QChar::ObjectReplacementCharacter) {
            if (!text.isEmpty()) {
                result.emplace_back(MsgInfo::Type::kText, std::move(text));
            }
            while (index_url < count) {
                auto& msg = msg_list_[index_url];
                index_url++;
                if (this->document()->toHtml().contains(msg.content, Qt::CaseSensitive)) {
                    result.push_back(std::move(msg));
                    break;
                }
            }
        } else {
            text.append(c);
        }
    }
    if (!text.isEmpty()) {
        result.emplace_back(MsgInfo::Type::kText, std::move(text));
    }
    msg_list_.clear();
    this->clear();
    return result;
}

void MessageTextEdit::dragEnterEvent(QDragEnterEvent* event) {
    if (this == event->source()) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MessageTextEdit::dropEvent(QDropEvent* event) {
    insertFromMimeData(event->mimeData());
    event->accept();
}

void MessageTextEdit::keyPressEvent(QKeyEvent* event) {
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) &&
        !(event->modifiers() & Qt::ShiftModifier)) {
        emit send();
        return;
    }
    QTextEdit::keyPressEvent(event);
}

void MessageTextEdit::insertFromMimeData(const QMimeData* source) {
    QStringList urls = getUrl(source->text());
    insertFileFromUrl(urls);
}

bool MessageTextEdit::isImage(const QString& url) {
    static const QString imageFormat =
        "bmp,jpg,png,tif,gif,pcx,tga,exif,fpx,svg,psd,cdr,pcd,dxf,ufo,eps,ai,raw,wmf,webp";
    static const QStringList imageFormatList = imageFormat.split(",");
    QFileInfo fileInfo(url);
    QString suffix = fileInfo.suffix();
    if (imageFormatList.contains(suffix, Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

void MessageTextEdit::insertFileFromUrl(const QStringList& urls) {
    for (const auto& url : urls) {
        if (isImage(url)) {
            insertImage(url);
        } else {
            insertTextFile(url);
        }
    }
}

void MessageTextEdit::insertImage(const QString& url) {
    QImage image(url);
    // 按比例缩放图片
    if (image.width() > 120 || image.height() > 80) {
        if (image.width() > image.height()) {
            image = image.scaledToWidth(120, Qt::SmoothTransformation);
        } else
            image = image.scaledToHeight(80, Qt::SmoothTransformation);
    }
    QTextCursor cursor = this->textCursor();
    // QTextDocument *document = this->document();
    // document->addResource(QTextDocument::ImageResource, QUrl(url), QVariant(image));
    cursor.insertImage(image, url);

    msg_list_.emplace_back(MsgInfo::Type::kImage, url, QPixmap::fromImage(image));
}

void MessageTextEdit::insertTextFile(const QString& url) {
    QFileInfo fileInfo(url);
    if (fileInfo.isDir()) {
        QMessageBox::information(this, "提示", "只允许拖拽单个文件!");
        return;
    }

    if (fileInfo.size() > 100 * 1024 * 1024) {
        QMessageBox::information(this, "提示", "发送的文件大小不能大于100M");
        return;
    }

    QPixmap pix = getFileIconPixmap(url);
    QTextCursor cursor = this->textCursor();
    cursor.insertImage(pix.toImage(), url);
    msg_list_.emplace_back(MsgInfo::Type::kFile, url, std::move(pix));
}

QStringList MessageTextEdit::getUrl(const QString& text) {
    QStringList urls;
    if (text.isEmpty()) return urls;

    QStringList list = text.split("\n");
    for (const auto& url : list) {
        if (!url.isEmpty()) {
            QStringList str = url.split("///");
            if (str.size() >= 2) {
                urls.append(str.at(1));
            }
        }
    }
    return urls;
}

QPixmap MessageTextEdit::getFileIconPixmap(const QString& url) {
    QFileIconProvider provder;
    QFileInfo fileinfo(url);
    QIcon icon = provder.icon(fileinfo);

    QString strFileSize = getFileSize(fileinfo.size());

    QFont font(QString("宋体"), 10, QFont::Normal, false);
    QFontMetrics fontMetrics(font);
    QSize textSize = fontMetrics.size(Qt::TextSingleLine, fileinfo.fileName());

    QSize FileSize = fontMetrics.size(Qt::TextSingleLine, strFileSize);
    int maxWidth = textSize.width() > FileSize.width() ? textSize.width() : FileSize.width();
    QPixmap pix(50 + maxWidth + 10, 50);
    pix.fill();

    QPainter painter;
    // painter.setRenderHint(QPainter::Antialiasing, true);
    // painter.setFont(font);
    painter.begin(&pix);
    // 文件图标
    QRect rect(0, 0, 50, 50);
    painter.drawPixmap(rect, icon.pixmap(40, 40));
    painter.setPen(Qt::black);
    // 文件名称
    QRect rectText(50 + 10, 3, textSize.width(), textSize.height());
    painter.drawText(rectText, fileinfo.fileName());
    // 文件大小
    QRect rectFile(50 + 10, textSize.height() + 5, FileSize.width(), FileSize.height());
    painter.drawText(rectFile, strFileSize);
    painter.end();
    return pix;
}

QString MessageTextEdit::getFileSize(qint64 size) {
    QString Unit;
    double num;
    if (size < 1024) {
        num = size;
        Unit = "B";
    } else if (size < 1024 * 1224) {
        num = size / 1024.0;
        Unit = "KB";
    } else if (size < 1024 * 1024 * 1024) {
        num = size / 1024.0 / 1024.0;
        Unit = "MB";
    } else {
        num = size / 1024.0 / 1024.0 / 1024.0;
        Unit = "GB";
    }
    return QString::number(num, 'f', 2) + " " + Unit;
}