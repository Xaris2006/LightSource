#pragma once

#include <iostream>
#include <Windows.h>

#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#include <thread>
#include <chrono>
#include <string>

class Process
{

#if 1

#define BUFSIZE 4096 
public:
    Process(std::wstring processNamepath, std::wstring cmdarg)
    {
        SECURITY_ATTRIBUTES saAttr;
		
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        HANDLE ChildStd_OUT_Wr = NULL;
        HANDLE ChildStd_IN_Rd = NULL;

        if (!CreatePipe(&m_ChildStd_OUT_Rd, &ChildStd_OUT_Wr, &saAttr, 0))
            throw std::runtime_error("StdoutRd CreatePipe");

        if (!SetHandleInformation(m_ChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
            throw std::runtime_error("Stdout SetHandleInformation");

        if (!CreatePipe(&ChildStd_IN_Rd, &m_ChildStd_IN_Wr, &saAttr, 0))
            throw std::runtime_error("Stdin CreatePipe");

        if (!SetHandleInformation(m_ChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
            throw std::runtime_error("Stdin SetHandleInformation");


        TCHAR szCmdline[] = TEXT("");
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        BOOL bSuccess = FALSE;

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = ChildStd_OUT_Wr;
        siStartInfo.hStdOutput = ChildStd_OUT_Wr;
        siStartInfo.hStdInput = ChildStd_IN_Rd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process. 

        bSuccess = CreateProcess(processNamepath.c_str(),
            szCmdline,     // command line 
            NULL,          // process security attributes 
            NULL,          // primary thread security attributes 
            TRUE,          // handles are inherited 
			CREATE_NO_WINDOW,// creation flags 
            NULL,          // use parent's environment 
            NULL,          // use parent's current directory 
            &siStartInfo,  // STARTUPINFO pointer 
            &piProcInfo);  // receives PROCESS_INFORMATION 

		if (!bSuccess)
			throw std::runtime_error("CreateProcess");
        else
        {
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
            CloseHandle(ChildStd_OUT_Wr);
            CloseHandle(ChildStd_IN_Rd);
        }
    }

    ~Process()
    {
        CloseHandle(m_ChildStd_IN_Wr);
        CloseHandle(m_ChildStd_OUT_Rd);
    }

    void Write(std::string message)
    {
        DWORD dwWritten;
        BOOL bSuccess = FALSE;

        message += '\n';

        bSuccess = WriteFile(m_ChildStd_IN_Wr, message.c_str(), message.size(), &dwWritten, NULL);
        if (!bSuccess)
            throw std::runtime_error("Write to Process");
    }

    std::string Read()
    {
        DWORD dwRead;
        CHAR chBuf[BUFSIZE];
        BOOL bSuccess = FALSE;
        std::string output;

        do
        {
            dwRead = 0;
            bSuccess = ReadFile(m_ChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
            if (!bSuccess || dwRead == 0)
                break;
            if (!bSuccess)//will not be hitted 
                throw std::runtime_error("Read Process");
            output += std::string(chBuf, dwRead);
        } while (dwRead == BUFSIZE);

        return output;
    }

private:
   
    HANDLE m_ChildStd_IN_Wr = NULL;
    HANDLE m_ChildStd_OUT_Rd = NULL;

#endif // PLATFORM_WINDOWS_64
};

#if 0
int notmain()
{
    std::string overall;
    float score = 0;

    Process stockfish(L"stockfish.exe", L"");
    stockfish.Write("ucinewgame");
    stockfish.Write("position fen 4r3/6pp/pp1k1b2/2p1N3/5P2/2P3P1/P4K1P/4R3 w - - 5 31");
    stockfish.Write("d");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    std::cout << stockfish.Read() << '\n' << score;
    stockfish.Write("go infinite");
    for (int i = 0; i < 200; i++)
    {
        overall.clear();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.4s);
        overall += stockfish.Read();
        int index = overall.find("score cp");
        if (index + 9 < overall.size() && index >= 0)
        {
            index += 9;
            std::string strscore;
            for (int j = index; j < overall.size(); j++)
            {
                if (overall[j] == ' ')
                    break;
                strscore += overall[j];
            }
            score = (float)std::stoi(strscore) / 100.0f;
        }
        std::cout << '\r' << score;
    }

    //stockfish.Write("stop");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.4s);
    std::cout << stockfish.Read();

    
    printf("\n->End of parent execution.\n");
}
#endif
