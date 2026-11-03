# MiniExtItemView - GDD

Stan: 2026-11-03.

Samodzielny lekki plugin poprawia czytelnosc oryginalnego opisu przedmiotu:
zawija etykiety, zachowuje liczby i kolory, a panel moze urosnac do 150%
natywnej wysokosci. Nie dodaje nowych opisow, przewijania ani ustawien.
Gdy ukladu nie da sie bezpiecznie przebudowac, pozostawia natywny panel.

Jest alternatywnym wariantem prezentacji, a nie pelnym ExtendedItemView.
Pakiet kompilowany jest dla G1/G1A/G2/G2A; odbior wizualny zalezy od silnika
i innych rozszerzen interfejsu.

Na G1 z fyryNy GD3D11 zgloszono losowy crash przy ladowaniu tekstury
podczas rysowania UI. Podobne ResizeTexture AV wystepowaly wedlug usera
takze przed Mini; przyczyna i konkretna tekstura nie sa jeszcze
potwierdzone. Nie wdrozono poprawki ani ograniczenia funkcjonalnosci.

[Dokumentacja](documentation.md).

Dystrybucja standalone: `Bezimienny111/MiniExtItemView`, `v0.1.0`
prerelease, tylko VDF do `Data/Plugins`. Nie wymaga CZM; nie instalowac
drugiej kopii Mini. Wydzielenie nie dodaje ani nie zmienia mechanik.
