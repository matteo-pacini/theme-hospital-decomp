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
          src = ./tools/owdemangle;
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
    in
    {
      overlays.default = overlay;

      packages.${system} = {
        inherit (pkgs) wibo objdiff-cli watcom-10_6 owdemangle;
      };

      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          wibo
          objdiff-cli
          open-watcom-v2 # wdis, for reading OMF objects
          owdemangle
          binutils
          (rizin.withPlugins (p: [ p.rz-ghidra ]))
          ninja
          jq
          innoextract
          (python3.withPackages (ps: [ ps.capstone ]))
        ];
        WATCOM_10_6 = pkgs.watcom-10_6;
      };
    };
}
