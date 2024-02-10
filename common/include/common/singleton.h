#pragma once

/// @brief 单例模板基类
/// 用法示例:
/// class MySingletonClass : public SingletonTemplate<MySingletonClass> {
///     friend class SingletonTemplate<MySingletonClass>; // 让基类可以访问派生类的私有构造函数
///
/// private:
///     // 私有化构造函数
///     MySingletonClass() {}
/// };
template <typename T>
class SingletonTemplate {
public:
    // 删除拷贝构造函数和赋值操作符以防止复制
    SingletonTemplate(const SingletonTemplate&) = delete;
    SingletonTemplate& operator=(const SingletonTemplate&) = delete;

    // 提供一个访问唯一实例的方法
    static T& Instance() {
        static T instance;  // 局部静态变量，只会被初始化一次
        return instance;
    }

protected:
    // 构造函数为 protected，以防止外部构造
    SingletonTemplate() = default;
};
