# UltraOS Optimized Build System
# Author: zeroday
# Version: 2.0
# Date: 2025-12-17

# Compiler settings
CC ?= gcc
AS ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
OBJDUMP ?= objdump
STRIP ?= strip

# Architecture detection and settings
ARCH ?= x86_64
ifeq ($(ARCH),x86_64)
    TARGET_TRIPLE = x86_64-elf
    CROSS_COMPILE = x86_64-elf-
    CFLAGS_ARCH = -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3
    ASFLAGS_ARCH = -D__x86_64__
    LDFLAGS_ARCH = -m elf_x86_64
    ARCH_DEFINES = -DARCH_X86_64=1 -D__x86_64__ -D__x86_64
    QEMU_ARCH = x86_64
else ifeq ($(ARCH),arm64)
    TARGET_TRIPLE = aarch64-elf
    CROSS_COMPILE = aarch64-elf-
    CFLAGS_ARCH = -march=armv8-a -mgeneral-regs-only -D__arm64__
    ASFLAGS_ARCH = -D__arm64__
    LDFLAGS_ARCH = -m aarch64elf
    ARCH_DEFINES = -DARCH_ARM64=1 -D__arm64__ -D__arm64
    QEMU_ARCH = aarch64
else ifeq ($(ARCH),riscv)
    TARGET_TRIPLE = riscv64-elf
    CROSS_COMPILE = riscv64-elf-
    CFLAGS_ARCH = -march=rv64imafdc -mabi=lp64d
    ASFLAGS_ARCH = -D__riscv__
    LDFLAGS_ARCH = -m elf64lriscv
    ARCH_DEFINES = -DARCH_RISCV=1 -D__riscv__
    QEMU_ARCH = riscv64
else ifeq ($(ARCH),powerpc)
    TARGET_TRIPLE = powerpc64-elf
    CROSS_COMPILE = powerpc64-elf-
    CFLAGS_ARCH = -mcpu=power8 -mtune=power8
    ASFLAGS_ARCH = -D__powerpc64__
    LDFLAGS_ARCH = -m elf64ppc
    ARCH_DEFINES = -DARCH_POWERPC=1 -D__powerpc64__
    QEMU_ARCH = ppc64
else ifeq ($(ARCH),mips)
    TARGET_TRIPLE = mips64-elf
    CROSS_COMPILE = mips64-elf-
    CFLAGS_ARCH = -march=mips64r2 -mabi=64
    ASFLAGS_ARCH = -D__mips__
    LDFLAGS_ARCH = -m elf64btsmip
    ARCH_DEFINES = -DARCH_MIPS=1 -D__mips__
    QEMU_ARCH = mips64
else
    $(error Unsupported architecture: $(ARCH))
endif

# Build type configuration
DEBUG ?= 0
RELEASE ?= 0
ifeq ($(DEBUG),1)
    CFLAGS_BUILD = -g -DDEBUG -O0 -DULTRAOS_DEBUG=1
    BUILD_SUFFIX = debug
else ifeq ($(RELEASE),1)
    CFLAGS_BUILD = -O3 -DNDEBUG -DULTRAOS_DEBUG=0
    BUILD_SUFFIX = release
else
    CFLAGS_BUILD = -O2 -DULTRAOS_DEBUG=0
    BUILD_SUFFIX = default
endif

# Feature flags
FEATURES ?=
ifeq ($(findstring AI,$(FEATURES)),AI)
    CFLAGS_FEATURES += -DULTRAOS_AI=1
endif
ifeq ($(findstring QUANTUM,$(FEATURES)),QUANTUM)
    CFLAGS_FEATURES += -DULTRAOS_QUANTUM=1
endif
ifeq ($(findstring SECURITY,$(FEATURES)),SECURITY)
    CFLAGS_FEATURES += -DULTRAOS_SECURITY=1
endif

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -fno-builtin \
         -ffreestanding -fno-exceptions -fno-rtti -fstack-protector-strong \
         -D__KERNEL__ $(ARCH_DEFINES) $(CFLAGS_ARCH) $(CFLAGS_BUILD) \
         $(CFLAGS_FEATURES)

ASFLAGS = $(CFLAGS_ARCH) -c -x assembler-with-cpp
LDFLAGS = -nostdlib -z max-page-size=0x1000 $(LDFLAGS_ARCH) -T linker.ld.$(ARCH)

# Include directories
INCLUDE_DIRS = -I./include -I./include/kernel -I./include/arch \
               -I./include/drivers -I./include/utils
CFLAGS += $(INCLUDE_DIRS)

# Source directories
SRC_DIR = kernel
INC_DIR = include
TOOLS_DIR = tools
TEST_DIR = tests
BUILD_DIR = build

# Architecture-specific directories
ifeq ($(ARCH),x86_64)
    ARCH_DIR = arch/x86_64
else ifeq ($(ARCH),arm64)
    ARCH_DIR = arch/arm64
else ifeq ($(ARCH),riscv)
    ARCH_DIR = arch/riscv
else ifeq ($(ARCH),powerpc)
    ARCH_DIR = arch/powerpc
else ifeq ($(ARCH),mips)
    ARCH_DIR = arch/mips
endif

# Build directories
OBJ_DIR = $(BUILD_DIR)/obj/$(ARCH)/$(BUILD_SUFFIX)
BIN_DIR = $(BUILD_DIR)/bin/$(ARCH)/$(BUILD_SUFFIX)
TEST_BIN_DIR = $(BUILD_DIR)/tests

# Output files
KERNEL_BIN = $(BIN_DIR)/kernel.elf
KERNEL_IMAGE = $(BIN_DIR)/kernel.img
KERNEL_DEBUG = $(BIN_DIR)/kernel.debug

# Linker script
LINKER_SCRIPT = linker.ld.$(ARCH)

# Source files
KERNEL_C_SOURCES = \
    $(SRC_DIR)/core/kernel.c \
    $(SRC_DIR)/core/string.c \
    $(SRC_DIR)/core/printk.c \
    $(SRC_DIR)/memory/memory.c \
    $(SRC_DIR)/scheduler/scheduler.c \
    $(SRC_DIR)/scheduler/process.c \
    $(SRC_DIR)/scheduler/thread.c \
    $(SRC_DIR)/hal/hal.c \
    $(SRC_DIR)/arch/$(ARCH_DIR)/cpu.c \
    $(SRC_DIR)/arch/$(ARCH_DIR)/interrupt.c \
    $(SRC_DIR)/arch/$(ARCH_DIR)/context_switch.S \
    $(SRC_DIR)/drivers/serial.c \
    $(SRC_DIR)/drivers/timer.c \
    $(SRC_DIR)/drivers/keyboard.c \
    $(SRC_DIR)/security/security.c

# Add feature-specific sources
ifeq ($(findstring AI,$(FEATURES)),AI)
    KERNEL_C_SOURCES += \
        $(SRC_DIR)/ai/gpu_accel.c \
        $(SRC_DIR)/ai/inference_engine.c \
        $(SRC_DIR)/ai/tensor.c
endif

ifeq ($(findstring QUANTUM,$(FEATURES)),QUANTUM)
    KERNEL_C_SOURCES += \
        $(SRC_DIR)/quantum/quantum.c \
        $(SRC_DIR)/quantum/circuit.c \
        $(SRC_DIR)/quantum/simulator.c
endif

ifeq ($(findstring SECURITY,$(FEATURES)),SECURITY)
    KERNEL_C_SOURCES += \
        $(SRC_DIR)/security/crypto.c \
        $(SRC_DIR)/security/pqc.c \
        $(SRC_DIR)/security/zerotrust.c
endif

# Assembly sources
KERNEL_ASM_SOURCES = \
    $(SRC_DIR)/arch/$(ARCH_DIR)/context_switch.S \
    $(SRC_DIR)/arch/$(ARCH_DIR)/boot.S

# Test sources
TEST_C_SOURCES = \
    $(TEST_DIR)/unit/test_kernel.c \
    $(TEST_DIR)/unit/test_memory.c \
    $(TEST_DIR)/unit/test_scheduler.c \
    $(TEST_DIR)/integration/test_hal.c

# Object files
KERNEL_OBJECTS = $(KERNEL_C_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
KERNEL_OBJECTS += $(KERNEL_ASM_SOURCES:$(SRC_DIR)/%.S=$(OBJ_DIR)/%.o)
TEST_OBJECTS = $(TEST_C_SOURCES:$(TEST_DIR)/%.c=$(OBJ_DIR)/tests/%.o)

# Default target
.PHONY: all clean help kernel drivers tests run-test install debug release
.PHONY: qemu-test qemu-test-all qemu-test-interactive
.PHONY: check-deps info tree backup detect-cpu
.PHONY: docs api-docs examples

all: check-deps kernel tests

# Check build dependencies
check-deps:
	@echo "Checking build dependencies..."
	@command -v $(CC) >/dev/null 2>&1 || { echo "$(CC) not found"; exit 1; }
	@command -v $(AS) >/dev/null 2>&1 || { echo "$(AS) not found"; exit 1; }
	@command -v $(LD) >/dev/null 2>&1 || { echo "$(LD) not found"; exit 1; }
	@command -v $(OBJCOPY) >/dev/null 2>&1 || { echo "$(OBJCOPY) not found"; exit 1; }
	@command -v $(OBJDUMP) >/dev/null 2>&1 || { echo "$(OBJDUMP) not found"; exit 1; }
	@echo "All dependencies satisfied"

# Build kernel
kernel: $(KERNEL_BIN) $(KERNEL_IMAGE)
	@echo "Built kernel for $(ARCH) ($(BUILD_SUFFIX))"
	@$(OBJDUMP) -h $(KERNEL_BIN) | head -20

# Build kernel binary
$(KERNEL_BIN): $(KERNEL_OBJECTS) $(LINKER_SCRIPT)
	@echo "Linking kernel..."
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $^
	@echo "Kernel linked successfully"

# Create kernel image
$(KERNEL_IMAGE): $(KERNEL_BIN)
	@echo "Creating kernel image..."
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary -S $< $@
	$(STRIP) --strip-debug $< -o $(KERNEL_DEBUG)
	@echo "Kernel image created: $@"

# Build kernel objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@echo "Assembling $<"
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

# Build tests
tests: $(TEST_BIN_DIR)/kernel_test
	@echo "Built test suite"

# Build test binary
$(TEST_BIN_DIR)/kernel_test: $(TEST_OBJECTS) $(KERNEL_BIN)
	@echo "Linking test suite..."
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) -o $@ $^ -L$(OBJ_DIR) -lultraos -lpthread
	@echo "Tests linked successfully"

# Build test objects
$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.c
	@echo "Compiling test $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DTEST_BUILD -I./tests/include -c -o $@ $<

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Show help
help:
	@echo "UltraOS Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build kernel and tests (default)"
	@echo "  kernel      - Build kernel only"
	@echo "  tests       - Build test suite"
	@echo "  clean       - Remove all build artifacts"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Architecture targets:"
	@echo "  x86_64      - Build for x86_64 (default)"
	@echo "  arm64       - Build for ARM64"
	@echo "  riscv       - Build for RISC-V"
	@echo "  powerpc     - Build for PowerPC"
	@echo "  mips        - Build for MIPS"
	@echo ""
	@echo "Build types:"
	@echo "  debug       - Debug build with symbols (DEBUG=1)"
	@echo "  release     - Optimized release build (RELEASE=1)"
	@echo ""
	@echo "Features:"
	@echo "  AI          - Enable AI/ML acceleration (FEATURES=AI)"
	@echo "  QUANTUM     - Enable quantum computing (FEATURES=QUANTUM)"
	@echo "  SECURITY    - Enable security features (FEATURES=SECURITY)"
	@echo ""
	@echo "Usage examples:"
	@echo "  make ARCH=x86_64"
	@echo "  make ARCH=arm64 DEBUG=1"
	@echo "  make ARCH=riscv RELEASE=1 FEATURES=AI,SECURITY"
	@echo "  make clean all"
	@echo ""
	@echo "Environment variables:"
	@echo "  ARCH        - Target architecture"
	@echo "  DEBUG       - Enable debug mode (0/1)"
	@echo "  RELEASE     - Enable release mode (0/1)"
	@echo "  FEATURES    - Enable specific features"
	@echo "  CC          - C compiler"
	@echo "  AS          - Assembler"
	@echo "  LD          - Linker"

# Debug build
debug: DEBUG=1
debug: all

# Release build
release: RELEASE=1
release: all

# Architecture-specific targets
x86_64: ARCH = x86_64
x86_64: all

arm64: ARCH = arm64
arm64: all

riscv: ARCH = riscv
riscv: all

powerpc: ARCH = powerpc
powerpc: all

mips: ARCH = mips
mips: all

# Install targets (placeholder)
install: kernel image
	@echo "Installing kernel..."
	# Would copy kernel to installation directory
	@echo "Installation complete"

# Unit test targets
test: tests
	@echo "Running unit tests..."
	@if [ -f $(TEST_BIN_DIR)/kernel_test ]; then \
		$(TEST_BIN_DIR)/kernel_test; \
	else \
		echo "Test binary not found. Run 'make tests' first."; \
	fi

# Quick build for testing
test-build: clean
	@echo "Quick test build..."
	$(MAKE) kernel

# Build all architectures
all-archs:
	@echo "Building for all architectures..."
	$(MAKE) clean
	$(MAKE) ARCH=x86_64 all
	$(MAKE) ARCH=arm64 all
	$(MAKE) ARCH=riscv all
	@echo "All architectures built"

# Build information
info:
	@echo "Build Configuration:"
	@echo "  Architecture:     $(ARCH)"
	@echo "  Build Type:       $(BUILD_SUFFIX)"
	@echo "  Compiler:         $(CC)"
	@echo "  Assembler:        $(AS)"
	@echo "  Linker:           $(LD)"
	@echo "  CFLAGS:          $(CFLAGS)"
	@echo "  LDFLAGS:         $(LDFLAGS)"
	@echo "  Features:         $(FEATURES)"
	@echo "  Kernel sources:   $(words $(KERNEL_C_SOURCES))"
	@echo "  Assembly sources: $(words $(KERNEL_ASM_SOURCES))"
	@echo "  Object files:     $(words $(KERNEL_OBJECTS))"
	@echo "  Build directory:  $(BUILD_DIR)"

# Show source tree
tree:
	@echo "Source tree:"
	@find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.S" -o -name "*.ld" -o -name "Makefile" \) | sort

# Detect CPU features
detect-cpu:
	@echo "CPU Feature Detection:"
	@if grep -q avx2 /proc/cpuinfo; then echo "  AVX2: yes"; else echo "  AVX2: no"; fi
	@if grep -q avx /proc/cpuinfo; then echo "  AVX: yes"; else echo "  AVX: no"; fi
	@if grep -q sse4_2 /proc/cpuinfo; then echo "  SSE4.2: yes"; else echo "  SSE4.2: no"; fi
	@if grep -q sse4_1 /proc/cpuinfo; then echo "  SSE4.1: yes"; else echo "  SSE4.1: no"; fi
	@if grep -q ssse3 /proc/cpuinfo; then echo "  SSSE3: yes"; else echo "  SSSE3: no"; fi
	@if grep -q sse3 /proc/cpuinfo; then echo "  SSE3: yes"; else echo "  SSE3: no"; fi
	@if grep -q sse2 /proc/cpuinfo; then echo "  SSE2: yes"; else echo "  SSE2: no"; fi
	@if grep -q sse /proc/cpuinfo; then echo "  SSE: yes"; else echo "  SSE: no"; fi

# Documentation targets
docs:
	@echo "Generating documentation..."
	@mkdir -p docs/api
	@echo "# UltraOS API Documentation" > docs/api/README.md
	@find include -name "*.h" -exec basename {} \; | sort >> docs/api/README.md
	@echo "Documentation generated in docs/api/"

api-docs:
	@echo "Generating API documentation..."
	@mkdir -p docs/api
	@doxygen docs/doxygen.conf 2>/dev/null || echo "Doxygen not found, skipping API docs"

examples:
	@echo "Building examples..."
	@mkdir -p $(BIN_DIR)/examples
	@for example in examples/basic/*.c; do \
		if [ -f "$$example" ]; then \
			$$(echo $(CC) $(CFLAGS) -o $(BIN_DIR)/examples/$$(basename $$example .c) $$example); \
		fi; \
	done
	@echo "Examples built in $(BIN_DIR)/examples"

# QEMU testing
qemu-test: kernel
	@echo "Starting QEMU automated testing..."
	@command -v qemu-system-$(QEMU_ARCH) >/dev/null 2>&1 || { echo "QEMU for $(QEMU_ARCH) not found"; exit 1; }
	@if [ -f $(KERNEL_IMAGE) ]; then \
		qemu-system-$(QEMU_ARCH) -kernel $(KERNEL_IMAGE) -m 512M -nographic -serial stdio -no-reboot; \
	else \
		echo "Kernel image not found. Run 'make kernel' first."; \
	fi

qemu-test-interactive: kernel
	@echo "Starting interactive QEMU testing..."
	@command -v qemu-system-$(QEMU_ARCH) >/dev/null 2>&1 || { echo "QEMU for $(QEMU_ARCH) not found"; exit 1; }
	@if [ -f $(KERNEL_IMAGE) ]; then \
		qemu-system-$(QEMU_ARCH) -kernel $(KERNEL_IMAGE) -m 512M -nographic -serial stdio -no-reboot -s -S & \
		echo "QEMU started in debug mode. Connect with gdb:"; \
		echo "  gdb $(KERNEL_DEBUG)"; \
		echo "  target remote localhost:1234"; \
	else \
		echo "Kernel image not found. Run 'make kernel' first."; \
	fi

# Docker container targets
container-build:
	@echo "Building UltraOS container..."
	@command -v docker >/dev/null 2>&1 || { echo "Docker not found"; exit 1; }
	docker build -t ultraos-builder .

container-run: container-build
	@echo "Running UltraOS container..."
	docker run -it -v $(PWD):/workspace ultraos-builder

# Backup source code
backup:
	@echo "Creating source backup..."
	@tar -czf ultraos-src-$(shell date +%Y%m%d-%H%M%S).tar.gz \
		--exclude='build' --exclude='*.o' --exclude='*.a' \
		*.c *.h *.S *.ld Makefile docs examples
	@echo "Backup created"

.DEFAULT_GOAL := all