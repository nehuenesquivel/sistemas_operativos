################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: broker

dependents:
	-cd /home/utnso/tp-2020-1c-Skynet/utils/Debug && $(MAKE) all

# Tool invocations
broker: $(OBJS) $(USER_OBJS) /home/utnso/tp-2020-1c-Skynet/utils/Debug/libutils.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2020-1c-Skynet/utils/Debug" -o "broker" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) broker
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/tp-2020-1c-Skynet/utils/Debug/libutils.so:

-include ../makefile.targets
