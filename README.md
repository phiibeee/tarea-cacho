# Cacho-CPP

A standalone C++ implementation of the Cacho game engine (Alalay rules), including a Web API and a Web Client.

## Repository Structure

- `src/`: Core engine headers and standalone simulation runner.
- `api/`: Web API server using the Crow framework.
- `web/`: Frontend web interface.

## 🚀 Getting Started

### Prerequisites

- `g++` (C++17 support)
- `make`
- `boost-system`, `boost-thread` (required for the Web API)

### Building the Project

You can build both the core simulation and the Web API from the root directory:

```bash
make
```

Alternatively, you can build them individually:

```bash
# To build the standalone simulation
cd src
make

# To build the Web API
cd api
make
```

## 🎲 Running simulations

The `src/cacho` executable runs a game between automated players (Random).

```bash
cd src
./cacho
```

## 🌐 Running the Web API

The `api/cacho_api` executable starts a server on `http://localhost:8080`.

```bash
cd api
./cacho_api
```

Once running, open [http://localhost:8080](http://localhost:8080) in your browser to play.

## 🛠 Features

- **Decoupled Engine**: Pure C++ implementation with no external dependencies (except for Crow/Boost in the API).
- **Alalay Rules**: Full support for Alalay tournament rules.
- **Web Interface**: Modern web client for human play.
- **Random Player**: Included for testing and simulation.
- **Human Player**: Base class and logic for human interaction.
