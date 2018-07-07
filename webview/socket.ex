defmodule Crazyweb.Socket do
  @behaviour :cowboy_websocket

  def init(req, state) do
    {:cowboy_websocket, req, state}
  end

  def websocket_init(state) do
    Crazyflie.Server.subscribe()
    IO.puts "new socket"
    {:ok, %{}}
  end

  # def websocket_handle({:text, message}, state) do
  #   json = Jason.decode!(message)
  #   websocket_handle({:json, json}, state)
  # end

  def websocket_handle({:text, _message}, state) do
    {:ok, state}
  end

  def websocket_info({Crazyflie.Server, {"radio://0/80/250k", {kind, data}}}, state) do
    {:reply, {:text, Jason.encode!(%{kind: kind, data: data})}, state}
  end

  def websocket_info(_, state) do
    {:ok, state}
  end

  def terminate(_reason, _req, _state) do
    :ok
  end
end
