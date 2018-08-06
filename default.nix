{ pkgs ? import <nixpkgs> {} }:
with pkgs;

stdenv.mkDerivation {
  name = "h";

  src = ./.;

  buildInputs = [ ruby ];

  installPhase = ''
    mkdir -p $out/bin
    cp h $out/bin
    cp up $out/bin
  '';
}
