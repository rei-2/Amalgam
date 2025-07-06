final: prev: {
  olm-gcc-cmake = prev.gccStdenv.mkDerivation {
    name = "olm_gcc_cmake";

    src = ./..;

    nativeBuildInputs = [ prev.cmake ];
    doCheck = true;
    checkPhase = ''
      (cd tests && ctest . -j $NIX_BUILD_CORES)
    '';
  };

  olm-clang-cmake = prev.clangStdenv.mkDerivation {
    name = "olm_clang_cmake";

    src = ./..;

    nativeBuildInputs = [ prev.cmake ];

    doCheck = true;
    checkPhase = ''
      (cd tests && ctest . -j $NIX_BUILD_CORES)
    '';
  };

  olm-gcc-make = prev.gccStdenv.mkDerivation {
    name = "olm";

    src = ./..;

    doCheck = true;
    makeFlags = [ "PREFIX=$out" ];
  };

  olm-javascript = final.buildEmscriptenPackage {
    pname = "olm_javascript";
    inherit (builtins.fromJSON (builtins.readFile ../javascript/package.json)) version;

    src = ./..;

    nativeBuildInputs = with prev; [ gnumake python3 nodejs ];

    postPatch = ''
      patchShebangs .
    '';

    configurePhase = false;

    buildPhase = ''
      export EM_CACHE=$TMPDIR
      make javascript/exported_functions.json
      make js
    '';

    installPhase = ''
      mkdir -p $out/javascript
      cd javascript
      echo sha256: > checksums.txt
      sha256sum olm.js olm_legacy.js olm.wasm >> checksums.txt
      echo sha512: >> checksums.txt
      sha512sum olm.js olm_legacy.js olm.wasm >> checksums.txt
      cp package.json olm.js olm.wasm olm_legacy.js index.d.ts README.md checksums.txt $out/javascript
      cd ..
    '';

    checkPhase = ''
      cd javascript
      export HOME=$TMPDIR
      ln -s ${final.node_modules}/node_modules ./node_modules
      npm test
      cd ..
    '';
  };

}
