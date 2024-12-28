#pragma once

#include <QEvent>
#include <QMainWindow>
#include <QTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private Q_SLOTS:
    void on_open_triggered();

    void on_save_triggered();

    void on_saveAs_triggered();

    void on_autoNewLine_triggered(bool enable);

private:
    void setAutoNewLine(bool enable);

    bool checkUnsavedModify();

    void openFile(const QString& path);

    void saveFile(const QString& path);

private:
    QTextEdit* text_edit_;
    QString current_path_;
};
