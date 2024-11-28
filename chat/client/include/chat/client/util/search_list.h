#pragma once

#include <QDialog>
#include <QListWidget>
#include <memory>

#include "chat/client/user_data.h"

class CustomizeEdit;
class LoadingDialog;

class SearchList : public QListWidget {
    Q_OBJECT

public:
    explicit SearchList(QWidget* parent = nullptr);

    void setSearchEdit(CustomizeEdit* edit);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void waitPending(bool pending);

    void addTipItem();

    // 清除弹出框
    void closeFindDialog();

signals:
    void sig_jump_chat_item(const std::shared_ptr<SearchInfo>& search_info);

private slots:
    void slot_item_clicked(QListWidgetItem* item);

    void slot_user_search(const std::shared_ptr<SearchInfo>& search_info);

private:
    bool send_pending_ = false;
    CustomizeEdit* search_edit_;
    LoadingDialog* loading_dialog_;
    std::unique_ptr<QDialog> find_dialog_;
};