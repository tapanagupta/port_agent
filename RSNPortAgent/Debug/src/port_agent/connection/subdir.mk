################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-connection.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-instrument_botpt_connection.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-instrument_rsn_connection.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-instrument_serial_connection.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-instrument_tcp_connection.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-observatory_connection.o \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/libport_agent_connection_a-observatory_multi_connection.o 

CXX_SRCS += \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/connection.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_botpt_connection.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_rsn_connection.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_serial_connection.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_tcp_connection.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/observatory_connection.cxx \
/Users/admin/Workspace/src/port_agent/src/port_agent/connection/observatory_multi_connection.cxx 

OBJS += \
./src/port_agent/connection/connection.o \
./src/port_agent/connection/instrument_botpt_connection.o \
./src/port_agent/connection/instrument_rsn_connection.o \
./src/port_agent/connection/instrument_serial_connection.o \
./src/port_agent/connection/instrument_tcp_connection.o \
./src/port_agent/connection/observatory_connection.o \
./src/port_agent/connection/observatory_multi_connection.o 

CXX_DEPS += \
./src/port_agent/connection/connection.d \
./src/port_agent/connection/instrument_botpt_connection.d \
./src/port_agent/connection/instrument_rsn_connection.d \
./src/port_agent/connection/instrument_serial_connection.d \
./src/port_agent/connection/instrument_tcp_connection.d \
./src/port_agent/connection/observatory_connection.d \
./src/port_agent/connection/observatory_multi_connection.d 


# Each subdirectory must supply rules for building sources it contributes
src/port_agent/connection/connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/connection/instrument_botpt_connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_botpt_connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/connection/instrument_rsn_connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_rsn_connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/connection/instrument_serial_connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_serial_connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/connection/instrument_tcp_connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/instrument_tcp_connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/connection/observatory_connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/observatory_connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/port_agent/connection/observatory_multi_connection.o: /Users/admin/Workspace/src/port_agent/src/port_agent/connection/observatory_multi_connection.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


