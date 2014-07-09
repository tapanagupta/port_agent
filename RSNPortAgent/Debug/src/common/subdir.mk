################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/Users/admin/Workspace/src/port_agent/src/common/libcommon_a-daemon_process.o \
/Users/admin/Workspace/src/port_agent/src/common/libcommon_a-log_file.o \
/Users/admin/Workspace/src/port_agent/src/common/libcommon_a-logger.o \
/Users/admin/Workspace/src/port_agent/src/common/libcommon_a-spawn_process.o \
/Users/admin/Workspace/src/port_agent/src/common/libcommon_a-timestamp.o \
/Users/admin/Workspace/src/port_agent/src/common/libcommon_a-util.o 

CXX_SRCS += \
/Users/admin/Workspace/src/port_agent/src/common/daemon_process.cxx \
/Users/admin/Workspace/src/port_agent/src/common/log_file.cxx \
/Users/admin/Workspace/src/port_agent/src/common/logger.cxx \
/Users/admin/Workspace/src/port_agent/src/common/spawn_process.cxx \
/Users/admin/Workspace/src/port_agent/src/common/timestamp.cxx \
/Users/admin/Workspace/src/port_agent/src/common/util.cxx 

OBJS += \
./src/common/daemon_process.o \
./src/common/log_file.o \
./src/common/logger.o \
./src/common/spawn_process.o \
./src/common/timestamp.o \
./src/common/util.o 

CXX_DEPS += \
./src/common/daemon_process.d \
./src/common/log_file.d \
./src/common/logger.d \
./src/common/spawn_process.d \
./src/common/timestamp.d \
./src/common/util.d 


# Each subdirectory must supply rules for building sources it contributes
src/common/daemon_process.o: /Users/admin/Workspace/src/port_agent/src/common/daemon_process.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/log_file.o: /Users/admin/Workspace/src/port_agent/src/common/log_file.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/logger.o: /Users/admin/Workspace/src/port_agent/src/common/logger.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/spawn_process.o: /Users/admin/Workspace/src/port_agent/src/common/spawn_process.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/timestamp.o: /Users/admin/Workspace/src/port_agent/src/common/timestamp.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/util.o: /Users/admin/Workspace/src/port_agent/src/common/util.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


