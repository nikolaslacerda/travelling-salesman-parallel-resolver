#include<bits/stdc++.h>
#include <ctime>
#include<omp.h>
#include "timer.h"

using namespace std;

const int INF = 1e9;

int get_path_cost(vector<vector<int>> &matrix, vector<int> &path) {
    int cost = 0;
    for (int i = 0; i < path.size() - 1; i++) {
        cost += matrix[path[i]][path[i + 1]];
    }
    return cost;
}

vector<int> search_best_path_with_openMP(vector<vector<int>> &matrix) {

    // Numero de cidades
    int num_nodes = matrix.size() - 1;
    //printf("Nodes number: %d\n", num_nodes);

    // Melhor custo
    int optimal_cost = INF;

    // Caminho com o melhor custo
    vector<int> solution_list;

    //
    vector<int> nodes;

#pragma omp parallel private(nodes) shared(solution_list, optimal_cost) num_threads(4)
    {
        // Thread id
        int id = omp_get_thread_num();

        printf("%d", omp_get_num_threads());

        // Numero de cidades por thread
        int num_nodes_per_thread = (matrix.size() - 1) / omp_get_num_threads();
        printf("Numero de cidades por thread %d\n", num_nodes_per_thread);

        // Se a divisão não for exata
        int rest = (matrix.size() - 1) % omp_get_num_threads();
        printf("Numero de cidades que sobram %d\n", rest);

        // Inicial Node
        int initial_node = (omp_get_thread_num() * num_nodes_per_thread);

        if (id < rest) {
            num_nodes_per_thread += 1;
            initial_node += 1;
        } else {
            initial_node += 1 + rest;
        }

        for (int i = initial_node; i < initial_node + num_nodes_per_thread; i++) {
            printf("=> Inicial node da thread %d: %d\n", id, i);
            nodes.erase(nodes.begin(), nodes.end());
            for (int l = 1; l <= num_nodes; l++)
                if (l != i)
                    nodes.push_back(l);
            do {
                vector<int> possible_solution_list = nodes;
                possible_solution_list.push_back(0);
                possible_solution_list.insert(possible_solution_list.begin(), 0);
                possible_solution_list.insert(possible_solution_list.begin() + 1, i);
                int current_path_weight = get_path_cost(matrix, possible_solution_list);
#pragma omp critical
                {
                    if (current_path_weight < optimal_cost) {
                        optimal_cost = current_path_weight;
                        solution_list = possible_solution_list;
                    }
                }
            } while (next_permutation(nodes.begin(), nodes.end()));
        }
    }
    return solution_list;
}

int main(int argc, char *argv[]) {
    // Cria a matrix de distancia a partir do arquivo passado pelo argumento
    freopen(argv[1], "r", stdin);
    size_t n;
    std::cin >> n;
    // Instancia a matriz
    vector<vector<int>> matrix(n, vector<int>(n));
    // Popula a matriz
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            std::cin >> matrix[i][j];
    // Inicia o algoritmo
    SimpleTimer timer;
    timer.start();
    vector<int> ans = search_best_path_with_openMP(matrix);
    timer.stop();
    double time = timer.elapsedSeconds();
    // Imprime resultados
    int num_nodes = matrix.size();
    printf("Solution: ");
    for (int i = 0; i < num_nodes; i++)
        printf("%d -> ", ans[i]);
    printf("0\n");
    printf("Cost: %d\n", get_path_cost(matrix, ans));
    printf("Time: %f seconds\n", time);
    return 0;
}
