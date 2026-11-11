#pragma once

#include <cstring>

namespace ui::aircraftnames {

struct NameEntry {
  const char* code;
  const char* name;
};

/** ICAO type code (e.g. "B738") -> readable model. Unknown codes are returned as-is. */
inline const char* modelFromType(const char* type) {
  static const NameEntry kTypes[] = {
      {"A318", "A318"},     {"A319", "A319"},
      {"A320", "A320"},     {"A321", "A321"},
      {"A19N", "A319neo"},  {"A20N", "A320neo"},
      {"A21N", "A321neo"},  {"A332", "A330"},
      {"A333", "A330"}, {"A338", "A330"},
      {"A339", "A330"}, {"A343", "A340"},
      {"A346", "A340"}, {"A359", "A350"},
      {"A35K", "A350"}, {"A388", "A380"},
      {"A400", "A400M"},    {"BCS1", "A220"},
      {"BCS3", "A220"}, {"B733", "737"},
      {"B734", "737"},  {"B736", "737"},
      {"B737", "737"},  {"B738", "737"},
      {"B739", "737"},  {"B37M", "737 MAX"},
      {"B38M", "737 MAX"}, {"B39M", "737 MAX"},
      {"B744", "747"},  {"B748", "747"},
      {"B752", "757"},  {"B753", "757"},
      {"B762", "767"},  {"B763", "767"},
      {"B764", "767"},  {"B772", "777"},
      {"B77L", "777"}, {"B773", "777"},
      {"B77W", "777"}, {"B788", "787"},
      {"B789", "787"},    {"B78X", "787"},
      {"MD11", "MD-11"}, {"E145", "E145"},
      {"E170", "E170"},     {"E75L", "E175"},
      {"E75S", "E175"},     {"E190", "E190"},
      {"E195", "E195"},     {"E290", "E190-E2"},
      {"E295", "E195-E2"}, {"CRJ2", "CRJ-200"},
      {"CRJ7", "CRJ-700"},         {"CRJ9", "CRJ-900"},
      {"CRJX", "CRJ-1000"},        {"AT72", "ATR 72"},
      {"AT76", "ATR 72-600"},      {"AT45", "ATR 42"},
      {"DH8D", "Dash 8 Q400"},     {"SF34", "Saab 340"},
      {"B190", "Beech 1900"},      {"C172", "Cessna 172"},
      {"C208", "Cessna Caravan"},  {"PC12", "Pilatus PC-12"},
      {"SR22", "Cirrus SR22"},     {"PA28", "Piper PA-28"},
      {"DA40", "Diamond DA40"},    {"GLF5", "Gulfstream V"},
      {"GLF6", "Gulfstream G650"}, {"C56X", "Citation Excel"},
      {"FA7X", "Falcon 7X"},       {"E55P", "Phenom 300"},
      {"EC35", "H135"},     {"EC45", "H145"},
      {"A139", "AW139"},           {"R44", "Robinson R44"},
      {"C17", "C-17 Globemaster"}, {"C130", "C-130 Hercules"},
      {"A124", "Antonov An-124"},
  };
  if (type == nullptr || type[0] == '\0') {
    return "";
  }
  for (const NameEntry& e : kTypes) {
    if (strcmp(e.code, type) == 0) {
      return e.name;
    }
  }
  return type;
}

/** 3-letter ICAO airline prefix from a callsign like "BAW123"; "" if unknown. */
inline const char* airlineFromCallsign(const char* callsign) {
  static const NameEntry kAirlines[] = {
      {"BAW", "British Airways"}, {"EZY", "easyJet"},
      {"RYR", "Ryanair"},         {"VIR", "Virgin Atlantic"},
      {"EIN", "Aer Lingus"},      {"EXS", "Jet2"},
      {"TOM", "TUI Airways"},     {"LOG", "Loganair"},
      {"CFE", "BA CityFlyer"},    {"SHT", "BA Shuttle"},
      {"WZZ", "Wizz Air"},        {"DLH", "Lufthansa"},
      {"AFR", "Air France"},      {"KLM", "KLM"},
      {"IBE", "Iberia"},          {"VLG", "Vueling"},
      {"SAS", "SAS"},             {"NAX", "Norwegian"},
      {"NOZ", "Norwegian"},       {"FIN", "Finnair"},
      {"SWR", "Swiss"},           {"AUA", "Austrian"},
      {"BEL", "Brussels Airlines"}, {"TAP", "TAP Air Portugal"},
      {"LOT", "LOT Polish"},      {"THY", "Turkish Airlines"},
      {"TRA", "Transavia"},       {"TVF", "Transavia France"},
      {"ICE", "Icelandair"},      {"LGL", "Luxair"},
      {"UAE", "Emirates"},        {"QTR", "Qatar Airways"},
      {"ETD", "Etihad"},          {"SVA", "Saudia"},
      {"ELY", "El Al"},           {"MSR", "EgyptAir"},
      {"ETH", "Ethiopian"},       {"SAA", "South African"},
      {"AIC", "Air India"},       {"IGO", "IndiGo"},
      {"SIA", "Singapore Airlines"}, {"CPA", "Cathay Pacific"},
      {"ANA", "ANA"},             {"JAL", "Japan Airlines"},
      {"KAL", "Korean Air"},      {"AAR", "Asiana"},
      {"CCA", "Air China"},       {"CES", "China Eastern"},
      {"CSN", "China Southern"},  {"THA", "Thai Airways"},
      {"MAS", "Malaysia Airlines"}, {"QFA", "Qantas"},
      {"ANZ", "Air New Zealand"}, {"AAL", "American Airlines"},
      {"DAL", "Delta"},           {"UAL", "United"},
      {"SWA", "Southwest"},       {"JBU", "JetBlue"},
      {"ASA", "Alaska Airlines"}, {"ACA", "Air Canada"},
      {"WJA", "WestJet"},         {"AMX", "Aeromexico"},
      {"AVA", "Avianca"},         {"CMP", "Copa Airlines"},
      {"GLO", "Gol"},             {"AZU", "Azul"},
      {"FDX", "FedEx"},           {"UPS", "UPS"},
      {"DHK", "DHL"},             {"BCS", "DHL"},
      {"RRR", "Royal Air Force"},
  };
  if (callsign == nullptr || strlen(callsign) < 4) {
    return "";
  }
  for (int i = 0; i < 3; ++i) {
    if (callsign[i] < 'A' || callsign[i] > 'Z') {
      return "";
    }
  }
  if (callsign[3] < '0' || callsign[3] > '9') {
    return "";
  }
  for (const NameEntry& e : kAirlines) {
    if (strncmp(e.code, callsign, 3) == 0) {
      return e.name;
    }
  }
  return "";
}

}  // namespace ui::aircraftnames