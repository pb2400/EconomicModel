#ifndef LUDO_HPP
#define LUDO_HPP

#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <vector>

struct Player {
  int index;
  double skill;
  int money;
  int life;
  int interation;

  Player(double s, int c, int l, int i, int iter);

  void GiveLife(int l);
  void GiveIndex(int i);
  void GiveSkill();
  void GiveOneCoin();
  void Print();
  void operator=(const Player &other) {
    skill = other.skill;
    money = other.money;
    life = other.life;
    index = other.index;
    interation= other.interation;
  }
  // Functor to be used as a sorting policy
  // inside the sorting algotihm in Dynamics method
  struct indexGreater {
    bool operator()(Player a, Player b) const {
      return a.index < b.index; // Ordina in ordine cresente
    }
  };
};

struct indexGreater {
  bool operator()(Player a, Player b) const {
    return a.index < b.index; // Ordina in ordine cresente
  }
};

Player::Player(double s, int c, int l, int i, int iter)
    : skill(s), money(c), life(l), index(i), interation(iter){};

// assegna una vita randomica tra 1 e 100 ai giocatori
void Player::GiveLife(int l) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> int_distr(1, l);
  life = int_distr(gen);
}

// Assegna l'indice ad ogni vettore, utile quando verranno riordinati
void Player::GiveIndex(int i) { index = i; }

// Somma le probabilità entro un range specificato
inline double partialSum(const std::vector<double> &probVector, int start,
                         int end) {
  double sum = 0.;
  for (int i = start; i < end; ++i)
    sum += probVector[i];
  return sum;
}

// Distribuisce le abilità
// void Player::GiveSkill() {
//   std::random_device rd;
//   std::mt19937 gen(rd());
//   std::uniform_real_distribution<double> real_distr(0., 1.);
//   std::vector<double> skillVector{0.15, 0.2, 0.3, 0.2, 0.15};

//   double rand = real_distr(gen);
//   for (int i = 0; i < skillVector.size(); ++i) {
//     if (rand > partialSum(skillVector, 0, i) &&
//         rand <= partialSum(skillVector, 0, i + 1)) {
//       skill = i + 1;
//     }
//   }
// }

void Player::GiveSkill() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> real_distr(0., 1.);
  std::vector<double> skillVector{0.04, 0.08, 0.12, 0.16, 0.2, 0.16, 0.12, 0.08, 0.04};

  double rand = real_distr(gen);
  for (int i = 0; i < skillVector.size(); ++i) {
    if (rand > partialSum(skillVector, 0, i) &&
        rand <= partialSum(skillVector, 0, i + 1)) {
      skill = i + 1;
    }
  }
}

// Da un soldo al giocatore
void Player::GiveOneCoin() { money += 1; }

// Prints Player values on the terminal, useful to check on the whole program
void Player::Print() {
  std::cout << " " << index << " " << std::setprecision(6) << skill << " "
            << money << " " << life << " "
            << "\n";
}

// Metodo utile dentro alla creazione dei tavoli
inline int PoorCounter(std::vector<Player> const &population) {
   int count = 0;
   for (auto &player : population) {
     if (player.money == 0) {
       count += 1;
     }
   }
   return count;
}

// Booleam function used in CurrentTable method
bool PlayerAlreadyInTable(std::vector<Player> currentTable, Player selected) {
  for (auto &player : currentTable) {
    if (selected.index == player.index) {
      return true;
    }
  }
  return false;
}

// Raccoglie i giocatori in tavoli
// Genera un indice randomico tra tutti quelli possibili e
// infila il giocatore scelto dentro a currentTable e poi lo rimuove da
// population, a meno che questo non sia povero o già selezionato.
inline std::vector<Player> CurrentTable(std::vector<Player> &population,
                                        int m) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> int_distr(0, population.size() - 1);
  std::vector<Player> currentTable;

  currentTable.reserve(m);
  for (int i = 0; i < m; ++i) {
    int index = int_distr(gen);
    Player selected = population[index];

    // Il ciclo va avanti fintanto che il giocatore selezionato è povero o
    // uguale ad uno già presente nel tavolo.
    while (PlayerAlreadyInTable(currentTable, selected) ||
           selected.money == 0) {
      index = int_distr(gen);
      selected = population[index];
    }
    currentTable.push_back(selected);
    // Una volta scelto, il giocatore viene rimosso dal vettore popolazione.
    population.erase(population.begin() + index);
  }
  // Gestione delle eccezioni
  for (int i = 0; i < currentTable.size(); ++i) {
    for (int j = 0; j < currentTable.size(); ++j) {
      if (currentTable[i].index == currentTable[j].index && i != j) {
        std::cerr << "Duplicate index in currentTable:\n";
        std::cerr << "Player " << i << ": ";
        currentTable[i].Print();
        std::cerr << "Player " << j << ": ";
        currentTable[j].Print();

        throw std::runtime_error{"A player can't play against himself."};
      }
    }
    if (currentTable[i].money == 0) {
      throw std::runtime_error{"No unfair game, players must be non-poor."};
    }
  }
  return currentTable;
}

// Normalizza il vettore di probabilità a 1
inline void Normalize(std::vector<double> &probVector) {
  double total_prob = 0.;
  for (auto &probs : probVector) {
    total_prob += probs;
  }
  for (auto &prob : probVector) {
    prob /= total_prob;
  }
}

// Condizione booleana per assicurarsi che tutti i contributi della fortuna
// siano compresi tra 0 e 1.
inline bool LuckElementsArentOk(const std::vector<double> &luckVector, double epsilon) {
  for (auto &luck : luckVector) {
    if (luck < - epsilon * 0.04 || luck > epsilon * 0.04) {
      return true;
    }
  }
  return false;
}

// Condizione booleana che controlla che la somma totale della fortuna sia nulla
inline bool TotalLuckIsntNull(const std::vector<double> &luckVector, double epsilon) {
  double total_luck = 0.;
  double tolerance = 1e-6;
  for (auto &luck : luckVector) {
    total_luck += luck;
  }
  if (total_luck > epsilon * tolerance) {
    return true;
  } else {
    return false;
  }
}

// Normalizza il vettore di fortuna a 0
inline void NormalizeLuck(std::vector<double> &luckVector) {
  double total_luck = 0.;
  for (auto &luck : luckVector) {
    total_luck += luck;
  }
  double splitResidue = total_luck / luckVector.size();
  for (auto &luck : luckVector) {
    luck -= splitResidue;
  }
}

// Tronca i contributi della fortuna troppo grandi alla massima magnitudine
// permessa
inline void TruncLucks(std::vector<double> &luckVector) {
  for (auto &luck : luckVector) {
    if (luck < -0.04) {
      luck = -0.04;
    } else if (luck > 0.04) {
      luck = 0.04;
    } else {
    }
  }
}

// Function used in the ProbVector method.
// Generates a set of m random number and a set of random +/- to modify
// the single probabilities, so that their total sum is zero
inline void Luck(std::vector<double> &probVector, double epsilon) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> luck_distr(-0.04, 0.04);
  double randomNum = 0.;
  std::vector<double> luckVector;
  int m = probVector.size();
  luckVector.reserve(m);

  // generate m - 1 random numbers and a last one that
  // nullify the others
  for (int i = 0; i < probVector.size(); ++i) {
    randomNum = luck_distr(gen);
    luckVector.push_back(randomNum);
  }

  do {
    NormalizeLuck(luckVector);
    TruncLucks(luckVector);
  } while (LuckElementsArentOk(luckVector, 1.) ||
           TotalLuckIsntNull(luckVector, 1.));

  for (auto &luck : luckVector) {
    luck *= epsilon;
  }

  // Fa uno shuffle degli elementi di luckVector e modifica
  // gli elementi del vettore delle probabilità.
  std::shuffle(luckVector.begin(), luckVector.end(), gen);
  for (int i = 0; i < probVector.size(); ++i) {
    probVector[i] += luckVector[i];
    // Se il contributo della fortuna dovesse rendere negativa la probabilità,
    // invece la annulla, se la rendesse maggiore di uno allora quel giocatore
    // vince sempre. Poi bisogna rinormlizzare.
    if (probVector[i] < 0.) {
      probVector[i] = 0.;
    }
    if (probVector[i] > 1.) {
      for (auto &prob : probVector) {
        prob = 0.;
      }
      probVector[i] = 1.;
    }
  }
  Normalize(probVector);

  // Gestione delle eccezioni
  // Controlla che la somma totale sia nulla.
  if (TotalLuckIsntNull(luckVector, epsilon)) {
    int count = 0;
    double total_luck = 0.;
    for (auto &luck : luckVector) {
      std::cerr << count << ": " << luck << "\n";
      ++count;
      total_luck += luck;
    }
    std::cerr << "Total luck: " << total_luck << "\n";
    throw std::runtime_error{"Total contribution of luckyness is not null."};
  }

  // Controlla che i contributi della fortuna siano corretti.
  if (LuckElementsArentOk(luckVector, epsilon)) {
    int count = 0;
    for (auto &luck : luckVector) {
      std::cerr << count << ": " << luck << "\n";
      ++count;
    }
    throw std::runtime_error{"Luck contribution are out of the range."};
  }

  // Controlla che i vettori di prob e fortuna siano compatibili-
  if (luckVector.size() != probVector.size()) {
    std::cerr << "luckVector's size: " << luckVector.size() << "\n";
    std::cerr << "probVector's size: " << probVector.size() << "\n";
    throw std::runtime_error{
        "luckVector and probVector don't have the same size."};
  }
}

// Crea il vettore di probabilità
inline std::vector<double> ProbVector(const std::vector<Player> &currentTable) {
  std::vector<double> probVector;
  int m = currentTable.size();
  probVector.reserve(m);

  // Costruisce il vettore di probabilità e lo normalizza
  for (auto &player : currentTable) {
    double playerProb = player.skill;
    probVector.push_back(playerProb);
  }
  Normalize(probVector);
  // Il vettore viene modificato dalla fortuna.
  Luck(probVector, 8.);

  // Gestione delle eccezioni
  double const tolerance = 1e-6;
  double total_prob = 0.;
  for (auto &probs : probVector) {
    total_prob += probs;
  }
  // Controlla che la prob totale sia 1.
  if (std::abs(total_prob - 1.) > tolerance) {
    std::cerr << "total probability: " << total_prob << "\n";
    for (auto &probs : probVector) {
      std::cerr << probs << "\n";
    }
    throw std::runtime_error{"Total probability is not equal to 1."};
  }
  // Controlla che giocatori e probabilità siano compatibili.
  if (currentTable.size() != probVector.size()) {
    throw std::runtime_error{
        "currentTable and probVector don't have the same size."};
  }
  return probVector;
}

// Simula lo scambio probabilistico di denaro
inline void Exchange(std::vector<Player> &currentTable,
                     std::vector<Player> &residualPlayers) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> real_distr(0., 1.);

  bool MatchDidintHappen = true;
  double rand = real_distr(gen);
  int pot = currentTable.size() - 1; // = m
  std::vector<double> probs = ProbVector(currentTable);

  // Cicla sul tavolo
  for (int i = 0; i < currentTable.size(); ++i) {
    // se rand sta nell'intervallo corrispondente all'i-esimo giocatore,
    // questo vince: guadagna m - 1 e tutti gli altri ne perdono 1.
    if (rand > partialSum(probs, 0, i) && rand <= partialSum(probs, 0, i + 1)) {
      currentTable[i].money += pot + 1;
      // cicla su tutti i perdenti e li toglie un soldo
      for (auto &player : currentTable) {
        player.money -= 1;
      }
      // std::cout << "winner: " << currentTable[i].index << "\n";
      MatchDidintHappen = false;
    }
  }
  if (MatchDidintHappen) {
    // Spits out the random number and the probabilities.
    std::cerr << rand << "\n\n";
    for (int k = 0; k < probs.size(); ++k) {
      std::cerr << k << ": " << partialSum(probs, 0, k) << " - "
                << partialSum(probs, 0, k + 1) << "\n";
    }
    throw std::runtime_error{"Match didn't happen."};
  }
  // infila il resto dei giocatori nel vettore dei giocatori residui
  for (auto &player : currentTable) {
    residualPlayers.push_back(player);
  }
}

// It has to store in another container the dead person.
// Then it revives it with 1 or 100 coins.
inline void DeadControl(std::vector<Player> &population,
                        std::vector<Player> &deadPopulation) {
  // Reduce the life of every player that has already participated in the match
  for (int i = 0; i < population.size(); ++i) {
    population[i].life -= 1;
    // Se il giocatore è morto lo infila dentro a un vettore di raccolta
    // e lo resuscita con caratteristiche standard.
    if (population[i].life == 0) {
      deadPopulation.push_back(population[i]);
      // std::sort(deadPopulation.begin(), deadPopulation.end(),
      // indexGreater());
      population[i].life = 1000;
      population[i].money = 600;
      population[i].GiveSkill();
    }
  }
}

// Funzione da usare se prevediamo che i giocatori diventino poveri per
// ricalcolare il numero di tavoli.
inline int CountTables(std::vector<Player> const &population, int m) {
  int pCount = PoorCounter(population);
  int playingPopulation = population.size() - pCount;
  int residue = playingPopulation % m;
  int numbOfTables = (playingPopulation - residue) / m;
  return numbOfTables;
}

// Simula la dinamica tutta combinando i metodi già definiti
inline void Dynamics(std::vector<Player> &population,
                     std::vector<Player> &deadPopulation,
                     std::vector<int> &moneyVector, int m) {
  std::vector<Player> currentTable;
  std::vector<Player> residualPlayers;
  int size = population.size();
  int numbOfTables = 0;
  // Il count serve a regolare la frequenza della partite rispetto a
  // distribuione dei soldi
  int count = 0;

  // Dà una moneta a tutti i giocatori
  for (auto &player : population) {
    player.GiveOneCoin();
  }

  while (count < 4) {
    numbOfTables = CountTables(population, m);
    residualPlayers.clear();
    int totMoney = 0;
    for (int i = 0; i < numbOfTables; ++i) {
      // Ogni volta svuota il vettore currentTable
      // e lo riempe con nuovi giocatori.
      currentTable.clear();
      currentTable = CurrentTable(population, m);
      // Funzione di scambio, modifica vita e monete dei giocatori al tavolo
      // e poi li inserisce dentro a residual players.
      Exchange(currentTable, residualPlayers);
    }

    for (int i = 0; i < residualPlayers.size(); ++i) {
      totMoney += residualPlayers[i].money;
    }
    for (int i = 0; i < population.size(); ++i) {
      totMoney += population[i].money;
    }

    moneyVector.push_back(totMoney);

    for(int i = 0; i < residualPlayers.size(); ++i) {
      residualPlayers[i].interation += 1;
    }
    // Qui riinfila i giocatori nella popolazione.
    // sortati sull'indice.
    for (auto &player : residualPlayers) {
      population.push_back(player);
    }
    if (population.size() != size) {
      throw std::runtime_error{"Population wasn't refilled correctly."};
    }
    // Riordina i giocatori dentro alla popolazione.
    std::sort(population.begin(), population.end(), indexGreater());
    // Gestione dei morti.
    DeadControl(population, deadPopulation);
    ++count;
  }
}

inline void CountMoney(std::vector<int> moneyVector, int nIterations) {
  long int mean = 0;
  for (int i = 0; i < moneyVector.size(); ++i) {
    mean += moneyVector[i];
  }
  long int equilibrium = mean / (moneyVector.size());
   std::cout << "Capital at equilibrium is: " << equilibrium << "\n";
   std::cout << mean << "\n";
   std::cout << moneyVector.size() << "\n";
   std::cout << moneyVector[nIterations*4 - 5] << "\n";
   std::cout << moneyVector[nIterations*4 - 4] << "\n";
   std::cout << moneyVector[nIterations*4 - 3] << "\n";
   std::cout << moneyVector[nIterations*4 - 2] << "\n";
   std::cout << moneyVector[nIterations*4 - 1] << "\n";
}

inline void PrintMoney(std::vector<int> const &moneyVector, std::string fname) {
  std::ofstream output_file(fname);
  if (output_file.is_open()) {
    output_file << "Money, Time\n";
    for(int i = 0; i < moneyVector.size(); ++i) {
      output_file << " " << moneyVector[i] << ", "
                  << i << "\n";
    }
  }
}

// To print out all the dead players with their info.
inline void PlayerPrintOut(std::vector<Player> const &deadPopulation,
                           std::string fname) {
  std::ofstream output_file(fname);
  if (output_file.is_open()) {
    output_file << "Index, Skill, Money, iteration, life\n";
    for (int i = 0; i < deadPopulation.size(); ++i) {
      output_file << " " << deadPopulation[i].index << ", "
                  << std::setprecision(5) << deadPopulation[i].skill << ", "
                  << deadPopulation[i].money << "," << deadPopulation[i].interation << "," << deadPopulation[i].life << "\n";
    }
  }
}

// Stampa su un file i soldi posseduti dai morti insieme alle loro occorrenze
// normalizzate.
inline void PrintOut(std::vector<Player> const &deadPopulation,
                     std::string fname) {
  std::ofstream output_file(fname);
  if (output_file.is_open()) {
    std::map<int, long int> deadMap;
    long int totOccurrence = 0;
    for (unsigned long int i = 1000; i < deadPopulation.size(); ++i) {
      deadMap[deadPopulation[i].money]++;
      totOccurrence++;
    }
    std::cout << "Number of dead people: " << deadPopulation.size() << "\n";
    output_file << "Money,Occurence\n";
    for (int i = 0; i < deadMap.size(); ++i) {
      if (deadMap[i] > 1e-8) {
        double normOcc = static_cast<double> (deadMap[i]) / totOccurrence;
        output_file << " " << i << "," << normOcc << "\n";
      }
    }
  }
}

// Per esprimere la condizione sulle abilità
inline bool NeededSkill(Player player, int s) {
  if (player.skill == s) {
    return true;
  }
  return false;
}

// Stampa su un file i soldi posseduti dai morti insieme alle loro occorrenze
// normalizzate.
// inline void PrintOut_If(std::vector<Player> const &deadPopulation,
//                         std::string fname, int s) {
//   std::ofstream output_file(fname);
//   if (output_file.is_open()) {
//     std::map<int, long int> deadMap;
//     long int totOccurrence = 0;
//     long int count = 0;
//     switch(s) {
//       case 1:
//       for (auto &player : deadPopulation) {
//         if (NeededSkill(player, s)) {
//           if (count > 150) {
//             deadMap[player.money]++;
//             totOccurrence++;
//           }
//           count++;
//         }
//       }
//       break;
//       case 2:
//       for (auto &player : deadPopulation) {
//         if (NeededSkill(player, s)) {
//           if (count > 200) {
//             deadMap[player.money]++;
//             totOccurrence++;
//           }
//           count++;
//         }
//       }
//       break;
//       case 3:
//       for (auto &player : deadPopulation) {
//         if (NeededSkill(player, s)) {
//           if (count > 300) {
//             deadMap[player.money]++;
//             totOccurrence++;
//           }
//           count++;
//         }
//       }
//       break;
//       case 4:
//       for (auto &player : deadPopulation) {
//         if (NeededSkill(player, s)) {
//           if (count > 200) {
//             deadMap[player.money]++;
//             totOccurrence++;
//           }
//           count++;
//         }
//       }
//       break;
//       case 5:
//       for (auto &player : deadPopulation) {
//         if (NeededSkill(player, s)) {
//           if (count > 150) {
//             deadMap[player.money]++;
//             totOccurrence++;
//           }
//           count++;
//         }
//       }
//       break;
//     }
    
//     output_file << "Money,Occurence\n";
//     for (int i = 0; i < deadMap.size(); ++i) {
//       if (deadMap[i] > 1e-8) {
//         double normOcc = static_cast<double>(deadMap[i]) / totOccurrence ;
//         output_file << " " << i << "," << normOcc << "\n";
//       }
//     }
//   }
// }

inline void PrintOut_If(std::vector<Player> const &deadPopulation,
                        std::string fname, int s) {
  std::ofstream output_file(fname);
  if (output_file.is_open()) {
    std::map<int, long int> deadMap;
    long int totOccurrence = 0;
    long int count = 0;
    switch(s) {
      case 1:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 40) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 2:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 80) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 3:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 120) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 4:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 160) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 5:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 200) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 6:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 160) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 7:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 120) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 8:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 80) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
      case 9:
      for (auto &player : deadPopulation) {
        if (NeededSkill(player, s)) {
          if (count > 40) {
            deadMap[player.money]++;
            totOccurrence++;
          }
          count++;
        }
      }
      break;
    }
    
    output_file << "Money,Occurence\n";
    for (int i = 0; i < deadMap.size(); ++i) {
      if (deadMap[i] > 1e-8) {
        double normOcc = static_cast<double>(deadMap[i]) / totOccurrence ;
        output_file << " " << i << "," << normOcc << "\n";
      }
    }
  }
}

#endif

