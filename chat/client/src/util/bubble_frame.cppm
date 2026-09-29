module;

#include <QFrame>
#include <QHBoxLayout>

export module chat.client.util.bubble_frame;

export import chat.client.define;

export class BubbleFrame : public QFrame {
public:
    explicit BubbleFrame(ChatRole role, QWidget* parent = nullptr);

    void setMargin(int margin);

    void setWidget(QWidget* w);

private:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    ChatRole role_;
    int margin_;
    QHBoxLayout* h_layout_;
};
