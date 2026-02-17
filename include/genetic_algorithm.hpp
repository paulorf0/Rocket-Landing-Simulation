#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

inline std::random_device rd;
inline std::mt19937 gen(rd());

template <typename T> using DNA = std::vector<T>;
template <typename T> struct Gene {
  DNA<T> dna;
  double fitness = 0.0;

  bool alive = true;
  int generation = 0;
  int id;
};

template <typename T> using Population = std::vector<Gene<T>>;
template <typename T> using eval = const std::function<double(const DNA<T> &)>;
template <typename T> using gen_rule = const std::function<DNA<T>()>;
template <typename T> using mut_rule = const std::function<void(T &)>;
template <typename T>
using ParentPair = std::pair<const Gene<T> &, const Gene<T> &>;
template <typename T>
using tournament_rule =
    const std::function<ParentPair<T>(const Population<T> &)>;
// tournament_rule <- return parents and receive the population that occur
// tournament.

/*
 * N = Population Size
 * M = DNA size
 */
template <typename T>
Population<T> initial_pop(const int N, const gen_rule<T> &generation_rule) {
  Population<T> pop;

  for (auto i = 0; i < N; i++) {
    Gene<T> gene;
    gene.dna = generation_rule();
    gene.generation = 0;
    gene.id = i;

    pop.push_back(gene);
  }

  return pop;
}

template <typename T> double fitness(Gene<T> &gene, const eval<T> &evaluator) {

  gene.fitness = evaluator(gene.dna);

  return gene.fitness;
}

/*
 * N = Number of parents selected.
 */
template <typename T>
std::vector<Gene<T>> selection(int N, const std::vector<Gene<T>> &pop) {
  if (pop.empty())
    return {};

  Population<T> selected_genes;
  selected_genes.reserve(N);

  double total_fitness = 0.0;

  for (const auto &gene : pop) {
    total_fitness += gene.fitness;
  }

  if (total_fitness <= 0.0) {
    std::cerr << "Warning: Total fitness is zero or negative. Cannot select."
              << std::endl;
    return {};
  }

  std::uniform_real_distribution<> dis(0.0, total_fitness);

  for (auto i = 0; i < N; i++) {
    const auto r = dis(gen);
    double sum = 0;

    for (auto j = 0; j < pop.size(); j++) {
      sum += pop[j].fitness;

      if (r <= sum) {
        selected_genes.push_back(pop[j]);
        break;
      }
    }
  }

  std::sort(
      selected_genes.begin(), selected_genes.end(),
      [](const Gene<T> &a, const Gene<T> &b) { return a.fitness > b.fitness; });

  return selected_genes;
}

template <typename T>
Gene<T> crossover(const Gene<T> &parentA, const Gene<T> &parentB) {
  if (parentA.dna.size() != parentB.dna.size()) {
    std::cerr << "ParentA and ParentB size mismatch" << std::endl;
    return {};
  }

  const auto size = parentA.dna.size();

  Gene<T> child;
  child.generation = parentA.generation + 1;
  child.dna.reserve(size);

  std::uniform_int_distribution<> dis(0, size - 1);

  const auto midpoint = dis(gen);
  for (auto i = 0; i < size; i++) {
    if (i <= midpoint) {
      child.dna.push_back(parentA.dna[i]);
    } else {
      child.dna.push_back(parentB.dna[i]);
    }
  }

  return child;
}

template <typename T>
void mutate(DNA<T> &dna, const mut_rule<T> &rule, double mut) {
  std::uniform_real_distribution<> uniform_dist(0.0, 1.0);

  for (auto i = 0; i < dna.size(); i++) {
    if (uniform_dist(gen) < mut) {
      rule(dna[i]);
    }
  }
}

template <typename T>
void create_next_generation(Population<T> &new_pop,
                            const Population<T> &old_pop,
                            const Population<T> &best_genes,
                            const mut_rule<T> &mutation_rule, double mut,
                            const tournament_rule<T> tm_rule) {
  if (mut < 0 || mut > 1) {
    std::cerr << "Warning: mut is lower that 0 or bigger than 1" << std::endl;
    return;
  }

  const int generation = old_pop[0].generation + 1;

  // pop.size() = 50
  // best.size() = 5
  // fill: 0 - 4
  // missing: best.size() .. pop.size()

  for (auto i = 0; i < best_genes.size(); i++) {
    new_pop[i] = best_genes[i];
    new_pop[i].generation = generation;
    new_pop[i].id = i;
  }

  for (auto i = best_genes.size(); i < old_pop.size(); i++) {
    const auto &[parentA, parentB] = tm_rule(old_pop);
    auto child = crossover(parentA, parentB);

    mutate(child.dna, mutation_rule, mut);

    child.id = i;
    new_pop[i] = child;
  }
}

template <typename T> std::string debug(const Gene<T> &g) {
  std::ostringstream debug;
  std::ostringstream dna;

  if (!g.dna.empty()) {
    dna << '(';
    for (auto i = 0; i < g.dna.size() - 1; i++) {
      dna << "'" << g.dna[i] << "' ";
    }

    dna << "'" << g.dna.back() << "')";
  } else {
    dna << "Nothing";
  }

  debug << "Fitness: " << g.fitness << "\n"
        << "Id: " << g.id << "\n"
        << "Dna: " << dna.str() << "\n";

  return debug.str();
}

namespace Rules {
namespace Tournament {

template <typename T> tournament_rule<T> Tournament_K_best(int K) {
  return [K](const Population<T> &pop) -> ParentPair<T> {
    if (pop.empty())
      throw std::runtime_error(
          "Error: Population is Empty - Tournament_K_best");
    std::uniform_int_distribution<> dis(0, pop.size() - 1);

    auto pick_one = [&]() -> const Gene<T> & {
      int best_idx = dis(gen);

      for (int i = 0; i < K; i++) {
        int idx = dis(gen);
        if (pop[idx].fitness > pop[best_idx].fitness)
          best_idx = idx;
      }

      return pop[best_idx];
    };

    return {pick_one(), pick_one()};
  };
}

} // namespace Tournament

namespace Mutation {}

} // namespace Rules
