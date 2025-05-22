#pragma once
 
#include <version> // for __cpp_lib_concepts
#ifdef __cpp_lib_concepts
 
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <utility>
 
#ifdef __cpp_lib_ranges
#include <ranges>
#endif // __cpp_lib_ranges
 
#include <oxidd/util.hpp>
 
namespace oxidd::concepts {
 
 
namespace detail {
 
// Dummy input_iterator and input_range to specify the substitution concept
 
template <typename T> struct input_iterator {
  using difference_type = std::ptrdiff_t;
  using value_type = T;
 
  const T &operator*() const;
 
  input_iterator &operator++();
  void operator++(int);
 
  bool operator==(const input_iterator &) const;
};
 
static_assert(std::input_iterator<input_iterator<int>>);
static_assert(std::equality_comparable<input_iterator<int>>);
 
#ifdef __cpp_lib_ranges
 
template <typename T> struct input_range {
  input_iterator<T> begin();
  input_iterator<T> end();
};
 
static_assert(std::ranges::input_range<input_range<int>>);
 
#endif // __cpp_lib_ranges
 
} // namespace detail
 
 
template <class F>
concept function = (
  std::regular<F> &&
  std::totally_ordered<F> &&
  std::same_as<typename F::manager::function, F> &&
  requires(const F &f) {
    { f.is_invalid() } -> std::same_as<bool>;
    { f.containing_manager() } -> std::same_as<typename F::manager>;
 
    { std::hash<F>{}(f) } -> std::convertible_to<std::size_t>;
 
    { f.node_count() } -> std::same_as<std::size_t>;
  } &&
  // manager requirements
  std::regular<typename F::manager> &&
  requires(const typename F::manager &m) {
    { m.is_invalid() } -> std::same_as<bool>;
 
    { m.num_inner_nodes() } -> std::same_as<size_t>;
  }
);
 
template <class M>
concept manager = function<typename M::function> &&
                  std::same_as<typename M::function::manager, M>;
 
template <class F>
concept function_subst =
    function<F> && std::same_as<typename F::substitution::function, F> &&
    requires(const F &f, const typename F::substitution &s) {
      { f.substitute(s) } -> std::same_as<F>;
    } &&
    // substitution requirements
    std::movable<typename F::substitution> &&
    std::default_initializable<typename F::substitution> &&
    std::constructible_from<
        typename F::substitution,
        detail::input_iterator<std::tuple<const F &, const F &>>,
        detail::input_iterator<std::tuple<const F &, const F &>>> &&
#ifdef __cpp_lib_ranges
    std::constructible_from<
        typename F::substitution,
        detail::input_range<std::tuple<const F &, const F &>>> &&
#endif // __cpp_lib_ranges
    requires(const typename F::substitution &s) {
      { s.is_invalid() } -> std::same_as<bool>;
    };
 
template <class S>
concept substitution = function_subst<typename S::function> &&
                       std::same_as<typename S::function::substitution, S>;
 
template <class F>
concept boolean_function =
    function<F> &&
    // `f.sat_count_double` should take `levels` by-value. We require that it
    // isn't taken as a mutable reference, at least.
    requires(const F &f, F &mut_f, const level_no_t &levels,
             util::slice<std::pair<F, bool>> args) {
      // cofactors
      { f.cofactors() } -> std::same_as<std::pair<F, F>>;
      { f.cofactor_true() } -> std::same_as<F>;
      { f.cofactor_false() } -> std::same_as<F>;
 
      // negation
      { ~f } -> std::same_as<F>;
      // conjunction
      { f &f } -> std::same_as<F>;
      { mut_f &= f } -> std::same_as<F &>;
      // disjunction
      { f | f } -> std::same_as<F>;
      { mut_f |= f } -> std::same_as<F &>;
      // exclusive disjunction
      { f ^ f } -> std::same_as<F>;
      { mut_f ^= f } -> std::same_as<F &>;
      // negated conjunction
      { f.nand(f) } -> std::same_as<F>;
      // negated disjunction
      { f.nor(f) } -> std::same_as<F>;
      // equivalence
      { f.equiv(f) } -> std::same_as<F>;
      // implication
      { f.imp(f) } -> std::same_as<F>;
      // strict implication
      { f.imp_strict(f) } -> std::same_as<F>;
      // if-then-else
      { f.ite(f, f) } -> std::same_as<F>;
 
      { f.satisfiable() } -> std::same_as<bool>;
      { f.valid() } -> std::same_as<bool>;
 
      { f.sat_count_double(levels) } -> std::same_as<double>;
 
      { f.pick_cube() } -> std::same_as<util::assignment>;
      { f.pick_cube_dd() } -> std::same_as<F>;
      { f.pick_cube_dd_set(f) } -> std::same_as<F>;
 
      { f.eval(args) } -> std::same_as<bool>;
    } &&
    // manager requirements
    requires(const typename F::manager &m, typename F::manager &mut_m) {
      { mut_m.new_var() } -> std::same_as<F>;
      { m.t() } -> std::same_as<F>;
      { m.f() } -> std::same_as<F>;
    };
 
template <class M>
concept boolean_function_manager =
    manager<M> && boolean_function<typename M::function>;
 
template <class F>
concept boolean_function_quant =
    boolean_function<F> &&
    // `op` should be passed by-value. We require that the methods don't take it
    // as a mutable reference, at least.
    requires(const F &f, const util::boolean_operator &op) {
      { f.forall(f) } -> std::same_as<F>;
      { f.exists(f) } -> std::same_as<F>;
      { f.unique(f) } -> std::same_as<F>;
 
      { f.apply_forall(op, f, f) } -> std::same_as<F>;
      { f.apply_exists(op, f, f) } -> std::same_as<F>;
      { f.apply_unique(op, f, f) } -> std::same_as<F>;
    };
 
} // namespace oxidd::concepts
 
#endif // __cpp_lib_concepts