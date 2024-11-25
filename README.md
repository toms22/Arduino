Odpoctove stopky, zobrazovani na 3 mistne 7 segmentovce.
Defaultni cas 3 minuty, nastaveni casu po 30 sekundovych krocich, pri dosazeni casu pod 15 sekund se displej rozblika s periodou 200 ms.

Vse vyse uvedene lze nastavit pomoci konstant na zacatku programu -> komentare v kodu.
Minimalni nastavitelny cas je 30 sekund, maximalni 5 minut.

Pro debug kodu vypis stavu a kazdou sekundu vypis mereneho casu na seriovy monitor.

Ovladani stopek pres seriovou linku (UART) a tlacitka

Tlacitka a analogove vstupy:
- A0 -> Rizeni jasu displeje DutyCycle 5-100%
- A1 -> Start
- A2 -> Minus nastavitelny krok - 30 sekund
- A3 -> Plus nastavitelny krok - 30 sekund
- A4 -> Reset

Serial:
- '0' -> Reset
- '1' -> Start
- '2' -> Stop
- '+' -> Plus nastavitelny krok - 30 sekund
- '-' -> Minus nastavitelny krok - 30 sekund
- 'I' nebo 'i' -> Nastaveni inkrementace casu (ladeni pri vyvoji)
- 'D' nebo 'd' -> Nastaveni dekrementace casu (ladeni pri vyvoji)

!!! Dulezite info !!!

Kod byl napsan pro tlacitka ktere spinaji GND vuci internimu Pull-Up rezistoru v MCU.
Pro tvoje zapojeni je potřeba zakomentovat #define INPUT_TESTING_PULLUP na zacatku programu, tim se obrati logika vyhodnoceni vstupu (viz. kod ve funkci CheckButtons())
