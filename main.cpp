#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <string>
#include <iostream>

using namespace std;

#define BUFF 2048

enum MODE
{
    FOREGROUND,
    BACKGROUND
};

struct ProcessInfo
{
    DWORD pid;
    char command[BUFF];
    int status;
};

char cmd[BUFF];
ProcessInfo listProcess[100];
int countProcess = 0;
int check = 0;

string workingdir()
{
    char buf[256];
    GetCurrentDirectoryA(256, buf);
    return string(buf) + '\\';
}

bool checkFile(const char *name)
{
    struct stat buffer;
    return (stat(name, &buffer) == 0);
}

void closeApp(DWORD pid)
{
    printf("close pid = %u\n", pid);
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, pid);
    if (!TerminateProcess(hProcess, 0))
    {
        printf("Process does not exist\n");
        CloseHandle(hProcess);
        for (int i = 0; i < countProcess; i++)
        {
            if (listProcess[i].pid == pid)
            {
                for (int j = i; j < countProcess - 1; j++)
                {
                    listProcess[j] = listProcess[j + 1];
                }

                memset(&listProcess[countProcess - 1], 0, sizeof(ProcessInfo));
                countProcess--;
                break;
            }
        }
        return;
    }

    CloseHandle(hProcess);

    for (int i = 0; i < countProcess; i++)
    {
        if (listProcess[i].pid == pid)
        {
            for (int j = i; j < countProcess - 1; j++)
            {
                listProcess[j] = listProcess[j + 1];
            }

            memset(&listProcess[countProcess - 1], 0, sizeof(ProcessInfo));
            countProcess--;
            break;
        }
    }
    return;
}

void addNewProcess(DWORD pid, const char *cmd)
{
    ProcessInfo newProcess;
    memset(&newProcess, 0, sizeof(ProcessInfo));

    newProcess.pid = pid;
    newProcess.status = 1;
    strcpy(newProcess.command, cmd);

    listProcess[countProcess] = newProcess;
    countProcess++;
}

void sigintHandler(int sig_num)
{
    closeApp(listProcess[countProcess - 1].pid);
    check = 1;
}

void openApp(char *cmd, const char *path, MODE mode)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    int rs = CreateProcess(path, NULL, NULL, NULL, FALSE,
                           CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (rs != 0)
    {
        addNewProcess(pi.dwProcessId, cmd);
        if (mode == BACKGROUND)
            WaitForSingleObject(pi.hProcess, 0);
        else
        {
            signal(SIGINT, sigintHandler);
            WaitForSingleObject(pi.hProcess, INFINITE);
            
            if(check==1) 
            {
                check = 0;
            }
            else{
                printf("close pid = %u\n", listProcess[countProcess-1].pid);
                countProcess--;
            }

        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return;
    }
    else
    {
        printf("error: %u\n", GetLastError());
        return;
    }
}

int main()
{

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 10);
    printf("         WELCOME TO MY TINYSHELL\n");
    printf("---------------------------------------------\n");
    printf("%-10s - Current time\n", "time");
    printf("%-10s - Current date\n", "date");
    printf("%-10s - Current directory \n", "dir");
    printf("%-10s - Clear \n", "cls");
    printf("%-10s - Excute application\n", "<cmd>");
    printf("%-10s - Excute application in background\n", "<cmd> &");
    printf("%-10s - Kill process by pid\n", "kill <pid>");
    printf("%-10s - Run file *.bat\n", "bat <file>");
    printf("%-10s - List all backround process\n", "list");
    printf("%-10s - Restart computer\n", "restart");
    printf("%-10s - Turn off computer\n", "shutdown");
    printf("%-10s - Close Shell\n", "exit");
    printf("---------------------------------------------\n");
    SetConsoleTextAttribute(hConsole, 15);
    while (true)
    {
        SetConsoleTextAttribute(hConsole, 14);
        printf("SHELL>");
        SetConsoleTextAttribute(hConsole, 15);
        gets(cmd);

        if (strlen(cmd) == 0)
            continue;

        else if (strstr(cmd, "calc") != NULL)
        {
            if (strstr(cmd, "&") != NULL)
                openApp(cmd, "calc.exe", BACKGROUND);
            else
                openApp(cmd, "calc.exe", FOREGROUND);
        }

        else if (strstr(cmd, "clock") != NULL)
        {
            if (strstr(cmd, "&") != NULL)
                openApp(cmd, "clock.exe", BACKGROUND);
            else
                openApp(cmd, "clock.exe", FOREGROUND);
        }

        else if (strstr(cmd, "kill") != NULL)
        {
            if (strlen(cmd) > 5)
            {
                char pid[256];
                memset(pid, 0, sizeof(pid));
                strncpy(pid, cmd + 5, strlen(cmd) - 5);

                DWORD _pid = atoi(pid);
                closeApp(_pid);
            }
            else
            {
                printf("Required to enter process id\n");
            }
        }

        else if (strcmp(cmd, "date") == 0)
        {
            SYSTEMTIME st;
            GetLocalTime(&st);
            printf("Current date: %02d/%02d/%d\n", st.wDay, st.wMonth, st.wYear);
        }

        else if (strcmp(cmd, "time") == 0)
        {
            SYSTEMTIME st;
            GetLocalTime(&st);
            printf("Current time: %02d:%02d:%02d\n", st.wHour, st.wMinute, st.wSecond);
        }

        else if (strcmp(cmd, "list") == 0)
        {
            printf("----------------------------------------\n");
            printf("| %-10s | %-10s | %-10s |\n", "PID", "COMMAND", "STATUS");
            printf("----------------------------------------\n");
            for (int i = 0; i < countProcess; i++)
            {
                printf("| %-10u | %-10s | %-10d |\n", listProcess[i].pid, listProcess[i].command, listProcess[i].status);
                printf("----------------------------------------\n");
            }
        }

        else if (strcmp(cmd, "help") == 0)
        {
            printf("%-10s - Current time\n", "time");
            printf("%-10s - Current date\n", "date");
            printf("%-10s - Current directory \n", "dir");
            printf("%-10s - Clear \n", "cls");
            printf("%-10s - Excute application\n", "<cmd>");
            printf("%-10s - Excute application in background\n", "<cmd> &");
            printf("%-10s - Kill process by pid\n", "kill <pid>");
            printf("%-10s - Run file *.bat\n", "bat <file>");
            printf("%-10s - List all backround process\n", "list");
            printf("%-10s - List all command\n", "help");
            printf("%-10s - Restart computer\n", "restart");
            printf("%-10s - Turn off computer\n", "shutdown");
            printf("%-10s - Close Shell\n", "exit");
        }

        else if (strstr(cmd, "bat") != NULL)
        {
            if (strlen(cmd) > 4)
            {
                char fileName[256];
                memset(fileName, 0, sizeof(fileName));
                strncpy(fileName, cmd + 4, strlen(cmd) - 4);

                if (checkFile(fileName))
                    system(fileName);
                else
                    printf("file is not exist\n");
            }
            else
            {
                printf("Required file name\n");
            }
        }

        else if (strcmp(cmd, "restart") == 0)
        {
            system("c:\\windows\\system32\\shutdown /r /t 0");
        }

        else if (strcmp(cmd, "shutdown") == 0)
        {
            system("c:\\windows\\system32\\shutdown /s /t 0");
        }

        else if (strcmp(cmd, "exit") == 0)
        {
            for (int i = 0; i < countProcess; i++)
            {
                closeApp(listProcess[i].pid);
            }
            exit(0);
        }
        else if (strcmp(cmd, "dir") == 0)
        {
            cout << workingdir() << endl;
        }

        else if (strcmp(cmd, "cls") == 0)
        {
            system("cls");
        }

        else
        {
            printf("Command '%s' not found\n", cmd);
        }
    }
    return 0;
}
