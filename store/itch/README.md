# Jumper Chicken auf itch.io veröffentlichen

## Dateien

Fertig zum Hochladen (alles in `dist/`):

| Datei | Auf itch.io |
|---|---|
| `JumperChicken-web.zip` | Upload, Haken bei **„This file will be played in the browser“** |
| `JumperChicken-windows.zip` | Upload, Haken bei **Windows** |
| `JumperChicken-linux.zip` | Upload, Haken bei **Linux** |
| `JumperChicken-android.apk` | Upload, Haken bei **Android** (werbefrei, signiert) |
| `JumperChicken-macos.zip` | kommt aus GitHub Actions (siehe unten), Haken bei **macOS** |

Bilder und Video (in diesem Ordner):

| Datei | Auf itch.io |
|---|---|
| `itch-cover.png` (630×500) | *Cover image* |
| `trailer.mp4` (37 s, 720p, Stereo) | Auf YouTube hochladen, Link bei *Gameplay video or trailer* (itch.io bettet nur YouTube/Vimeo ein) |
| `gameplay.gif` und `../screenshots/*.png` | *Screenshots* |
| `itch-text.txt` | Kurztext und Seitentext zum Einfügen |

**Browser-Version (Embed options):** *Viewport dimensions* 1080 × 720, Haken bei
*Fullscreen button*, *Mobile friendly* (Querformat). Der Spielstand wird im Browser
gespeichert und bleibt nach dem Neuladen erhalten.

### Neu bauen

```bash
ZIG=/pfad/zu/zig tools/build-desktop.sh      # Windows + Linux
EMSDK=/pfad/zu/emsdk tools/build-web.sh      # Browser
tools/build-android-itch.sh                  # Android, werbefrei
```

Das Android-APK wird mit dem Schlüssel aus `~/.jumperchicken/` signiert. **Diesen Ordner
sichern** (z. B. auf einen USB-Stick oder in den Passwortmanager) – ohne ihn kannst du
keine Updates veröffentlichen, die sich über die alte Version installieren lassen.

### macOS

Ein Mac-Build lässt sich nur auf einem Mac erstellen. Das übernimmt GitHub Actions: nach
dem Push unter *Actions → letzter Lauf → Artifacts* `JumperChicken-macOS` herunterladen,
das enthaltene `JumperChicken-macos.zip` hochladen. Weil die App nicht bei Apple
notarisiert ist (dafür bräuchte man ein Apple-Entwicklerkonto für 99 $/Jahr), müssen
Mac-Spieler sie beim ersten Start per **Rechtsklick → Öffnen** starten. Schreib das in
die Beschreibung.

Optional mit [butler](https://itch.io/docs/butler/) hochladen (Spieler bekommen dann
automatisch Updates über die itch-App):

```bash
butler push dist/JumperChicken-web.zip DEINNAME/jumper-chicken:html
butler push dist/JumperChicken-windows.zip DEINNAME/jumper-chicken:windows
butler push dist/JumperChicken-linux.zip DEINNAME/jumper-chicken:linux
butler push dist/JumperChicken-android.apk DEINNAME/jumper-chicken:android
```

## Einstellungen auf der Projektseite

| Feld | Wert |
|---|---|
| Title | Jumper Chicken |
| Project URL | `jumper-chicken` |
| Short description / tagline | Bounce from wall to wall, dodge the spikes, collect coins! |
| Classification | Games |
| Kind of project | HTML (Browser-Version + Downloads) |
| Release status | Released |
| Pricing | siehe Preisempfehlung unten |
| Genre | Action |
| Tags | `arcade`, `pixel-art`, `casual`, `endless`, `one-button`, `chiptune`, `retro`, `2d`, `singleplayer`, `raylib` |
| App store links | Google-Play-Link, sobald die App dort live ist |
| Custom noun | – |
| Community | Comments |
| Visibility | erst *Draft*, dann *Public* |

## Beschreibung (zum Einfügen)

> **Jumper Chicken** is a fast one-button arcade game in pixel style.
>
> Tap, click or press a key and your chicken jumps – it bounces from wall to wall, and every
> wall is full of spikes. Every bounce makes it a bit faster, every level adds another spike.
> How far can you get?
>
> **Features**
> - One-button controls: easy to learn, hard to master
> - Gets faster and harder the further you get
> - Collect coins and unlock 5 chickens, 4 backgrounds and 4 music tracks
> - Day, sunset, starry night and candy worlds
> - Original chiptune soundtrack, synthesized live in the game
> - Play in the browser, or download for Windows, macOS, Linux and an ad-free Android version
>
> **Controls (PC)**
> - X / Space / mouse click: jump
> - Arrow keys + X: menus and shop
> - Backspace: back · M: music on/off · Esc: quit
>
> **Android:** tap to jump and to use the menus.
>
> Made with C and [raylib](https://www.raylib.com).

Deutsche Variante für den unteren Teil der Seite (optional):

> Spring mit deinem Huhn von Wand zu Wand und weich den Stacheln aus! Mit jeder Wand wird
> es schneller, mit jedem Level kommen mehr Stacheln dazu. Sammle Münzen und schalte neue
> Hühner, Hintergründe und Musik frei. Für Windows, Linux und Android (werbefrei).

## Preisempfehlung

**„No payments“ ist zu wenig, ein fester Preis zu viel – nimm „Pay what you want“:**

- **Minimum: 0 $** · **Vorgeschlagener Preis: 1,99 $**

Warum:

1. **Die Android-Version gibt es auf Google Play kostenlos (mit Werbung).** Ein Pflichtpreis
   auf itch.io wäre schwer zu verkaufen; mit „Pay what you want“ zahlt, wer das Spiel
   unterstützen will, und alle anderen laden es trotzdem herunter (Downloads und Bewertungen
   helfen dir auf itch.io, gefunden zu werden).
2. **Vergleichbare kleine Arcade-Spiele** auf itch.io sind meist kostenlos oder kosten 1–3 $.
   1,99 $ liegt unter der „Lohnt sich das?“-Schwelle und wirkt fair für ein kleines Spiel.
3. **Der Mehrwert für Zahlende:** werbefreie Android-Version und PC-Versionen.

Wenn du lieber einen festen Preis willst: **0,99 $** als Untergrenze, sonst kauft es kaum
jemand. itch.io behält standardmäßig 10 % (einstellbar), dazu kommen Zahlungsgebühren.
Für Auszahlungen musst du in den itch.io-Einstellungen PayPal oder Bankdaten und ein
Steuerformular hinterlegen.
