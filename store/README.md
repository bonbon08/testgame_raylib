# Jumper Chicken im Play Store veröffentlichen

Alles, was in diesem Ordner liegt, ist für den Store-Eintrag vorbereitet:

| Datei | Wofür |
|---|---|
| `icon-512.png` | App-Symbol (512×512) |
| `feature-graphic.png` | Feature-Grafik / Banner (1024×500) |
| `screenshots/*.png` | Smartphone-Screenshots (1920×1080) |
| `listing.md` | Texte für den Store-Eintrag (Deutsch + Englisch) |
| `app-ads.txt` | Vorlage für AdMob (siehe Schritt 4) |
| `../docs/privacy-policy.html` | Datenschutzerklärung (vorher Platzhalter ausfüllen!) |

Die Schritte unten musst du selbst erledigen, weil sie dein Google-Konto, deine
Zahlungs- und Steuerdaten und deinen Signierschlüssel betreffen.

---

## 1. Konten anlegen

1. **Google Play Console**: https://play.google.com/console – einmalig 25 US-$, danach
   Identitätsprüfung (Ausweis). Kontotyp „Privatperson“ oder „Organisation“.
2. **AdMob**: https://admob.google.com – mit demselben Google-Konto. Unter
   *Zahlungen* Adresse, Steuerinfos und Bankverbindung eintragen (Auszahlung ab ca. 70 €).

> **Wichtig für neue private Entwicklerkonten:** Google verlangt vor der Veröffentlichung
> einen **geschlossenen Test mit mindestens 12 Testern über 14 Tage am Stück**. Erst danach
> kannst du „Produktionszugriff“ beantragen. Such dir also früh Freunde/Familie mit
> Android-Handy, die mitmachen.

> **Händlerstatus (EU):** Die Play Console fragt, ob du „Händler“ bist. Wer mit der App Geld
> verdient (auch über Werbung), ist in der Regel Händler – dann werden **Name, Adresse,
> Telefonnummer und E-Mail öffentlich** im Store angezeigt. Überleg dir vorher, welche
> Adresse du dafür verwenden willst.

## 2. Upload-Schlüssel erstellen (einmalig)

Im Terminal, ein eigenes Passwort wählen und **gut aufheben** (z. B. im Passwortmanager).
Den Schlüssel nie ins Git-Repo legen:

```bash
keytool -genkeypair -v -keystore ~/jumperchicken-upload.jks -alias upload -keyalg RSA -keysize 2048 -validity 10000
```

SHA-1 anzeigen (brauchst du für Play Games):

```bash
keytool -list -v -keystore ~/jumperchicken-upload.jks -alias upload
```

## 3. AdMob einrichten

1. *Apps → App hinzufügen* → Android, „Noch nicht in einem Store veröffentlicht“.
2. *Anzeigenblöcke → Anzeigenblock hinzufügen → Prämienanzeige* (Rewarded).
   Belohnung z. B. „10 Coins“.
3. *Datenschutz und Mitteilungen → DSGVO* → Mitteilung erstellen, für die App
   veröffentlichen (das ist der Einwilligungsdialog im Spiel).
4. App-ID (`ca-app-pub-…~…`) und Anzeigenblock-ID (`ca-app-pub-…/…`) notieren.

## 4. Website, Datenschutzerklärung und app-ads.txt

1. In `docs/privacy-policy.html` und `docs/index.html` alle `[PLATZHALTER]` ausfüllen,
   committen und pushen.
2. GitHub → Repository → *Settings → Pages* → Source „Deploy from a branch“, Branch
   `main`, Ordner `/docs`. Danach ist die Seite erreichbar unter
   `https://bonbon08.github.io/testgame_raylib/` und die Datenschutzerklärung unter
   `https://bonbon08.github.io/testgame_raylib/privacy-policy.html`.
3. **app-ads.txt** muss im Hauptverzeichnis der Domain liegen. Dafür ein neues öffentliches
   Repository **`bonbon08.github.io`** anlegen, `store/app-ads.txt` hineinlegen, deine
   Publisher-ID (`pub-…`, steht in AdMob unter *Einstellungen*) eintragen und Pages
   aktivieren. Im Store-Eintrag als Website `https://bonbon08.github.io` angeben.

## 5. Release bauen

IDs und Schlüssel in `~/.gradle/gradle.properties` eintragen (nicht ins Repo!):

```properties
jumperchicken.admobAppId=ca-app-pub-XXXXXXXXXXXXXXXX~YYYYYYYYYY
jumperchicken.admobRewardedId=ca-app-pub-XXXXXXXXXXXXXXXX/ZZZZZZZZZZ
jumperchicken.keystore=/home/bonbon/jumperchicken-upload.jks
jumperchicken.keystorePassword=DEIN_PASSWORT
jumperchicken.keyAlias=upload
jumperchicken.keyPassword=DEIN_PASSWORT
```

Dann:

```bash
cd android && ./gradlew bundleRelease
```

Das Ergebnis liegt in `android/app/build/outputs/bundle/release/app-release.aab`.
Fehlt etwas, bricht der Build mit einer Meldung ab – so landet nie aus Versehen eine
Version mit Test-Werbung im Store. Für jedes Update in `android/app/build.gradle.kts`
`versionCode` um 1 erhöhen.

## 6. App in der Play Console anlegen

*App erstellen* → Name „Jumper Chicken“, Spiel, kostenlos. Dann unter *App-Inhalte*:

| Abschnitt | Antwort |
|---|---|
| Datenschutzerklärung | URL aus Schritt 4 |
| App-Zugriff | Alle Funktionen ohne Anmeldung nutzbar |
| Werbung | **Ja**, die App enthält Werbung |
| Einstufung (IARC-Fragebogen) | Ehrlich beantworten: Cartoon-Huhn, das an Stacheln „stirbt“, kein Blut, keine Käufe, keine Chats |
| Zielgruppe | 13 Jahre und älter (nicht für Kinder) |
| Datensicherheit | siehe Tabelle unten |
| Werbe-ID | **Ja**, die App nutzt die Werbe-ID (für Werbung) |

**Datensicherheit** – die App selbst sendet nichts, aber die Google-SDKs tun es.
Vor dem Ausfüllen Googles aktuelle Angaben prüfen:
[AdMob-Angaben](https://developers.google.com/admob/android/privacy/play-data-disclosure)
und [Play Games Services](https://developer.android.com/games/pgs/data-collection).
Typischerweise:

| Datentyp | Erhoben | Geteilt | Zweck |
|---|---|---|---|
| Ungefährer Standort (über IP) | ja | ja | Werbung, Betrugsprävention |
| Geräte- oder andere IDs (Werbe-ID) | ja | ja | Werbung, Analyse, Betrugsprävention |
| App-Interaktionen | ja | ja | Werbung, Analyse |
| Absturzprotokolle / Diagnose | ja | ja | Analyse |
| Spielername, Punktzahlen (Play Games) | ja, optional | nein | App-Funktion |

Daten werden verschlüsselt übertragen: **ja**. Löschung: Spielstand wird mit der App
gelöscht; Play-Games-Daten über das Google-Konto.

## 7. Store-Eintrag

*Hauptinhalte des Store-Eintrags*: Texte aus `listing.md`, Icon, Feature-Grafik und
mindestens 2 Screenshots aus diesem Ordner hochladen. Kategorie: *Spiele → Arcade*.
Kontakt-E-Mail und Website angeben.

## 8. Leaderboard (Play Games)

*Play Games-Dienste → Einrichtung und Verwaltung*: Projekt erstellen, Anmeldedaten für
Android hinzufügen (Paketname `io.github.bonbon08.jumperchicken`, SHA-1 des
Upload-Schlüssels **und** des App-Signaturschlüssels von Google unter *Einrichtung →
App-Integrität*), Bestenliste anlegen (Zahl, höher ist besser), Tester eintragen,
veröffentlichen. Die beiden IDs in `android/gradle.properties` eintragen (siehe README).

## 9. Testen und veröffentlichen

1. *Testen → Interner Test*: AAB hochladen, dich selbst als Tester, installieren, prüfen.
2. *Geschlossener Test*: mindestens 12 Tester, 14 Tage (siehe oben).
3. *Produktionszugriff beantragen* → *Produktion* → Release erstellen → zur Prüfung senden.
   Die erste Prüfung dauert meist ein paar Tage.
4. Nach der Veröffentlichung in AdMob die App mit dem Play-Store-Eintrag **verknüpfen**,
   sonst werden keine echten Anzeigen ausgeliefert.

## Gut zu wissen

- **Nie auf die eigene echte Werbung tippen** – AdMob sperrt dafür Konten. Zum Testen den
  Debug-Build verwenden (zeigt immer Test-Werbung) oder dein Handy in AdMob als
  Testgerät eintragen.
- **Einnahmen versteuern:** Werbeeinnahmen sind Einkommen. Regelmäßige Einnahmen sind in
  Deutschland meist ein Gewerbe (Gewerbeanmeldung, ggf. Kleinunternehmerregelung).
  Im Zweifel kurz bei einem Steuerberater oder dem Finanzamt nachfragen.
- Die Datenschutzerklärung ist eine Vorlage nach bestem Wissen, keine Rechtsberatung.
