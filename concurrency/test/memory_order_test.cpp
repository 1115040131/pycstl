#include <atomic>
#include <thread>

#include <gtest/gtest.h>

namespace pyc {
namespace concurrency {

// memory_order_seq_cst
// 代表全局一致性顺序，可以用于 store, load 和 read-modify-write 操作, 实现 sequencial consistent 的顺序模型.
// 在这个模型下, 所有线程看到的所有操作都有一个一致的顺序, 即使这些操作可能针对不同的变量, 运行在不同的线程.
TEST(MemoryOrderTest, MemoryOrderSeqCst) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::atomic<bool> x, y;
        {
            // write_x_then_y
            std::jthread t1{[&]() {
                x.store(true, std::memory_order_seq_cst);  // 1
                y.store(true, std::memory_order_seq_cst);  // 2
            }};

            // read_y_then_x
            std::jthread t2{[&]() {
                while (!y.load(std::memory_order_seq_cst)) {  // 3
                }
                EXPECT_TRUE(x.load(std::memory_order_seq_cst));  // 4
            }};
        }
    }
}

// memory_order_relaxed
// 可以用于 store, load 和 read-modify-write 操作, 实现 relaxed 的顺序模型. 这种模型下,
// 只能保证操作的原子性和修改顺序 (modification order) 一致性, 无法实现 synchronizes-with 的关系。
TEST(MemoryOrderTest, MemoryOrderRelaxed) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::atomic<bool> x, y;
        {
            // write_x_then_y
            std::jthread t1{[&]() {
                // x86不支持宽松的内存顺序, 实际 1 必定在 2 之前执行
                x.store(true, std::memory_order_relaxed);  // 1
                y.store(true, std::memory_order_relaxed);  // 2
            }};

            // read_y_then_x
            std::jthread t2{[&]() {
                while (!y.load(std::memory_order_relaxed)) {  // 3
                }
                EXPECT_TRUE(x.load(std::memory_order_relaxed));  // 4
            }};
        }
    }
}

// Acquire-Release
// 在 acquire-release 模型中, 会使用 memory_order_acquire, memory_order_release 和 memory_order_acq_rel
// 这三种内存顺序. 它们的用法具体是这样的:
//     对原子变量的 load 可以使用 memory_order_acquire 内存顺序. 这称为 acquire 操作.
//     对原子变量的 store 可以使用 memory_order_release 内存顺序. 这称为 release 操作.
//     read-modify-write 操作即读 (load) 又写 (store), 它可以使用 memory_order_acquire, memory_order_release 和
//     memory_order_acq_rel:
//         如果使用 memory_order_acquire, 则作为 acquire 操作;
//         如果使用 memory_order_release, 则作为 release 操作;
//         如果使用 memory_order_acq_rel, 则同时为两者.
// Acquire-release 可以实现 synchronizes-with 的关系. 如果一个 acquire 操作在同一个原子变量上读取到了一个 release
// 操作写入的值, 则这个 release 操作 “synchronizes-with” 这个 acquire 操作.
TEST(MemoryOrderTest, AcquireRelease) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::atomic<bool> x, y;
        {
            // write_x_then_y
            std::jthread t1{[&]() {
                x.store(true, std::memory_order_relaxed);  // 1
                y.store(true, std::memory_order_release);  // 2
            }};

            // read_y_then_x
            std::jthread t2{[&]() {
                while (!y.load(std::memory_order_acquire)) {  // 3
                }
                EXPECT_TRUE(x.load(std::memory_order_relaxed));  // 4
            }};

            // t1执行到2将ry 设置为true, 因为使用了Acquire-release 顺序， 所以 t2 执行到3时读取ry为true， 因此2和3
            // 可以构成同步关系。
            // 又因为单线程t1内 1 sequence before 2,所以1 happens-before 3. 因为单线程t2内 3 sequence before 4.
            // 所以 1 happens-before 4.
        }
    }
}

TEST(MemoryOrderTest, ReleasAcquireDanger) {
    std::atomic<int> z;
    for (std::size_t i = 0; i < 3000; ++i) {
        std::atomic<int> x, y;
        {
            // 多个线程对同一个变量 release 操作，另一个线程对这个变量 acquire，那么只有一个线程的 release
            // 操作和这个 acquire 线程构成同步关系
            std::jthread t1{[&]() {
                x.store(1, std::memory_order_relaxed);  // 1
                y.store(1, std::memory_order_release);  // 2
            }};

            std::jthread t2{[&]() {
                y.store(2, std::memory_order_release);  // 3
            }};

            std::jthread t3{[&]() {
                while (!y.load(std::memory_order_acquire)) {  // 4
                }
                if (x.load(std::memory_order_relaxed) != 1) {  // 5
                    z++;
                }
            }};
        }
    }
    EXPECT_TRUE(z != 0);  // 4
}

// release sequence
// 并不是只有在 acquire 操作读取到 release 操作写入的值时才能构成 synchronizes-with 关系. 为了说这种情况,
// 我们需要引入 release sequence 这个概念.
// 针对一个原子变量 M 的 release 操作 A 完成后, 接下来 M 上可能还会有一连串的其他操作. 如果这一连串操作是由
//     1. 同一线程上的写操作
//     2. 任意线程上的 read-modify-write 操作
// 这两种构成的, 则称这一连串的操作为以 release 操作 A 为首的 release sequence. 这里的写操作和 read-modify-write
// 操作可以使用任意内存顺序.
// 如果一个 acquire 操作在同一个原子变量上读到了一个 release 操作写入的值, 或者读到了以这个 release 操作为首的
// release sequence 写入的值, 那么这个 release 操作 “synchronizes-with” 这个 acquire 操作.
TEST(MemoryOrderTest, ReleaseSequence) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::vector<int> data;
        std::atomic<int> flag{0};
        std::jthread t1([&]() {
            data.push_back(42);                        // (1)
            flag.store(1, std::memory_order_release);  // (2)
        });

        std::jthread t2([&]() {
            int expected = 1;
            while (!flag.compare_exchange_strong(expected, 2, std::memory_order_relaxed))  // (3)
                expected = 1;
        });

        std::jthread t3([&]() {
            while (flag.load(std::memory_order_acquire) < 2) {  // (4)
            }
            EXPECT_EQ(data.at(0), 42);  // (5)
        });
    }
}

// memory_order_consume
// 可以用于 load 操作. 使用 memory_order_consume 的 load 称为 consume 操作. 如果一个 consume
// 操作在同一个原子变量上读到了一个 release 操作写入的值, 或以其为首的 release sequence 写入的值, 则这个 release
// 操作 “dependency-ordered before” 这个 consume 操作.
TEST(MemoryOrderTest, MemoryOrderConsume) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::atomic<std::string*> ptr = nullptr;
        int data = 0;

        std::jthread t1([&]() {
            std::string* p = new std::string("Hello World");  // (1)
            data = 42;                                        // (2)
            ptr.store(p, std::memory_order_release);          // (3)
        });

        std::jthread t2([&]() {
            std::string* p2;
            while (!(p2 = ptr.load(std::memory_order_consume))) {  // (4)
            }
            EXPECT_EQ(*p2, "Hello World");  // (5)
            // data 并不依赖于 ptr, 所以 6 有可能触发
            EXPECT_EQ(data, 42);  // (6)
        });
    }
}

// 通过栅栏 2 和 5 同步, 1 和 6 也实现了同步
TEST(MemoryOrderTest, Fence) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::atomic<bool> x{false};
        std::atomic<bool> y{false};
        std::atomic<int> z{0};
        {
            std::jthread write_x_then_y_fence([&]() {
                x.store(true, std::memory_order_relaxed);             // 1
                std::atomic_thread_fence(std::memory_order_release);  // 2
                y.store(true, std::memory_order_relaxed);             // 3
            });
            std::jthread read_y_then_x_fence([&]() {
                while (!y.load(std::memory_order_relaxed)) {  // 4
                }
                std::atomic_thread_fence(std::memory_order_acquire);  // 5
                if (x.load(std::memory_order_relaxed)) {              // 6
                    ++z;
                }
            });
        }
        EXPECT_EQ(z.load(), 1);
    }
}

// 利用智能指针解决释放问题
class SingleMemoryModel {
private:
    SingleMemoryModel() {}
    SingleMemoryModel(const SingleMemoryModel&) = delete;
    SingleMemoryModel& operator=(const SingleMemoryModel&) = delete;

public:
    ~SingleMemoryModel() { std::cout << "single auto delete success " << std::endl; }
    static std::shared_ptr<SingleMemoryModel> GetInst() {
        // 1 处
        if (_b_init.load(std::memory_order_acquire)) {
            return single;
        }
        // 2 处
        s_mutex.lock();
        // 3 处
        if (_b_init.load(std::memory_order_relaxed)) {
            s_mutex.unlock();
            return single;
        }
        // 4处
        single = std::shared_ptr<SingleMemoryModel>(new SingleMemoryModel);
        _b_init.store(true, std::memory_order_release);
        s_mutex.unlock();
        return single;
    }

private:
    static std::shared_ptr<SingleMemoryModel> single;
    static std::mutex s_mutex;
    static std::atomic<bool> _b_init;
};

std::shared_ptr<SingleMemoryModel> SingleMemoryModel::single = nullptr;
std::mutex SingleMemoryModel::s_mutex;
std::atomic<bool> SingleMemoryModel::_b_init = false;

}  // namespace concurrency
}  // namespace pyc