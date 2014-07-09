################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/test/basic_packet_test.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/test/buffered_single_char_test.o 

CXX_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/test/basic_packet_test.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/test/buffered_single_char_test.cxx 

OBJS += \
./src/port_agent/packet/test/basic_packet_test.o \
./src/port_agent/packet/test/buffered_single_char_test.o 

CXX_DEPS += \
./src/port_agent/packet/test/basic_packet_test.d \
./src/port_agent/packet/test/buffered_single_char_test.d 


# Each subdirectory must supply rules for building sources it contributes
src/port_agent/packet/test/basic_packet_test.o: /Users/admin/Workspace/src/port_agent/src/port_agent/packet/test/basic_packet_test.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/packet/test/buffered_single_char_test.o: /Users/admin/Workspace/src/port_agent/src/port_agent/packet/test/buffered_single_char_test.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


