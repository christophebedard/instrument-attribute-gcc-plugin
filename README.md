# instrument-attribute-gcc-plugin

`gcc` plugin providing an `instrument_function` attribute and other `gcc` flags to whitelist functions to be instrumented.

## What it does

When enabling function instrumentation (`-finstrument-functions`), the built-in flags (`-finstrument-functions-exclude-file-list=file,file,…`, `-finstrument-functions-exclude-function-list=sym,sym,…`) and attribute (`no_instrument_function`) only allow you to *exclude* functions from being instrumented.

This plugin allows you to instrument individual functions by:

* adding the `instrument_function` attribute to a function
* giving a list of file paths (`-fplugin-arg-instrument_attribute-include-file-list=file,file,…`)
* giving a list of function names (`-fplugin-arg-instrument_attribute-include-function-list=sym,sym,…`)

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

## Use the plugin

To use this plugin when building your own application with `gcc`, add the `instrument_function` attribute to the function(s) you want to instrument. For example, to instrument `main()` and `instrumented_function()`, but not `NOT_instrumented_function()`:

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

Then, to build, simply enable `-finstrument-functions` and set the path to the plugin with `-fplugin=path/to/instrument_attribute.so`. Assuming the file above is named `test.c`:

```shell
$ gcc -fplugin=./instrument_attribute.so -finstrument-functions test.c -o test
```

Similar to `gcc`'s [built-in flags](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html), you can also instrument functions by giving a list of file paths and/or by giving a list of names.

```
-fplugin-arg-instrument_attribute-include-file-list=file,file,…
```

```
-fplugin-arg-instrument_attribute-include-function-list=sym,sym,…
```

These matches are done on substrings. If the given `file` value is a substring of a file's path, its functions will be instrumented; if the given `sym` value is a substring of a function's user-visible name, it will be instrumented. 

## Debugging

You can use the `-fplugin-arg-instrument_attribute-debug` flag to enable debugging. It will print the functions for which instrumentation is enabled.

```shell
$ gcc -fplugin=./instrument_attribute.so -finstrument-functions \
  -fplugin-arg-instrument_attribute-debug test/test.c -o test/test

Plugin: instrument_function attribute
instrumented function: (test/test.c:4) instrumented_function
instrumented function: (test/test.c:19) main
```

You can also use the `VERBOSE` environment variable to get even more information and figure out how functions were instrumented.

```shell
$ VERBOSE=1 gcc -fplugin=./instrument_attribute.so -finstrument-functions \
  -fplugin-arg-instrument_attribute-include-file-list=test/other \
  -fplugin-arg-instrument_attribute-include-function-list=instrumented_with_function_list \
  test/test.c -o test/test

Parameters:
        include-file-list: test/other
        include-function-list: instrumented_with_function_list
Plugin: instrument_function attribute
        function instrumented from file list: test/other (instrumented_with_file_list)
instrumented function: (test/other/other_file.h:3) instrumented_with_file_list
instrumented function: (test/test.c:4) instrumented_function
        function instrumented from function name list: instrumented_with_function_list
instrumented function: (test/test.c:14) instrumented_with_function_list
instrumented function: (test/test.c:19) main
```
