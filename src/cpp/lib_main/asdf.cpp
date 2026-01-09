#include <string>
#include <iostream>
#include <experimental/meta>
using namespace std::meta;

struct suppress_t {} Suppress;

consteval auto is_nonspecial_member_function(info mem) {
  return is_function(mem)  && !is_special_member_function(mem);
}

consteval auto pretty_name(info ty) -> std::string_view {
    if (ty == dealias(^^std::string)) {
        return "std::string";
    }
    // else
    return display_string_of(ty);
}

consteval auto interface(info proto) -> std::string_view {
    static constexpr auto ctx = std::meta::access_context::unchecked();
    auto name = std::string{ identifier_of(proto) };

    auto ret = std::string{};
    ret += "class " + name+ " {\n"
        +  "public:\n";
    for (auto mem : members_of(proto, ctx)) {
        if (is_nonspecial_member_function(mem)
            && (
                annotations_of(mem).empty()
                || type_of(annotations_of(mem)[0]) != ^^suppress_t
            )
        ) {
            ret += std::string{"    virtual "} + pretty_name(return_type_of(mem))
                +  " " + identifier_of(mem)
                +  "( ";
            for (bool first = true; auto param : parameters_of(mem)) { 
                if (!first) { ret += ","; }
                first = false;
                ret += pretty_name( type_of(param) );
            }
            ret += " ) =0;\n";
        }
    }
    ret += "    virtual ~" + name + "() = default;\n"
        +  "    " + name + "() = default;\n"
        +  "    " + name + "(" + name + " const&) = delete;\n"
        +  "    void operator=(" + name + " const&) = delete;\n"
        +  "};\n";
    return std::define_static_string(ret);
}

//--------------------------------------------------------------------

namespace __proto {
    class Widget {
        int f();
        void g(std::string);
    };
}

int main() {
    std::cout << "#pragma once\n\n#include <string>\n\n" << interface(^^__proto::Widget);
}
