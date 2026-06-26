#ifndef ANOTADOR_H
#define ANOTADOR_H

#include "Actuacion.h"
#include "Jugador.h"
#include "Marcador.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

class Anotador {
public:
  bool modoCompeticion;
  std::vector<std::string> listaJugadores;
  std::vector<std::string> juegosPosibles;
  std::map<std::string, std::shared_ptr<Jugador>> jugadores;
  std::map<std::string, Marcador> marcadores;
  std::mt19937 gen;

  // New state variables for step-by-step play
  std::vector<int> dados = {0, 0, 0, 0, 0};
  std::vector<int> idxDados = {0, 1, 2, 3, 4};
  std::vector<Actuacion> actuacionesPosibles;
  Anotacion resPrev = {"", -1};
  bool yaAn = false;
  int lanzamiento = 0;
  int intento = 1;
  std::string jugadorActual = "";
  int currentRound = 1;
  bool turnoTerminado = false;

  Anotador() {
    static std::random_device rd;
    gen.seed(rd());
    juegosPosibles = {"balas",  "tontos", "trenes",   "cuadras",
                      "quinas", "senas",  "escalera", "full",
                      "poker",  "grande", "grande2"};
    modoCompeticion = false;
  }

  Anotador(const std::vector<std::shared_ptr<Jugador>> &bots, bool comp = false)
      : modoCompeticion(comp) {
    static std::random_device rd;
    gen.seed(rd());
    juegosPosibles = {"balas",  "tontos", "trenes",   "cuadras",
                      "quinas", "senas",  "escalera", "full",
                      "poker",  "grande", "grande2"};
    for (const auto &bot : bots) {
      agregarJugador(bot->nombre, bot);
    }
  }

  void agregarJugador(const std::string &nome,
                      std::shared_ptr<Jugador> bot = nullptr) {
    listaJugadores.push_back(nome);
    if (bot)
      jugadores[nome] = bot;
    marcadores[nome] = Marcador();
  }

  void prepararTurno(const std::string &nome) {
    jugadorActual = nome;
    lanzamiento = 0;
    intento = 1;
    resPrev = {"", -1};
    yaAn = false;
    idxDados = {0, 1, 2, 3, 4};
    turnoTerminado = false;
    // Removed automatic realizarLanzamiento() to allow explicit first roll
    actualizarActuaciones();
  }

  void realizarLanzamiento() {
    lanzamiento++;
    std::uniform_int_distribution<> dist(1, 6);
    for (int idx : idxDados) {
      dados[idx] = dist(gen);
    }
    actualizarActuaciones();
  }

  void actualizarActuaciones() {
    actuacionesPosibles = calcularActuaciones(
        jugadorActual, intento, lanzamiento, idxDados, dados, resPrev, yaAn);
  }

  void ejecutarAccion(int choice) {
    if (choice < 0 || choice >= (int)actuacionesPosibles.size())
      return;
    Actuacion selected = actuacionesPosibles[choice];

    if (selected.accion == "lanzar") {
      idxDados = selected.indiceDados;
      if (selected.seRespeta) {
        lanzamiento--;
        if (selected.anotacion.juego == "escalera")
          marcadores[jugadorActual].escaleraRespetada = true;
        else if (selected.anotacion.juego == "full")
          marcadores[jugadorActual].fullRespetado = true;
        else if (selected.anotacion.juego == "poker")
          marcadores[jugadorActual].pokerRespetado = true;
      }
      if (lanzamiento < 2) {
        realizarLanzamiento();
      } else {
        // Must score even if lanzar was chosen but max lanzamiento reached
        actualizarActuaciones();
      }
    } else {
      // Score, Tachar, Sleep
      anotar(jugadorActual, selected);
      if (selected.accion == "dormida") {
        turnoTerminado = true;
        return;
      }

      if (intento == 1) {
        if (selected.accion == "anotar" || selected.accion == "tachar") {
          yaAn = true;
        } else if (selected.accion == "sobre") {
          resPrev = selected.anotacion;
        }

        if (marcadores[jugadorActual].marcadorIncompleto() &&
            !marcadores[jugadorActual].tieneAlalay()) {
          // Proceed to Attempt 2
          intento = 2;
          lanzamiento = 0;
          idxDados = {0, 1, 2, 3, 4};
          actualizarActuaciones();
        } else {
          turnoTerminado = true;
        }
      } else {
        turnoTerminado = true;
      }
    }
  }

  std::vector<Actuacion> generarTodoLanzamientoPosible() {
    std::vector<Actuacion> lista;
    for (int i = 1; i < 32; ++i) {
      std::vector<int> indices;
      for (int j = 0; j < 5; ++j) {
        if ((i >> j) & 1)
          indices.push_back(j);
      }
      lista.emplace_back("lanzar", Anotacion{"", -1}, indices, false);
    }
    return lista;
  }

  bool verificarSiYaAnotado(const std::string &juego,
                            const std::string &jogador) {
    if (juego == "grande") {
      return marcadores[jogador].yaAnotado("grande") &&
             marcadores[jogador].yaAnotado("grande2");
    }
    return marcadores[jogador].yaAnotado(juego);
  }

  void anotar(const std::string &jogador, Actuacion act) {
    if (act.accion == "anotar" || act.accion == "tachar" ||
        act.accion == "sobre") {
      std::string juego = act.anotacion.juego;
      size_t pos = juego.find(" (de la 1ra mano)");
      if (pos != std::string::npos)
        juego = juego.substr(0, pos);
      if (juego == "grande" && marcadores[jogador].yaAnotado("grande"))
        juego = "grande2";
      if (act.accion != "sobre")
        marcadores[jogador].anotar(juego, act.anotacion.puntos);
    } else if (act.accion == "dormida") {
      marcadores[jogador].anotarDormida();
    }
  }

  bool esFinDeJuego(const std::string &jogador) {
    if (marcadores[jogador].dormida)
      return true;
    if (marcadores[jogador].tieneAlalay())
      return true;
    if (!marcadores[jogador].marcadorIncompleto())
      return true;
    return false;
  }

  // --- rules logic remains the same ---
  bool verificarEscaleraDeMano(std::vector<int> dList) {
    std::sort(dList.begin(), dList.end());
    int i = (dList[0] == 1 && dList[1] == 3) ? 1 : 0;
    bool ordenados = true;
    for (; i < 4; ++i)
      if (dList[i + 1] != dList[i] + 1) {
        ordenados = false;
        break;
      }
    return ordenados;
  }
  bool verificarFullDeMano(std::vector<int> dList) {
    std::sort(dList.begin(), dList.end());
    return (dList[0] == dList[1] && dList[3] == dList[4] &&
            dList[0] != dList[4]) &&
           (dList[2] == dList[3] || dList[2] == dList[1]);
  }
  bool verificarPokerDeMano(std::vector<int> dList) {
    std::sort(dList.begin(), dList.end());
    return ((dList[0] == dList[3]) && (dList[3] != dList[4])) ||
           ((dList[1] == dList[4]) && (dList[0] != dList[1]));
  }
  bool verificarDormida(const std::vector<int> &dList) {
    return std::all_of(dList.begin(), dList.end(),
                       [&](int x) { return x == dList[0]; });
  }

  std::vector<Actuacion> verificarJugadasDeMano(const std::string &jogador,
                                                const std::vector<int> &dList,
                                                bool puedeSerSobre,
                                                bool esPrimerTiro) {
    std::vector<Actuacion> lista;
    if (verificarEscaleraDeMano(dList)) {
      if (!verificarSiYaAnotado("escalera", jogador)) {
        lista.emplace_back("anotar", Anotacion{"escalera", 25},
                           std::vector<int>{}, false);
        if (puedeSerSobre)
          lista.emplace_back("sobre", Anotacion{"escalera", 25},
                             std::vector<int>{}, false);
      } else if (marcadores[jogador].yaAnotado("escalera") &&
                 !marcadores[jogador].escaleraRespetada && esPrimerTiro) {
        lista.emplace_back("lanzar", Anotacion{"escalera", -1},
                           std::vector<int>{0, 1, 2, 3, 4}, true);
      }
    }
    if (verificarFullDeMano(dList)) {
      if (!verificarSiYaAnotado("full", jogador)) {
        lista.emplace_back("anotar", Anotacion{"full", 35}, std::vector<int>{},
                           false);
        if (puedeSerSobre)
          lista.emplace_back("sobre", Anotacion{"full", 35}, std::vector<int>{},
                             false);
      } else if (marcadores[jogador].yaAnotado("full") &&
                 !marcadores[jogador].fullRespetado && esPrimerTiro) {
        lista.emplace_back("lanzar", Anotacion{"full", -1},
                           std::vector<int>{0, 1, 2, 3, 4}, true);
      }
    }
    if (verificarPokerDeMano(dList)) {
      if (!verificarSiYaAnotado("poker", jogador)) {
        lista.emplace_back("anotar", Anotacion{"poker", 45}, std::vector<int>{},
                           false);
        if (puedeSerSobre)
          lista.emplace_back("sobre", Anotacion{"poker", 45},
                             std::vector<int>{}, false);
      } else if (marcadores[jogador].yaAnotado("poker") &&
                 !marcadores[jogador].pokerRespetado && esPrimerTiro) {
        lista.emplace_back("lanzar", Anotacion{"poker", -1},
                           std::vector<int>{0, 1, 2, 3, 4}, true);
      }
    }
    if (verificarDormida(dList)) {
      lista.emplace_back("dormida", Anotacion{"", -1}, std::vector<int>{},
                         false);
    }
    return lista;
  }

  int verificarSuma(int valor, const std::vector<int> &dList) {
    std::vector<int> d = dList;
    std::sort(d.begin(), d.end());
    if (d[0] == valor && d[4] == valor)
      return 4 * valor;
    int s = 0, vCount = 0;
    for (int v : dList) {
      if (v == valor)
        s += valor;
      else if (v == 7 - valor && vCount < 2) {
        s += valor;
        vCount++;
      }
    }
    return s;
  }
  int verificarSumaSinVuelque(int valor, const std::vector<int> &dList) {
    int s = 0;
    for (int v : dList)
      if (v == valor)
        s += valor;
    return s;
  }

  bool verificarSiBorres(const std::vector<int> &dList,
                         const std::string &jogador) {
    for (int i = 0; i < 5; ++i) {
      std::vector<int> d = dList;
      d[i] = 7 - d[i];
      if (esInutil(d, jogador))
        return true;
      for (int j = i + 1; j < 5; ++j) {
        std::vector<int> d2 = d;
        d2[j] = 7 - d2[j];
        if (esInutil(d2, jogador))
          return true;
      }
    }
    return false;
  }
  bool esInutil(const std::vector<int> &d, const std::string &jogador) {
    int f = 0, c = 0;
    for (int k = 1; k <= 6; ++k) {
      if (!verificarSiYaAnotado(juegosPosibles[k - 1], jogador)) {
        f++;
        if (verificarSumaSinVuelque(k, d) == 0)
          c++;
        else
          return false;
      }
    }
    if (!verificarSiYaAnotado("escalera", jogador)) {
      f++;
      if (!verificarEscaleraDeMano(d))
        c++;
      else
        return false;
    }
    if (!verificarSiYaAnotado("full", jogador)) {
      f++;
      if (!verificarFullDeMano(d))
        c++;
      else
        return false;
    }
    if (!verificarSiYaAnotado("poker", jogador)) {
      f++;
      if (!verificarPokerDeMano(d))
        c++;
      else
        return false;
    }
    if (!verificarSiYaAnotado("grande", jogador)) {
      f++;
      if (!verificarDormida(d))
        c++;
      else
        return false;
    }
    return c == f;
  }

  bool verificarEscaleraDeHuevo(const std::vector<int> &dList) {
    int v3 = std::count(dList.begin(), dList.end(), 3),
        v4 = std::count(dList.begin(), dList.end(), 4);
    if (v3 + v4 != 2)
      return false;
    int v1 = std::count(dList.begin(), dList.end(), 1),
        v6 = std::count(dList.begin(), dList.end(), 6);
    int s = v1 + v6;
    if (s == 0 || s == 3)
      return false;
    if (v3 == 0 || v4 == 0) {
      int v2 = std::count(dList.begin(), dList.end(), 2);
      if ((v1 == 2 || v6 == 2) && v2 == 1)
        return false;
    }
    return true;
  }
  bool verificarFullDeHuevo(const std::vector<int> &dList) {
    for (int i = 1; i <= 2; ++i) {
      int s = std::count(dList.begin(), dList.end(), i) +
              std::count(dList.begin(), dList.end(), 7 - i);
      if (s == 0)
        continue;
      if (s == 5)
        return true;
      if (s == 1 || s == 4)
        return false;
      int s2 = std::count(dList.begin(), dList.end(), i + 1) +
               std::count(dList.begin(), dList.end(), 7 - i - 1);
      if (s2 == 1 || s + s2 == 4)
        return false;
    }
    return true;
  }
  bool verificarPokerDeHuevo(const std::vector<int> &dList) {
    for (int i = 1; i <= 3; ++i)
      if (std::count(dList.begin(), dList.end(), i) +
              std::count(dList.begin(), dList.end(), 7 - i) >=
          4)
        return true;
    return false;
  }
  bool verificarGrande(const std::vector<int> &dList) {
    std::vector<int> d = dList;
    std::sort(d.begin(), d.end());
    if (d[0] == 7 - d[4]) {
      if (d[0] == d[3] || d[1] == d[4])
        return true;
      if ((d[0] == d[2] && d[3] == d[4]) || (d[2] == d[4] && d[0] == d[1]))
        return true;
    }
    return false;
  }

  std::vector<Actuacion>
  verificarJugadasConVuelque(const std::string &jogador,
                             const std::vector<int> &dList,
                             bool puedeSerSobre) {
    std::vector<Actuacion> lista;
    bool inclBorres = verificarSiBorres(dList, jogador);
    for (int i = 1; i <= 6; ++i) {
      if (!verificarSiYaAnotado(juegosPosibles[i - 1], jogador)) {
        int res = verificarSuma(i, dList);
        if (res > 0 || inclBorres) {
          lista.emplace_back("anotar", Anotacion{juegosPosibles[i - 1], res},
                             std::vector<int>{}, false);
          if (puedeSerSobre)
            lista.emplace_back("sobre", Anotacion{juegosPosibles[i - 1], res},
                               std::vector<int>{}, false);
        }
      }
    }
    auto checkGame = [&](const std::string &name,
                         bool (Anotador::*func)(const std::vector<int> &),
                         int pts) {
      if (!verificarSiYaAnotado(name, jogador)) {
        if ((this->*func)(dList)) {
          lista.emplace_back("anotar", Anotacion{name, pts}, std::vector<int>{},
                             false);
          if (puedeSerSobre)
            lista.emplace_back("sobre", Anotacion{name, pts},
                               std::vector<int>{}, false);
        } else if (inclBorres) {
          lista.emplace_back("anotar", Anotacion{name, 0}, std::vector<int>{},
                             false);
          if (puedeSerSobre)
            lista.emplace_back("sobre", Anotacion{name, 0}, std::vector<int>{},
                               false);
        }
      }
    };
    checkGame("escalera", &Anotador::verificarEscaleraDeHuevo, 20);
    checkGame("full", &Anotador::verificarFullDeHuevo, 30);
    checkGame("poker", &Anotador::verificarPokerDeHuevo, 40);
    checkGame("grande", &Anotador::verificarGrande, 50);
    return lista;
  }

  std::vector<Actuacion> calcularActuaciones(const std::string &jogador,
                                             int intento, int lanzamiento,
                                             const std::vector<int> &indices,
                                             const std::vector<int> &dados,
                                             const Anotacion &resPrev,
                                             bool yaAn) {
    std::vector<Actuacion> lista;
    bool puedeSerSobre = (intento == 1);

    if (lanzamiento == 0) {
      // First roll of the turn
      lista.emplace_back("lanzar", Anotacion{"", -1},
                         std::vector<int>{0, 1, 2, 3, 4}, false);
      return lista;
    }

    if (lanzamiento == 1) {
      auto allLanzamientos = generarTodoLanzamientoPosible();
      lista.insert(lista.end(), allLanzamientos.begin(), allLanzamientos.end());
      auto mano = verificarJugadasDeMano(jogador, dados, puedeSerSobre, true);
      lista.insert(lista.end(), mano.begin(), mano.end());
    } else {
      if (intento == 2) {
        if (yaAn)
          lista.emplace_back("nada", Anotacion{"", -1}, std::vector<int>{},
                             false);
        else if (resPrev.puntos != -1) {
          Anotacion resClarified = resPrev;
          resClarified.juego += " (de la 1ra mano)";
          lista.emplace_back("anotar", resClarified, std::vector<int>{}, false);
        }
      }
      if (indices.size() == 5) {
        auto mano =
            verificarJugadasDeMano(jogador, dados, puedeSerSobre, false);
        lista.insert(lista.end(), mano.begin(), mano.end());
      }
      auto vuelque = verificarJugadasConVuelque(jogador, dados, puedeSerSobre);
      lista.insert(lista.end(), vuelque.begin(), vuelque.end());
    }
    return lista;
  }

  // --- Simulation logic ---
  bool ejecutarTurno(const std::string &nome) {
    prepararTurno(nome);
    auto j = jugadores[nome];
    if (!j)
      return false; // Should not happen in simulation if bots are added

    j->setNuevoTurno();
    while (!turnoTerminado) {
      if (lanzamiento == 0) {
        realizarLanzamiento();
      }
      int choice = j->jugar(marcadores, actuacionesPosibles, dados, resPrev);
      ejecutarAccion(choice);
    }
    return marcadores[nome].tieneAlalay();
  }

  std::vector<std::string> hacerJugar() {
    bool alalayODormida = false;
    std::vector<std::string> ganadores;

    while (!alalayODormida) {
      bool allFinished = true;
      for (const auto &nome : listaJugadores) {
        if (!esFinDeJuego(nome)) {
          allFinished = false;
          alalayODormida = ejecutarTurno(nome);
          if (alalayODormida) {
            ganadores.push_back(nome);
            break;
          }
        }
      }
      if (allFinished)
        break;
    }

    if (!alalayODormida) {
      int maxP = -1;
      for (const auto &n : listaJugadores)
        maxP = std::max(maxP, marcadores[n].suma);
      for (const auto &n : listaJugadores)
        if (marcadores[n].suma == maxP)
          ganadores.push_back(n);
    }
    return ganadores;
  }
};

#endif // ANOTADOR_H
