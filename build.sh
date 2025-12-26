#!/bin/bash

# Script de build universel pour projet SDL3
# Compatible toutes distributions Linux et RedHat

set -e  # Arrêter en cas d'erreur

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="mon_projet"
SRC_DIR="src"
BUILD_DIR="build"
EXTERNAL_DIR="external"

# Détection de la distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_VERSION=$VERSION_ID
    else
        DISTRO="unknown"
    fi
    echo -e "${GREEN}Distribution détectée: $DISTRO${NC}"
}

# Installation des dépendances système
install_system_deps() {
    echo -e "${YELLOW}Vérification des dépendances système...${NC}"
    
    case $DISTRO in
        ubuntu|debian|linuxmint)
            echo "Installation des dépendances pour Debian/Ubuntu..."
            sudo apt-get update
            sudo apt-get install -y build-essential cmake git \
                libx11-dev libxext-dev libwayland-dev libxkbcommon-dev \
                libasound2-dev libpulse-dev libudev-dev \
                libdbus-1-dev libfreetype6-dev libharfbuzz-dev \
                libpng-dev libjpeg-dev libwebp-dev libtiff-dev
            ;;
        fedora|rhel|centos|rocky|almalinux)
            echo "Installation des dépendances pour RedHat/Fedora..."
            if command -v dnf &> /dev/null; then
                sudo dnf groupinstall -y "Development Tools"
                sudo dnf install -y cmake git \
                    libX11-devel libXext-devel wayland-devel libxkbcommon-devel \
                    alsa-lib-devel pulseaudio-libs-devel systemd-devel \
                    dbus-devel freetype-devel harfbuzz-devel \
                    libpng-devel libjpeg-turbo-devel libwebp-devel libtiff-devel
            else
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y cmake git \
                    libX11-devel libXext-devel wayland-devel libxkbcommon-devel \
                    alsa-lib-devel pulseaudio-libs-devel systemd-devel \
                    dbus-devel freetype-devel harfbuzz-devel \
                    libpng-devel libjpeg-turbo-devel libwebp-devel libtiff-devel
            fi
            ;;
        arch|manjaro)
            echo "Installation des dépendances pour Arch Linux..."
            sudo pacman -Sy --needed --noconfirm base-devel cmake git \
                libx11 libxext wayland libxkbcommon \
                alsa-lib libpulse systemd \
                dbus freetype2 harfbuzz \
                libpng libjpeg-turbo libwebp libtiff
            ;;
        opensuse*|sles)
            echo "Installation des dépendances pour openSUSE..."
            sudo zypper install -y -t pattern devel_basis
            sudo zypper install -y cmake git \
                libX11-devel libXext-devel wayland-devel libxkbcommon-devel \
                alsa-devel libpulse-devel systemd-devel \
                dbus-1-devel freetype2-devel harfbuzz-devel \
                libpng16-devel libjpeg8-devel libwebp-devel libtiff-devel
            ;;
        *)
            echo -e "${YELLOW}Distribution non reconnue. Installation manuelle des dépendances peut être nécessaire.${NC}"
            ;;
    esac
    
    echo -e "${GREEN}Dépendances système installées.${NC}"
}

# Vérification des bibliothèques SDL3
check_sdl_libs() {
    echo -e "${YELLOW}Vérification des bibliothèques SDL3...${NC}"
    
    local missing=0
    
    if [ ! -d "$EXTERNAL_DIR/sdl3" ]; then
        echo -e "${RED}ERREUR: $EXTERNAL_DIR/sdl3 introuvable!${NC}"
        missing=1
    fi
    
    if [ ! -d "$EXTERNAL_DIR/sdl3_mixer" ]; then
        echo -e "${RED}ERREUR: $EXTERNAL_DIR/sdl3_mixer introuvable!${NC}"
        missing=1
    fi
    
    if [ ! -d "$EXTERNAL_DIR/sdl3_ttf" ]; then
        echo -e "${RED}ERREUR: $EXTERNAL_DIR/sdl3_ttf introuvable!${NC}"
        missing=1
    fi
    
    if [ ! -d "$EXTERNAL_DIR/sdl3_image" ]; then
        echo -e "${RED}ERREUR: $EXTERNAL_DIR/sdl3_image introuvable!${NC}"
        missing=1
    fi
    
    if [ $missing -eq 1 ]; then
        echo -e "${RED}Une ou plusieurs bibliothèques SDL3 sont manquantes dans $EXTERNAL_DIR${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Toutes les bibliothèques SDL3 sont présentes.${NC}"
}

# Compilation du projet
build_project() {
    echo -e "${YELLOW}Compilation du projet...${NC}"
    
    # Créer le dossier build si nécessaire
    mkdir -p "$BUILD_DIR"
    
    # Utiliser le Makefile
    make clean
    make -j$(nproc)
    
    echo -e "${GREEN}Compilation terminée avec succès!${NC}"
}

# Configuration de LD_LIBRARY_PATH
setup_library_path() {
    export LD_LIBRARY_PATH="$PWD/$EXTERNAL_DIR/sdl3/lib:$PWD/$EXTERNAL_DIR/sdl3_mixer/lib:$PWD/$EXTERNAL_DIR/sdl3_ttf/lib:$PWD/$EXTERNAL_DIR/sdl3_image/lib:$LD_LIBRARY_PATH"
    echo -e "${GREEN}LD_LIBRARY_PATH configuré.${NC}"
}

# Exécution du programme
run_project() {
    echo -e "${YELLOW}Exécution du projet...${NC}"
    setup_library_path
    ./"$BUILD_DIR/$PROJECT_NAME"
}

# Affichage de l'aide
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Afficher cette aide"
    echo "  -i, --install-deps      Installer les dépendances système"
    echo "  -b, --build             Compiler le projet uniquement"
    echo "  -r, --run               Compiler et exécuter le projet"
    echo "  -c, --clean             Nettoyer les fichiers de build"
    echo ""
    echo "Exemples:"
    echo "  $0 --install-deps       # Installer les dépendances"
    echo "  $0 --build              # Compiler"
    echo "  $0 --run                # Compiler et exécuter"
}

# Fonction principale
main() {
    detect_distro
    
    case "$1" in
        -h|--help)
            show_help
            ;;
        -i|--install-deps)
            install_system_deps
            ;;
        -b|--build)
            check_sdl_libs
            build_project
            ;;
        -r|--run)
            check_sdl_libs
            build_project
            run_project
            ;;
        -c|--clean)
            make clean
            echo -e "${GREEN}Nettoyage terminé.${NC}"
            ;;
        *)
            echo -e "${YELLOW}Build automatique...${NC}"
            check_sdl_libs
            build_project
            echo ""
            echo -e "${GREEN}Build terminé! Utilisez './build.sh --run' pour exécuter.${NC}"
            ;;
    esac
}

# Exécution
main "$@"