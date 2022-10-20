#include <cstdio>
#include <cstring>
#include <vector>
#include "mpi.h"

#define TAG_ASK_FOR_JOB 1
#define TAG_JOB_DATA 2
#define TAG_STOP 3
#define TAG_RESULT 4

using namespace std;

// Retorna vetores de 2 cidades a partir do numero de cidades passado
vector<vector<int>> get_tasks(int num_cities) {
    vector<vector<int>> tasks;
    for (int i = 1; i < num_cities; i++) {
        for (int j = 1; j < num_cities; j++) {
            if (i != j) {
                vector<int> tuple;
                tuple.push_back(i);
                tuple.push_back(j);
                tasks.push_back(tuple);
            }
        }
    }
    return tasks;
}

int master() {
    printf("[Mestre] Started\n");

    // Saco de tarefas
    vector<vector<int>> bag_of_tasks = get_tasks(4);

    // Variavel que recebe resultados
    int work;

    // Status
    MPI_Status status, status2;

    // Controla slaves
    int slaves_working = 0;
    bool has_slave_alive = false;

    while (!bag_of_tasks.empty() || has_slave_alive) {

        // Aguardando pedido de tarefas
        MPI_Probe(1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int slave_rank = status.MPI_SOURCE;

        // Caso em que o escravo solicitou trabalho
        if (status.MPI_TAG == TAG_ASK_FOR_JOB) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 1, TAG_ASK_FOR_JOB, MPI_COMM_WORLD, &status2);
            printf("[Mestre] Recebi solicitacao de task do escravo %d\n", slave_rank);

            // Se existem tarefas
            if (!bag_of_tasks.empty()) {

                // Busca tarefa
                vector<int> task_to_send = bag_of_tasks[0];
                bag_of_tasks.pop_back();
                int size = task_to_send.size();

                // Envia tarefa
                printf("[Mestre] Mandando task para o escravo %d\n", slave_rank);
                MPI_Send(&task_to_send[0], size, MPI_INT, 1, TAG_JOB_DATA, MPI_COMM_WORLD);

                // Controla Slaves
                slaves_working++;
                has_slave_alive = true;
            } else {
                MPI_Send(&work, 1, MPI_INT, 1, TAG_STOP, MPI_COMM_WORLD);
                if (slaves_working == 0) {
                    has_slave_alive = false;
                }
            }
        }

        // Caso em que o escravo retornou resultado
        if (status.MPI_TAG == TAG_RESULT) {

            // Recebendo o resultado
            MPI_Recv(&work, 1, MPI_INT, 1, TAG_RESULT, MPI_COMM_WORLD, &status2);
            printf("[Mestre] Recebi resultado da task do escravo %d, %d\n", slave_rank, work);
            slaves_working--;
        }
    }

}

int slave() {
    printf("[Escravo] Iniciado\n");

    // Recebe tarefas
    vector<int> task_received;

    MPI_Status status, status2;
    int work;
    int stop = 0;

    do {
        printf("[Escravo] Pedindo tarefa para o mestre\n");
        MPI_Send(&work, 1, MPI_INT, 0, TAG_ASK_FOR_JOB, MPI_COMM_WORLD);

        // Aguardando a resposta do mestre
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Caso em que mestre retornou tarefa
        if (status.MPI_TAG == TAG_JOB_DATA) {
            // Espera 2 cidades
            task_received.resize(2);

            // Recebendo a mensagem
            MPI_Recv(&task_received[0], 2, MPI_INT, 0, TAG_JOB_DATA, MPI_COMM_WORLD, &status2);
            printf("[Escravo] Tarefa recebida: %d\n", task_received[0]);

            // TODO Processar a tarefa
            work = task_received[0] + task_received[1];

            // Envia o resultado para o mestre
            MPI_Send(&work, 1, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
        }

        // Caso em que mestre retornou parada
        if (status.MPI_TAG == TAG_STOP) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 0, TAG_STOP, MPI_COMM_WORLD, &status2);
            printf("[Escravo] Parando de trabalhar\n");
            stop = 1;
        }
    } while (stop == 0);
}

int main(int argc, char **argv) {
    int my_rank;
    int proc_n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    if (my_rank == 0) {
        master();
    } else {
        slave();
    }
    MPI_Finalize();
}