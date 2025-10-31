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

        pTimestamps = pkgs.writeShellScriptBin "pTimestamps" ''
          ${pkgs.sqlite}/bin/sqlite3 -cmd ".mode box" -cmd ".headers on" timestamps.db "SELECT * FROM timestamps;"
        '';

        pDailyhours = pkgs.writeShellScriptBin "pDailyhours" ''
          ${pkgs.sqlite}/bin/sqlite3 -cmd ".mode box" -cmd ".headers on" timestamps.db "SELECT * FROM dailyhours;"
        '';
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # C++ Compiler and Tools
            gcc15

            # Build Systems
            cmake

            # Additional dependencies
            sqlite
            cli11 # CLI argument parsing

            # Custom scripts
            pTimestamps
            pDailyhours
          ];

          shellHook = ''
            echo "C++20 Development Environment"
            echo "============================="
            echo "GCC Version: $(gcc --version | head -n1)"
            echo "CMake Version: $(cmake --version | head -n1)"
            echo "SQLite Version: $(sqlite3 --version)"
            echo ""
            echo "Available tools: gcc, g++, cmake"
            echo ""

            # Set C++20 environment variables for build systems
            export CXXFLAGS="-std=c++20"
            export CMAKE_CXX_STANDARD=20
            export CMAKE_CXX_STANDARD_REQUIRED=ON

            echo "Use pTimestamps to print timestamps"
            echo "Use pDailyhours to print dailyhours"
          '';
        };

        packages.default = pkgs.gcc15Stdenv.mkDerivation {
          pname = "timetagger";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
          ];

          buildInputs = with pkgs; [
            sqlite
            cli11
          ];

          # CMake flags
          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
          ];

          installPhase = ''
            mkdir -p $out/bin
            cp timetagger $out/bin/
          '';

          meta = {
            description = "A C++20 timing tool for tracking Kommen/Gehen timestamps";
            license = pkgs.lib.licenses.mit;
          };
        };
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/timetagger";
        };
      }
    );
}
