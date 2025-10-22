# UntitledOpen
[![CI](https://github.com/MadLadSquad/UntitledOpen/actions/workflows/CI.yaml/badge.svg)](https://github.com/MadLadSquad/UntitledOpen/actions/workflows/CI.yaml)
[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)
[![Discord](https://img.shields.io/discord/717037253292982315.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/4wgH8ZE)

A wrapper library that allows for opening doing the following in a cross-platform way:
1. Using native file pickers
1. Opening URIs using the default application for the given URI

Dependencies:
1. [nativefiledialog-extended](https://github.com/MadLadSquad/nativefiledialog-extended) - All file picking operations
1. Opening URIs
   1. Windows - the standard Windows runtime libraries
   1. MacOS - the `CoreFoundation` & `ApplicationServices` frameworks
   1. Linux & Unix-based systems - DBus & XDG desktop portals

## Building & installing
All information can be found on the [wiki](https://github.com/MadLadSquad/UntitledOpen/wiki/Home).
