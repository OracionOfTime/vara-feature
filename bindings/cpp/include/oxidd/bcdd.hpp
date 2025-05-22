#pragma once
 
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>
#include <version>
 
#ifdef __cpp_lib_concepts
#include <concepts>
#endif // __cpp_lib_concepts
 
#include <oxidd/capi.h>
#include <oxidd/util.hpp>
 
namespace oxidd {
 
class bcdd_function;
class bcdd_substitution;
 
class bcdd_manager {
  capi::oxidd_bcdd_manager_t _manager = {._p = nullptr};
 
  friend class bcdd_function;
 
  bcdd_manager(capi::oxidd_bcdd_manager_t manager) noexcept
      : _manager(manager) {}
 
public:
  using function = bcdd_function;
 
  bcdd_manager() noexcept = default;
  bcdd_manager(size_t inner_node_capacity, size_t apply_cache_capacity,
               uint32_t threads) noexcept
      : _manager(capi::oxidd_bcdd_manager_new(inner_node_capacity,
                                              apply_cache_capacity, threads)) {}
  bcdd_manager(const bcdd_manager &other) noexcept : _manager(other._manager) {
    capi::oxidd_bcdd_manager_ref(_manager);
  }
  bcdd_manager(bcdd_manager &&other) noexcept : _manager(other._manager) {
    other._manager._p = nullptr;
  }
 
  ~bcdd_manager() noexcept { capi::oxidd_bcdd_manager_unref(_manager); }
 
  bcdd_manager &operator=(const bcdd_manager &rhs) noexcept {
    if (this != &rhs) {
      capi::oxidd_bcdd_manager_unref(_manager);
      _manager = rhs._manager;
      capi::oxidd_bcdd_manager_ref(_manager);
    }
    return *this;
  }
  bcdd_manager &operator=(bcdd_manager &&rhs) noexcept {
    assert(this != &rhs || !rhs._manager._p);
    capi::oxidd_bcdd_manager_unref(_manager);
    _manager = rhs._manager;
    rhs._manager._p = nullptr;
    return *this;
  }
 
  friend bool operator==(const bcdd_manager &lhs,
                         const bcdd_manager &rhs) noexcept {
    return lhs._manager._p == rhs._manager._p;
  }
  friend bool operator!=(const bcdd_manager &lhs,
                         const bcdd_manager &rhs) noexcept {
    return !(lhs == rhs);
  }
 
  [[nodiscard]] bool is_invalid() const noexcept {
    return _manager._p == nullptr;
  }
 
  template <typename R> R run_in_worker_pool(std::function<R()> f) const {
    assert(_manager._p);
    return oxidd::util::detail::run_in_worker_pool(
        capi::oxidd_bcdd_manager_run_in_worker_pool, _manager, std::move(f));
  }
 
 
  [[nodiscard]] bcdd_function new_var() noexcept;
 
  [[nodiscard]] bcdd_function t() const noexcept;
  [[nodiscard]] bcdd_function f() const noexcept;
 
 
  [[nodiscard]] size_t num_inner_nodes() const noexcept {
    assert(_manager._p != nullptr);
    return capi::oxidd_bcdd_num_inner_nodes(_manager);
  }
 
};
 
class bcdd_function {
  capi::oxidd_bcdd_t _func = {._p = nullptr};
 
  friend class bcdd_manager;
  friend class bcdd_substitution;
  friend struct std::hash<bcdd_function>;
 
  bcdd_function(capi::oxidd_bcdd_t func) noexcept : _func(func) {}
 
public:
  using manager = bcdd_manager;
  using substitution = bcdd_substitution;
 
  bcdd_function() noexcept = default;
  bcdd_function(const bcdd_function &other) noexcept : _func(other._func) {
    capi::oxidd_bcdd_ref(_func);
  }
  bcdd_function(bcdd_function &&other) noexcept : _func(other._func) {
    other._func._p = nullptr;
  }
 
  ~bcdd_function() noexcept { capi::oxidd_bcdd_unref(_func); }
 
  bcdd_function &operator=(const bcdd_function &rhs) noexcept {
    if (this != &rhs) {
      capi::oxidd_bcdd_unref(_func);
      _func = rhs._func;
      capi::oxidd_bcdd_ref(_func);
    }
    return *this;
  }
  bcdd_function &operator=(bcdd_function &&rhs) noexcept {
    assert(this != &rhs || !rhs._func._p);
    capi::oxidd_bcdd_unref(_func);
    _func = rhs._func;
    rhs._func._p = nullptr;
    return *this;
  }
 
  friend bool operator==(const bcdd_function &lhs,
                         const bcdd_function &rhs) noexcept {
    return lhs._func._i == rhs._func._i && lhs._func._p == rhs._func._p;
  }
  friend bool operator!=(const bcdd_function &lhs,
                         const bcdd_function &rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator<(const bcdd_function &lhs,
                        const bcdd_function &rhs) noexcept {
    return std::tie(lhs._func._p, lhs._func._i) <
           std::tie(rhs._func._p, rhs._func._i);
  }
  friend bool operator>(const bcdd_function &lhs,
                        const bcdd_function &rhs) noexcept {
    return rhs < lhs;
  }
  friend bool operator<=(const bcdd_function &lhs,
                         const bcdd_function &rhs) noexcept {
    return !(rhs < lhs);
  }
  friend bool operator>=(const bcdd_function &lhs,
                         const bcdd_function &rhs) noexcept {
    return !(lhs < rhs);
  }
 
  [[nodiscard]] bool is_invalid() const noexcept { return _func._p == nullptr; }
 
  [[nodiscard]] bcdd_manager containing_manager() const noexcept {
    assert(!is_invalid());
    return capi::oxidd_bcdd_containing_manager(_func);
  }
 
 
  [[nodiscard]] std::pair<bcdd_function, bcdd_function>
  cofactors() const noexcept {
    const capi::oxidd_bcdd_pair_t p = capi::oxidd_bcdd_cofactors(_func);
    return {p.first, p.second};
  }
  [[nodiscard]] bcdd_function cofactor_true() const noexcept {
    return capi::oxidd_bcdd_cofactor_true(_func);
  }
  [[nodiscard]] bcdd_function cofactor_false() const noexcept {
    return capi::oxidd_bcdd_cofactor_false(_func);
  }
 
  [[nodiscard]] level_no_t level() const noexcept {
    return capi::oxidd_bcdd_level(_func);
  }
 
  [[nodiscard]] bcdd_function operator~() const noexcept {
    return capi::oxidd_bcdd_not(_func);
  }
  [[nodiscard]] friend bcdd_function
  operator&(const bcdd_function &lhs, const bcdd_function &rhs) noexcept {
    return capi::oxidd_bcdd_and(lhs._func, rhs._func);
  }
  bcdd_function &operator&=(const bcdd_function &rhs) noexcept {
    return (*this = *this & rhs);
  }
  [[nodiscard]] friend bcdd_function
  operator|(const bcdd_function &lhs, const bcdd_function &rhs) noexcept {
    return capi::oxidd_bcdd_or(lhs._func, rhs._func);
  }
  bcdd_function &operator|=(const bcdd_function &rhs) noexcept {
    return (*this = *this | rhs);
  }
  [[nodiscard]] friend bcdd_function
  operator^(const bcdd_function &lhs, const bcdd_function &rhs) noexcept {
    return capi::oxidd_bcdd_xor(lhs._func, rhs._func);
  }
  bcdd_function &operator^=(const bcdd_function &rhs) noexcept {
    return (*this = *this ^ rhs);
  }
  [[nodiscard]] bcdd_function nand(const bcdd_function &rhs) const noexcept {
    return capi::oxidd_bcdd_nand(_func, rhs._func);
  }
  [[nodiscard]] bcdd_function nor(const bcdd_function &rhs) const noexcept {
    return capi::oxidd_bcdd_nor(_func, rhs._func);
  }
  [[nodiscard]] bcdd_function equiv(const bcdd_function &rhs) const noexcept {
    return capi::oxidd_bcdd_equiv(_func, rhs._func);
  }
  [[nodiscard]] bcdd_function imp(const bcdd_function &rhs) const noexcept {
    return capi::oxidd_bcdd_imp(_func, rhs._func);
  }
  [[nodiscard]] bcdd_function
  imp_strict(const bcdd_function &rhs) const noexcept {
    return capi::oxidd_bcdd_imp_strict(_func, rhs._func);
  }
  [[nodiscard]] bcdd_function ite(const bcdd_function &t,
                                  const bcdd_function &e) const noexcept {
    return capi::oxidd_bcdd_ite(_func, t._func, e._func);
  }
 
  [[nodiscard]] bcdd_function
  substitute(const bcdd_substitution &substitution) const noexcept;
 
  [[nodiscard]] bcdd_function forall(const bcdd_function &vars) const noexcept {
    return capi::oxidd_bcdd_forall(_func, vars._func);
  }
  [[nodiscard]] bcdd_function exists(const bcdd_function &vars) const noexcept {
    return capi::oxidd_bcdd_exists(_func, vars._func);
  }
  [[nodiscard, deprecated]]
  bcdd_function exist(const bcdd_function &vars) const noexcept {
    return exists(vars);
  }
  [[nodiscard]] bcdd_function unique(const bcdd_function &vars) const noexcept {
    return capi::oxidd_bcdd_unique(_func, vars._func);
  }
 
  [[nodiscard]] bcdd_function
  apply_forall(const util::boolean_operator op, const bcdd_function &rhs,
               const bcdd_function &vars) const noexcept {
    return capi::oxidd_bcdd_apply_forall(
        static_cast<capi::oxidd_boolean_operator>(op), _func, rhs._func,
        vars._func);
  }
 
  [[nodiscard]] bcdd_function
  apply_exists(const util::boolean_operator op, const bcdd_function &rhs,
               const bcdd_function &vars) const noexcept {
    return capi::oxidd_bcdd_apply_exists(
        static_cast<capi::oxidd_boolean_operator>(op), _func, rhs._func,
        vars._func);
  }
  [[nodiscard, deprecated]]
  bcdd_function apply_exist(const util::boolean_operator op,
                            const bcdd_function &rhs,
                            const bcdd_function &vars) const noexcept {
    return apply_exists(op, rhs, vars);
  }
 
  [[nodiscard]] bcdd_function
  apply_unique(const util::boolean_operator op, const bcdd_function &rhs,
               const bcdd_function &vars) const noexcept {
    return capi::oxidd_bcdd_apply_unique(
        static_cast<capi::oxidd_boolean_operator>(op), _func, rhs._func,
        vars._func);
  }
 
 
  [[nodiscard]] std::size_t node_count() const noexcept {
    assert(_func._p);
    return capi::oxidd_bcdd_node_count(_func);
  }
 
  [[nodiscard]] bool satisfiable() const noexcept {
    assert(_func._p);
    return capi::oxidd_bcdd_satisfiable(_func);
  }
 
  [[nodiscard]] bool valid() const noexcept {
    assert(_func._p);
    return capi::oxidd_bcdd_valid(_func);
  }
 
  [[nodiscard]] double sat_count_double(level_no_t vars) const noexcept {
    assert(_func._p);
    return capi::oxidd_bcdd_sat_count_double(_func, vars);
  }
 
  [[nodiscard]] util::assignment pick_cube() const noexcept {
    assert(_func._p);
    return util::assignment(capi::oxidd_bcdd_pick_cube(_func));
  }
 
  [[nodiscard]] bcdd_function pick_cube_dd() const noexcept {
    return capi::oxidd_bcdd_pick_cube_dd(_func);
  }
 
  [[nodiscard]] bcdd_function
  pick_cube_dd_set(const bcdd_function &literal_set) const noexcept {
    return capi::oxidd_bcdd_pick_cube_dd_set(_func, literal_set._func);
  }
 
  [[nodiscard]] bool
  eval(util::slice<std::pair<bcdd_function, bool>> args) const noexcept {
    assert(_func._p);
 
    // From a C++ perspective, it is nicer to have elements of type
    // `std::pair<bcdd_function, bool>` than `capi::oxidd_bcdd_bool_pair_t`. In
    // the following we ensure that their layouts are compatible such that the
    // pointer cast below is safe.
    using c_pair = capi::oxidd_bcdd_bool_pair_t;
    using cpp_pair = std::pair<bcdd_function, bool>;
    static_assert(std::is_standard_layout_v<cpp_pair>);
    static_assert(sizeof(cpp_pair) == sizeof(c_pair));
    static_assert(offsetof(cpp_pair, first) == offsetof(c_pair, func));
    static_assert(offsetof(cpp_pair, second) == offsetof(c_pair, val));
    static_assert(alignof(cpp_pair) == alignof(c_pair));
 
    return capi::oxidd_bcdd_eval(
        _func,
        reinterpret_cast<const c_pair *>(args.data()), // NOLINT(*-cast)
        args.size());
  }
 
};
 
inline bcdd_function bcdd_manager::new_var() noexcept {
  assert(_manager._p);
  return capi::oxidd_bcdd_new_var(_manager);
}
inline bcdd_function bcdd_manager::t() const noexcept {
  assert(_manager._p);
  return capi::oxidd_bcdd_true(_manager);
}
inline bcdd_function bcdd_manager::f() const noexcept {
  assert(_manager._p);
  return capi::oxidd_bcdd_false(_manager);
}
 
class bcdd_substitution {
  capi::oxidd_bcdd_substitution_t *_subst = nullptr;
 
  friend class bcdd_function;
 
public:
  using function = bcdd_function;
 
  bcdd_substitution() = default;
  bcdd_substitution(const bcdd_substitution &other) = delete;
  bcdd_substitution(bcdd_substitution &&other) noexcept : _subst(other._subst) {
    other._subst = nullptr;
  }
 
  bcdd_substitution &operator=(const bcdd_substitution &) = delete;
  bcdd_substitution &operator=(bcdd_substitution &&rhs) noexcept {
    assert(this != &rhs || !rhs._subst);
    capi::oxidd_bcdd_substitution_free(_subst);
    _subst = rhs._subst;
    rhs._subst = nullptr;
    return *this;
  }
 
  ~bcdd_substitution() noexcept { capi::oxidd_bcdd_substitution_free(_subst); }
 
#ifdef __cpp_lib_concepts
  template <std::input_iterator IT>
    requires(std::equality_comparable<IT> &&
             util::pair_like<std::iter_value_t<IT>, const bcdd_function &,
                             const bcdd_function &>)
#else  // __cpp_lib_concepts
  template <typename IT>
#endif // __cpp_lib_concepts
  bcdd_substitution(IT begin, IT end)
      : _subst(capi::oxidd_bcdd_substitution_new(util::size_hint(
            begin, end,
            typename std::iterator_traits<IT>::iterator_category()))) {
    for (; begin != end; ++begin) {
      const auto &pair = *begin;
      const bcdd_function &var = std::get<0>(pair);
      const bcdd_function &replacement = std::get<1>(pair);
      assert(var._func._p);
      assert(replacement._func._p);
      capi::oxidd_bcdd_substitution_add_pair(_subst, var._func,
                                             replacement._func);
    }
  }
 
#if defined(__cpp_lib_ranges) && defined(__cpp_lib_concepts)
  template <std::ranges::input_range R>
    requires(util::pair_like<std::ranges::range_value_t<R>,
                             const bcdd_function &, const bcdd_function &>)
  bcdd_substitution(R &&range)
      : _subst(capi::oxidd_bcdd_substitution_new(util::size_hint(range))) {
    for (const auto &[var, replacement] : std::forward<R>(range)) {
      assert(var._func._p);
      assert(replacement._func._p);
      capi::oxidd_bcdd_substitution_add_pair(_subst, var._func,
                                             replacement._func);
    }
  }
#endif // defined(__cpp_lib_ranges) && defined(__cpp_lib_concepts)
 
  [[nodiscard]] bool is_invalid() const { return _subst == nullptr; }
};
 
inline bcdd_function bcdd_function::substitute(
    const bcdd_substitution &substitution) const noexcept {
  assert(substitution._subst);
  return capi::oxidd_bcdd_substitute(_func, substitution._subst);
}
 
} // namespace oxidd
 
 
template <> struct std::hash<oxidd::bcdd_function> {
  std::size_t operator()(const oxidd::bcdd_function &f) const noexcept {
    return std::hash<const void *>{}(f._func._p) ^ f._func._i;
  }
};
 