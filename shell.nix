with (import <nixpkgs> {});

stdenv.mkDerivation {
  name = "jq";
  buildInputs = [
    git cacert cmake automake autoconf oniguruma libtool
  ];
}
