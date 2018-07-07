#ifndef Crazyflie_NIF_H
#define Crazyflie_NIF_H

#include <Crazyflie.h>

using namespace std;

typedef struct SharedData {
  Crazyflie* copter;
  ErlNifPid self;
  bool exit;
} shared_data_t;

typedef struct ResourceData {
  ErlNifTid crazyflie_handler_tid;
  shared_data_t* shared;
} resource_data_t;

typedef struct PrivData {
    ERL_NIF_TERM atom_ok;
    ERL_NIF_TERM atom_undefined;
    ERL_NIF_TERM atom_error;
    ERL_NIF_TERM atom_nil;
    ERL_NIF_TERM atom_number;
    ERL_NIF_TERM atom_true;
    ERL_NIF_TERM atom_false;
} priv_data_t;

static ERL_NIF_TERM crazyflie_connect(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM runtime_error_to_error_tuple(const runtime_error& error, ErlNifEnv *env, priv_data_t* priv);

static void rt_dtor(ErlNifEnv *env, void *obj);
static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info);
static void unload(ErlNifEnv* env, void* priv);

static ErlNifResourceType *resource_type;

static ErlNifFunc nif_funcs[] = {
  {"connect", 1, crazyflie_connect, ERL_NIF_DIRTY_JOB_CPU_BOUND},
};

ERL_NIF_INIT(Elixir.Crazyflie, nif_funcs, &load, &reload, &upgrade, &unload)

#endif
