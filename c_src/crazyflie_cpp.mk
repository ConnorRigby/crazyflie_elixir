crazyflie_cpp_SRC_DIR     := $(C_SRC_DIR)/crazyflie_cpp/src
crazyflie_cpp_INCLUDE_DIR := $(C_SRC_DIR)/crazyflie_cpp/include/crazyflie_cpp
crazyflie_cpp_SRC         := $(wildcard $(crazyflie_cpp_SRC_DIR)/*.cpp)
crazyflie_cpp_OBJ         := $(crazyflie_cpp_SRC:.cpp=.o)
crazyflie_cpp             := $(BUILD_DIR)/crazyflie_cpp.a

crazyflie_cpp_CXX_FLAGS ?= -std=c++11 -fPIC -O2
crazyflie_cpp_CXX_FLAGS += $(shell pkg-config libusb)

ifeq ($(MIX_ENV),prod)
else
crazyflie_cpp_CXX_FLAGS += -DDEBUG -g
endif

crazyflie_cpp_LDFLAGS ?= -fPIC -shared -pedantic

ALL   += crazyflie_cpp
CLEAN += crazyflie_cpp_clean
PHONY += crazyflie_cpp crazyflie_cpp_clean

crazyflie_cpp: $(crazyflie_cpp)

crazyflie_cpp_clean:
	$(RM) $(crazyflie_cpp)
	$(RM) $(crazyflie_cpp_OBJ)

$(crazyflie_cpp): $(crazyflie_cpp_OBJ)
	$(AR) rcs $@ $^

$(crazyflie_cpp_SRC_DIR)/%.o: $(crazyflie_cpp_SRC_DIR)/%.cpp
	$(CXX) -I$(crazyflie_cpp_INCLUDE_DIR) -c $(crazyflie_cpp_CXX_FLAGS) -o $@ $<
