#include <charconv>
#include <cmath>
#include <deque>
#include <limits>
#include <map>
#include <string_view>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "common/cin_redirect.h"

namespace pyc {

class RPN {
public:
    double op(std::string_view str_view) {
        double value;
        auto [ptr, ec] = std::from_chars(str_view.data(), str_view.data() + str_view.size(), value);
        if (ec != std::errc()) {
            return optor(str_view);
        }
        deq_.push_front(value);
        return value;
    }

    void clear() { deq_.clear(); }

    std::string get_stack_string() const {
        std::string str{};
        for (auto v : deq_) {
            str += fmt::format("{} ", v);
        }
        return str;
    }

private:
    std::pair<double, double> pop_get2() {
        if (deq_.size() < 2) {
            return {zero_, zero_};
        }
        double v1 = deq_.front();
        deq_.pop_front();
        double v2 = deq_.front();
        deq_.pop_front();
        return {v2, v1};
    }

    double optor(std::string_view op) {
        std::map<std::string_view, double (*)(double, double)> opmap{
            {"+", [](double lhs, double rhs) { return lhs + rhs; }},
            {"-", [](double lhs, double rhs) { return lhs - rhs; }},
            {"*", [](double lhs, double rhs) { return lhs * rhs; }},
            {"/", [](double lhs, double rhs) { return lhs / rhs; }},
            {"^", [](double lhs, double rhs) { return std::pow(lhs, rhs); }},
            {"%", [](double lhs, double rhs) { return std::fmod(lhs, rhs); }},
        };
        if (opmap.find(op) == opmap.end()) {
            return zero_;
        }
        auto [lhs, rhs] = pop_get2();
        if (op == "/" && rhs == zero_) {
            deq_.push_front(inf_);
        } else {
            deq_.push_front(opmap[op](lhs, rhs));
        }
        return deq_.front();
    }

private:
    constexpr static double zero_{0.0};
    constexpr static double inf_{std::numeric_limits<double>::infinity()};

private:
    std::deque<double> deq_{};
};

TEST(RPNTest, RPNTest) {
    // 创建一个 string stream 对象用于模拟输入
    std::stringstream simulated_input("9 6 * 2 3 * +");
    CinRedirect cin_redirect(simulated_input);

    RPN rpn;
    for (std::string o{}; std::cin >> o;) {
        rpn.op(o);
        fmt::println("stack: {}", rpn.get_stack_string());
    }
}

}  // namespace pyc
