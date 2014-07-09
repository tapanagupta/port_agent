################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/libport_agent_a-port_agent.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/port_agent-port_agent_main.o 

CXX_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/port_agent.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/port_agent_main.cxx 

OBJS += \
./src/port_agent/port_agent.o \
./src/port_agent/port_agent_main.o 

CXX_DEPS += \
./src/port_agent/port_agent.d \
./src/port_agent/port_agent_main.d 


# Each subdirectory must supply rules for building sources it contributes
src/port_agent/port_agent.o: /Users/admin/Workspace/src/port_agent/src/port_agent/port_agent.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/port_agent_main.o: /Users/admin/Workspace/src/port_agent/src/port_agent/port_agent_main.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


