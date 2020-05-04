with (import <nixpkgs> {});

mkShell {
  buildInputs = [
    git cacert cmake automake autoconf libtool flex bison
  ];
}
