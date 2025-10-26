{
  description = "C++20 development environment with GCC 15 and SQLite";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # C++ Compiler and Tools
            gcc15

            # Build Systems
            cmake
            ninja
            pkg-config

            # SQLite
            sqlite

            # Development Tools
            gdb
            valgrind
            clang-tools # For clang-format, clang-tidy

            # Additional useful libraries
            sqlite.dev # SQLite headers
          ];

          shellHook = ''
            echo "C++20 Development Environment"
            echo "============================="
            echo "GCC Version: $(gcc --version | head -n1)"
            echo "CMake Version: $(cmake --version | head -n1)"
            echo "SQLite Version: $(sqlite3 --version)"
            echo ""
            echo "Available tools: gcc, g++, cmake, ninja, gdb, valgrind"
            echo ""

            # Set C++20 environment variables for build systems
            export CXXFLAGS="-std=c++20"
            export CMAKE_CXX_STANDARD=20
            export CMAKE_CXX_STANDARD_REQUIRED=ON

            # Create aliases for direct compiler use with C++20
            alias g++='g++ -std=c++20 -lsqlite3'
            alias gcc='gcc -std=c++20 -lsqlite3'
            alias sqlite3='sqlite3 -cmd ".mode box" -cmd ".headers on"'

            echo "Note: g++ and gcc aliases set to use -std=c++20 -lsqlite3 by default"
            echo "Note: sqlite3 aliases set to use -cmd ".mode box" -cmd ".headers on" by default"
          '';
        };

        packages.default = pkgs.gcc15Stdenv.mkDerivation {
          pname = "timing-tool";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
          ];

          buildInputs = with pkgs; [
            sqlite
          ];

          # CMake flags
          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
          ];

          installPhase = ''
            mkdir -p $out/bin
            cp timing-tool $out/bin/
          '';

          meta = {
            description = "A C++20 timing tool for tracking Kommen/Gehen timestamps";
            license = pkgs.lib.licenses.mit;
          };
        };
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/timing-tool";
        };
      }
    );
}
