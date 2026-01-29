# Program Huffman w C

## Opis
Program implementuje **kompresję i dekompresję plików tekstowych** za pomocą **kodowania Huffmana**.  
Algorytm tworzy optymalne drzewo binarne na podstawie częstotliwości występowania znaków, generując krótsze kody dla częściej występujących znaków.  

Pliki wynikowe są zapisywane w formacie **binarnym** – znaki `'0'` i `'1'` są przechowywane jako faktyczne bity, nie jako tekst ASCII.

---

## Wymagania
- Kompilator C (np. `gcc`, `clang`)
- System operacyjny obsługujący standardowe pliki tekstowe i binarne

---

## Kompilacja
```bash
gcc -o huffman huffman-poprawiony.c