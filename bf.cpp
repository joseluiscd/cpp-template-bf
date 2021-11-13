#include <cstddef>
#include <cstdio>
#include <string>
#include <type_traits>

using std::size_t;

// List and utils
// --------------

template <char... T>
struct list {
    using type = list;
    static constexpr size_t size = sizeof...(T);
};

template <typename... T>
struct type_list {
    using type = type_list;
    static constexpr size_t size = sizeof...(T);
};

template <char T, typename List>
struct prepend;

template <char T, typename List>
struct append;

template <typename ListA, typename ListB>
struct concat;

template <typename List>
struct head;

template <typename List>
struct tail;

template <char T, char... Q>
struct prepend<T, list<Q...>> {
    using type = list<T, Q...>;
};

template <char T, char... Q>
struct append<T, list<Q...>> {
    using type = list<Q..., T>;
};

template <char... T, char... Q>
struct concat<list<T...>, list<Q...>> {
    using type = list<T..., Q...>;
};

template <char H, char... T>
struct head<list<H, T...>> {
    static constexpr const char value = H;
};

template <>
struct head<list<>> {
    static constexpr const char value = 0;
};

template <char H, char... T>
struct tail<list<H, T...>> {
    using type = list<T...>;
};

template <>
struct tail<list<>> {
    using type = list<>;
};

template <typename T, typename List>
struct push_type;

template <typename List>
struct pop_type;

template <typename List>
struct top;

template <typename T, typename... List>
struct push_type<T, type_list<List...>> {
    using type = type_list<T, List...>;
};

template <typename T, typename... List>
struct pop_type<type_list<T, List...>> {
    using type = type_list<List...>;
};

template <typename T, typename... List>
struct top<type_list<T, List...>> {
    using type = T;
};

template <typename T>
constexpr T cmp(T a, T b)
{
    if (a > b) {
        return -1;
    } else if (a < b) {
        return 1;
    } else {
        return 0;
    }
}

template <typename Program, typename Machine>
struct eval_instr;

template <typename Machine>
struct eval_instr<list<>, Machine> {
    using type = Machine;
};

template <char... I, typename Machine>
struct eval_instr<list<'>', I...>, Machine> {
    struct eval : Machine {
        using tape_left = typename prepend<Machine::tape_val, typename Machine::tape_left>::type;
        static constexpr const char tape_val = head<typename Machine::tape_right>::value;
        using tape_right = std::conditional_t<Machine::tape_right::size == 0, list<>, typename tail<typename Machine::tape_right>::type>;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<'<', I...>, Machine> {
    struct eval : Machine {
        using tape_left = std::conditional_t<Machine::tape_left::size == 0, list<>, typename tail<typename Machine::tape_left>::type>;
        static constexpr const char tape_val = Machine::tape_left::size == 0 ? 0 : head<typename Machine::tape_left>::value;
        using tape_right = typename prepend<Machine::tape_val, typename Machine::tape_right>::type;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<'+', I...>, Machine> {
    struct eval : Machine {
        static constexpr const char tape_val = Machine::tape_val + 1;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<'-', I...>, Machine> {
    struct eval : Machine {
        static constexpr const char tape_val = (Machine::tape_val + 255) % 256;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<'.', I...>, Machine> {
    struct eval : Machine {
        using output = typename append<Machine::tape_val, typename Machine::output>::type;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<',', I...>, Machine> {
    struct eval : Machine {
        static constexpr const char tape_val = head<typename Machine::input>::value;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<'[', I...>, Machine> {
    struct eval : Machine {
        using stack = typename push_type<list<I...>, typename Machine::stack>::type;
    };

    using type = typename eval_instr<list<I...>, eval>::type;
};

template <char... I, typename Machine>
struct eval_instr<list<']', I...>, Machine> {
    using new_code = std::conditional_t<Machine::tape_val == 0, list<I...>, typename top<typename Machine::stack>::type>;

    struct eval : Machine {
        using stack = std::conditional_t<Machine::tape_val == 0, typename pop_type<typename Machine::stack>::type, typename Machine::stack>;
    };

    using type = typename eval_instr<new_code, eval>::type;
};


template <typename List>
struct make_str; 

template <char... C>
struct make_str<list<C...>> {
    static constexpr const char value[] = { C..., 0 };
};

template <typename T>
void print()
{
    std::printf("%s\n", make_str<T>::value);
}

template <typename Char, Char... C>
constexpr auto operator"" _list()
{
    return list<C...>();
}

template <typename Input>
struct machine {
    using output = list<>;
    using input = Input;

    using tape_left = list<>;
    static constexpr char tape_val = 0;
    using tape_right = list<>;

    using stack = type_list<>;
};

template <typename Program, typename Input = list<>>
using eval_machine = typename eval_instr<Program, machine<Input>>::type::output;

// Hello world
using program = decltype(">++++++++[<+++++++++>-]<.>>++++++++++[<++++++++++>-]<+.>>+++++++++[<++++++++++++>-]<.>>+++++++++[<++++++++++++>-]<.>>++++++++++[<+++++++++++>-]<+.>>++++[<++++++++>-]<.>>+++++++++++[<++++++++>-]<-.>>++++++++++[<+++++++++++>-]<+.>>++++++++++[<++++++++++++>-]<------.>>+++++++++[<++++++++++++>-]<.>>++++++++++[<++++++++++>-]<.>>++++++[<++++++>-]<---."_list);
using result = eval_machine<program>; 

int main()
{
    print<result>();

    return 0;
}
