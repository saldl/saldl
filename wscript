#! /usr/bin/env python

# python imports
from os import environ as os_env
from os import path

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
        install_path = '${MANDIR}',
        shell = False,
        reentrant = False
        )


#------------------------------------------------------------------------------

def options(opt):
    opt.load('compiler_c')

    ins_gr = opt.get_option_group('Installation and uninstallation options')
    bld_gr = opt.get_option_group('Build and installation options')
    conf_gr = opt.get_option_group('Configuration options')

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
            '--enable-link_optimizations',
            dest = 'ENABLE_LINK_OPTIMIZATIONS',
            default = def_enable_link_optimizations,
            action= "store_true",
            help = "Enable link optimization flags. (default: %s)" % def_enable_link_optimizations
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

    conf.start_msg('Get saldl version from GIT')

    try:
        saldl_version = conf.cmd_and_log(['git', 'describe', '--dirty']).rstrip()
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
def set_defines(conf):
    conf.env.append_value('DEFINES', '_FILE_OFFSET_BITS=64')
    conf.env.append_value('DEFINES', '_GNU_SOURCE')
    conf.env.append_value('DEFINES', '_XOPEN_SOURCE=501')


@conf
def check_api(conf):
    print('Checking API support:')
    check_function_mkdir(conf)
    check_timer_support(conf)
    conf.check_cc(function_name='asprintf', header_name="stdio.h", mandatory=False)
    conf.check_cc(function_name='strcasestr', header_name="string.h", mandatory=False)
    conf.check_cc(function_name='strsignal', header_name="string.h", mandatory=False)
    conf.check_cc(function_name='sigaction', header_name="signal.h", mandatory=False)
    conf.check_cc(function_name='sigaddset', header_name="signal.h", mandatory=False)

@conf
def check_flags(conf):
    # Load this before checking flags
    conf.load('compiler_c')

    check_required_flags(conf)

    if not conf.options.DISABLE_COMPILER_WARNINGS:
        check_warning_cflags(conf)

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
    conf.check_cc(cflags = '-std=c99', uselib_store='SAL_C99', mandatory=False)
    conf.check_cc(cflags = '-fPIE', uselib_store='SAL_PIE', mandatory=False)
    conf.check_cc(linkflags = '-pie', uselib_store='SAL_PIE', mandatory=False)


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
            mandatory=False)
    conf.check_cc(function_name='gettimeofday', header_name="sys/time.h", mandatory=False)

    if not ('HAVE_CLOCK_MONOTONIC_RAW=1' in conf.env['DEFINES'] or 'HAVE_GETTIMEOFDAY=1' in conf.env['DEFINES']):
        conf.fatal('Neither clock_gettime() with CLOCK_MONOTONIC_RAW nor gettimeofday() is available!')

@conf
def check_function_mkdir(conf):
    conf.check_cc(function_name='mkdir', header_name="sys/stat.h", mandatory=False)
    conf.check_cc(function_name='_mkdir', header_name="direct.h", mandatory=False)

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
                ['-pedantic'],
                ['-Wall'],
                ['-Wextra'],
                ['-Wmissing-format-attribute'],
        ]

    for w in warn_flags:
        conf.check_cc(cflags = w, uselib_store='SAL_WARNING', mandatory=False)

    # Disable stupid clang warnings
    if conf.env['CC_NAME'] == 'clang':
        clang_no_warn_flags = [
                ['-Wno-newline-eof'], # Fails CLOCK_MONOTONIC_RAW test.
                ['-Wno-missing-field-initializers'] # Fails {0} initializations.
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
        #print('Added warning flags: ' + str(conf.env['CFLAGS_SAL_WARNING']))
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
            print('Added sanitizers to cflags: ' + str(conf.env['CFLAGS_SAL_SANITIZE']))
        if conf.env['LINKFLAGS_SAL_SANITIZE']:
            conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_SAL_SANITIZE'])
            print('Added sanitizers to linkflags: ' + str(conf.env['LINKFLAGS_SAL_SANITIZE']))
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
                ['-fstack-protector-strong']
        ]

    for d in debug_flags:
        conf.check_cc(cflags = d, uselib_store='SAL_DEBUG', mandatory=False)

    if conf.env['CFLAGS_SAL_DEBUG']:
        conf.env.append_value('CFLAGS', conf.env['CFLAGS_SAL_DEBUG'])
        #print('Added debug flags: ' + str(conf.env['CFLAGS_SAL_DEBUG']))
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
                ['-Wl,--hash-style=gnu']
        ]

    for l in linkflags:
        conf.check_cc(linkflags = l, uselib_store='SAL', mandatory=False)

    if conf.env['LINKFLAGS_SAL']:
        conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_SAL'])
        #print('Added linkflags: ' + str(conf.env['LINKFLAGS_SAL']))


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
            print('Added lto flags to cflags: ' + str(conf.env['CFLAGS_SAL_LTO']))

        if conf.env['LINKFLAGS_SAL_LTO']:
            conf.env.append_value('LINKFLAGS', conf.env['LINKFLAGS_SAL_LTO'])
            print('Added lto flags to linkflags: ' + str(conf.env['LINKFLAGS_SAL_LTO']))

    else:
        if not (l_os_flags or c_os_flags):
            print('lto flags not supported by the compiler!')


@conf
def check_pkg_deps(conf):

    print('Check dependencies:')

    # Make sure LIB, INCLUDES, CFLAGS and LDFLAGS exist
    for v in ['LIB', 'INCLUDES', 'CFLAGS' 'LDFLAGS']:
        if not v in conf.env:
            conf.env[v] = []

    check_libcurl(conf)
    check_libevent_pthreads(conf)

    # Deps with no pkg-config support
    conf.env.append_value('LIB', 'pthread')

@conf
def check_libcurl(conf):
    pkg_name = 'libcurl'
    check_args = ['--cflags', '--libs']
    min_ver = '7.42'
    check_pkg(conf, pkg_name, check_args, min_ver)

@conf
def check_libevent_pthreads(conf):
    pkg_name = 'libevent_pthreads'
    check_args = ['--cflags', '--libs']
    min_ver = '2.0.20'
    check_pkg(conf, pkg_name, check_args, min_ver)


@conf
def check_pkg(conf, pkg_name, check_args, min_ver):

    conf_opts_dict = eval( str(conf.options) )

    opt_cflags_var = pkg_name.upper() + '_CFLAGS'
    opt_libs_var = pkg_name.upper() + '_LIBS'

    opt_cflags = conf_opts_dict[opt_cflags_var] != None
    opt_libs = conf_opts_dict[opt_libs_var] != None
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
        conf.check_cfg(package = pkg_name, args = check_args, atleast_version = min_ver)
        conf.check_cfg(package = pkg_name, variables = ['includedir', 'prefix'])

        includes_var = 'INCLUDES_' + pkg_name.upper()
        if conf.env[includes_var]:
            conf.env.INCLUDES += conf.env[includes_var]

        if conf.env[pkg_name.upper() + '_includedir']:
            conf.env.INCLUDES += [ conf.env[pkg_name.upper() + '_includedir'] ]

        lib_var = 'LIB_' + pkg_name.upper()
        if conf.env[lib_var]:
            conf.env.LIB += conf.env[lib_var]


#------------------------------------------------------------------------------

def configure(conf):
    get_saldl_version(conf)
    prep_man(conf)
    check_flags(conf)
    set_defines(conf)
    check_api(conf)
    check_pkg_deps(conf)

#------------------------------------------------------------------------------

def build(bld):

    bld.objects(
            source = [
                'progress.c',
                'events.c',
                'queue.c',
                'merge.c',
                'status.c',
                'resume.c',
                'ctrl.c',
                'common.c',
                'exit.c',
                'log.c',
                'utime.c',
                'utils.c',
                ],
            target = ['saldl-objs']
            )

    bld.program(
            use = ['saldl-objs'],
            source = ['main.c', 'saldl.c'],
            target = 'saldl'
            )

    if not bld.env['DISABLE_MAN']:
        bld(
                source = ['saldl.1.txt'],
                )


#------------------------------------------------------------------------------
