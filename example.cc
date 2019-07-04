#include <stdio.h>

extern "C" {
// clang-format off
#define SCFG_APP_CONFIG(XX)                                                            \
  XX((input),              SCFG_STRING,  'i', SCFG_NO_DEFAULT, "required string arg")  \
  XX((num),                SCFG_UNSIGNED, 0,  8,               "num with default val") \
  /**/                                                                                 \
  XX((enabled,   filter),  SCFG_BOOL,     0,  false,           "filter on/off")        \
  XX((threshold, filter),  SCFG_DOUBLE,   0,  1.4,             "filter threhsold")     \
  /**/                                                                                 \
// clang-format on

#include "./scfg.h"
}

int
main(int argc, char **argv)
{
    if (!scfg_init(argc, argv))
        exit(EXIT_FAILURE);

    printf("input='%s', filter: [%d ; %f]\n",
            cfg.input,
            cfg.filter_enabled,
            cfg.filter_threshold
          );
}
