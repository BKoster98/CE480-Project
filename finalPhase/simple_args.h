//
// A _VERY_ simple arg parser, will not handle a number of cases, but
// it is small with no dependencies.
//

#ifndef ARGS_SIMPLE_ARGS_H
#define ARGS_SIMPLE_ARGS_H

#include <string>

class simple_args {
public:
    simple_args(int _argc, const char** _argv) : argc(_argc), argv(_argv) { }

    std::string operator()(std::string flag, std::string default_value = "") {
        return each_pair([flag](std::string key, std::string) {
            return key.ends_with(flag);
        }, default_value);
    }

private:
    template <typename F>
    std::string each_pair(F&& lambda, std::string default_value = "");

    int argc;
    const char** argv;
};

template<typename F>
std::string simple_args::each_pair(F &&lambda, std::string default_value) {
    for (int i = 1; i < argc - 1; i += 2) {
        if (lambda(argv[i], argv[i + 1])) return argv[i + 1];
    }
    return default_value;
}

#endif //ARGS_SIMPLE_ARGS_H
