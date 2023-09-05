# UntitledOpen
[![CI](https://github.com/MadLadSquad/UntitledOpen/actions/workflows/ci.yaml/badge.svg?branch=master)](https://github.com/MadLadSquad/UntitledOpen/actions/workflows/ci.yaml)
[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)
[![trello](https://img.shields.io/badge/Trello-UDE-blue])](https://trello.com/b/HmfuRY2K/untitleddesktop)
[![Discord](https://img.shields.io/discord/717037253292982315.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/4wgH8ZE)

C bindins for the [opener](https://github.com/Seeker14491/opener) library. Since the library is written in Rust,
we decided to write C bindings for it, so we can use it in the 
[UntitledImGuiFramework](https://github.com/MadLadSquad/UntitledImGuiFramework).

## Building
To build, use CMake, either by using it from the CLI or by adding this project as a subdirectory.

## API
Call the `open` function with a URL string as its only argument:
```c
open("https://madladsquad.com/");
```
File URLs, as standardised by the 
[freedesktop-uri-spec](https://www.freedesktop.org/wiki/Specifications/file-uri-spec/) are also accepted.

Additionally, the `reveal` function can open a file browser for a file or directory:
```c
reveal("./");
```

## Dependencies
1. A modern installation of the Rust programming language
1. A C compiler
