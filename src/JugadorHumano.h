#ifndef JUGADORHUMANO_H
#define JUGADORHUMANO_H

#include "Jugador.h"
#include <iostream>
#include <limits>

class JugadorHumano : public Jugador {
public:
  JugadorHumano(std::string nom = "Humano") : Jugador(nom) {}

  int jugar(const std::map<std::string, Marcador> & /*marcadores*/,
            const std::vector<Actuacion> &actuacionesPosibles,
            const std::vector<int> &dados,
            const Anotacion & /*resultadoPrevio*/) override {

    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string GREEN = "\033[32m";
    const std::string BLUE = "\033[34m";
    const std::string RED = "\033[31m";
    const std::string YELLOW = "\033[33m";

    std::cout << "\n"
              << BOLD << "========================================" << RESET
              << std::endl;
    std::cout << BOLD << "TURNO DE: " << GREEN << nombre << RESET << std::endl;
    std::cout << BOLD << "Dados actuales: " << YELLOW;
    for (int d : dados)
      std::cout << "[" << d << "] ";
    std::cout << RESET << std::endl;

    std::cout << "         Index: ";
    for (int i = 0; i < 5; ++i)
      std::cout << " (" << i << ") ";
    std::cout << std::endl;

    std::cout << BOLD << "Intento: " << intento
              << " | Lanzamiento: " << lanzamiento << RESET << std::endl;
    std::cout << BOLD << "========================================" << RESET
              << std::endl;

    // Categorize options
    std::vector<int> scoringIdx;
    std::vector<int> rerollIdx;
    std::vector<int> otherIdx;

    for (size_t i = 0; i < actuacionesPosibles.size(); ++i) {
      const std::string &acc = actuacionesPosibles[i].accion;
      if (acc == "anotar" || acc == "tachar" || acc == "dormida" ||
          acc == "sobre") {
        scoringIdx.push_back(i);
      } else if (acc == "lanzar") {
        rerollIdx.push_back(i);
      } else {
        otherIdx.push_back(i);
      }
    }

    if (!scoringIdx.empty()) {
      std::cout << BOLD << BLUE << "\n--- OPCIONES DE ANOTACIÓN ---" << RESET
                << std::endl;
      for (int i : scoringIdx) {
        const auto &a = actuacionesPosibles[i];
        if (a.accion == "dormida") {
          std::cout << "[" << i << "] " << GREEN << BOLD
                    << "DORMIDA! (Gana el juego)" << RESET << std::endl;
        } else if (a.accion == "sobre") {
          std::cout << "[" << i << "] " << BLUE << BOLD
                    << "RESERVAR (Sobre): " << RESET << BOLD
                    << a.anotacion.juego << RESET << " (+" << a.anotacion.puntos
                    << " pts)" << std::endl;
        } else {
          bool esTachar = (a.anotacion.puntos == 0);
          std::cout << "[" << i << "] " << (esTachar ? RED : GREEN)
                    << (esTachar ? "Tachar " : "Anotar ") << BOLD
                    << a.anotacion.juego << RESET << " (+" << a.anotacion.puntos
                    << " pts)" << std::endl;
        }
      }
    }

    if (!rerollIdx.empty()) {
      std::cout << BOLD << YELLOW << "\n--- OPCIONES DE RELANZAMIENTO ---"
                << RESET << std::endl;
      for (int i : rerollIdx) {
        const auto &a = actuacionesPosibles[i];
        std::cout << "[" << i << "] " << BLUE << "Relanzar índices: " << RESET
                  << "[ ";
        for (int d : a.indiceDados)
          std::cout << d << " ";
        std::cout << "]" << (a.seRespeta ? GREEN + " (Respetando)" + RESET : "")
                  << std::endl;
      }
    }

    if (!otherIdx.empty()) {
      std::cout << BOLD << RED << "\n--- OTRAS OPCIONES ---" << RESET
                << std::endl;
      for (int i : otherIdx) {
        const auto &a = actuacionesPosibles[i];
        if (a.accion == "nada") {
          std::cout << "[" << i << "] " << RED << BOLD
                    << "NO HACER NADA (Pasar)" << RESET << std::endl;
        } else {
          std::cout << "[" << i << "] " << a.accion << std::endl;
        }
      }
    }

    int choice = -1;
    while (true) {
      std::cout << BOLD << "\nAcción elegida (0-"
                << actuacionesPosibles.size() - 1 << "): " << RESET;
      if (std::cin >> choice && choice >= 0 &&
          choice < (int)actuacionesPosibles.size()) {
        break;
      } else {
        std::cout << RED << "Opción inválida. Intenta de nuevo." << RESET
                  << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
    }

    return choice;
  }
};

#endif // JUGADORHUMANO_H
