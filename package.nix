{
  lib,
  stdenv,
  darwinMinVersionHook,
  meson,
  ninja,
  pkg-config,
  python312,
  uv,
  libllvm,
  libxml2,
  pythonManylinuxPackages,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "akarin";
  version = "1.2.0";

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
      ]
    );
  };

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    python312
    uv
  ];

  buildInputs =
    [
      libllvm
      libxml2
    ]
    # `std::to_chars()` for floating-point types was introduced in macOS 13.3.
    # But then `darwinMinVersionHook "13.0"` yields "error: 'from_chars' is
    # unavailable: introduced in macOS 26.0".
    ++ lib.optional stdenv.hostPlatform.isDarwin (darwinMinVersionHook "26.0");

  env = {
    UV_PYTHON_DOWNLOADS = "never";
    UV_PYTHON_PREFERENCE = "only-system";
  } // lib.optionalAttrs stdenv.isLinux {
    LD_LIBRARY_PATH = lib.makeLibraryPath pythonManylinuxPackages.manylinux1;
  };

  meta = {
    homepage = "https://github.com/Jaded-Encoding-Thaumaturgy/akarin-vapoursynth-plugin";
    license = lib.licenses.lgpl3;
    platforms = lib.platforms.all;
  };
})
