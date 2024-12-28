#include "notepad/mainwindow.h"

#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    text_edit_ = new QTextEdit;
    text_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(text_edit_);

    QSettings settings("pycstl", "notepad");
    bool enable_auto_new_line = settings.value("enable_auto_new_line", true).toBool();

    QMenuBar* menu_bar = new QMenuBar;
    setMenuBar(menu_bar);

    QMenu* menu = menu_bar->addMenu("文件");
    menu->addAction("打开")->setObjectName("open");
    menu->addSeparator();
    menu->addAction("保存")->setObjectName("save");
    menu->addAction("另保存")->setObjectName("saveAs");

    menu = menu_bar->addMenu("编辑");
    QAction* action = menu->addAction("自动换行");
    action->setObjectName("autoNewLine");
    action->setCheckable(true);
    action->setChecked(enable_auto_new_line);
    setAutoNewLine(enable_auto_new_line);

    QMetaObject::connectSlotsByName(this);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (checkUnsavedModify()) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::on_open_triggered() {
    if (checkUnsavedModify()) {
        return;
    }

    auto path = QFileDialog::getOpenFileName(this, "打开文件", "qt/notepad/res", "文本文件(*.txt);;所有文件(*)");
    if (path.isEmpty()) {
        return;
    }

    openFile(path);
}

void MainWindow::on_save_triggered() {
    if (current_path_.isEmpty()) {
        on_saveAs_triggered();
    } else {
        saveFile(current_path_);
    }
}

void MainWindow::on_saveAs_triggered() {
    auto path = QFileDialog::getSaveFileName(this, "另保存文件", "qt/notepad/res", "");
    if (path.isEmpty()) {
        return;
    }

    saveFile(path);
}

void MainWindow::on_autoNewLine_triggered(bool enable) {
    setAutoNewLine(enable);
    QSettings settings("pycstl", "notepad");
    settings.setValue("enable_auto_new_line", enable);
}

void MainWindow::setAutoNewLine(bool enable) {
    if (enable) {
        text_edit_->setLineWrapMode(QTextEdit::WidgetWidth);
    } else {
        text_edit_->setLineWrapMode(QTextEdit::NoWrap);
    }
}

bool MainWindow::checkUnsavedModify() {
    if (text_edit_->document()->isModified()) {
        auto btn = QMessageBox::question(this, "notepad", "您有未保存的更改，是否保存？",
                                         QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (btn) {
            case QMessageBox::Save:
                on_save_triggered();
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Cancel:
                return true;
            default:
                break;
        }
    }
    return false;
}

void MainWindow::openFile(const QString& path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open file";
        return;
    }

    QString text = QString::fromUtf8(file.readAll());
    text_edit_->setPlainText(text);
    current_path_ = path;
    text_edit_->document()->setModified(false);
}

void MainWindow::saveFile(const QString& path) {
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Failed to save file";
        return;
    }

    file.write(text_edit_->toPlainText().toUtf8());
    current_path_ = path;
    text_edit_->document()->setModified(false);
}
