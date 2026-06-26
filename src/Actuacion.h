#ifndef ACTUACION_H
#define ACTUACION_H

#include <string>
#include <vector>
#include <iostream>
#include <utility>

struct Anotacion {
    std::string juego;
    int puntos;
};

class Actuacion {
public:
    std::string accion;
    Anotacion anotacion;
    std::vector<int> indiceDados;
    bool seRespeta;

    Actuacion(std::string acc, Anotacion anot, std::vector<int> idx, bool respeta)
        : accion(acc), anotacion(anot), indiceDados(idx), seRespeta(respeta) {}

    void display() const {
        std::cout << "accion: " << accion << std::endl;
        std::cout << "anotacion: (" << anotacion.juego << ", " << anotacion.puntos << ")" << std::endl;
        std::cout << "indiceDados: [";
        for (size_t i = 0; i < indiceDados.size(); ++i) {
            std::cout << indiceDados[i] << (i == indiceDados.size() - 1 ? "" : ", ");
        }
        std::cout << "]" << std::endl;
        std::cout << "seRespeta: " << (seRespeta ? "True" : "False") << std::endl;
    }

    void displayCompacto(int indice = -1) const {
        if (indice > -1) {
            std::cout << "[" << indice << "]";
        }
        std::cout << "(" << accion << ",('" << anotacion.juego << "', " << anotacion.puntos << "), " 
                  << "[";
        for (size_t i = 0; i < indiceDados.size(); ++i) {
            std::cout << indiceDados[i] << (i == indiceDados.size() - 1 ? "" : ",");
        }
        std::cout << "], " << (seRespeta ? "True" : "False") << ")" << std::endl;
    }
};

#endif // ACTUACION_H
