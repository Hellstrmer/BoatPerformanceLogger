# BoatPerformanceLogger – PCB-ändringar

Oct 25, 2026 · @Jesper Hellström

Kortet behöver fem motoringångar: en pulsingång (RPM), två digitala larm och två resistiva givare, plus batterispänningen, bränsleflödet (FS-40 på egen plint), lyftgivaren (J6) och vattentemperaturen (J7). Alla värden nedan är uppmätta på kabeln till originalmätaren (6Y5), mot svart = motorjord.

## Status och nästa steg

Rev 1 sitter i drift på bänken med batteri, trim, tank och vattentryck i firmware. Rev 2 är ritad med vattentemperaturen och ska beställas när punkterna nedan är klara.

**PCB rev 2**

- [ ] R30 som 1 kΩ 1 %, J7:s footprint och de överlappande texterna vid IMU-headern.
- [ ] Pinnamnen på silkscreen syns inte i JLC:s 2D-vy: kontrollera att de ligger på Top Silkscreen och inte på pads.
- [ ] ESP:ns footprint: gör ett eget speglat footprint för topplagret och jämför 5V/GND och GPIO1/2 mot rev 1.
- [ ] RESERV1/2: besluta om lödbrygga för 5 V-läge (6,8 kΩ parallellt med 39 kΩ), 5 V-matning till givaren och en omonterad 1206-pull-up mot +12 V.
- [ ] Om tilt och fuel byter GPIO: uppdatera pinnplanen och `analog.cpp`.
- [ ] Shift+B, DRC och kontroll av Gerber i JLC:s visare före beställning.
- [ ] Beställ JLC med J-delarna från LCSC i samma order, och M12-kontakter till lådan via företaget.

**Givare att köpa**

- [ ] Bosch 0 280 130 037 med motkontakt; verifiera kurvan i isvatten och kokande vatten.
- [ ] Vattentrycksgivare 30 psi, 0,5–4,5 V, och adapter för 1/8"-27 NPT.
- [ ] Plugg för J6 (5 poler).

**Firmware och Pi**

- [ ] Kalibrera i båten: trimkvot i ändlägena, tankkvot vid tom och full tank, vattentryckets nollpunkt och `PULSES_PER_REV`.
- [ ] Skriv koden för vattentemperaturen (GPIO5, Bosch-tabellen), FS-40 och lyftgivaren på J6.
- [ ] `logger.py`: lägg till `tank_level` och `watertemp` i `build_line`.
- [ ] Dashen: ruta för vattentemperatur, `water_max` 2,0 och `trim_min`/`trim_max` 0/100 i config.
- [ ] Byte av vy: HD7T saknar touch mot datorn, så vyerna roterar A/B varje minut. En fysisk knapp på GPIO är ett alternativ.

## Att rätta i BPL V0.1

Schemat granskades 2026-10-25. Kärnkretsarna är rätt; de fyra första punkterna måste rättas före beställning.

- [x] GENERALALARM\_OUT: flytta från GPIO20 (USB D+) till GPIO38.
- [x] 12VDC\_OUT: flytta från GPIO42 (saknar ADC) till GPIO4.
- [x] Döp om porten vid R14 från `12VDC` till `12VDC_IN`, annars når plintens pol 9 aldrig delaren.
- [x] Ge VCC en källa: buckmodul från pol 9 enligt Matning och avkoppling, eller ta bort VCC-flaggan och mata via USB.
- [x] Lägg avkoppling vid GPS- och IMU-headern.
- [x] Extra GND på plinten (pol 9 signal-GND, pol 10 matnings-GND).
- [x] Rita reservkanaler på GPIO6 (orange) och GPIO7 (vattentryck), till pol 6 och 7.
- [x] Döp om designatorer: U3/U6/U7 → C1–C3, U8 → J1, IMU → J2, GPS → J3.
- [x] Kontrollera att D1–D3 i BOM:en är 1N4148 (symbolen är ritad som zener).
- [x] Valfritt: IMU RST till GPIO9, och anslut alla GND-ben på ESP:n.
- [x] Firmware: GPS-TX går till GPIO18, så befintliga `gpsSerial.begin(9600, SERIAL_8N1, 18, 17)`.

Andra granskningen (samma dag):

- [x] Vänd D7 och D9: båda sitter baklänges, så ingen 12 V-matning når ESP:n.
- [x] Byt C5 och C6 (10 nF) mot 10 µF.
- [ ] Kontrollera att C8 är 22 µF 50 V och C9 10 µF i BOM:en (µ syns inte i PDF-exporten).

Tredje granskningen (flödesgivare och reserver):

- [x] Lägg 100 nF mellan J4 pol 1 (+3.3V) och GND, nära plinten. C10 i schemat är 1 nF-filtret på signalen, så avkopplingen saknas.
- [ ] Beställ matchande pluggdelar till J1 (2EMGR-5.08-10P) och J4/J5 (KF2EDGK-5.08, 3 poler).

Fjärde granskningen (tryckgivare):

- [x] J5 pol 1 och C16: byt +3.3V mot `VCC` (5 V). Givaren kräver 5 V.
- [x] Ta bort R27, R28, C15 och D14 och koppla J5 pol 2 direkt till `WATERPRESS_IN`. Pullupen förskjuter mätvärdet och D14 klipper signalen över \~3,6 V, alltså allt över \~19 psi.
- [x] D13: flytta klamringens övre ände från `VCC` till +3.3V. Mot 5 V skyddar den inte ESP:n.

Rev 2 (november 2026). Rev 1 är levererat och fungerar. Rev 2 lägger till vattentemperaturen och rättar detaljer:

- [x] Vattentemperatur på J7 och GPIO5, se avsnittet Vattentemperatur.
- [ ] R30 (pull-up på J7) som 1 kΩ 1 % i BOM:en.
- [ ] J7:s footprint: samma plintserie som J4/J5 (schemat anger KF2EDGV).
- [ ] Flytta de överlappande texterna vid IMU-headern (J7:s värde, C13 och C15).
- [ ] Pinnamn på silkscreen vid J1 och J4–J7 med tillägget Generate Pad Silkscreen (Advanced → Extensions Manager). Korta namn utan å/ä/ö, minst 1 mm höjd och 0,15 mm linje.

Redan rätt: PC817-benordning, diodernas riktning, R1 och R9 680 Ω, R3 2,2 kΩ, de analoga kretsarna och I2C på GPIO13/14. Bränsle sitter på GPIO1 och trim på GPIO2, omvänt mot förra specen; specen följer nu schemat.

## Uppmätta motorsignaler

| Tråd | Signal | Typ | Uppmätt | Ingångskrets |
| --- | --- | --- | --- | --- |
| Svart | Jord | — | — | GND |
| Grön | Tach-/tändpuls | Fyrkantpuls | \~5 V topp, \~200 Hz vid 1000 rpm | Optokopplare (RPM) |
| Grön/vit | Oljelarm | Digital | 0 / 5 V | Optokopplare |
| Grå | Allmänt larm | Digital | 0 / 12 V | Optokopplare |
| Rosa | Trim/tilt | Resistiv givare | 5–300 Ω | Analog, ADC1 |
| Vit | Bränslenivå | Resistiv givare | 2–165 Ω | Analog, ADC1 |
| Orange | Okänd | ? | 226 MΩ mot jord, motor av | Reservingång |

## Rättelser mot tidigare råd

När allt lades ihop föll fem saker jag sagt tidigare. De här gäller före allt annat i chatten.

1. **D1 är 5,6 V, inte 6,2 V.** 1N4734A är en 5,6 V-zener (6,2 V är 1N4735A). Parallellt över LED:en gör den ändå ingen nytta, eftersom LED:en själv klamrar vid \~1,2 V. Den ersätts, se RPM-avsnittet.
2. **R1 (RPM) blir 680 Ω, inte 330 Ω.** PC817 behöver bara några mA för att mätta mot en 10 kΩ last. 680 Ω ger \~5,6 mA vid 5 V och tål spikar bättre.
3. **Galvanisk isolation går inte att få.** ESP:n matas från båtens 12 V och de resistiva givarna måste mätas mot motorjord. Jordarna blir alltså gemensamma ändå. Optokopplarna behålls för nivåanpassning och spikskydd, men ett gemensamt GND-nät räcker.
4. **3,3 V-zener som ADC-skydd byts mot BAT54S.** En 3,3 V-zener läcker redan från \~2 V och förvränger mätningen. En dubbel Schottky (BAT54S) mot 3V3 och GND klamrar utan att påverka signalen.
5. **Spänningsdelaren för trim byggdes för en spänningssignal.** Den visade sig vara en resistiv givare, så R4/R2/R5/R6-kretsen ritas om enligt analog-avsnittet.

## RPM-ingång

RPM läses via PC817 med 680 Ω i serie och 10 kΩ pulldown, signalen icke-inverterad till GPIO12 (PCNT).

| Ref | Komponent | Värde | Från → till |
| --- | --- | --- | --- |
| R1 | Motstånd, 0,25 W | 680 Ω | Grön (tach) → U2 ben 1 |
| U2 | PC817 | — | Ben 1 anod, ben 2 katod, ben 3 emitter, ben 4 kollektor |
| D1 | 1N4148 (ersätter 1N4734A) | — | Över U2 ben 1–2, katod mot ben 1 (backspänningsskydd, PC817 tål bara 6 V back) |
| — | U2 ben 2 | — | → GND |
| — | U2 ben 4 (kollektor) | — | → 3V3 |
| — | U2 ben 3 (emitter) | — | → GPIO12 |
| R2 | Motstånd | 10 kΩ | GPIO12 → GND (pulldown) |

**Kontrollerat i BPL V0.1:** ben 3 (emitter) går till GPIO12 med pulldown och ben 4 (kollektor) till 3V3. Samma gäller U4 och U5.

Tips: en PC847 (fyra kanaler i ett DIP-16) täcker RPM, båda larmen och en reserv i en komponent.

## Larmingångar

Båda larmen kopierar RPM-kretsen; bara seriemotståndet skiljer, dimensionerat för \~5 mA genom LED:en.

| Larm | Tråd | Insignal | Seriemotstånd | Diod över LED | Pulldown | GPIO |
| --- | --- | --- | --- | --- | --- | --- |
| Olja | Grön/vit | 0 / 5 V | 680 Ω | 1N4148 | 10 kΩ | GPIO21 |
| Allmänt | Grå | 0 / 12 V (upp till 14,4 V vid laddning) | 2,2 kΩ, 0,25 W | 1N4148 | 10 kΩ | GPIO38 |

Kollektor till 3V3 och emitter till GPIO med pulldown, precis som RPM. Utgången blir hög när trådens spänning är hög. Vilken nivå som betyder larm (hög eller låg) avgörs i firmware, så kortet behöver inte veta det.

## Analoga ingångar: trim och bränsle

Alt B är valt: originalmätarna sitter kvar. BPL V0.1 har alt B-kretsen för bränsle, trim och batterispänning, och reservkanalerna på GPIO6/7 ritas likadant.

Skälet är en sak vi missade: originalmätarna matar redan ström genom givarna. Om mätarna sitter kvar får du inte mata givaren själv (alternativ A), utan ska läsa den spänning mätaren redan skapar (alternativ B).

### Kretsen (per kanal)

| Ref | Mellan | Alt A: mätaren bortkopplad | Alt B: mätaren kvar |
| --- | --- | --- | --- |
| Rser | Givartråd → ADC-nod | 0 Ω bygel | 39 kΩ |
| Rpull | 3V3 → ADC-nod | 100 Ω, 0,25 W | Ej monterad |
| Rbot | ADC-nod → GND | Ej monterad | 10 kΩ |
| C | ADC-nod → GND | 100 nF | 100 nF |
| D | BAT54S: mittben på ADC-nod, ändar mot 3V3 och GND | Monterad | Monterad |
| Rgpio | ADC-nod → GPIO | 1 kΩ | 1 kΩ |

### Förväntade ADC-spänningar

| Kanal | GPIO | Givare | Alt A | Alt B |
| --- | --- | --- | --- | --- |
| Trim | GPIO2 (ADC1\_CH1) | 5–300 Ω | 0,16–2,48 V | 0,20 × trådspänningen |
| Bränsle | GPIO1 (ADC1\_CH0) | 2–165 Ω | 0,07–2,06 V | 0,20 × trådspänningen |
| Batteri | GPIO4 (ADC1\_CH3) | Pol 9, +12 V | — | 0,20 × batterispänningen |

Alt B klarar upp till 14,4 V på tråden (ger 2,94 V). Alt A drar upp till \~32 mA per kanal från 3V3, vilket devkitens regulator klarar.

Firmware räknar trim och bränsle som kvoten trådspänning / batterispänning. Då slår inte laddspänningen (12,4–14,4 V) igenom i visningen, eftersom mätaren och givaren matas från samma batteri.

**Varning för alt A:** sitter originalmätaren kvar matar den in 12 V via givartråden mot 3V3 genom 100 Ω. Det ger fel värden på båda instrumenten och kan lyfta 3V3-rälsen. Alt A kräver att tråden är bortkopplad från mätaren.

Bränslekanalens låga ände (2 Ω ≈ 0,07 V) hamnar nära ADC:ns golv, så upplösningen blir sämre i den änden. Kontrollera om 2 Ω är full eller tom tank.

## Jord

Ett enda GND-nät på kortet; ta bort tanken på separat MOTOR\_GND.

- Svart motorjord ansluts till kortets GND i en punkt, vid kontaktdonet.
- Givarnas och larmens returer går till samma punkt, inte utspridda över kortet.
- Dra ADC-kanalernas GND (Rbot, C, BAT54S) som ett eget spår till den punkten, skilt från WiFi-modulens och optokopplarnas jordström.

## Matning och avkoppling

ESP:n matas från båtens 12 V via en färdig buckmodul till 5V-benet. I BPL V0.1 har VCC-nätet ännu ingen källa.

| Steg | Komponent | Exempel | Varför |
| --- | --- | --- | --- |
| 1 | Säkring | MF-R050 (F1) på kortet + 2 A flatstiftssäkring i kabeln | Kortslutning på kortet ska lösa ut, inte brinna |
| 2 | Backströmsskydd | Schottky SS34 (D7) i serie, katod mot TSR | Felvänd matning |
| 3 | TVS mot GND | SMBJ18A | Klipper spikar från startmotor och trimpump; 14,4 V laddning ligger under 18 V |
| 4 | Buck 12 → 5 V | Traco TSR 1-2450 (U3), 6,5–36 V in, 1 A | Linjärregulator blir för varm vid 12 V → 5 V |
| 5 | Schottky | SS34 (D9), anod på TSR:ens utgång, katod mot 5V-benet | Bucken och USB:ns 5 V matar inte mot varandra |

Batteridelaren (R14) tas direkt från pol 9, före dioden, så den mäter verklig batterispänning. Felvänd matning ger bara \~0,3 mA genom 39 kΩ, som BAT54S klamrar.

### Avkoppling

| Var | Kondensatorer | Mellan |
| --- | --- | --- |
| GPS-header (J3) | 10 µF + 100 nF | ben 1 (VCC) och ben 2 (GND) |
| IMU-header (J2) | 100 nF, valfritt + 10 µF | ben 10 (VCC) och ben 9 (GND) |
| ESP 5V-ben | 10 µF | 5V och GND |
| Buckens ingång | 10 µF, 50 V keramisk | efter TVS, mot GND |

Placera dem så nära benen som möjligt, 100 nF närmast, med en via direkt ner i jordplanet. Keramiska X5R/X7R i 0805 räcker.

## Bränsleflöde: FlowTrecs FS-40

FS-40 mäter flödet optiskt med en rotor och ger 0–200 Hz pulser. Den sitter på egen plint J4 och läses med pulsräknaren (PCNT) på GPIO11. D150H är förgasad utan returledning, så en givare mäter hela förbrukningen.

| Egenskap | Värde |
| --- | --- |
| Matning | 3–5 V (kortet matar 3,3 V) |
| Utsignal | Puls; open collector eller push-pull anges inte |
| Mätområde | 0,5–150 l/h |
| Noggrannhet | ±5 % |
| Temperatur | 0–40 °C |
| Tryck | −1 till +1 bar |
| Slanganslutning | 9,5 mm (FS-40-10) eller 12 mm (FS-40-12) |
| Vätskor | Bensin, diesel, vatten, alkohol |
| Kabel | 0,6 m PVC med 3,5 mm-plugg |

### Krets på J4

Kretsen klarar både open collector och push-pull, och även 5 V-matning om det behövs senare.

| Ref | Värde | Mellan |
| --- | --- | --- |
| J4 pol 1 | — | +3.3V (givarens VCC) |
| J4 pol 2 | — | Givarens signal → R16 |
| J4 pol 3 | — | GND |
| R16 | 1 kΩ | J4 pol 2 → `FUEL_FLOW_IN` (strömbegränsning) |
| R17 | 10 kΩ | `FUEL_FLOW_IN` → +3.3V (pullup) |
| C10 | 1 nF | `FUEL_FLOW_IN` → GND (störfilter, τ = 1 µs) |
| D10 | BAT54S | Mittben på `FUEL_FLOW_IN`, ändar mot GND och 3V3 |
| Ny | 100 nF | J4 pol 1 → GND, nära plinten (saknas ännu) |

Med open collector drar givaren signalen till \~0,3 V genom 1k mot 10k, vilket är en säker nolla.

**Innan pluggen klipps:** mät med multimeter vilken ledare som går till spets, ring och hylsa. Databladets ritning ser ut att visa GND på spetsen, signal på ringen och VCC på hylsan, men den är otydlig. Fel polaritet kan förstöra givaren.

### Munstycke

Givaren finns med fem munstycken. Storleken avgör både mätområdet och hur många pulser du får per liter, och den står på givarens kropp (S, M, L, XL eller XXL). Värdena nedan är avlästa ur databladets bensingraf och är ungefärliga.

| Variant | Munstycke | ca l/h vid 100 Hz | ca l/h vid 200 Hz | ca pulser/liter vid 200 Hz | ca Hz vid 70 l/h | För D150H |
| --- | --- | --- | --- | --- | --- | --- |
| S | 1,2 mm | 15 | 27 | 26 700 | över 200 | Nej, för litet |
| M | 2,0 mm | 26 | 52 | 13 800 | över 200 | Nej, slår i taket före full gas |
| L | 3,0 mm | 47 | 89 | 8 100 | \~155 | Ja, bäst upplösning |
| XL | 4,0 mm | 78 | 151 | 4 800 | \~90 | Ja, mer marginal och mindre strypning |
| XXL | 4,5 mm | 96 | \~190 | 3 800 | \~70 | Fungerar, men toppen ligger över specens 150 l/h |

- **Rekommendation:** L om toppförbrukningen ligger under \~85 l/h; med `fuel_max` 70 l/h hamnar full gas runt 155 Hz. XL om du vill ha marginal.
- **Strypning:** ett mindre munstycke stryper sugledningen mer. Databladet anger inget tryckfall. Hackar motorn vid full gas efter monteringen, byt till ett större munstycke.
- **Kurvorna är inte raka linjer genom noll.** Pulser per liter ändras med flödet, särskilt för S och M. En konstant K-faktor räcker därför inte; firmware använder en tabell.
- **Låga flöden:** vid tomgång (några l/h) blir det bara några Hz med L och XL. Mät då periodtiden eller medelvärdet över flera sekunder.

### Montering

- I bränsleslangen inne i båten, efter filter/vattenavskiljare och före primerbollen. Inte under motorkåpan: givaren tål bara 40 °C.
- "TOP SIDE" uppåt och flödespilen mot motorn.
- FS-40-10 passar 3/8" (9,5 mm) bränsleslang.

### Kalibrering

1. Lägg in ditt munstyckes kurva som en tabell med (Hz, l/h)-punkter i config; firmware interpolerar linjärt.
2. Summera förbrukade liter som l/h × tid, inte pulser × en fast faktor.
3. Vid tankning: jämför tankade liter med loggade liter och justera med en korrektionsfaktor. Databladets ±5 % blir då betydligt bättre.

## Vattentryck: 5 V-tryckgivare

Givaren ger 0,5–4,5 V linjärt över sitt tryckområde. Den sitter på egen plint J5, matas med 5 V och läses analogt på GPIO10 (ADC1\_CH9) genom en delare på 0,64.

| Egenskap | Värde |
| --- | --- |
| Matning | 5 V DC |
| Utsignal | 0,5–4,5 V linjär (0 psi = 0,5 V, halva skalan = 2,5 V, full skala = 4,5 V) |
| Tryckområde | 30, 100 eller 150 psi; 30 psi rekommenderas för kylvatten |
| Noggrannhet | ±2 % FS |
| Överlast | 2–4 × nominellt tryck |
| Svarstid | ≤ 1 ms |
| Temperatur | −40 till +120 °C drift, kompenserad 0–80 °C |
| Gänga | 1/8"-27 NPT, rostfritt 316L, IP67 |
| Ledare | Röd +5 V, svart GND, blå signal |

### Krets

| Ref | Värde | Mellan |
| --- | --- | --- |
| J5 pol 1 | — | `VCC` (5 V från TSR), röd ledare |
| J5 pol 2 | — | `WATERPRESS_IN`, blå ledare (direkt, utan pullup eller klamring) |
| J5 pol 3 | — | GND, svart ledare |
| C16 | 100 nF | `VCC` → GND, nära J5 |
| R25 | 10 kΩ | `WATERPRESS_IN` → ADC-nod |
| R24 | 18 kΩ | ADC-nod → GND |
| C14 | 100 nF | ADC-nod → GND |
| D13 | BAT54S | Mittben på ADC-noden, ändar mot GND och +3.3V |
| R26 | 1 kΩ | ADC-nod → `WATERPRESS_OUT` → GPIO10 |

| Givarens utsignal | Tryck, 30 psi-givare | På ADC:n |
| --- | --- | --- |
| 0,5 V | 0 psi | 0,32 V |
| 2,5 V | 15 psi (1,03 bar) | 1,61 V |
| 4,5 V | 30 psi (2,07 bar) | 2,89 V |
| 5,0 V (kortslutning mot +5 V) | — | 3,21 V |

Givaren matas bara när 12 V är på, eftersom `VCC` är TSR:ens utgång. Den totala lasten på 28 kΩ klarar givaren.

### Montering och beräkning

- Montera i motorns vattentrycksuttag eller i ett T-stycke på kontrollstrålens slang, med adapter till 1/8"-27 NPT.
- Med 30 psi-givaren slutar skalan på 2,07 bar; sätt dashens Vatten-skala till 0–2 bar.
- `v_sensor = v_adc / 0.643`, `psi = (v_sensor − 0.5) / 4.0 × 30`, `bar = psi × 0.0689`.
- Under 0,3 V på givaren = avbrott eller kortslutning mot GND; över 4,7 V = kortslutning mot 5 V. Visa fel i stället för ett tryck.
- Läs nollpunkten med motorn avstängd och spara den, så tar du bort felet från ADC:n och 5 V-matningens tolerans.

## Lyftgivare: magnetisk kvadraturgivare (J6)

Givaren ger kvadratursignal (A, B) och en indexpuls (R) med 5 V TTL. Den ansluts på J6 (KF2EDGVC-5.08-05P, 5 poler). R används inte i dag och är inte kopplad.

| Del | Värde | Not |
| --- | --- | --- |
| Matning | J6 pol 5, +5 V (VCC) | C17 100 nF + C16 10 µF vid kontakten |
| Serieresistans | R27, R28 2,2 kΩ | På A och B |
| Klämma | D14, D15 BAT54S | Mot GND och +3.3V, noden hamnar på ca 3,5 V |
| Ingångar | GPIO15 (A), GPIO16 (B) | PCNT i kvadraturläge |

- Ingen spänningsdelare: 2,2 kΩ + BAT54S begränsar strömmen till ca 0,6 mA när utgången är 5 V. Har givaren öppen kollektor i stället för push-pull behövs en pullup.
- Givaren är inkrementell och tappar positionen vid avstängning. Använd R-pulsen som referens vid ett känt läge, eller spara positionen i flash.
- Mekaniskt: kropp 35 × 25 × 10 mm, två fästlånghål (R1,6) med 17 mm mellan hålen, kabeln ut på kortsidan.

## Vattentemperatur: NTC-givare (J7)

Kylvattnets temperatur mäts med en isolerad tvåpolig NTC-givare från Bosch på J7 och läses analogt på GPIO5 (ADC1\_CH4). Kretsen finns i rev 2, inte i rev 1.

| Ref | Värde | Mellan |
| --- | --- | --- |
| Givare | Bosch 0 280 130 037, M14x1,5, 2 poler | Isolerad från gängan. Alternativ: 0 280 130 026 (M12x1,5) med publicerad tabell |
| J7 pol 1 | — | GND |
| J7 pol 2 | — | `WATERT_IN` |
| R30 | 1 kΩ 1 %, 0805 | +3.3V → `WATERT_IN` (pull-up) |
| C18 | 100 nF | `WATERT_IN` → GND |
| D23 | BAT54S | Mittben på `WATERT_IN`, ändar mot GND och +3.3V |
| R31 | 1 kΩ | `WATERT_IN` → `WATERT_OUT` → GPIO5 |

- Resistans: `R = 1000 × V / (3,3 − V)`, där V är spänningen på ADC-noden. Strömmen blir högst cirka 3 mA.
- Boschs standardkurva (0 280 130 026, [datablad](https://cdn.shopify.com/s/files/1/0057/7712/9587/files/Temperature_Sensor_NTC_M12_Datasheet_51_en_2782569739.pdf)): 0 °C 5 896 Ω, 20 °C 2 500 Ω, 40 °C 1 175 Ω, 60 °C 596 Ω, 80 °C 323 Ω, 90 °C 243 Ω, 100 °C 187 Ω, 120 °C 113 Ω. Firmware interpolerar i log(R).
- Kurvan för 0 280 130 037 är inte bekräftad. Mät givaren i isvatten (cirka 5,9 kΩ) och kokande vatten (cirka 187 Ω); avviker den räknas en egen kurva ur mätpunkterna.
- Montering: T-stycke på kontrollstrålens slang eller vid termostathuset. Yamahas egen överhettningsgivare sitter kvar.
- Skickas som `watertemp` i °C.

## Pinnplan ESP32-S3 (N16R8)

Analoga signaler bara på ADC1 (GPIO1–10), eftersom ADC2 inte fungerar med WiFi påslaget.

| GPIO | Funktion | Kommentar |
| --- | --- | --- |
| 1 | Bränsle (ADC1\_CH0) | Analog, alt B |
| 2 | Trim (ADC1\_CH1) | Analog, alt B |
| 4 | Batterispänning (ADC1\_CH3) | Analog, för kvotberäkning |
| 5 | Vattentemperatur (ADC1\_CH4) | Analog, NTC via 1 kΩ pull-up, J7 pol 2 |
| 12 | RPM (PCNT) | Digital, flyttad från GPIO5 |
| 6 | RESERV1 (ADC1\_CH5) | Analog, J1 pol 6 |
| 7 | RESERV2 (ADC1\_CH6) | Analog, J1 pol 7 |
| 8 | GPS PPS | Digital |
| 9 | IMU RST | Digital |
| 11 | Bränsleflöde FS-40 (PCNT) | Digital, J4 pol 2 |
| 13 / 14 | IMU SDA / SCL | I2C, adress 0x4B |
| 17 / 18 | GPS RX ← GPIO17, GPS TX → GPIO18 | ESP RX = 18, TX = 17 |
| 21 | Oljelarm | Digital |
| 38 | Allmänt larm | Digital |
| 10 | Vattentryck (ADC1\_CH9) | Analog via 10k/18k, J5 pol 2 |
| 15 | Lyft A (PCNT, kvadratur) | Digital, J6 pol 4 |
| 16 | Lyft B (PCNT, kvadratur) | Digital, J6 pol 3 |

Undvik: 0, 3, 45, 46 (strapping), 19/20 (USB), 43/44 (UART0), 35–37 (upptagna av PSRAM på N16R8) och 48 (RGB-LED).

## Kontaktdon mot kabelstammen

J1 är en 10-polig pluggbar skruvplint med 5,08 mm raster (2EMGR-5.08-10P). Flödesgivaren och tryckgivaren sitter på var sin 3-polig plint, J4 och J5 (KF2EDGK-5.08).

| Plint | Pol | Tråd | Signal |
| --- | --- | --- | --- |
| J1 | 1 | Grön/vit | Oljelarm |
| J1 | 2 | Vit | Bränsle |
| J1 | 3 | Rosa | Trim |
| J1 | 4 | Grå | Allmänt larm |
| J1 | 5 | Grön | RPM |
| J1 | 6 | Orange | RESERV1 (analog, GPIO6) |
| J1 | 7 | — | RESERV2 (analog, GPIO7) |
| J1 | 8 | Röd | +12 V, matning och batterimätning |
| J1 | 9 | Svart | GND, signalretur (stjärnpunkt) |
| J1 | 10 | — | GND, matningsretur |
| J4 | 1 | FS-40 VCC | +3.3V |
| J4 | 2 | FS-40 signal | Pulser till GPIO11 |
| J4 | 3 | FS-40 GND | GND |
| J5 | 1 | Röd | +5 V (VCC) |
| J5 | 2 | Blå | Vattentryck, 0,5–4,5 V |
| J5 | 3 | Svart | GND |
| J6 | 1 | Ej mätt | GND (0 V) |
| J6 | 2 | — | R (index), ej ansluten |
| J6 | 3 | Ej mätt | B, via R28 till GPIO16 |
| J6 | 4 | Ej mätt | A, via R27 till GPIO15 |
| J6 | 5 | Ej mätt | +5 V (VCC) |
| J7 | 1 | Vattentempgivare | GND |
| J7 | 2 | Vattentempgivare | WATERT\_IN, NTC till GPIO5 (rev 2) |

**TVS-skydd:** J1 pol 1–7 har var sin SMAJ15A (D16–D22) mot GND, katoden mot signalen. De sitter nära plinten och har var sin kort GND-via.

## Komponentval (BOM)

J-komponenterna monteras inte av JLC. De köps från LCSC i samma beställning (samma konto, samfrakt) och löds för hand.

| Ref | Val | LCSC | Not |
| --- | --- | --- | --- |
| D1–D3 | 1N4148WS, SOD-323 | C2128 | Basic |
| D16–D22 | SMAJ15A | — | TVS på J1 pol 1–7 |
| 100 nF | 0805 | C49678 | Basic |
| C10 | 1 nF | C46653 | Basic |
| C7 | 22 µF 25 V, 1206 | C12891 |  |
| R1, R3, R9 | 1206 | — | Effekt; R3:s värde skrivs "2.2kΩ" |
| U3 | TSR 1-2450-klon (YLPTEC) | — |  |
| D8 | SMBJ18A (SPSEMI) | — | Kontrollera polariteten, alternativt C10212 |
| R30 | 1 kΩ 1 %, 0805 | — | Rev 2, pull-up för vattentemp |
| J1, J4–J7 | — | — | Do not place, löds för hand |

## Öppna punkter

- [x] Bestäm om originalmätarna sitter kvar. Det väljer alt A eller B för trim och bränsle.
- [ ] Om de sitter kvar: mät DC på rosa och vit mot svart med tändning på, i båda ändlägena. Bekräftar att 39k/10k räcker.
- [ ] Bekräfta att trimgivarens 5–300 Ω gäller rosa mot svart. Satt givaren mellan rosa och orange behöver kopplingen ändras.
- [ ] Ta reda på vad orange är: DC mot jord med motorn igång, och skopa den.
- [ ] Mät grön/vit och grå i larmläge om det går, för att veta vilken nivå som är larm.
- [ ] Kontrollera i datablad: PC817X2NSZ9F:s benordning (ben 3 emitter, ben 4 kollektor).
- [ ] FS-40: mät vilken ledare som går till spets, ring och hylsa innan pluggen klipps.
- [ ] FS-40: kontrollera munstycksstorleken på givarens kropp och lägg in rätt kurva i config.
- [x] Rätta C16 i schemat från 100 pF till 100 nF och lägg 10 µF parallellt.
- [ ] J6: mät ledarfärgerna för +5 V, GND, A och B innan pluggen kopplas, och kontrollera att utgången är push-pull och inte öppen kollektor.
- [ ] J6: köp plugg för KF2EDGVC-5.08-05P (5 poler).

## Firmware som följer med

- `PULSES_PER_REV = 12` tills vidare. Mätningarna gav 12 vid 1000 rpm men \~15 vid 2000 rpm, så kalibrera mot originalmätaren när kretsen sitter.
- RPM GPIO12, vattentemperatur GPIO5, olja GPIO21, allmänt larm GPIO38, bränsle GPIO1, trim GPIO2, batteri GPIO4.
- GPS: `gpsSerial.begin(9600, SERIAL_8N1, 18, 17)` (RX = 18, TX = 17), eftersom GPS-modulens TX går till GPIO18. IMU: BNO\_RESET = 9.
- ADC med 11 dB dämpning och medelvärde över flera läsningar; trim och bränsle räknas som kvot mot batterispänningen och omvandlas till grader och procent via kalibreringstabell i ändlägena.
- Larmens aktiva nivå sätts i config när den är uppmätt.
- Bränsleflöde: PCNT på GPIO11 med glitchfilter på några µs. Räkna pulser per sekund; under \~10 Hz mäts periodtid eller medelvärde över flera sekunder. l/h via munstyckets tabell i config, förbrukade liter som l/h × tid. Ger `fuel` (l/h) och underlag för tanknivån på dashen.
- Reserver: RESERV1 på GPIO6 och RESERV2 på GPIO7, samma ADC-inställning som trim och bränsle.
- Vattentryck: ADC på GPIO10, omräkning och felgränser enligt avsnittet Vattentryck. Skickas som `waterpressure` i bar.
- Lyft: PCNT i kvadraturläge på GPIO15 (A) och GPIO16 (B), glitchfilter på några µs. Positionen räknas från ett nollläge; R-pulsen (J6 pol 2, ej kopplad än) kan nollställa. Kan ersätta ESPNow-givaren som källa till lyfthöjden.

Tillagt i november 2026:

- Analoga kanaler i `analog.cpp`: 16 läsningar per mätning med `analogReadMilliVolts`, utjämning med EMA och 20 Hz utan att blockera loopen. Trim och tank räknas som kvot mot batteriet och mappas linjärt mellan två kalibreringspunkter som mäts i båten.
- Tanknivån har ALPHA 0,01 (cirka 5 s) mot skvalp.
- Vattentryck: `DIV_WATER_PRESSURE` = 28/18; under 0,3 V eller över 4,7 V skickas null.
- UDP-fält: `battery`, `trim`, `tank_level`, `waterpressure`, `watertemp`. NaN skickas som null, så visar dashen ett streck. JSON-bufferten är 512 byte.
- USB: `Serial.setTxTimeoutMs(0)` och ingen `while (!Serial)`, annars startar ESP:n inte utan USB-kabel.
- `analogSetPinAttenuation` fungerar först efter en första `analogRead` på pinnen.
- Pi: `logger.py` skriver ett fält per nyckel och hoppar över null och NaN. Nyckeln `battery` loggas som `batteryvoltage` i Influx.
