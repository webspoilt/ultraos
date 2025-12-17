#!/bin/bash

# UltraOS Build Script
# Author: zeroday
# Date: 2025-12-17
# Version: 2.0

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

# Default values
ARCH="${ARCH:-x86_64}"
BUILD_TYPE="${BUILD_TYPE:-debug}"
FEATURES="${FEATURES:-}"
CLEAN=0
VERBOSE=0
HELP=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --arch)
            ARCH="$2"
            shift 2
            ;;
        --type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --features)
            FEATURES="$2"
            shift 2
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --help|-h)
            HELP=1
            shift
            ;;
        setup|configure|build|test|clean|all)
            COMMAND="$1"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Default command
COMMAND="${COMMAND:-build}"

# Print usage information
print_usage() {
    cat << EOF
UltraOS Build Script
Usage: $0 [COMMAND] [OPTIONS]

Commands:
    setup      Create build directories and setup environment
    configure  Configure build for specific architecture and features
    build      Build the kernel and components
    test       Run tests
    clean      Clean build artifacts
    all        Run setup, configure, build, and test

Options:
    --arch ARCH         Target architecture (x86_64, arm64, riscv, powerpc, mips)
    --type TYPE         Build type (debug, release)
    --features FEATURES Comma-separated list of features (AI,QUANTUM,SECURITY)
    --clean            Clean build artifacts before building
    --verbose          Enable verbose output
    --help, -h         Show this help message

Examples:
    $0 setup --arch x86_64
    $0 build --arch x86_64 --type release --features AI,SECURITY
    $0 all --arch arm64 --clean
    $0 test --verbose

Environment Variables:
    ARCH        Target architecture (default: x86_64)
    BUILD_TYPE  Build type (default: debug)
    FEATURES    Features to enable

EOF
}

# Print colored output
print_color() {
    local color="$1"
    shift
    echo -e "${color}$@${NC}"
}

# Print error and exit
error() {
    print_color "$RED" "ERROR: $@"
    exit 1
}

# Print warning
warning() {
    print_color "$YELLOW" "WARNING: $@"
}

# Print success
success() {
    print_color "$GREEN" "SUCCESS: $@"
}

# Print info
info() {
    print_color "$BLUE" "INFO: $@"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check build dependencies
check_dependencies() {
    info "Checking build dependencies..."
    
    local missing_deps=()
    
    # Check for required tools
    for tool in gcc make objcopy objdump strip; do
        if ! command_exists "$tool"; then
            missing_deps+=("$tool")
        fi
    done
    
    # Check for cross-compilers if needed
    if [[ "$ARCH" == "arm64" ]] && ! command_exists "aarch64-linux-gnu-gcc"; then
        missing_deps+=("aarch64-linux-gnu-gcc")
    fi
    
    if [[ "$ARCH" == "riscv" ]] && ! command_exists "riscv64-elf-gcc"; then
        missing_deps+=("riscv64-elf-gcc")
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        error "Missing dependencies: ${missing_deps[*]}"
        echo ""
        echo "Install missing dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install build-essential gcc-aarch64-linux-gnu gcc-riscv64-linux-gnu"
        echo "  CentOS/RHEL:   sudo yum groupinstall 'Development Tools' && sudo yum install gcc-aarch64-linux-gnu gcc-riscv64-linux-gnu"
        echo "  macOS:         brew install gcc aarch64-linux-gnu riscv64-elf-gcc"
        exit 1
    fi
    
    success "All dependencies satisfied"
}

# Setup build environment
setup_build() {
    info "Setting up build environment for $ARCH..."
    
    # Create build directories
    mkdir -p "$PROJECT_DIR/build"
    mkdir -p "$PROJECT_DIR/build/obj/$ARCH"
    mkdir -p "$PROJECT_DIR/build/bin/$ARCH"
    mkdir -p "$PROJECT_DIR/build/tests"
    
    # Create output directories
    mkdir -p "$PROJECT_DIR/build/bin/$ARCH/debug"
    mkdir -p "$PROJECT_DIR/build/bin/$ARCH/release"
    
    success "Build environment setup complete"
}

# Configure build
configure_build() {
    info "Configuring build for $ARCH ($BUILD_TYPE)..."
    
    # Set build flags
    local cflags=""
    local ldflags=""
    
    case "$BUILD_TYPE" in
        debug)
            cflags="-g -O0 -DDEBUG -DULTRAOS_DEBUG=1"
            ;;
        release)
            cflags="-O3 -DNDEBUG -DULTRAOS_DEBUG=0"
            ;;
        *)
            error "Unknown build type: $BUILD_TYPE"
            ;;
    esac
    
    # Add feature flags
    if [[ -n "$FEATURES" ]]; then
        IFS=',' read -ra FEATURE_ARRAY <<< "$FEATURES"
        for feature in "${FEATURE_ARRAY[@]}"; do
            case "$feature" in
                AI)
                    cflags="$cflags -DULTRAOS_AI=1"
                    ;;
                QUANTUM)
                    cflags="$cflags -DULTRAOS_QUANTUM=1"
                    ;;
                SECURITY)
                    cflags="$cflags -DULTRAOS_SECURITY=1"
                    ;;
                *)
                    warning "Unknown feature: $feature"
                    ;;
            esac
        done
    fi
    
    # Set architecture-specific flags
    case "$ARCH" in
        x86_64)
            cflags="$cflags -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3"
            ldflags="-m elf_x86_64"
            ;;
        arm64)
            cflags="$cflags -march=armv8-a -mgeneral-regs-only"
            ldflags="-m aarch64elf"
            ;;
        riscv)
            cflags="$cflags -march=rv64imafdc -mabi=lp64d"
            ldflags="-m elf64lriscv"
            ;;
        powerpc)
            cflags="$cflags -mcpu=power8 -mtune=power8"
            ldflags="-m elf64ppc"
            ;;
        mips)
            cflags="$cflags -march=mips64r2 -mabi=64"
            ldflags="-m elf64btsmip"
            ;;
        *)
            error "Unsupported architecture: $ARCH"
            ;;
    esac
    
    # Export environment variables
    export ARCH
    export BUILD_TYPE
    export FEATURES
    export CFLAGS="$cflags"
    export LDFLAGS="$ldflags"
    
    info "Build configuration:"
    echo "  Architecture: $ARCH"
    echo "  Build Type: $BUILD_TYPE"
    echo "  Features: ${FEATURES:-none}"
    echo "  CFLAGS: $cflags"
    echo "  LDFLAGS: $ldflags"
    
    success "Build configuration complete"
}

# Build the kernel
build_kernel() {
    info "Building UltraOS kernel for $ARCH..."
    
    cd "$PROJECT_DIR"
    
    # Clean if requested
    if [[ $CLEAN -eq 1 ]]; then
        info "Cleaning build artifacts..."
        make clean ARCH="$ARCH"
    fi
    
    # Build kernel
    if [[ $VERBOSE -eq 1 ]]; then
        make ARCH="$ARCH" all
    else
        make ARCH="$ARCH" all > /dev/null 2>&1
    fi
    
    if [[ $? -eq 0 ]]; then
        success "Kernel build complete"
        
        # Show build info
        if [[ -f "build/bin/$ARCH/$BUILD_TYPE/kernel.elf" ]]; then
            local kernel_size=$(stat -f%z "build/bin/$ARCH/$BUILD_TYPE/kernel.elf" 2>/dev/null || stat -c%s "build/bin/$ARCH/$BUILD_TYPE/kernel.elf" 2>/dev/null)
            info "Kernel size: $kernel_size bytes"
        fi
    else
        error "Kernel build failed"
    fi
}

# Run tests
run_tests() {
    info "Running tests..."
    
    cd "$PROJECT_DIR"
    
    # Build tests first
    make tests ARCH="$ARCH" > /dev/null 2>&1
    
    if [[ $? -eq 0 ]]; then
        # Run test binary
        if [[ -f "build/tests/kernel_test" ]]; then
            if [[ $VERBOSE -eq 1 ]]; then
                ./build/tests/kernel_test
            else
                ./build/tests/kernel_test > /dev/null 2>&1
            fi
            
            if [[ $? -eq 0 ]]; then
                success "All tests passed"
            else
                error "Some tests failed"
            fi
        else
            warning "Test binary not found"
        fi
    else
        error "Test build failed"
    fi
}

# Test in QEMU
test_qemu() {
    info "Testing in QEMU..."
    
    cd "$PROJECT_DIR"
    
    # Check if kernel image exists
    if [[ ! -f "build/bin/$ARCH/$BUILD_TYPE/kernel.img" ]]; then
        error "Kernel image not found. Run build first."
    fi
    
    # Check if QEMU is available
    case "$ARCH" in
        x86_64)
            if ! command_exists "qemu-system-x86_64"; then
                warning "QEMU for x86_64 not found"
                return
            fi
            qemu-system-x86_64 -kernel "build/bin/$ARCH/$BUILD_TYPE/kernel.img" -m 512M -nographic -serial stdio -no-reboot
            ;;
        arm64)
            if ! command_exists "qemu-system-aarch64"; then
                warning "QEMU for arm64 not found"
                return
            fi
            qemu-system-aarch64 -kernel "build/bin/$ARCH/$BUILD_TYPE/kernel.img" -m 512M -nographic -serial stdio -machine virt
            ;;
        *)
            warning "QEMU testing not supported for $ARCH"
            ;;
    esac
}

# Show build information
show_info() {
    info "UltraOS Build Information"
    echo "  Project Directory: $PROJECT_DIR"
    echo "  Architecture: $ARCH"
    echo "  Build Type: $BUILD_TYPE"
    echo "  Features: ${FEATURES:-none}"
    echo "  Build Directory: $PROJECT_DIR/build"
    
    if [[ -d "build/bin/$ARCH" ]]; then
        echo "  Available Builds:"
        ls -la "build/bin/$ARCH/" 2>/dev/null | grep "^d" | awk '{print "    " $9}' || true
    fi
}

# Main execution
main() {
    if [[ $HELP -eq 1 ]]; then
        print_usage
        exit 0
    fi
    
    # Check dependencies first
    check_dependencies
    
    case "$COMMAND" in
        setup)
            setup_build
            ;;
        configure)
            setup_build
            configure_build
            ;;
        build)
            setup_build
            configure_build
            build_kernel
            ;;
        test)
            run_tests
            ;;
        clean)
            info "Cleaning build artifacts..."
            make clean ARCH="$ARCH" > /dev/null 2>&1
            rm -rf build/
            success "Build artifacts cleaned"
            ;;
        all)
            setup_build
            configure_build
            build_kernel
            run_tests
            ;;
        *)
            error "Unknown command: $COMMAND"
            ;;
    esac
}

# Show info if no command specified
if [[ -z "$COMMAND" ]]; then
    show_info
    echo ""
    print_usage
    exit 1
fi

# Run main function
main