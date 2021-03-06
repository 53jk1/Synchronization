// > g++ event.cc -Wall -Wextra
// > a

#include <stdio.h>
#include <windows.h>

#include <list>

std::list<int> g_work;
CRITICAL_SECTION g_work_mutex; //Dostęp do listy musi być zsynchronizowany niezależnie od życia zdaarzeń, szczególnie jeśli jest ona modyfikowana w pętli.

HANDLE g_work_ready_ev; //Zdarzenie, którego użyjemy do wybudzania wątku przetwarzającego przygotowane dane.

HANDLE g_word_completed_ev; //Zdarzenie, którego użyjemy do poinformowania wątku o zakończeniu pracy.

DWORD WINAPI Producer(LPVOID) {
    for (int i = 0; i < 16; i++) {
        //Przygotuj nowe dane. W prawdziwym programie byłoby tu dużo więcej obliczeń.
        Sleep(150); //Przygotowywanie danych trwa długo.

        int new_data = i;

        //Dodaj nowy pakiet danych na koniec listy, synchronizując dostęp do struktury za pomocą sekcji krytycznej.
        EnterCriticalSection(&g_work_mutex);
        g_work.push_back(new_data);
        LeaveCriticalSection(&g_work_mutex);

        //Poinformuj drugi wątek, że dane są gotowe do przetwarzania.
        SetEvent(g_workd_ready_ev);
    }

    SetEvent(g_work_completed_ev);
    return 0;
}

DWORD WINAPI Consumer(LPVOID) {
    HANDLE events[] = { g_work_ready_ev, g_work_completed_ev};
    for (;;) {
        //Poczekaj na jedno ze zdarzeń. Zmienna ret zawiera indekt zdarzenia, które zostało zasygnalizowane. Jeśli oba zdarzenia zostały zasygnalizowane, ret zawiera mniejszy indeks.
        printf("** Zzz...!\n");
        DWORD ret = WaitForMultipleObjects(2, events, FALSE, INFINITE);

        if (ret >= sizeof(events)) {
            //Wystąpił błąd lub jedno ze zdarzeń zostało porzucone. Aplikacja prawdopodobnie zostanie zakończona z błędem, więc można zakończyć wątek. Nie powinno się to nigdy zdarzyć w przypadku tego przykładu.
            return 1;
        }

    //Czy wątek został obudzony przez g_work_ready_ev?
    if (ret == 0) {
        printf("-- Signaled: Data ready!\n");

        //Przetwórz dane.
        for(;;) {
            bool done = false;
            int data;

            //Sprawdź czy dane są dostępne.
            EnterCriticalSection(&g_work_mutex);
            if (g_work.size() > 0) {
                //Pobierz dane.
                data = g_work.front();
                g_work.pop_front();
            } else {
                //Brak danych.
                done = true;
            }
            LeaveCriticalSection(&g_work_mutex);
            if (done) {
                break;
            }

            //Przetwarzanie danych.
            printf("Working on data: %i\n", data);
            Sleep(100); //Symulacja ciężkiej pracy.
            printf("Done working on: %i\n", data);
        }
        continue;
    }

    //Czy wątek został obudzony przez g_work_completed_ev?
    if (ret == 1) {
        //Zdarzenie g_work_completed_ev zostało zasygnalizowane. Koniec pracy.
        printf("-- Signaled: Work complete!\n");
        break;
    }
}

return 0;
}

int main(){
    //Stwórz zdarzenie, które będzie automatycznie resetowane w przypadku, gdy przynajmniej jeden wątek zostanie zwolniony po zasygnalizowaniu.
    g_work_ready_ev = CreateEvent(NULL, FALSE, FALSE, NULL);

    //Poniższe zdarzenie nie będzie samoresetujące.
    g_work_completed_ev = CreateEvent(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection(&g_work_mutex);
    HANDLE h[2] = {
        CreateThread(NULL, 0, Producer, NULL, 0, NULL),
        CreateThread(NULL, 0, Consumer, NULL, 0, NULL)
    };

    WaitForMultipleObjects(2, h, TRUE, INFINITE);

    CloseHandle(h[0]);
    CloseHandle(h[1]);
    CloseHandle(g_work_ready_ev);
    CloseHandle(g_work_completed_ev);
    DeleteCriticalSection(&g_work_mutex);

    return 0;
}