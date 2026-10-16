# Steuerung – Desktop und Android

Die Steuerung ist in `docs/CONTROLS.md` dokumentiert und spiegelt sich in den Header-Dateien (`input.h`) und der Implementierung (`input.c`) wider.

## Desktop-Steuerung

| Aktion | Taste / Eingabe | Beschreibung |
|---|---|---|
| Springen | `Leertaste`, `X`, `Mausklick` (links) | Huhn springt nach oben |
| Menü: Navigieren | `Oben` / `Unten` oder Pfeiltasten | Auswahl im Hauptmenü oder im Todesbildschirm |
| Menü: Bestätigen | `X` oder `Enter` | Starten des Spiels / Kaufen / Bestätigen |
| Zurück zum Menü | `Backspace` oder `Escape` | Geht vom Spiel zum Hauptmenü zurück |
| Musik umschalten | `M` | Schaltet Hintergrundmusik ein/aus |
| Vollbildmodus | `F11` (falls unterstützt) | Wechselt zwischen Fenster und Vollbild |

### Menü-Interaktionen

- **Hauptmenü**: Start, Shop, Exit
  - Auswahl mit Pfeiltasten oder Mausklick
  - Bestätigung mit `X`
- **Shop**: 3 Kategorien (Charaktere, Hintergründe, Musik)
  - Navigation: Pfeiltasten
  - Kauf: `X` auf Karte (wenn genug Münzen vorhanden)
  - Zurück: `Backspace`
- **Todesbildschirm**: RETRY, MENU, REVIVE (nur wenn Ad verfügbar)
  - RETRY startet neu
  - MENU geht zum Hauptmenü zurück
  - REVIVE (nur Android/Debug, wenn Werbung verfügbar) setzt das Spiel fort

## Android-Steuerung

| Aktion | Eingabe | Beschreibung |
|---|---|---|
| Springen | Bildschirmberührung (Tap) | Huhn springt nach oben |
| Menü: Navigieren | Wischen / Tipp auf Karte | Selektiert Menü-Option oder Shop-Item |
| Menü: Bestätigen | Doppeltipp auf sicheres Element | Startet Spiel / Kauf / Bestätigung |
| Zurück | Android-Back-Button | Geht zum Hauptmenü zurück oder beendet App |
| Musik umschalten | Tipp auf Musik-Button oben links | Schaltet Musik ein/aus |

### Besonderheiten auf Android

- **Touch-Position**: `PointerPosition()` konvertiert die physische Touch-Position in die virtuelle Bildschirmauflösung, sodass das Spiel auf verschiedenen Displaygrößen korrekt skaliert.
- **Virtueller Bildschirm**: `InitVirtualScreen()` anpassbare Auflösung (Standard: 1080×720 für Desktop, dynamisch für Android).
- **Full-Screen**: Das Spiel startet automatisch im Vollbildmodus (`InitWindow(0, 0, ...)`) auf Android-Geräten.

## Steuerungs-Referenz im Code

Die Steuerung wird durch die Datei `src/shared/input.c` und den Header `include/shared/input.h` implementiert.

- `PointerPosition()` – Wandelt Bildschirm-Koordinaten in virtuelle Koordinaten um
- `PointerPressedIn(Rectangle area)` – Prüft, ob auf einen Bereich geklickt wurde
- `BackPressed()` – Prüft `KEY_BACK` und `KEY_BACKSPACE`
- `InitVirtualScreen()` – Setzt die virtuelle Auflösung und Skalierung
- `BeginVirtualScreen()` / `EndVirtualScreen()` – Zeichnet in einen Render-Texture und skaliert auf das echte Fenster

## Barrierefreiheit / Hinweise

- Die Tasten sind nicht konfigurierbar (fest verdrahtet im Code)
- Das Spiel unterstützt nur eine einzige Eingabequelle (Tastatur/Maus oder Touch) zur gleichen Zeit
- Keine Gamepad-Unterstützung
- Keine Spracheinstellungen im Code (nur deutsche Dokumentation)