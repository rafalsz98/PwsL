make - aby zbudować zarówno narzędzie do wysyłania sygnałów, jak i rejestrator
make sender
make rejestrator
make clean - czysci pliki wykonywalne

Specyfikacja sendera:
Przyjmuje kolejno wprowadzane po spacji liczby całkowite: PID, numer sygnału RT, wartość dla sygnału
Przykład: ./sender 1619 34 1048576000 (Co w rejestratorze da floata 0.25)

Rejestrator oczekuje sygnałów z danymi, w których wartość int jest reprezentacją binarną liczby float
Domyślny stan rejestratora to stop