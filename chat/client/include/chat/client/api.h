#pragma once

#include <QString>
#include <QWidget>

/**
 * @brief 刷新 qss
 */
void Repolish(QWidget* w);

/**
 * @brief 简单的字符串加密
 */
QString xorString(const QString& input);