#ifndef JUGADOR_ESTUDIANTE_H
#define JUGADOR_ESTUDIANTE_H

#include "Jugador.h"
#include "Marcador.h"
#include "Actuacion.h"

#include <vector>
#include <string>
#include <map>

class JugadorEstudiante : public Jugador{
    private:
        std::string nombreEstudiante;

    public:
        JugadorEstudiante() {
            nombreEstudiante = "Sophia Ignacia Gulppi Tapia"; 
        }

    // Implementación obligatoria de la lógica de juego
        int jugar(const std::map<std::string, Marcador> &marcadores,
            const std::vector<Actuacion> &actuacionesPosibles,
            const std::vector<int> &dados,
            const Anotacion &resultadoPrevio) override {

            //ver si obtuvo la dormida (gana inmediatamente)  
            for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                if(actuacionesPosibles[i].accion == "dormida"){
                    return i;
                }
            }
            for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                if(actuacionesPosibles[i].accion == "anotar"){
                    //fin de turno, momento de decidir que y donde anotar el puntaje
                    if(actuacionesPosibles[i].anotacion.juego == "full" && actuacionesPosibles[i].anotacion.puntos > 0||
                    actuacionesPosibles[i].anotacion.juego == "poker" && actuacionesPosibles[i].anotacion.puntos > 0 ||
                    actuacionesPosibles[i].anotacion.juego == "escalera" && actuacionesPosibles[i].anotacion.puntos > 0){
                        return i;
                    }
                }
            }
            //si es que aun nos quedan lanzamientos en el turno, relanzar
            if(lanzamiento < 2){
                for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                    if(actuacionesPosibles[i].accion == "relanzar"){
                        return i;
                    }
                }
            }
            //si es que no quedan mas lanzamientos anotar el mayor puntaje
            int puntaje_max = -1; //
            int index = 0;
            for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                if(actuacionesPosibles[i].accion == "anotar"){
                    if(actuacionesPosibles[i].anotacion.puntos > puntaje_max){
                        puntaje_max = actuacionesPosibles[i].anotacion.puntos;
                        index = i;
                    }
                }
            }
        }
        std::string getNombreEstudiante() const {
            return nombreEstudiante;
        }
};
#endif
