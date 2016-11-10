# Overview

This repository contains an implementation of the sock API for Linux.
sock tries to be a simplified replacement for the venerable POSIX socket API.

You can also find a collection of network protocol implementations based on
sock, with the aim of using minimal code and memory.

## What's here?

- nanocoap: a CoAP implementation
- dns: a simple, synchronous DNS client
- ndhcp: simple DHCPv4 client

## How to build

You'll need a checkout of [RIOT](https://github.com/RIOT-OS/RIOT).

Some of the applications have a Makefile, just go into their directory and run

    # RIOTBASE=<path/to/RIOT/checkout> make

There's also a set of [pyjam](https://github.com/kaspar030/pyjam) buildfiles.

Use them like this:

    # RIOTBASE=<path/to/RIOT/checkout> pyj -a

