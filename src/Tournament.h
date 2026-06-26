#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "Anotador.h"
#include "Jugador.h"
#include <algorithm>
#include <atomic>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct Stats {
  int wins = 0;    // 1st places
  int seconds = 0; // 2nd places
  int thirds = 0;  // 3rd places
  double tournamentPoints = 0;
  int totalScore = 0;
  int gamesPlayed = 0;
};

#include <functional>

class Tournament {
public:
  using PlayerCreator = std::function<std::shared_ptr<Jugador>()>;
  std::vector<PlayerCreator> creators;
  std::map<std::string, Stats> leaderboard;

  void addPlayer(PlayerCreator pc) {
    auto p = pc();
    creators.push_back(pc);
    leaderboard[p->nombre] = Stats();
  }

  void processGame(Anotador &game, const std::vector<std::string> & /*winners*/,
                   std::mutex &mtx) {
    std::vector<std::string> players = game.listaJugadores;

    // Sort players by score (descending)
    std::sort(players.begin(), players.end(),
              [&](const std::string &a, const std::string &b) {
                return game.marcadores[a].suma > game.marcadores[b].suma;
              });

    std::lock_guard<std::mutex> lock(mtx);

    for (size_t i = 0; i < players.size(); ++i) {
      const std::string &nome = players[i];
      leaderboard[nome].totalScore += game.marcadores[nome].suma;
      leaderboard[nome].gamesPlayed++;

      // Effective rank for 2-1-0 points
      // If tied with previous player, use their rank
      size_t rank = i;
      while (rank > 0 && game.marcadores[players[rank]].suma ==
                             game.marcadores[players[rank - 1]].suma) {
        rank--;
      }

      double pts = 0;
      if (rank == 0) {
        pts = 2;
        leaderboard[nome].wins++;
      } else if (rank == 1) {
        pts = 1;
        leaderboard[nome].seconds++;
      } else {
        leaderboard[nome].thirds++;
      }

      leaderboard[nome].tournamentPoints += pts;
    }
  }

  void run(int gamesPerMatch = 10) {
    std::cout << "Starting Tournament with " << creators.size() << " players!"
              << std::endl;

    for (int i = 0; i < gamesPerMatch; ++i) {
      std::vector<std::shared_ptr<Jugador>> bots;
      for (const auto &pc : creators)
        bots.push_back(pc());

      Anotador game(bots, true);
      auto winners = game.hacerJugar();

      for (const auto &w : winners)
        leaderboard[w].wins++;
      for (const auto &nome : game.listaJugadores) {
        leaderboard[nome].totalScore += game.marcadores[nome].suma;
        leaderboard[nome].gamesPlayed++;
      }
    }
  }

  void runParallel(int gamesPerMatch = 10) {
    std::cout << "Starting Parallel Tournament with " << creators.size()
              << " players (Pooled)!" << std::endl;
    std::mutex mtx;
    std::atomic<int> gamesLeft{gamesPerMatch};
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
      numThreads = 4;

    std::vector<std::thread> workers;
    for (int t = 0; t < numThreads; ++t) {
      workers.emplace_back([&]() {
        while (gamesLeft-- > 0) {
          std::vector<std::shared_ptr<Jugador>> bots;
          for (const auto &pc : creators)
            bots.push_back(pc());

          Anotador game(bots, true);
          auto winners = game.hacerJugar();
          processGame(game, winners, mtx);
        }
      });
    }
    for (auto &w : workers)
      w.join();
  }

  void runCombinatorialTriplets(int repetitions = 1) {
    int numPlayers = creators.size();
    if (numPlayers < 3)
      return;

    struct Triple {
      int i, j, k;
    };
    std::vector<Triple> triples;
    for (int i = 0; i < numPlayers; ++i) {
      for (int j = i + 1; j < numPlayers; ++j) {
        for (int k = j + 1; k < numPlayers; ++k) {
          triples.push_back({i, j, k});
        }
      }
    }

    std::cout << "Starting Combinatorial Triplets Tournament: "
              << triples.size() << " triplets, " << repetitions
              << " repetitions each." << std::endl;

    std::mutex mtx;
    std::atomic<size_t> nextTriple{0};
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
      numThreads = 4;

    std::vector<std::thread> workers;
    for (int t = 0; t < numThreads; ++t) {
      workers.emplace_back([&]() {
        while (true) {
          size_t idx = nextTriple++;
          if (idx >= triples.size())
            break;

          if (idx % 100000 == 0) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Progress: " << idx << " / " << triples.size()
                      << " triplets (" << (idx * 100.0 / triples.size()) << "%)"
                      << std::endl;
          }

          Triple tri = triples[idx];
          for (int r = 0; r < repetitions; ++r) {
            std::vector<std::shared_ptr<Jugador>> bots = {
                creators[tri.i](), creators[tri.j](), creators[tri.k]()};

            Anotador game(bots, true);
            auto winners = game.hacerJugar();
            processGame(game, winners, mtx);
          }
        }
      });
    }

    for (auto &w : workers)
      w.join();
  }

  void displayResults() {
    std::cout << "\n--- TOURNAMENT RESULTS ---" << std::endl;
    std::cout << std::left << std::setw(20) << "Player" << std::setw(12)
              << "T-Points" << std::setw(8) << "1st" << std::setw(8) << "2nd"
              << std::setw(8) << "3rd" << std::setw(15) << "Avg Score"
              << std::endl;

    std::vector<std::pair<std::string, Stats>> sorted;
    for (auto const &[name, stat] : leaderboard) {
      sorted.push_back({name, stat});
    }

    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
      if (a.second.tournamentPoints != b.second.tournamentPoints)
        return a.second.tournamentPoints > b.second.tournamentPoints;
      return a.second.wins > b.second.wins;
    });

    for (const auto &item : sorted) {
      double avg =
          item.second.gamesPlayed > 0
              ? (double)item.second.totalScore / item.second.gamesPlayed
              : 0;
      std::cout << std::left << std::setw(20) << item.first << std::setw(12)
                << std::fixed << std::setprecision(1)
                << item.second.tournamentPoints << std::setw(8)
                << item.second.wins << std::setw(8) << item.second.seconds
                << std::setw(8) << item.second.thirds << std::setw(15)
                << std::fixed << std::setprecision(2) << avg << std::endl;
    }
  }
};

#endif // TOURNAMENT_H
