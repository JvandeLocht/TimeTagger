{
  description = "C++20 development environment with GCC 15 and SQLite";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    tabulate = {
      url = "github:p-ranav/tabulate/v1.5";
      flake = false;
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      tabulate,
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
        tabulate-pkg = pkgs.stdenv.mkDerivation {
          pname = "tabulate";
          version = "1.5";
          src = tabulate;

          nativeBuildInputs = [ pkgs.cmake ];

          # tabulate uses CMake, so this should work out of the box
          cmakeFlags = [
            "-DBUILD_TESTS=OFF"
            "-DSAMPLES=OFF"
          ];

          # If tabulate doesn't have proper CMake install, we can create it manually
          postInstall = ''
            # Create CMake config if it doesn't exist
            if [ ! -f "$out/lib/cmake/tabulate/tabulateConfig.cmake" ]; then
              mkdir -p $out/lib/cmake/tabulate
              cat > $out/lib/cmake/tabulate/tabulateConfig.cmake << EOF
            # tabulateConfig.cmake
            get_filename_component(TABULATE_CMAKE_DIR "\''${CMAKE_CURRENT_LIST_FILE}" PATH)
            get_filename_component(TABULATE_INCLUDE_DIR "\''${TABULATE_CMAKE_DIR}/../../../include" ABSOLUTE)

            if(NOT TARGET tabulate::tabulate)
              add_library(tabulate::tabulate INTERFACE IMPORTED)
              set_target_properties(tabulate::tabulate PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "\''${TABULATE_INCLUDE_DIR}"
              )
            endif()

            set(tabulate_FOUND TRUE)
            EOF
            fi
          '';

          meta = {
            description = "Table Maker for Modern C++";
            homepage = "https://github.com/p-ranav/tabulate";
            license = pkgs.lib.licenses.mit;
          };
        };
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
            sqlitecpp # c++ wrapper for sqlite
            cli11 # CLI argument parsing
            tomlplusplus

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

            export CMAKE_PREFIX_PATH="${tabulate-pkg}:$CMAKE_PREFIX_PATH"

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
            sqlitecpp
            cli11
            tabulate-pkg
            tomlplusplus
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
