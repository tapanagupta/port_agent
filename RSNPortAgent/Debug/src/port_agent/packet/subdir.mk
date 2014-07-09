################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/libport_agent_packet_a-buffered_single_char.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/libport_agent_packet_a-packet.o 

CXX_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/buffered_single_char.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/packet/packet.cxx 

OBJS += \
./src/port_agent/packet/buffered_single_char.o \
./src/port_agent/packet/packet.o 

CXX_DEPS += \
./src/port_agent/packet/buffered_single_char.d \
./src/port_agent/packet/packet.d 


# Each subdirectory must supply rules for building sources it contributes
src/port_agent/packet/buffered_single_char.o: /Users/admin/Workspace/src/port_agent/src/port_agent/packet/buffered_single_char.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/packet/packet.o: /Users/admin/Workspace/src/port_agent/src/port_agent/packet/packet.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


