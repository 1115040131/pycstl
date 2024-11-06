#include "chat/client/widget/apply_friend_dialog.h"

#include <QScrollBar>

#include "chat/client/user_mgr.h"
#include "chat/client/widget/ui_apply_friend_dialog.h"

ApplyFriendDialog::ApplyFriendDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ApplyFriendDialog) {
    ui->setupUi(this);

    // 隐藏对话框标题
    setWindowFlag(Qt::FramelessWindowHint);

    this->setModal(true);
    ui->name_edit->setPlaceholderText(tr("梦的延续"));
    ui->label_edit->setPlaceholderText(tr("搜索、添加标签"));
    ui->back_edit->setPlaceholderText(tr("虚妄的诺言"));

    ui->label_edit->SetMaxLength(21);
    ui->label_edit->move(2, 2);
    ui->label_edit->setFixedHeight(20);
    ui->label_edit->setMaxLength(10);

    ui->input_tip_widget->hide();  // 输入标签后再显示搜索框

    tip_cur_point_ = QPoint(5, 5);

    tip_data_ = {"同学",          "家人",           "菜鸟教程",       "C++ Primer",
                 "Rust 程序设计", "父与子学Python", "nodejs开发指南", "go 语言开发指南",
                 "游戏伙伴",      "金融投资",       "微信读书",       "拼多多拼友"};

    connect(ui->more_label, &ClickedOnceLabel::clicked, this, &ApplyFriendDialog::slot_show_more_label);

    initTipLabels();

    connect(ui->label_edit, &CustomizeEdit::returnPressed, this, &ApplyFriendDialog::slot_label_enter);
    connect(ui->label_edit, &CustomizeEdit::textChanged, this, &ApplyFriendDialog::slot_label_text_change);
    connect(ui->label_edit, &CustomizeEdit::editingFinished, this, &ApplyFriendDialog::slot_label_edit_finished);
    // connect(ui->tip_label, &ClickedOnceLabel::clicked, this,
    //         &ApplyFriendDialog::slot_add_friend_label_by_click_tip);
}

ApplyFriendDialog::~ApplyFriendDialog() { delete ui; }

void ApplyFriendDialog::setSearchInfo(const std::shared_ptr<SearchInfo>& search_info) {
    search_info_ = search_info;
    ui->name_edit->setText(UserMgr::GetInstance().GetName());
    ui->back_edit->setText(search_info->name);
}

bool ApplyFriendDialog::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->scrollArea) {
        if (event->type() == QEvent::Enter) {
            ui->scrollArea->verticalScrollBar()->setHidden(false);
        } else if (event->type() == QEvent::Leave) {
            ui->scrollArea->verticalScrollBar()->setHidden(true);
        }
    }
    return QDialog::eventFilter(watched, event);
}

void ApplyFriendDialog::initTipLabels() {}

void ApplyFriendDialog::addTipLabel(ClickedLabel* tip_label, const QPoint& cur_point, const QPoint& next_point,
                                    int text_width, int text_height) {
    (void)tip_label;
    (void)cur_point;
    (void)next_point;
    (void)text_width;
    (void)text_height;
}

void ApplyFriendDialog::resetLabels() {}

void ApplyFriendDialog::addLabel(const QString& name) {
    if (friend_labels_.count(name) > 0) {
        return;
    }

    auto friend_label = new FriendLabel(ui->grid_widget);
    friend_label->setText(name);
    friend_label->setObjectName("FriendLabel");

    auto max_width = ui->grid_widget->width();
    // friend_label 换行
    if (label_point_.x() + friend_label->width() > max_width) {
        label_point_.setY(label_point_.y() + friend_label->height() + 6);
        label_point_.setX(2);
    }
    friend_label->move(label_point_);
    friend_label->show();
    friend_labels_[name] = friend_label;

    connect(friend_label, &FriendLabel::sig_close, this, &ApplyFriendDialog::slot_remove_friend_label);

    // 将 edit 移动到添加的标签后面
    label_point_.setX(label_point_.x() + friend_label->width() + 2);

    constexpr int kMinApplyLbaelEditLength = 40;  // 申请好友标签输入框最低长度
    if (label_point_.x() + kMinApplyLbaelEditLength > ui->grid_widget->width()) {
        ui->label_edit->move(2, label_point_.y() + friend_label->height() + 2);
    } else {
        ui->label_edit->move(label_point_);
    }

    ui->label_edit->clear();

    // grid_widget 换行
    if (ui->grid_widget->height() < label_point_.y() + friend_label->height() + 2) {
        ui->grid_widget->setFixedHeight(label_point_.y() + friend_label->height() * 2 + 2);
    }
}

void ApplyFriendDialog::slot_show_more_label() {}

void ApplyFriendDialog::slot_label_enter() {
    if (ui->label_edit->text().isEmpty()) {
        return;
    }

    addLabel(ui->label->text());

    ui->input_tip_widget->hide();
}

void ApplyFriendDialog::slot_remove_friend_label(const QString& name) { (void)name; }

void ApplyFriendDialog::slot_change_friend_label_by_tip(const QString& name, ClickedLabel::State state) {
    (void)name;
    (void)state;
}

void ApplyFriendDialog::slot_label_text_change(const QString& text) { (void)text; }

void ApplyFriendDialog::slot_label_edit_finished() {}

void ApplyFriendDialog::slot_add_friend_label_by_click_tip(const QString& text) { (void)text; }

void ApplyFriendDialog::slot_apply_sure() {}

void ApplyFriendDialog::slot_apply_cancel() {}
