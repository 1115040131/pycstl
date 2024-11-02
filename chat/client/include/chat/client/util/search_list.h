#pragma once

#include <QListWidget>

#include "chat/client/search_info.h"


class SearchList : public QListWidget {
    Q_OBJECT

public:
    explicit SearchList(QWidget* parent = nullptr);

    void closeFindDialog();

    void setSearchEdit(QWidget* edit);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void waitPending(bool pending);

    void addTipItem();

private slots:
    void slot_item_clicked(QListWidgetItem* item);

    void slot_user_search(const SearchInfo& search_info);

private:
    bool send_pending_ = false;
};