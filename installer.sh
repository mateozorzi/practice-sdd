#!/bin/bash

# Nedd4Speed Installer Script para Ubuntu
# Descarga dependencias y compila el proyecto

set -e  # Exit on any error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para imprimir mensajes
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

# Verificar que estamos en Ubuntu
check_ubuntu() {
    if [ ! -f /etc/os-release ]; then
        print_error "No se pudo detectar el sistema operativo"
        exit 1
    fi
    
    source /etc/os-release
    if [ "$ID" != "ubuntu" ] && [ "$ID" != "debian" ]; then
        print_warning "Este script está optimizado para Ubuntu/Debian. Detected: $ID"
        read -p "¿Continuar de todos modos? (y/n): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Instalar dependencias
install_dependencies() {
    print_info "Actualizando lista de paquetes..."
    sudo apt update

    print_info "Instalando herramientas de compilación..."
    sudo apt install -y \
        build-essential \
        cmake \
        git \
        pkg-config

    print_info "Instalando SDL2 y sus componentes..."
    sudo apt install -y \
        libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-ttf-dev \

    print_info "Instalando dependencias de SDL2_mixer"
    sudo apt install -y \
        libopus-dev \
        libopusfile-dev \
        libxmp-dev \
        libfluidsynth-dev \
        fluidsynth \
        libwavpack-dev \
        libfreetype-dev

    print_info "Instalando Box2D..."
    sudo apt install -y libbox2d-dev

    print_info "Instalando Qt6..."
    sudo apt install -y \
        qt6-base-dev \
        qt6-tools-dev \
        qt6-l10n-tools \
        qt6-multimedia-dev 

    print_info "Instalando JSON y YAML..."
    sudo apt install -y \
        libjsoncpp-dev \
        libyaml-cpp-dev

    print_info "Instalando dependencias adicionales..."
    sudo apt install -y \
        libgl1-mesa-dev \
        libgles2-mesa-dev \
        libegl1-mesa-dev
}

# Verificar dependencias instaladas
verify_dependencies() {
    print_info "Verificando dependencias instaladas..."
    
    local missing_deps=()
    
    # Verificar comandos básicos
    for cmd in cmake g++ pkg-config; do
        if ! command -v $cmd &> /dev/null; then
            missing_deps+=("$cmd")
        fi
    done
    
    # Verificar librerías con pkg-config
    for lib in sdl2 SDL2_image SDL2_ttf jsoncpp yaml-cpp; do
        if ! pkg-config --exists $lib; then
            missing_deps+=("$lib")
        fi
    done
    
    # Verificar Qt6
    if ! pkg-config --exists Qt6Core; then
        missing_deps+=("qt6")
    fi
    
    # Verificar Box2D (puede no tener pkg-config)
    if [ ! -f /usr/include/box2d/box2d.h ] && [ ! -f /usr/local/include/box2d/box2d.h ]; then
        missing_deps+=("box2d")
    fi
    
    if [[ ${#missing_deps[@]} -ne 0 ]]; then
        print_error "Dependencias faltantes: ${missing_deps[*]}"
        return 1
    fi
    
    print_success "Todas las dependencias verificadas correctamente"
    return 0
}

# Compilar el proyecto
compile_project() {
    print_info "Compilando el proyecto Nedd4Speed..."
    
    # Eliminar carpeta build si existe
    if [ -d "build" ]; then
        print_info "Eliminando carpeta build existente..."
        rm -rf build
    fi
    
    # Crear y entrar a build
    mkdir build
    cd build
    
    print_info "Ejecutando cmake con flags..."
    cmake .. -DNFS_EDITOR=ON -DNFS_CLIENT=ON -DNFS_SERVER=ON
    
    # Detectar número de cores para compilación paralela
    CORES=$(nproc)
    print_info "Compilando con make usando $CORES cores..."
    make -j$CORES
    
    # Volver al directorio root
    cd ..
    
    print_success "Compilación completada exitosamente"
}

# Mostrar resumen final
show_summary() {
    print_success "¡Instalación de Nedd4Speed completada!"
    echo ""
    print_info "Resumen:"
    echo "  ✓ Dependencias instaladas"
    echo "  ✓ Proyecto compilado"
    echo "  ✓ Tests ejecutados"
    echo ""
    print_info "Los ejecutables están en la carpeta: $(pwd)/build/"
    echo ""
    
    # Listar ejecutables si existen
    if [ -d "build" ]; then
        print_info "Ejecutables encontrados:"
        find build -maxdepth 1 -type f -executable -exec echo "  - {}" \;
    fi
}

run_tests() {
    echo ""
    echo "=========================================="
    echo "        EJECUTANDO TESTS"
    echo "=========================================="
    echo ""
    
    # Verificar si el ejecutable de tests existe
    if [ ! -f "build/nfs_tests" ]; then
        print_error "No se encontró el ejecutable de tests: build/nfs_tests"
        return 1
    fi
    
    # Navegar al directorio build y ejecutar tests
    cd build
    echo "Ejecutando tests..."
    echo ""
    
    # Ejecutar los tests con verbosity
    ./nfs_tests
}

# Función principal
main() {
    echo "=========================================="
    echo "    INSTALADOR NEDD4SPEED PARA UBUNTU"
    echo "=========================================="
    echo ""
    
    # Verificar Ubuntu
    check_ubuntu
    
    # Instalar dependencias
    install_dependencies
    
    # Verificar dependencias
    if ! verify_dependencies; then
        print_error "Hay problemas con las dependencias. Revisa los mensajes anteriores."
        exit 1
    fi
    
    # Compilar proyecto
    compile_project

    # Ejecutar tests
    run_tests
    
    # Mostrar resumen
    show_summary
}

# Ejecutar función principal
main "$@"