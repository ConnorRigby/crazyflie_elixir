#include <erl_nif.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <ostream>
#include <semaphore.h>
#include <memory>
#include <math.h>
#include <unistd.h>

#include <Crazyflie.h>
#include "crazyflie.hpp"

using namespace std;

static void rt_dtor(ErlNifEnv *env, void *obj) {
    enif_fprintf(stderr, "rt_dtor called\r\n");
    priv_data_t* priv = (priv_data_t*)enif_priv_data(env);
    resource_data_t *rd = (resource_data_t *)obj;
    (void)priv;

    rd->shared->exit = true;
    enif_thread_join(rd->crazyflie_handler_tid, NULL);
    // delete rd->shared->copter;
    enif_fprintf(stderr, "Thread joined.\r\n");
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

float degToRad(float input) {
  float halfC = M_PI / 180;
  return input * halfC;
}

static int map_put(ErlNifEnv *env, ERL_NIF_TERM map_in, ERL_NIF_TERM* map_out, ERL_NIF_TERM key, ERL_NIF_TERM value) {
  return enif_make_map_put(env, map_in, key, value, map_out);
}

static void *crazyflie_thread(void *arg) {
    enif_fprintf(stderr, "crazyflie_thread started\r\n");
    ErlNifEnv *env = enif_alloc_env();
    shared_data_t* shared = (shared_data_t*)arg;

    shared->copter->logReset();
    shared->copter->requestLogToc();

    ERL_NIF_TERM atom_link_quality = enif_make_atom(env, "link_quality");
    std::function<void(float)> link_quality_callback([env, shared, atom_link_quality](float quality){
      double data = quality;
      ERL_NIF_TERM msg = enif_make_tuple2(env, atom_link_quality, enif_make_double(env, data));
      enif_send(env, &shared->self, NULL, msg);
    });

    std::function<void(const char*)> console_callback([](const char* message){
      enif_fprintf(stderr, "console data: %s\r\n", message);
    });

    shared->copter->setLinkQualityCallback(link_quality_callback);
    shared->copter->setConsoleCallback(console_callback);

    struct logImu {
      float acc_x;
      float acc_y;
      float acc_z;
      float gyro_x;
      float gyro_y;
      float gyro_z;
    } __attribute__((packed));

    struct log2 {
      float mag_x;
      float mag_y;
      float mag_z;
      float baro_temp;
      float baro_pressure;
      float pm_vbat;
    } __attribute__((packed));

    std::function<void(uint32_t, logImu*)> imu_callback([env, shared](uint32_t timestamp, logImu* data) {
      ERL_NIF_TERM timestamp_term = enif_make_uint(env, timestamp);
      ERL_NIF_TERM atom_acc_x = enif_make_atom(env, "acc_x");
      ERL_NIF_TERM atom_acc_y = enif_make_atom(env, "acc_y");
      ERL_NIF_TERM atom_acc_z = enif_make_atom(env, "acc_z");
      ERL_NIF_TERM atom_gyro_x = enif_make_atom(env, "gyro_x");
      ERL_NIF_TERM atom_gyro_y = enif_make_atom(env, "gyro_y");
      ERL_NIF_TERM atom_gyro_z = enif_make_atom(env, "gyro_z");
      ERL_NIF_TERM atom_timestamp = enif_make_atom(env, "timestamp");
      ERL_NIF_TERM atom_logImu = enif_make_atom(env, "log_imu");

      ERL_NIF_TERM map = enif_make_new_map(env);
      map_put(env, map, &map, atom_acc_x, enif_make_double(env, data->acc_x));
      map_put(env, map, &map, atom_acc_y, enif_make_double(env, data->acc_y));
      map_put(env, map, &map, atom_acc_z, enif_make_double(env, data->acc_z));
      map_put(env, map, &map, atom_gyro_x, enif_make_double(env, data->gyro_x));
      map_put(env, map, &map, atom_gyro_y, enif_make_double(env, data->gyro_y));
      map_put(env, map, &map, atom_gyro_z, enif_make_double(env, data->gyro_z));
      map_put(env, map, &map, atom_timestamp, timestamp_term);
      enif_send(env, &shared->self, NULL, enif_make_tuple2(env, atom_logImu, map));
      usleep(10000);

    });

    std::function<void(uint32_t, log2*)> log2_callback([env, shared](uint32_t timestamp, log2* data) {
      ERL_NIF_TERM timestamp_term = enif_make_uint(env, timestamp);
      ERL_NIF_TERM atom_mag_x = enif_make_atom(env, "mag_x");
      ERL_NIF_TERM atom_mag_y = enif_make_atom(env, "mag_y");
      ERL_NIF_TERM atom_mag_z = enif_make_atom(env, "mag_z");
      ERL_NIF_TERM atom_baro_temp = enif_make_atom(env, "baro_temp");
      ERL_NIF_TERM atom_baro_pressure = enif_make_atom(env, "baro_pressure");
      ERL_NIF_TERM atom_pm_vbat = enif_make_atom(env, "pm_vbat");
      ERL_NIF_TERM atom_timestamp = enif_make_atom(env, "timestamp");
      ERL_NIF_TERM atom_log2 = enif_make_atom(env, "log2");

      ERL_NIF_TERM map = enif_make_new_map(env);
      map_put(env, map, &map, atom_mag_x, enif_make_double(env, data->mag_x));
      map_put(env, map, &map, atom_mag_y, enif_make_double(env, data->mag_y));
      map_put(env, map, &map, atom_mag_z, enif_make_double(env, data->mag_z));
      map_put(env, map, &map, atom_baro_temp, enif_make_double(env, data->baro_temp));
      map_put(env, map, &map, atom_baro_pressure, enif_make_double(env, data->baro_pressure));
      map_put(env, map, &map, atom_pm_vbat, enif_make_double(env, data->pm_vbat));
      map_put(env, map, &map, atom_timestamp, timestamp_term);
      enif_send(env, &shared->self, NULL, enif_make_tuple2(env, atom_log2, map));
      usleep(10000);

    });

    std::unique_ptr<LogBlock <logImu>> logBlockImu;
    logBlockImu.reset(new LogBlock<logImu>(
      shared->copter,{
        {"acc", "x"},
        {"acc", "y"},
        {"acc", "z"},
        {"gyro", "x"},
        {"gyro", "y"},
        {"gyro", "z"},
      }, imu_callback));
    logBlockImu->start(1); // 10ms

    std::unique_ptr<LogBlock<log2> > logBlock2;
    logBlock2.reset(new LogBlock<log2>(
      shared->copter,{
        {"mag", "x"},
        {"mag", "y"},
        {"mag", "z"},
        {"baro", "temp"},
        {"baro", "pressure"},
        {"pm", "vbat"},
      }, log2_callback));
    logBlock2->start(10); // 100ms

    shared->copter->requestParamToc();
    for (auto iter = shared->copter->paramsBegin(); iter != shared->copter->paramsEnd(); ++iter) {
      ERL_NIF_TERM param_term;
      auto entry = *iter;
      switch (entry.type) {
        case Crazyflie::ParamTypeUint8: {
          unsigned int data = shared->copter->getParam<uint8_t>(entry.id);
          param_term = enif_make_uint(env, data);
        } break;
        case Crazyflie::ParamTypeInt8: {
          int data = shared->copter->getParam<int8_t>(entry.id);
          param_term = enif_make_int(env, data);
        } break;
        case Crazyflie::ParamTypeUint16: {
          unsigned int data = shared->copter->getParam<uint16_t>(entry.id);
          param_term = enif_make_uint(env, data);
        } break;
        case Crazyflie::ParamTypeInt16: {
          int data = shared->copter->getParam<int16_t>(entry.id);
          param_term = enif_make_int(env, data);
        } break;
        case Crazyflie::ParamTypeUint32: {
          int data = (int)shared->copter->getParam<uint32_t>(entry.id);
          param_term = enif_make_uint(env, data);
        } break;
        case Crazyflie::ParamTypeInt32: {
          int data = shared->copter->getParam<int32_t>(entry.id);
          param_term = enif_make_int(env, data);
        } break;
        case Crazyflie::ParamTypeFloat: {
          double data = shared->copter->getParam<float>(entry.id);
          param_term = enif_make_double(env, data);
        } break;
        default: {
          param_term = enif_make_atom(env, "nil");
        } break;
      }
      ERL_NIF_TERM msg = enif_make_tuple3(env,
        enif_make_atom(env, entry.group.c_str()),
        enif_make_atom(env, entry.name.c_str()),
        param_term);
      enif_send(env, &shared->self, NULL, msg);
    }

    while(shared->exit == false) {
      shared->copter->transmitPackets();
      shared->copter->sendPing();
      std::vector<Crazyradio::Ack> packets = shared->copter->retrieveGenericPackets();
      if (!packets.empty()) {
        std::vector<Crazyradio::Ack>::iterator it;
        for (it = packets.begin(); it != packets.end(); it++) {
          ErlNifBinary bin;
          if(!enif_alloc_binary(it->size, &bin))
            break;

          for(int i = 0; i < it->size; i++) {
            bin.data[i] = it->data[i+1];
          }

          ERL_NIF_TERM msg = enif_make_tuple2(env, enif_make_int(env, it->data[0]), enif_make_binary(env, &bin));
          enif_send(env, &shared->self, NULL, msg);
        }
      }
      // usleep(100000);
    }

    enif_fprintf(stderr, "cleaning up thread\r\n");

    logBlock2.release();
    sleep(2); // i dont know.
    logBlockImu.release();

    delete shared->copter;
    free(shared);

    enif_free_env(env);
    enif_fprintf(stderr, "crazyflie_thread ended\r\n");
    return NULL;
}

static ERL_NIF_TERM crazyflie_connect(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
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
    rd->shared = (shared_data_t*)malloc(sizeof(shared_data_t));
    ErlNifPid self;
    ErlNifPid* self_ptr;
    self_ptr = enif_self(env, &self);
    assert(self_ptr != NULL);

    rd->shared->self = self;
    try {
      rd->shared->copter = new Crazyflie(uri);
    } catch(const runtime_error& error) {
      return runtime_error_to_error_tuple(error, env, priv);
    }

    if (enif_thread_create((char*)"crazyflie_handler", &rd->crazyflie_handler_tid, crazyflie_thread, rd->shared, NULL) != 0) {
        // const char* error = "enif_thread_create failed"
        return priv->atom_error;
    }

    res = enif_make_resource(env, rd);
    enif_release_resource(rd);

    return enif_make_tuple2(env, priv->atom_ok, res);
}

static ERL_NIF_TERM runtime_error_to_error_tuple(const runtime_error& error, ErlNifEnv *env, priv_data_t* priv) {
  ErlNifBinary error_bin;
  string error_str = error.what();
  const char* error_cstr = error_str.c_str();
  enif_alloc_binary(error_str.size(), &error_bin);
  memcpy(error_bin.data, error_cstr, error_str.size());
  return enif_make_tuple2(env, priv->atom_error, enif_make_binary(env, &error_bin));
}
