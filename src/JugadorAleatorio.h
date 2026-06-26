#ifndef JUGADORALEATORIO_H
#define JUGADORALEATORIO_H

#include "Jugador.h"
#include <map>
#include <random>
#include <string>
#include <vector>

class JugadorAleatorio : public Jugador {
public:
  JugadorAleatorio(std::string nom = "JugadorAleatorio") : Jugador(nom) {}

  int jugar(const std::map<std::string, Marcador> & /*marcadores*/,
            const std::vector<Actuacion> &actuacionesPosibles,
            const std::vector<int> & /*dados*/,
            const Anotacion & /*resultadoPrevio*/) override {

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, actuacionesPosibles.size() - 1);

    return dist(gen);
  }
};

#endif // JUGADORALEATORIO_H
