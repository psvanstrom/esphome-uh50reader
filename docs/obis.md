# OBIS

![OBIS](obis.jpg)

OBIS (**Ob**ject **I**dentification **S**ystem) Kennzahlen (ehemals EDIS) werden zur eindeutigen Identifikation von Messwerten (Energiemengen, Zählerstände) und auch abstrakter Daten beim elektronischen Datenaustausch zwischen den beteiligten Kommunikationspartnern in verschiedenen Nachrichtentypen (MSCONS, UTILMD) verwendet.

Der Dreierblock vor dem Stern (z.B. 1.8.0*) nennt uns Messgröße.Messart.Tarif*.

## Auslesen der Zähler

Der [vzlogger](software:controller:vzlogger) zeigt bei höchster Detailstufe (`"verbosity":15,`) im Logfile alle verfügbaren Daten eines [SML](software:sml) sprechenden Zählers an.
Mit Hilfe des [libsml](https://github.com/volkszaehler/libsml) beiliegenden `sml_server` kann man die OBIS aus einem SML-Telegramm extrahieren: `cat /dev/stromzaehler0 | ./sml_server -`
gonium hat etwas C-Code bereitgestellt um die Daten via RS232 auszulesen: [libehz](https://github.com/gonium/libehz).
Für node.js existiert auch ein kleines Tool: [nodeehz](https://github.com/sunny5/nodeehz)

Es ist nicht ungewöhnlich das ein Zähler OBIS Codes ausgibt die gar nicht in Gebrauch sind. Die meisten Zähler listen z.B. zwei Tarife (x.x.1 und x.x.2), gezählt wird aber nur einer. Für Volkszähler ist der Gesamtwert (x.x.0) am ehesten zu gebrauchen.

## Beispiele

Diese Smartmeter benutzen die OBIS Kennzahlen nur um verschieden Messgrößen zu kennzeichnen.
Die Übertragenen Daten sind selbst **kein** OBIS sondern [SML](software:sml), ein zeilenbasiertes Protokoll das in [DIN EN 62056-21](development:standards) beschrieben ist.

### Iskraemeco MT175

| OBIS-Kennzahl | Bezeichung | Kanaltyp | Auflösung | Hinweis |
| --- | --- | --- | --- | --- |
| `1-0:0.0.1*255` | Seriennummer | | | |
| `1-0:0.0.9*255` | "Geräteeinzelidentifikation" | | | |
| `1-0:1.8.0*255` | Zählerstand Bezug | El. Energie (Zählerstände) | 1000 | Summe aus 1.8.1 und 1.8.2 |
| `1-0:1.8.1*255` | Zählerstand Bezug Tarif 1 | El. Energie (Zählerstände) | 1000 | |
| `1-0:1.8.2*255` | Zählerstand Bezug Tarif 2 | El. Energie (Zählerstände) | 1000 | |
| `1-0:2.8.0*255` | Zählerstand Lieferung | El. Energie (Zählerstände) | 1000 | Summe aus 2.8.1 und 2.8.2 |
| `1-0:2.8.1*255` | Zählerstand Lieferung Tarif 1 | El. Energie (Zählerstände) | 1000 | |
| `1-0:2.8.2*255` | Zählerstand LieferungTarif 2 | El. Energie (Zählerstände) | 1000 | |
| `1-0:16.7.0*255` | Leistung (Momentan) | El. Energie (Leistungswerte) | | Summe aus 36.7.0, 56.7.0 und 76.7.0 |
| `1-0:36.7.0*255` | Leistung L1 (Momentan) | El. Energie (Leistungswerte) | | |
| `1-0:56.7.0*255` | Leistung L2 (Momentan) | El. Energie (Leistungswerte) | | |
| `1-0:76.7.0*255` | Leistung L3 (Momentan) | El. Energie (Leistungswerte) | | |
| `129-129:199.130.3*255` | Herstelleridentifikation | | | Immer 3 Zeichen, z.B. "ISK" |
| `129-129:199.130.5*255` | Public Key | | | Zum Prüfen der Signatur der übermittelten Daten |

### Easymeter Q3D

| OBIS-Kennzahl | Bezeichung | Kanaltyp | Auflösung | Hinweis |
| --- | --- | --- | --- | --- |
| `0:0.0.0*255(54920)` | Eigentumsnr. | | | |
| `1-0:1.8.0*255(00003984.9338242*kWh)` | Zählerstand Bezug | El. Energie (Zählerstände) | 1 | |
| `1-0:21.7.255*255(000176.85*W)` | Wirkleistung L1 Bezug (Momentan) | | | |
| `1-0:41.7.255*255(001025.66*W)` | Wirkleistung L2 Bezug (Momentan) | | | |
| `1-0:61.7.255*255(002529.68*W)` | Wirkleistung L3 Bezug (Momentan) | | | |
| `1-0:1.7.255*255(003732.19*W)` | Leistung (Momentan) | El. Energie (Leistungswerte) | | |
| `1-0:96.5.5*255(82)` | Status | | | hexadezimal |
| `0-0:96.1.255*255(1ESY0943000XXX)!` | | | | |

### Easymeter Q3M

| OBIS-Kennzahl | Bezeichung | Kanaltyp | Auflösung | Hinweis |
| --- | --- | --- | --- | --- |
| `1-0:1.8.0*255` | Zählerstand Bezug | El. Energie (Zählerstände) | 1000 | |
| `1-0:1.8.1*255` | Zählerstand Bezug | El. Energie (Zählerstände) | 1000 | Tarif 1 |
| `1-0:1.8.2*255` | Zählerstand Bezug | El. Energie (Zählerstände) | 1000 | Tarif 2 |
| `1-0:2.8.0*255` | Zählerstand Lieferung | El. Energie (Zählerstände) | 1000 | |
| `1-0:16.7.0*255` | Leistung (Momentan) | El. Energie (Leistungswerte) | | Summe aus P_L1, P_L2, P_L3 |
| `1-0:36.7.0*255` | Leistung an L1 (Momentan) | El. Energie (Leistungswerte) | | |
| `1-0:56.7.0*255` | Leistung an L2 (Momentan) | El. Energie (Leistungswerte) | | |
| `1-0:76.7.0*255` | Leistung an L3 (Momentan) | El. Energie (Leistungswerte) | | |

### EMH eHZ FW8E2A500AK1

| OBIS-Kennzahl | Bezeichung | Kanaltyp | Auflösung | Hinweis |
| --- | --- | --- | --- | --- |
| `1-0:0.0.0*255(123456-1234567)` | Eigentumsnr. | | | |
| `1-0:1.8.1*255(068673.6968)` | Zählerstand Bezug Tarif 1 | El. Energie (Zählerstände) | 1000 | |
| `1-0:96.5.5*255(82)` | Status | | | hexadezimal |
| `0-0:96.1.255*255(0005555555)` | Fabriknummer | | | |

### Hager eHZ

| OBIS-Kennzahl | Bezeichung | Kanaltyp | Auflösung | Hinweis |
| --- | --- | --- | --- | --- |
| `1-0:0.0.0*255(1234567890123456)` | Eigentumsnr. | | | |
| `1-0:1.8.0*255(000001.2963)` | Zählerstand Bezug | El. Energie (Zählerstände) | 1000 | |
| `1-0:96.5.5*255(82)` | Status | | | hexadezimal |
| `0-0:96.1.255*255(0000067865)` | | | | |
| `1-0:52.7.0*255(228.38*V)` | Spannung L2 (Momentan) | | | |
| `1-0:51.7.0*255(000.16*A)` | Strom L2 (Momentan) | | | |
| `1-0:41.7.0*255(+00026*W)` | Wirkleistung L2 Bezug (Momentan) | | | |
| `1-0:96.50.0*0(45)` | | | | |
| `1-0:96.50.0*1(07D0)` | | | | Netzfrequenz? |
| `1-0:96.50.0*2(16)` | | | | Zählertemperatur? |
| `1-0:96.50.0*3(11)` | | | | |
| `1-0:96.50.0*4(1F)` | | | | |
| `1-0:96.50.0*5(18)` | | | | |
| `1-0:96.50.0*6(003D381B070AF6B0CF05140900009F80)` | | | | |
| `1-0:96.50.0*7(00)` | | | | |

### DZG_DVSB20.2

| OBIS-Kennzahl | Bezeichung | Kanaltyp | Auflösung | Hinweis |
| --- | --- | --- | --- | --- |
| `0:0.0.0*255(54920)` | Eigentumsnr. | | | |
| `1-0:1.8.0*255` | Zählerstand Bezug | El. Energie (Zählerstände) | 0.0001 | |
| `1-0:2.8.0*255` | Zählerstand Lieferung | El. Energie (Zählerstände) | 0.0001 | |
| `1-0:16.7.0*255` | Leistung (Momentan) | El. Energie (Leistungswerte) | 0.01 | Wirkleistung L1+L2+L3 Bezug (Momentan)|

## Quellen
* http://de.wikipedia.org/wiki/OBIS-Kennzahlen
* https://www.promotic.eu/en/pmdoc/Subsystems/Comm/PmDrivers/IEC62056_OBIS.htm
