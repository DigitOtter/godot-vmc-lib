#!/bin/bash

pushd ..

zip -r godot-vmc-lib.zip godot-vmc-lib/*.gdnlib godot-vmc-lib/*.gdns godot-vmc-lib/bin --exclude *.a --exclude *.zip
mv godot-vmc-lib.zip godot-vmc-lib/bin/

popd
