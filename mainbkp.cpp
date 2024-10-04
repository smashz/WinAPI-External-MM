// run x86 or x64 if game module not detected
// set Character Set to Multi-Byte in Visual Studio

#include <Windows.h>
#include<TlHelp32.h>
#include <tchar.h> // _tcscmp
#include <iostream>
#include <vector>



DWORD GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD procID) {
    DWORD dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procID); // make snapshot of all modules within process
    MODULEENTRY32 ModuleEntry32 = { 0 };
    ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &ModuleEntry32)) //store first Module in ModuleEntry32
    {
        do {
            if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) // if Found Module matches Module we look for -> done!
            {
                dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &ModuleEntry32)); // go through Module entries in Snapshot and store in ModuleEntry32


    }
    CloseHandle(hSnapshot);
    return dwModuleBaseAddress;
}



int main() {
    
    DWORD procID = NULL; // process id (A DWORD store values like a variable)
    

    HWND hwnd = FindWindowA(NULL, "Counter-Strike Source"); // function finds process with name we want
    GetWindowThreadProcessId(hwnd, &procID); // takes process it found assigns to procID
    HANDLE handle = NULL;
    handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID); // open the process id of program with full priviledge

    if (handle == NULL || handle == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return -1;
    }

    if (hwnd == NULL) { // case it doesn't find anything
        std::cout << "Open Counter-Strike" << std::endl;
        Sleep(3000);
        exit(-1);
    }
    else {
        
        std::cout << "procID: " << procID << std::endl;
        char baseName[] = "server.dll"; //((((("server.dll")))))))+004EE66C
        DWORD gameBaseAddress = GetModuleBaseAddress(_T(baseName), procID); // How to get base address - start at entry point (server.dll)
        std::cout << "gameBaseAddress: " << std::hex << gameBaseAddress << std::endl;



        //Ammo
        DWORD BulletBaseAdress = 0x004EE66C; // offsetGameToBaseAdress
        std::vector<DWORD> BulletPointOffsets{ 0x0,0x14,0x150,0x18,0x34,0x4B0 }; // values that shoe when you click to change address on cheat engine

        DWORD bulletBaseAddress = NULL;
        //Get value at gamebase+offset -> store it in bulletBaseAddress
        ReadProcessMemory(handle, (LPVOID)(gameBaseAddress + BulletBaseAdress), &bulletBaseAddress, sizeof(bulletBaseAddress), NULL);
        std::cout << "debugginfo: bulletBaseaddress = " << std::hex << bulletBaseAddress << std::endl;
        DWORD bulletCodeAddress = bulletBaseAddress; //the Adress we need -> change now while going through offsets
        for (int i = 0; i < BulletPointOffsets.size() - 1; i++) // -1 because we dont want the value at the last offset
        {
            ReadProcessMemory(handle, (LPVOID)(bulletCodeAddress + BulletPointOffsets.at(i)), &bulletCodeAddress, sizeof(bulletCodeAddress), NULL);
            std::cout << "debugginfo: Value at offset = " << bulletCodeAddress << std::endl;
        }
        bulletCodeAddress += BulletPointOffsets.at(BulletPointOffsets.size() - 1); //Add Last offset (0) -> done!!
        
        std::cout << "\nWinAPI MM External" << std::endl << '\n';
        //std::cout << "Press DELETE to EXIT" << std::endl;
        std::cout << "[DELETE] To Change Ammo" << std::endl;
        while (true) {
            Sleep(50);
            if (GetAsyncKeyState(VK_NUMPAD1)) { // Exit
                return 0;
            }
            if (GetAsyncKeyState(VK_DELETE)) {//Mouseposition
                std::cout << "Amount: ";
                int ammo = 0;
                std::cin >> ammo;
                //(handle, value_to_change, new_value, sizeof_value, how many bytes are writen)
                WriteProcessMemory(handle, (LPVOID)(bulletCodeAddress), &ammo, 4, 0);
            }
        }

        
    }

    return 0;

}

