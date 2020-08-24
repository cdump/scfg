#pragma once

#ifndef SCFG_APP_CONFIG
#error You MUST define SCFG_APP_CONFIG
// #define SCFG_APP_CONFIG(XX)
#endif

// clang-format off
#define SCFG_GEN_CONFIG(XX)                                                         \
  /* (NAME,SECTION)       TYPE        OPT  DEFAULT DESCRIPTION */                   \
    XX((help),            SCFG_BOOL,   'h', false, "display this help and exit")    \
    XX((config),          SCFG_STRING, 'c', "",    "path to config file")           \
    XX((config_template), SCFG_BOOL,   'C', false, "echo default config to stdout") \
    /**/                                                                            \
    SCFG_APP_CONFIG(XX)
// clang-format on

//------------------------
//------------------------
//------------------------
#include <stdlib.h>
#include <getopt.h>

#include "ini.h"

#define SCFG_NO_DEFAULT ((int64_t)0xdeadbeef)

#define STRNAME_SCFG_STRING "string"
#define HASARG_SCFG_STRING 1
#define TYPE_SCFG_STRING char *
#define PSPEC_SCFG_STRING "s"
#define PVAL_SCFG_STRING(name) cfg.name
#define PARSE_SCFG_STRING(name, val) cfg.name = strdup(val ? val : "")

#define STRNAME_SCFG_UNSIGNED "number"
#define HASARG_SCFG_UNSIGNED 1
#define TYPE_SCFG_UNSIGNED unsigned
#define PSPEC_SCFG_UNSIGNED "u"
#define PVAL_SCFG_UNSIGNED(name) cfg.name
#define PARSE_SCFG_UNSIGNED(name, val) cfg.name = atol(val ? val : "")

#define STRNAME_SCFG_DOUBLE "double"
#define HASARG_SCFG_DOUBLE 1
#define TYPE_SCFG_DOUBLE double
#define PSPEC_SCFG_DOUBLE "f"
#define PVAL_SCFG_DOUBLE(name) cfg.name
#define PARSE_SCFG_DOUBLE(name, val) cfg.name = atof(val ? val : "")

#define STRNAME_SCFG_BOOL "bool"
#define HASARG_SCFG_BOOL 0
#define TYPE_SCFG_BOOL bool
#define PSPEC_SCFG_BOOL "s"
#define PVAL_SCFG_BOOL(name) (cfg.name ? "true" : "false")
#define PARSE_SCFG_BOOL(name, val) cfg.name = scfg_parse_bool(val)

#define SCFG_FULL_VAR_0(NAME, ...) NAME
#define SCFG_FULL_VAR_1(NAME, SECTION) SECTION##_##NAME

#define SCFG_SECTION_STR_0(NAME, ...) ""
#define SCFG_SECTION_STR_1(NAME, SECTION) #SECTION

#define SCFG_NAME_STR_0(NAME, ...) #NAME
#define SCFG_NAME_STR_1(NAME, SECTION) #NAME

#define SCFG_VNAME_CALL(FN, A, B, N, ...) FN##_##N(A, B)

#define SCFG_FULL_VAR(VNAME, ...) SCFG_VNAME_CALL(SCFG_FULL_VAR, VNAME, ##__VA_ARGS__, 1, 0)
#define SCFG_SECTION_STR(VNAME, ...) SCFG_VNAME_CALL(SCFG_SECTION_STR, VNAME, ##__VA_ARGS__, 1, 0)
#define SCFG_NAME_STR(VNAME, ...) SCFG_VNAME_CALL(SCFG_NAME_STR, VNAME, ##__VA_ARGS__, 1, 0)

typedef struct {
#define XX(VNAME, TYPE, ...) TYPE_##TYPE(SCFG_FULL_VAR VNAME);
    SCFG_GEN_CONFIG(XX)
#undef XX
} scfg_config_t;

scfg_config_t cfg;

#ifndef COUNTOF
#define COUNTOF(v) (sizeof(v) / sizeof(*v))
#endif

static struct {
#define XX(VNAME, ...) bool(SCFG_FULL_VAR VNAME);
    SCFG_GEN_CONFIG(XX)
#undef XX
} scfg_config_parsed;
bool scfg_init(int argc, char *argv[]);

static bool
scfg_parse_bool(const char *s)
{
    if (!s)
        return true;

    static struct {
        const char *str;
        bool val;
    } tf_strings[] = {
        {"1", true},
        {"true", true},
        {"on", true},
        {"yes", true},
        {"enabled", true},
        {"0", false},
        {"false", false},
        {"off", false},
        {"no", false},
        {"disabled", false},
    };
    unsigned i;
    for (i = 0; i < COUNTOF(tf_strings); i++) {
        if (!strcasecmp(s, tf_strings[i].str))
            return tf_strings[i].val;
    }
    fprintf(stderr, "Can't convert '%s' to bool\n", s);
    exit(EXIT_FAILURE);
}

static int
scfg_handler_ini(void *udata, const char *ini_section, const char *ini_name, const char *ini_value)
{
    (void)udata;

#define XX(VNAME, TYPE, ...) do {                                                                     \
    if (!strcmp(ini_section, (SCFG_SECTION_STR VNAME)) && !strcmp(ini_name, (SCFG_NAME_STR VNAME))) { \
        PARSE_##TYPE(SCFG_FULL_VAR VNAME, ini_value);                                                 \
        scfg_config_parsed.SCFG_FULL_VAR VNAME = true;                                                \
        return 1;                                                                                     \
    }                                                                                                 \
} while(0);

    SCFG_APP_CONFIG(XX)
#undef XX

    fprintf(stderr, "Config file: unknown option %s.%s = %s\n", ini_section, ini_name, ini_value);
    return 0;
}

static char *
scfg_get_longopt_name(const char *section, const char *name)
{
    static char buf[32768];
    static char *bp = buf;
    char *b = bp;
    if (strlen(section)) {
        bp += 2 + sprintf(b, "%s-%s", section, name);
    } else {
        bp += 2 + sprintf(b, "%s", name);
    }
    char *p = b;
    while (*p) {
        if (*p == '_')
            *p = '-';
        p++;
    }
    return b;
}

static char *
scfg_get_env_name(const char *section, const char *name)
{
    static char buf[32768];
    static char *bp = buf;
    char *b = bp;
    if (strlen(section)) {
        bp += 2 + sprintf(b, "CFG_%s_%s", section, name);
    } else {
        bp += 2 + sprintf(b, "CFG_%s", name);
    }
    char *p = b;
    while (*p) {
        *p = toupper(*p);
        p++;
    }
    return b;
}

// Returns:
// true on success
// false on error
static bool
scfg_parse_argv(int argc, char *argv[])
{
    unsigned i = 0;
    optind = 0;

#define XX(...) +1
    static struct option long_options[1 SCFG_GEN_CONFIG(XX)];
#undef XX

#define XX(VNAME, TYPE, ...) do {                                                                  \
    long_options[i].name = scfg_get_longopt_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)); \
    long_options[i].has_arg = HASARG_##TYPE ? required_argument : optional_argument;               \
    long_options[i].flag = NULL;                                                                   \
    long_options[i].val = i;                                                                       \
    i++;                                                                                           \
} while(0);

    SCFG_GEN_CONFIG(XX);
#undef XX

    memset(&long_options[i], 0, sizeof(struct option));

#define XX(...) +3
    char optstring[1 SCFG_GEN_CONFIG(XX)];
    char *p = optstring;
#undef XX

#define XX(VNAME, TYPE, OPT, ...) do { \
    if (OPT) {                         \
        *p++ = (char)OPT;              \
        if (HASARG_##TYPE)             \
            *p++ = ':';                \
    }                                  \
} while(0);

    SCFG_GEN_CONFIG(XX)
#undef XX

    *p = 0;

    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, optstring, long_options, &option_index);
        if (c == -1)
            break;

        if (c < 'A') {
            if (option_index != c)
                return false;
            int pos = 0;

#define XX(VNAME, TYPE, ...) do {                      \
    if (pos++ == option_index) {                       \
        PARSE_##TYPE(SCFG_FULL_VAR VNAME, optarg);     \
        scfg_config_parsed.SCFG_FULL_VAR VNAME = true; \
    }                                                  \
} while(0);

            SCFG_GEN_CONFIG(XX)
#undef XX

        } else {

#define XX(VNAME, TYPE, OPT, ...) do {                 \
    if (OPT == c) {                                    \
        PARSE_##TYPE(SCFG_FULL_VAR VNAME, optarg);     \
        scfg_config_parsed.SCFG_FULL_VAR VNAME = true; \
    }                                                  \
} while(0);

            SCFG_GEN_CONFIG(XX)
#undef XX

        }
    }
    return true;
}

static void
scfg_parse_env()
{
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION) do {                               \
    char *ename = scfg_get_env_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)); \
    char *val = getenv(ename);                                                        \
    if (val) {                                                                        \
        PARSE_##TYPE(SCFG_FULL_VAR VNAME, val);                                       \
        scfg_config_parsed.SCFG_FULL_VAR VNAME = true;                                \
    }                                                                                 \
} while(0);

    SCFG_GEN_CONFIG(XX)
#undef XX
}

static void
scfg_show_help(char *selfname)
{
    fprintf(stderr, "Usage: %s [options]\n", selfname);

    char *lnames[256];
    int lpos = 0, max_lopt_len = 0, max_type_len = 0;

#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION) do {                                    \
    lnames[lpos] = scfg_get_longopt_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)); \
    int cur_lopt_len = strlen(lnames[lpos]);                                               \
    if (cur_lopt_len > max_lopt_len)                                                       \
        max_lopt_len = cur_lopt_len;                                                       \
    int cur_type_len = sizeof(STRNAME_##TYPE) - 1;                                         \
    if (cur_type_len > max_type_len)                                                       \
        max_type_len = cur_type_len;                                                       \
    lpos++;                                                                                \
} while(0);

    SCFG_GEN_CONFIG(XX)
#undef XX

    size_t pos = 0;
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION) do {     \
    fprintf(stderr, "  %c%c%c --%-*s {%s}%-*c - %s\n",      \
            OPT ? '-' : ' ',                                \
            OPT ? OPT : ' ',                                \
            OPT ? ',' : ' ',                                \
            max_lopt_len + 1,                               \
            lnames[pos],                                    \
            STRNAME_##TYPE,                                 \
            max_type_len - (int)sizeof(STRNAME_##TYPE) + 2, \
            ' ',                                            \
            DESCRIPTION);                                   \
    pos++;                                                  \
} while(0);

    SCFG_GEN_CONFIG(XX)
#undef XX
}

static void
scfg_generate_config()
{
    const char *section = NULL, *last_section = NULL;
#define XX(VNAME, TYPE, OPT, DEFAULT, DESCRIPTION) do {                                          \
    section = SCFG_SECTION_STR VNAME;                                                            \
    if (strlen(section) && (!last_section || strcmp(section, last_section))) {                   \
        printf("#------------------------------\n"                                               \
               "[%s]\n"                                                                          \
               "#------------------------------\n",                                              \
               section);                                                                         \
        last_section = section;                                                                  \
    }                                                                                            \
    printf("## %s\n## type:" STRNAME_##TYPE " env:%s longopt:--%s\n", DESCRIPTION,               \
        scfg_get_env_name(section, SCFG_NAME_STR VNAME),                                         \
        scfg_get_longopt_name(section, SCFG_NAME_STR VNAME));                                    \
    printf("%s = %" PSPEC_##TYPE "\n\n", SCFG_NAME_STR VNAME, PVAL_##TYPE(SCFG_FULL_VAR VNAME)); \
} while(0);

    SCFG_APP_CONFIG(XX)
#undef XX
}

static void
scfg_set_defaults() {
#define XX(VNAME, TYPE, OPT, DEFAULT, ...) do {         \
    if ((int64_t)DEFAULT != SCFG_NO_DEFAULT) {          \
        cfg.SCFG_FULL_VAR VNAME = (TYPE_##TYPE)DEFAULT; \
    }                                                   \
} while(0);

    SCFG_GEN_CONFIG(XX)
#undef XX
}

static bool
scfg_check() {
    bool ret = true;

#define XX(VNAME, TYPE, OPT, DEFAULT, ...) do {                                                     \
    if (!scfg_config_parsed.SCFG_FULL_VAR VNAME && (int64_t)DEFAULT == SCFG_NO_DEFAULT) {           \
        const char *lname = scfg_get_longopt_name((SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME)); \
        if (OPT) {                                                                                  \
            fprintf(stderr, "Option -%c (--%s) must be set\n", OPT, lname);                         \
        } else {                                                                                    \
            fprintf(stderr, "Option --%s must be set\n", lname);                                    \
        }                                                                                           \
        ret = false;                                                                                \
    }                                                                                               \
} while(0);

    SCFG_GEN_CONFIG(XX)
#undef XX

    return ret;
}

bool
scfg_init(int argc, char *argv[])
{
    // 0. set defaults
    scfg_set_defaults();

    // 1. argv to get config filename
    if (!scfg_parse_argv(argc, argv))
        return false;

    // 2. read options from config filename
    if (cfg.config && strlen(cfg.config) && ini_parse(cfg.config, scfg_handler_ini, NULL) < 0) {
        fprintf(stderr, "Failed to parse config file '%s'\n", cfg.config);
        return false;
    }

    // 3. parse env
    scfg_parse_env();

    // 4. parse argv again to overwrite config/env values
    if (!scfg_parse_argv(argc, argv))
        return false;

    // 5. error if no values for options without default
    if (!cfg.help && !cfg.config_template && !scfg_check())
        return false;

    if (cfg.help) {
        scfg_show_help(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (cfg.config_template) {
        scfg_generate_config();
        exit(EXIT_SUCCESS);
    }

    // 5. print values:
    fprintf(stderr, "Config values, config-file: %s\n", (cfg.config && strlen(cfg.config)) ? cfg.config : "(not specified)");

#define XX(VNAME, TYPE, ...) fprintf(stderr, "  %s.%s = %" PSPEC_##TYPE "\n", (SCFG_SECTION_STR VNAME), (SCFG_NAME_STR VNAME), PVAL_##TYPE(SCFG_FULL_VAR VNAME));
    SCFG_APP_CONFIG(XX)
#undef XX

    fprintf(stderr, "\n");

    return true;
}
