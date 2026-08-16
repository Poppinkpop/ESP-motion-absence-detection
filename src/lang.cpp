#include "lang.h"
#include "lang_config.h"

#if !defined(LANG_NL) && !defined(LANG_EN) && !defined(LANG_DE) && !defined(LANG_FR) && !defined(LANG_ES)
#error "lang_config.h: define exactly one LANG_xx macro"
#endif

namespace Lang {

const char* htmlLangCode() {
#if defined(LANG_NL)
    return "nl";
#elif defined(LANG_EN)
    return "en";
#elif defined(LANG_DE)
    return "de";
#elif defined(LANG_FR)
    return "fr";
#elif defined(LANG_ES)
    return "es";
#endif
}

const char* tabStatus() {
#if defined(LANG_NL)
    return "Status";
#elif defined(LANG_EN)
    return "Status";
#elif defined(LANG_DE)
    return "Status";
#elif defined(LANG_FR)
    return "État";
#elif defined(LANG_ES)
    return "Estado";
#endif
}

const char* tabSettings() {
#if defined(LANG_NL)
    return "Instellingen";
#elif defined(LANG_EN)
    return "Settings";
#elif defined(LANG_DE)
    return "Einstellungen";
#elif defined(LANG_FR)
    return "Paramètres";
#elif defined(LANG_ES)
    return "Configuración";
#endif
}

const char* tabLog() {
#if defined(LANG_NL)
    return "Log";
#elif defined(LANG_EN)
    return "Log";
#elif defined(LANG_DE)
    return "Protokoll";
#elif defined(LANG_FR)
    return "Journal";
#elif defined(LANG_ES)
    return "Registro";
#endif
}

const char* dayName(int day) {
#if defined(LANG_NL)
    static const char* names[7] = {"maandag", "dinsdag", "woensdag", "donderdag", "vrijdag", "zaterdag", "zondag"};
#elif defined(LANG_EN)
    static const char* names[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
#elif defined(LANG_DE)
    static const char* names[7] = {"Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag", "Sonntag"};
#elif defined(LANG_FR)
    static const char* names[7] = {"lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi", "dimanche"};
#elif defined(LANG_ES)
    static const char* names[7] = {"lunes", "martes", "miércoles", "jueves", "viernes", "sábado", "domingo"};
#endif
    if (day < 0 || day > 6) return "?";
    return names[day];
}

const char* blockHourSuffix() {
#if defined(LANG_NL)
    return "u";
#else
    return "h";
#endif
}

const char* sensitivityMoreWord() {
#if defined(LANG_NL)
    return "Gevoeliger";
#elif defined(LANG_EN)
    return "More sensitive";
#elif defined(LANG_DE)
    return "Empfindlicher";
#elif defined(LANG_FR)
    return "Plus sensible";
#elif defined(LANG_ES)
    return "Más sensible";
#endif
}

const char* sensitivityNormalWord() {
#if defined(LANG_NL)
    return "Normaal";
#elif defined(LANG_EN)
    return "Normal";
#elif defined(LANG_DE)
    return "Normal";
#elif defined(LANG_FR)
    return "Normal";
#elif defined(LANG_ES)
    return "Normal";
#endif
}

const char* sensitivityLessWord() {
#if defined(LANG_NL)
    return "Minder gevoelig";
#elif defined(LANG_EN)
    return "Less sensitive";
#elif defined(LANG_DE)
    return "Weniger empfindlich";
#elif defined(LANG_FR)
    return "Moins sensible";
#elif defined(LANG_ES)
    return "Menos sensible";
#endif
}

String sensitivityCustomLabel(uint16_t threshold) {
#if defined(LANG_NL)
    return "Aangepast (" + String(threshold) + ")";
#elif defined(LANG_EN)
    return "Custom (" + String(threshold) + ")";
#elif defined(LANG_DE)
    return "Benutzerdefiniert (" + String(threshold) + ")";
#elif defined(LANG_FR)
    return "Personnalisé (" + String(threshold) + ")";
#elif defined(LANG_ES)
    return "Personalizado (" + String(threshold) + ")";
#endif
}

const char* active() {
#if defined(LANG_NL)
    return "ACTIEF";
#elif defined(LANG_EN)
    return "ACTIVE";
#elif defined(LANG_DE)
    return "AKTIV";
#elif defined(LANG_FR)
    return "ACTIF";
#elif defined(LANG_ES)
    return "ACTIVO";
#endif
}

const char* inactive() {
#if defined(LANG_NL)
    return "geen";
#elif defined(LANG_EN)
    return "none";
#elif defined(LANG_DE)
    return "keine";
#elif defined(LANG_FR)
    return "aucun";
#elif defined(LANG_ES)
    return "ninguno";
#endif
}

// --- Status-tab ---------------------------------------------------

String statusTimeLine(bool timeAvailableFlag, const String &timeStr) {
#if defined(LANG_NL)
    return "Tijd: " + (timeAvailableFlag ? timeStr : String("nog geen NTP-sync"));
#elif defined(LANG_EN)
    return "Time: " + (timeAvailableFlag ? timeStr : String("no NTP sync yet"));
#elif defined(LANG_DE)
    return "Zeit: " + (timeAvailableFlag ? timeStr : String("noch keine NTP-Synchronisierung"));
#elif defined(LANG_FR)
    return "Heure : " + (timeAvailableFlag ? timeStr : String("pas encore de synchronisation NTP"));
#elif defined(LANG_ES)
    return "Hora: " + (timeAvailableFlag ? timeStr : String("aún sin sincronización NTP"));
#endif
}

String statusWifiLine(bool connected, int rssiDbm) {
#if defined(LANG_NL)
    String s = "WiFi: " + String(connected ? "verbonden" : "NIET verbonden");
#elif defined(LANG_EN)
    String s = "WiFi: " + String(connected ? "connected" : "NOT connected");
#elif defined(LANG_DE)
    String s = "WLAN: " + String(connected ? "verbunden" : "NICHT verbunden");
#elif defined(LANG_FR)
    String s = "WiFi : " + String(connected ? "connecté" : "NON connecté");
#elif defined(LANG_ES)
    String s = "WiFi: " + String(connected ? "conectado" : "NO conectado");
#endif
    if (connected) s += " (" + String(rssiDbm) + " dBm)";
    return s;
}

String statusCurrentBlockLine(const String &dayNm, int block, int startHour, int endHour) {
    String suffix = blockHourSuffix();
#if defined(LANG_NL)
    return "Huidige weekdag/blok: " + dayNm + ", blok " + String(block) + " (" +
           String(startHour) + "-" + String(endHour) + suffix + ")";
#elif defined(LANG_EN)
    return "Current weekday/block: " + dayNm + ", block " + String(block) + " (" +
           String(startHour) + "-" + String(endHour) + suffix + ")";
#elif defined(LANG_DE)
    return "Aktueller Wochentag/Block: " + dayNm + ", Block " + String(block) + " (" +
           String(startHour) + "-" + String(endHour) + " Uhr)";
#elif defined(LANG_FR)
    return "Jour/bloc actuel : " + dayNm + ", bloc " + String(block) + " (" +
           String(startHour) + "-" + String(endHour) + suffix + ")";
#elif defined(LANG_ES)
    return "Día/bloque actual: " + dayNm + ", bloque " + String(block) + " (" +
           String(startHour) + "-" + String(endHour) + suffix + ")";
#endif
}

String statusLiveCountLine(uint16_t count) {
#if defined(LANG_NL)
    return "Live telling dit blok: <b>" + String(count) + "</b>";
#elif defined(LANG_EN)
    return "Live count this block: <b>" + String(count) + "</b>";
#elif defined(LANG_DE)
    return "Live-Zählung dieses Blocks: <b>" + String(count) + "</b>";
#elif defined(LANG_FR)
    return "Comptage en direct de ce bloc : <b>" + String(count) + "</b>";
#elif defined(LANG_ES)
    return "Recuento en vivo de este bloque: <b>" + String(count) + "</b>";
#endif
}

String statusLastMovementLine(unsigned long minutesAgo) {
#if defined(LANG_NL)
    return "Laatste beweging: " + String(minutesAgo) + " minuten geleden";
#elif defined(LANG_EN)
    return "Last movement: " + String(minutesAgo) + " minutes ago";
#elif defined(LANG_DE)
    return "Letzte Bewegung: vor " + String(minutesAgo) + " Minuten";
#elif defined(LANG_FR)
    return "Dernier mouvement : il y a " + String(minutesAgo) + " minutes";
#elif defined(LANG_ES)
    return "Último movimiento: hace " + String(minutesAgo) + " minutos";
#endif
}

String statusRestModeBanner() {
#if defined(LANG_NL)
    return "RUST MODUS actief — 3 meldingen verstuurd zonder reactie op beweging. "
           "Live telling gaat door, maar wordt niet in het geleerde patroon opgeslagen. "
           "Systeem hervat automatisch zodra er weer beweging is.";
#elif defined(LANG_EN)
    return "REST MODE active — 3 notifications sent without any movement in response. "
           "Live counting continues, but is not saved into the learned pattern. "
           "The system resumes automatically as soon as movement is detected again.";
#elif defined(LANG_DE)
    return "RUHEMODUS aktiv — 3 Benachrichtigungen ohne Bewegung als Reaktion gesendet. "
           "Die Live-Zählung läuft weiter, wird aber nicht in das gelernte Muster übernommen. "
           "Das System setzt sich automatisch fort, sobald wieder Bewegung erkannt wird.";
#elif defined(LANG_FR)
    return "MODE REPOS actif — 3 notifications envoyées sans aucun mouvement en réponse. "
           "Le comptage en direct continue, mais n'est pas enregistré dans le modèle appris. "
           "Le système reprend automatiquement dès qu'un mouvement est à nouveau détecté.";
#elif defined(LANG_ES)
    return "MODO DE REPOSO activo — se enviaron 3 notificaciones sin ningún movimiento en respuesta. "
           "El recuento en vivo continúa, pero no se guarda en el patrón aprendido. "
           "El sistema se reanuda automáticamente en cuanto se detecta movimiento de nuevo.";
#endif
}

String statusNotificationLine(uint8_t count, uint16_t cooldownBlocksRemaining, bool showCooldown) {
#if defined(LANG_NL)
    String s = "Meldingen in huidige episode: " + String(count) + "/3";
    if (showCooldown) s += " (volgende over " + String(cooldownBlocksRemaining) + " blok(ken))";
#elif defined(LANG_EN)
    String s = "Notifications in current episode: " + String(count) + "/3";
    if (showCooldown) s += " (next in " + String(cooldownBlocksRemaining) + " block(s))";
#elif defined(LANG_DE)
    String s = "Benachrichtigungen in dieser Episode: " + String(count) + "/3";
    if (showCooldown) s += " (nächste in " + String(cooldownBlocksRemaining) + " Block/Blöcken)";
#elif defined(LANG_FR)
    String s = "Notifications dans cet épisode : " + String(count) + "/3";
    if (showCooldown) s += " (prochaine dans " + String(cooldownBlocksRemaining) + " bloc(s))";
#elif defined(LANG_ES)
    String s = "Notificaciones en este episodio: " + String(count) + "/3";
    if (showCooldown) s += " (siguiente en " + String(cooldownBlocksRemaining) + " bloque(s))";
#endif
    return s;
}

String statusAlarmBuildupLine(uint16_t sum, uint8_t streakLen, uint16_t threshold, const String &sensitivityLabel) {
#if defined(LANG_NL)
    return "Alarmopbouw: som=" + String(sum) + " (" + String(streakLen) +
           " afwijkende blokken op rij), drempel=" + String(threshold) + " (" + sensitivityLabel + ")";
#elif defined(LANG_EN)
    return "Alarm build-up: sum=" + String(sum) + " (" + String(streakLen) +
           " consecutive deviating blocks), threshold=" + String(threshold) + " (" + sensitivityLabel + ")";
#elif defined(LANG_DE)
    return "Alarmaufbau: Summe=" + String(sum) + " (" + String(streakLen) +
           " aufeinanderfolgende abweichende Blöcke), Schwelle=" + String(threshold) + " (" + sensitivityLabel + ")";
#elif defined(LANG_FR)
    return "Accumulation d'alarme : somme=" + String(sum) + " (" + String(streakLen) +
           " blocs déviants consécutifs), seuil=" + String(threshold) + " (" + sensitivityLabel + ")";
#elif defined(LANG_ES)
    return "Acumulación de alarma: suma=" + String(sum) + " (" + String(streakLen) +
           " bloques desviados consecutivos), umbral=" + String(threshold) + " (" + sensitivityLabel + ")";
#endif
}

const char* labelBlockAlarm() {
#if defined(LANG_NL)
    return "Blok-alarm: ";
#elif defined(LANG_EN)
    return "Block alarm: ";
#elif defined(LANG_DE)
    return "Block-Alarm: ";
#elif defined(LANG_FR)
    return "Alarme de bloc : ";
#elif defined(LANG_ES)
    return "Alarma de bloque: ";
#endif
}

String labelFlatSafetyNet(uint16_t hours) {
    const char* suffix = blockHourSuffix();
#if defined(LANG_NL)
    return "Vlak vangnet (" + String(hours) + suffix + "): ";
#elif defined(LANG_EN)
    return "Flat safety net (" + String(hours) + suffix + "): ";
#elif defined(LANG_DE)
    return "Pauschales Sicherheitsnetz (" + String(hours) + " Std.): ";
#elif defined(LANG_FR)
    return "Filet de sécurité fixe (" + String(hours) + suffix + ") : ";
#elif defined(LANG_ES)
    return "Red de seguridad fija (" + String(hours) + suffix + "): ";
#endif
}

String labelBootstrapFallback(uint16_t hours) {
    const char* suffix = blockHourSuffix();
#if defined(LANG_NL)
    return "Bootstrap-fallback (" + String(hours) + suffix + " in blokken, tijdelijk per weekdag): ";
#elif defined(LANG_EN)
    return "Bootstrap fallback (" + String(hours) + suffix + " in blocks, temporary per weekday): ";
#elif defined(LANG_DE)
    return "Bootstrap-Rückfall (" + String(hours) + " Std. in Blöcken, vorübergehend je Wochentag): ";
#elif defined(LANG_FR)
    return "Filet de secours bootstrap (" + String(hours) + suffix + " par blocs, temporaire par jour) : ";
#elif defined(LANG_ES)
    return "Red de respaldo bootstrap (" + String(hours) + suffix + " en bloques, temporal por día): ";
#endif
}

const char* labelOnrust() {
#if defined(LANG_NL)
    return "Onrust in dagpatronen: ";
#elif defined(LANG_EN)
    return "Instability in daily patterns: ";
#elif defined(LANG_DE)
    return "Unruhe in Tagesmustern: ";
#elif defined(LANG_FR)
    return "Instabilité dans les schémas journaliers : ";
#elif defined(LANG_ES)
    return "Inestabilidad en los patrones diarios: ";
#endif
}

String statusLearnedPatternHeading(uint8_t weeks) {
#if defined(LANG_NL)
    return "Geleerd patroon (gemiddelde per blok, laatste " + String(weeks) + " weken)";
#elif defined(LANG_EN)
    return "Learned pattern (average per block, last " + String(weeks) + " weeks)";
#elif defined(LANG_DE)
    return "Gelerntes Muster (Durchschnitt pro Block, letzte " + String(weeks) + " Wochen)";
#elif defined(LANG_FR)
    return "Modèle appris (moyenne par bloc, " + String(weeks) + " dernières semaines)";
#elif defined(LANG_ES)
    return "Patrón aprendido (promedio por bloque, últimas " + String(weeks) + " semanas)";
#endif
}

String statusSeverityHint() {
#if defined(LANG_NL)
    return "Severity: eerste getal = binnen deze weekdag, tweede = over alle dagen samen (zie DETECTION_METHOD.md §4).";
#elif defined(LANG_EN)
    return "Severity: first number = within this weekday, second = across all days combined (see DETECTION_METHOD.md §4).";
#elif defined(LANG_DE)
    return "Schweregrad: erste Zahl = innerhalb dieses Wochentags, zweite = über alle Tage zusammen (siehe DETECTION_METHOD.md §4).";
#elif defined(LANG_FR)
    return "Gravité : premier chiffre = pour ce jour de la semaine, second = tous les jours confondus (voir DETECTION_METHOD.md §4).";
#elif defined(LANG_ES)
    return "Gravedad: primer número = dentro de este día de la semana, segundo = en todos los días combinados (ver DETECTION_METHOD.md §4).";
#endif
}

const char* statusTableDayHeader() {
#if defined(LANG_NL)
    return "Dag";
#elif defined(LANG_EN)
    return "Day";
#elif defined(LANG_DE)
    return "Tag";
#elif defined(LANG_FR)
    return "Jour";
#elif defined(LANG_ES)
    return "Día";
#endif
}

const char* statusSeverityAbbrev() {
#if defined(LANG_NL)
    return "sev";
#elif defined(LANG_EN)
    return "sev";
#elif defined(LANG_DE)
    return "Schw.";
#elif defined(LANG_FR)
    return "grav.";
#elif defined(LANG_ES)
    return "grav.";
#endif
}

String statusAveragesHint(uint8_t weeks) {
#if defined(LANG_NL)
    return "Gemiddelden zijn geaggregeerd over de laatste " + String(weeks) +
           " weken — er wordt bewust geen dag-voor-dag geschiedenis getoond (privacy).";
#elif defined(LANG_EN)
    return "Averages are aggregated over the last " + String(weeks) +
           " weeks — day-by-day history is deliberately not shown (privacy).";
#elif defined(LANG_DE)
    return "Durchschnittswerte sind über die letzten " + String(weeks) +
           " Wochen aggregiert — aus Datenschutzgründen wird bewusst kein tagesgenauer Verlauf angezeigt.";
#elif defined(LANG_FR)
    return "Les moyennes sont agrégées sur les " + String(weeks) +
           " dernières semaines — l'historique jour par jour n'est délibérément pas affiché (confidentialité).";
#elif defined(LANG_ES)
    return "Los promedios se agregan durante las últimas " + String(weeks) +
           " semanas — deliberadamente no se muestra el historial día a día (privacidad).";
#endif
}

// --- Settings-tab ---------------------------------------------------

const char* settingsSaved() {
#if defined(LANG_NL)
    return "Instellingen opgeslagen.";
#elif defined(LANG_EN)
    return "Settings saved.";
#elif defined(LANG_DE)
    return "Einstellungen gespeichert.";
#elif defined(LANG_FR)
    return "Paramètres enregistrés.";
#elif defined(LANG_ES)
    return "Configuración guardada.";
#endif
}

const char* settingsSensitivityLegend() {
#if defined(LANG_NL)
    return "Gevoeligheid";
#elif defined(LANG_EN)
    return "Sensitivity";
#elif defined(LANG_DE)
    return "Empfindlichkeit";
#elif defined(LANG_FR)
    return "Sensibilité";
#elif defined(LANG_ES)
    return "Sensibilidad";
#endif
}

String settingsSensitivityMore(uint16_t threshold) {
#if defined(LANG_NL)
    return "Gevoeliger (drempel " + String(threshold) + ")";
#elif defined(LANG_EN)
    return "More sensitive (threshold " + String(threshold) + ")";
#elif defined(LANG_DE)
    return "Empfindlicher (Schwelle " + String(threshold) + ")";
#elif defined(LANG_FR)
    return "Plus sensible (seuil " + String(threshold) + ")";
#elif defined(LANG_ES)
    return "Más sensible (umbral " + String(threshold) + ")";
#endif
}

String settingsSensitivityNormal(uint16_t threshold) {
#if defined(LANG_NL)
    return "Normaal (drempel " + String(threshold) + ", standaard)";
#elif defined(LANG_EN)
    return "Normal (threshold " + String(threshold) + ", default)";
#elif defined(LANG_DE)
    return "Normal (Schwelle " + String(threshold) + ", Standard)";
#elif defined(LANG_FR)
    return "Normal (seuil " + String(threshold) + ", par défaut)";
#elif defined(LANG_ES)
    return "Normal (umbral " + String(threshold) + ", predeterminado)";
#endif
}

String settingsSensitivityLess(uint16_t threshold) {
#if defined(LANG_NL)
    return "Minder gevoelig (drempel " + String(threshold) + ")";
#elif defined(LANG_EN)
    return "Less sensitive (threshold " + String(threshold) + ")";
#elif defined(LANG_DE)
    return "Weniger empfindlich (Schwelle " + String(threshold) + ")";
#elif defined(LANG_FR)
    return "Moins sensible (seuil " + String(threshold) + ")";
#elif defined(LANG_ES)
    return "Menos sensible (umbral " + String(threshold) + ")";
#endif
}

String settingsNonStandardHint(uint16_t threshold) {
#if defined(LANG_NL)
    return "Huidige drempel is een niet-standaard waarde (" + String(threshold) +
           "); kies een van bovenstaande opties om terug te gaan naar een preset.";
#elif defined(LANG_EN)
    return "The current threshold is a non-standard value (" + String(threshold) +
           "); choose one of the options above to return to a preset.";
#elif defined(LANG_DE)
    return "Der aktuelle Schwellenwert ist ein nicht standardmäßiger Wert (" + String(threshold) +
           "); wählen Sie eine der obigen Optionen, um zu einer Voreinstellung zurückzukehren.";
#elif defined(LANG_FR)
    return "Le seuil actuel est une valeur non standard (" + String(threshold) +
           ") ; choisissez l'une des options ci-dessus pour revenir à un préréglage.";
#elif defined(LANG_ES)
    return "El umbral actual es un valor no estándar (" + String(threshold) +
           "); elige una de las opciones anteriores para volver a un valor predefinido.";
#endif
}

const char* settingsFlatSafetyNetLegend() {
#if defined(LANG_NL)
    return "Vlak vangnet (permanente achtervang)";
#elif defined(LANG_EN)
    return "Flat safety net (permanent fallback)";
#elif defined(LANG_DE)
    return "Pauschales Sicherheitsnetz (dauerhafte Rückfalloption)";
#elif defined(LANG_FR)
    return "Filet de sécurité fixe (secours permanent)";
#elif defined(LANG_ES)
    return "Red de seguridad fija (respaldo permanente)";
#endif
}

const char* settingsFlatSafetyNetLabel() {
#if defined(LANG_NL)
    return "Uren zonder beweging voordat er sowieso gewaarschuwd wordt:";
#elif defined(LANG_EN)
    return "Hours without movement before a warning is sent regardless:";
#elif defined(LANG_DE)
    return "Stunden ohne Bewegung, bevor in jedem Fall gewarnt wird:";
#elif defined(LANG_FR)
    return "Heures sans mouvement avant qu'une alerte soit envoyée dans tous les cas :";
#elif defined(LANG_ES)
    return "Horas sin movimiento antes de que se envíe una advertencia de todos modos:";
#endif
}

const char* settingsFlatSafetyNetHint() {
#if defined(LANG_NL)
    return "Standaard 12. Geldt altijd, ook na de leerperiode.";
#elif defined(LANG_EN)
    return "Default 12. Always applies, even after the learning period.";
#elif defined(LANG_DE)
    return "Standard 12. Gilt immer, auch nach der Lernphase.";
#elif defined(LANG_FR)
    return "Par défaut 12. S'applique toujours, même après la période d'apprentissage.";
#elif defined(LANG_ES)
    return "Predeterminado 12. Siempre se aplica, incluso después del periodo de aprendizaje.";
#endif
}

const char* settingsBootstrapLegend() {
#if defined(LANG_NL)
    return "Bootstrap-fallback (tijdelijk, eerste weken per weekdag)";
#elif defined(LANG_EN)
    return "Bootstrap fallback (temporary, first weeks per weekday)";
#elif defined(LANG_DE)
    return "Bootstrap-Rückfall (vorübergehend, erste Wochen je Wochentag)";
#elif defined(LANG_FR)
    return "Filet de secours bootstrap (temporaire, premières semaines)";
#elif defined(LANG_ES)
    return "Red de respaldo bootstrap (temporal, primeras semanas)";
#endif
}

String settingsBootstrapLabel(uint8_t blockDurationHours) {
    const char* suffix = blockHourSuffix();
#if defined(LANG_NL)
    return "Uren zonder beweging, in blokken van " + String(blockDurationHours) + suffix +
           ", zolang een weekdag nog geen 3 gevulde weken heeft:";
#elif defined(LANG_EN)
    return "Hours without movement, in blocks of " + String(blockDurationHours) + suffix +
           ", as long as a weekday has fewer than 3 filled weeks:";
#elif defined(LANG_DE)
    return "Stunden ohne Bewegung, in Blöcken von " + String(blockDurationHours) +
           " Std., solange ein Wochentag noch keine 3 vollständigen Wochen hat:";
#elif defined(LANG_FR)
    return "Heures sans mouvement, par blocs de " + String(blockDurationHours) + suffix +
           ", tant qu'un jour de la semaine n'a pas encore 3 semaines complètes :";
#elif defined(LANG_ES)
    return "Horas sin movimiento, en bloques de " + String(blockDurationHours) + suffix +
           ", mientras un día de la semana tenga menos de 3 semanas completas:";
#endif
}

const char* settingsBootstrapHint() {
#if defined(LANG_NL)
    return "Standaard 16 (= 4 blokken). Stopt vanzelf zodra een weekdag een geleerd patroon heeft.";
#elif defined(LANG_EN)
    return "Default 16 (= 4 blocks). Stops automatically once a weekday has a learned pattern.";
#elif defined(LANG_DE)
    return "Standard 16 (= 4 Blöcke). Endet automatisch, sobald ein Wochentag ein gelerntes Muster hat.";
#elif defined(LANG_FR)
    return "Par défaut 16 (= 4 blocs). S'arrête automatiquement dès qu'un jour a un modèle appris.";
#elif defined(LANG_ES)
    return "Predeterminado 16 (= 4 bloques). Se detiene en cuanto un día tiene un patrón aprendido.";
#endif
}

const char* settingsSaveButton() {
#if defined(LANG_NL)
    return "Opslaan";
#elif defined(LANG_EN)
    return "Save";
#elif defined(LANG_DE)
    return "Speichern";
#elif defined(LANG_FR)
    return "Enregistrer";
#elif defined(LANG_ES)
    return "Guardar";
#endif
}

const char* settingsTelegramLegend() {
    return "Telegram";
}

const char* settingsTelegramTestButton() {
#if defined(LANG_NL)
    return "Stuur testbericht naar Telegram";
#elif defined(LANG_EN)
    return "Send test message to Telegram";
#elif defined(LANG_DE)
    return "Testnachricht an Telegram senden";
#elif defined(LANG_FR)
    return "Envoyer un message de test à Telegram";
#elif defined(LANG_ES)
    return "Enviar mensaje de prueba a Telegram";
#endif
}

const char* settingsTelegramTestOk() {
#if defined(LANG_NL)
    return "Testbericht verstuurd — check Telegram.";
#elif defined(LANG_EN)
    return "Test message sent — check Telegram.";
#elif defined(LANG_DE)
    return "Testnachricht gesendet — bitte Telegram prüfen.";
#elif defined(LANG_FR)
    return "Message de test envoyé — vérifiez Telegram.";
#elif defined(LANG_ES)
    return "Mensaje de prueba enviado — comprueba Telegram.";
#endif
}

const char* settingsTelegramTestFail() {
#if defined(LANG_NL)
    return "Versturen mislukt — check de Log-tab voor de foutmelding.";
#elif defined(LANG_EN)
    return "Sending failed — check the Log tab for the error.";
#elif defined(LANG_DE)
    return "Senden fehlgeschlagen — Fehlermeldung im Log-Tab prüfen.";
#elif defined(LANG_FR)
    return "Échec de l'envoi — consultez l'onglet Journal pour l'erreur.";
#elif defined(LANG_ES)
    return "Error al enviar — consulta la pestaña Registro para ver el error.";
#endif
}

// --- Log-tab ---------------------------------------------------

String logLiveEventsHeading(uint8_t maxEvents) {
#if defined(LANG_NL)
    return "Live PIR-events (vandaag, laatste " + String(maxEvents) + ")";
#elif defined(LANG_EN)
    return "Live PIR events (today, last " + String(maxEvents) + ")";
#elif defined(LANG_DE)
    return "Live-PIR-Ereignisse (heute, letzte " + String(maxEvents) + ")";
#elif defined(LANG_FR)
    return "Événements PIR en direct (aujourd'hui, " + String(maxEvents) + " derniers)";
#elif defined(LANG_ES)
    return "Eventos PIR en vivo (hoy, últimos " + String(maxEvents) + ")";
#endif
}

const char* logSystemLogHeading() {
#if defined(LANG_NL)
    return "Systeemlog";
#elif defined(LANG_EN)
    return "System log";
#elif defined(LANG_DE)
    return "Systemprotokoll";
#elif defined(LANG_FR)
    return "Journal système";
#elif defined(LANG_ES)
    return "Registro del sistema";
#endif
}

String logVerboseStatus(bool on) {
#if defined(LANG_NL)
    return "Verbose logging: " + String(on ? "AAN" : "UIT");
#elif defined(LANG_EN)
    return "Verbose logging: " + String(on ? "ON" : "OFF");
#elif defined(LANG_DE)
    return "Ausführliche Protokollierung: " + String(on ? "AN" : "AUS");
#elif defined(LANG_FR)
    return "Journalisation détaillée : " + String(on ? "ACTIVÉE" : "DÉSACTIVÉE");
#elif defined(LANG_ES)
    return "Registro detallado: " + String(on ? "ACTIVADO" : "DESACTIVADO");
#endif
}

const char* logVerboseToggleLink() {
#if defined(LANG_NL)
    return "omschakelen";
#elif defined(LANG_EN)
    return "toggle";
#elif defined(LANG_DE)
    return "umschalten";
#elif defined(LANG_FR)
    return "basculer";
#elif defined(LANG_ES)
    return "cambiar";
#endif
}

// --- Telegram ---------------------------------------------------

String telegramTestMessage() {
#if defined(LANG_NL)
    return "Testbericht vanaf esp-motion-absence-detection — als je dit ontvangt, werkt de Telegram-koppeling correct.";
#elif defined(LANG_EN)
    return "Test message from esp-motion-absence-detection — if you receive this, the Telegram connection is working correctly.";
#elif defined(LANG_DE)
    return "Testnachricht von esp-motion-absence-detection — wenn Sie dies erhalten, funktioniert die Telegram-Verbindung korrekt.";
#elif defined(LANG_FR)
    return "Message de test depuis esp-motion-absence-detection — si vous recevez ceci, la connexion Telegram fonctionne correctement.";
#elif defined(LANG_ES)
    return "Mensaje de prueba desde esp-motion-absence-detection — si recibes esto, la conexión con Telegram funciona correctamente.";
#endif
}

String telegramStartupMessage(const String &ip) {
#if defined(LANG_NL)
    return "De sensor is aangesloten en actief. U kunt de instellingen wijzigen als u in de ruimte "
           "bent waar de sensor is geplaatst en verbonden bent met het huisnetwerk, via http://" + ip;
#elif defined(LANG_EN)
    return "The sensor is connected and active. You can change the settings while in the room where "
           "the sensor is installed and connected to the home network, via http://" + ip;
#elif defined(LANG_DE)
    return "Der Sensor ist verbunden und aktiv. Sie können die Einstellungen ändern, wenn Sie sich im "
           "Raum befinden, in dem der Sensor installiert ist, und mit dem Heimnetzwerk verbunden sind, "
           "über http://" + ip;
#elif defined(LANG_FR)
    return "Le capteur est connecté et actif. Vous pouvez modifier les paramètres en étant dans la "
           "pièce où le capteur est installé et connecté au réseau domestique, via http://" + ip;
#elif defined(LANG_ES)
    return "El sensor está conectado y activo. Puedes cambiar la configuración estando en la "
           "habitación donde está instalado el sensor y conectado a la red doméstica, a través de http://" + ip;
#endif
}

String telegramWeakSignalNote(int rssiDbm) {
#if defined(LANG_NL)
    return "Let op: het WiFi-signaal is zwak (" + String(rssiDbm) +
           " dBm) op deze locatie — dit kan het versturen van meldingen minder betrouwbaar maken.";
#elif defined(LANG_EN)
    return "Note: the WiFi signal is weak (" + String(rssiDbm) +
           " dBm) at this location — this may make sending notifications less reliable.";
#elif defined(LANG_DE)
    return "Hinweis: Das WLAN-Signal ist an diesem Standort schwach (" + String(rssiDbm) +
           " dBm) — dies kann das Senden von Benachrichtigungen weniger zuverlässig machen.";
#elif defined(LANG_FR)
    return "Remarque : le signal WiFi est faible (" + String(rssiDbm) +
           " dBm) à cet endroit — cela peut rendre l'envoi des notifications moins fiable.";
#elif defined(LANG_ES)
    return "Nota: la señal WiFi es débil (" + String(rssiDbm) +
           " dBm) en esta ubicación — esto puede hacer que el envío de notificaciones sea menos fiable.";
#endif
}

String telegramReasonBlockAlarm() {
#if defined(LANG_NL)
    return "Bewegingsalarm: het bewegingspatroon wijkt significant af van het normale patroon.";
#elif defined(LANG_EN)
    return "Movement alarm: the movement pattern deviates significantly from the normal pattern.";
#elif defined(LANG_DE)
    return "Bewegungsalarm: Das Bewegungsmuster weicht erheblich vom normalen Muster ab.";
#elif defined(LANG_FR)
    return "Alarme de mouvement : le schéma de mouvement s'écarte considérablement du schéma normal.";
#elif defined(LANG_ES)
    return "Alarma de movimiento: el patrón de movimiento se desvía significativamente del patrón normal.";
#endif
}

String telegramReasonFlatSafetyNet(uint16_t hours) {
#if defined(LANG_NL)
    return "Waarschuwing: al " + String(hours) + " uur geen beweging waargenomen.";
#elif defined(LANG_EN)
    return "Warning: no movement detected for " + String(hours) + " hours.";
#elif defined(LANG_DE)
    return "Warnung: seit " + String(hours) + " Stunden keine Bewegung erkannt.";
#elif defined(LANG_FR)
    return "Avertissement : aucun mouvement détecté depuis " + String(hours) + " heures.";
#elif defined(LANG_ES)
    return "Advertencia: no se ha detectado movimiento durante " + String(hours) + " horas.";
#endif
}

String telegramReasonBootstrapFallback(uint16_t hours) {
#if defined(LANG_NL)
    return "Waarschuwing: geen beweging waargenomen gedurende " + String(hours) +
           " uur — dit tijdsblok-gebaseerde vangnet geldt tijdelijk, zolang deze weekdag nog geen "
           "volledig geleerd patroon heeft.";
#elif defined(LANG_EN)
    return "Warning: no movement detected for " + String(hours) +
           " hours — this block-based safety net applies temporarily, as long as this weekday does "
           "not yet have a fully learned pattern.";
#elif defined(LANG_DE)
    return "Warnung: seit " + String(hours) + " Stunden keine Bewegung erkannt — dieses blockbasierte "
           "Sicherheitsnetz gilt vorübergehend, solange dieser Wochentag noch kein vollständig "
           "gelerntes Muster hat.";
#elif defined(LANG_FR)
    return "Avertissement : aucun mouvement détecté depuis " + String(hours) +
           " heures — ce filet de sécurité basé sur les blocs s'applique temporairement, tant que ce "
           "jour de la semaine n'a pas encore de modèle entièrement appris.";
#elif defined(LANG_ES)
    return "Advertencia: no se ha detectado movimiento durante " + String(hours) +
           " horas — esta red de seguridad basada en bloques se aplica temporalmente, mientras este "
           "día de la semana aún no tenga un patrón completamente aprendido.";
#endif
}

String telegramReasonDefault() {
#if defined(LANG_NL)
    return "Waarschuwing: afwijkend bewegingspatroon gedetecteerd.";
#elif defined(LANG_EN)
    return "Warning: deviating movement pattern detected.";
#elif defined(LANG_DE)
    return "Warnung: abweichendes Bewegungsmuster erkannt.";
#elif defined(LANG_FR)
    return "Avertissement : schéma de mouvement anormal détecté.";
#elif defined(LANG_ES)
    return "Advertencia: se ha detectado un patrón de movimiento anómalo.";
#endif
}

String telegramRestModeUpcomingNote() {
#if defined(LANG_NL)
    return "Wordt er na de volgende melding nog steeds geen beweging waargenomen, dan gaat het "
           "systeem daarna in rustmodus: geen meldingen meer en geen leren meer totdat er weer "
           "beweging is.";
#elif defined(LANG_EN)
    return "If no movement is detected after the next notification either, the system will then "
           "enter rest mode: no more notifications and no more learning until movement is detected "
           "again.";
#elif defined(LANG_DE)
    return "Wird auch nach der nächsten Benachrichtigung keine Bewegung erkannt, geht das System "
           "danach in den Ruhemodus: keine weiteren Benachrichtigungen und kein weiteres Lernen, bis "
           "wieder Bewegung erkannt wird.";
#elif defined(LANG_FR)
    return "Si aucun mouvement n'est détecté après la prochaine notification non plus, le système "
           "passera alors en mode repos : plus de notifications et plus d'apprentissage jusqu'à ce "
           "qu'un mouvement soit à nouveau détecté.";
#elif defined(LANG_ES)
    return "Si tampoco se detecta movimiento después de la próxima notificación, el sistema entrará "
           "en modo de reposo: no habrá más notificaciones ni aprendizaje hasta que se detecte "
           "movimiento de nuevo.";
#endif
}

String telegramRestModeEnteringNote() {
#if defined(LANG_NL)
    return "Het systeem gaat nu in rustmodus: er worden geen meldingen meer verstuurd en er wordt "
           "niet meer geleerd totdat er weer beweging is waargenomen. Eén keer per week volgt een "
           "kort bericht dat het systeem nog werkt.";
#elif defined(LANG_EN)
    return "The system is now entering rest mode: no more notifications will be sent and no more "
           "learning will happen until movement is detected again. A short weekly message will "
           "confirm the system is still working.";
#elif defined(LANG_DE)
    return "Das System geht jetzt in den Ruhemodus: Es werden keine weiteren Benachrichtigungen "
           "gesendet und es wird nicht mehr gelernt, bis wieder Bewegung erkannt wird. Einmal pro "
           "Woche folgt eine kurze Nachricht, dass das System noch funktioniert.";
#elif defined(LANG_FR)
    return "Le système passe maintenant en mode repos : plus aucune notification ne sera envoyée et "
           "plus aucun apprentissage n'aura lieu jusqu'à ce qu'un mouvement soit à nouveau détecté. "
           "Un court message hebdomadaire confirmera que le système fonctionne toujours.";
#elif defined(LANG_ES)
    return "El sistema entra ahora en modo de reposo: no se enviarán más notificaciones ni se "
           "realizará más aprendizaje hasta que se detecte movimiento de nuevo. Un breve mensaje "
           "semanal confirmará que el sistema sigue funcionando.";
#endif
}

String telegramOnrustNote() {
#if defined(LANG_NL)
    return "Daarnaast vertoont het bewegingspatroon van de afgelopen dagen meer onrust dan "
           "gebruikelijk (minder regelmatig dan voorheen).";
#elif defined(LANG_EN)
    return "In addition, the movement pattern of the past few days shows more instability than "
           "usual (less regular than before).";
#elif defined(LANG_DE)
    return "Außerdem zeigt das Bewegungsmuster der letzten Tage mehr Unruhe als üblich (weniger "
           "regelmäßig als zuvor).";
#elif defined(LANG_FR)
    return "De plus, le schéma de mouvement des derniers jours montre plus d'instabilité que "
           "d'habitude (moins régulier qu'avant).";
#elif defined(LANG_ES)
    return "Además, el patrón de movimiento de los últimos días muestra más inestabilidad de lo "
           "habitual (menos regular que antes).";
#endif
}

String telegramWeeklyReassurance(const String &lastMovementTimestamp) {
#if defined(LANG_NL)
    return "Het systeem staat in rustmodus omdat er sinds " + lastMovementTimestamp +
           " geen beweging is waargenomen. Er wordt niet meer gealarmeerd en er wordt niet meer "
           "geleerd totdat er weer beweging is — het systeem werkt verder nog gewoon.";
#elif defined(LANG_EN)
    return "The system is in rest mode because no movement has been detected since " +
           lastMovementTimestamp + ". No more alarms and no more learning until movement is "
           "detected again — the system otherwise continues to work normally.";
#elif defined(LANG_DE)
    return "Das System befindet sich im Ruhemodus, da seit " + lastMovementTimestamp +
           " keine Bewegung erkannt wurde. Es gibt keine weiteren Alarme und kein weiteres Lernen, "
           "bis wieder Bewegung erkannt wird — ansonsten funktioniert das System weiterhin normal.";
#elif defined(LANG_FR)
    return "Le système est en mode repos car aucun mouvement n'a été détecté depuis " +
           lastMovementTimestamp + ". Plus d'alarmes et plus d'apprentissage jusqu'à ce qu'un "
           "mouvement soit à nouveau détecté — le système continue sinon de fonctionner normalement.";
#elif defined(LANG_ES)
    return "El sistema está en modo de reposo porque no se ha detectado movimiento desde " +
           lastMovementTimestamp + ". No habrá más alarmas ni aprendizaje hasta que se detecte "
           "movimiento de nuevo — por lo demás, el sistema sigue funcionando con normalidad.";
#endif
}

} // namespace Lang
