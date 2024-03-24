{
  description = "faster shell navigation of projects";

  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.0";
  };

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-darwin" "x86_64-darwin" "aarch64-linux" ];
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        inherit system;
        pkgs = nixpkgs.legacyPackages.${system};
      });
    in
    {
      packages = forEachSupportedSystem ({ pkgs, ... }: {
        default = import ./default.nix { inherit pkgs; };
      });

      darwinModules.default = { lib, config, pkgs, ... }:
        let
          h = self.packages.${pkgs.stdenv.system}.default;
        in
        {
          options = {
            programs.h.codeRoot = lib.mkOption {
              type = lib.types.str;
              default = "~/src";
              description = lib.mdDoc ''
                Root location for checking out your code.
              '';
            };
          };
          config = {
            environment.systemPackages = [ h ];

            environment.extraInit = ''
              eval "$(${h}/bin/h --setup ${lib.escapeShellArg config.programs.h.codeRoot})"
            '';
          };
        };

      homeModules.default = { lib, config, pkgs, ... }:
        let
          h = self.packages.${pkgs.stdenv.system}.default;
        in
        {
          options = {
            programs.h.codeRoot = lib.mkOption {
              type = lib.types.str;
              default = "~/src";
              description = lib.mdDoc ''
                Root location for checking out your code.
              '';
            };
          };
          config = let
            hook = ''
              eval "$(${h}/bin/h --setup ${lib.escapeShellArg config.programs.h.codeRoot})"
            '';
          in {
            home.packages = [ h ];

            programs.bash.initExtra = hook;
            programs.zsh.initExtra = hook;
          };
        };
    };
}
