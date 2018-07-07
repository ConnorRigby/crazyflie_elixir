defmodule Crazyflie.MixProject do
  use Mix.Project

  def project do
    [
      app: :crazyflie,
      version: "0.1.0",
      elixir: "~> 1.6",
      start_permanent: Mix.env() == :prod,
      aliases: [format: [&format_c/1, "format"]],
      elixirc_paths: elixirc_paths(Mix.env()),
      compilers: [:elixir_make] ++ Mix.compilers(),
      make_error_message: make_error_message(),
      make_clean: ["clean"],
      make_env: make_env(),
      deps: deps()
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger, :eex]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:elixir_make, "~> 0.4.1", runtime: false},
      {:cowboy, "~> 2.0", only: :dev},
      {:plug, "~> 1.0", only: :dev},
      {:jason, "~> 1.1", only: :dev}
    ]
  end

  defp elixirc_paths(:dev) do
    ["./lib", "./webview"]
  end

  defp elixirc_paths(_) do
    ["./lib"]
  end

  defp format_c([]) do
    astyle =
      System.find_executable("astyle") ||
        Mix.raise("""
        Could not format C code since astyle is not available.
        """)

    System.cmd(astyle, ["-n", "-r", "c_src/*.c", "c_src/*.h"], into: IO.stream(:stdio, :line))
  end

  defp format_c(_args), do: true

  defp make_error_message do
    """
    crazyflie failed to compile it's C dependencies. See the above error message
    for what exactly went wrong.
    Make sure you have all of the following installed on your system:
    * GCC
    * GNU Make
    """
  end

  defp make_env,
    do: %{
      "MIX_ENV" => to_string(Mix.env()),
      "BUILD_DIR" => Mix.Project.build_path(),
      "DEPS_DIR" => Mix.Project.deps_path(),
      "C_SRC_DIR" => Path.join(__DIR__, "c_src"),
      "PRIV_DIR" => Path.join(__DIR__, "priv"),
      "ERL_EI_INCLUDE_DIR" =>
        System.get_env("ERL_EI_INCLUDE_DIR") || Path.join([:code.root_dir(), "usr", "include"]),
      "ERL_EI_LIBDIR" =>
        System.get_env("ERL_EI_LIBDIR") || Path.join([:code.root_dir(), "usr", "lib"])
    }
end
