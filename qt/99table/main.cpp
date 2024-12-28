#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>

class Label : public QLabel {
public:
    explicit Label(QWidget* parent = nullptr) : QLabel(parent) {
        setAlignment(Qt::AlignCenter);
        setStyleSheet(R"(
            QLabel {
                margin: 10px;
                font-size: 20px;
            }
        )");
    }
};

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("九九乘法表");
        setGeometry(1000, 1000, 600, 400);

        setCentralWidget(new QWidget);

        QGridLayout* layout = new QGridLayout;
        centralWidget()->setLayout(layout);

        for (int i = 1; i < 10; i++) {
            for (int j = 1; j < 10; j++) {
                auto label = new Label;
                label->setText(QString("%1 x %2 = %3").arg(i).arg(j).arg(i * j));
                layout->addWidget(label, i - 1, j - 1);
            }
        }
    }
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    MainWindow main_window;
    main_window.show();

    return app.exec();
}
