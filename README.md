# Program Huffman w C

## Opis
Program implementuje **kompresję i dekompresję plików tekstowych** za pomocą **kodowania Huffmana**.  
Pliki wynikowe są zapisywane w formacie **binarnym** – znaki `'0'` i `'1'` nie są zapisywane, tylko faktyczne bity.  

Program używa własnej **kolejki priorytetowej**, zgodnie z materiałami z wykładu, z funkcjami:
- dodawania elementu (`push`)
- usuwania elementu o najmniejszej częstotliwości (`pop`)
- zmiany priorytetu (`changePriority`)
- budowania kolejki z tablicy (`buildFromArray`)  

---

## Wymagania
- Kompilator C (np. `gcc`)  
- System obsługujący standardowe pliki tekstowe  

---

## Kompilacja
```bash
gcc -o huffman huffman.c


