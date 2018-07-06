defmodule Crazyflie do
  @moduledoc """
  Documentation for Crazyflie.
  """

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

  def init(_uri), do: :erlang.nif_error("crazyflie nif not loaded")
  def ping(_copter), do: :erlang.nif_error("crazyflie nif not loaded")
end
