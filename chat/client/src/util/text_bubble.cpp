#include "chat/client/util/text_bubble.h"

#include <QEvent>
#include <QTextBlock>
#include <QTextLayout>

TextBubble::TextBubble(ChatRole role, const QString& text, QWidget* parent) : BubbleFrame(role, parent) {
    text_edit_ = new QTextEdit();
    text_edit_->setReadOnly(true);
    text_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    text_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    text_edit_->installEventFilter(this);
    QFont font("Microsoft YaHei");
    font.setPointSize(12);
    text_edit_->setFont(font);
    setPlainText(text);
    setWidget(text_edit_);
    initStyleSheet();
}

bool TextBubble::eventFilter(QObject* watched, QEvent* event) {
    if (text_edit_ == watched && event->type() == QEvent::Paint) {
        adjustTextHeight();
    }
    return BubbleFrame::eventFilter(watched, event);
}

void TextBubble::adjustTextHeight() {
    qreal doc_margin = text_edit_->document()->documentMargin();  // 字体到边框的距离默认为4
    QTextDocument* doc = text_edit_->document();
    qreal text_height = 0;
    // 把每一段的高度相加=文本高
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        QTextLayout* pLayout = it.layout();
        QRectF text_rect = pLayout->boundingRect();  // 这段的rect
        text_height += text_rect.height();
    }
    int vMargin = this->layout()->contentsMargins().top();
    // 设置这个气泡需要的高度 文本高+文本边距+TextEdit边框到气泡边框的距离
    setFixedHeight(text_height + doc_margin * 2 + vMargin * 2);
}

void TextBubble::setPlainText(const QString& text) {
    text_edit_->setPlainText(text);
    // 找到段落中最大宽度
    qreal doc_margin = text_edit_->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();
    QFontMetricsF fm(text_edit_->font());
    QTextDocument* doc = text_edit_->document();
    int max_width = 0;
    // 遍历每一段找到 最宽的那一段
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        int txtW = int(fm.horizontalAdvance(it.text()));  // 字体总长
        max_width = max_width < txtW ? txtW : max_width;  // 找到最长的那段
    }
    // 设置这个气泡的最大宽度 只需要设置一次
    constexpr int kOffset = 5;
    setMaximumWidth(max_width + doc_margin * 2 + (margin_left + margin_right) + kOffset);  // 设置最大宽度
}

void TextBubble::initStyleSheet() { text_edit_->setStyleSheet("QTextEdit{background:transparent;border:none}"); }
