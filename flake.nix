{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (_system: pkgs:
        let
          libghostty-vt = pkgs.stdenv.mkDerivation {
            pname = "libghostty-vt";
            version = "1.0.1";
            src = pkgs.fetchurl {
              url = "https://release.files.ghostty.org/libghostty-vt/1.0.1/libghostty-vt-1.0.1.tar.gz";
              sha256 = "333dfce97641ef9b536bb9e08907ad86abaaada50e8f7417b70d9284adef37aa";
            };
            deps = pkgs.fetchurl {
              url = "https://release.files.ghostty.org/libghostty-vt/1.0.1/libghostty-vt-1.0.1-deps.tar.gz";
              sha256 = "91a4e10960a52bf5bf4b15aa2a2cddaff8a89d6cd903d3055bda4fd17de3356d";
            };
            nativeBuildInputs = [ pkgs.zig_0_17 ];
            dontConfigure = true;
            buildPhase = ''
              mkdir -p zig-packages zig-cache
              tar -xzf "$deps" --strip-components=1 -C zig-packages
              ZIG_GLOBAL_CACHE_DIR="$PWD/zig-cache" zig build \
                --system "$PWD/zig-packages" \
                -Doptimize=ReleaseFast \
                -Dcpu=baseline \
                --prefix "$out"
            '';
            dontInstall = true;
          };
        in
        {
          default = pkgs.clangStdenv.mkDerivation {
            pname = "kwi";
            version = "0.1.0";
            src = ./.;
            meta.mainProgram = "kwi";

            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
              gettext
              python3
              qt6.wrapQtAppsHook
              kdePackages.extra-cmake-modules
            ];

            buildInputs = [
              libghostty-vt
              pkgs.llvmPackages.openmp
            ] ++ (with pkgs.qt6; [
              qtbase
              qtdeclarative
              qtshadertools
              qtsvg
              qttools
            ]) ++ (with pkgs.kdePackages; [
              kcoreaddons
              kconfig
              kguiaddons
              ki18n
              kirigami
              kpty
              kio
              knotifications
              kwindowsystem
              kcolorscheme
              kirigami-addons
              kdbusaddons
              kirigami-app-components
              kparts
              kxmlgui
              kwidgetsaddons
              kiconthemes
              qqc2-desktop-style
              kitemmodels
            ]);
          };
        });

      devShells = forAllSystems (system: pkgs: {
        default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          inputsFrom = [ self.packages.${system}.default ];
          packages = with pkgs; [
            clang-tools
            lldb
          ];
          CMAKE_EXPORT_COMPILE_COMMANDS = "1";
        };
      });
    };
}
