{
  description = "Matching decompilation of Theme Hospital (DOS)";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";

      overlay = final: prev: {
        # nixpkgs ships wibo 0.6.14 (i686-only), which cannot run Watcom
        # ("Unhandled function LocalAlloc"). 1.2.0's CMake build pulls
        # mimalloc and Wine DLLs via FetchContent, which the sandbox forbids,
        # so use the upstream static release binary instead.
        wibo = final.stdenvNoCC.mkDerivation rec {
          pname = "wibo";
          version = "1.2.0";
          src = final.fetchurl {
            url = "https://github.com/decompals/wibo/releases/download/${version}/wibo-x86_64";
            hash = "sha256-E/hqLWGPDb5nF500liU0Xqv5tGRQKVy0yQTkn2r/ha8=";
          };
          dontUnpack = true;
          installPhase = "install -Dm755 $src $out/bin/wibo";
          meta.mainProgram = "wibo";
        };

        # nixpkgs has 3.8.0; pin the CLI to the release the pipeline was validated with.
        objdiff-cli = final.stdenvNoCC.mkDerivation rec {
          pname = "objdiff-cli";
          version = "3.8.1";
          src = final.fetchurl {
            url = "https://github.com/encounter/objdiff/releases/download/v${version}/objdiff-cli-linux-x86_64";
            hash = "sha256-yCkCgeghFLzBoG/3MGERDTkCoXeCLnUDN94lNxiONY8=";
          };
          dontUnpack = true;
          installPhase = "install -Dm755 $src $out/bin/objdiff-cli";
          meta.mainProgram = "objdiff-cli";
        };

        # Open Watcom's C++ name demangler as a filter (mangled names on stdin,
        # declarations on stdout), built from the Open Watcom v2 source.
        owdemangle = final.stdenv.mkDerivation {
          pname = "owdemangle";
          version = final.open-watcom-v2-unwrapped.version;
          src = ./owdemangle;
          buildPhase = ''
            $CC -O2 -w -I. -o owdemangle \
              ${final.open-watcom-v2-unwrapped.src}/bld/lib_misc/c/demangle.c main.c \
              -I${final.open-watcom-v2-unwrapped.src}/bld/lib_misc/h
          '';
          installPhase = "install -Dm755 owdemangle $out/bin/owdemangle";
        };

        # Period-correct Watcom C/C++ 10.6 (Win32 host binaries, run through wibo),
        # as packaged for decomp.me.
        watcom-10_6 = final.stdenvNoCC.mkDerivation {
          pname = "watcom";
          version = "10.6";
          src = final.fetchurl {
            url = "https://github.com/OmniBlade/decomp.me/releases/download/wcc10.5/wcc10.6.tar.gz";
            hash = "sha256-xFXt8y3HPynCtOCqXGmIdogJZBAz71RwByUkJxGwTik=";
          };
          sourceRoot = ".";
          installPhase = "mkdir -p $out && cp -r binnt binw h $out/";
        };

        # Watcom C/C++ 10.0a, which built part of the C runtime (see
        # docs/MATCHING.md); as packaged by decomp.me.
        watcom-10_0a = final.stdenvNoCC.mkDerivation {
          pname = "watcom";
          version = "10.0a";
          src = final.fetchurl {
            url = "https://github.com/decompme/compilers/releases/download/compilers/wcc10.0a.tar.gz";
            hash = "sha256-kgqWdNGxDyJ65lFxwYchKwbX/DiyYzE5ImFcT16bJKY=";
          };
          sourceRoot = ".";
          installPhase = "mkdir -p $out && cp -r binnt binb h $out/";
        };

        # The 32-bit runtime libraries from the Watcom C/C++ 10.5 and 10.6
        # CDs (the CD images archived on archive.org), which several runtime
        # units are built from (see docs/MATCHING.md). Only LIB386 is kept from
        # each image.
        watcom-10_5-libs = final.stdenvNoCC.mkDerivation {
          pname = "watcom-libs";
          version = "10.5";
          src = final.fetchurl {
            name = "watcom-cpp-10.5.iso";
            url = "https://archive.org/download/Watcom_C_10.5/Watcom_C++_10.5.iso";
            hash = "sha256-59gnW3HwXHgraZGDZcdwcWvej1agj83GL+I9xD2Ai5s=";
          };
          nativeBuildInputs = [ final.p7zip ];
          unpackPhase = ''
            7z x $src -oiso LIB386 BINW/WSTUB.EXE >/dev/null
          '';
          # BINW/WSTUB.EXE: the DOS/4GW stub wlink put in front of HOSPITAL.EXE.
          installPhase = "mkdir -p $out/binw && cp -r iso/LIB386/. $out/ && cp iso/BINW/WSTUB.EXE $out/binw/wstub.exe";
        };

        watcom-10_6-libs = final.stdenvNoCC.mkDerivation {
          pname = "watcom-libs";
          version = "10.6";
          src = final.fetchurl {
            name = "watcom-cpp-10.6.iso";
            url = "https://archive.org/download/Watcom_C_10.6/Watcom_C++_10.6.iso";
            hash = "sha256-GN2YrpyNiD4taAv55O2hxL94N2xfBeBxtV5XpmGa7PI=";
          };
          nativeBuildInputs = [ final.p7zip ];
          unpackPhase = ''
            7z x $src -oiso LIB386 >/dev/null
          '';
          installPhase = "mkdir -p $out && cp -r iso/LIB386/. $out/";
        };
      };

      pkgs = import nixpkgs {
        inherit system;
        overlays = [ overlay ];
        config.allowUnfreePredicate =
          pkg:
          builtins.elem (nixpkgs.lib.getName pkg) [
            "open-watcom-v2"
            "open-watcom-v2-unwrapped"
          ];
      };

      buildTools = with pkgs; [
        wibo
        objdiff-cli
        open-watcom-v2 # wdis, for reading OMF objects; wlink for tools/link.py
        objconv # ELF to OMF for tools/link.py
        owdemangle
        binutils
        ninja
        (python3.withPackages (ps: [ ps.capstone ]))
      ];

      watcomEnv = {
        WATCOM_10_6 = pkgs.watcom-10_6;
        WATCOM_10_0A = pkgs.watcom-10_0a;
        WATCOM_LIBS_10_5 = pkgs.watcom-10_5-libs;
        WATCOM_LIBS_10_6 = pkgs.watcom-10_6-libs;
      };

      # `nix run`: build, rebuild HOSPITAL.EXE byte-identical from the units,
      # and play it in DOSBox with the game data in orig/.
      play = pkgs.writeShellApplication {
        name = "theme-hospital";
        runtimeInputs = buildTools ++ (with pkgs; [ dosbox-staging dosbox-x dosbox coreutils gnused diffutils ]);
        runtimeEnv = watcomEnv;
        text = builtins.readFile ./play.sh;
      };

      # `nix run path:./nix#gog-unpack -- PATH/setup_theme_hospital_v3_(28027).exe`:
      # check GOG's installer and extract the game into orig/.
      gog-unpack = pkgs.writeShellApplication {
        name = "gog-unpack";
        runtimeInputs = with pkgs; [ innoextract coreutils ];
        text = builtins.readFile ./gog-unpack.sh;
      };

      # CMake, Ninja and SDL3 for enhanced/ (Dear ImGui is fetched by CMake).
      enhancedTools = with pkgs; [ cmake ninja pkg-config sdl3 git ];

      # `nix run path:./nix#enhanced`: build enhanced/ and run it with the
      # game data in orig/.
      enhanced = pkgs.writeShellApplication {
        name = "theme-hospital-enhanced";
        runtimeInputs = enhancedTools ++ [ pkgs.stdenv.cc ];
        # Outside the dev shell the compiler wrapper adds no include paths, so
        # CMake has to find SDL3 through its package config.
        runtimeEnv.CMAKE_PREFIX_PATH = "${pkgs.sdl3.dev}";
        text = builtins.readFile ./enhanced.sh;
      };

      # Both dev shells in one, plus what the replay scripts call outside
      # them (ps, flock), for the remote gate's image.
      gateShell = pkgs.mkShell (
        {
          packages =
            buildTools
            ++ enhancedTools
            ++ (with pkgs; [
              clang-tools
              dosbox-x
              zstd
              procps
              util-linux
            ]);
        }
        // watcomEnv
      );
    in
    {
      overlays.default = overlay;

      packages.${system} = {
        inherit (pkgs) wibo objdiff-cli watcom-10_6 watcom-10_0a watcom-10_5-libs watcom-10_6-libs owdemangle;

        # enhanced/tools/remote.sh's container: a script that streams the
        # image to stdout (`| podman load`). The container reads the source
        # tree as a tar on stdin into /build/w and runs remote_gate.sh there.
        gate-image = pkgs.dockerTools.streamNixShellImage {
          drv = gateShell;
          name = "th-gate-env";
          run = "mkdir -p /build/w && cd /build/w && tar x && exec sh enhanced/tools/remote_gate.sh";
        };
      };

      apps.${system} = {
        default = {
          type = "app";
          program = "${play}/bin/theme-hospital";
        };
        gog-unpack = {
          type = "app";
          program = "${gog-unpack}/bin/gog-unpack";
        };
        enhanced = {
          type = "app";
          program = "${enhanced}/bin/theme-hospital-enhanced";
        };
      };

      devShells.${system} = {
        default = pkgs.mkShell (
          {
            packages =
              buildTools
              ++ (with pkgs; [
                (rizin.withPlugins (p: [ p.rz-ghidra ]))
                jq
                innoextract
                clang-tools # clang-format for src/ and include/ (.clang-format)
                dosbox-x # the replay check runs the original in it
                zstd # compresses its cached traces
              ]);
          }
          // watcomEnv
        );

        # `nix develop path:./nix#enhanced`
        enhanced = pkgs.mkShell {
          # clang-tools for clang-tidy (enhanced/tools/tidy.sh) and clang-format
          packages = enhancedTools ++ [ pkgs.clang-tools ];
        };
      };
    };
}
