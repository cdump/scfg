#pragma once

// #include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

#ifdef SCFG_APP_CONFIG
#error You must include this file BEFORE defining SCFG_APP_CONFIG
#endif

namespace scfg {

template<class T>
void parse(std::string_view s, T &out);

template<class T>
inline constexpr
std::string_view type_name{};

template<class T>
std::string format(const T &v);

template<class T>
inline constexpr bool opt_arg_required = true;



template<>
inline void parse(std::string_view s, std::string &out) {
    out = s;
}
template<>
inline constexpr std::string_view type_name<std::string>{"string"};
template<>
inline std::string format(const std::string &v) {
    return v;
}


template<>
inline void parse(std::string_view s, bool &out) {
    if (s.empty() || s=="true" || s=="1" || s=="on" || s=="yes" || s=="enabled") {
        out = true;
    } else if (s=="false" || s=="0" || s=="off" || s=="no" || s == "disabled") {
        out = false;
    } else {
        throw std::runtime_error{"supported values are true/1/on/yes/enabled/false/0/off/no/disabled"};
    }
}
template<>
inline constexpr std::string_view type_name<bool>{"bool"};
template<>
inline std::string format(const bool &v) {
    return v ? "true" : "false";
}
template<>
inline constexpr bool opt_arg_required<bool> = false;


template<>
inline void parse(std::string_view s, unsigned &out) {
    out = std::atol(std::string{s}.c_str());
    // auto res = std::from_chars(s.data(), s.data() + s.size(), out);
    // if (res.ec == std::errc::invalid_argument)
    //     throw std::invalid_argument{"invalid_argument"};
    // if (res.ec == std::errc::result_out_of_range)
    //     throw std::out_of_range{"out_of_range"};
}
template<>
inline constexpr std::string_view type_name<unsigned>{"number"};
template<>
inline std::string format(const unsigned &v) {
    return std::to_string(v);
}


template<>
inline void parse(std::string_view s, double &out) {
    out = std::atof(std::string{s}.c_str());
    // auto res = std::from_chars(s.data(), s.data() + s.size(), out);
    // if (res.ec == std::errc::invalid_argument)
    //     throw std::invalid_argument{"invalid_argument"};
    // if (res.ec == std::errc::result_out_of_range)
    //     throw std::out_of_range{"out_of_range"};
}
template<>
inline constexpr std::string_view type_name<double>{"double"};
template<>
inline std::string format(const double &v) {
    return std::to_string(v);
}
} // namespace scfg
