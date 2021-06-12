# instrument-attribute-gcc-plugin

`gcc` plugin providing an `instrument_function` attribute and other `gcc` flags to specify which functions should be instrumented.

## What it does

When enabling function instrumentation (`-finstrument-functions`), the [built-in flags](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#Instrumentation-Options) (`-finstrument-functions-exclude-file-list=file,file,…`, `-finstrument-functions-exclude-function-list=sym,sym,…`) and [attribute](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes) (`no_instrument_function`) only allow you to *exclude* functions from being instrumented.

This plugin allows you to instrument individual functions, by:

* adding the `instrument_function` attribute to a function
* giving a list of paths to files with function definitions (`-fplugin-arg-instrument_attribute-include-file-list=file,file,…`)
* giving a list of function names (`-fplugin-arg-instrument_attribute-include-function-list=sym,sym,…`)

For example, you might want to use this when you want to instrument only select functions and avoid instrumenting _everything_, since it adds a measurable overhead.

Of course, you will have to keep in mind that not all functions were instrumented when interpreting the data.
For example, the self-time of a function may be misleading if only a portion of the functions it calls are instrumented.

## Build the plugin

This has been tested with `gcc` 7.4.0, 7.5.0, 8.4.0, 9.3.0, and 10.2.0, but it probably works with other versions as well.

Download plugin headers.
The exact package name depends on the `gcc` major version (e.g. `gcc-9-plugin-dev` for 9.3.0).

```shell
$ sudo apt-get install gcc-9-plugin-dev
```

Then clone and build.

```shell
$ git clone https://github.com/christophebedard/instrument-attribute-gcc-plugin.git
$ cd instrument-attribute-gcc-plugin/
$ make
```

## Use the plugin

To use this plugin when building your own application with `gcc`, add the `instrument_function` attribute to the function(s) you want to instrument.
For example, to instrument `main()` and `instrumented_function()`, but not `NOT_instrumented_function()`:

```c
void __attribute__((instrument_function)) instrumented_function()
{
  // This is instrumented
}

void NOT_instrumented_function()
{
  // This is NOT instrumented
}

int __attribute__((instrument_function)) main()
{
  // This is instrumented
  instrumented_function();
  NOT_instrumented_function();
  return 0;
}
```

Then, to build, simply enable `-finstrument-functions` and set the path to the plugin with `-fplugin=path/to/instrument_attribute.so`.
Assuming the file above is named `test.c`:

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

These matches are done on substrings.
If the given `file` value is a substring of a file's path, its functions will be instrumented; if the given `sym` value is a substring of a function's user-visible name, it will be instrumented.

## Verbose mode and debugging

You can use the `VERBOSE` environment variable, i.e. `VERBOSE=1`.
It will print the functions for which instrumentation is enabled.

```shell
$ VERBOSE=1 gcc -Itest/e2e/include \
    -fplugin=./instrument_attribute.so -finstrument-functions \
    -fplugin-arg-instrument_attribute-include-file-list=test/e2e/src/some_,test/e2e/include/other/other_file.h \
    -fplugin-arg-instrument_attribute-include-function-list=instrumented_with_function_list,myawesomelib_,random_other_function_name \
    -c test/e2e/src/main.c -o test/e2e/obj/main.o
Plugin: instrument_function attribute
  instrumented function: (test/e2e/include/other/other_file.h:23) other_file_instrumented_with_file_list
  instrumented function: (test/e2e/src/main.c:28) instrumented_function
  instrumented function: (test/e2e/src/main.c:38) instrumented_with_function_list
  instrumented function: (test/e2e/src/main.c:43) main
```

You can also use the `-fplugin-arg-instrument_attribute-debug` flag to enable debugging to get much more information and figure out how functions were instrumented.
Enabling debugging will also enable verbose mode.

```shell
$ gcc -Itest/e2e/include -MMD -MP \
    -fplugin=./instrument_attribute.so -finstrument-functions \
    -fplugin-arg-instrument_attribute-debug \
    -fplugin-arg-instrument_attribute-include-file-list=test/e2e/src/some_,test/e2e/include/other/other_file.h \
    -fplugin-arg-instrument_attribute-include-function-list=instrumented_with_function_list,myawesomelib_,random_other_function_name \
    -c test/e2e/src/main.c -o test/e2e/obj/main.o
Plugin parameter:
  include-file-list: test/e2e/src/some_,test/e2e/include/other/other_file.h
    list of size 2: test/e2e/src/some_, test/e2e/include/other/other_file.h, 
Plugin parameter:
  include-function-list: instrumented_with_function_list,myawesomelib_,random_other_function_name
    list of size 3: instrumented_with_function_list, myawesomelib_, random_other_function_name, 
Plugin: instrument_function attribute
    checking file: test/e2e/include/other/other_file.h
      function instrumented from file list: test/e2e/include/other/other_file.h (other_file_instrumented_with_file_list)
  instrumented function: (test/e2e/include/other/other_file.h:23) other_file_instrumented_with_file_list
    function instrumented from attribute: instrumented_function
  instrumented function: (test/e2e/src/main.c:28) instrumented_function
    checking file: test/e2e/src/main.c
    checking function: not_instrumented_function
  NOT instrumented function: (test/e2e/src/main.c:33) not_instrumented_function
    checking file: test/e2e/src/main.c
    checking function: instrumented_with_function_list
      function instrumented from function name list: instrumented_with_function_list
  instrumented function: (test/e2e/src/main.c:38) instrumented_with_function_list
    function instrumented from attribute: main
  instrumented function: (test/e2e/src/main.c:43) main
```

## Tracing example with LTTng

Install [LTTng](https://lttng.org/docs/v2.10/#doc-installing-lttng) (this only requires userspace tracing).

```shell
$ sudo apt-get install lttng-tools liblttng-ust-dev babeltrace
```

Start a session daemon if it's not already running.

```shell
$ lttng-sessiond --daemon
```

Build the plugin, then build your application using the plugin ([see above](#use-the-plugin)).

Then, create an LTTng session.

```shell
$ lttng create test --output=./my-test-trace/
```

Enable the `func_entry`/`func_exit` events.

```shell
$ lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_entry
$ lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_exit
```

Start tracing.

```shell
$ lttng start
```

Run your application, making sure to preload the profiling library (note: the path to the shared library might be different on your system).

```shell
$ LD_PRELOAD=/usr/lib/x86_64-linux-gnu/liblttng-ust-cyg-profile-fast.so ./your/application
```

Stop tracing.

```shell
$ lttng stop
$ lttng destroy
```

Use [babeltrace](https://babeltrace.org/#bt1-get) to view the output.

```shell
$ sudo apt-get install babeltrace
$ babeltrace my-test-trace/
```

Process the trace data however you want (suggestion: [Trace Compass](https://www.eclipse.org/tracecompass/)).

## Contributing

See [`CONTRIBUTING.md`](./CONTRIBUTING.md).
