#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QValidator>
#include <vector>

class PushButton : public QPushButton {
public:
    PushButton(QWidget* parent = nullptr) : QPushButton(parent) { setMinimumHeight(25); }
};

class MainWindow : public QMainWindow {
    enum class Operator {
        kNone,
        kPlus,
        kMinus,
        kMultiply,
        kDivide,
    };

public:
    explicit MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("计算器");

        setCentralWidget(new QWidget);

        QGridLayout* layout = new QGridLayout;
        centralWidget()->setLayout(layout);

        // 表达式
        equation_ = new QLabel;
        equation_->setText("");
        layout->addWidget(equation_, 0, 0, 1, 5);

        // 输入框
        line_edit_ = new QLineEdit;
        line_edit_->setText("0");
        line_edit_->setValidator(new QDoubleValidator);
        layout->addWidget(line_edit_, 1, 0, 1, 5);

        // 数字键 1~9
        for (int i = 1; i < 10; i++) {
            PushButton* button = new PushButton;
            button->setText(QString::number(i));
            layout->addWidget(button, 2 + (i - 1) / 3, (i - 1) % 3);
            connect(button, &PushButton::clicked, this, [this, i] { this->pressNumber(i); });
        }

        // 正负号
        {
            PushButton* button = new PushButton;
            button->setText("+/-");
            layout->addWidget(button, 5, 0);
            connect(button, &PushButton::clicked, this, &MainWindow::pressSign);
        }

        // 数字键 0
        {
            PushButton* button = new PushButton;
            button->setText("0");
            layout->addWidget(button, 5, 1);
            connect(button, &PushButton::clicked, this, [this] { this->pressNumber(0); });
        }

        // 小数点
        {
            PushButton* button = new PushButton;
            button->setText(".");
            layout->addWidget(button, 5, 2);
            connect(button, &PushButton::clicked, this, &MainWindow::pressDot);
        }

        // 归零
        {
            PushButton* button = new PushButton;
            button->setText("C");
            layout->addWidget(button, 2, 3);
            connect(button, &PushButton::clicked, this, &MainWindow::pressClear);
        }

        // 退格键
        {
            PushButton* button = new PushButton;
            button->setText("<=");
            layout->addWidget(button, 2, 4);
            connect(button, &PushButton::clicked, this, &MainWindow::pressBackspace);
        }

        // 运算键
        std::vector<std::pair<Operator, QString>> operators = {
            {Operator::kPlus, "+"},
            {Operator::kMinus, "-"},
            {Operator::kMultiply, "x"},
            {Operator::kDivide, "/"},
        };

        for (size_t i = 0; i < operators.size(); i++) {
            const auto& [op, name] = operators[i];
            PushButton* button = new PushButton;
            button->setText(name);
            layout->addWidget(button, 3 + i / 2, 3 + i % 2);
            connect(button, &PushButton::clicked, this, [this, op] { this->pressOperator(op); });
        }

        // =
        {
            PushButton* button = new PushButton;
            button->setText("=");
            layout->addWidget(button, 5, 3, 1, 2);
            connect(button, &PushButton::clicked, this, &MainWindow::pressEqual);
        }
    }

private:
    /// @brief 校验输入的文本是否合法并设置文本
    void setLineText(QString text) {
        int pos;
        if (line_edit_->validator()->validate(text, pos) != QValidator::Invalid) {
            line_edit_->setText(text);
        }
    }

    /// @brief 计算最后一个运算符
    void calculateLast() {
        if (operators_.empty()) {
            qDebug() << "operators_ is empty";
            return;
        }

        auto op = operators_.back();
        operators_.pop_back();

        double rhs;
        switch (op) {
            case Operator::kNone:
                qDebug() << "Operator::kNone";
                return;
            case Operator::kPlus:
            case Operator::kMinus:
            case Operator::kMultiply:
            case Operator::kDivide:
                if (numbers_.size() < 2) {
                    qDebug() << "numbers_.size() < 2";
                    return;
                }
                rhs = numbers_.back();
                numbers_.pop_back();
                break;
        }

        switch (op) {
            case Operator::kNone:
                qDebug() << "Operator::kNone";
                return;
            case Operator::kPlus:
                numbers_.back() += rhs;
                break;
            case Operator::kMinus:
                numbers_.back() -= rhs;
                break;
            case Operator::kMultiply:
                numbers_.back() *= rhs;
                break;
            case Operator::kDivide:
                numbers_.back() /= rhs;
                break;
        }
    }

    /// @brief 清理上一次的表达式
    void clearEquation() {
        if (enter_pressed_) {
            enter_pressed_ = false;
            equation_->clear();
        }
    }

    void pressClear() { setLineText("0"); }

    void pressBackspace() {
        clearEquation();

        QString text = line_edit_->text();
        if (text.size() == 1) {
            setLineText("0");
        } else if (!text.isEmpty()) {
            text.erase(text.end() - 1);
            setLineText(text);
        }
    }

    void pressNumber(int n) {
        clearEquation();

        if (line_edit_->text() == "0") {
            setLineText(QString::number(n));
        } else {
            setLineText(line_edit_->text() += QString::number(n));
        }
    }

    void pressSign() {
        clearEquation();

        auto text = line_edit_->text();
        if (!text.isEmpty()) {
            if (text.startsWith("-")) {
                text.erase(text.begin());
            } else {
                text = "-" + text;
            }
        }
        setLineText(text);
    }

    void pressDot() {
        clearEquation();
        setLineText(line_edit_->text() + ".");
    }

    void pressOperator(Operator op) {
        clearEquation();

        // 处理表达式显示
        auto equation = equation_->text() + line_edit_->text();
        switch (op) {
            case Operator::kNone:
                break;
            case Operator::kPlus:
                equation += " + ";
                break;
            case Operator::kMinus:
                equation += " - ";
                break;
            case Operator::kMultiply:
                equation += " x ";
                break;
            case Operator::kDivide:
                equation += " / ";
                break;
        }
        equation_->setText(equation);

        // 处理计算结果
        numbers_.push_back(line_edit_->text().toDouble());
        if (!operators_.empty() && operator_precedence_[op] <= operator_precedence_[operators_.back()]) {
            calculateLast();
        }
        operators_.push_back(op);

        setLineText("0");
    }

    void pressEqual() {
        // 重复点击
        if (numbers_.empty()) {
            equation_->clear();
            return;
        }

        numbers_.push_back(line_edit_->text().toDouble());

        while (!operators_.empty()) {
            calculateLast();
        }

        equation_->setText(equation_->text() + line_edit_->text() + " = " +
                           QString::number(numbers_.back(), 'g', 10));

        numbers_.clear();
        operators_.clear();
        enter_pressed_ = true;

        setLineText("0");
    }

private:
    QLabel* equation_;
    QLineEdit* line_edit_;

    std::unordered_map<Operator, int> operator_precedence_ = {
        {Operator::kPlus, 1},
        {Operator::kMinus, 1},
        {Operator::kMultiply, 2},
        {Operator::kDivide, 2},
    };

    std::vector<double> numbers_;
    std::vector<Operator> operators_;
    bool enter_pressed_ = false;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    MainWindow main_window;
    main_window.show();

    return app.exec();
}
