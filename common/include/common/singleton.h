#pragma once

#include "common/noncopyable.h"

namespace pyc {

/// @brief 单例模板基类
/// 用法示例:
/// class MySingletonClass : public Singleton<MySingletonClass> {
///     friend class Singleton<MySingletonClass>; // 让基类可以访问派生类的私有构造函数
///
/// private:
///     // 私有化构造函数
///     MySingletonClass() = default;
/// };
template <typename T>
class Singleton : public Noncopyable {
public:
    // 提供一个访问唯一实例的方法
    static T& GetInstance() {
        static T instance;  // 局部静态变量，只会被初始化一次
        return instance;
    }

protected:
    // 构造函数为 protected，以防止外部构造
    Singleton() = default;
};

}  // namespace pyc