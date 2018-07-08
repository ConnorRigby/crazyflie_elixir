defmodule Crazyflie.Server do
  use GenServer
  require Logger

  def start_link(args \\ ["radio://0/80/250k"]) do
    [uri] = args
    GenServer.start_link(__MODULE__, args, name: name(uri))
  end

  def subscribe(uri \\ "radio://0/80/250k") do
    GenServer.call(name(uri), {:subscribe, self()})
  end

  def init([uri]) do
    {:ok, cf} = Crazyflie.connect(uri)
    {:ok, %{cf: cf, uri: uri, reg: nil, state: %Crazyflie{}}}
  end

  def handle_info(info, %{reg: nil} = state) do
    {:noreply, %{state | state: handle_cf_msg(info, state.state)}}
  end

  def handle_info(info, state) do
    new_state = %{state | state: handle_cf_msg(info, state.state)}
    send(state.reg, {__MODULE__, {state.uri, new_state.state}})
    {:noreply, new_state}
  end

  def handle_call({:subscribe, pid}, _, state) do
    {:reply, :ok, %{state | reg: pid}}
  end

  def handle_cf_msg(
        {:log_imu,
         %{
           acc_x: acc_x,
           acc_y: acc_y,
           acc_z: acc_z,
           gyro_x: gyro_x,
           gyro_y: gyro_y,
           gyro_z: gyro_z,
           timestamp: _
         } = data},
        state
      ) do
    %{
      state
      | acc_x: acc_x,
        acc_y: acc_y,
        acc_z: acc_z,
        gyro_x: gyro_x,
        gyro_y: gyro_y,
        gyro_z: gyro_z
    }
  end

  def handle_cf_msg(
        {:log2,
         %{
           baro_pressure: baro_pressure,
           baro_temp: baro_temp,
           mag_x: mag_x,
           mag_y: mag_y,
           mag_z: mag_z,
           pm_vbat: pm_vbat,
           timestamp: _
         } = data},
        state
      ) do
    %{
      state
      | baro_pressure: baro_pressure,
        baro_temp: baro_temp,
        mag_x: mag_x,
        mag_y: mag_y,
        mag_z: mag_z,
        pm_vbat: pm_vbat
    }
  end

  def handle_cf_msg({:param_toc, group, name, value}, state) do
    new_params = Map.put(state.params[group] || %{}, name, value)
    %{state | params: new_params}
  end

  def handle_cf_msg({:link_quality, quality}, state) do
    %{state | link_quality: quality}
  end

  def handle_cf_msg(info, state) do
    Logger.warn("unhandled_msg: #{inspect(info)}")
    {:noreply, state}
  end

  defp name(uri) do
    :"#{uri}"
  end
end
