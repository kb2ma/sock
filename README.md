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

The RIOT source tree contains necessary sock headers. It is a submodule of this repository.
Update it using

    # git submodule update --init

Some of the applications have a Makefile, just go into their directory and run

    # make

There's also a set of [pyjam](https://github.com/kaspar030/pyjam) buildfiles.

If you've got pyjam installed, use them like this:

    # pyj -a

## Unit tests

Unit tests are provided by reuse of the Embedded Unit framework in the RIOT submodule.
First, compile the framework.

    # cd tests/embunit
    # make

Then, make and run the unit tests as required.

    # cd ..
    # make
    # ./tests