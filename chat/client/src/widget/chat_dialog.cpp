#include "chat/client/widget/chat_dialog.h"

#include <QAction>
#include <QRandomGenerator>

#include "chat/client/widget/chat_user_widget.h"
#include "chat/client/widget/ui_chat_dialog.h"

ChatDialog::ChatDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ChatDialog) {
    ui->setupUi(this);

    // 创建一个搜索图标
    QAction* search_action = new QAction(ui->search_edit);
    search_action->setIcon(QIcon("chat/client/res/search.png"));
    ui->search_edit->addAction(search_action, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(tr("搜索"));

    // 创建一个清除图标
    QAction* clear_action = new QAction(ui->search_edit);
    clear_action->setIcon(QIcon("chat/client/res/close_transparent.png"));
    ui->search_edit->addAction(clear_action, QLineEdit::TrailingPosition);
    // 有输入时显示清除图标
    connect(ui->search_edit, &QLineEdit::textChanged, [clear_action](const QString& text) {
        if (text.isEmpty()) {
            clear_action->setIcon(QIcon("chat/client/res/close_transparent.png"));
        } else {
            clear_action->setIcon(QIcon("chat/client/res/close_search.png"));
        }
    });
    // 点击清除图标清空输入框
    connect(clear_action, &QAction::triggered, [this]() {
        ui->search_edit->clear();
        ShowSearch(false);
    });

    ShowSearch(false);

    addChatUserList();  // TODO: 测试函数, 添加用户列表
}

ChatDialog::~ChatDialog() { delete ui; }

void ChatDialog::ShowSearch(bool is_search) {
    if (is_search) {
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        mode_ = UIMode::kSearchMode;
    } else if (state_ == UIMode::kChatMode) {
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
        mode_ = UIMode::kChatMode;
    } else if (state_ == UIMode::kContactMode) {
        ui->chat_user_list->hide();
        ui->con_user_list->show();
        ui->search_list->hide();
        mode_ = UIMode::kContactMode;
    }
}

static const std::vector<QString> strs = {"hello world !", "nice to meet u", "New year, new life",
                                          "You have to love yourself",
                                          "My love is written in the wind ever since the whole world is you"};
static const std::vector<QString> heads = {"chat/client/res/head_1.jpg", "chat/client/res/head_2.jpg",
                                           "chat/client/res/head_3.jpg", "chat/client/res/head_4.jpg",
                                           "chat/client/res/head_5.jpg"};
static const std::vector<QString> names = {"llfc", "zack", "golang", "cpp", "java", "nodejs", "python", "rust"};

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