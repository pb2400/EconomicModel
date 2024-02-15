#include "ludo.hpp"

// Per compilare: g++ ludo_main.cpp per simulare
//                python3 graph.py  per graficare
// Ci sono ulteriori istruzioni dentro a graph.py

///mnt/c/Users/admin/Documents/Ludo_Dataset/1-5/epsilon_2

int main() {

  double skill = 0.;
  int life = 0;
  int money = 600;
  int index = 0;
  int interation = 0;
  int m = 10;                 // number of players at the table
  int N = 1000;               // number of players
  int nIterations = 500000 ; // number of iterations of the game
  Player player(skill, money, life, index, interation);
  std::vector<Player> population(N, player);

  try {
    // Initialize the population
    for (int i = 0; i < population.size(); ++i) {
      population[i].GiveIndex(i);
      population[i].GiveLife(1000);
      population[i].GiveSkill();
    }
    // Proceed with the dynamics
    std::vector<Player> deadPopulation;
    std::vector<int> moneyVector;
    for (int i = 1; i <= nIterations; ++i) {
      // if (nIterations % 100 == 0) {
      std::cout << "Iteration: " << i << "\n";
      
      Dynamics(population, deadPopulation, moneyVector, m);
    }
    CountMoney(moneyVector, nIterations);

    // Prin data on a .csv file
    PrintOut(deadPopulation, "ludo.csv");
    PrintOut_If(deadPopulation, "ludo_1.csv", 1);
    PrintOut_If(deadPopulation, "ludo_2.csv", 2);
    PrintOut_If(deadPopulation, "ludo_3.csv", 3);
    PrintOut_If(deadPopulation, "ludo_4.csv", 4);
    PrintOut_If(deadPopulation, "ludo_5.csv", 5);
    PrintOut_If(deadPopulation, "ludo_6.csv", 6);
    PrintOut_If(deadPopulation, "ludo_7.csv", 7);
    PrintOut_If(deadPopulation, "ludo_8.csv", 8);
    PrintOut_If(deadPopulation, "ludo_9.csv", 9);
    PrintMoney(moneyVector, "capital.csv");
    PlayerPrintOut(deadPopulation, "contollo100002.csv");
  } catch (const std::runtime_error &e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "caught unknown exception\n";
    return 1;
  }
  return 0;
}