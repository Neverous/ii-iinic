AKTUALNE USTAWIENIA, UWAGI ITP
==============================

Zbiór przydatnych informacji.

* Synchronizacja dla **TDMA** bazuje na algorytmie **FTSP** (http://www.eecs.harvard.edu/~mdw/course/cs263/papers/ftsp-sensys04.pdf) i **PulseSync** (http://people.mpi-inf.mpg.de/~clenzen/pubs/LSW14pulsesync.pdf)
* **TDMA**: Czas podzielony na **ramki** które to są podzielone na **sloty** w których tylko jedno urządzenie powinno nadawać
* **FTSP**: W wielkim skrócie urządzenia wybierają lidera (najmniejszy słyszalny **MAC**) który co jakiś czas wysyła synchronizacje - jaki on twierdzi że jest czas, pozostałe urządzenia przekazują takie wiadomości dalej (też raz na jakiś czas) z numerem synchronizacji żeby się nie zapętlać na starych wiadomościach
* **PulseSync**: główna różnica z **FTSP** jest taka że jak dostaniemy nową synchronizację to staramy się ją jak najszybciej przekazać dalej
* Działanie podzielone na dwie fazy inicjalizacje i normalne działanie - w tym momencie **TDMA**
* W trakcie inicjalizacji radio odbiera wszystkie wiadomości ale wysyła tylko i wyłącznie wykrywanie sąsiadów/korzenia (discovery)
* W trakcie normalnego działania wysyła i odbiera wszystkie wiadomości - w szczególności bierze czynny udział w synchronizacji czasu


* Bitrate (przepływność?): **115200** bitów na sekundę ~ **14KB/s**
* Czas ramki w fazie inicjalizacji i normalnej pracy: ~**0.25s**
* Częstotliwość wysyłania synchronizacji: **8s**
* Liczba slotów: **16** (każde urządzenie zna swoje id (mamy 16 urządzeń więc 0-15) i to id jest jego slotem)
* Daje to teoretycznie przepustowość **225B/s** na urządzenie rzeczywista wychodzi ok. **100B/s**


* Aktualne zużycie pamięci na program i RAM: code: **20266 bytes** (**61%**); ram: **1263 bytes** (**61%**)


* Wiadomość wykrywania sąsiadów ([discovery](../messages/discovery)) waży **7B** {kind:**1B**, macaddr: **2B**, root_macaddr: **2B**, crc: **2B**}
* Wiadomość synchronizacji czasu ([synchronization](../messages/synchronization)) waży **15B** {kind: **1B**, macaddr: **2B**, root_macaddr: **2B**, seq_id: **2B**, global_time: **6B**, crc: **2B**}
* Wiadomość pokazywania sąsiadów ([neighbours](../messages/neighbours)) waży **5B** {kind: **1B**, macaddr: **2B**, crc: **2B**}
* Wiadomość backoff ([backoff](../messages/backoff)) waży **16B** {kind: **1B**, seq_id: **1B**, src_macaddr: **2B**, dest_macaddr: **2B**, payload: **8B**, crc: **2B**}
* Wiadomość backoffack ([backoff](../messages/backoff)) waży **7B** {kind: **1B**, src_macaddr: **2B**, dest_macaddr: **2B**, crc: **2B**}
