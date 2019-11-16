# instrument-attribute-gcc-plugin

`gcc` plugin providing an `instrument_function` attribute to whitelist functions to be instrumented.

## What it does

When enabling function instrumentation (`-finstrument-functions`), the built-in flags (`-finstrument-functions-exclude-file-list=file,file,…`, `-finstrument-functions-exclude-function-list=sym,sym,…`) and attribute (`no_instrument_function`) only allow you to *exclude* functions from being instrumented.

This plugin allows you to instrument individual functions with the `instrument_function` attribute.

For example, you might want to use this when you want to instrument only select functions and avoid instrumenting the standard library, since it adds a measurable overhead.

## Build the plugin

This has been tested with `gcc` 7.4.0, but it probably works with other versions as well.

Download plugin headers.

```shell
$ sudo apt-get install gcc-7-plugin-dev
```

Then clone and build.

```shell
$ git clone https://github.com/christophebedard/instrument-attribute-gcc-plugin.git
$ cd instrument-attribute-gcc-plugin/
$ make
```

## Example

To use this plugin when building your own application with `gcc`, simply enable `-finstrument-functions` and set the path to the plugin with `-fplugin=path/to/instrument_attribute.so`.

Add the `instrument_function` attribute to the function(s) you want to instrument. For example, to instrument `main()` and `instrumented_function()`, but not `NOT_instrumented_function()`:

```c
#include <stdio.h>

void __attribute__((instrument_function)) instrumented_function()
{
    printf("this is instrumented\n");
}

void NOT_instrumented_function()
{
    printf("this is NOT instrumented");
}

int __attribute__((instrument_function)) main()
{
    instrumented_function();
    NOT_instrumented_function();
    return 0;
}
```

Then build, assuming the file above is named [`test.c`](./test.c):

```shell
$ gcc -fplugin=./instrument_attribute.so -finstrument-functions test.c -o test
```

## Debugging

You can use the `-fplugin-arg-instrument_attribute-debug` flag to enable debugging. It will print the functions for which instrumentation is enabled.

```shell
$ gcc -fplugin=./instrument_attribute.so -finstrument-functions \
  -fplugin-arg-instrument_attribute-debug test.c -o test

Plugin: instrument_function attribute
instrument_function: (test.c:3) instrumented_function
instrument_function: (test.c:13) main
```
