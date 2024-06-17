#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// diretiva para setar o valor global do maximo de processos
#define MAX_PROCESS 10

// estrutura da tabela de paginas
// cada entrada na tabela de paginas contem um numero de quadro (frameNumber)
// que indica qual quadro de memoria fisica contém a página correspondente
typedef struct {
    int frameNumber;
} PageTableEntry;

// estrutura de um processo 
typedef struct {
    int processID;
    int processSize;
    int numPages;
    unsigned char *logicalMemory; // Ponteiro para a memoria lógica do processo
    PageTableEntry *pageTable; // Ponteiro para a tabela de páginas do processo
} Process;


typedef struct FrameNode {
    int frameNumber;
    struct FrameNode *next; // ponteiro para o proximo framenode para a lista encadeada
} FrameNode;

unsigned char *physicalMemory;
FrameNode *freeFrames;
int numFrames;
int frameSize;
int physicalMemorySize;
int maxProcessSize;
Process processes[MAX_PROCESS];
int numProcesses = 0;

// configura o tamanho de memória e dos quadros
// aloca e inicializa a memoria fisica
// cria a lista encadeada de quadros livres
void initializeMemory(int memSize, int fSize) {
    physicalMemorySize = memSize;
    frameSize = fSize;
    numFrames = physicalMemorySize / frameSize; // numero de frames = tamanho_memoria_fisica / tamanho_quadro
    physicalMemory = (unsigned char * )malloc(physicalMemorySize);
    memset(physicalMemory, 0, physicalMemorySize);

    // cria um novo nó para cada quadro disponivel
    freeFrames = NULL;
    for (int i=numFrames - 1; i >= 0; i--) {
        FrameNode *node = (FrameNode *)malloc(sizeof(FrameNode)); // aloca memória para um novo nó
        node->frameNumber = i; // atribui o numero do quadro ao nó
        node->next = freeFrames;
        freeFrames = node;
    }
}

// verifica se há quadros livres
// remove um quadro da lista de quadros livres e retorna seu numero 
int allocateFrame() {
    if (freeFrames == NULL) {
        return -1;
    }

    int frameNumber = freeFrames->frameNumber;
    FrameNode *temp = freeFrames;
    freeFrames = freeFrames->next;
    free(temp);
    return frameNumber;
}

void freeFrame(int frameNumber) {
    FrameNode *node = (FrameNode *)malloc(sizeof(FrameNode));
    node->frameNumber = frameNumber;
    node->next = freeFrames;
    freeFrames = node;
}


void createProcess(int processID, int processSize) {
    if (processSize > maxProcessSize) {
        printf("Erro: Tamanho do processo excede o máximo permitido.\n");
        return;
    }

    int numPages = (processSize + frameSize - 1) / frameSize;
    if (numPages > numFrames) {
        printf("Erro: Memória insuficiente para alocar o processo.\n");
        return;
    }

    Process p;
    p.processID = processID;
    p.processSize = processSize;
    p.numPages = numPages;
    p.logicalMemory = (unsigned char *)malloc(processSize);
    p.pageTable = (PageTableEntry *)malloc(numPages * sizeof(PageTableEntry));

    // Inicializar memória lógica com valores aleatórios
    srand(time(NULL));
    for (int i = 0; i < processSize; i++) {
        p.logicalMemory[i] = rand() % 256;
    }

    // Alocar frames e configurar a tabela de páginas
    for (int i = 0; i < numPages; i++) {
        int frameNumber = allocateFrame();
        if (frameNumber == -1) {
            printf("Erro: Memória insuficiente durante a alocação.\n");
            free(p.logicalMemory);
            free(p.pageTable);
            return;
        }
        p.pageTable[i].frameNumber = frameNumber;

        // Copiar dados da memória lógica para a memória física
        int offset = i * frameSize;
        for (int j = 0; j < frameSize && (offset + j) < processSize; j++) {
            physicalMemory[frameNumber * frameSize + j] = p.logicalMemory[offset + j];
        }
    }

    processes[numProcesses++] = p;
    printf("Processo %d criado com sucesso.\n", processID);
}

void displayMemory() {
    int freeFramesCount = 0;
    FrameNode *node = freeFrames;
    while (node != NULL) {
        freeFramesCount++;
        node = node->next;
    }
    printf("Memória livre: %.2f%%\n", (freeFramesCount / (float)numFrames) * 100);

    for (int i = 0; i < numFrames; i++) {
        printf("Frame %d: ", i);
        for (int j = 0; j < frameSize; j++) {
            printf("%02X ", physicalMemory[i * frameSize + j]);
        }
        printf("\n");
    }
}

void displayPageTable(int processID) {
    for (int i = 0; i < numProcesses; i++) {
        if (processes[i].processID == processID) {
            Process p = processes[i];
            printf("Tabela de páginas do processo %d (Tamanho: %d bytes):\n", processID, p.processSize);
            for (int j = 0; j < p.numPages; j++) {
                printf("Página %d -> Quadro %d\n", j, p.pageTable[j].frameNumber);
            }
            return;
        }
    }
    printf("Erro: Processo %d não encontrado.\n", processID);
}

int main() {
    int choice, processID, processSize;

    // Configurar tamanhos
    printf("Digite o tamanho da memória física (em bytes): ");
    scanf("%d", &physicalMemorySize);
    printf("Digite o tamanho do quadro/página (em bytes): ");
    scanf("%d", &frameSize);
    printf("Digite o tamanho máximo de um processo (em bytes): ");
    scanf("%d", &maxProcessSize);

    // Inicializar memória
    initializeMemory(physicalMemorySize, frameSize);

    while (true) {
        printf("\nMenu:\n");
        printf("1. Visualizar memória\n");
        printf("2. Criar processo\n");
        printf("3. Visualizar tabela de páginas\n");
        printf("4. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayMemory();
                break;
            case 2:
                printf("Digite o ID do processo: ");
                scanf("%d", &processID);
                printf("Digite o tamanho do processo (em bytes): ");
                scanf("%d", &processSize);
                createProcess(processID, processSize);
                break;
            case 3:
                printf("Digite o ID do processo: ");
                scanf("%d", &processID);
                displayPageTable(processID);
                break;
            case 4:
                exit(0);
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    }

    return 0;
}