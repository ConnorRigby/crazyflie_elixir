crazyflie_NIF_SRC_DIR := $(C_SRC_DIR)/crazyflie
crazyflie_NIF_SRC     := $(wildcard $(crazyflie_NIF_SRC_DIR)/*.cpp)
crazyflie_NIF_OBJ     := $(crazyflie_NIF_SRC:.cpp=.o)
crazyflie_NIF         := $(PRIV_DIR)/crazyflie_nif.so

crazyflie_NIF_CXX_FLAGS ?= -std=c++11 -fPIC -Wall -Wextra -O2
crazyflie_NIF_CXX_FLAGS += -I$(crazyflie_cpp_INCLUDE_DIR)
ifeq ($(MIX_ENV),prod)
else
crazyflie_NIF_CXX_FLAGS += -DDEBUG -g
endif

crazyflie_NIF_LDFLAGS ?= -fPIC -shared -pedantic

ALL   += crazyflie_nif
CLEAN += crazyflie_nif_clean
PHONY += crazyflie_nif crazyflie_nif_clean

crazyflie_nif: $(crazyflie_NIF)

crazyflie_nif_clean:
	$(RM) $(crazyflie_NIF)
	$(RM) $(crazyflie_NIF_OBJ)

$(crazyflie_NIF): $(crazyflie_NIF_OBJ)
	$(CXX) $^ $(BUILD_DIR)/crazyflie_cpp.a $(ERL_LDFLAGS) $(crazyflie_NIF_LDFLAGS) -lusb -o $@

$(crazyflie_NIF_SRC_DIR)/%.o: $(crazyflie_NIF_SRC_DIR)/%.cpp
	$(CXX) -c $(ERL_CFLAGS) $(crazyflie_NIF_CXX_FLAGS) -o $@ $<
