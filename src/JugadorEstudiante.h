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
        JugadorEstudiante(std::string nombre = "JugadorEstudiante") : Jugador(nombre) {
            nombreEstudiante = "Sophia Ignacia Gulppi Tapia"; 
        }

    // Implementación obligatoria de la lógica de juego
        int jugar(const std::map<std::string, Marcador> &marcadores,
            const std::vector<Actuacion> &actuacionesPosibles,
            const std::vector<int> &dados,
            const Anotacion &resultadoPrevio) override {

            //obtener el marcador (tablero)
            const Marcador& miMarcador = marcadores.at(this->nombre);
            
            //ver si obtuvo la dormida (gana inmediatamente)  
            for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                if(actuacionesPosibles[i].accion == "dormida"){
                    return i;
                }
            }
            for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                if(actuacionesPosibles[i].accion == "anotar"){
                    std::string juego = actuacionesPosibles[i].anotacion.juego;
                    int puntaje = actuacionesPosibles[i].anotacion.puntos;
                    if(miMarcador.yaAnotado(juego) == false){
                        if(lanzamiento == 0){
                            if(juego == "grande" || (juego == "poker" && puntaje >= 45)|| (juego == "full" && puntaje >= 35) || (juego == "escalera" && puntaje >= 25)){
                                return i;
                            }
                        }else{
                            if(juego == "grande" || (juego == "poker" && puntaje >= 40) || (juego == "full" && puntaje >= 30) || (juego == "escalera" && puntaje >= 20)){
                                return i;
                            }
                        }
                    }
                }
            }
            //si es que aun nos quedan lanzamientos en el turno, relanzar
            if(lanzamiento < 2){
                int relanzar = -1;
                int dados = -1;
                for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                    if(actuacionesPosibles[i].accion == "lanzar"){
                        int cant_dados = actuacionesPosibles[i].indiceDados.size();
                        if(cant_dados > dados){
                            dados = cant_dados;
                            relanzar = i;
                        }
                    }
                }
                if(relanzar != -1) return relanzar;
            }
            //si es que no quedan mas lanzamientos anotar el mayor puntaje
            int puntaje_max = -1; //
            int index = -1;
            for(size_t i = 0; i < actuacionesPosibles.size(); i++){
                if(actuacionesPosibles[i].accion == "anotar"){
                    std::string juego = actuacionesPosibles[i].anotacion.juego;
                    int puntos = actuacionesPosibles[i].anotacion.puntos;
                    if(miMarcador.yaAnotado(juego) == false){
                        if(puntos > puntaje_max){
                            puntaje_max = puntos;
                            index = i;
                        }
                    }
                }
            }
            if (index != -1) {
                return index;
            } else {
                return 0;
            }
        }
        std::string getNombreEstudiante() const {
            return nombreEstudiante;
        }
};
#endif
