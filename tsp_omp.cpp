#include <cstdio>
#include <cstring>
#include <vector>
#include "mpi.h"

#define TAG_ASK_FOR_JOB 1
#define TAG_JOB_DATA 2
#define TAG_STOP 3
#define TAG_RESULT 4

using namespace std;

int master() {
    printf("[Mestre] Started\n");
    int work;
    MPI_Status status, status2;
    int jobs = 4;
    int slaves_working = 0;
    bool has_slave_alive = false;

    while (jobs > 0 || has_slave_alive) {

        // Aguardando pedido de tarefas
        MPI_Probe(1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int slave_rank = status.MPI_SOURCE;

        // Caso em que o escravo solicitou trabalho
        if (status.MPI_TAG == TAG_ASK_FOR_JOB) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 1, TAG_ASK_FOR_JOB, MPI_COMM_WORLD, &status2);
            printf("[Mestre] Recebi solicitacao de task do escravo %d\n", slave_rank);
            
            // Se existem tarefas
            if (jobs > 0) {
                printf("[Mestre] Mandando task %d para o escravo %d\n", jobs, slave_rank);
                MPI_Send(&jobs, 1, MPI_INT, 1, TAG_JOB_DATA, MPI_COMM_WORLD);
                slaves_working++;
                has_slave_alive = true;
                jobs--;
            } else {
                MPI_Send(&jobs, 1, MPI_INT, 1, TAG_STOP, MPI_COMM_WORLD);
                if (slaves_working == 0) {
                    has_slave_alive = false;
                }
            }
        }

        // Caso em que o escravo retornou resultado
        if (status.MPI_TAG == TAG_RESULT) {

            // Recebendo o resultado
            MPI_Recv(&work, 1, MPI_INT, 1, TAG_RESULT, MPI_COMM_WORLD, &status2);
            printf("[Mestre] Recebi resultado da task do escravo %d\n", slave_rank);
            slaves_working--;
        }
    }

}

int slave() {
    printf("[Escravo] Iniciado\n");
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

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 0, TAG_JOB_DATA, MPI_COMM_WORLD, &status2);
            printf("[Escravo] Tarefa recebida: %d\n", work);

            // TODO Processar a tarefa
            work = work + 1;

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


// Retorna vetores de 2 cidades a partir do numero de cidades passado
vector<vector<int>> get_tasks(int num_cities) {
    vector<vector<int>> test;
    for (int i = 1; i < num_cities; i++) {
        for (int j = 1; j < num_cities; j++) {
            if (i != j) {
                vector<int> g1;
                g1.push_back(i);
                g1.push_back(j);
                test.push_back(g1);
            }
        }
    }
    return test;
}
