# Simple Config

https://github.com/cdump/scfg

## How to start
Just embed the following code in your file with main function:
```
#include "scfg/scfg_types.hpp"
// clang-format off
#define SCFG_APP_CONFIG(XX)                                                            \
  XX((input),              std::string,  'i', SCFG_NO_DEFAULT, "required string arg")  \
  XX((num),                unsigned, 0,  8,                    "num with default val") \
  /**/                                                                                 \
  XX((enabled,   filter),  bool,     0,  false,                "filter on/off")        \
  XX((threshold, filter),  double,   0,  1.4,                  "filter threhsold")     \
  /**/                                                                                 \
// clang-format on

#include "scfg/scfg.hpp"
scfg::config cfg;

...

int
main(int argc, char **argv)
{
    try {
        cfg = scfg::init(argc, argv);
    } catch(std::exception &ex) {
        printf("Err: %s\n", ex.what());
        return 1;
    }
    ...
}
```

Now you can use parsed params as `cfg.input` (std::string), `cfg.num` (unsigned), `cfg.filter_enabled` (bool), `cfg.filter_threshold` (double)

## Options priority
0. Default values
1. Values from config file
2. Values from ENV
3. Values from command line

For example, command line option overwrites the config value, ENV var overwrites config value and so on

## Example
See full example in example.cc

* Compile:

```sh
$ g++ example.cc
```

* Show help:

```sh
$ ./a.out --help
Usage: ./a.out [options]
  -h, --help               {bool}        - display this help and exit
  -c, --config             {string}      - path to config file
  -C, --config-template    {bool}        - echo default config to stdout
  -i, --input              {string}      - required string arg
      --num                {number}      - num with default val
      --filter-enabled     {bool}        - filter on/off
      --filter-threshold   {double}      - filter threhsold
```

* Run without args:

```sh
$ ./a.out
Option -i (--input) must be set
```

* Run with required arg:

```sh
$ ./a.out --input xxx
Config values, config-file: (not specified)
  .input = xxx
  .num = 8
  filter.enabled = false
  filter.threshold = 1.400000

input='xxx', filter: [0 ; 1.400000]
```

* Generate default config:

```sh
$ ./a.out -i defaultinput --config-template > config.ini && cat config.ini

## required string arg
## type:string env:CFG_INPUT longopt:--input
input = defaultinput

## num with default val
## type:number env:CFG_NUM longopt:--num
num = 8

#------------------------------
[filter]
#------------------------------
## filter on/off
## type:bool env:CFG_FILTER_ENABLED longopt:--filter-enabled
enabled = false

## filter threhsold
## type:double env:CFG_FILTER_THRESHOLD longopt:--filter-threshold
threshold = 1.400000
```

* Run with config & override config params:

```sh
$ CFG_NUM=41 ./a.out --config config.ini --input aaa

Config values, config-file: config.ini
  .input = aaa
  .num = 41
  filter.enabled = false
  filter.threshold = 1.400000

input='aaa', filter: [0 ; 1.400000]
```
