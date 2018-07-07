defmodule Crazyweb do
  use Supervisor
  alias Crazyweb.{Socket, Router}

  def start_link(args) do
    Supervisor.start_link(__MODULE__, args, name: __MODULE__)
  end

  def init([]) do
    children = [
      {Crazyflie.Server, ["radio://0/80/250k"]},
      Plug.Adapters.Cowboy2.child_spec(
        scheme: :http,
        plug: Router,
        options: [
          dispatch: dispatch(),
          port: 4000
        ]
      )
    ]

    Supervisor.init(children, strategy: :one_for_all)
  end

  defp dispatch do
    [
      {:_,
       [
         {"/ws", Socket, []},
         {:_, Plug.Adapters.Cowboy2.Handler, {Router, []}}
       ]}
    ]
  end
end
