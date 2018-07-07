defmodule Crazyweb.Router do
  use Plug.Router

  plug Plug.Static, from: {:crazyflie, "priv/static"}, at: "/"
  plug(:match)
  plug(:dispatch)

  get "/" do
    render(conn, 200, "index", [])
  end

  match _ do
    render(conn, 404, "404", [])
  end

  defp render(conn, code, page, info) do
    info = Keyword.put(info, :assigns, Keyword.put(info[:assigns] || [], :conn, conn))
    data = EEx.eval_file(path(page), info)
    send_resp(conn, code, data)
  end

  @templates_dir Path.join([:code.priv_dir(:crazyflie), "static", "templates"])
  defp path(page) do
    Path.join(@templates_dir, "#{page}.html.eex")
  end
end
