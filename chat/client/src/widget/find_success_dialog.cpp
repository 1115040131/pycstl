#include "chat/client/widget/find_success_dialog.h"

#include "chat/client/widget/apply_friend_dialog.h"
#include "chat/client/widget/ui_find_success_dialog.h"

FindSuccessDialog::FindSuccessDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FindSuccessDialog) {
    ui->setupUi(this);

    // 隐藏对话框标题
    setWindowFlag(Qt::FramelessWindowHint);

    // this->setModal(true);
    setWindowModality(Qt::WindowModal);
}

FindSuccessDialog::~FindSuccessDialog() { delete ui; }

void FindSuccessDialog::setSearchInfo(const std::shared_ptr<SearchInfo>& search_info) {
    ui->name_label->setText(search_info->name);

    // 获取当前应用程序路径
    // QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path = search_info->icon;
    // 设置头像
    QPixmap head_pix(pix_path);
    ui->head_label->setPixmap(head_pix);
    ui->head_label->setScaledContents(true);

    search_info_ = search_info;
}

void FindSuccessDialog::on_add_friend_btn_clicked() {
    this->hide();

    // 弹出添加好友界面
    auto apply_friend_dialog = new ApplyFriendDialog(static_cast<QWidget*>(this->parent()));
    apply_friend_dialog->setSearchInfo(search_info_);
    apply_friend_dialog->setModal(true);
    apply_friend_dialog->show();
}
