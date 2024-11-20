#include "chat/client/widget/apply_friend_dialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>

#include "chat/client/tcp_mgr.h"
#include "chat/client/user_mgr.h"
#include "chat/client/widget/ui_apply_friend_dialog.h"

inline constexpr int kMinApplyLbaelEditLength = 40;  // 申请好友标签输入框最低长度

inline constexpr std::string_view kAddPrefix = "添加标签";

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

    connect(ui->more_label, &ClickedOnceLabel::clicked, this, &ApplyFriendDialog::slot_show_more_label);

    initTipLabels();

    // 连接输入标签事件
    connect(ui->label_edit, &CustomizeEdit::returnPressed, this, &ApplyFriendDialog::slot_label_enter);
    connect(ui->label_edit, &CustomizeEdit::textChanged, this, &ApplyFriendDialog::slot_label_text_change);
    connect(ui->label_edit, &CustomizeEdit::editingFinished, this, &ApplyFriendDialog::slot_label_edit_finished);
    connect(ui->tip_label, &ClickedOnceLabel::clicked, this,
            &ApplyFriendDialog::slot_add_friend_label_by_click_tip);

    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);

    // 连接确认和取消按钮的槽函数
    ui->sure_btn->setState("normal", "hover", "press");
    ui->cancel_btn->setState("normal", "hover", "press");
    connect(ui->sure_btn, &ClickedBtn::clicked, this, &ApplyFriendDialog::slot_apply_sure);
    connect(ui->cancel_btn, &ClickedBtn::clicked, this, &ApplyFriendDialog::slot_apply_cancel);
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

ApplyFriendDialog::TipLabelInfo ApplyFriendDialog::makeNewTipLabel(const QString& text, QWidget* parent) {
    auto label = new ClickedLabel(parent);
    label->setState("normal", "hover", "pressed", "selected_normal", "selected_hover", "selected_pressed");
    label->setObjectName("tipslb");
    label->setText(text);
    connect(label, &ClickedLabel::clicked, this, &ApplyFriendDialog::slot_change_friend_label_by_tip);

    QFontMetrics fontMetrics(label->font());                        // 获取QLabel控件的字体信息
    int text_width = fontMetrics.horizontalAdvance(label->text());  // 获取文本的宽度
    int text_height = fontMetrics.height();                         // 获取文本的高度

    return {label, text_width, text_height};
}

void ApplyFriendDialog::initTipLabels() {
    // 模拟创建多个标签展示
    tip_data_ = {"同学",          "家人",           "菜鸟教程",       "C++ Primer",
                 "Rust 程序设计", "父与子学Python", "nodejs开发指南", "go 语言开发指南",
                 "游戏伙伴",      "金融投资",       "微信读书",       "拼多多拼友"};

    int lines = 1;

    for (const auto& tip : tip_data_) {
        auto [label, text_width, text_height] = makeNewTipLabel(tip, ui->label_list);

        if (tip_cur_point_.x() + text_width + kTipOffset > ui->label_list->width()) {
            lines++;
            if (lines > 2) {
                delete label;
                return;
            }

            tip_cur_point_.setX(kTipOffset);
            tip_cur_point_.setY(tip_cur_point_.y() + text_height + 15);
        }

        tip_cur_point_ = addTipLabel(label, tip_cur_point_, text_width, text_height);
    }
}

QPoint ApplyFriendDialog::addTipLabel(ClickedLabel* tip_label, const QPoint& cur_point, int text_width, int) {
    tip_label->move(cur_point);
    tip_label->show();

    add_labels_.emplace(tip_label->text(), tip_label);
    add_label_keys.push_back(tip_label->text());

    QPoint next_point(tip_label->x() + text_width + 15, tip_label->y());

    return next_point;
}

void ApplyFriendDialog::resetLabels() {
    if (friend_labels_.empty()) {
        ui->label_edit->move(label_point_);
        return;
    }

    auto max_width = ui->grid_widget->width();
    auto label_height = 0;

    for (const auto& friend_label_key : friend_label_keys_) {
        auto friend_label = friend_labels_[friend_label_key];
        if (label_point_.x() + friend_label->width() > max_width) {
            label_point_.setX(2);
            label_point_.setY(label_point_.y() + friend_label->height() + 6);
        }

        friend_label->move(label_point_);
        friend_label->show();

        label_point_.setX(label_point_.x() + friend_label->width() + 2);
        label_point_.setY(label_point_.y());
        label_height = friend_label->height();
    }

    if (label_point_.x() + kMinApplyLbaelEditLength > ui->grid_widget->width()) {
        ui->label_edit->move(2, label_point_.y() + label_height + 6);
    } else {
        ui->label_edit->move(label_point_);
    }
}

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
        label_point_.setX(2);
        label_point_.setY(label_point_.y() + friend_label->height() + 6);
    }
    friend_label->move(label_point_);
    friend_label->show();
    friend_labels_[name] = friend_label;
    friend_label_keys_.push_back(name);

    connect(friend_label, &FriendLabel::sig_close, this, &ApplyFriendDialog::slot_remove_friend_label);

    // 将 edit 移动到添加的标签后面
    label_point_.setX(label_point_.x() + friend_label->width() + 2);

    if (label_point_.x() + kMinApplyLbaelEditLength > ui->grid_widget->width()) {
        label_point_.setX(2);
        label_point_.setY(label_point_.y() + friend_label->height() + 2);
    }

    ui->label_edit->move(label_point_);
    ui->label_edit->clear();

    // grid_widget 换行
    if (ui->grid_widget->height() < label_point_.y() + friend_label->height() + 13) {
        int diff_height = label_point_.y() + friend_label->height() + 13 - ui->grid_widget->height();
        ui->grid_widget->setFixedHeight(label_point_.y() + friend_label->height() + 13);
        ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
    }
}

void ApplyFriendDialog::slot_show_more_label() {
    ui->more_label_widget->hide();
    ui->label_list->setFixedWidth(343);
    tip_cur_point_ = QPoint(kTipOffset, 5);

    // 重排现有的label
    int text_width;
    int text_height;
    for (const auto& key : add_label_keys) {
        auto label = add_labels_[key];

        QFontMetrics fontMetrics(label->font());                    // 获取QLabel控件的字体信息
        text_width = fontMetrics.horizontalAdvance(label->text());  // 获取文本的宽度
        text_height = fontMetrics.height();                         // 获取文本的高度

        if (tip_cur_point_.x() + text_width + kTipOffset > ui->label_list->width()) {
            tip_cur_point_.setX(kTipOffset);
            tip_cur_point_.setY(tip_cur_point_.y() + text_height + 15);
        }
        label->move(tip_cur_point_);

        tip_cur_point_.setX(label->x() + text_width + 15);
    }

    // 添加未添加的
    for (const auto& tip : tip_data_) {
        if (add_labels_.count(tip) > 0) {
            continue;
        }

        auto label_info = makeNewTipLabel(tip, ui->label_list);
        auto label = label_info.label;
        text_width = label_info.text_width;
        text_height = label_info.text_height;

        if (tip_cur_point_.x() + text_width + kTipOffset > ui->label_list->width()) {
            tip_cur_point_.setX(kTipOffset);
            tip_cur_point_.setY(tip_cur_point_.y() + text_height + 15);
        }

        tip_cur_point_ = addTipLabel(label, tip_cur_point_, text_width, text_height);
    }

    int diff_height = tip_cur_point_.y() + text_height + 15 - ui->label_list->height();
    ui->label_list->setFixedHeight(tip_cur_point_.y() + text_height + 15);
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDialog::slot_label_enter() {
    auto text = ui->label_edit->text();
    if (text.isEmpty()) {
        return;
    }

    addLabel(text);

    ui->input_tip_widget->hide();

    if (std::find(tip_data_.begin(), tip_data_.end(), text) == tip_data_.end()) {
        tip_data_.push_back(text);
    }

    // 判断标签展示栏是否有该标签
    auto iter = add_labels_.find(text);
    if (iter != add_labels_.end()) {
        iter->second->setSelected(ClickedLabel::State::kSelected);
        return;
    }

    // 标签展示栏也增加一个标签, 并设置绿色选中
    auto [label, text_width, text_height] = makeNewTipLabel(text, ui->label_list);
    label->setSelected(ClickedLabel::State::kSelected);

    if (tip_cur_point_.x() + text_width + kTipOffset > ui->label_list->width()) {
        tip_cur_point_.setX(kTipOffset);
        tip_cur_point_.setY(tip_cur_point_.y() + text_height + 15);
    }

    tip_cur_point_ = addTipLabel(label, tip_cur_point_, text_width, text_height);

    int diff_height = tip_cur_point_.y() + text_height + 15 - ui->label_list->height();
    ui->label_list->setFixedHeight(tip_cur_point_.y() + text_height + 15);
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

void ApplyFriendDialog::slot_remove_friend_label(const QString& name) {
    label_point_.setX(2);
    label_point_.setY(6);

    if (auto iter = friend_labels_.find(name); iter == friend_labels_.end()) {
        return;
    } else {
        friend_labels_.erase(iter);
        delete iter->second;
    }

    std::erase_if(friend_label_keys_, [&](const QString& text) { return text == name; });

    resetLabels();

    if (auto iter = add_labels_.find(name); iter == add_labels_.end()) {
        return;
    } else {
        iter->second->setSelected(ClickedLabel::State::kNormal);
    }
}

void ApplyFriendDialog::slot_change_friend_label_by_tip(const QString& name, ClickedLabel::State state) {
    auto iter = add_labels_.find(name);
    if (iter == add_labels_.end()) {
        return;
    }

    if (state == ClickedLabel::State::kSelected) {
        addLabel(name);
    } else {
        slot_remove_friend_label(name);
    }
}

void ApplyFriendDialog::slot_label_text_change(const QString& text) {
    if (text.isEmpty()) {
        ui->input_tip_widget->hide();
        return;
    }

    auto iter = std::find(tip_data_.begin(), tip_data_.end(), text);
    if (iter == tip_data_.end()) {
        ui->tip_label->setText(kAddPrefix.data() + text);
    } else {
        ui->tip_label->setText(*iter);
    }
    ui->input_tip_widget->show();
}

void ApplyFriendDialog::slot_label_edit_finished() { ui->input_tip_widget->hide(); }

void ApplyFriendDialog::slot_add_friend_label_by_click_tip(const QString&) { slot_label_enter(); }

void ApplyFriendDialog::slot_apply_sure() {
    qDebug() << "Slot Apply Sure";
    QJsonObject root;

    auto uid = UserMgr::GetInstance().GetUid();
    root["uid"] = uid;

    auto name = !ui->name_edit->text().isEmpty() ? ui->name_edit->text() : ui->name_edit->placeholderText();
    root["apply_name"] = name;

    auto back_name = !ui->back_edit->text().isEmpty() ? ui->back_edit->text() : ui->back_edit->placeholderText();
    root["back_name"] = back_name;

    root["to_uid"] = search_info_->uid;

    QJsonDocument doc(root);

    emit TcpMgr::GetInstance().sig_send_data(ReqId::kAddFriendReq, doc.toJson(QJsonDocument::Compact));
    this->hide();
    deleteLater();
}

void ApplyFriendDialog::slot_apply_cancel() {
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}
