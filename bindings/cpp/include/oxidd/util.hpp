#pragma once
 
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>
#include <version>
 
#ifdef __cpp_lib_concepts
#include <concepts>
#endif // __cpp_lib_concepts
 
#ifdef __cpp_lib_ranges
#include <ranges>
#endif // __cpp_lib_ranges
 
#include <oxidd/capi.h>
 
namespace oxidd {
 
using level_no_t = capi::oxidd_level_no_t;
 
namespace util {
 
enum class opt_bool : int8_t {
  // NOLINTBEGIN(readability-identifier-naming)
  NONE = -1,
  FALSE = 0,
  TRUE = 1,
  // NOLINTEND(readability-identifier-naming)
};
 
enum class boolean_operator : uint8_t {
  // NOLINTBEGIN(readability-identifier-naming)
  AND = capi::OXIDD_BOOLEAN_OPERATOR_AND,
  OR = capi::OXIDD_BOOLEAN_OPERATOR_OR,
  XOR = capi::OXIDD_BOOLEAN_OPERATOR_XOR,
  EQUIV = capi::OXIDD_BOOLEAN_OPERATOR_EQUIV,
  NAND = capi::OXIDD_BOOLEAN_OPERATOR_NAND,
  NOR = capi::OXIDD_BOOLEAN_OPERATOR_NOR,
  IMP = capi::OXIDD_BOOLEAN_OPERATOR_IMP,
  IMP_STRICT = capi::OXIDD_BOOLEAN_OPERATOR_IMP_STRICT,
  // NOLINTEND(readability-identifier-naming)
};
 
// Inspired by LLVM's ArrayRef class
template <typename T> class slice {
public:
  using value_type = T;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;
  using iterator = const_pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
 
private:
  const T *_data = nullptr;
  size_t _len = 0;
 
public:
 
  constexpr slice() = default;
 
  constexpr slice(std::nullopt_t /*unused*/) {}
 
  constexpr slice(const T *data, size_t len) : _data(data), _len(len) {}
 
  constexpr slice(const T *begin, const T *end)
      : _data(begin), _len(end - begin) {
    assert(begin <= end);
  }
 
  constexpr slice(const T &element) : _data(&element), _len(1) {}
 
  template <typename A>
  slice(const std::vector<T, A> &vec) : _data(vec.data()), _len(vec.size()) {}
 
  template <size_t N>
  constexpr slice(const std::array<T, N> &array)
      : _data(array.data()), _len(array.size()) {}
 
  template <size_t N>
  constexpr slice(const T (&array)[N]) // NOLINT(*-c-arrays)
      : _data(array), _len(N) {}
 
  constexpr slice(const std::initializer_list<T> &list)
      : _data(list.begin() == list.end() ? (const T *)nullptr : list.begin()),
        _len(list.size()) {}
 
 
  [[nodiscard]] constexpr size_t size() const noexcept { return _len; }
  [[nodiscard]] constexpr bool empty() const noexcept { return _len == 0; }
 
  [[nodiscard]] constexpr const T *data() const noexcept { return _data; }
 
  [[nodiscard]] constexpr iterator begin() const noexcept { return _data; }
  [[nodiscard]] constexpr iterator end() const noexcept { return _data + _len; }
 
  [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }
  [[nodiscard]] constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }
 
  [[nodiscard]] constexpr const T &front() const noexcept {
    assert(_len != 0 && "slice must be non-empty");
    return _data[0]; // NOLINT(*-pointer-arithmetic)
  }
  [[nodiscard]] constexpr const T &back() const noexcept {
    assert(_len != 0 && "slice must be non-empty");
    return _data[_len - 1]; // NOLINT(*-pointer-arithmetic)
  }
 
  constexpr const T &operator[](size_t index) const noexcept {
    assert(index < _len && "index out of bounds");
    return _data[index]; // NOLINT(*-pointer-arithmetic)
  }
 
  [[nodiscard]] constexpr slice subslice(size_t start,
                                         size_t end) const noexcept {
    assert(start <= end);
    assert(end <= _len);
    return slice(_data + start, end - start);
  }
 
  [[nodiscard]] std::vector<T> vec() const {
    return std::vector<T>(begin(), end());
  }
  operator std::vector<T>() const { return vec(); }
};
 
class assignment {
public:
  using value_type = opt_bool;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;
  using iterator = const_pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
 
private:
  capi::oxidd_assignment_t _assignment;
 
public:
  assignment(const assignment &) = delete; // use `vec()` to copy
 
  explicit assignment(capi::oxidd_assignment_t a) noexcept : _assignment(a) {}
 
  assignment(assignment &&other) noexcept : _assignment(other._assignment) {
    other._assignment.data = nullptr;
    other._assignment.len = 0;
  }
 
  assignment &operator=(const assignment &) = delete;
  assignment &operator=(assignment &&rhs) noexcept {
    assert(this != &rhs || !rhs._assignment.data);
    capi::oxidd_assignment_free(_assignment);
    _assignment = rhs._assignment;
    rhs._assignment.data = nullptr;
    rhs._assignment.len = 0;
    return *this;
  }
 
  ~assignment() noexcept { capi::oxidd_assignment_free(_assignment); }
 
  [[nodiscard]] const opt_bool *data() const noexcept {
    // NOLINTNEXTLINE(*-cast)
    return reinterpret_cast<const opt_bool *>(_assignment.data);
  }
  [[nodiscard]] size_t size() const noexcept { return _assignment.len; }
 
  [[nodiscard]] iterator begin() const noexcept { return data(); }
  [[nodiscard]] iterator end() const noexcept {
    return data() + size(); // NOLINT(*-pointer-arithmetic)
  }
 
  [[nodiscard]] reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }
  [[nodiscard]] reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }
 
  const opt_bool &operator[](size_t index) const noexcept {
    assert(index < size());
    return data()[index]; // NOLINT(*-pointer-arithmetic)
  }
 
  [[nodiscard]] util::slice<opt_bool> slice() const noexcept {
    return {data(), size()};
  }
  operator util::slice<opt_bool>() const noexcept { return slice(); }
 
  [[nodiscard]] std::vector<opt_bool> vec() const { return {begin(), end()}; }
  operator std::vector<opt_bool>() const { return vec(); }
};
 
template <typename IT>
inline std::size_t size_hint(IT begin, IT end,
                             std::forward_iterator_tag /*tag*/) {
  return std::distance(begin, end);
}
 
template <typename IT>
inline std::size_t size_hint(IT /*begin*/, IT /*end*/,
                             std::input_iterator_tag /*tag*/) {
  return 0;
}
 
#ifdef __cpp_lib_ranges
 
template <std::ranges::input_range R> inline std::size_t size_hint(R &&range) {
  if constexpr (std::ranges::forward_range<R> || std::ranges::sized_range<R>) {
    const auto d = std::ranges::distance(std::forward<R>(range));
    return d >= 0 ? d : 0; // d is signed
  } else {
    return 0;
  }
}
 
#endif // __cpp_lib_ranges
 
#ifdef __cpp_lib_concepts
 
template <typename P, typename T, typename U>
concept pair_like =
    std::convertible_to<std::tuple_element_t<0, P>, T> &&
    std::convertible_to<std::tuple_element_t<1, P>, U> &&
    std::tuple_size_v<std::remove_cvref_t<P>> == 2 && requires(P pair) {
      { std::get<0>(pair) } -> std::convertible_to<T>;
      { std::get<1>(pair) } -> std::convertible_to<U>;
    };
 
#endif // __cpp_lib_concepts
 
 
namespace detail {
 
extern "C" inline void *oxidd_callback_helper(void *f) {
  return (*static_cast<std::function<void *()> *>(f))();
}
 
template <typename M, typename R>
R run_in_worker_pool(void *(*run_fn)(M, void *(void *), void *), M manager,
                     const std::function<R()> f) {
  if constexpr (std::is_void_v<R>) {
    const std::function<void *()> wrapper([f = std::move(f)]() -> void * {
      f();
      return nullptr;
    });
    run_fn(manager, oxidd_callback_helper, (void *)(&wrapper));
  } else if constexpr (std::is_reference_v<R>) {
    const std::function<void *()> wrapper(
        [f = std::move(f)]() -> void * { return (void *)(&f()); });
    return *static_cast<std::remove_reference_t<R> *>(
        run_fn(manager, oxidd_callback_helper, (void *)(&wrapper)));
  } else if constexpr (std::is_pointer_v<R>) {
    const std::function<void *()> wrapper(
        [f = std::move(f)]() -> void * { return (void *)f(); });
    return static_cast<R>(
        run_fn(manager, oxidd_callback_helper, (void *)(&wrapper)));
  } else {
    // union type to not call the R's default constructor (or even require one)
    union {
      R v;
    } return_val;
    const std::function<void *()> wrapper(
        [f = std::move(f), &return_val]() -> void * {
          return_val.v = f();
          return nullptr;
        });
    run_fn(manager, oxidd_callback_helper, (void *)(&wrapper));
    return return_val.v;
  }
}
 
} // namespace detail
 
 
} // namespace util
 
} // namespace oxidd