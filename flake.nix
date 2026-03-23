{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
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
          llvm_20 = pkgs.callPackage ./package.nix {libllvm = pkgs.llvmPackages_20.libllvm;};
          llvm_21 = pkgs.callPackage ./package.nix {libllvm = pkgs.llvmPackages_21.libllvm;};
          llvm_22 = pkgs.callPackage ./package.nix {libllvm = pkgs.llvmPackages_22.libllvm;};
          default = llvm_22;
        };

        formatter = pkgs.alejandra;
      }
    );
}
