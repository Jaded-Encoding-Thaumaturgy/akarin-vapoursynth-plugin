{
  lib,
  stdenv,
  meson,
  ninja,
  pkg-config,
  libllvm,
  libxml2,
  vapoursynth,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "akarin";
  version = "0.0.0";

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
  ];

  buildInputs = [
    libllvm
    libxml2
    vapoursynth
  ];

  postPatch = ''
    substituteInPlace meson.build \
      --replace-fail "vapoursynth_dep.get_pkgconfig_variable('libdir')" "get_option('libdir')"
  '';

  meta = {
    homepage = "https://github.com/Jaded-Encoding-Thaumaturgy/akarin-vapoursynth-plugin";
    license = lib.licenses.lgpl3;
    platforms = lib.platforms.all;
  };
})
