#!/bin/bash
set -euo pipefail
headers=$(find ./src/code -name '*.h' | sed 's/^/-i /')
# hpp2plantuml $headers -o output.puml
plantuml -tpng output.puml -o output.png
