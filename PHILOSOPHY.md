# Filosofie

> **Dit systeem is nooit een vervanging van menselijke zorg — hoogstens
> een aanvulling.** Het observeert alleen *of* er beweging is, neemt geen
> beslissingen, en pretendeert geen veiligheidsgarantie te zijn.

## Het probleem

De meeste systemen voor mensen die alleen wonen — vaak ouderen — zijn
gebaseerd op twee uitersten: cameratoezicht of een persoonlijke
noodknop.

Een camera kan veel waarnemen, maar vormt een grote inbreuk op de
privacy. Een noodknop is privacyvriendelijk, maar werkt alleen wanneer
de persoon zelf in staat en bereid is om hulp in te schakelen — precies
op het moment dat dat vaak niet meer lukt.

Dit project kiest bewust voor een eenvoudige tussenoplossing.

## Het uitgangspunt

Eén PIR-bewegingssensor in de woonkamer registreert uitsluitend **of**
er beweging wordt waargenomen. Er wordt geen beeld gemaakt, geen
geluid opgenomen, en er wordt niet geprobeerd te bepalen wát iemand
aan het doen is.

De belangrijkste informatie is juist de **afwezigheid** van beweging.
Het systeem probeert daarom niet vast te stellen of iemand veilig is —
het probeert vast te stellen wanneer het uitblijven van beweging
**ongewoon wordt**, en dat aanleiding geeft om familie te waarschuwen.

De basisregel is simpel: zodra er beweging wordt waargenomen, begint
de beoordeling opnieuw. Het aantal bewegingen is op zichzelf niet
belangrijk — één beweging en duizend bewegingen zijn voor het actuele
alarmsysteem allebei voldoende om vast te stellen dat er beweging is
geweest. De aantallen worden alleen op de achtergrond gebruikt om te
bepalen wat op een bepaald moment normaal is.

## Zelflerend, maar eenvoudig

Het systeem houdt per weekdag en tijdsblok (6 blokken van 4 uur) een
voortschrijdend gemiddelde bij over de laatste 6 weken — een beperkte,
lokale dataset die op de ESP zelf past, zonder externe server of
cloud. De getallen hebben geen absolute betekenis: het maakt niet uit
of een tijdsblok normaal 30 of 30.000 bewegingen oplevert. Het
relevante gegeven is de verhouding tot het normale patroon van dát
specifieke tijdsblok.

De intelligentie zit niet in het herkennen van activiteiten, maar in
het **wegen van geen beweging**. Geen beweging tijdens een periode
waarin normaal nauwelijks beweging wordt geregistreerd (bv. 's nachts)
hoeft weinig te betekenen. Geen beweging tijdens een periode waarin
normaal veel beweging plaatsvindt, is veel opvallender. Daarom krijgt
het ontbreken van beweging een andere zwaarte, afhankelijk van het
tijdstip, de dag, en wat daar normaal gesproken gebeurt.

Naast dit acute signaal kijkt het systeem ook naar het patroon over
langere tijd: wordt het hele dagritme onregelmatiger dan gebruikelijk
— in beide richtingen, dus ook ongebruikelijk veel beweging in normaal
rustige uren — dan is dat een apart, minder urgent vroegsignaal
("onrust in dagpatronen"), dat wordt meegestuurd met een eventueel
acuut alarm.

Hoe dit alles precies is uitgewerkt — de rekenregels, de drempels, wat
er gebeurt als niemand reageert — staat in `ALGORITHM.md`; de
technische opzet in `README.md`. Dit document blijft bewust op het
niveau van het waarom.

## Privacy by design

Privacy is geen extra functie, maar een uitgangspunt. De sensor
registreert geen beeld, geen geluid, geen identiteit, geen locatie
binnen de woning, geen activiteitstype en geen persoonlijke inhoud —
alleen: er is een bewegingsgebeurtenis geregistreerd. Alle verwerking
gebeurt lokaal op de ESP. Nergens in het systeem — niet in de
webinterface — wordt een dag-voor-dag-logboek van
aanwezigheid getoond of verstuurd; alleen het geaggregeerde, geleerde
patroon is zichtbaar.

## Geen perfectie als doel

Het systeem kan niet vaststellen wat er daadwerkelijk aan de hand is.
Iemand kan bijvoorbeeld langere tijd stilzitten of slapen, en een
sensor kan een beweging missen. Het systeem pretendeert dan ook geen
feilloze veiligheidsbewaking te zijn. Het doel is eenvoudiger: een
systeem maken dat aanzienlijk beter is dan niets, zonder daarvoor
voortdurend toezicht op de persoon te introduceren. Het neemt niet de
plaats in van familie of professionele hulp — het zorgt er alleen voor
dat een ongebruikelijke situatie eerder onder de aandacht kan komen.

## KIS

Het project volgt bewust het principe **Keep It Simple**. De hardware
bestaat in essentie uit een ESP en één bewegingssensor. De software
bestaat uit: beweging registreren → gegevens per tijdsblok bijhouden →
normaal patroon bepalen → afwezigheid van beweging wegen → eventueel
familie waarschuwen. Geen camera, geen complexe sensornetwerken, geen
zware AI en geen grote database.

De kracht van het systeem zit in de eenvoudige vraag die het probeert
te beantwoorden:

> **Wanneer is "geen beweging" ongewoon genoeg om iemand te laten
> controleren of alles goed gaat? (familie of professionele hulp)**

## Vergelijkbare projecten

- **Automatic Alarm System for the Elderly Living Alone (Japan)** —
  [jstage.jst.go.jp](https://www.jstage.jst.go.jp/article/jami/26/1/26_1/_article/-char/en) —
  PIR-sensoren bij alleenwonende ouderen, gebaseerd op het uitblijven
  van beweging.
- **Seoul National University — Nonresponse Interval** —
  [snu.elsevierpure.com](https://snu.elsevierpure.com/en/publications/detection-of-abnormal-living-patterns-for-elderly-living-alone-us) —
  introduceert het *Nonresponse Interval*, waar dit project in feite een
  sterk vereenvoudigde variant van is.
- **Low-cost Binary Sensors (2019)** —
  [mdpi.com](https://www.mdpi.com/1424-8220/19/10/2264) /
  [PubMed](https://pubmed.ncbi.nlm.nih.gov/31100824/) —
  laat zien dat goedkope binaire sensoren, waaronder PIR, zowel acute
  afwijkingen als langzaam veranderende patronen kunnen signaleren.
- **IBED — Inactivity-Based Emergency Detection** —
  [github.com/WilhelmSebastian/IBED](https://github.com/WilhelmSebastian/IBED) —
  open-source project met een vergelijkbaar doel, eveneens op PIR-basis.

Wat dit project onderscheidt is vooral de nadruk op **volledig lokale
verwerking op goedkope consumentenhardware** (één ESP8266 + één
PIR-sensor, geen server, geen cloud) en de expliciete, harde
privacygrens: nergens is een herleidbaar dag-voor-dag-logboek zichtbaar
— zelfs niet voor de gebruiker zelf.
