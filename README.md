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

Note: There are multiple Makefiles in this project spanning across different directories, when this README references a Makefile, you can pretty much be sure it's referring to the Makefile under `/root/stip-backend/os-sim/src`, unless explicitly stated otherwise.

## Adding new .h or .c files
In order to add new .h or .c files to this codebase, we must manually tell the Makefile to create the appropriate object (.o) file(s). Locate the `STIP_OBJECTS` variable inside the Makefile and add your filename:
```
STIP_OBJECTS = <some-old-file>.$(OBJEXT) \
                <your-filename>.$(OBJEXT)
```
Trying to compile using make without doing this *will* result in errors.

Do note that `STIP_OBJECTS` is a variable we manually created. This Makefile is autogenerated by autogen.sh. Ideally, instead of manually editing the Makefile, we should automate this step (which will require some research unless you're already familiar with automake). For now, editing the Makefile is the easier way to go.

## Installing MongoDB
1. `sudo apt-get install -y mongodb`
    - The official website recommendeds the `mongodb-org` package (instead of `mongodb`), but I ran into issues trying to install that.
        - Relevant link: https://stackoverflow.com/questions/28945921/e-unable-to-locate-package-mongodb-org
    - `mongodb` is the unofficial mongodb package provided by Ubuntu and it is not maintained by MongoDB and conflicts with MongoDB’s officially supported packages.
        - It's also not the latest version, but for now, my goal was to just install any version of mongoDB and see if we can get stip to insert alarms there.
    - Possible future to do:
        - Install the latest version (whatever is supported for Debian 9) of mongodb from source.
            - Note, that this *may* require some modification/update to the mongoc-driver code.

### Installing the MongoDB C driver
1. Surprisingly, installation of the C driver isn't straightforward either, I had to build it from source following the instructions linked below.
    - Relevant links:
        - https://stackoverflow.com/questions/51530526/cant-find-mongoc-h
        - https://mongoc.org/libmongoc/current/installing.html
    - Again, this is not the latest version of the driver, we may be interested in upgrading this later.


### Including MongoDB's C driver into our Makefile
(*You don't have to do this. This is meant to outline how I added MongoDB into our Makefile for future reference.*)
1. Created two variables: `LIBMONGOC_CFLAGS` and `LIBMONGOC_LIBS`
    - The former will contain the output of `pkg-config --cflags libmongoc-static-1.0`
        - -DMONGOC_STATIC -DBSON_STATIC -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0
    - The latter will contain the output of `pkg-config --libs libmongoc-static-1.0`
        - -L/usr/local/lib -lmongoc-static-1.0 -lssl -lcrypto -lrt -lresolv -pthread -lz -licuuc -lbson-static-1.0 -lc /usr/lib/x86_64-linux-gnu/librt.so /usr/lib/x86_64-linux-gnu/libm.so -pthread
1. Added `LIBMONGOC_LIBS` into `ossim_server_LDADD`
1. Added `LIBMONGOC_CFLAGS into:
    - `AM_CPPFLAGS`
    - `libparser_a_CFLAGS`

## Common issues
This is the place to list common issues with fairly simple fixes.

**Issue 01:** Make worked, but my ossim-server executable exits with the following!

```
(ossim-server:19164): libsoup-CRITICAL **: soup_server_quit: assertion 'priv->listeners != NULL' failed

(ossim-server:19164): GLib-GObject-CRITICAL **: object SoupServer 0x55a1b65250a0 finalized while still in-construction

(ossim-server:19164): GLib-GObject-CRITICAL **: Custom constructor for class SoupServer returned NULL (which is invalid). Please use GInitable instead.

(ossim-server:19164): libsoup-CRITICAL **: soup_server_listen_local: assertion 'SOUP_IS_SERVER (server)' failed
```

Solution: This happens when an ossim-server instance is already running (probably in the background). Kill it with `pgrep ossim-server && pkill -9 ossim-server` and it should run.

**Issue 02:** The UI no longer loads. Error message: `operation was not completed due to a database error`.
This can happen due to multiple issues. For me, the mysql root user's password had mysteriously been changed (it's supposed to be blank...). So running `mysql -u root` would give:
- `ERROR 1045 (28000): Access denied for user 'root'@'localhost' (using password: NO)` (on mysql shell)

Solution:
The steps outlined in the answer by Li Yingjun, are a good starting point.
- https://stackoverflow.com/questions/10299148/mysql-error-1045-28000-access-denied-for-user-billlocalhost-using-passw

Investigate if this really is your issue by viewing the users table. If it is, follow the commands below:
```
service mysqld stop
mysqld_safe --skip-grant-tables
mysql -u root (in a new terminal)
use mysql;
flush privileges;
ALTER USER 'root'@'localhost' IDENTIFIED BY '';
flush privileges;
quit
service restart mysql
```

## About STIP
STIP stands for Security & Threat Intelligence Platform and is our fork of Alienvault (now AT&T)'s OSSIM. Our goal is to add more features in here and keep the codebase beautiful and bug-free. (Spaghetti coders, stay away).

## STIP-specific features
- Alarm Forwarding
    - Writing alarms into a file
    - Writing alarms into a different database (trying Mongodb)

## File structure and coding guidelines
### File structure
Any STIP related functionality must be in the appropriately named .h/.c file. We will try to keep our changes to the OSSIM codebase minimal (barring bug-fixes). Thus, all you need to do is add your new functionality into the `stip_<something>` files, and simply call your functions from the appropriate location in the OSSIM code.

### Coding guidelines
When in doubt, follow the the way things are done throughout the codebase. This will help keep things consistent and uniform.
Function names are `<modulename>_<something>`
- For example: for a module named `sim-correlation`, you may notice its function names are `sim_correlation_<something>`
- We will keep this convention within our STIP files, so for a module named `stip-mongo`, the function names should be `stip_mongo_<something>`

## Important changes to OSSIM files

Occasionally, I expect that we'll discover a bug contained in the original OSSIM code (not written by us). This section aims to outline our fixes for those bugs so that we can keep track of which files/functions we modified, and perhaps in the future merge them with more recent versions of the OSSIM codebase (as it gets released by AT&T). Before adding anything here, it is *very very* important to first trace and confirm that the issue is not from our additions. This can be tricky, but bear in mind, it'll make things 10x trickier for the next person if you get this wrong.

### sim_directive.c: sim_directive_get_node_branch_by_level()

#### Behavior
Inside the for loop which runs `up_level` times.<br>
A **segfault** occurs at the line: `ret = ret->parent;`. The segfault occurrence is inconsistent, meaning it does not occur *every* time.

#### Investigation
In certain cases (it's not obvious exactly *when*), `up_level` can become a very large number (like up to 9-digits). This causes the for loop to continue on for that long. `ret` is a pointer to a GNode struct. GNode is basically a glib implementation of an N-ary tree. After some iterations, `ret->parent` no longer exists, `ret` gets set to `NULL`. On the next iteration `NULL->parent` raises a segmentation fault because we're trying to dereference a `NULL` pointer.

#### Fix
My best guess is that the for loop should run `g_node_depth(node)` times instead of `up_level`. However, it looks like someone specifically added an `up_level` calculation. A `level` parameter also gets passed as an argument, so I'm not confident we can change that without being sure of what is meant to be done. The quick and easy fix is to check if `ret` is `NULL`, and if so, then break out of the for loop.

Note that fixes like this are akin to fixing the symptom instead of addressing the root cause. But hey, it works, for now...
