#include <cstdio>

#include "scfg/scfg_types.hpp"

// clang-format off
#define SCFG_APP_CONFIG(XX)                                                                    \
  XX((input),                       std::string, 'i', SCFG_NO_DEFAULT, "required string arg")  \
  XX((file),                        std::string, 0,   "/etc/passwd",   "string arg default")   \
  XX((num),                         unsigned,    0,   8,               "num with default val") \
  /**/                                                                                         \
  XX((enabled,             filter), bool,        0,   false,           "filter on/off")        \
  XX((threshold,           filter), double,      0,   1.5,             "filter threhsold")     \
  XX((val,       subgroup, filter), double,      0,   1.2,             "filter subgroup")      \
  /**/                                                                                         \
// clang-format on

#include "scfg/scfg.hpp"

scfg::config cfg;

int
main(int argc, char **argv)
{
    try {
        cfg = scfg::init(argc, argv);
    } catch(std::exception &ex) {
        printf("Err: %s\n", ex.what());
        return 1;
    }

    printf("input='%s', filter: [%d ; %f; %f]\n",
            cfg.input.c_str(),
            cfg.filter_enabled,
            cfg.filter_threshold,
            cfg.filter_subgroup_val
          );
}
