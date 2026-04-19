set -euxo pipefail

VCPKG_ROOT="C:\\vcpkg" # TODO nice printing if not found

os_name=$(uname)
if [[ "$os_name" == "Linux" ]]; then
    TRIPLET="x64-linux"
elif [[ "$os_name" == "CYGWIN"* || "$os_name" == "MINGW"* || "$os_name" == "MSYS"* ]]; then
    TRIPLET="x64-windows"
else
    echo "Unsupported OS: $os_name"
    exit 1
fi

# TODO ne vzdycky se to musi instalovat
$VCPKG_ROOT/bootstrap-vcpkg.sh;
export PATH=$PATH:$VCPKG_ROOT;
$VCPKG_ROOT/vcpkg install fmt:$TRIPLET
$VCPKG_ROOT/vcpkg install gtkmm:$TRIPLET
$VCPKG_ROOT/vcpkg install libsigcpp:$TRIPLET
$VCPKG_ROOT/vcpkg install pkgconf:$TRIPLET
CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake";

cmake ./CMakeLists.txt -DVCPKG_ROOT=$VCPKG_ROOT -DTRIPLET=$TRIPLET -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE
