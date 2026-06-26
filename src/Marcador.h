#ifndef MARCADOR_H
#define MARCADOR_H

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class Marcador {
public:
  int porAnotar;
  std::map<std::string, int> puntajes;
  bool dormida;
  bool panzaDeOro;
  bool escaleraRespetada;
  bool fullRespetado;
  bool pokerRespetado;
  int suma;

  Marcador() {
    porAnotar = 11;
    std::vector<std::string> juegos = {
        "balas",    "tontos", "trenes", "cuadras", "quinas", "senas",
        "escalera", "full",   "poker",  "grande",  "grande2"};
    for (const auto &j : juegos) {
      puntajes[j] = -1;
    }
    dormida = false;
    panzaDeOro = false;
    escaleraRespetada = false;
    fullRespetado = false;
    pokerRespetado = false;
    suma = 0;
  }

  bool yaAnotado(const std::string &juego) const {
    auto it = puntajes.find(juego);
    if (it != puntajes.end()) {
      return it->second != -1;
    }
    return false;
  }

  void anotar(const std::string &juego, int puntos) {
    if (puntajes.count(juego)) {
      puntajes[juego] = puntos;
      porAnotar--;
      suma += puntos;
    }
  }

  void anotarDormida() { dormida = true; }

  bool marcadorIncompleto() const { return porAnotar > 0; }

  bool tieneAlalay() const {
    bool todasGrandes =
        (puntajes.at("grande") == 50 && puntajes.at("grande2") == 50);
    bool mayoresMano =
        (puntajes.at("escalera") == 25 && puntajes.at("full") == 35 &&
         puntajes.at("poker") == 45);
    bool chicasMano =
        (puntajes.at("balas") == 4 && puntajes.at("tontos") == 8 &&
         puntajes.at("trenes") == 12 && puntajes.at("cuadras") == 16 &&
         puntajes.at("quinas") == 20 && puntajes.at("senas") == 24);
    return todasGrandes && (mayoresMano || chicasMano);
  }

  void display() const {
    for (const auto &[juego, puntos] : puntajes) {
      std::cout << std::left << std::setw(20) << juego << " = " << puntos
                << std::endl;
    }
    std::cout << std::left << std::setw(20) << "dormida" << " = "
              << (dormida ? "True" : "False") << std::endl;
    std::cout << std::left << std::setw(20) << "Panza de oro" << " = "
              << (panzaDeOro ? "True" : "False") << std::endl;
    std::cout << std::left << std::setw(20) << "escaleraRespetada" << " = "
              << (escaleraRespetada ? "True" : "False") << std::endl;
    std::cout << std::left << std::setw(20) << "fullRespetado" << " = "
              << (fullRespetado ? "True" : "False") << std::endl;
    std::cout << std::left << std::setw(20) << "pokerRespetado" << " = "
              << (pokerRespetado ? "True" : "False") << std::endl;
    std::cout << std::left << std::setw(20) << "suma" << " = " << suma
              << std::endl;
  }
};

#endif // MARCADOR_H
