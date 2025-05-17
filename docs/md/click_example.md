# click_example Usage

This example demonstrates how to build and run a CLI application using `click.hpp`. It also shows loading option values from an INI file.

## Build

```sh
cmake -S . -B build --preset build-linux
cmake --build build
```

The resulting binary is `build/bin/example_click`.

## Sample INI file

The example looks for `example/example.ini`. A minimal configuration:

```ini
[example_cli]
verbose=true

[example_cli.process]
input_file=input.dat
output=output.dat
count=5
threshold=0.7
retries=2
coords=1,2
```

## Running

Show help:

```sh
build/bin/example_click --help
```

Run the `process` subcommand with arguments:

```sh
build/bin/example_click process input.dat -n 10
```

Options may also come from the INI file.

## Using INI configuration

The application loads `example/example.ini` by default. You can specify an
alternative file with `--ini <file>`. Values from the file populate both options
and arguments after the CLI is set up. Command line input still overrides the
file. If `apply_ini_config` is called before options are added, the values will
not be applied.

Example using only the configuration values:

```sh
build/bin/example_click process --ini example/example.ini
```
