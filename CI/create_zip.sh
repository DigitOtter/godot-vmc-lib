#!/bin/bash

pushd ..

zip -r godot-vmc-lib.zip godot-vmc-lib/*.gdnlib godot-vmc-lib/*.gdns godot-vmc-lib/bin
mv godot-vmc-lib.zip godot-vmc-lib/bin/

popd
