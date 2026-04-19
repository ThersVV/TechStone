set -euxo pipefail

if [[ -f /etc/os-release ]]; then
    # Linux installation
    . /etc/os-release
    if [[ "$ID" == "ubuntu" ]]; then
        apt update && apt -y install \
            sudo build-essential cmake gdb nasm pkg-config git curl zip unzip tar python3 \
            bison libxext-dev autoconf libtool libxrandr-dev libxi-dev libxcursor-dev \
            libxdamage-dev libxinerama-dev libdrm-dev
    elif [[ "$ID" == "fedora" ]]; then
        sudo dnf update -y && dnf -y install \
            cmake gdb nasm pkgconf-pkg-config git curl zip unzip tar python3 gcc-c++ \
            bison autoconf libtool libXcursor-devel libdrm-devel libXext-devel libXrandr-devel \
            libXi-devel libXdamage-devel libXinerama-devel
        # This looks an extra package but for some reason it needs to be done in fedora only:
        # Maybe only on docker, I cannot be sure about that.
        sudo dnf install glibc-gconv-extra -y
    else
        echo "Unsupported Linux distribution: $ID"
        exit 1
    fi
fi
