#include "chat/client/user_mgr.h"

#include <algorithm>

bool UserMgr::AlreadyApply(int uid) const {
    return std::ranges::find_if(apply_list_, [uid](const std::shared_ptr<ApplyInfo>& apply) {
               return apply->uid == uid;
           }) != apply_list_.end();
}

void UserMgr::AppendApplyList(const QJsonArray& apply_list) {
    for (const QJsonValue& apply : apply_list) {
        auto apply_info = std::make_shared<ApplyInfo>(
            apply["uid"].toInt(), apply["sex"].toInt(), apply["name"].toString(), apply["nick"].toString(),
            apply["icon"].toString(), apply["desc"].toString(), apply["status"].toInt());
        apply_list_.push_back(apply_info);
    }
}
