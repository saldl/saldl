#! /usr/bin/env python

# python imports
from os import environ as os_env
from os import path
import hashlib

# waf imports
from waflib.Configure import conf
from waflib import TaskGen
import waflib.Context as ctx

#------------------------------------------------------------------------------

TaskGen.declare_chain(
        name   = 'a2x-man-1',
        rule   = '${A2X} -f manpage -D. ${SRC}',
        ext_in = '.1.txt',
        ext_out = '.1',
        install_path = '${MANDIR}/man1',
        shell = False,
        reentrant = False
        )


#------------------------------------------------------------------------------

def options(opt):
    opt.load('compiler_c')

    ins_gr = opt.get_option_group('Installation and uninstallation options')
    bld_gr = opt.get_option_group('Build and installation options')
    conf_gr = opt.get_option_group('Configuration options')

    def_chunk_size_mib = 1
    def_chunk_size = def_chunk_size_mib * 1024 * 1024 # 1 MiB
    conf_gr.add_option(
            '--default-chunk-size',
            dest = 'SALDL_DEF_CHUNK_SIZE',
            default = str(def_chunk_size),
            action = 'store',
            help = "Set default chunk size in bytes. (default: %s (%s MiB))." % (def_chunk_size, def_chunk_size_mib)
            )

    def_num_connections = 6
    conf_gr.add_option(
            '--default-num-connections',
            dest = 'SALDL_DEF_NUM_CONNECTIONS',
            default = str(def_num_connections),
            action = 'store',
            help = "Set default number of connections. (default: %s)." % def_num_connections
            )

    def_saldl_version = None # Use git describe
    conf_gr.add_option(
            '--saldl-version',
            dest = 'SALDL_VERSION',
            default = def_saldl_version,
            action= "store",
            help = "Set version manually, passing nothing or empty string falls back to git describe. (default: %s)." % def_saldl_version
            )

    def_a2x = 'a2x'
    conf_gr.add_option(
            '--a2x',
            dest = 'A2X',
            default = def_a2x,
            help = 'a2x executable. (default: %s)' % def_a2x
            )


    def_mandir = 'share/man'
    conf_gr.add_option(
            '--mandir',
            dest = 'MANDIR',
            default = def_mandir,
            help = 'Relative to PREFIX if not absoloute. (default: %s)' % def_mandir
            )

    def_disable_man = False
    conf_gr.add_option(
            '--disable-man',
            dest = 'DISABLE_MAN',
            default = def_disable_man,
            action= "store_true",
            help = "Don't build manpage. (default: %s)" % def_disable_man
            )

    def_inline_ca_bundle = False
    conf_gr.add_option(
            '--inline-ca-bundle',
            dest = 'INLINE_CA_BUNDLE',
            default = def_inline_ca_bundle,
            action= "store_true",
            help = "Inline CA bundle in executable binary. (default: %s)" % def_inline_ca_bundle
            )

    def_disable_compiler_warnings = False
    conf_gr.add_option(
            '--disable-compiler-warnings',
            dest = 'DISABLE_COMPILER_WARNINGS',
            default = def_disable_compiler_warnings,
            action= "store_true",
            help = "Don't enable compiler warnings. (default: %s)" % def_disable_compiler_warnings
            )

    def_no_werror = False
    conf_gr.add_option(
            '--no-werror',
            dest = 'NO_WERROR',
            default = def_no_werror,
            action= "store_true",
            help = "Don't consider warnings fatal. (default: %s)" % def_no_werror
            )

    def_enable_lto = False
    conf_gr.add_option(
            '--enable-lto',
            dest = 'ENABLE_LTO',
            default = def_enable_lto,
            action= "store_true",
            help = "Set lto flags. Conflicts with '--disable-debug'. (default: %s)" % def_enable_lto
            )

    def_enable_debug = False
    conf_gr.add_option(
            '--enable-debug',
            dest = 'ENABLE_DEBUG',
            default = def_enable_debug,
            action= "store_true",
            help = "Set debug flags. (default: %s)" % def_enable_debug
            )

    def_enable_profiler = False
    conf_gr.add_option(
            '--enable-profiler',
            dest = 'ENABLE_PROFILER',
            default = def_enable_profiler,
            action= "store_true",
            help = "Enable profiler from gperftools. (default: %s)" % def_enable_profiler
            )

    def_enable_sanitizers= False
    conf_gr.add_option(
            '--enable-sanitizers',
            dest = 'ENABLE_SANITIZERS',
            default = def_enable_sanitizers,
            action= "store_true",
            help = "Enable sanitizers. For developers only. Requires '--enable-debug' (default: %s)" % def_enable_sanitizers
            )

    def_enable_link_optimizations = False
    conf_gr.add_option(
            '--enable-link-optimizations',
            dest = 'ENABLE_LINK_OPTIMIZATIONS',
            default = def_enable_link_optimizations,
            action= "store_true",
            help = "Enable link optimization flags. (default: %s)" % def_enable_link_optimizations
            )

    def_enable_static = False
    conf_gr.add_option(
            '--enable-static',
            dest = 'ENABLE_STATIC',
            default = def_enable_static,
            action= "store_true",
            help = "Enable static build. (default: %s)" % def_enable_static
            )

    def_libcurl_cflags = None # Use pkg-config or env
    conf_gr.add_option(
            '--libcurl-cflags',
            dest = 'LIBCURL_CFLAGS',
            default = def_libcurl_cflags,
            action= "store",
            help = "Skip pkg-config and set libcurl cflags explicitly (default: %s)" % def_libcurl_cflags
            )

    def_libcurl_libs = None # Use pkg-config or env
    conf_gr.add_option(
            '--libcurl-libs',
            dest = 'LIBCURL_LIBS',
            default = def_libcurl_libs,
            action= "store",
            help = "Skip pkg-config and set libcurl libs explicitly (default: %s)" % def_libcurl_libs
            )

    def_libevent_pthreads_cflags = None # Use pkg-config or env
    conf_gr.add_option(
            '--libevent-pthreads-cflags',
            dest = 'LIBEVENT_PTHREADS_CFLAGS',
            default = def_libevent_pthreads_cflags,
            action= "store",
            help = "Skip pkg-config and set libevent_pthreads cflags explicitly (default: %s)" % def_libevent_pthreads_cflags
            )

    def_libevent_pthreads_libs = None # Use pkg-config or env
    conf_gr.add_option(
            '--libevent-pthreads-libs',
            dest = 'LIBEVENT_PTHREADS_LIBS',
            default = def_libevent_pthreads_libs,
            action= "store",
            help = "Skip pkg-config and set libevent_pthreads libs explicitly (default: %s)" % def_libevent_pthreads_libs
            )

#------------------------------------------------------------------------------

@conf
def get_saldl_version(conf):

    if conf.options.SALDL_VERSION and len(conf.options.SALDL_VERSION):
        conf.start_msg('Get saldl version from configure option:')
        saldl_version = conf.options.SALDL_VERSION
        conf.end_msg(saldl_version)
        conf.env.append_value('DEFINES', 'SALDL_VERSION="%s"' % saldl_version)
    else:
        conf.start_msg('Get saldl version from GIT')

        try:
            git_ver_cmd = ['git', 'describe', '--tags', '--long', '--dirty']
            saldl_version = conf.cmd_and_log(git_ver_cmd).rstrip().replace('-', '.')
            conf.end_msg(saldl_version)
            conf.env.append_value('DEFINES', 'SALDL_VERSION="%s"' % saldl_version)
        except:
            conf.end_msg('(failed)')


@conf
def prep_man(conf):

    # We set this in the environment because it will be used in build()
    conf.env['DISABLE_MAN'] = conf.options.DISABLE_MAN

    if not conf.env['DISABLE_MAN']:
        conf.find_program(conf.options.A2X, var='A2X')

        conf.start_msg('Setting MANDIR')

        if conf.options.MANDIR[0] == path.sep:
            conf.env['MANDIR'] = conf.options.MANDIR
        else:
            conf.env['MANDIR'] = conf.env['PREFIX'] + path.sep + conf.options.MANDIR

        conf.end_msg(conf.env['MANDIR'])

@conf
def prep_inline_ca_bundle(conf):
    if conf.options.INLINE_CA_BUNDLE:
        ca_bundle_url = 'https://curl.se/ca/cacert.pem'
        ca_bundle_hash_url = 'https://curl.se/ca/cacert.pem.sha256'

        ca_bundle_dl_cmd = ['curl', '-Lf', ca_bundle_url]
        ca_bundle_hash_dl_cmd = ['curl', '-Lf', ca_bundle_hash_url]

        conf.start_msg('Download CA bundle from %s' % ca_bundle_url)
        try:
            ca_bundle_str = conf.cmd_and_log(ca_bundle_dl_cmd)
            conf.end_msg("%s bytes" % str(len(ca_bundle_str)))
        except:
            conf.end_msg("failed", color="RED")
            conf.fatal("Failed to download CA bundle")

        conf.start_msg('Download CA bundle_hash SHA256 hash from %s' % ca_bundle_hash_url)
        try:
            ca_bundle_hash_str = conf.cmd_and_log(ca_bundle_hash_dl_cmd)
            conf.end_msg("%s bytes" % str(len(ca_bundle_hash_str)))
        except:
            conf.end_msg("failed", color="RED")
            conf.fatal("Failed to download CA bundle SHA256 hash")

        if len(ca_bundle_str) < 128*1024 or len(ca_bundle_hash_str) < 64:
            conf.msg("Checking size of downloaded CA bundle data", "failed", color="RED")
            conf.fatal("Failed size check for downloaded CA bundle data")
        else:
            conf.msg("Checking size of downloaded CA bundle data", "pass")

        sha256_hex_str = hashlib.sha256(ca_bundle_str.encode()).hexdigest()

        if ca_bundle_hash_str[0:64] == sha256_hex_str:
            conf.msg("Check downloaded vs. expected CA bundle SHA256 hash", sha256_hex_str)
        else:
            conf.msg("Check downloaded CA bundle SHA256 hash", "failed", color="RED")
            conf.fatal("Failed SHA256 hash check for downloaded CA bundle data: %s" % ca_bundle_hash_str[0:64] + " != " + sha256_hex_str)

        conf.start_msg("Writing 'ca_bundle.h' file")
        ca_bundle_str = ''.join(filter(lambda c: c.isascii(), ca_bundle_str)).replace('\n', '\\n\\\n')
        conf.env.WAF_CONFIG_H_PRELUDE = 'const char* CA_BUNDLE = "%s";' % ca_bundle_str
        conf.write_config_header('saldl_inline_ca_bundle.h')
        del conf.env.WAF_CONFIG_H_PRELUDE
        conf.end_msg('done')

        conf.env.append_value('DEFINES', 'INLINE_CA_BUNDLE')


@conf
def set_defines(conf):
    conf.env.append_value('DEFINES', '_FILE_OFFSET_BITS=64')
    conf.env.append_value('DEFINES', '_GNU_SOURCE')

    if conf.options.SALDL_DEF_NUM_CONNECTIONS:
        conf.env.append_value('DEFINES', 'SALDL_DEF_NUM_CONNECTIONS=' + conf.options.SALDL_DEF_NUM_CONNECTIONS)

    if conf.options.SALDL_DEF_CHUNK_SIZE:
        conf.env.append_value('DEFINES', 'SALDL_DEF_CHUNK_SIZE=' + conf.options.SALDL_DEF_CHUNK_SIZE)

@conf
def check_func(conf, f_name, h_name, mandatory):
    fragment = ''
    if len(h_name) > 0:
        fragment += '#include <' + h_name + '>\n'
    fragment +='int main() { void(*p)(void) = (void(*)(void)) ' + f_name + '; return !p; }\n'

    msg = 'Checking for ' + f_name + '()'
    define_name = 'HAVE_' + f_name.upper()

    conf.check_cc(fragment=fragment, define_name=define_name,  mandatory=mandatory, msg=msg)

@conf
def check_api(conf):
    print('Checking API support:')
    check_function_mkdir(conf)
    check_timer_support(conf)
    check_function_tty_width(conf)
    check_func(conf, 'GetModuleFileName', 'windows.h', False)
    check_func(conf, 'strftime', 'time.h', False)
    check_func(conf, 'asprintf', 'stdio.h', False)
    check_func(conf, 'strcasestr', 'string.h', False)
    check_func(conf, 'strsignal', 'string.h', False)
    check_func(conf, 'sigaction', 'signal.h', False)
    check_func(conf, 'sigaddset', 'signal.h', False)
    check_func(conf, 'mmap', 'sys/mman.h', False)

@conf
def check_flags(conf):
    # Load this before checking flags
    conf.load('compiler_c')

    # Set warning flags 1st so -Werror catches all warnings
    if not conf.options.DISABLE_COMPILER_WARNINGS:
        check_warning_cflags(conf)

    check_required_flags(conf)

    if conf.options.ENABLE_DEBUG and conf.options.ENABLE_LTO:
        conf.fatal('Both --enable-debug and --enable-lto were passed.')

    if conf.options.ENABLE_DEBUG:
        check_debug_cflags(conf)

    if conf.options.ENABLE_SANITIZERS:
        if not conf.options.ENABLE_DEBUG:
            conf.fatal('--enable-sanitizers require --enable-debug .')
        else:
            check_sanitize_flags(conf)

    if conf.options.ENABLE_LINK_OPTIMIZATIONS:
        check_link_flags(conf)

    if conf.options.ENABLE_LTO:
        check_lto_flags(conf)


@conf
def check_required_flags(conf):
    # -std=c99 disables many POSIX functions in MSYS2 like fileno()
    conf.check_cc(cflags = '-std=gnu99', uselib_store='SAL_REQUIRED', mandatory=False)

    # Avoid -Werror=format with MinGW builds
    if conf.env['DEST_OS'] != 'win32':
        conf.check_cc(cflags = '-pedantic', uselib_store='SAL_REQUIRED', mandatory=False)

    conf.check_cc(cflags = '-fPIE', uselib_store='SAL_REQUIRED', mandatory=False)
    conf.check_cc(cflags = '-fcommon', uselib_store='SAL_REQUIRED', mandatory=False)
    conf.check_cc(linkflags = '-pie', uselib_store='SAL_REQUIRED', mandatory=False)

    if conf.env['CFLAGS_SAL_REQUIRED']:
        conf.env.append_value('CFLAGS', conf.env['CFLAGS_SAL_REQUIRED'])


@conf
def check_timer_support(conf):
    conf.check_cc(fragment=
            '''
            #include <time.h>
            int main() {
              struct timespec tp;
              clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
              return 0;
            }
            ''',
            define_name="HAVE_CLOCK_MONOTONIC_RAW",
            msg = "Checking for clock_gettime() with CLOCK_MONOTONIC_RAW support",
            execute=True,
            mandatory=False)
    check_func(conf, 'gettimeofday', 'sys/time.h', False)

    if not ('HAVE_CLOCK_MONOTONIC_RAW=1' in conf.env['DEFINES'] or 'HAVE_GETTIMEOFDAY=1' in conf.env['DEFINES']):
        conf.fatal('Neither clock_gettime() with CLOCK_MONOTONIC_RAW nor gettimeofday() is available!')


@conf
def check_function_tty_width(conf):
    check_func(conf, '_fileno', 'stdio.h', False)
    check_func(conf, 'fileno', 'stdio.h', False)

    if not ('HAVE__FILENO=1' in conf.env['DEFINES'] or 'HAVE_FILENO=1' in conf.env['DEFINES']):
        conf.fatal('Neither _fileno() nor fileno() available!')

    check_func(conf, '_isatty', 'io.h', False)
    check_func(conf, 'isatty', 'unistd.h', False)

    if not ('HAVE__ISATTY=1' in conf.env['DEFINES'] or 'HAVE_ISATTY=1' in conf.env['DEFINES']):
        conf.fatal('Neither _isatty() nor isatty() available!')

    check_func(conf, 'ioctl', 'sys/ioctl.h', False)
    check_func(conf, 'GetConsoleScreenBufferInfo', 'windows.h', False)

    if not ('HAVE_IOCTL=1' in conf.env['DEFINES'] or 'HAVE_GETCONSOLESCREENBUFFERINFO=1' in conf.env['DEFINES']):
        conf.fatal('Neither ioctl() nor GetConsoleScreenBufferInfo() available!')

@conf
def check_function_mkdir(conf):
    check_func(conf, '_mkdir', 'direct.h', False)
    check_func(conf, 'mkdir', 'sys/stat.h', False)

    if not ('HAVE_MKDIR=1' in conf.env['DEFINES'] or 'HAVE__MKDIR=1' in conf.env['DEFINES']):
        conf.fatal('Neither mkdir() nor _mkdir() is available!')

@conf
def check_warning_cflags(conf):

    print('Checking for warning CFLAGS support:')
    os_flags = 0

    if 'CFLAGS_SAL_WARNING' in os_env:
        warn_flags = os_env['CFLAGS_SAL_WARNING'].split(' ')
        os_flags = 1
    else:
        warn_flags = [
                ['-Wall'],
                ['-Wextra'],
                ['-Wmissing-format-attribute'],
                # In GCC 7, -Wextra enables -Wimplicit-fallthrough
                ['-Wno-implicit-fallthrough'],
                # For CA bundle
                ['-Wno-overlength-strings'],
        ]

    for w in warn_flags:
        conf.check_cc(cflags = w, uselib_store='SAL_WARNING', mandatory=False)

    # Disable stupid clang warnings
    if conf.env['CC_NAME'] == 'clang':
        clang_no_warn_flags = [
                ['-Wno-newline-eof'] # Fails CLOCK_MONOTONIC_RAW test.
        ]

        for no_w in clang_no_warn_flags:
            conf.check_cc(cflags = no_w, uselib_store='SAL_WARNING', mandatory=False)

    # Make warnings fatal unless explicitly told not to
    if not conf.options.NO_WERROR:
        conf.check_cc(cflags = ['-Werror'], uselib_store='SAL_WARNING', mandatory=False)
    else:
        conf.check_cc(cflags = ['-Wno-error'], uselib_store='SAL_WARNING', mandatory=False)



    if conf.env['CFLAGS_SAL_WARNING']:
        conf.env.append_value('CFLAGS', conf.env['CFLAGS_SAL_WARNING'])
    else:
        if not os_flags:
            conf.fatal('None of the warning CFLAGS are supported by the compiler!')

@conf
def check_sanitize_flags(conf):

    print('Checking for sanitize CFLAGS/LINKFLAGS support:')
    os_flags = 0

    if 'CFLAGS_SAL_SANITIZE' in os_env:
        sanitizers = os_env['CFLAGS_SAL_SANITIZE'].split(' ')
        os_flags = 1
    else:
        sanitizers = [
                #['-fsanitize=leak'], # Can't use this if either address or thread is enabled.
                #['-fsanitize=thread'], Does not work with new kernels.
                ['-fsanitize=undefined'],
                ['-fsanitize=address'],
                ['-fsanitize-recover=all']
        ]

    for s in sanitizers:
        conf.check_cc(cflags = s, uselib_store='SAL_SANITIZE', mandatory=False)
        conf.check_cc(linkflags = s, uselib_store='SAL_SANITIZE', mandatory=False)

    if conf.env['CFLAGS_SAL_SANITIZE'] or conf.env['LINKFLAGS_SAL_SANITIZE']:
        if conf.env['CFLAGS_SAL_SANITIZE']:
            conf.env.append_value('CFLAGS', conf.env['CFLAGS_SAL_SANITIZE'])
        if conf.env['LINKFLAGS_SAL_SANITIZE']:
            conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_SAL_SANITIZE'])
    else:
            if not os_flags:
                print('None of the sanitizers are supported by the compiler!')

@conf
def check_debug_cflags(conf):

    print('Checking for debug CFLAGS support:')
    os_flags = 0

    if 'CFLAGS_SAL_DEBUG' in os_env:
        debug_flags = os_env['CFLAGS_SAL_DEBUG'].split(' ')
        os_flags = 1
    else:
        debug_flags = [
                ['-Og'],
                ['-ggdb'],
                ['-fvar-tracking-assignments'],
                ['-fno-omit-frame-pointer'],
                ['-fstack-protector-strong'],
                ['-fstack-check']
        ]

    for d in debug_flags:
        conf.check_cc(cflags = d, uselib_store='SAL_DEBUG', mandatory=False)

    if conf.env['CFLAGS_SAL_DEBUG']:
        conf.env.append_value('CFLAGS', conf.env['CFLAGS_SAL_DEBUG'])
    else:
        if not os_flags:
            conf.fatal('None of the debug CFLAGS are supported by the compiler!')

@conf
def check_link_flags(conf):

    print('Checking for optimized LINKFLAGS support:')
    os_flags = 0

    if 'LINKFLAGS_SAL' in os_env:
        linkflags = os_env['LINKFLAGS_SAL'].split(' ')
        os_flags = 1
    else:
        linkflags = [
                ['-Wl,-O1'],
                ['-Wl,--sort-common'],
                ['-Wl,--as-needed'],
                ['-Wl,-z,relro'],
                ['-Wl,-z,now'],
        ]

    for l in linkflags:
        conf.check_cc(linkflags = l, uselib_store='SAL', mandatory=False)

    if conf.env['LINKFLAGS_SAL']:
        conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_SAL'])


@conf
def check_lto_flags(conf):

    print('Checking for LTO CFLAGS/LINKFLAGS support:')
    c_os_flags = 0
    l_os_flags = 0

    if 'CFLAGS_SAL_LTO' in os_env:
        lto_cflags = os_env['CFLAGS_SAL_LTO'].split(' ')
        c_os_flags = 1
    else:
        lto_cflags = [
                ['-flto'],
        ]

    if 'LINKFLAGS_SAL_LTO' in os_env:
        lto_linkflags = os_env['LINKFLAGS_SAL_LTO'].split(' ')
        l_os_flags = 1
    else:
        lto_linkflags = [
                ['-flto']
        ]
    for c in lto_cflags:
        conf.check_cc(cflags = c, uselib_store='SAL_LTO', mandatory=False)

    for l in lto_linkflags:
        conf.check_cc(linkflags = l, uselib_store='SAL_LTO', mandatory=False)

    if conf.env['CFLAGS_SAL_LTO'] or conf.env['LINKFLAGS_SAL_LTO']:
        if conf.env['CFLAGS_SAL_LTO']:
            conf.env.append_value('CFLAGS', conf.env['CFLAGS_SAL_LTO'])

        if conf.env['LINKFLAGS_SAL_LTO']:
            conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_SAL_LTO'])

    else:
        if not (l_os_flags or c_os_flags):
            print('lto flags not supported by the compiler!')


@conf
def check_pkg_deps(conf):

    print('Check dependencies:')

    # Make sure INCLUDES, RPATH, CFLAGS and LDFLAGS exist
    for v in ['INCLUDES', 'RPATH', 'CFLAGS', 'LDFLAGS']:
        if not v in conf.env:
            conf.env[v] = []

    # Either LIB or STLIB should exist

    if conf.options.ENABLE_STATIC:
        conf.env['STLIBPATH'] = []
        conf.env['STLIB'] = []
        # Prevent waf from adding -Wl,-Bdynamic
        conf.env['SHLIB_MARKER'] = None
    else:
        conf.env['LIBPATH'] = []
        conf.env['LIB'] = []

    # This order is important if we are providing flags ourselves
    check_libevent_pthreads(conf)
    check_libcurl(conf)

    if conf.options.ENABLE_PROFILER:
        check_libprofiler(conf)
        # libprofiler won't be linked if --as-needed is enabled
        conf.env.append_value('LDFLAGS', '-Wl,--no-as-needed')
        conf.env.append_value('LINKFLAGS', '-Wl,--no-as-needed')

    # Add libpthread
    if conf.options.ENABLE_STATIC:
        conf.env.append_value('STLIB', 'pthread')
    else:
        conf.env.append_value('LIB', 'pthread')

@conf
def check_libcurl(conf):
    pkg_name = 'libcurl'
    check_args = ['--cflags', '--libs']
    min_ver = '7.55'
    check_pkg(conf, pkg_name, check_args, min_ver)

@conf
def check_libevent_pthreads(conf):
    pkg_name = 'libevent_pthreads'
    check_args = ['--cflags', '--libs']
    min_ver = '2.0.20'
    check_pkg(conf, pkg_name, check_args, min_ver)

@conf
def check_libprofiler(conf):
    pkg_name = 'libprofiler'
    check_args = ['--cflags', '--libs']
    min_ver = '2.4'
    check_pkg(conf, pkg_name, check_args, min_ver)

@conf
def check_pkg(conf, pkg_name, check_args, min_ver):

    conf_opts_dict = eval( str(conf.options) )

    opt_cflags_var = pkg_name.upper() + '_CFLAGS'
    opt_libs_var = pkg_name.upper() + '_LIBS'

    opt_cflags = (opt_cflags_var in conf_opts_dict) and  conf_opts_dict[opt_cflags_var] != None
    opt_libs = (opt_libs_var in conf_opts_dict) and conf_opts_dict[opt_libs_var] != None
    opt_total = opt_cflags + opt_libs

    conf.start_msg('Checking %s:' % pkg_name)

    if opt_total == 1:
        conf.fatal('Either set both %s and %s or let pkg-config do the checking.' % (opt_cflags_var, opt_libs_var))

    elif opt_total == 2:
        conf.end_msg('user-provided')
        conf.env['CFLAGS'] += conf_opts_dict[opt_cflags_var].split(' ')
        conf.env['LDFLAGS'] += conf_opts_dict[opt_libs_var].split(' ')

    else:
        conf.end_msg('pkg-config')
        if conf.options.ENABLE_STATIC:
            check_args += [ '--static' ]
        conf.check_cfg(package = pkg_name, args = check_args, atleast_version = min_ver)
        conf.check_cfg(package = pkg_name, variables = ['includedir', 'prefix'])

        defines_var = 'DEFINES_' + pkg_name.upper()
        if conf.env[defines_var]:
            conf.env.DEFINES += conf.env[defines_var]

        includes_var = 'INCLUDES_' + pkg_name.upper()
        if conf.env[includes_var]:
            conf.env.INCLUDES += conf.env[includes_var]

        if conf.env[pkg_name.upper() + '_includedir']:
            conf.env.INCLUDES += [ conf.env[pkg_name.upper() + '_includedir'] ]

        # NetBSD relies on RPATH instead of ldconfig
        # This would only be set if -Wl,-R<dir> (or -Wl,-rpath<DIR>) is a part of Libs
        rpath_var = 'RPATH_' + pkg_name.upper()
        if conf.env[rpath_var]:
            conf.env.RPATH += conf.env[rpath_var]

        libpath_var = 'LIBPATH_' + pkg_name.upper()
        if conf.env[libpath_var]:
            conf.env.LIBPATH += conf.env[libpath_var]

        cflags_var = 'CFLAGS_' + pkg_name.upper()
        if conf.env[cflags_var]:
            conf.env.CFLAGS += conf.env[cflags_var]

        lib_var = 'LIB_' + pkg_name.upper()
        if conf.env[lib_var]:
            conf.env.LIB += conf.env[lib_var]

        if conf.options.ENABLE_STATIC:
            stlibpath_var = 'STLIBPATH_' + pkg_name.upper()
            if conf.env[stlibpath_var]:
                conf.env.STLIBPATH += conf.env[stlibpath_var]

            stlib_var = 'STLIB_' + pkg_name.upper()
            if conf.env[stlib_var]:
                conf.env.STLIB += conf.env[stlib_var]



#------------------------------------------------------------------------------

def configure(conf):
    get_saldl_version(conf)
    prep_man(conf)
    prep_inline_ca_bundle(conf)
    check_flags(conf)
    set_defines(conf)
    check_api(conf)
    check_pkg_deps(conf)

#------------------------------------------------------------------------------

def build(bld):
    bld.objects(
            source = [
                'src/progress.c',
                'src/events.c',
                'src/write_modes.c',
                'src/queue.c',
                'src/merge.c',
                'src/status.c',
                'src/resume.c',
                'src/ctrl.c',
                'src/common.c',
                'src/exit.c',
                'src/log.c',
                'src/utime.c',
                'src/transfer.c',
                'src/saldl.c',
                ],
            includes = bld.out_dir, # for CA bundle
            target = ['saldl-objs']
            )

    bld.program(
            use = ['saldl-objs'],
            source = ['src/main.c'],
            includes = bld.out_dir, # for CA bundle
            target = 'saldl'
            )

    if not bld.env['DISABLE_MAN']:
        bld(
                source = ['saldl.1.txt'],
                )


#------------------------------------------------------------------------------
