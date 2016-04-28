AKTUALNE USTAWIENIA, UWAGI ITP
==============================

Zbiór przydatnych informacji.

* Synchronizacja dla **TDMA** bazuje na algorytmie **FTSP** (http://www.eecs.harvard.edu/~mdw/course/cs263/papers/ftsp-sensys04.pdf)
* **TDMA**: Czas podzielony na **ramki** które to są podzielone na **sloty** w których tylko jedno urządzenie powinno nadawać
* **FTSP**: W wielkim skrócie urządzenia wybierają lidera (najmniejszy słyszalny **MAC**) który co jakiś czas wysyła synchronizacje - jaki on twierdzi że jest czas, pozostałe urządzenia przekazują takie wiadomości dalej (też raz na jakiś czas) z numerem synchronizacji żeby się nie zapętlać na starych wiadomościach
* Działanie podzielone na dwie fazy inicjalizacje i normalne działanie - w tym momencie **TDMA**
* W trakcie inicjalizacji radio odbiera wszystkie wiadomości ale wysyła tylko i wyłącznie wykrywanie sąsiadów/korzenia (discovery)
* W trakcie normalnego działania wysyła i odbiera wszystkie wiadomości - w szczególności bierze czynny udział w synchronizacji czasu


* Bitrate (przepływność?): **115200** bitów na sekundę ~ **14KB/s**
* Czas ramki w fazie inicjalizacji i normalnej pracy: ~**1s**
* Częstotliwość wysyłania synchronizacji: **8s**
* Liczba slotów: **128** (każde urządzenie losuje w którym chce nadawać to daje jakieś **37%** szans że nie będzie kolizji przy maksymalnym zagęszczeniu) [wzorek o ile umiem liczyć to: n! / ((n-k)! * n^k), gdzie **n** to liczba slotów a **k** to liczba urządzeń]
* Daje to teoretycznie przepustowość **100B/s** na urządzenie rzeczywista wychodzi ok. **50B/s**


* Aktualne zużycie pamięci na program i RAM: code: **13466 bytes** (**41%**); ram: **1346 bytes** (**65%**)


* Wiadomość wykrywania sąsiadów ([discovery](../messages/discovery.h)) waży **7B** {kind:**1B**, macaddr: **2B**, root_macaddr: **2B**, crc: **2B**}
* Wiadomość synchronizacji czasu ([synchronization](../messages/synchronization.h)) waży **15B** {kind: **1B**, macaddr: **2B**, root_macaddr: **2B**, seq_id: **2B**, global_time: **6B**, crc: **2B**}
* Wiadomość pokazywania sąsiadów ([neighbours](../messages/neighbours.h)) waży **5B** {kind: **1B**, macaddr: **2B**, crc: **2B**}
