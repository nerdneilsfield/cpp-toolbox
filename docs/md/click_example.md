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
config=config.json

[example_cli.process]
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

Options may also come from the INI file. You can specify an alternate file for
a command using `--ini`:

```sh
build/bin/example_click process --ini process.ini
```

## Using INI configuration

The application loads `example.ini` by default and applies the values to
matching options once all commands are defined. Command line arguments still
take precedence. You may override the file per command with `--ini` as shown
above. Internally this uses `apply_ini_file`, which can be called after the CLI
setup. Calling it earlier would have no effect because the options would not be
registered yet.
