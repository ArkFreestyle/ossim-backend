# STIP Backend

## Development Environment
In order to modify, build and run the ossim-server (backend, really), you need to first set up your development environment.
Follow the steps outlined in the SOP for source installation of the ossim correlation engine.
Make sure you set up all the dependencies and verify that you can run `make` without errors.

At a high-level, you must:
1. Install Alienvault OSSIM and set up your VM (using the iso from AT&T's website).
1. Follow the procedure in the SOP (for building the correlation engine from source).
1. Clone this repository.
1. `cd stip-backend/os-sim/src`
1. Run `make clean && make install` and see if it works.
    - (I think it should work, but I haven't tested this yet. If you run into errors, please update this readme with the solution under common issues).
    - You may need to `export PKG_CONFIG_PATH=/root/gnet-2.0.8/` if it complains about gnet.
1. Run the generated executable using `./ossim-server`
1. `./ossim-server -h` displays the flags that ossim-server can run with.
1. Great, now you're ready to work on the ossim-server. Your development workflow will likely look like:
    - modify source code -> build using the Makefile -> run the built executable and observe if it does what you intended -> repeat

## Common issues
This is the place to list common issues with fairly simple fixes.

Issue 01: Make worked, but my ossim-server executable exits with the following!

```
(ossim-server:19164): libsoup-CRITICAL **: soup_server_quit: assertion 'priv->listeners != NULL' failed

(ossim-server:19164): GLib-GObject-CRITICAL **: object SoupServer 0x55a1b65250a0 finalized while still in-construction

(ossim-server:19164): GLib-GObject-CRITICAL **: Custom constructor for class SoupServer returned NULL (which is invalid). Please use GInitable instead.

(ossim-server:19164): libsoup-CRITICAL **: soup_server_listen_local: assertion 'SOUP_IS_SERVER (server)' failed
```

Solution: This happens when an ossim-server instance is already running (probably in the background). Kill it with `pgrep ossim-server && pkill -9 ossim-server` and it should run.

## About STIP
STIP stands for Security & Threat Intelligence Platform and is our fork of Alienvault (now AT&T)'s OSSIM. Our goal is to add more features in here and keep the codebase beautiful and bug-free. (Spaghetti coders, stay away).

## STIP-specific features
- Alarm Forwarding
    - Writing alarms into a file
    - Writing alarms into a different database (trying Mongodb)
