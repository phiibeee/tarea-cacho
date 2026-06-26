#ifndef JUGADOR_H
#define JUGADOR_H

#include <string>
#include <vector>
#include <map>
#include "Marcador.h"
#include "Actuacion.h"

class Jugador {
public:
    std::string nombre;
    int intento;
    int turno;
    int lanzamiento;

    Jugador(std::string nom = "generico") 
        : nombre(nom), intento(0), turno(0), lanzamiento(0) {}

    virtual ~Jugador() = default;

    void setNuevoTurno() {
        turno++;
        intento = 0;
    }

    void setNuevoIntento() {
        intento++;
        lanzamiento = 0;
    }

    void setNuevoLanzamiento() {
        lanzamiento++;
    }

    void setSeRespeta() {
        lanzamiento--;
    }

    virtual int jugar(const std::map<std::string, Marcador>& marcadores,
                      const std::vector<Actuacion>& actuacionesPosibles,
                      const std::vector<int>& dados,
                      const Anotacion& resultadoPrevio) = 0;
};

#endif // JUGADOR_H
