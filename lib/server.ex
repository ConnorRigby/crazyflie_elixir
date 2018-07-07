defmodule Crazyflie.Server do
  use GenServer
  @reg Crazyflie.Registry

  def start_link(uri \\ ["radio://0/80/250k"]) do
    GenServer.start_link(__MODULE__, uri)
  end

  def subscribe(uri \\ "radio://0/80/250k") do
    Elixir.Registry.register(@reg, __MODULE__, self())
  end

  def init([uri]) do
    {:ok, cf} = Crazyflie.connect(uri)
    partitions = 10
    opts = [keys: :duplicate, partitions: partitions, name: @reg]
    {:ok, reg} = Elixir.Registry.start_link(opts)
    {:ok, %{cf: cf, uri: uri, reg: reg}}
  end

  def terminate(reason, _) do
    IO.inspect(reason, label: "Server died")
  end

  def handle_info(info, state) do
    Elixir.Registry.dispatch(@reg, __MODULE__, fn(entries) ->
      for {pid, _} <- entries, do: send(pid, {__MODULE__, {state.uri, info}})
    end)
    {:noreply, state}
  end
end
