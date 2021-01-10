#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <set>
#include <utility>
#include <memory>

/// =========== comparators ===========

namespace {
    template<typename A, typename V>
    struct FunCmp;

    template<typename A, typename V>
    struct MaxsCmp;
}

/// =========== exceptions ===========

class InvalidArg : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Invalid Argument";
    }
};

/// =========== declarations ===========

template<typename A, typename V>
class FunctionMaxima {
public:
// Typ point_type umożliwiający dostęp do „punktów” funkcji, udostępniający
// następujące funkcje:
    class point_type;

/// =========== usings ===========

    using size_type = size_t;
    using iterator = typename std::set<point_type>::iterator;
    using mx_iterator = typename std::set<point_type>::iterator;

/// =========== rest ===========

    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima &fm) = default;

    FunctionMaxima &operator=(const FunctionMaxima &other); ///*strong*

// Typ size_type reprezentujący rozmiar dziedziny i funkcja zwracająca ten
// rozmiar:
    size_type size() const noexcept;

// Zwraca wartość w punkcie a, rzuca wyjątek InvalidArg, jeśli a nie
// należy do dziedziny funkcji. Złożoność najwyżej O(log n).
    V const &value_at(A const &a) const;

// Zmienia funkcję tak, żeby zachodziło f(a) = v. Jeśli a nie należy do
// obecnej dziedziny funkcji, jest do niej dodawany. Najwyżej O(log n).
    void set_value(A const &a, V const &v); ///*strong*

// Usuwa a z dziedziny funkcji. Jeśli a nie należało do dziedziny funkcji,
// nie dzieje się nic. Złożoność najwyżej O(log n).
    void erase(A const &a); ///*strong*

/// ======== Metody dające dostęp do punktów funkcji: ==========

// iterator wskazujący na pierwszy punkt
    iterator begin() const noexcept;

// iterator wskazujący za ostatni punkt
    iterator end() const noexcept;

// Iterator, który wskazuje na punkt funkcji o argumencie a lub end(),
// jeśli takiego argumentu nie ma w dziedzinie funkcji.
    iterator find(A const &a) const;

/// ======= Metody dające dostęp do lokalnych maksimów funkcji: ======

// iterator wskazujący na pierwsze lokalne maksimum
    mx_iterator mx_begin() const noexcept;

// iterator wskazujący za ostatnie lokalne maksimum
    mx_iterator mx_end() const noexcept;

private:
    using fun_t = std::multiset<point_type, FunCmp<A, V>>;
    using maxs_t = std::set<point_type, MaxsCmp<A, V>>;
    fun_t fun;
    maxs_t maxs;

// safe version of erase, when provided .end() does nothing
    void fun_erase(iterator it) noexcept;
    void maxs_erase(mx_iterator it) noexcept;

// returns --it iff it != .begin(), else returns .end()
    iterator left(iterator it) const;

// returns ++it iff it!= .end(), else returns .end()
    iterator right(iterator it) const;

    bool isLeftLess(iterator before, iterator it) const;

    bool isRightLess(iterator it, iterator after) const;

    bool isMaximum(iterator before, iterator it, iterator after) const;

// enum type used to deduce how set was or should be modified,
// useful in case of raised exception mid execution
    enum Update_t {
        Ufailed, UtoErase, Uinserted, Unothing
    };

// if "it" is maximum inserts it to the set,
// else if point *it should be erased from the set returns {UtoErase, "iterator"}
// if there is nothing to do returns {Unothing, .end()}
//
// function doesn't return {Ufailed,...}, this value should be set as default
// value of pair it assigns to, so one can deduce if function ended properly or not
    std::pair<Update_t, mx_iterator> updateMaxsPrep(iterator before, iterator it, iterator after);

// compares points *p1_it, *p2_it by value,
// if any of provided iterators is .end() returns false
    bool pointsEq(iterator p1_it, iterator p2_it) const;
};

template<typename A, typename V>
class FunctionMaxima<A, V>::point_type {
public:
// Zwraca argument funkcji.
    A const &arg() const;

// Zwraca wartość funkcji w tym punkcie.
    V const &value() const;

    point_type(const point_type &pt) = default;

    point_type &operator=(const point_type &other); ///*strong*

private:
    std::shared_ptr<A> _arg;
    std::shared_ptr<V> _val;

    point_type(const A &a, const V &v);

    explicit point_type(const A &a);

    friend class FunctionMaxima<A, V>;

};

/// =========== comparators implementation ===========

namespace {
    template<typename A, typename V>
    struct FunCmp {
        bool operator()(const typename FunctionMaxima<A, V>::point_type &p1,
                        const typename FunctionMaxima<A, V>::point_type &p2) const {
            return p1.arg() < p2.arg();
        }
    };

    template<typename A, typename V>
    struct MaxsCmp {
        bool operator()(const typename FunctionMaxima<A, V>::point_type &p1,
                        const typename FunctionMaxima<A, V>::point_type &p2) const {
            if (!(p1.value() < p2.value()) && !(p2.value() < p1.value()))
                return p1.arg() < p2.arg();
            return (!(p1.value() < p2.value()));
        }
    };
}

/// =========== point_type implementation ===========

template<typename A, typename V>
typename FunctionMaxima<A, V>::point_type &
FunctionMaxima<A, V>::point_type::operator=(
        const FunctionMaxima<A, V>::point_type &other
) {
    if (this == &other)
        return *this;
    std::shared_ptr<A> arg_tmp = other._arg;
    std::shared_ptr<V> val_tmp = other._val;

    std::swap(_arg, arg_tmp);
    std::swap(_val, val_tmp);
}

template<typename A, typename V>
FunctionMaxima<A, V>::point_type::point_type(const A &a, const V &v) {
    _arg = std::make_shared<A>(a);
    _val = std::make_shared<V>(v);
}

template<typename A, typename V>
FunctionMaxima<A, V>::point_type::point_type(const A &a) {
    _arg = std::make_shared<A>(a);
}

template<typename A, typename V>
const A &FunctionMaxima<A, V>::point_type::arg() const {
    return *_arg;
}

template<typename A, typename V>
const V &FunctionMaxima<A, V>::point_type::value() const {
    return *_val;
}

/// =========== FunctionMaxima implementation ===========

template<typename A, typename V>
bool FunctionMaxima<A, V>::pointsEq(iterator p1_it, iterator p2_it) const {
    if (p1_it == fun.end() || p2_it == fun.end())
        return false;
    MaxsCmp<A, V> cmp;
    return !cmp(*p1_it, *p2_it) && !cmp(*p2_it, *p1_it);
}

template<typename A, typename V>
FunctionMaxima<A, V> &
FunctionMaxima<A, V>::operator=(const FunctionMaxima<A, V> &other) {
    if (this == &other)
        return *this;
    fun_t fun_tmp = other.fun;
    maxs_t maxs_tmp = other.maxs;

    std::swap(fun, fun_tmp);
    std::swap(maxs, maxs_tmp);

    return *this;
}

template<typename A, typename V>
void FunctionMaxima<A, V>::fun_erase(iterator it) noexcept {
    if (it == fun.end())
        return;
    fun.erase(it);
}

template<typename A, typename V>
void FunctionMaxima<A, V>::maxs_erase(mx_iterator it) noexcept {
    if (it == maxs.end())
        return;
    maxs.erase(it);
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::left(iterator it) const {
    if (it == fun.begin())
        return fun.end();
    if (it == fun.end())
        return fun.end();
    return --it;
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::right(iterator it) const {
    if (it == fun.end()) {
        return fun.end();
    }
    return ++it;
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::isLeftLess(iterator before, iterator it) const {
    if (it == fun.end())
        return false;
    if (before == fun.end())
        return true;
    if (!(it->value() < before->value()))
        return true;
    return false;
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::isRightLess(iterator it, iterator after) const {
    if (it == fun.end())
        return false;
    if (after == fun.end())
        return true;
    if (!(it->value() < after->value()))
        return true;
    return false;
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::isMaximum(iterator before,
                                     iterator it,
                                     iterator after) const {
    if (it == fun.end())
        return false;
    return isLeftLess(before, it) && isRightLess(it, after);
}

template<typename A, typename V>
std::pair<typename FunctionMaxima<A, V>::Update_t, typename FunctionMaxima<A, V>::mx_iterator>
FunctionMaxima<A, V>::updateMaxsPrep(iterator before, iterator it, iterator after) {
    if (it == fun.end()) {
        return {Unothing, maxs.end()};
    } else if (isMaximum(before, it, after)) {
        auto[ret_it, inserted] = maxs.insert(*it);
        return {inserted ? Uinserted : Unothing, ret_it};
    } else {
        auto ret_it = maxs.find(*it);
        return {ret_it == maxs.end() ? Unothing : UtoErase, ret_it};
    }
}

template<typename A, typename V>
V const &FunctionMaxima<A, V>::value_at(A const &a) const {
    auto point_it = fun.find(point_type(a));

    if (point_it == fun.end())
        throw InvalidArg();

    return point_it->value();
}

template<typename A, typename V>
void FunctionMaxima<A, V>::erase(A const &a) {
    auto point_it = fun.find(point_type(a));
    auto point_maxit = maxs.end();
    if (point_it != fun.end())
        point_maxit = maxs.find(*point_it);
    auto left_it = left(point_it);
    auto right_it = right(point_it);
    std::pair<Update_t, mx_iterator> leftStatus = {Ufailed, maxs.end()};
    std::pair<Update_t, mx_iterator> rightStatus = {Ufailed, maxs.end()};

    try {
        leftStatus = updateMaxsPrep(left(left_it), left_it, right_it);
        rightStatus = updateMaxsPrep(left_it, right_it, right(right_it));

        fun_erase(point_it);
        maxs_erase(point_maxit);

        if (leftStatus.first == UtoErase)
            maxs_erase(leftStatus.second);
        if (rightStatus.first == UtoErase)
            maxs_erase(rightStatus.second);
    } catch (...) {
        if (leftStatus.first == Uinserted)
            maxs_erase(leftStatus.second);
        if (rightStatus.first == Uinserted)
            maxs_erase(rightStatus.second);
        throw;
    }
}

template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(A const &a, V const &v) {
    auto oldPoint_it = fun.find(point_type(a));
    auto oldPoint_maxit = maxs.end();
    iterator left_it;
    iterator right_it;
    if (oldPoint_it != fun.end()) {
        oldPoint_maxit = maxs.find(*oldPoint_it);
        left_it = left(oldPoint_it);
        right_it = right(oldPoint_it);
    }
    auto newPoint_it = fun.insert(point_type(a, v));
    if (oldPoint_it == fun.end()) {
        left_it = left(newPoint_it);
        right_it = right(newPoint_it);
    }
    std::pair<Update_t, mx_iterator> leftStatus = {Ufailed, maxs.end()};
    std::pair<Update_t, mx_iterator> newStatus = {Ufailed, maxs.end()};
    std::pair<Update_t, mx_iterator> rightStatus = {Ufailed, maxs.end()};

    try {
        leftStatus = updateMaxsPrep(left(left_it), left_it, newPoint_it);
        rightStatus = updateMaxsPrep(newPoint_it, right_it, right(right_it));
        newStatus = updateMaxsPrep(left_it, newPoint_it, right_it);

        if (!pointsEq(oldPoint_it, newPoint_it))
            maxs_erase(oldPoint_maxit);
        fun_erase(oldPoint_it);

        if (leftStatus.first == UtoErase)
            maxs_erase(leftStatus.second);
        if (rightStatus.first == UtoErase)
            maxs_erase(rightStatus.second);
    } catch (...) {
        if (leftStatus.first == Uinserted)
            maxs_erase(leftStatus.second);
        if (rightStatus.first == Uinserted)
            maxs_erase(rightStatus.second);
        if (newStatus.first == Uinserted)
            maxs_erase(newStatus.second);
        fun_erase(newPoint_it);
        throw;
    }
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::size_type FunctionMaxima<A, V>::size() const noexcept {
    return fun.size();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::begin() const noexcept {
    return fun.begin();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::end() const noexcept {
    return fun.end();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::find(const A &a) const {
    return fun.find(point_type(a));
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_begin() const noexcept {
    return maxs.begin();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_end() const noexcept {
    return maxs.end();
}

#endif //FUNCTION_MAXIMA_H