# 50shades

Simple concurrent programming project using ncurses lib made during my studies. A producer prints "50 Shades of Grey" books, which are delievered to bookstores. These bookstores are visited by readers who infinitely demand more copies.

You can find a description of this project in Polish below � it was used as a base of what I was supposed to do.

---

Systemy Operacyjne 2 // Opis projektu � dystrybucja ksi��ek

Producent drukuje kolejne egzemplarze ksi��ek �50 Twarzy Greya�, kt�re rozchodz� si� natychmiastowo. Ksi��ki przewo�one s� przez dostawc�w do ksi�gar�, w kt�rych to z kolei ustawiaj� si� kolejki klient�w, oczekuj�cych na kolejne dostawy.
Sekcjami krytycznymi w tym wypadku s� punkty, w kt�rych stykaj� si� poszczeg�lne ogniwa: tylko jeden dostawca mo�e znajdowa� si� w danej chwili u producenta i �adowa� ksi��ki, r�wnie� maksymalnie jeden dostawca mo�e roz�adowywa� ksi��ki do sklepu. Opr�cz tego, tylko jeden klient w danej chwili mo�e dokonywa� zakupu ksi��ki.
Interfejs u�ytkownika zrealizowany b�dzie przy u�yciu biblioteki ncurses. Na ekranie wy�wietlane b�d� informacje o liczbie dostawc�w, ksi�garni i klient�w. Opr�cz tego producent opisany b�dzie liczb� aktualnie posiadanych druk�w i pr�dko�ci produkcji, przy ka�dym z dostawc�w b�dzie informacja o jego aktualnym zadaniu (podr� z albo do drukarni, roz�adunek, za�adunek), a ka�dy sklep opisany b�dzie informacj� o liczbie aktualnie posiadanych ksi��ek i liczbie klient�w w kolejce.