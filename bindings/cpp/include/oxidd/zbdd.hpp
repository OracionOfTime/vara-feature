#pragma once
 
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
 
#include <oxidd/capi.h>
#include <oxidd/util.hpp>
 
namespace oxidd {
 
class zbdd_function;
 
class zbdd_manager {
  capi::oxidd_zbdd_manager_t _manager = {._p = nullptr};
 
  friend class zbdd_function;
 
  zbdd_manager(capi::oxidd_zbdd_manager_t manager) noexcept
      : _manager(manager) {}
 
public:
  using function = zbdd_function;
 
  zbdd_manager() noexcept = default;
  zbdd_manager(size_t inner_node_capacity, size_t apply_cache_capacity,
               uint32_t threads) noexcept
      : _manager(capi::oxidd_zbdd_manager_new(inner_node_capacity,
                                              apply_cache_capacity, threads)) {}
  zbdd_manager(const zbdd_manager &other) noexcept : _manager(other._manager) {
    capi::oxidd_zbdd_manager_ref(_manager);
  }
  zbdd_manager(zbdd_manager &&other) noexcept : _manager(other._manager) {
    other._manager._p = nullptr;
  }
 
  ~zbdd_manager() noexcept { capi::oxidd_zbdd_manager_unref(_manager); }
 
  zbdd_manager &operator=(const zbdd_manager &rhs) noexcept {
    if (this != &rhs) {
      capi::oxidd_zbdd_manager_unref(_manager);
      _manager = rhs._manager;
      capi::oxidd_zbdd_manager_ref(_manager);
    }
    return *this;
  }
  zbdd_manager &operator=(zbdd_manager &&rhs) noexcept {
    assert(this != &rhs || !rhs._manager._p);
    capi::oxidd_zbdd_manager_unref(_manager);
    _manager = rhs._manager;
    rhs._manager._p = nullptr;
    return *this;
  }
 
  friend bool operator==(const zbdd_manager &lhs,
                         const zbdd_manager &rhs) noexcept {
    return lhs._manager._p == rhs._manager._p;
  }
  friend bool operator!=(const zbdd_manager &lhs,
                         const zbdd_manager &rhs) noexcept {
    return !(lhs == rhs);
  }
 
  [[nodiscard]] bool is_invalid() const noexcept {
    return _manager._p == nullptr;
  }
 
  template <typename R> R run_in_worker_pool(std::function<R()> f) const {
    assert(_manager._p);
    return oxidd::util::detail::run_in_worker_pool(
        capi::oxidd_zbdd_manager_run_in_worker_pool, _manager, std::move(f));
  }
 
 
  [[nodiscard]] zbdd_function new_singleton() noexcept;
  [[nodiscard]] zbdd_function new_var() noexcept;
 
  [[nodiscard]] zbdd_function empty() const noexcept;
  [[nodiscard]] zbdd_function base() const noexcept;
  [[nodiscard]] zbdd_function t() const noexcept;
  [[nodiscard]] zbdd_function f() const noexcept;
 
 
  [[nodiscard]] size_t num_inner_nodes() const noexcept {
    assert(_manager._p != nullptr);
    return capi::oxidd_zbdd_num_inner_nodes(_manager);
  }
 
};
 
class zbdd_function {
  capi::oxidd_zbdd_t _func = {._p = nullptr};
 
  friend class zbdd_manager;
  friend struct std::hash<zbdd_function>;
 
  zbdd_function(capi::oxidd_zbdd_t func) noexcept : _func(func) {}
 
public:
  using manager = zbdd_manager;
 
  zbdd_function() noexcept = default;
  zbdd_function(const zbdd_function &other) noexcept : _func(other._func) {
    capi::oxidd_zbdd_ref(_func);
  }
  zbdd_function(zbdd_function &&other) noexcept : _func(other._func) {
    other._func._p = nullptr;
  }
 
  ~zbdd_function() noexcept { capi::oxidd_zbdd_unref(_func); }
 
  zbdd_function &operator=(const zbdd_function &rhs) noexcept {
    if (this != &rhs) {
      capi::oxidd_zbdd_unref(_func);
      _func = rhs._func;
      capi::oxidd_zbdd_ref(_func);
    }
    return *this;
  }
  zbdd_function &operator=(zbdd_function &&rhs) noexcept {
    assert(this != &rhs || !rhs._func._p);
    capi::oxidd_zbdd_unref(_func);
    _func = rhs._func;
    rhs._func._p = nullptr;
    return *this;
  }
 
  friend bool operator==(const zbdd_function &lhs,
                         const zbdd_function &rhs) noexcept {
    return lhs._func._i == rhs._func._i && lhs._func._p == rhs._func._p;
  }
  friend bool operator!=(const zbdd_function &lhs,
                         const zbdd_function &rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator<(const zbdd_function &lhs,
                        const zbdd_function &rhs) noexcept {
    return std::tie(lhs._func._p, lhs._func._i) <
           std::tie(rhs._func._p, rhs._func._i);
  }
  friend bool operator>(const zbdd_function &lhs,
                        const zbdd_function &rhs) noexcept {
    return rhs < lhs;
  }
  friend bool operator<=(const zbdd_function &lhs,
                         const zbdd_function &rhs) noexcept {
    return !(rhs < lhs);
  }
  friend bool operator>=(const zbdd_function &lhs,
                         const zbdd_function &rhs) noexcept {
    return !(lhs < rhs);
  }
 
  [[nodiscard]] bool is_invalid() const noexcept { return _func._p == nullptr; }
 
  [[nodiscard]] zbdd_manager containing_manager() const noexcept {
    assert(!is_invalid());
    return capi::oxidd_zbdd_containing_manager(_func);
  }
 
 
  [[nodiscard]] std::pair<zbdd_function, zbdd_function>
  cofactors() const noexcept {
    const capi::oxidd_zbdd_pair_t p = capi::oxidd_zbdd_cofactors(_func);
    return {p.first, p.second};
  }
  [[nodiscard]] zbdd_function cofactor_true() const noexcept {
    return capi::oxidd_zbdd_cofactor_true(_func);
  }
  [[nodiscard]] zbdd_function cofactor_false() const noexcept {
    return capi::oxidd_zbdd_cofactor_false(_func);
  }
 
  [[nodiscard]] level_no_t level() const noexcept {
    return capi::oxidd_zbdd_level(_func);
  }
 
  [[nodiscard]] zbdd_function var_boolean_function() const noexcept {
    return capi::oxidd_zbdd_var_boolean_function(_func);
  }
 
  [[nodiscard]] zbdd_function operator~() const noexcept {
    return capi::oxidd_zbdd_not(_func);
  }
  [[nodiscard]] friend zbdd_function
  operator&(const zbdd_function &lhs, const zbdd_function &rhs) noexcept {
    return capi::oxidd_zbdd_intsec(lhs._func, rhs._func);
  }
  zbdd_function &operator&=(const zbdd_function &rhs) noexcept {
    return (*this = *this & rhs);
  }
  [[nodiscard]] friend zbdd_function
  operator|(const zbdd_function &lhs, const zbdd_function &rhs) noexcept {
    return capi::oxidd_zbdd_union(lhs._func, rhs._func);
  }
  zbdd_function &operator|=(const zbdd_function &rhs) noexcept {
    return (*this = *this | rhs);
  }
  [[nodiscard]] friend zbdd_function
  operator^(const zbdd_function &lhs, const zbdd_function &rhs) noexcept {
    return capi::oxidd_zbdd_xor(lhs._func, rhs._func);
  }
  zbdd_function &operator^=(const zbdd_function &rhs) noexcept {
    return (*this = *this ^ rhs);
  }
  [[nodiscard]] friend zbdd_function
  operator-(const zbdd_function &lhs, const zbdd_function &rhs) noexcept {
    return capi::oxidd_zbdd_diff(lhs._func, rhs._func);
  }
  [[nodiscard]] zbdd_function &operator-=(const zbdd_function &rhs) noexcept {
    return (*this = *this - rhs);
  }
  [[nodiscard]] zbdd_function nand(const zbdd_function &rhs) const noexcept {
    return capi::oxidd_zbdd_nand(_func, rhs._func);
  }
  [[nodiscard]] zbdd_function nor(const zbdd_function &rhs) const noexcept {
    return capi::oxidd_zbdd_nor(_func, rhs._func);
  }
  [[nodiscard]] zbdd_function equiv(const zbdd_function &rhs) const noexcept {
    return capi::oxidd_zbdd_equiv(_func, rhs._func);
  }
  [[nodiscard]] zbdd_function imp(const zbdd_function &rhs) const noexcept {
    return capi::oxidd_zbdd_imp(_func, rhs._func);
  }
  [[nodiscard]] zbdd_function
  imp_strict(const zbdd_function &rhs) const noexcept {
    return capi::oxidd_zbdd_imp_strict(_func, rhs._func);
  }
  [[nodiscard]] zbdd_function ite(const zbdd_function &t,
                                  const zbdd_function &e) const noexcept {
    return capi::oxidd_zbdd_ite(_func, t._func, e._func);
  }
 
  // NOLINTBEGIN(*-moved)
  [[nodiscard]] zbdd_function make_node(zbdd_function &&hi,
                                        zbdd_function &&lo) const noexcept {
    const capi::oxidd_zbdd_t h = hi._func, l = lo._func;
    hi._func._p = nullptr;
    lo._func._p = nullptr;
    return capi::oxidd_zbdd_make_node(_func, h, l);
  }
  // NOLINTEND(*-moved)
 
 
  [[nodiscard]] std::size_t node_count() const noexcept {
    assert(_func._p);
    return capi::oxidd_zbdd_node_count(_func);
  }
 
  [[nodiscard]] bool satisfiable() const noexcept {
    assert(_func._p);
    return capi::oxidd_zbdd_satisfiable(_func);
  }
 
  [[nodiscard]] bool valid() const noexcept {
    assert(_func._p);
    return capi::oxidd_zbdd_valid(_func);
  }
 
  [[nodiscard]] double sat_count_double(level_no_t vars) const noexcept {
    assert(_func._p);
    return capi::oxidd_zbdd_sat_count_double(_func, vars);
  }
 
  [[nodiscard]] util::assignment pick_cube() const noexcept {
    assert(_func._p);
    return util::assignment(capi::oxidd_zbdd_pick_cube(_func));
  }
 
  [[nodiscard]] zbdd_function pick_cube_dd() const noexcept {
    return capi::oxidd_zbdd_pick_cube_dd(_func);
  }
 
  [[nodiscard]] zbdd_function
  pick_cube_dd_set(const zbdd_function &literal_set) const noexcept {
    return capi::oxidd_zbdd_pick_cube_dd_set(_func, literal_set._func);
  }
 
  [[nodiscard]] bool
  eval(util::slice<std::pair<zbdd_function, bool>> args) const noexcept {
    assert(_func._p);
 
    // From a C++ perspective, it is nicer to have elements of type
    // `std::pair<bdd_function, bool>` than `capi::oxidd_bdd_bool_pair_t`. In
    // the following we ensure that their layouts are compatible such that the
    // pointer cast below is safe.
    using c_pair = capi::oxidd_zbdd_bool_pair_t;
    using cpp_pair = std::pair<zbdd_function, bool>;
    static_assert(std::is_standard_layout_v<cpp_pair>);
    static_assert(sizeof(cpp_pair) == sizeof(c_pair));
    static_assert(offsetof(cpp_pair, first) == offsetof(c_pair, func));
    static_assert(offsetof(cpp_pair, second) == offsetof(c_pair, val));
    static_assert(alignof(cpp_pair) == alignof(c_pair));
 
    return capi::oxidd_zbdd_eval(
        _func,
        reinterpret_cast<const c_pair *>(args.data()), // NOLINT(*-cast)
        args.size());
  }
 
};
 
inline zbdd_function zbdd_manager::new_singleton() noexcept {
  assert(_manager._p);
  return capi::oxidd_zbdd_new_singleton(_manager);
}
inline zbdd_function zbdd_manager::new_var() noexcept {
  assert(_manager._p);
  return capi::oxidd_zbdd_new_var(_manager);
}
inline zbdd_function zbdd_manager::empty() const noexcept {
  assert(_manager._p);
  return capi::oxidd_zbdd_empty(_manager);
}
inline zbdd_function zbdd_manager::base() const noexcept {
  assert(_manager._p);
  return capi::oxidd_zbdd_base(_manager);
}
inline zbdd_function zbdd_manager::t() const noexcept {
  assert(_manager._p);
  return capi::oxidd_zbdd_true(_manager);
}
inline zbdd_function zbdd_manager::f() const noexcept {
  assert(_manager._p);
  return capi::oxidd_zbdd_false(_manager);
}
 
} // namespace oxidd
 
 
template <> struct std::hash<oxidd::zbdd_function> {
  std::size_t operator()(const oxidd::zbdd_function &f) const noexcept {
    return std::hash<const void *>{}(f._func._p) ^ f._func._i;
  }
};
 