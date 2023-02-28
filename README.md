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

## GNU Build System
The GNU Build System or GNU autotools is a set of tools designed to assist in making source code packages portable to many Unix-like systems. In other words, it automates your build process. It's *actual* purpose is to make your life as a developer easier, although it will pretty much do the opposite if you're not familiar with how it works. It can be overwhelming to learn, but remember, you just need to learn *enough* that you can do basic things with it (like building, and knowing where to look when you want to add new dependencies). I will summarize the basics, in the context of OSSIM:
- There are two files that serve as entry points which generate everything else. These are the only two files you need to concern yourself with when making edits:
    - configure.ac
        - Parsed by autoconf
    - Makefile.am
        - Parsed by automake
- Running `autoreconf --install` uses the above two files and generates:
    - configure
        - which generates Makefile(s) and config.h in accordance with your OS.
    - config.h.in
        - used by configure as input to generate config.h
    - Makefile.in
        - used by configure as input to generate Makefile(s)
- An `autogen.sh` script takes care of generating *and* running configure, which then leaves it up to you to run `make` and create your program executable in the relevant directory. (This script runs a bunch of commands in the right order, and with the right arguments. So use this, instead of manually running commands and trying to generate/run configure yourself).

### References
1. [Automake Documentation](https://www.gnu.org/software/automake/manual/html_node/index.html)
1. [Autoconf Documentation](https://www.gnu.org/software/autoconf/manual/autoconf-2.71/html_node/index.html)

## Adding new .h or .c files
In order to add new .h or .c files to this codebase, locate the `ossim_server_SOURCES` variable inside Makefile.am and add only your .c filename:
```
ossim_server_SOURCES = ... <some-old-file>.c \
                <your-filename>.c
```
Run `./autogen.sh` and it will generate a new Makefile which should allow compilation with your new file(s).

## Adding a new library to the build process
- Edit configure.ac:
    - Add `PKG_CHECK_MODULES(<your_library>, [<your_library's package name>])` with the rest of `PKG_CHECK_MODULES()`.
        - Example: `PKG_CHECK_MODULES(LIBMONGOC, [libmongoc-static-1.0])`
    - Add `AC_SUBST(<your_library>_CFLAGS)` and `AC_SUBST(<your_library>_LIBS)` with the rest of the `AC_SUBST` declarations.
        - Example:
            - `AC_SUBST(LIBMONGOC_CFLAGS)`
            - `AC_SUBST(LIBMONGOC_LIBS)`
- Edit Makefile.ac:
    - Add `$(<your_library>_CFLAGS)` to the variables `AM_CPPFLAGS` and `libparser_a_CFLAGS`.
        - Example: `$(LIBMONGOC_CFLAGS)`
    - Add `$(<your_library>_LIBS)` to the variable `ossim_server_LDADD`.
        - Example: `$(LIBMONGOC_LIBS)`

## Installing MongoDB
Run the following commands to install MongoDB 5.0.x. Note that these steps *do not* work for MongoDB version 6.0.x.
```
wget -qO - https://www.mongodb.org/static/pgp/server-5.0.asc | sudo apt-key add -
echo "deb http://repo.mongodb.org/apt/debian stretch/mongodb-org/5.0 main" | sudo tee /etc/apt/sources.list.d/mongodb-org-5.0.list
sudo apt-get update
sudo apt-get install -y mongodb-org
sudo service mongodb start
```
Verify the installation worked by running `mongo`.

I suspect that MongoDB 5.0.x is the latest version we can get on Debian 9, even though the official documentation says otherwise (see references). I have tried to confirm this on the [MongoDB forums](https://www.mongodb.com/community/forums/t/installing-mongodb-6-x-on-debian-9-possible/215159).

### References:
1. https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-debian/ (claims MongoDB 6.0 is available on Debian 9)
1. https://www.mongodb.com/try/download/community (does not allow downloading MongoDB 6.0 for Debian 9)
1. https://www.mongodb.com/download-center/community/releases (Debian 9 is only listed under MongoDB versions <= 5.0)

### Installing the MongoDB C driver
1. Surprisingly, installation of the C driver isn't straightforward either, I had to build it from source following the instructions linked below.
    - Relevant links:
        - https://stackoverflow.com/questions/51530526/cant-find-mongoc-h
        - https://mongoc.org/libmongoc/current/installing.html
    - Again, this is not the latest version of the driver, we may be interested in upgrading this later.
1. You already installed `libbson` when building ossim from source, so no need to redo that.

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
This can happen due to multiple reasons. For me, the mysql root user's password had mysteriously been changed (it's supposed to be blank...). So running `mysql -u root` would give:
- `ERROR 1045 (28000): Access denied for user 'root'@'localhost' (using password: NO)` (on mysql shell)

Solution:
The steps outlined in the answer by Li Yingjun, are a good starting point.
- https://stackoverflow.com/questions/10299148/mysql-error-1045-28000-access-denied-for-user-billlocalhost-using-passw

Investigate if this *really* is your issue by viewing the mysql.users table. If it is, follow the commands below:
```
service mysql stop
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

### 1) sim-directive.c: sim_directive_get_node_branch_by_level()

#### Behavior
Inside the for loop which runs `up_level` times.<br>
A **segfault** occurs at the line: `ret = ret->parent;`. The segfault occurrence is inconsistent, meaning it does not occur *every* time.

#### Investigation
In certain cases (it's not obvious exactly *when*), `up_level` can become a very large number (like up to 9-digits). This causes the for loop to continue on for that long. `ret` is a pointer to a GNode struct. GNode is basically a glib implementation of an N-ary tree. After some iterations, `ret->parent` no longer exists, `ret` gets set to `NULL`. On the next iteration `NULL->parent` raises a segmentation fault because we're trying to dereference a `NULL` pointer.

#### Fix
My best guess is that the for loop should run `g_node_depth(node)` times instead of `up_level`. However, it looks like someone specifically added an `up_level` calculation. A `level` parameter also gets passed as an argument, so I'm not confident we can change that without being sure of what is meant to be done. The quick and easy fix is to check if `ret` is `NULL`, and if so, then break out of the for loop.

Note that fixes like this are akin to fixing the symptom instead of addressing the root cause. But hey, it works, for now...

### 2) sim-parser.c: const bson_mem_vtable_t vtable

#### Behavior
Installing/updating to libmongoc-1.23.2 also updates libbson (which is already being used in OSSIM). This ends up raising some warnings in sim-parser.c, at the declaration for `vtable`. Here's the exact warning that's raised:

```
make[1]: Entering directory '/root/stip-backend/os-sim/src'
  CC       libparser_a-sim-parser.o
sim-parser.c:71:1: error: braces around scalar initializer [-Werror]
 const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free,{0,0,0,0}};
 ^~~~~
sim-parser.c:71:1: note: (near initialization for 'vtable.aligned_alloc')
sim-parser.c:71:68: error: excess elements in scalar initializer [-Werror]
 const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free,{0,0,0,0}};
                                                                    ^
sim-parser.c:71:68: note: (near initialization for 'vtable.aligned_alloc')
sim-parser.c:71:70: error: excess elements in scalar initializer [-Werror]
 const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free,{0,0,0,0}};
                                                                      ^
sim-parser.c:71:70: note: (near initialization for 'vtable.aligned_alloc')
sim-parser.c:71:72: error: excess elements in scalar initializer [-Werror]
 const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free,{0,0,0,0}};
                                                                        ^
sim-parser.c:71:72: note: (near initialization for 'vtable.aligned_alloc')
sim-parser.c:71:1: error: missing initializer for field 'padding' of 'bson_mem_vtable_t {aka const struct _bson_mem_vtable_t}' [-Werror=missing-field-initializers]
 const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free,{0,0,0,0}};
 ^~~~~
In file included from /usr/local/include/libbson-1.0/bson/bson.h:40:0,
                 from /usr/local/include/libbson-1.0/bson.h:18,
                 from sim-parser.c:33:
/usr/local/include/libbson-1.0/bson/bson-memory.h:40:10: note: 'padding' declared here
    void *padding[3];
          ^~~~~~~
cc1: all warnings being treated as errors
Makefile:682: recipe for target 'libparser_a-sim-parser.o' failed
make[1]: *** [libparser_a-sim-parser.o] Error 1
make[1]: Leaving directory '/root/stip-backend/os-sim/src'
Makefile:1542: recipe for target 'all-recursive' failed
make: *** [all-recursive] Error 1
```
Note that the build configuration for this project is using gcc's `Werror` flag, which means it's treating any warnings as errors, and will prevent your code from compiling. (You can change this in configure.ac, *only for testing purposes*, but it's much better to fix the warnings you see because the compiler usually has good reasons for generating them).

#### Investigation
The version of libbson used by OSSIM code has the following declaration for the `bson_mem_vtable_t` struct:
```
typedef struct _bson_mem_vtable_t {
   void *(*malloc) (size_t num_bytes);
   void *(*calloc) (size_t n_members, size_t num_bytes);
   void *(*realloc) (void *mem, size_t num_bytes);
   void (*free) (void *mem);
   void *padding[4];
} bson_mem_vtable_t;
```
This is called in sim-parser.c like so: `const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free, {0,0,0,0}};`

The version of libbson that gets installed when we install the latest version of libmongoc has the following declaration for `bson_mem_vtable_t`:
```
typedef struct _bson_mem_vtable_t {
   void *(*malloc) (size_t num_bytes);
   void *(*calloc) (size_t n_members, size_t num_bytes);
   void *(*realloc) (void *mem, size_t num_bytes);
   void (*free) (void *mem);
   void *(*aligned_alloc) (size_t alignment, size_t num_bytes);
   void *padding[3];
} bson_mem_vtable_t;
```
This difference in declaration: the addition of a new struct member (aligned_alloc) and padding to have space for 3 elements instead of 4, is what raises our warning.

#### Fix
The warning can be avoided by simply declaring this variable in accordance with its new declaration.
This means we change
```
const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free, {0,0,0,0}};
```
to
```
const bson_mem_vtable_t vtable = {malloc, calloc, realloc, free, NULL, {0,0,0}};
```
According to the documentation, if `aligned_alloc` is undeclared, the `malloc` struct member is used in its place. This is fine, because OSSIM does not have a custom memory aligned allocation function anyway.
