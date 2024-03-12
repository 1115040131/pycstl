#pragma once

#include <functional>
#include <list>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class ThreadSafeHashTable : public Noncopyable {
private:
    class BucketType {
        friend class ThreadSafeHashTable;

        using BucketValue = std::pair<Key, Value>;                        // 存储元素类型
        using BucketData = std::list<BucketValue>;                        // 链表存储元素
        using BucketIterator = typename BucketData::iterator;             // 迭代器
        using ConstBucketIterator = typename BucketData::const_iterator;  // 迭代器

    public:
        /// @brief 查找 key 对应的 value, 如果不存在则返回默认值
        Value ValueFor(const Key& key, const Value& default_value = Value{}) const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto found_entry = FindEntryFor(key);
            return (found_entry == data_.end()) ? default_value : found_entry->second;
        }

        /// @brief 添加或更新 key 对应的 value
        void AddOrUpdateMapping(const Key& key, const Value& value) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            auto found_entry = FindEntryFor(key);
            if (found_entry == data_.end()) {
                data_.emplace_back(key, value);
            } else {
                found_entry->second = value;
            }
        }

        /// @brief 删除 key 对应的 value
        void RemoveMapping(const Key& key) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            auto found_entry = FindEntryFor(key);
            if (found_entry != data_.end()) {
                data_.erase(found_entry);
            }
        }

    private:
        BucketIterator FindEntryFor(const Key& key) {
            return std::find_if(data_.begin(), data_.end(),
                                [&](const BucketValue& item) { return item.first == key; });
        }

        ConstBucketIterator FindEntryFor(const Key& key) const {
            return std::find_if(data_.cbegin(), data_.cend(),
                                [&](const BucketValue& item) { return item.first == key; });
        }

    private:
        BucketData data_;
        mutable std::shared_mutex mutex_;
    };

public:
    ThreadSafeHashTable(std::size_t num_buckets = 19, const Hash& hasher = Hash{})
        : buckets_(num_buckets), hasher_(hasher) {}

    Value ValueFor(const Key& key, const Value& default_value = Value{}) const {
        return GetBucket(key).ValueFor(key, default_value);
    }

    void AddOrUpdateMapping(const Key& key, const Value& value) { GetBucket(key).AddOrUpdateMapping(key, value); }

    void RemoveMapping(const Key& key) { GetBucket(key).RemoveMapping(key); }

    std::unordered_map<Key, Value> GetMap() const {
        std::vector<std::unique_lock<std::shared_mutex>> locks;
        for (std::size_t i = 0; i < buckets_.size(); i++) {
            locks.emplace_back(buckets_[i].mutex_);
        }

        std::unordered_map<Key, Value> result;
        for (std::size_t i = 0; i < buckets_.size(); i++) {
            for (const auto& item : buckets_[i].data_) {
                result.insert(item);
            }
            for (auto it = buckets_[i].data_.begin(); it != buckets_[i].data_.end(); ++it) {
                result.insert(*it);
            }
        }

        return result;
    }

private:
    BucketType& GetBucket(const Key& key) {
        const std::size_t bucket_index = hasher_(key) % buckets_.size();
        return buckets_[bucket_index];
    }

    const BucketType& GetBucket(const Key& key) const {
        const std::size_t bucket_index = hasher_(key) % buckets_.size();
        return buckets_[bucket_index];
    }

private:
    std::vector<BucketType> buckets_;
    Hash hasher_;
};

}  // namespace concurrency
}  // namespace pyc
