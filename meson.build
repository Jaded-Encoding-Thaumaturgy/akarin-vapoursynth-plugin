project('Plugin', 'c', 'cpp',
  default_options: ['buildtype=release', 'b_ndebug=if-release', 'c_std=c99', 'cpp_std=c++20'],
  meson_version: '>=0.48.0',
  version: '1.0.0'
)

version_h = declare_dependency(
    sources: vcs_tag(
        command: ['git', 'describe', '--tags', '--long', '--always'],
        input: 'version.h.in',
        output: 'version.h'
    )
)

is_windows = target_machine.system() == 'windows' or target_machine.system() == 'cygwin'
use_asmjit = get_option('asmjit')
static_llvm = get_option('static-llvm')

sources_expr = [
  # expr
  'expr/vslog.cpp',
  'expr/cpufeatures.cpp',
  'expr/kernel/cpulevel.cpp',
  'expr/exprfilter.cpp',
]

sources_expr2 = [
  # expr2
  'expr2/exprfilter.cpp',
  'expr2/reactor/CPUID.cpp',
  'expr2/reactor/Debug.cpp',
  'expr2/reactor/ExecutableMemory.cpp',
  'expr2/reactor/LLVMAsm.cpp',
  'expr2/reactor/LLVMJIT.cpp',
  'expr2/reactor/LLVMReactor.cpp',
  'expr2/reactor/LLVMReactorDebugInfo.cpp',
  'expr2/reactor/Reactor.cpp',
  'expr2/reactor/ReactorDebugInfo.cpp',
]

sources_ngx = [
  # DLISR
  'ngx/ngx.cc',
  'ngx/ngximpl.cc',
]

sources_vfx = [
  # DLVFX
  'vfx/vfx.cc',
  'vfx/nvvfx/src/NVVideoEffectsProxy.cpp',
  'vfx/nvvfx/src/nvCVImageProxy.cpp',
]

sources_banding = [
  # Cambi
  'banding/cambifilter.c',
  'banding/libvmaf/picture.c',
  'banding/libvmaf/cambi.c',
  'banding/libvmaf/ref.c',
  'banding/libvmaf/mem.c',
  #'banding/libvmaf/opt.c',
  #'banding/libvmaf/test.c',
  #'banding/libvmaf/test_cambi.c',
]

sources_text = [
  'text/textfilter.cpp',
  'text/tmplfilter.cpp',
]

sources_common = [
  # main plugin
  'plugin.cpp',
]

deps = []
incdir = [include_directories('text')]

if use_asmjit
  sources = sources_common + sources_expr
  incdir += include_directories('.')
  if host_machine.cpu_family().startswith('x86')
    add_project_arguments('-DVS_TARGET_CPU_X86', '-mavx', language: 'cpp')
  else
    error('the asmjit backend only support x86.')
  endif
else
  sources = sources_common + sources_expr2
  incdir += include_directories('expr2/reactor')
  deps += dependency('llvm', version: ['>= 10.0', '< 21'], method: 'config-tool', static: static_llvm,
    modules: [
      'asmprinter', 'executionengine', 'target', 'orcjit', 'native',
    ])

  if static_llvm and is_windows
    # HACK: work around multiple definition of `llvm::getTypeName<llvm::LICMPass>()::Name`.
    add_project_link_arguments('-Wl,--allow-multiple-definition', language: 'cpp')
  endif
endif

libs = []

if is_windows
  add_project_arguments('-DVS_TARGET_OS_WINDOWS', language: 'cpp')

  # DLISR and DLVFX only supports Windows.
  sources += sources_ngx + sources_vfx
  incdir += include_directories('vfx/nvvfx/include')
  add_project_arguments('-DHAVE_NGX', '-DHAVE_VFX', language: 'cpp')
endif

sources += sources_banding
sources += sources_text

vapoursynth_dep = dependency('vapoursynth').partial_dependency(compile_args: true, includes: true)

shared_module('akarin', sources,
  dependencies: deps + [ vapoursynth_dep, version_h ],
  link_with: libs,
  install: true,
  include_directories: incdir,
  install_dir: join_paths(vapoursynth_dep.get_pkgconfig_variable('libdir'), 'vapoursynth'),
  gnu_symbol_visibility: 'hidden'
)
