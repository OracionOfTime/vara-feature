#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "iostream"

using oxidd::bdd_manager;
using oxidd::bdd_function;
using oxidd::capi::oxidd_bdd_manager_t;
using oxidd::capi::oxidd_bdd_t;

int main() {

bdd_manager mgr(32, 320, 1);

bdd_function a = mgr.new_var();
bdd_function b = mgr.new_var();
bdd_function f = a ^ b;

const oxidd_bdd_manager_t* c_mgr = reinterpret_cast<const oxidd_bdd_manager_t*>(&mgr);
const oxidd_bdd_t* c_a = reinterpret_cast<const oxidd_bdd_t*>(&a);
const oxidd_bdd_t* c_b = reinterpret_cast<const oxidd_bdd_t*>(&b);
const oxidd_bdd_t* c_f = reinterpret_cast<const oxidd_bdd_t*>(&f);

const oxidd_bdd_t functions[] = { *c_f };
const char* function_names[] = { "f" };

const oxidd_bdd_t vars[] = { *c_a, *c_b };
const char* var_names[] = { "a", "b" };

bool success = oxidd_bdd_manager_dump_all_dot_file(
    *c_mgr,
    "../../results-BDDs/bdd.dot",
    functions,
    function_names,
    1,
    vars,
    var_names,
    2
);

    if (success) {
        std::cout << "DOT file created: bdd.dot\n";
        std::cout << "Run:\n  dot -Tpng ../../results-BDDs/bdd.dot -o ../../results-BDDs/bdd.png\n";
    } else {
        std::cerr << "Failed to dump DOT file.\n";
    }

    return 0;
}