#include "chat/client/widget/chat_dialog.h"

#include <QEvent>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QTimer>

#include "chat/client/tcp_mgr.h"
#include "chat/client/user_mgr.h"
#include "chat/client/widget/chat_user_widget.h"
#include "chat/client/widget/loading_dialog.h"
#include "chat/client/widget/ui_chat_dialog.h"

ChatDialog::ChatDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ChatDialog) {
    ui->setupUi(this);

    // 创建一个搜索图标
    search_action_ = new QAction(ui->search_edit);
    search_action_->setIcon(QIcon("chat/client/res/search.png"));
    ui->search_edit->addAction(search_action_, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(tr("搜索"));

    // 创建一个清除图标
    clear_action_ = new QAction(ui->search_edit);
    clear_action_->setIcon(QIcon("chat/client/res/close_transparent.png"));
    ui->search_edit->addAction(clear_action_, QLineEdit::TrailingPosition);
    // 链接搜索框输入变化
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_search_text_changed);
    // 点击清除图标清空输入框
    connect(clear_action_, &QAction::triggered, [this]() {
        ui->search_edit->clear();
        ShowSearch(false);
    });

    ui->search_edit->setMaxLength(15);
    ShowSearch(false);

    // 为searchlist 设置search edit
    ui->search_list->setSearchEdit(ui->search_edit);

    // 连接动态加载信号
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this, &ChatDialog::slot_loading_chat_user);
    addChatUserList();  // TODO: 测试函数, 添加用户列表

    // 侧边栏
    QPixmap pixmap("chat/client/res/head_1.jpg");
    ui->side_head_label->setPixmap(pixmap);
    ui->side_head_label->setScaledContents(true);  // 自动缩放图片内容以适应
    // QPixmap scaled_pixmap = pixmap.scaled(ui->side_head_label->size(), Qt::KeepAspectRatio);
    // ui->side_head_label->setPixmap(scaled_pixmap);

    ui->side_chat_label->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                                  "selected_pressed");
    ui->side_chat_label->setSelected(ClickedLabel::State::kSelected);
    ui->side_contact_label->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                                     "selected_pressed");

    addLabelGroup(ui->side_chat_label);
    addLabelGroup(ui->side_contact_label);

    connect(ui->side_chat_label, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_label, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);

    // 检测点击鼠标位置是否需要清空搜索框
    this->installEventFilter(this);

    // 收到好友请求
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_friend_apply, this, &ChatDialog::slot_friend_apply);
}

ChatDialog::~ChatDialog() { delete ui; }

bool ChatDialog::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto mouse_event = static_cast<QMouseEvent*>(event);
        handleGlobalMousePress(mouse_event);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::ShowSearch(bool is_search) {
    if (is_search) {
        ui->chat_user_list->hide();
        ui->contact_user_list->hide();
        ui->search_list->show();
        mode_ = UIMode::kSearchMode;
    } else if (state_ == UIMode::kChatMode) {
        ui->chat_user_list->show();
        ui->contact_user_list->hide();
        ui->search_list->hide();
        mode_ = UIMode::kChatMode;
    } else if (state_ == UIMode::kContactMode) {
        ui->chat_user_list->hide();
        ui->contact_user_list->show();
        ui->search_list->hide();
        mode_ = UIMode::kContactMode;
    }
}

void ChatDialog::addChatUserList() {
    for (int i = 0; i < 13; i++) {
        int random_value = QRandomGenerator::global()->bounded(100);
        int name_index = random_value % names.size();
        int head_index = random_value % heads.size();
        int msg_index = random_value % strs.size();

        auto* chat_user_widget = new ChatUserWidget();
        chat_user_widget->SetInfo(names[name_index], heads[head_index], strs[msg_index]);
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(chat_user_widget->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_widget);
    }
}

void ChatDialog::addLabelGroup(StateWidget* label) { labels_.push_back(label); }

void ChatDialog::clearLabelState(StateWidget* select_label) {
    for (const auto& label : labels_) {
        if (label != select_label) {
            label->setSelected(ClickedLabel::State::kNormal);
        } else {
            label->setSelected(ClickedLabel::State::kSelected);
        }
    }
}

void ChatDialog::handleGlobalMousePress(QMouseEvent* event) {
    // 判断是否处于搜索模式
    if (mode_ != UIMode::kSearchMode) {
        return;
    }

    // 判断鼠标点击位置是否在搜索列表外
    QPoint position = ui->search_list->mapFromGlobal(event->globalPosition().toPoint());
    if (!ui->search_list->rect().contains(position)) {
        ui->search_edit->clear();
        ShowSearch(false);
    }
}

void ChatDialog::slot_search_text_changed(const QString& text) {
    if (text.isEmpty()) {
        clear_action_->setIcon(QIcon("chat/client/res/close_transparent.png"));
    } else {
        clear_action_->setIcon(QIcon("chat/client/res/close_search.png"));
        ShowSearch(true);
    }
}

void ChatDialog::slot_loading_chat_user() {
    if (is_loading_) {
        return;
    }

    is_loading_ = true;

    LoadingDialog* loading_dialog = new LoadingDialog(this);          // 将当前对话框设置为父对象
    loading_dialog->setAttribute(Qt::WA_DeleteOnClose);               // 对话框关闭时自动删除
    loading_dialog->show();                                           // 显示悬浮对话框
    QTimer::singleShot(500, loading_dialog, &LoadingDialog::accept);  // TODO: for debug
    addChatUserList();
    // loading_dialog->deleteLater();  // 加载完成后关闭对话框

    is_loading_ = false;
}

void ChatDialog::slot_side_chat() {
    qDebug() << "receive side chat clicked";
    clearLabelState(ui->side_chat_label);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    state_ = UIMode::kChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_contact() {
    qDebug() << "receive side contact clicked";
    clearLabelState(ui->side_contact_label);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    state_ = UIMode::kContactMode;
    ShowSearch(false);
    ui->side_contact_label->ShowRedPoint(false);  // 消除红点
}

void ChatDialog::slot_friend_apply(std::shared_ptr<ApplyInfo> apply) {
    if (UserMgr::GetInstance().AlreadyApply(apply->uid)) {
        return;
    }
    UserMgr::GetInstance().AddApplyList(apply->uid, apply);
    ui->side_contact_label->ShowRedPoint(true);
    ui->contact_user_list->showRedPoint(true);
    ui->friend_apply_page->addNewApply(apply);
}
