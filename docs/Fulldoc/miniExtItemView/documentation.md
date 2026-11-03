# MiniExtItemView - dokumentacja

Stan: 2026-11-03. Osobny MiniExtItemView.dll/VDF, nie modul wewnatrz CZM.dll.
[GDD](GDD.md), audyt (integration-repository reference; not exported).

## Zakres i dystrybucja

Lekki wrapping **natywnych** opisow itemow, bez dodatkowych linii, scrolla,
sidecara, externali, menu i INI. Kod:
[header](../../../src/modules/miniExtItemView/MiniExtItemViewModule.h),
[hooki](../../../src/modules/miniExtItemView/MiniExtItemViewModule.ipp).
Wspolny [TextWrap](../../../src/common/TextWrap.h) mierzy aktualny font.
CMakeLists (integration-repository reference; not exported) wydziela te translation units i generuje
G1/G1A/G2/G2A, a [DllMain](../../../src/DllMain.cpp) akceptuje te cztery silniki.
To zakres kompilacji, nie deklaracja odbioru runtime wszystkich wersji.

Target `MiniExtItemView`, packaging `build_mini_ext_item_view_vdf`, recipe
[build.vm](../../../mini-ext-item-view-vdf/build.vm). Pakiet zawiera DLL i
Union API przy wlaczonym GENERATE_UNION_API_DLL; nie ma wlasnego autorun .d.
CZM_VDF_DEPLOY_DIR moze kopiowac pakiet do podstawowej gry,
MINI_EIV_VDF_DEPLOY_DIR do dodatkowej. Sama zmiana docs nie wymaga buildu/deploy.

## Samodzielna publikacja (2026-11-03)

Repo publikacyjne: `Bezimienny111/MiniExtItemView`, pierwsza wersja
`v0.1.0` prerelease. Zrodlem prawdy pozostaje repo integracyjne CZM;
eksporter (integration-repository reference; not exported) kopiuje allowliste
aktualnych plikow worktree i przypina SDK jako submoduly. Manifest zawiera
SHA256 wejsc oraz bazowy commit; sam commit nie opisuje lokalnych zmian.
Nie przenosi innych modulow, ustawien lokalnych ani historii diagnostycznej.

Standalone CMake buduje tylko Mini i UnionAPI dla czterech silnikow,
bez zaleznosci od CZM/BetterInventory i bez auto-deploy. GothicVDFS jest
zewnetrznym narzedziem builda, nie czescia eksportu. Jedynym zalacznikiem
release jest `MiniExtItemView.vdf`, do skopiowania do `Data/Plugins`.
SHA256 i pelne noty licencyjne zaleznosci znajduja sie w opisie wydania.
Przy instalacji nie ladowac drugiej kopii tej samej DLL/VDF.
Ekstrakcja nie zmienia hookow, konfiguracji, ownership ani persistence.
Wyniki nowych buildow i weryfikacji paczek nalezy odroznic od testow gry;
znane ograniczenia renderera i layer-trace pozostaja aktualne.

## Przeplyw renderowania

Draw kontenera otacza `DrawPass`. DrawItemInfo tworzy `DrawContext` tylko
na czas original; hooki globalnych metod sa filtrowane po tym kontekscie.
Blit rozpoznaje wyklucznie jego `viewItemInfo`, nie dowolny view.

G1/G1A: PrintTimed/PrintTimedCX z time=-2 sa przechwytywane i zapamietywane
z pozycja/kolorem zamiast natywnego drukowania. Przy Blit pierwsze rysowanie
buduje layout; przy niepowodzeniu odtwarza oryginalne wydruki. Kolejne
Blit w tym samym kontekscie nie powiela rysunku.
G2/G2A: odczyt natywnych textLines, a przy braku poprawnego layoutu native Blit.

Teksty sortowane po Y/X. Pierwszy jest tytulem; dalej maks. 6 pol, po jednym
label i value na wysokosci. Tytul z drugim tekstem w tym samym Y, wiecej niz
dwa elementy w polu lub >13 retained wpisow oznacza fallback.
To rozpoznanie natywnej struktury, nie API do dowolnego custom UI.

Podczas mini draw PrintChars zbiera koncowe glyph runs do DrawPass (font,
kolor i pozycja). Po native Draw nastepuje `DrawFinalText`: bez tla, z
tymczasowym attach panelu, z replay guardem. Ma zachowac tekst nad preview.
RenderItem dla info-preview przesuwa je w poziomie do srodka 5500 virtual,
po native render wykonuje FlushAlphaPolys (G2/G2A) i FlushPolys.
DrawContext odtwarza pierwotny X preview po zakonczeniu DrawItemInfo.

## Layout

- Aktualny font i pixel geometry; brak stalej liczby znakow.
- Tytul do 2 linii; field label dostaje budzet po zarezerwowaniu co najmniej
  jednego wiersza dla kazdego pozostalego pola.
- Value jest drukowane tylko przy pierwszym wierszu pola i wyrównane do prawej.
- Dostepna szerokosc tekstu uwzglednia preview; values moga wykorzystac
  pelna szerokosc panelu.
- Wysokosc najwyzej 150% **zastanej** natywnej wysokosci i dostepnego miejsca.
  Dol panelu pozostaje staly; rosnie w gore. Nie rezerwuje nowych rzedow gridu.
- Brak miejsca, fontu, prawidlowej geometrii: fallback. Nadmiar tekstu moze
  byc skrocony przez WrapText; nie ma przewijania.

## Stan i ownership

DrawPass/DrawContext to stosowe scope z lancuchem previous i wektorami
kopii tekstow. Nie utrzymuja NPC/item refs miedzy frame.
Fonty, panel, texture i preview sa borrowed.
PanelScope zachowuje geometrie, font/fontColor, maxTextLength/textMaxLength
i preview geometry; czyści temporary text i przywraca pola.
AttachmentScope odpina tylko wlasny attach. BackgroundScope przywraca backTex.
G1 ClearText usuwa rowniez textLines; teksty sa zarzadzane w biezacym draw.

Brak managera Init/Exit, persistent heap view/world i danych kampanii.
Hooki sa statycznymi providerami Union; nie ma osobnego runtime disable.
Nie podlacza sie do Game_SaveBegin i nie zapisuje pustego SAV.

## Diagnostyka biezacej wersji

Header zawiera aktywny `Feedback::Trace`: po pierwszym DrawItemInfo zapisuje
do 250 zdarzen w `MiniExtItemView-layer.log` obok EXE, pierwszy zapis truncate.
Plik zawiera engine id, wskazniki, geometrie i fragmenty tekstow.
To pozostawiona diagnostyka layer-trace, **nie opcja INI**.
Brak mozliwosci otwarcia pliku jest obecnie cicho pomijany.

## Kompatybilnosc i odbior

Nie ma detekcji CZM/EIV ani negocjacji z innym pluginem przepisujacym
DrawItemInfo/Blit/PrintChars. Wspolne ladowanie z pelnym EIV wymaga osobnego
odbioru; nie zakladac, ze dwa wrappery uloza sie poprawnie.
Zrodla bazowe maja lokalne, niecommitowane zmiany; eksport zachowuje ich
aktualny stan i identyfikuje pliki przez SHA256.
2026-11-03: standalone x86 MSVC Release PASS z osobno skompilowanymi
G1/G1A/G2/G2A i wersja DLL 0.1.0.0, po pobraniu przypietych SDK z upstream.
VDF 449640 B: dokladnie MiniExtItemView.dll i UnionAPI.dll; katalog,
zgodnosc bajtowa payloadow i PE32 x86 PASS.
Bez nowego host testu Mini, reparse VM, wdrozenia ani odbioru layeringu
w grze; build nie potwierdza braku regresji renderera.

### Diagnoza ResizeTexture AV: G1 + fyryNy GD3D11 (2026-11-02)

Zgloszony stos przechodzi przez Mini, native Blit/DrawTile i CacheInNamed
do konwersji/ResizeTexture. DrawTile laduje teksture przed rysowaniem;
w referencyjnym Blit jest to backTex. To nie stos PrintChars ani preview.
Mini deleguje Blit do oryginalu; nie przeskalowuje danych tekstury.
Geometria panelu nie jest rozmiarem bufora tekstury.

SDK G1 potwierdza adresy CacheInNamed 0x005CBA70 oraz ConvertTextureFormat
0x00720DC0. Referencyjny kod silnika G2 (nie dowod identycznych instrukcji
G1) wykonuje ResizeTexture przed dekodowaniem DXT1, traktujac wewnetrzny
format12 jako 1 bajt/piksel zamiast blokow kompresji. To mocny podejrzany
odczyt poza buforem przy zmianie wymiarow. Wartosci 0x100/0x100/0x0C
w stosie pasuja do tego tropu, ale argumenty zoptymalizowanej funkcji
bez dumpa nie sa wystarczajacym dowodem formatu ani wymiarow zrodla.

[GD3D11 zCTexture.h](https://github.com/fyryNy/GD3D11/blob/3e1e8d496ee69301c155acd3d8a5149db5338904/D3D11Engine/zCTexture.h#L30)
nadal wywoluje oryginalne LoadResourceData i ma TODO o crashach niektorych
DTX1a. Komentarz nie dowodzi tej samej przyczyny; wersja DLL ze zgloszenia
nie jest znana. User potwierdzil podobne losowe ResizeTexture AV jeszcze
przed powstaniem Mini, czesciej przy wchodzeniu do wody/nurkowaniu.
Nie przypisano zatem przyczyny Mini ani nie uznano go za wykluczony
w kazdym mozliwym konflikcie.

Brak dumpa, logow tej sesji i reprodukcji. Nie zmieniano kodu, nie
uruchamiano builda ani gry. Do potwierdzenia potrzeba nazwy/headera
ladowanej tekstury, src/dst konwertera, instrukcji i adresu dostepu AV
oraz wersji DX11. Pomijanie tla lub konwersji nie jest naprawa.
