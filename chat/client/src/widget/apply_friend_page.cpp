#include "chat/client/widget/apply_friend_page.h"

#include <QRandomGenerator>

#include "chat/client/tcp_mgr.h"
#include "chat/client/user_mgr.h"
#include "chat/client/widget/apply_friend_item.h"
#include "chat/client/widget/ui_apply_friend_page.h"

ApplyFriendPage::ApplyFriendPage(QWidget* parent) : QDialog(parent), ui(new Ui::ApplyFriendPage) {
    ui->setupUi(this);

    connect(ui->apply_friend_list, &ApplyFriendList::sig_show_search, this, &ApplyFriendPage::sig_show_search);
    loadApplyList();

    // 连接 authrsp 信号处理
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_auth_rsp, this, &ApplyFriendPage::slot_auth_rsp);
}

ApplyFriendPage::~ApplyFriendPage() { delete ui; }

void ApplyFriendPage::addNewApply(const std::shared_ptr<ApplyInfo>& apply) {
    // 先模拟头像随机
    int random_value = QRandomGenerator::global()->bounded(100);
    int head_index = random_value % heads.size();

    auto apply_item = new ApplyFriendItem;
    apply_item->setInfo(ApplyInfo{apply->uid, 0, apply->name, apply->nick, heads[head_index], apply->desc, 0});
    apply_item->showAddBtn(true);

    auto item = new QListWidgetItem;
    item->setSizeHint(apply_item->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

    ui->apply_friend_list->insertItem(0, item);
    ui->apply_friend_list->setItemWidget(item, apply_item);

    // 收到审核好友信号
    // connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](ApplyInfo apply_info) {});
}

void ApplyFriendPage::loadApplyList() {
    const auto& apply_list = UserMgr::GetInstance().GetApplyList();
    for (const auto& [_, apply] : apply_list) {
        int random_value = QRandomGenerator::global()->bounded(100);
        int head_index = random_value % heads.size();
        apply->icon = heads[head_index];

        auto apply_item = new ApplyFriendItem;
        apply_item->setInfo(*apply);

        auto item = new QListWidgetItem;
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

        ui->apply_friend_list->insertItem(0, item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        if (apply->status) {
            apply_item->showAddBtn(false);
        } else {
            apply_item->showAddBtn(true);
            unauth_items_[apply_item->getUid()] = apply_item;
        }

        // 收到审核好友信号
        // connect(apply_item,&ApplyFriendItem::sig_auth_friend, [this](const ApplyInfo& apply_info){
        // });
    }

    // 模拟假数据
    for (int i = 0; i < 13; i++) {
        int random_value = QRandomGenerator::global()->bounded(100);
        int name_index = random_value % names.size();
        int head_index = random_value % heads.size();
        int msg_index = random_value % strs.size();

        auto apply_item = new ApplyFriendItem;
        apply_item->setInfo(
            ApplyInfo{0, 0, names[name_index], names[name_index], heads[head_index], strs[msg_index], 1});
        if (i < 3) {
            apply_item->showAddBtn(true);
        }

        auto item = new QListWidgetItem;
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);

        ui->apply_friend_list->addItem(item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        // 收到审核好友信号
        // connect(apply_item,&ApplyFriendItem::sig_auth_friend, [this](const ApplyInfo& apply_info){
        // });
    }
}

void ApplyFriendPage::slot_auth_rsp(const AuthRsp& auth_rsp) {
    auto uid = auth_rsp.uid;
    auto iter = unauth_items_.find(uid);
    if (iter == unauth_items_.end()) {
        return;
    }
    iter->second->showAddBtn(false);
}
