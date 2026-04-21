{
  lib,
  stdenv,
  darwinMinVersionHook,
  buildPythonPackage,
  hatchling,
  meson,
  ninja,
  packaging,
  pkg-config,
  vapoursynth,
  libllvm,
  libxml2,
  vapoursynth-lib ? builtins.head vapoursynth.buildInputs,
}:
buildPythonPackage {
  pname = "vapoursynth-akarin";
  version = "1.2.0";
  pyproject = true;

  src = lib.fileset.toSource {
    root = ./.;
    fileset = lib.fileset.intersection (lib.fileset.fromSource (lib.sources.cleanSource ./.)) (
      lib.fileset.unions [
        ./banding
        ./expr
        ./expr2
        ./ngx
        ./text
        ./vfx
        ./meson_options.txt
        ./meson.build
        ./plugin.cpp
        ./plugin.h
        ./version.h.in
        ./hatch_build.py
        ./pyproject.toml
        ./README.md
      ]
    );
  };

  postPatch = ''
        substituteInPlace pyproject.toml \
          --replace-fail "meson==1.11.0" "meson" \
          --replace-fail "ninja==1.13.0" "ninja" \
          --replace-fail "vapoursynth>=74" "vapoursynth"

        substituteInPlace meson.build \
          --replace-fail \
            "py = import('python').find_installation(pure: false)

    r = run_command(
      py,
      '-c',
      'import vapoursynth as vs; print(vs.get_include())',
      check: true,
    )
    inc_vs = include_directories(r.stdout().strip())
    incdir += inc_vs" \
            "deps += dependency('vapoursynth')"
  '';

  nativeBuildInputs = [
    libllvm.dev
    libxml2.dev
    pkg-config
  ];

  build-system = [
    hatchling
    meson
    ninja
    packaging
    vapoursynth
  ];

  buildInputs =
    [
      libllvm
      libxml2.out
      vapoursynth-lib
    ]
    # `std::to_chars()` for floating-point types was introduced in macOS 13.3.
    # But then `darwinMinVersionHook "13.0"` yields "error: 'from_chars' is
    # unavailable: introduced in macOS 26.0".
    ++ lib.optional stdenv.hostPlatform.isDarwin (darwinMinVersionHook "26.0");

  dependencies = [
    vapoursynth
  ];

  meta = {
    homepage = "https://github.com/Jaded-Encoding-Thaumaturgy/akarin-vapoursynth-plugin";
    license = lib.licenses.lgpl3;
    platforms = lib.platforms.all;
  };
}
