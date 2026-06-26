#define CROW_MAIN
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "../src/Anotador.h"
#include "crow.h"

struct GameState {
  Anotador anotador;
  bool gameStarted = false;
  bool gameFinished = false;
  std::vector<std::string> winners;
  std::string winMethod; // "Dormida", "Alalay", or "Puntos"
  int winningScore = 0;
};

GameState g_game;
std::mutex g_stateMutex;

crow::json::wvalue state_to_json(const GameState &state) {
  crow::json::wvalue x;
  x["gameStarted"] = state.gameStarted;
  x["gameFinished"] = state.gameFinished;

  if (state.gameFinished) {
    crow::json::wvalue::list winners_json;
    for (const auto &w : state.winners)
      winners_json.push_back(w);
    x["winners"] = std::move(winners_json);
    x["winMethod"] = state.winMethod;
    x["winningScore"] = state.winningScore;
  }

  if (!state.gameStarted && !state.gameFinished)
    return x;

  x["currentPlayer"] = state.anotador.jugadorActual;
  x["intento"] = state.anotador.intento;
  x["lanzamiento"] = state.anotador.lanzamiento;
  x["turnoTerminado"] = state.anotador.turnoTerminado;

  // Dice
  crow::json::wvalue::list dados_json;
  for (int d : state.anotador.dados)
    dados_json.push_back(d);
  x["dados"] = std::move(dados_json);

  // Markers
  crow::json::wvalue marcadores_json;
  for (const auto &[name, marker] : state.anotador.marcadores) {
    crow::json::wvalue m;
    for (const auto &[juego, puntos] : marker.puntajes) {
      m[juego] = puntos;
    }
    m["suma"] = marker.suma;
    m["dormida"] = marker.dormida;
    m["alalay"] = marker.tieneAlalay();
    marcadores_json[name] = std::move(m);
  }
  x["marcadores"] = std::move(marcadores_json);

  // Possible Actions
  crow::json::wvalue::list actions_json;
  for (size_t i = 0; i < state.anotador.actuacionesPosibles.size(); ++i) {
    const auto &act = state.anotador.actuacionesPosibles[i];
    crow::json::wvalue a;
    a["id"] = i;
    a["accion"] = act.accion;
    a["juego"] = act.anotacion.juego;
    a["puntos"] = act.anotacion.puntos;

    crow::json::wvalue::list idx_json;
    for (int idx : act.indiceDados)
      idx_json.push_back(idx);
    a["indiceDados"] = std::move(idx_json);
    a["seRespeta"] = act.seRespeta;

    actions_json.push_back(std::move(a));
  }
  x["possibleActions"] = std::move(actions_json);

  if (state.gameFinished) {
    std::vector<std::string> winners;
    int maxScore = -1;
    std::string method = "Puntos";

    // First check for Dormida or Alalay
    for (const auto &p : state.anotador.listaJugadores) {
      const auto &marker = state.anotador.marcadores.at(p);
      if (marker.dormida) {
        winners = {p};
        maxScore = marker.suma;
        method = "Dormida";
        break;
      }
      if (marker.tieneAlalay()) {
        winners = {p};
        maxScore = marker.suma;
        method = "Alalay";
        break;
      }
    }

    // If no instant win, check points
    if (method == "Puntos") {
      for (const auto &p : state.anotador.listaJugadores) {
        int score = state.anotador.marcadores.at(p).suma;
        if (score > maxScore) {
          maxScore = score;
          winners = {p};
        } else if (score == maxScore) {
          winners.push_back(p);
        }
      }
    }

    crow::json::wvalue::list winners_json;
    for (const auto &w : winners)
      winners_json.push_back(w);
    x["winners"] = std::move(winners_json);
    x["winningScore"] = maxScore;
    x["winMethod"] = method;
  }

  return x;
}

std::string read_file(const std::string &path) {
  std::ifstream f(path);
  if (!f)
    return "";
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

int main() {
  crow::SimpleApp app;

  // Static Server Routes
  CROW_ROUTE(app, "/")([]() {
    crow::response res;
    res.set_header("Content-Type", "text/html");
    res.body = read_file("../web/index.html");
    return res;
  });

  CROW_ROUTE(app, "/styles.css")([]() {
    crow::response res;
    res.set_header("Content-Type", "text/css");
    res.body = read_file("../web/styles.css");
    return res;
  });

  CROW_ROUTE(app, "/app.js")([]() {
    crow::response res;
    res.set_header("Content-Type", "application/javascript");
    res.body = read_file("../web/app.js");
    return res;
  });

  // API Routes
  CROW_ROUTE(app, "/start")
      .methods(crow::HTTPMethod::POST,
               crow::HTTPMethod::OPTIONS)([](const crow::request &req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
          crow::response res(204);
          res.add_header("Access-Control-Allow-Origin", "*");
          res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
          res.add_header("Access-Control-Allow-Headers", "Content-Type");
          return res;
        }
        auto x = crow::json::load(req.body);
        int numPlayers = 1;
        if (x && x.has("players")) {
          numPlayers = x["players"].i();
        }
        if (numPlayers < 1)
          numPlayers = 1;
        if (numPlayers > 4)
          numPlayers = 4;

        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_game.anotador = Anotador();
        g_game.winners.clear();
        g_game.winMethod = "";
        g_game.winningScore = 0;
        for (int i = 1; i <= numPlayers; ++i) {
          g_game.anotador.agregarJugador("Jugador" + std::to_string(i));
        }
        g_game.anotador.prepararTurno("Jugador1");
        g_game.gameStarted = true;
        g_game.gameFinished = false;

        crow::json::wvalue body;
        body["status"] = "success";
        body["message"] =
            "Game started with " + std::to_string(numPlayers) + " players";
        crow::response res(200, body);
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
      });

  CROW_ROUTE(app, "/state")
      .methods(crow::HTTPMethod::GET,
               crow::HTTPMethod::OPTIONS)([](const crow::request &req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
          crow::response res(204);
          res.add_header("Access-Control-Allow-Origin", "*");
          res.add_header("Access-Control-Allow-Methods", "GET, OPTIONS");
          res.add_header("Access-Control-Allow-Headers", "Content-Type");
          return res;
        }
        std::lock_guard<std::mutex> lock(g_stateMutex);
        crow::response res;
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        res.body = state_to_json(g_game).dump();
        return res;
      });

  CROW_ROUTE(app, "/play")
      .methods(crow::HTTPMethod::POST,
               crow::HTTPMethod::OPTIONS)([](const crow::request &req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
          crow::response res(204);
          res.add_header("Access-Control-Allow-Origin", "*");
          res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
          res.add_header("Access-Control-Allow-Headers", "Content-Type");
          return res;
        }

        auto x = crow::json::load(req.body);
        if (!x || !x.has("actionId")) {
          crow::response res(400, "Missing actionId");
          res.add_header("Access-Control-Allow-Origin", "*");
          return res;
        }

        int actionId = x["actionId"].i();

        std::lock_guard<std::mutex> lock(g_stateMutex);
        if (!g_game.gameStarted) {
          crow::response res(400, "Game not started");
          res.add_header("Access-Control-Allow-Origin", "*");
          return res;
        }

        if (actionId < 0 ||
            actionId >= (int)g_game.anotador.actuacionesPosibles.size()) {
          crow::response res(400, "Invalid actionId");
          res.add_header("Access-Control-Allow-Origin", "*");
          return res;
        }

        g_game.anotador.ejecutarAccion(actionId);

        // Check for Instant Wins (Dormida or Alalay)
        bool instantWin = false;
        std::string instantWinner = "";
        std::string method = "";

        for (const auto &p : g_game.anotador.listaJugadores) {
          if (g_game.anotador.marcadores[p].dormida) {
            instantWin = true;
            instantWinner = p;
            method = "Dormida";
            break;
          }
          if (g_game.anotador.marcadores[p].tieneAlalay()) {
            instantWin = true;
            instantWinner = p;
            method = "Alalay";
            break;
          }
        }

        if (instantWin) {
          g_game.gameFinished = true;
          g_game.gameStarted = false;
          g_game.winners = {instantWinner};
          g_game.winMethod = method;
          g_game.winningScore = g_game.anotador.marcadores[instantWinner].suma;
        } else {
          // Check if all players finished normally
          bool allFinished = true;
          for (const auto &p : g_game.anotador.listaJugadores) {
            if (!g_game.anotador.esFinDeJuego(p)) {
              allFinished = false;
              break;
            }
          }

          if (allFinished) {
            g_game.gameFinished = true;
            g_game.gameStarted = false;
            g_game.winMethod = "Puntos";

            int maxSuma = -1;
            for (const auto &p : g_game.anotador.listaJugadores) {
              if (g_game.anotador.marcadores[p].suma > maxSuma) {
                maxSuma = g_game.anotador.marcadores[p].suma;
              }
            }
            g_game.winningScore = maxSuma;
            g_game.winners.clear();
            for (const auto &p : g_game.anotador.listaJugadores) {
              if (g_game.anotador.marcadores[p].suma == maxSuma) {
                g_game.winners.push_back(p);
              }
            }
          } else if (g_game.anotador.turnoTerminado) {
            // Switch to next player who hasn't finished
            std::string current = g_game.anotador.jugadorActual;
            size_t startIdx = 0;
            for (size_t i = 0; i < g_game.anotador.listaJugadores.size(); ++i) {
              if (g_game.anotador.listaJugadores[i] == current) {
                startIdx = (i + 1) % g_game.anotador.listaJugadores.size();
                break;
              }
            }

            std::string nextPlayer = "";
            for (size_t i = 0; i < g_game.anotador.listaJugadores.size(); ++i) {
              size_t checkIdx =
                  (startIdx + i) % g_game.anotador.listaJugadores.size();
              std::string p = g_game.anotador.listaJugadores[checkIdx];
              if (!g_game.anotador.esFinDeJuego(p)) {
                nextPlayer = p;
                break;
              }
            }

            if (!nextPlayer.empty()) {
              g_game.anotador.prepararTurno(nextPlayer);
            }
          }
        }

        crow::response res;
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        res.body = state_to_json(g_game).dump();
        return res;
      });

  CROW_ROUTE(app, "/reset")
      .methods(crow::HTTPMethod::POST,
               crow::HTTPMethod::OPTIONS)([](const crow::request &req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
          crow::response res(204);
          res.add_header("Access-Control-Allow-Origin", "*");
          res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
          res.add_header("Access-Control-Allow-Headers", "Content-Type");
          return res;
        }
        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_game.gameStarted = false;
        g_game.gameFinished = false;
        g_game.winners.clear();
        g_game.winMethod = "";
        g_game.winningScore = 0;

        crow::json::wvalue body;
        body["status"] = "success";
        body["message"] = "Game reset successful";
        crow::response res(200, body);
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
      });

  app.port(8080).multithreaded().run();
}
