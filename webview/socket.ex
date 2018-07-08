defmodule Crazyweb.Socket do
  @behaviour :cowboy_websocket

  def init(req, state) do
    {:cowboy_websocket, req, state}
  end

  def websocket_init(state) do
    Crazyflie.Server.subscribe("radio://0/80/250k")
    {:ok, %{}}
  end

  def websocket_handle({:text, "ping"}, state) do
    {:ok, state}
  end

  def websocket_handle({:text, _message}, state) do
    {:ok, state}
  end

  def websocket_info({Crazyflie.Server, {"radio://0/80/250k", cf_state}}, state) do
    {:reply, {:text, Jason.encode!(%{kind: :cf_state, data: cf_state})}, state}
  end

  def websocket_info(_, state) do
    {:ok, state}
  end

  def terminate(_reason, _req, _state) do
    :ok
  end
end
