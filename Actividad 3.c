/**
 * Simulador de Administración de Memoria
 * 
 * Este programa simula un sistema de administración de memoria que permite:
 * - Administrar particiones fijas y dinámicas
 * - Simular carga y liberación de procesos
 * - Calcular fragmentación interna y externa
 * - Simular compactación de memoria
 */


 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdbool.h>
 
#define MAX_PROCESS_NAME 20
#define DEFAULT_MEMORY_SIZE 64  // Tamaño de memoria en MB

// Tipos de particiones
typedef enum {
    FIXED_PARTITIONS,
    DYNAMIC_PARTITIONS
} PartitionType;

// Estado de un bloque de memoria
typedef enum {
    FREE,
    OCCUPIED
} BlockStatus;

// Estructura para representar un bloque de memoria
typedef struct MemoryBlock {
    int id;                     // ID del bloque
    int start_address;          // Dirección de inicio
    int size;                   // Tamaño en MB
    BlockStatus status;         // Estado (FREE u OCCUPIED)
    char process_name[MAX_PROCESS_NAME]; // Nombre del proceso que ocupa el bloque
    struct MemoryBlock* next;   // Puntero al siguiente bloque
} MemoryBlock;

// Estructura principal para la simulación
typedef struct {
    MemoryBlock* memory_map;    // Lista enlazada de bloques de memoria
    int total_memory;           // Tamaño total de la memoria en MB
    int available_memory;       // Memoria disponible en MB
    PartitionType partition_type; // Tipo de partición actual
    int num_fixed_partitions;   // Número de particiones fijas (si se usa ese modo)
    int fixed_partition_size;   // Tamaño de cada partición fija
} MemorySimulator;

// Prototipos de funciones
MemorySimulator* initSimulator(int memory_size);
void setPartitionMode(MemorySimulator* sim, PartitionType type, int num_partitions);
void displayMemoryMap(MemorySimulator* sim);
bool allocateProcess(MemorySimulator* sim, char* process_name, int process_size);
bool deallocateProcess(MemorySimulator* sim, char* process_name);
void calculateFragmentation(MemorySimulator* sim, int* internal_frag, int* external_frag);
void compactMemory(MemorySimulator* sim);
void freeSimulator(MemorySimulator* sim);
void displayMenu();

int main() {
    int memory_size = DEFAULT_MEMORY_SIZE;
    MemorySimulator* simulator = initSimulator(memory_size);
    
    int choice;
    bool running = true;
    
    while (running) {
        displayMenu();
        printf("\nIngrese su opción: ");
        scanf("%d", &choice);
        getchar(); // Consumir el carácter de nueva línea
        
        switch (choice) {
            case 1: {
                int partition_type;
                printf("\nSeleccione el tipo de partición:\n");
                printf("1. Particiones Fijas\n");
                printf("2. Particiones Dinámicas\n");
                printf("Ingrese su opción: ");
                scanf("%d", &partition_type);
                
                if (partition_type == 1) {
                    int num_partitions;
                    printf("Ingrese el número de particiones fijas: ");
                    scanf("%d", &num_partitions);
                    setPartitionMode(simulator, FIXED_PARTITIONS, num_partitions);
                } else if (partition_type == 2) {
                    setPartitionMode(simulator, DYNAMIC_PARTITIONS, 0);
                } else {
                    printf("Opción inválida.\n");
                }
                break;
            }
            case 2: {
                char process_name[MAX_PROCESS_NAME];
                int process_size;
                
                printf("\nIngrese el nombre del proceso: ");
                scanf("%s", process_name);
                printf("Ingrese el tamaño del proceso (en MB): ");
                scanf("%d", &process_size);
                
                if (allocateProcess(simulator, process_name, process_size)) {
                    printf("\nProceso '%s' cargado exitosamente.\n", process_name);
                } else {
                    printf("\nNo se pudo cargar el proceso '%s'. Memoria insuficiente.\n", process_name);
                }
                
                displayMemoryMap(simulator);
                break;
            }
            case 3: {
                char process_name[MAX_PROCESS_NAME];
                
                printf("\nIngrese el nombre del proceso a liberar: ");
                scanf("%s", process_name);
                
                if (deallocateProcess(simulator, process_name)) {
                    printf("\nProceso '%s' liberado exitosamente.\n", process_name);
                } else {
                    printf("\nNo se encontró el proceso '%s'.\n", process_name);
                }
                
                displayMemoryMap(simulator);
                break;
            }
            case 4: {
                int internal_frag, external_frag;
                calculateFragmentation(simulator, &internal_frag, &external_frag);
                
                printf("\nFragmentación:\n");
                printf("- Fragmentación interna: %d MB\n", internal_frag);
                printf("- Fragmentación externa: %d MB\n", external_frag);
                break;
            }
            case 5: {
                printf("\nEstado de la memoria antes de la compactación:\n");
                displayMemoryMap(simulator);
                
                compactMemory(simulator);
                
                printf("\nEstado de la memoria después de la compactación:\n");
                displayMemoryMap(simulator);
                break;
            }
            case 6: {
                displayMemoryMap(simulator);
                break;
            }
            case 0: {
                running = false;
                break;
            }
            default:
                printf("\nOpción inválida.\n");
        }
    }
    
    freeSimulator(simulator);
    printf("\nSimulador finalizado.\n");
    
    return 0;
}

// Inicializa el simulador con un tamaño de memoria dado
MemorySimulator* initSimulator(int memory_size) {
    MemorySimulator* sim = (MemorySimulator*) malloc(sizeof(MemorySimulator));
    
    if (sim == NULL) {
        printf("Error: No se pudo asignar memoria para el simulador.\n");
        exit(1);
    }
    
    // Inicializar el primer bloque como libre, con el tamaño total de memoria
    MemoryBlock* initial_block = (MemoryBlock*) malloc(sizeof(MemoryBlock));
    
    if (initial_block == NULL) {
        printf("Error: No se pudo asignar memoria para el bloque inicial.\n");
        free(sim);
        exit(1);
    }
    
    initial_block->id = 0;
    initial_block->start_address = 0;
    initial_block->size = memory_size;
    initial_block->status = FREE;
    strcpy(initial_block->process_name, "");
    initial_block->next = NULL;
    
    sim->memory_map = initial_block;
    sim->total_memory = memory_size;
    sim->available_memory = memory_size;
    sim->partition_type = DYNAMIC_PARTITIONS;  // Por defecto
    sim->num_fixed_partitions = 0;
    sim->fixed_partition_size = 0;
    
    printf("Simulador inicializado con %d MB de memoria.\n", memory_size);
    
    return sim;
}

// Establece el modo de partición
void setPartitionMode(MemorySimulator* sim, PartitionType type, int num_partitions) {
    // Liberar la memoria y volver a inicializar con la nueva configuración
    while (sim->memory_map != NULL) {
        MemoryBlock* temp = sim->memory_map;
        sim->memory_map = sim->memory_map->next;
        free(temp);
    }
    
    sim->partition_type = type;
    sim->available_memory = sim->total_memory;
    
    if (type == FIXED_PARTITIONS) {
        sim->num_fixed_partitions = num_partitions;
        sim->fixed_partition_size = sim->total_memory / num_partitions;
        
        // Crear las particiones fijas
        MemoryBlock* prev_block = NULL;
        
        for (int i = 0; i < num_partitions; i++) {
            MemoryBlock* block = (MemoryBlock*) malloc(sizeof(MemoryBlock));
            
            if (block == NULL) {
                printf("Error: No se pudo asignar memoria para el bloque %d.\n", i);
                exit(1);
            }
            
            block->id = i;
            block->start_address = i * sim->fixed_partition_size;
            block->size = sim->fixed_partition_size;
            block->status = FREE;
            strcpy(block->process_name, "");
            block->next = NULL;
            
            if (prev_block == NULL) {
                sim->memory_map = block;
            } else {
                prev_block->next = block;
            }
            
            prev_block = block;
        }
        
        printf("Modo de particiones fijas establecido con %d particiones de %d MB cada una.\n", 
               num_partitions, sim->fixed_partition_size);
    } else {
        // Crear un único bloque libre con todo el espacio
        MemoryBlock* block = (MemoryBlock*) malloc(sizeof(MemoryBlock));
        
        if (block == NULL) {
            printf("Error: No se pudo asignar memoria para el bloque inicial.\n");
            exit(1);
        }
        
        block->id = 0;
        block->start_address = 0;
        block->size = sim->total_memory;
        block->status = FREE;
        strcpy(block->process_name, "");
        block->next = NULL;
        
        sim->memory_map = block;
        
        printf("Modo de particiones dinámicas establecido.\n");
    }
}

// Muestra el mapa de memoria actual
void displayMemoryMap(MemorySimulator* sim) {
    printf("\n=== Estado actual de la memoria (%d MB total, %d MB disponible) ===\n", 
           sim->total_memory, sim->available_memory);
    printf("Dirección\tTamaño\tEstado\t\tProceso\n");
    printf("--------------------------------------------------------------\n");
    
    MemoryBlock* current = sim->memory_map;
    int id_counter = 0;
    
    while (current != NULL) {
        printf("%4d MB\t\t%4d MB\t%s\t\t%s\n", 
               current->start_address, 
               current->size, 
               current->status == FREE ? "LIBRE" : "OCUPADO",
               current->status == FREE ? "-" : current->process_name);
        
        current = current->next;
        id_counter++;
    }
    
    printf("--------------------------------------------------------------\n");
    
    // Representación gráfica
    printf("\nRepresentación gráfica de la memoria:\n");
    printf("[");
    
    current = sim->memory_map;
    while (current != NULL) {
        // Imprimir bloques proporcionales a su tamaño
        int block_chars = (current->size * 50) / sim->total_memory;
        if (block_chars < 1) block_chars = 1;
        
        for (int i = 0; i < block_chars; i++) {
            if (current->status == FREE) {
                printf(".");
            } else {
                printf("#");
            }
        }
        
        if (current->next != NULL) {
            printf("|");
        }
        
        current = current->next;
    }
    
    printf("]\n");
    printf("Leyenda: [.] = Libre, [#] = Ocupado\n\n");
}

// Asigna memoria para un proceso según el modo de partición
bool allocateProcess(MemorySimulator* sim, char* process_name, int process_size) {
    if (process_size <= 0) {
        printf("Error: El tamaño del proceso debe ser mayor que cero.\n");
        return false;
    }
    
    if (process_size > sim->available_memory) {
        printf("Error: No hay suficiente memoria disponible para el proceso.\n");
        return false;
    }
    
    // Verificar si el proceso ya existe
    MemoryBlock* current = sim->memory_map;
    while (current != NULL) {
        if (current->status == OCCUPIED && strcmp(current->process_name, process_name) == 0) {
            printf("Error: Ya existe un proceso con el nombre '%s'.\n", process_name);
            return false;
        }
        current = current->next;
    }
    
    if (sim->partition_type == FIXED_PARTITIONS) {
        // Algoritmo de First-Fit para particiones fijas
        current = sim->memory_map;
        while (current != NULL) {
            if (current->status == FREE && process_size <= current->size) {
                current->status = OCCUPIED;
                strcpy(current->process_name, process_name);
                sim->available_memory -= current->size;  // Reservamos toda la partición
                return true;
            }
            current = current->next;
        }
        
        printf("Error: No se encontró una partición libre lo suficientemente grande.\n");
        return false;
    } else {
        // Algoritmo de First-Fit para particiones dinámicas
        current = sim->memory_map;
        int new_id = 0;
        
        while (current != NULL) {
            if (current->id > new_id) {
                new_id = current->id;
            }
            
            if (current->status == FREE && process_size <= current->size) {
                // Encontramos un bloque libre que puede contener el proceso
                if (current->size == process_size) {
                    // El bloque es exactamente del tamaño necesario
                    current->status = OCCUPIED;
                    strcpy(current->process_name, process_name);
                    sim->available_memory -= process_size;
                    return true;
                } else {
                    // El bloque es más grande de lo necesario, dividirlo
                    MemoryBlock* new_block = (MemoryBlock*) malloc(sizeof(MemoryBlock));
                    
                    if (new_block == NULL) {
                        printf("Error: No se pudo asignar memoria para el nuevo bloque.\n");
                        return false;
                    }
                    
                    new_block->id = new_id + 1;
                    new_block->start_address = current->start_address;
                    new_block->size = process_size;
                    new_block->status = OCCUPIED;
                    strcpy(new_block->process_name, process_name);
                    
                    // Ajustar el bloque original
                    current->start_address += process_size;
                    current->size -= process_size;
                    
                    // Insertar el nuevo bloque antes del actual
                    new_block->next = current;
                    
                    // Actualizar los punteros
                    if (current == sim->memory_map) {
                        sim->memory_map = new_block;
                    } else {
                        MemoryBlock* prev = sim->memory_map;
                        while (prev->next != current) {
                            prev = prev->next;
                        }
                        prev->next = new_block;
                    }
                    
                    sim->available_memory -= process_size;
                    return true;
                }
            }
            
            current = current->next;
        }
        
        printf("Error: No se encontró un bloque libre lo suficientemente grande.\n");
        return false;
    }
}

// Libera la memoria ocupada por un proceso
bool deallocateProcess(MemorySimulator* sim, char* process_name) {
    MemoryBlock* current = sim->memory_map;
    MemoryBlock* prev = NULL;
    bool found = false;
    
    while (current != NULL) {
        if (current->status == OCCUPIED && strcmp(current->process_name, process_name) == 0) {
            found = true;
            
            // Liberar el bloque
            current->status = FREE;
            strcpy(current->process_name, "");
            
            if (sim->partition_type == FIXED_PARTITIONS) {
                sim->available_memory += current->size;
            } else {
                sim->available_memory += current->size;
                
                // Intentar fusionar con bloques adyacentes (coalescing)
                MemoryBlock* next_block = current->next;
                
                // Fusionar con el bloque siguiente si está libre
                if (next_block != NULL && next_block->status == FREE) {
                    current->size += next_block->size;
                    current->next = next_block->next;
                    free(next_block);
                }
                
                // Fusionar con el bloque anterior si está libre
                if (prev != NULL && prev->status == FREE) {
                    prev->size += current->size;
                    prev->next = current->next;
                    free(current);
                    current = prev;
                }
            }
            
            break;
        }
        
        prev = current;
        current = current->next;
    }
    
    return found;
}

// Calcula la fragmentación interna y externa
void calculateFragmentation(MemorySimulator* sim, int* internal_frag, int* external_frag) {
    *internal_frag = 0;
    *external_frag = 0;
    
    MemoryBlock* current = sim->memory_map;
    
    while (current != NULL) {
        if (sim->partition_type == FIXED_PARTITIONS && current->status == OCCUPIED) {
            // Fragmentación interna en particiones fijas
            int used_size = 0;
            // En un sistema real, esto sería el tamaño real del proceso
            // Para este simulador, asumimos que el tamaño real es el 70% del tamaño de la partición
            used_size = (int)(current->size * 0.7);
            *internal_frag += (current->size - used_size);
        }
        
        if (sim->partition_type == DYNAMIC_PARTITIONS && current->status == FREE) {
            // Fragmentación externa en particiones dinámicas
            *external_frag += current->size;
        }
        
        current = current->next;
    }
    
    // En particiones fijas, la fragmentación externa es cero
    if (sim->partition_type == FIXED_PARTITIONS) {
        *external_frag = 0;
    }
}

// Compacta la memoria (solo para particiones dinámicas)
void compactMemory(MemorySimulator* sim) {
    if (sim->partition_type == FIXED_PARTITIONS) {
        printf("La compactación solo es aplicable al modo de particiones dinámicas.\n");
        return;
    }
    
    // Crear una nueva lista de bloques
    MemoryBlock* new_memory_map = NULL;
    MemoryBlock* new_memory_tail = NULL;
    
    // Primero, agregar todos los bloques ocupados
    MemoryBlock* current = sim->memory_map;
    int current_address = 0;
    
    while (current != NULL) {
        if (current->status == OCCUPIED) {
            MemoryBlock* new_block = (MemoryBlock*) malloc(sizeof(MemoryBlock));
            
            if (new_block == NULL) {
                printf("Error: No se pudo asignar memoria para el nuevo bloque.\n");
                return;
            }
            
            new_block->id = current->id;
            new_block->start_address = current_address;
            new_block->size = current->size;
            new_block->status = OCCUPIED;
            strcpy(new_block->process_name, current->process_name);
            new_block->next = NULL;
            
            if (new_memory_map == NULL) {
                new_memory_map = new_block;
                new_memory_tail = new_block;
            } else {
                new_memory_tail->next = new_block;
                new_memory_tail = new_block;
            }
            
            current_address += current->size;
        }
        
        current = current->next;
    }
    
    // Agregar un bloque libre al final si hay espacio disponible
    if (sim->available_memory > 0) {
        MemoryBlock* free_block = (MemoryBlock*) malloc(sizeof(MemoryBlock));
        
        if (free_block == NULL) {
            printf("Error: No se pudo asignar memoria para el bloque libre.\n");
            return;
        }
        
        free_block->id = (new_memory_tail != NULL) ? new_memory_tail->id + 1 : 0;
        free_block->start_address = current_address;
        free_block->size = sim->available_memory;
        free_block->status = FREE;
        strcpy(free_block->process_name, "");
        free_block->next = NULL;
        
        if (new_memory_map == NULL) {
            new_memory_map = free_block;
        } else {
            new_memory_tail->next = free_block;
        }
    }
    
    // Liberar la memoria del mapa antiguo
    while (sim->memory_map != NULL) {
        MemoryBlock* temp = sim->memory_map;
        sim->memory_map = sim->memory_map->next;
        free(temp);
    }
    
    // Actualizar el mapa de memoria
    sim->memory_map = new_memory_map;
    
    printf("Memoria compactada exitosamente.\n");
}

// Libera la memoria utilizada por el simulador
void freeSimulator(MemorySimulator* sim) {
    MemoryBlock* current = sim->memory_map;
    
    while (current != NULL) {
        MemoryBlock* temp = current;
        current = current->next;
        free(temp);
    }
    
    free(sim);
}

// Muestra el menú de opciones
void displayMenu() {
    printf("\n=== Simulador de Administración de Memoria ===\n");
    printf("1. Establecer modo de partición\n");
    printf("2. Cargar proceso\n");
    printf("3. Liberar proceso\n");
    printf("4. Calcular fragmentación\n");
    printf("5. Compactar memoria\n");
    printf("6. Mostrar estado de memoria\n");
    printf("0. Salir\n");
}