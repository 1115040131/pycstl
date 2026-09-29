module;

#include <QString>
#include <QWidget>

export module chat.client.api;

/**
 * @brief 刷新 qss
 */
export void repolish(QWidget* w);

/**
 * @brief 简单的字符串加密
 */
export QString xorString(const QString& input);
