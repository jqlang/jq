with (import <nixpkgs> {});

mkShell {
  name = "jq";
  buildInputs = [
    git cacert cmake automake autoconf libtool
  ];
}
