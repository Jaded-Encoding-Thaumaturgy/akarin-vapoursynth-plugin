{
  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        inherit (pkgs) lib;
      in {
        packages = rec {
          llvm_19 = pkgs.callPackage ./package.nix {};
          llvm_20 = pkgs.callPackage ./package.nix {libllvm = pkgs.llvmPackages_20.libllvm;};
          llvm_21 = pkgs.callPackage ./package.nix {libllvm = pkgs.llvmPackages_21.libllvm;};
          default = llvm_21;
        };

        formatter = pkgs.alejandra;

        checks = {
          default =
            pkgs.runCommandLocal "akarin-check"
            {
              buildInputs = with pkgs; [
                python3
                python3.pkgs.pytest
                (vapoursynth.withPlugins [
                  self.packages.${system}.default
                ])
              ];
            }
            ''
              python -c "import vapoursynth; print(vapoursynth.core); print(vapoursynth.core.akarin)"
              cd ${self}
              python -m pytest tests
              touch $out
            '';
        };
      }
    );
}