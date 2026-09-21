export module co_async.utils.non_void_helper;

export namespace pyc::co_async {

template <typename T = void>
struct NonVoidHelper {
    using Type = T;
};

template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;

    explicit NonVoidHelper() noexcept = default;
};

}  // namespace pyc::co_async
