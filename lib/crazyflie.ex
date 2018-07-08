defmodule Crazyflie do
  @moduledoc """
  Documentation for Crazyflie.
  """

  defstruct link_quality: 0,
            acc_x: 0,
            acc_y: 0,
            acc_z: 0,
            gyro_x: 0,
            gyro_y: 0,
            gyro_z: 0,
            baro_pressure: 0,
            baro_temp: 0,
            mag_x: 0,
            mag_y: 0,
            mag_z: 0,
            pm_vbat: 0,
            params: %{}

  @on_load :load_nif
  @doc false
  def load_nif do
    nif_file = Path.join(:code.priv_dir(:crazyflie), "crazyflie_nif") |> to_charlist()

    case :erlang.load_nif(nif_file, 0) do
      :ok -> :ok
      {:error, {:reload, _}} -> :ok
      {:error, reason} -> {:error, :load_failed, reason}
    end
  end

  def connect(_uri), do: :erlang.nif_error("crazyflie nif not loaded")
  def ping(_copter), do: :erlang.nif_error("crazyflie nif not loaded")
end
