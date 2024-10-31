#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#define TARGET_PROCESS_NAME "TheForest.exe"
#define BASE_ADDRESS 0x0142E8F8  // Endereço base fornecido

// Definições das offsets para vida, energia e stamina
int offsetLife[] = {0x50, 0x78, 0xD8, 0x30, 0x250};
int offsetEnergy[] = {0x58, 0x78, 0xD8, 0x30, 0x258};
int offsetStamina[] = {0x50, 0x78, 0xD8, 0x30, 0x24C};

// Função para obter o ID do processo a partir do nome
DWORD GetProcessIdByName(const char* processName) {
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return 0;
}

// Função para calcular o ponteiro final a partir de offsets
uintptr_t GetPointer(HANDLE hProcess, uintptr_t baseAddress, int* offsets, int offsetCount) {
    uintptr_t addr = baseAddress;
    SIZE_T bytesRead;

    for (int i = 0; i < offsetCount - 1; i++) {
        if (!ReadProcessMemory(hProcess, (BYTE*)addr, &addr, sizeof(addr), &bytesRead) || bytesRead != sizeof(addr)) {
            printf("[*] Falha ao ler memória no offset %d\n", i);
            return 0;  // Retorna 0 para indicar erro na leitura
        }
        addr += offsets[i];
    }
    addr += offsets[offsetCount - 1];
    return addr;
}

// Função para escrever valor float na memória do jogo
BOOL WriteFloat(HANDLE hProcess, uintptr_t address, float value) {
    SIZE_T bytesWritten;
    BOOL success = WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(value), &bytesWritten);
    if (!success || bytesWritten != sizeof(value)) {
        printf("[*] Falha ao escrever na memória. Código de erro: %lu\n", GetLastError());
    }
    return success;
}

// Função para configurar valores de vida, energia e stamina
void SetLife(HANDLE hProcess, uintptr_t baseAddress) {
    uintptr_t finalAddress = GetPointer(hProcess, baseAddress, offsetLife, sizeof(offsetLife) / sizeof(offsetLife[0]));
    if (finalAddress) {
        WriteFloat(hProcess, finalAddress, 100.0f);
    } else {
        printf("[*] Endereço de vida inválido\n");
    }
}

void SetEnergy(HANDLE hProcess, uintptr_t baseAddress) {
    uintptr_t finalAddress = GetPointer(hProcess, baseAddress, offsetEnergy, sizeof(offsetEnergy) / sizeof(offsetEnergy[0]));
    if (finalAddress) {
        WriteFloat(hProcess, finalAddress, 100.0f);
    } else {
        printf("[*] Endereço de energia inválido\n");
    }
}

void SetStamina(HANDLE hProcess, uintptr_t baseAddress) {
    uintptr_t finalAddress = GetPointer(hProcess, baseAddress, offsetStamina, sizeof(offsetStamina) / sizeof(offsetStamina[0]));
    if (finalAddress) {
        WriteFloat(hProcess, finalAddress, 100.0f);
    } else {
        printf("[*] Endereço de stamina inválido\n");
    }
}

// Função principal
int main() {
    DWORD processId = 0;
    HANDLE hProcess = NULL;
    uintptr_t baseAddress = BASE_ADDRESS;

    printf("[*] Procurando processo do jogo...\n");

    // Tenta encontrar o processo alvo em loop até encontrá-lo
    while (processId == 0) {
        processId = GetProcessIdByName(TARGET_PROCESS_NAME);
        if (processId == 0) {
            printf("[*] Processo não encontrado. Tentando novamente...\n");
            Sleep(3000);
        }
    }

    printf("[*] Processo encontrado! ID: %d\n", processId);

    // Abre o processo com permissões de leitura e escrita
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    if (hProcess == NULL) {
        printf("[*] Erro ao abrir o processo. Código de erro: %lu\n", GetLastError());
        return 1;
    }

    printf("[*] Iniciando...\n");
    Sleep(1000);
    printf("[*] Life: [OK]\n");
    Sleep(500);
    printf("[*] Energy: [OK]\n");
    Sleep(500);
    printf("[*] Stamina: [OK]\n");
    Sleep(1000);

    // Loop principal de atualização dos valores
    while (1) {
        SetLife(hProcess, baseAddress);
        SetEnergy(hProcess, baseAddress);
        SetStamina(hProcess, baseAddress);
        printf("\r[*] Injetado com sucesso!");  // Mantém mensagem em uma linha
        Sleep(100);
    }

    CloseHandle(hProcess);
    return 0;
}

// Teste de injeção usando API do WIndows, falha de injeção na offset 0xD8