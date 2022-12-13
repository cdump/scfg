# Simple Config

https://github.com/cdump/scfg

## How to start

1. Add to your bazel `WORKSPACE`:
```
http_archive(
    name = "com_github_cdump_scfg",
    strip_prefix = "scfg-<githash>",
    urls = [
        "https://github.com/cdump/scfg/archive/<githash>.tar.gz",
    ],
)

load("@com_github_cdump_scfg//:scfg_deps.bzl", "scfg_deps")

scfg_deps()
```


2. add the following code in your file with main function:
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
$ bazel build '@//:example'
```

* Show help:

```sh
$ ./bazel-bin/example --help
Usage: ./bazel-bin/example [options]
  -h, --help                 {bool}     - display this help and exit
  -c, --config               {string}   - path to config file
  -C, --config-template      {bool}     - echo default config to stdout
  -i, --input                {string}   - required string arg
      --file                 {string}   - string arg default
      --num                  {number}   - num with default val
      --filter-enabled       {bool}     - filter on/off
      --filter-threshold     {double}   - filter threhsold
      --filter-subgroup-val  {double}   - filter subgroup
```

* Run without args:

```sh
$ ./bazel-bin/example
Option -i (--input) must be set
Err: not all required options provided
```

* Run with required arg:

```sh
$ ./bazel-bin/example --input xxx 
Config values, config-file: (not specified)
  .input = xxx
  .file = /etc/passwd
  .num = 8
  filter.enabled = false
  filter.threshold = 1.500000
  filter.subgroup_val = 1.200000

input='xxx', filter: [0 ; 1.500000; 1.200000]
```

* Generate default config:

```sh
$ ./bazel-bin/example -i defaultinput --config-template > config.yaml && cat config.yaml

---
input: defaultinput
file: /etc/passwd
num: 8
filter:
  enabled: false
  threshold: 1.5
  subgroup:
    val: 1.2
...
```

* Run with config & override config params:

```sh
$ CFG_NUM=41 ./bazel-bin/example --config config.yaml --input aaa
Config values, config-file: config.yaml
  .input = aaa
  .file = /etc/passwd
  .num = 41
  filter.enabled = false
  filter.threshold = 1.500000
  filter.subgroup_val = 1.200000

input='aaa', filter: [0 ; 1.500000; 1.200000]
```
