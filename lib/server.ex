defmodule Crazyflie.Server do
  use GenServer

  def start_link(args \\ ["radio://0/80/250k"]) do
    [uri] = args
    GenServer.start_link(__MODULE__, args, name: name(uri))
  end

  def subscribe(uri \\ "radio://0/80/250k") do
    GenServer.call(name(uri), {:subscribe, self()})
  end

  def init([uri]) do
    {:ok, cf} = Crazyflie.connect(uri)
    {:ok, %{cf: cf, uri: uri, reg: nil}}
  end

  def terminate(reason, _) do
    IO.inspect(reason, label: "Server died")
  end

  def handle_info(_info, %{reg: nil} = state) do
    {:noreply, state}
  end

  def handle_info(info, state) do
    send(state.reg, {__MODULE__, {state.uri, info}})
    {:noreply, state}
  end

  def handle_call({:subscribe, pid}, _, state) do
    {:reply, :ok, %{state | reg: pid}}
  end

  defp name(uri) do
    :"#{uri}"
  end
end
