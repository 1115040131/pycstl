#pragma once

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
class Singleton {
public:
    // 删除拷贝构造函数和赋值操作符以防止复制
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 提供一个访问唯一实例的方法
    static T& Instance() {
        static T instance;  // 局部静态变量，只会被初始化一次
        return instance;
    }

protected:
    // 构造函数为 protected，以防止外部构造
    Singleton() = default;
};

}  // namespace pyc