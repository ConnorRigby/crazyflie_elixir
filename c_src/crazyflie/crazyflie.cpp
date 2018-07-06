#include <erl_nif.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <ostream>

#include <Crazyflie.h>
#include "crazyflie.hpp"

using namespace std;

static void rt_dtor(ErlNifEnv *env, void *obj) {
    enif_fprintf(stderr, "rt_dtor called\r\n");
    priv_data_t* priv = (priv_data_t*)enif_priv_data(env);
    resource_data_t *rd = (resource_data_t *)obj;
    (void)priv;
    try {
      rd->copter->alloff();
    } catch(const runtime_error& error) {
      enif_fprintf(stderr, "powering copter down failed: %s\r\n", error.what());
    }
}

static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    (void)info; // not used.
    priv_data_t* data = (priv_data_t*)enif_alloc(sizeof(priv_data_t));
    if (data == NULL) return 1;

    data->atom_ok = enif_make_atom(env, "ok");
    data->atom_undefined = enif_make_atom(env, "undefined");
    data->atom_error = enif_make_atom(env, "error");
    data->atom_nil = enif_make_atom(env, "nil");
    data->atom_true = enif_make_atom(env, "true");
    data->atom_false = enif_make_atom(env, "false");
    *priv = (void*)data;
    resource_type = enif_open_resource_type(env, "Elixir.Crazyflie", "crazyflie_nif", &rt_dtor, ERL_NIF_RT_CREATE, NULL);
    return !resource_type;
}

static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    (void)env;  // not used.
    (void)priv; // not used.
    (void)info; // not used.
    return 0;
}

static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info) {
    (void)env;  // not used.
    (void)priv; // not used.
    (void)old_priv; // not used.
    return load(env, priv, info);
}

static void unload(ErlNifEnv* env, void* priv) {
    (void)env;  // not used.
    (void)priv; // not used.
    enif_free(priv);
}

static ERL_NIF_TERM crazyflie_init(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    assert(argc == 1);
    (void)argv; // not used.
    priv_data_t* priv = (priv_data_t*)enif_priv_data(env);
    resource_data_t *rd;
    ErlNifBinary input;

    ERL_NIF_TERM res;
    if(!enif_inspect_binary(env, argv[0], &input))
      return enif_make_badarg(env);

    string uri = string( reinterpret_cast<char const*>(input.data), input.size);
    rd = (resource_data_t*)enif_alloc_resource(resource_type, sizeof(resource_data_t));

    try {
      rd->copter = new Crazyflie(uri);
    } catch(const runtime_error& error) {
      return runtime_error_to_error_tuple(error, env, priv);
    }

    res = enif_make_resource(env, rd);
    enif_release_resource(rd);

    return enif_make_tuple2(env, priv->atom_ok, res);
}

static ERL_NIF_TERM crazyflie_ping(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
  assert(argc == 1);
  resource_data_t *rd;
  priv_data_t* priv = (priv_data_t*)enif_priv_data(env);
  if(!enif_get_resource(env, argv[0], resource_type, (void **)&rd))
    return enif_make_badarg(env);

  rd->copter->sendPing();
  return priv->atom_ok;
}

static ERL_NIF_TERM runtime_error_to_error_tuple(const runtime_error& error, ErlNifEnv *env, priv_data_t* priv) {
  ErlNifBinary error_bin;
  string error_str = error.what();
  const char* error_cstr = error_str.c_str();
  enif_alloc_binary(error_str.size(), &error_bin);
  memcpy(error_bin.data, error_cstr, error_str.size());
  return enif_make_tuple2(env, priv->atom_error, enif_make_binary(env, &error_bin));
}
