#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_ubuntu() {
    if [ ! -f /etc/os-release ]; then
        print_error "No se pudo detectar el sistema operativo"
        exit 1
    fi
    
    source /etc/os-release
    if [ "$ID" != "ubuntu" ] && [ "$ID" != "debian" ]; then
        print_warning "Este instalador fue probado en Ubuntu/Debian. Detectado: $ID"
        read -p "¿Continuar de todos modos? (y/n): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

install_dependencies() {
    print_info "Actualizando paquetes..."
    sudo apt update

    print_info "Instalando dependencias..."
    sudo apt install -y \
        build-essential \
        cmake \
        pkg-config \
        libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-ttf-dev \
        libopus-dev \
        libopusfile-dev \
        libxmp-dev \
        libfluidsynth-dev \
        fluidsynth \
        libwavpack-dev \
        libfreetype-dev \
        libbox2d-dev \
        qt6-base-dev \
        qt6-tools-dev \
        qt6-l10n-tools \
        qt6-multimedia-dev \
        libjsoncpp-dev \
        libyaml-cpp-dev \
        libgl1-mesa-dev \
        libgles2-mesa-dev \
        libegl1-mesa-dev
}

verify_dependencies() {
    print_info "Verificando dependencias..."
    
    local missing=()

    for cmd in cmake g++; do
        if ! command -v $cmd &> /dev/null; then
            missing+=("$cmd")
        fi
    done
    
    for lib in sdl2 SDL2_image SDL2_ttf jsoncpp yaml-cpp; do
        if ! pkg-config --exists $lib; then
            missing+=("$lib")
        fi
    done
    
    if ! pkg-config --exists Qt6Core; then
        missing+=("qt6")
    fi
    
    if [ ! -f /usr/include/box2d/box2d.h ] && [ ! -f /usr/local/include/box2d/box2d.h ]; then
        missing+=("box2d")
    fi
    
    if [[ ${#missing[@]} -ne 0 ]]; then
        print_error "Dependencias faltantes: ${missing[*]}"
        return 1
    fi
    
    print_success "Dependencias OK"
    return 0
}

compile_project() {
    print_info "Compilando NFS..."

    if [ -d "build" ]; then
        print_info "La carpeta build ya existe, usando build existente"
        cd build
        cmake .. -DNFS_EDITOR=ON -DNFS_CLIENT=ON -DNFS_SERVER=ON
    else
        mkdir build
        cd build
        cmake .. -DNFS_EDITOR=ON -DNFS_CLIENT=ON -DNFS_SERVER=ON
    fi
    
    CORES=$(nproc)
    make -j$CORES
    cd ..
    print_success "Compilación completa"
}

run_tests() {
    echo ""
    echo "=========================================="
    echo "        EJECUTANDO TESTS"
    echo "=========================================="
    echo ""

    if [ ! -f "build/nfs_tests" ]; then
        print_error "No se encontró build/nfs_tests"
        return 1
    fi
    
    (cd build && ./nfs_tests)
}

install_binaries() {
    print_info "Instalando ejecutables en /usr/bin..."

    sudo install -m 755 build/nfs_client /usr/bin/nfs_client
    sudo install -m 755 build/nfs_server /usr/bin/nfs_server
    sudo install -m 755 build/nfs_editor /usr/bin/nfs_editor
    sudo install -m 755 build/nfs_tests /usr/bin/nfs_tests

    print_success "Ejecutables instalados"
}

install_assets() {
    print_info "Instalando assets en /var/nfs/assets..."

    sudo mkdir -p /var/nfs/assets/
    sudo cp -r client/assets/* /var/nfs/assets/

    print_info "Instalando races en /var/nfs/races..."
    sudo mkdir -p /var/nfs/races
    sudo cp -r server/races/* /var/nfs/races/

    print_info "Instalando world en /var/nfs/world..."
    sudo mkdir -p /var/nfs/world
    sudo cp -r server/world/* /var/nfs/world/

    print_info "Ajustando permisos..."
    sudo chown -R "$USER":"$USER" /var/nfs/assets
    sudo chown -R "$USER":"$USER" /var/nfs/races
    sudo chown -R "$USER":"$USER" /var/nfs/world

    print_success "Assets + races + world instalados"
}

install_configs() {
    print_info "Instalando configuraciones en /etc/nfs..."

    sudo mkdir -p /etc/nfs/
    sudo cp -r server/config/* /etc/nfs/

    print_success "Configuraciones instaladas"
}

show_summary() {
    print_success "Instalación completa"
    print_info "Ejecutables: /usr/bin"
    print_info "Assets: /var/nfs/assets"
    print_info "Races: /var/nfs/races"
    print_info "World: /var/nfs/world"
    print_info "Configs: /etc/nfs"
}

main() {
    echo "=========================================="
    echo "    INSTALADOR NFS PARA UBUNTU"
    echo "=========================================="
    echo ""
    
    check_ubuntu
    install_dependencies
    verify_dependencies
    compile_project
    run_tests
    install_binaries
    install_assets
    install_configs
    show_summary
}

main "$@"
