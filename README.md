# instrument-attribute-gcc-plugin

`gcc` plugin providing an `instrument_function` attribute to whitelist functions to be instrumented.

# What it does

When enabling function instrumentation (`-finstrument-functions`), the built-in flags (`-finstrument-functions-exclude-file-list=file,file,…`, `-finstrument-functions-exclude-function-list=sym,sym,…`) and attribute (`no_instrument_function`) only allow you to *exclude* functions from being instrumented.

This plugin allows you to instrument individual functions with the `instrument_function` attribute.

For example, you might want to use this when you want to instrument only select functions and avoid instrumenting the standard library, since it adds a measurable overhead.

## Build

This assumes `gcc` 7.4.0, but it probably works with other versions as well.

Download plugin headers.

```shell
$ sudo apt-get install gcc-7-plugin-dev
```

Then build.

```shell
$ make
```

## Using the plugin

To use this plugin when building your own application with `gcc`, simply set the path to the plugin with `-fplugin=path/to/plugin.so`. Of course, you also need `-finstrument-functions`.

For example, to build [`test.c`](./test.c):

```shell
$ gcc -fplugin=./instrument_attribute.so -finstrument-functions test.c -o test
```

## Debugging

You can use the `-fplugin-arg-instrument_attribute-debug` flag to enable debugging. It will print the functions for which instrumentation is enabled.

```shell
$ gcc -fplugin=./instrument_attribute.so -finstrument-functions -fplugin-arg-instrument_attribute-debug test.c -o test
```
