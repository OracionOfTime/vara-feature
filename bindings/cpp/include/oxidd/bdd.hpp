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
 
class bdd_function;
class bdd_substitution;
 
class bdd_manager {
  capi::oxidd_bdd_manager_t _manager = {._p = nullptr};
 
  friend class bdd_function;
 
  bdd_manager(capi::oxidd_bdd_manager_t manager) noexcept : _manager(manager) {}
 
public:
  using function = bdd_function;
 
  bdd_manager() noexcept = default;
  bdd_manager(size_t inner_node_capacity, size_t apply_cache_capacity,
              uint32_t threads) noexcept
      : _manager(capi::oxidd_bdd_manager_new(inner_node_capacity,
                                             apply_cache_capacity, threads)) {}
  bdd_manager(const bdd_manager &other) noexcept : _manager(other._manager) {
    capi::oxidd_bdd_manager_ref(_manager);
  }
  bdd_manager(bdd_manager &&other) noexcept : _manager(other._manager) {
    other._manager._p = nullptr;
  }
 
  ~bdd_manager() noexcept {
    if (_manager._p != nullptr)
      capi::oxidd_bdd_manager_unref(_manager);
  }
 
  bdd_manager &operator=(const bdd_manager &rhs) noexcept {
    if (this != &rhs) {
      capi::oxidd_bdd_manager_unref(_manager);
      _manager = rhs._manager;
      capi::oxidd_bdd_manager_ref(_manager);
    }
    return *this;
  }
  bdd_manager &operator=(bdd_manager &&rhs) noexcept {
    assert(this != &rhs || !rhs._manager._p);
    capi::oxidd_bdd_manager_unref(_manager);
    _manager = rhs._manager;
    rhs._manager._p = nullptr;
    return *this;
  }
 
  friend bool operator==(const bdd_manager &lhs,
                         const bdd_manager &rhs) noexcept {
    return lhs._manager._p == rhs._manager._p;
  }
  friend bool operator!=(const bdd_manager &lhs,
                         const bdd_manager &rhs) noexcept {
    return !(lhs == rhs);
  }
 
  [[nodiscard]] bool is_invalid() const noexcept {
    return _manager._p == nullptr;
  }
 
  template <typename R> R run_in_worker_pool(std::function<R()> f) const {
    assert(_manager._p);
    return oxidd::util::detail::run_in_worker_pool(
        capi::oxidd_bdd_manager_run_in_worker_pool, _manager, std::move(f));
  }
 
 
  [[nodiscard]] bdd_function new_var() noexcept;
 
  [[nodiscard]] bdd_function t() const noexcept;
  [[nodiscard]] bdd_function f() const noexcept;
 
 
  [[nodiscard]] size_t num_inner_nodes() const noexcept {
    assert(_manager._p != nullptr);
    return capi::oxidd_bdd_num_inner_nodes(_manager);
  }
 
};
 
class bdd_function {
  capi::oxidd_bdd_t _func = {._p = nullptr};
 
  friend class bdd_manager;
  friend class bdd_substitution;
  friend struct std::hash<bdd_function>;
 
  bdd_function(capi::oxidd_bdd_t func) noexcept : _func(func) {}
 
public:
  using manager = bdd_manager;
  using substitution = bdd_substitution;
 
  bdd_function() noexcept = default;
  bdd_function(const bdd_function &other) noexcept : _func(other._func) {
    capi::oxidd_bdd_ref(_func);
  }
  bdd_function(bdd_function &&other) noexcept : _func(other._func) {
    other._func._p = nullptr;
  }
 
  ~bdd_function() noexcept {
    if (_func._p != nullptr)
      capi::oxidd_bdd_unref(_func);
  }
 
  bdd_function &operator=(const bdd_function &rhs) noexcept {
    if (this != &rhs) {
      capi::oxidd_bdd_unref(_func);
      _func = rhs._func;
      capi::oxidd_bdd_ref(_func);
    }
    return *this;
  }
  bdd_function &operator=(bdd_function &&rhs) noexcept {
    assert(this != &rhs || !rhs._func._p);
    capi::oxidd_bdd_unref(_func);
    _func = rhs._func;
    rhs._func._p = nullptr;
    return *this;
  }
 
  friend bool operator==(const bdd_function &lhs,
                         const bdd_function &rhs) noexcept {
    return lhs._func._i == rhs._func._i && lhs._func._p == rhs._func._p;
  }
  friend bool operator!=(const bdd_function &lhs,
                         const bdd_function &rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator<(const bdd_function &lhs,
                        const bdd_function &rhs) noexcept {
    return std::tie(lhs._func._p, lhs._func._i) <
           std::tie(rhs._func._p, rhs._func._i);
  }
  friend bool operator>(const bdd_function &lhs,
                        const bdd_function &rhs) noexcept {
    return rhs < lhs;
  }
  friend bool operator<=(const bdd_function &lhs,
                         const bdd_function &rhs) noexcept {
    return !(rhs < lhs);
  }
  friend bool operator>=(const bdd_function &lhs,
                         const bdd_function &rhs) noexcept {
    return !(lhs < rhs);
  }
 
  [[nodiscard]] bool is_invalid() const noexcept { return _func._p == nullptr; }
 
  [[nodiscard]] bdd_manager containing_manager() const noexcept {
    assert(!is_invalid());
    return capi::oxidd_bdd_containing_manager(_func);
  }
 
 
  [[nodiscard]] std::pair<bdd_function, bdd_function>
  cofactors() const noexcept {
    const capi::oxidd_bdd_pair_t p = capi::oxidd_bdd_cofactors(_func);
    return {p.first, p.second};
  }
  [[nodiscard]] bdd_function cofactor_true() const noexcept {
    return capi::oxidd_bdd_cofactor_true(_func);
  }
  [[nodiscard]] bdd_function cofactor_false() const noexcept {
    return capi::oxidd_bdd_cofactor_false(_func);
  }
 
  [[nodiscard]] level_no_t level() const noexcept {
    return capi::oxidd_bdd_level(_func);
  }
 
  [[nodiscard]] bdd_function operator~() const noexcept {
    return capi::oxidd_bdd_not(_func);
  }
  [[nodiscard]] friend bdd_function
  operator&(const bdd_function &lhs, const bdd_function &rhs) noexcept {
    return capi::oxidd_bdd_and(lhs._func, rhs._func);
  }
  bdd_function &operator&=(const bdd_function &rhs) noexcept {
    return (*this = *this & rhs);
  }
  friend bdd_function operator|(const bdd_function &lhs,
                                const bdd_function &rhs) noexcept {
    return capi::oxidd_bdd_or(lhs._func, rhs._func);
  }
  bdd_function &operator|=(const bdd_function &rhs) noexcept {
    return (*this = *this | rhs);
  }
  friend bdd_function operator^(const bdd_function &lhs,
                                const bdd_function &rhs) noexcept {
    return capi::oxidd_bdd_xor(lhs._func, rhs._func);
  }
  bdd_function &operator^=(const bdd_function &rhs) noexcept {
    return (*this = *this ^ rhs);
  }
  [[nodiscard]] bdd_function nand(const bdd_function &rhs) const noexcept {
    return capi::oxidd_bdd_nand(_func, rhs._func);
  }
  [[nodiscard]] bdd_function nor(const bdd_function &rhs) const noexcept {
    return capi::oxidd_bdd_nor(_func, rhs._func);
  }
  [[nodiscard]] bdd_function equiv(const bdd_function &rhs) const noexcept {
    return capi::oxidd_bdd_equiv(_func, rhs._func);
  }
  [[nodiscard]] bdd_function imp(const bdd_function &rhs) const noexcept {
    return capi::oxidd_bdd_imp(_func, rhs._func);
  }
  [[nodiscard]] bdd_function
  imp_strict(const bdd_function &rhs) const noexcept {
    return capi::oxidd_bdd_imp_strict(_func, rhs._func);
  }
  [[nodiscard]] bdd_function ite(const bdd_function &t,
                                 const bdd_function &e) const noexcept {
    return capi::oxidd_bdd_ite(_func, t._func, e._func);
  }
 
  [[nodiscard]] bdd_function
  substitute(const bdd_substitution &substitution) const noexcept;
 
  [[nodiscard]] bdd_function forall(const bdd_function &vars) const noexcept {
    return capi::oxidd_bdd_forall(_func, vars._func);
  }
  [[nodiscard]] bdd_function exists(const bdd_function &vars) const noexcept {
    return capi::oxidd_bdd_exists(_func, vars._func);
  }
  [[nodiscard, deprecated]]
  bdd_function exist(const bdd_function &vars) const noexcept {
    return exists(vars);
  }
  [[nodiscard]] bdd_function unique(const bdd_function &vars) const noexcept {
    return capi::oxidd_bdd_unique(_func, vars._func);
  }
 
  [[nodiscard]] bdd_function
  apply_forall(const util::boolean_operator op, const bdd_function &rhs,
               const bdd_function &vars) const noexcept {
    return capi::oxidd_bdd_apply_forall(
        static_cast<capi::oxidd_boolean_operator>(op), _func, rhs._func,
        vars._func);
  }
 
  [[nodiscard]] bdd_function
  apply_exists(const util::boolean_operator op, const bdd_function &rhs,
               const bdd_function &vars) const noexcept {
    return capi::oxidd_bdd_apply_exists(
        static_cast<capi::oxidd_boolean_operator>(op), _func, rhs._func,
        vars._func);
  }
  [[nodiscard, deprecated]]
  bdd_function apply_exist(const util::boolean_operator op,
                           const bdd_function &rhs,
                           const bdd_function &vars) const noexcept {
    return apply_exists(op, rhs, vars);
  }
 
  [[nodiscard]] bdd_function
  apply_unique(const util::boolean_operator op, const bdd_function &rhs,
               const bdd_function &vars) const noexcept {
    return capi::oxidd_bdd_apply_unique(
        static_cast<capi::oxidd_boolean_operator>(op), _func, rhs._func,
        vars._func);
  }
 
 
  [[nodiscard]] std::size_t node_count() const noexcept {
    assert(_func._p);
    return capi::oxidd_bdd_node_count(_func);
  }
 
  [[nodiscard]] bool satisfiable() const noexcept {
    assert(_func._p);
    return capi::oxidd_bdd_satisfiable(_func);
  }
 
  [[nodiscard]] bool valid() const noexcept {
    assert(_func._p);
    return capi::oxidd_bdd_valid(_func);
  }
 
  [[nodiscard]] double sat_count_double(level_no_t vars) const noexcept {
    assert(_func._p);
    return capi::oxidd_bdd_sat_count_double(_func, vars);
  }
 
  [[nodiscard]] util::assignment pick_cube() const noexcept {
    assert(_func._p);
    return util::assignment(capi::oxidd_bdd_pick_cube(_func));
  }
 
  [[nodiscard]] bdd_function pick_cube_dd() const noexcept {
    return capi::oxidd_bdd_pick_cube_dd(_func);
  }
 
  [[nodiscard]] bdd_function
  pick_cube_dd_set(const bdd_function &literal_set) const noexcept {
    return capi::oxidd_bdd_pick_cube_dd_set(_func, literal_set._func);
  }
 
  [[nodiscard]] bool
  eval(util::slice<std::pair<bdd_function, bool>> args) const noexcept {
    assert(_func._p);
 
    // From a C++ perspective, it is nicer to have elements of type
    // `std::pair<bdd_function, bool>` than `capi::oxidd_bdd_bool_pair_t`. In
    // the following we ensure that their layouts are compatible such that the
    // pointer cast below is safe.
    using c_pair = capi::oxidd_bdd_bool_pair_t;
    using cpp_pair = std::pair<bdd_function, bool>;
    static_assert(std::is_standard_layout_v<cpp_pair>);
    static_assert(sizeof(cpp_pair) == sizeof(c_pair));
    static_assert(offsetof(cpp_pair, first) == offsetof(c_pair, func));
    static_assert(offsetof(cpp_pair, second) == offsetof(c_pair, val));
    static_assert(alignof(cpp_pair) == alignof(c_pair));
 
    return capi::oxidd_bdd_eval(
        _func,
        reinterpret_cast<const c_pair *>(args.data()), // NOLINT(*-cast)
        args.size());
  }
 
};
 
inline bdd_function bdd_manager::new_var() noexcept {
  assert(_manager._p);
  return capi::oxidd_bdd_new_var(_manager);
}
inline bdd_function bdd_manager::t() const noexcept {
  assert(_manager._p);
  return capi::oxidd_bdd_true(_manager);
}
inline bdd_function bdd_manager::f() const noexcept {
  assert(_manager._p);
  return capi::oxidd_bdd_false(_manager);
}
 
class bdd_substitution {
  capi::oxidd_bdd_substitution_t *_subst = nullptr;
 
  friend class bdd_function;
 
public:
  using function = bdd_function;
 
  bdd_substitution() = default;
  bdd_substitution(const bdd_substitution &other) = delete;
  bdd_substitution(bdd_substitution &&other) noexcept : _subst(other._subst) {
    other._subst = nullptr;
  }
 
  bdd_substitution &operator=(const bdd_substitution &) = delete;
  bdd_substitution &operator=(bdd_substitution &&rhs) noexcept {
    assert(this != &rhs || !rhs._subst);
    capi::oxidd_bdd_substitution_free(_subst);
    _subst = rhs._subst;
    rhs._subst = nullptr;
    return *this;
  }
 
  ~bdd_substitution() noexcept { capi::oxidd_bdd_substitution_free(_subst); }
 
#ifdef __cpp_lib_concepts
  template <std::input_iterator IT>
    requires(std::equality_comparable<IT> &&
             util::pair_like<std::iter_value_t<IT>, const bdd_function &,
                             const bdd_function &>)
#else  // __cpp_lib_concepts
  template <typename IT>
#endif // __cpp_lib_concepts
  bdd_substitution(IT begin, IT end)
      : _subst(capi::oxidd_bdd_substitution_new(util::size_hint(
            begin, end,
            typename std::iterator_traits<IT>::iterator_category()))) {
    for (; begin != end; ++begin) {
      const auto &pair = *begin;
      const bdd_function &var = std::get<0>(pair);
      const bdd_function &replacement = std::get<1>(pair);
      assert(var._func._p);
      assert(replacement._func._p);
      capi::oxidd_bdd_substitution_add_pair(_subst, var._func,
                                            replacement._func);
    }
  }
 
#if defined(__cpp_lib_ranges) && defined(__cpp_lib_concepts)
  template <std::ranges::input_range R>
    requires(util::pair_like<std::ranges::range_value_t<R>,
                             const bdd_function &, const bdd_function &>)
  bdd_substitution(R &&range)
      : _subst(capi::oxidd_bdd_substitution_new(util::size_hint(range))) {
    for (const auto &[var, replacement] : std::forward<R>(range)) {
      assert(var._func._p);
      assert(replacement._func._p);
      capi::oxidd_bdd_substitution_add_pair(_subst, var._func,
                                            replacement._func);
    }
  }
#endif // defined(__cpp_lib_ranges) && defined(__cpp_lib_concepts)
 
  [[nodiscard]] bool is_invalid() const { return _subst == nullptr; }
};
 
inline bdd_function
bdd_function::substitute(const bdd_substitution &substitution) const noexcept {
  assert(substitution._subst);
  return capi::oxidd_bdd_substitute(_func, substitution._subst);
}
 
} // namespace oxidd
 
 
template <> struct std::hash<oxidd::bdd_function> {
  std::size_t operator()(const oxidd::bdd_function &f) const noexcept {
    return std::hash<const void *>{}(f._func._p) ^ f._func._i;
  }
};
 