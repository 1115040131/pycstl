#include "chat/client/widget/apply_friend_page.h"

#include <QRandomGenerator>

#include "chat/client/tcp_mgr.h"
#include "chat/client/user_mgr.h"
#include "chat/client/widget/apply_friend_item.h"
#include "chat/client/widget/auth_friend_dialog.h"
#include "chat/client/widget/ui_apply_friend_page.h"

ApplyFriendPage::ApplyFriendPage(QWidget* parent) : QDialog(parent), ui(new Ui::ApplyFriendPage) {
    ui->setupUi(this);

    connect(ui->apply_friend_list, &ApplyFriendList::sig_show_search, this, &ApplyFriendPage::sig_show_search);
    loadApplyList();

    // 连接 auth_rsp 信号处理
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_auth_rsp, this, &ApplyFriendPage::slot_auth_rsp);
}

ApplyFriendPage::~ApplyFriendPage() { delete ui; }

void ApplyFriendPage::addNewApply(const std::shared_ptr<ApplyInfo>& apply_info) {
    // 先模拟头像随机
    int random_value = QRandomGenerator::global()->bounded(100);
    int head_index = random_value % heads.size();
    apply_info->icon = heads[head_index];

    auto apply_item = new ApplyFriendItem;
    apply_item->setApplyInfo(apply_info);
    apply_item->showAddBtn(true);

    auto item = new QListWidgetItem;
    item->setSizeHint(apply_item->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

    ui->apply_friend_list->insertItem(0, item);
    ui->apply_friend_list->setItemWidget(item, apply_item);

    // 收到审核好友信号
    connect(apply_item, &ApplyFriendItem::sig_auth_friend, this, &ApplyFriendPage::slot_auth_friend);
}

void ApplyFriendPage::loadApplyList() {
    const auto& apply_list = UserMgr::GetInstance().GetApplyList();
    for (const auto& apply_info : apply_list) {
        int random_value = QRandomGenerator::global()->bounded(100);
        int head_index = random_value % heads.size();
        apply_info->icon = heads[head_index];

        auto apply_item = new ApplyFriendItem;
        apply_item->setApplyInfo(apply_info);

        auto item = new QListWidgetItem;
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

        ui->apply_friend_list->insertItem(0, item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        if (apply_info->status) {
            apply_item->showAddBtn(false);
        } else {
            apply_item->showAddBtn(true);
            unauth_items_[apply_item->getUid()] = apply_item;
        }

        // 收到审核好友信号
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, this, &ApplyFriendPage::slot_auth_friend);
    }

    // 模拟假数据
    for (int i = 0; i < 13; i++) {
        int random_value = QRandomGenerator::global()->bounded(100);
        int name_index = random_value % names.size();
        int head_index = random_value % heads.size();
        int msg_index = random_value % strs.size();

        auto apply_item = new ApplyFriendItem;
        auto apply_info = std::make_shared<ApplyInfo>(0, 0, names[name_index], names[name_index],
                                                      heads[head_index], strs[msg_index], 1);
        apply_item->setApplyInfo(apply_info);
        if (i < 3) {
            apply_item->showAddBtn(true);
        }

        auto item = new QListWidgetItem;
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

        ui->apply_friend_list->addItem(item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        // 收到审核好友信号
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, this, &ApplyFriendPage::slot_auth_friend);
    }
}

void ApplyFriendPage::slot_auth_friend(const std::shared_ptr<ApplyInfo>& apply_info) {
    auto auth_friend_dialog = new AuthFriendDialog(this);
    auth_friend_dialog->setModal(true);
    auth_friend_dialog->setApplyInfo(apply_info);
    auth_friend_dialog->show();
}

void ApplyFriendPage::slot_auth_rsp(const std::shared_ptr<AuthInfo>& auth_rsp) {
    auto uid = auth_rsp->uid;
    auto iter = unauth_items_.find(uid);
    if (iter == unauth_items_.end()) {
        return;
    }
    iter->second->showAddBtn(false);
}
