#pragma once

// only for editing, must be already included
#include "scfg_types.hpp"

#ifndef SCFG_APP_CONFIG
// #error You MUST define SCFG_APP_CONFIG
#define SCFG_APP_CONFIG(XX)
#endif

#include <algorithm>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <getopt.h>

// clang-format off
#define SCFG_GEN_CONFIG(XX)                                                                                \
  /* (NAME,SECTION)       TYPE        OPT  DEFAULT  DESCRIPTION */                                         \
    XX((help),                   bool,        'h', false,  "display this help and exit")                   \
    XX((config),                 std::string, 'c', "",     "path to config file")                          \
    XX((config_template),        bool,        'C', false,  "echo default config to stdout")                \
    XX((config_template_format), std::string,  0,  "yaml", "config-template output format (yaml/env/ini)") \
    /**/                                                                                                   \
    SCFG_APP_CONFIG(XX)
// clang-format on

#define SCFG_NO_DEFAULT

#define SCFG_FULL_VAR_0(NAME, ...) NAME
#define SCFG_FULL_VAR_1(NAME, SECTION, ...) SECTION##_##NAME
#define SCFG_FULL_VAR_2(NAME, SUBSECTION, SECTION) SECTION##_##SUBSECTION##_##NAME

#define SCFG_SECTION_STR_0(NAME, ...) ""
#define SCFG_SECTION_STR_1(NAME, SECTION, ...) #SECTION
#define SCFG_SECTION_STR_2(NAME, SUBSECTION, SECTION) #SECTION

#define SCFG_SUBSECTION_STR_0(NAME, ...) ""
#define SCFG_SUBSECTION_STR_1(NAME, SECTION, ...) ""
#define SCFG_SUBSECTION_STR_2(NAME, SUBSECTION, SECTION) #SUBSECTION

#define SCFG_NAME_STR_0(NAME, ...) #NAME
#define SCFG_NAME_STR_1(NAME, SECTION, ...) #NAME
#define SCFG_NAME_STR_2(NAME, SUBSECTION, SECTION) #SUBSECTION"_"#NAME

#define SCFG_YAMLNAME_STR_0(NAME, ...) #NAME
#define SCFG_YAMLNAME_STR_1(NAME, ...) #NAME
#define SCFG_YAMLNAME_STR_2(NAME, ...) #NAME

#define SCFG_VNAME_CALL(FN, A, B, C, N, ...) FN##_##N(A, B, C)

#define SCFG_FULL_VAR(VNAME, ...) SCFG_VNAME_CALL(SCFG_FULL_VAR, VNAME, ##__VA_ARGS__, 2, 1, 0)
#define SCFG_SECTION_STR(VNAME, ...) SCFG_VNAME_CALL(SCFG_SECTION_STR, VNAME, ##__VA_ARGS__, 2, 1, 0)
#define SCFG_SUBSECTION_STR(VNAME, ...) SCFG_VNAME_CALL(SCFG_SUBSECTION_STR, VNAME, ##__VA_ARGS__, 2, 1, 0)
#define SCFG_NAME_STR(VNAME, ...) SCFG_VNAME_CALL(SCFG_NAME_STR, VNAME, ##__VA_ARGS__, 2, 1, 0)
#define SCFG_YAMLNAME_STR(VNAME, ...) SCFG_VNAME_CALL(SCFG_YAMLNAME_STR, VNAME, ##__VA_ARGS__, 2, 1, 0)

namespace scfg {

struct config {
#define XX(VNAME, TYPE, OPT, DEFAULT, ...) TYPE SCFG_FULL_VAR VNAME{DEFAULT};
    SCFG_GEN_CONFIG(XX)
#undef XX
};

namespace impl { // internal


namespace ini {
#include "ini.h"
} // namespace ini


struct config_parsed {
#define XX(VNAME, ...) bool SCFG_FULL_VAR VNAME {false};
    SCFG_GEN_CONFIG(XX)
#undef XX
};

template <class T>
inline void
parse(std::string_view value, T &out, std::string_view param_name)
{
    try {
        scfg::parse(value, out);
    } catch (std::exception &e) {
        throw std::invalid_argument{"option '" + std::string{param_name} + "' value '" + std::string{value} + "' parse error: " + e.what()};
    }
}
template <class T>
inline void
parse_yaml(const YAML::Node &value, T &out, std::string_view param_name)
{
    try {
        scfg::parse_yaml(value, out);
    } catch (std::exception &e) {
        throw std::invalid_argument{"option '" + std::string{param_name} + "' parse error: " + e.what()};
    }
}

inline std::string
longopt_name(std::string_view section, std::string_view name)
{
    std::string ret;
    auto t = [](std::string_view::value_type c) {
        return c == '_' ? '-' : c;
    };
    if (!section.empty()) {
        std::transform(section.begin(), section.end(), std::back_inserter(ret), t);
        ret += '-';
    }
    std::transform(name.begin(), name.end(), std::back_inserter(ret), t);
    return ret;
}

inline std::string
env_name(std::string_view section, std::string_view name)
{
    std::string ret{"CFG_"};
    auto t = [](std::string_view::value_type c) {
        return std::toupper(c);
    };
    if (!section.empty()) {
        std::transform(section.begin(), section.end(), std::back_inserter(ret), t);
        ret += '_';
    }
    std::transform(name.begin(), name.end(), std::back_inserter(ret), t);
    return ret;
}

inline int
handler_ini(void *udata, const char *ini_section, const char *ini_name, const char *ini_value)
{
    auto &[cfg, parsed_options] = *reinterpret_cast<std::pair<scfg::config&, scfg::impl::config_parsed&>*>(udata);

    std::string_view section{ini_section}, name{ini_name}, value{ini_value};
#define XX(VNAME, TYPE, ...)                                                    \
    if (section == SCFG_SECTION_STR VNAME && name == SCFG_NAME_STR VNAME) {     \
        scfg::impl::parse(value, cfg.SCFG_FULL_VAR VNAME, SCFG_NAME_STR VNAME); \
        parsed_options.SCFG_FULL_VAR VNAME = true;                              \
        return 1;                                                               \
    }
    SCFG_APP_CONFIG(XX)
#undef XX

    // If we can't find var for ini name='.filter_enabled', try 'filter_enabled' (section=filter, name=enabled),
    // because we split some vars to section+name during yaml migration
    std::string_view::size_type p{0};
    if ((p = name.find('_')) != std::string_view::npos) {
        section = name.substr(0, p);
        name = name.substr(p+1);
#define XX(VNAME, TYPE, ...)                                                    \
    if (section == SCFG_SECTION_STR VNAME && name == SCFG_NAME_STR VNAME) {     \
        scfg::impl::parse(value, cfg.SCFG_FULL_VAR VNAME, SCFG_NAME_STR VNAME); \
        parsed_options.SCFG_FULL_VAR VNAME = true;                              \
        return 1;                                                               \
    }
    SCFG_APP_CONFIG(XX)
#undef XX
    }

    fprintf(stderr, "Config file: unknown option %s.%s = %s\n", ini_section, ini_name, ini_value);
    return 0;
}

inline void
parse_config_ini(scfg::config &cfg, scfg::impl::config_parsed &parsed_options)
{
    auto ini_ctx = std::make_pair(std::ref(cfg), std::ref(parsed_options));
    if (scfg::impl::ini::ini_parse(cfg.config.c_str(), scfg::impl::handler_ini, &ini_ctx) < 0)
        throw std::runtime_error{"failed to parse config file " + cfg.config};
}

inline void
parse_config_yaml(scfg::config &cfg, scfg::impl::config_parsed &parsed_options)
{
    YAML::Node root = YAML::LoadFile(cfg.config);
#define XX(VNAME, TYPE, ...) do {                                                   \
    std::string section{SCFG_SECTION_STR VNAME};                                    \
    std::string subsection{SCFG_SUBSECTION_STR VNAME};                              \
    std::string key{SCFG_YAMLNAME_STR VNAME};                                       \
    YAML::Node node;                                                                \
    if (section.empty()) {                                                          \
        node = root[key];                                                           \
    } else if (subsection.empty()) {                                                \
        node = root[section][key];                                                  \
    } else {                                                                        \
        node = root[section][subsection][key];                                      \
    }                                                                               \
    if (node) {                                                                     \
        scfg::impl::parse_yaml(node, cfg.SCFG_FULL_VAR VNAME, SCFG_NAME_STR VNAME); \
        parsed_options.SCFG_FULL_VAR VNAME = true;                                  \
    }                                                                               \
} while(0);
    SCFG_APP_CONFIG(XX)
#undef XX
}

inline void
parse_argv(scfg::config &cfg, int argc, char *argv[], scfg::impl::config_parsed &parsed_options)
{
    constexpr auto longopt_offset = 256;

#define XX(...) +1
    constexpr size_t options_cnt = 0 SCFG_GEN_CONFIG(XX);
#undef XX

    struct option long_options[options_cnt + 1];
    char optstring[options_cnt * 2 + 1];
    char *p = optstring;

    std::vector<std::string> names;
    names.reserve(options_cnt);

    int pos = 0;
#define XX(VNAME, TYPE, OPT, ...)                                                                  \
    do {                                                                                           \
        names.emplace_back(scfg::impl::longopt_name(SCFG_SECTION_STR VNAME, SCFG_NAME_STR VNAME)); \
        long_options[pos].name = names.back().c_str();                                             \
        long_options[pos].has_arg = scfg::opt_arg_required<TYPE> ? 1 : 2;                          \
        long_options[pos].flag = NULL;                                                             \
        long_options[pos].val = longopt_offset + pos;                                              \
        pos++;                                                                                     \
        if (OPT) {                                                                                 \
            *p++ = (char)OPT;                                                                      \
            if (scfg::opt_arg_required<TYPE>)                                                      \
                *p++ = ':';                                                                        \
        }                                                                                          \
    } while (0);
    SCFG_GEN_CONFIG(XX);
#undef XX

    std::memset(&long_options[pos], 0, sizeof(struct option));
    *p = '\0';

    optind = 1; // ext global var
    int c{0};
    while ((c = getopt_long(argc, argv, optstring, long_options, nullptr)) != -1) {
        int pos{0};
#define XX(VNAME, TYPE, OPT, ...)                                                              \
    if (OPT == c || (pos + longopt_offset == c)) {                                             \
        scfg::impl::parse(optarg ? optarg : "", cfg.SCFG_FULL_VAR VNAME, SCFG_NAME_STR VNAME); \
        parsed_options.SCFG_FULL_VAR VNAME = true;                                             \
        continue;                                                                              \
    }                                                                                          \
    pos++;
        SCFG_GEN_CONFIG(XX)
#undef XX
        throw std::runtime_error{"bug option"};
    }
}

inline void
parse_env(scfg::config &cfg, scfg::impl::config_parsed &parsed_options)
{
#define XX(VNAME, ...)                                                                      \
    do {                                                                                    \
        auto ename = scfg::impl::env_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)); \
        if (char *val = ::getenv(ename.data()); val) {                                      \
            scfg::impl::parse(val, cfg.SCFG_FULL_VAR VNAME, SCFG_NAME_STR VNAME);           \
            parsed_options.SCFG_FULL_VAR VNAME = true;                                      \
        }                                                                                   \
    } while (0);
    SCFG_GEN_CONFIG(XX)
#undef XX
}

inline void
show_help(const char *selfname)
{
    fprintf(stderr, "Usage: %s [options]\n", selfname);

    std::vector<std::string> longopt_names;
    size_t max_lopt_len{0};
    size_t max_type_len{0};
#define XX(VNAME, TYPE, OPT, ...)                                                                      \
    longopt_names.emplace_back(scfg::impl::longopt_name(SCFG_SECTION_STR VNAME, SCFG_NAME_STR VNAME)); \
    max_lopt_len = std::max(max_lopt_len, longopt_names.back().size());                                \
    max_type_len = std::max(max_type_len, scfg::type_name<TYPE>.size());
    SCFG_GEN_CONFIG(XX)
#undef XX

    size_t pos = 0;
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION)                                          \
    do {                                                                                    \
        static_assert(!type_name<TYPE>.empty(),                                             \
            "no scfg::type_name template specialization for type " #TYPE);                  \
        fprintf(                                                                            \
            stderr,                                                                         \
            "  %c%c%c --%-*s {%.*s}%-*c - " DESCRIPTION " (env:%s)\n",                      \
            (OPT) ? '-' : ' ',                                                              \
            (OPT) ? (OPT) : ' ',                                                            \
            (OPT) ? ',' : ' ',                                                              \
            int(max_lopt_len + 1),                                                          \
            longopt_names[pos].c_str(),                                                     \
            int(scfg::type_name<TYPE>.size()),                                              \
            scfg::type_name<TYPE>.data(),                                                   \
            int(max_type_len - scfg::type_name<TYPE>.size() + 2),                           \
            ' ',                                                                            \
            scfg::impl::env_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)).c_str()   \
        );                                                                                  \
        pos++;                                                                              \
    } while (0);
    SCFG_GEN_CONFIG(XX)
#undef XX
}

inline void
print_config_ini(const scfg::config &cfg)
{
    std::string_view last_section;
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION)                          \
    do {                                                                    \
        std::string_view section{SCFG_SECTION_STR VNAME};                   \
        if (!section.empty() && section != last_section) {                  \
            printf(                                                         \
                "#------------------------------\n"                         \
                "[%.*s]\n"                                                  \
                "#------------------------------\n",                        \
                (int)section.size(),                                        \
                section.data());                                            \
            last_section = section;                                         \
        }                                                                   \
        printf(                                                             \
            "## " DESCRIPTION "\n"                                          \
            "## type:%s env:%s longopt:--%s\n"                              \
            "%s = %s\n\n",                                                  \
            scfg::type_name<TYPE>.data(),                                   \
            scfg::impl::env_name(section, SCFG_NAME_STR VNAME).c_str(),     \
            scfg::impl::longopt_name(section, SCFG_NAME_STR VNAME).c_str(), \
            SCFG_NAME_STR VNAME,                                            \
            scfg::format(cfg.SCFG_FULL_VAR VNAME).c_str());                 \
    } while (0);

    SCFG_APP_CONFIG(XX)
#undef XX
}

inline std::string
generate_config_yaml(const scfg::config &cfg)
{
    YAML::Node node;
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION)               \
    do {                                                         \
        std::string section{SCFG_SECTION_STR VNAME};             \
        std::string subsection{SCFG_SUBSECTION_STR VNAME};       \
        std::string key{SCFG_YAMLNAME_STR VNAME};                \
        auto value = scfg::format_yaml(cfg.SCFG_FULL_VAR VNAME); \
        if (section.empty()) {                                   \
            node[key] = value;                                   \
        } else {                                                 \
            if (subsection.empty()) {                            \
                node[section][key] = value;                      \
            } else {                                             \
                node[section][subsection][key] = value;          \
            }                                                    \
        }                                                        \
    } while (0);

    SCFG_APP_CONFIG(XX)
#undef XX

    YAML::Emitter out;
    out << node;
    return std::string{out.c_str()};
}

inline void
print_config_env(const scfg::config &cfg)
{
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION)                                    \
    printf("%s=%s\n",                                                                 \
       scfg::impl::env_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)).c_str(), \
       scfg::format(cfg.SCFG_FULL_VAR VNAME).c_str()                                  \
    );

    SCFG_APP_CONFIG(XX)
#undef XX
}

static void
check(const scfg::config &cfg, const scfg::impl::config_parsed &parsed_options)
{
    bool ok{true};
#define XX(VNAME, TYPE, OPT, DEFAULT, ...)                                                            \
    do {                                                                                              \
        if (std::string_view{#DEFAULT} == "SCFG_NO_DEFAULT" && !parsed_options.SCFG_FULL_VAR VNAME) { \
            auto lname = scfg::impl::longopt_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME));   \
            if (OPT) {                                                                                \
                fprintf(stderr, "Option -%c (--%s) must be set\n", OPT, lname.c_str());               \
            } else {                                                                                  \
                fprintf(stderr, "Option --%s must be set\n", lname.c_str());                          \
            }                                                                                         \
            ok = false;                                                                               \
        }                                                                                             \
    } while (0);
    SCFG_GEN_CONFIG(XX)
#undef XX
    if (!ok)
        throw std::runtime_error{"not all required options provided"};
}

inline void
print(const scfg::config &cfg)
{
    fprintf(stderr, "Config values, config-file: %s\n", !cfg.config.empty() ? cfg.config.c_str() : "(not specified)");
    fprintf(stderr, "---\n%s\n...\n", generate_config_yaml(cfg).c_str());
}
}; // namespace impl

inline scfg::config
init(int argc, char *argv[])
{
    scfg::config cfg;
    scfg::impl::config_parsed parsed_options;

    // 1. extract config-file path fron env & command-line
    {
        scfg::config tmpcfg;
        scfg::impl::parse_env(tmpcfg, parsed_options);
        scfg::impl::parse_argv(tmpcfg, argc, argv, parsed_options);
        cfg.config = tmpcfg.config;
    }

    // 2. read options from config filename
    if (!cfg.config.empty()) {
        if (!scfg::impl::ini::ini_parse(cfg.config.c_str(), +[](void *, const char *, const char *, const char *){ return 1; }, nullptr)) {
            scfg::impl::parse_config_ini(cfg, parsed_options);
        } else {
            scfg::impl::parse_config_yaml(cfg, parsed_options);
        }
    }

    // 3. parse env
    scfg::impl::parse_env(cfg, parsed_options);

    // 4. parse argv
    scfg::impl::parse_argv(cfg, argc, argv, parsed_options);

    if (cfg.help) {
        scfg::impl::show_help(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (cfg.config_template) {
        if (cfg.config_template_format == "yaml") {
            auto s = scfg::impl::generate_config_yaml(cfg);
            printf("---\n%s\n...\n", s.c_str());
        } else if (cfg.config_template_format == "ini") {
            scfg::impl::print_config_ini(cfg);
        } else if (cfg.config_template_format == "env") {
            scfg::impl::print_config_env(cfg);
        } else {
            throw std::invalid_argument{R"(only "yaml", "env" and "ini" --config-template-format supported)"};
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    // 5. error if no values for options without default
    scfg::impl::check(cfg, parsed_options);

    // 6. print values
    scfg::impl::print(cfg);

    return cfg;
}
}; // namespace scfg
