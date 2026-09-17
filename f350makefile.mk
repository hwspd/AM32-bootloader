MCU := F350
PART := GD32F350

HAL_FOLDER_$(MCU) := $(HAL_FOLDER)/$(call lc,$(MCU))

MCU_$(MCU) := -mfloat-abi=soft -mthumb -mcpu=cortex-m4

SRC_BASE_DIR_$(MCU) := \
	$(HAL_FOLDER_$(MCU))/Drivers/GD32F3x0_standard_peripheral/Source \
	$(HAL_FOLDER_$(MCU))/Startup

CFLAGS_$(MCU) += \
	-I$(HAL_FOLDER_$(MCU))/Inc \
	-I$(HAL_FOLDER_$(MCU))/Drivers/CMSIS/Include \
	-I$(HAL_FOLDER_$(MCU))/Drivers/CMSIS/Core/Include \
	-I$(HAL_FOLDER_$(MCU))/Drivers/GD32F3x0_standard_peripheral/Include

CFLAGS_$(MCU) += \
	-DGD32$(MCU) \
	-D$(PART) \
	-DUSE_STDPERIPH_DRIVER \
	-DRAM_LIMIT_KB=8

SRC_$(MCU)_BL := $(foreach dir,$(SRC_BASE_DIR_$(MCU)),$(wildcard $(dir)/*.[cs])) \
	$(wildcard $(HAL_FOLDER_$(MCU))/Src/*.c)

# REF_F350 uses PB4 for its one-wire bootloader and DShot input.
BOOTLOADER_PINS_$(MCU) := PB4
