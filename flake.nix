{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-26.05-darwin";
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
      in {
        packages = rec {
          llvm_20 = pkgs.python3Packages.callPackage ./package.nix {libllvm = pkgs.llvmPackages_20.libllvm;};
          llvm_21 = pkgs.python3Packages.callPackage ./package.nix {libllvm = pkgs.llvmPackages_21.libllvm;};
          llvm_22 = pkgs.python3Packages.callPackage ./package.nix {libllvm = pkgs.llvmPackages_22.libllvm;};
          llvm_23 = pkgs.python3Packages.callPackage ./package.nix {
            # Skip LLVM tests on macOS to avoid <https://github.com/NixOS/nixpkgs/pull/552246>
            # and cut down the build time.
            libllvm =
              if pkgs.stdenv.hostPlatform.isDarwin
              then pkgs.llvmPackages_23.libllvm.overrideAttrs {doCheck = false;}
              else pkgs.llvmPackages_23.libllvm;
          };
          default = llvm_23;
        };

        devShells.default = pkgs.mkShellNoCC {
          packages = [pkgs.uv];
        };

        formatter = pkgs.alejandra;
      }
    );
}
