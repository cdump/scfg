load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def scfg_deps():
    if "com_github_jbeder_yaml_cpp" not in native.existing_rules():
        http_archive(
            name = "com_github_jbeder_yaml_cpp",
            sha256 = "40ece1e7e4b3d075d92c58eaa2a16948518cd2d303152cfcb6eb3dab8f3b9ee7",
            strip_prefix = "yaml-cpp-1b50109f7bea60bd382d8ea7befce3d2bd67da5f",
            urls = [
                "https://github.com/jbeder/yaml-cpp/archive/1b50109f7bea60bd382d8ea7befce3d2bd67da5f.zip",
            ],
        )
