#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <thread>

struct MarkerContext {
    int id;
    int arraySize;
    int* arr;
    HANDLE startEvent;
    HANDLE stopEvent;
    HANDLE continueEvent;
    HANDLE cannotContinueEvent;
    int markedCount;
    bool isDone;
};

DWORD WINAPI MarkerThread(LPVOID lpParam) {
    MarkerContext* ctx = static_cast<MarkerContext*>(lpParam);
    const int id = ctx->id;
    const int n = ctx->arraySize;
    int* arr = ctx->arr;

    WaitForSingleObject(ctx->startEvent, INFINITE);

    srand(id);

    ctx->markedCount = 0;
    ctx->isDone = false;

    while (!ctx->isDone) {
        int r = rand();
     
        int idx = r % n;

        if (arr[idx] == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
     
            arr[idx] = id;
            ctx->markedCount++;
      
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
      
            continue;
        }
        else {
     
            std::cout << "[Marker " << id << "] Cannot mark index " << idx
                << ". Marked: " << ctx->markedCount << "\n";
          
            SetEvent(ctx->cannotContinueEvent);
           
            HANDLE waitHandles[2] = { ctx->stopEvent, ctx->continueEvent };
            DWORD waitRes = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
            if (waitRes == WAIT_OBJECT_0) {
             
                ctx->isDone = true;
                break;
            }
          
        }
    }

    if (ctx->isDone) {
        for (int i = 0; i < n; ++i) {
            if (arr[i] == id) {
                arr[i] = 0;
            }
        }
    }

    return 0;
}

int main() {
    setlocale(LC_ALL, "ru-RU");

    int n;
    std::cout << "Введите размерность массива: ";
    std::cin >> n;
    if (n <= 0) {
        std::cerr << "Размерность должна быть положительной.\n";
        return 1;
    }

    int* arr = new int[n];
   
    for (int i = 0; i < n; ++i) arr[i] = 0;

    int markerCount;
    std::cout << "Введите количество потоков marker: ";
    std::cin >> markerCount;
    if (markerCount <= 0) {
        std::cerr << "Количество потоков должно быть положительным.\n";
        delete[] arr;
        return 1;
    }

    std::vector<MarkerContext> contexts(markerCount);
    std::vector<HANDLE> markerThreads(markerCount);

    HANDLE startEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr); 
    std::vector<HANDLE> cannotContinueEvents(markerCount);
    std::vector<HANDLE> stopEvents(markerCount);
    std::vector<HANDLE> continueEvents(markerCount);

    for (int i = 0; i < markerCount; ++i) {
        cannotContinueEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        stopEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        continueEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        contexts[i].id = i + 1; 
        contexts[i].arraySize = n;
        contexts[i].arr = arr;
        contexts[i].startEvent = startEvent;
        contexts[i].stopEvent = stopEvents[i];
        contexts[i].continueEvent = continueEvents[i];
        contexts[i].cannotContinueEvent = cannotContinueEvents[i];

        markerThreads[i] = CreateThread(nullptr, 0, MarkerThread, &contexts[i], 0, nullptr);
    }

    SetEvent(startEvent);

    while (true) {
        
        DWORD res = WaitForMultipleObjects(markerCount, cannotContinueEvents.data(), TRUE, INFINITE);
        if (res == WAIT_FAILED) {
            std::cerr << "Ошибка ожидания событий.\n";
            break;
        }

        std::cout << "Массив: ";
        for (int i = 0; i < n; ++i) std::cout << arr[i] << " ";
        std::cout << "\n";

        bool allDone = true;
        for (int i = 0; i < markerCount; ++i) {
            if (!contexts[i].isDone) {
                allDone = false;
                break;
            }
        }
        if (allDone) break; 

        int targetId;
        std::cout << "Введите номер потока marker для завершения (1.." << markerCount << "): ";
        std::cin >> targetId;
        if (targetId < 1 || targetId > markerCount) {
            std::cerr << "Неверный номер потока. Пропуск шага.\n";
            continue;
        }
        int idx = targetId - 1;

        SetEvent(stopEvents[idx]);

        WaitForSingleObject(markerThreads[idx], INFINITE);
        contexts[idx].isDone = true; 

        std::cout << "Массив после завершения потока " << targetId << ": ";
        for (int i = 0; i < n; ++i) std::cout << arr[i] << " ";
     
        for (int i = 0; i < markerCount; ++i) {
            if (!contexts[i].isDone) {
                SetEvent(continueEvents[i]);
            }
        }
    }
    delete[] arr;
    for (auto h : markerThreads) CloseHandle(h);
    CloseHandle(startEvent);
    for (auto h : cannotContinueEvents) CloseHandle(h);
    for (auto h : stopEvents) CloseHandle(h);
    for (auto h : continueEvents) CloseHandle(h);

    return 0;
}
