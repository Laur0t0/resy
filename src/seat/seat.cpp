#include "seat.h"

#include "reservation/reservation.h"
#include "test.h"

namespace {

/// Liefert die Position einer Station in einer Route.
size_t station_pos(std::vector<std::string> const& route,
                   std::string const& station) {
  for(int i = 0; i < std::size(route); i++){
    if(station == route[i]){
      return i;
      }
  }
  return route.size();
}

/// Prüft, ob eine Station in einer Route gültig ist.
bool station_valid_for_route(std::vector<std::string> const& route,
                             std::string const& station) {
  if(station_pos(route, station) != route.size()){
    return true;
    }
  return false;
}

}  // namespace

namespace resy {

bool Seat::reservations_valid_for_route(
    std::vector<std::string> const& route) const {
  for(int i = 0; i < reservations.size(); i++){
    if(!reservations[i].is_valid_for_route(route)){
      return false;
    }
  }
  return true;
}

bool Seat::reservations_overlap_for_route(
    std::vector<std::string> const& route) const {
  for(int i = 0; i < reservations.size(); i++){
    for(int n = 1; n < reservations.size(); n++){
      if(reservations_valid_for_route(route) && reservations[i].destination_pos(route) <= reservations[n].origin_pos(route)){
        return false;
      }
      if(reservations_valid_for_route(route) && reservations[n].destination_pos(route) <= reservations[i].origin_pos(route)){
        return false;
      }
    }
  }
  return true;
}

bool Seat::seat_valid(std::vector<std::string> const& route) const {
  if(reservations_valid_for_route(route) && !reservations_overlap_for_route(route)){
        return true;
      }
  return false;
}

std::vector<Reservation> Seat::reservations_sorted(
    std::vector<std::string> const& route) const {
  std::vector<Reservation> result = reservations;
  // TODO
  return result;
}

std::string Seat::display_for_station(std::vector<std::string> const& route,
                                      std::string const& station) const {
  // TODO
  return "frei";
}

TEST_CASE("reservations_valid_for_route") {
  std::vector<Reservation> reservations{
      {"Mannheim", "Darmstadt"},
      {"Darmstadt", "Frankfurt"},
  };
  Seat seat{42, reservations};

  SUBCASE("all valid") {
    CHECK(seat.reservations_valid_for_route(
        {"Mannheim", "Darmstadt", "Frankfurt"}));
  }

  SUBCASE("one invalid") {
    CHECK_FALSE(seat.reservations_valid_for_route({"Mannheim", "Frankfurt"}));
  }
}

TEST_CASE("reservations_overlap_for_route") {
  std::vector<std::string> route{"Mannheim", "Darmstadt", "Frankfurt"};

  SUBCASE("no overlap") {
    std::vector<Reservation> reservations{
        {"Mannheim", "Darmstadt"},
        {"Darmstadt", "Frankfurt"},
    };
    Seat seat{42, reservations};
    CHECK_FALSE(seat.reservations_overlap_for_route(route));
  }

  SUBCASE("overlap") {
    std::vector<Reservation> reservations{
        {"Mannheim", "Frankfurt"},
        {"Darmstadt", "Frankfurt"},
    };
    Seat seat{42, reservations};
    CHECK(seat.reservations_overlap_for_route(route));
  }
}

TEST_CASE("seat_valid") {
  std::vector<std::string> route{"Mannheim", "Darmstadt", "Frankfurt"};

  SUBCASE("valid") {
    std::vector<Reservation> reservations{
        {"Mannheim", "Darmstadt"},
        {"Darmstadt", "Frankfurt"},
    };
    Seat seat{42, reservations};
    CHECK(seat.seat_valid(route));
  }

  SUBCASE("missing station") {
    std::vector<Reservation> reservations{
        {"Mannheim", "Darmstadt"},
        {"Darmstadt", "Kassel"},
    };
    Seat seat{42, reservations};
    CHECK_FALSE(seat.seat_valid(route));
  }

  SUBCASE("overlap") {
    std::vector<Reservation> reservations{
        {"Mannheim", "Frankfurt"},
        {"Darmstadt", "Frankfurt"},
    };
    Seat seat{42, reservations};
    CHECK_FALSE(seat.seat_valid(route));
  }
}

TEST_CASE("reservations_sorted") {
  std::vector<Reservation> reservations{
      {"Hannover", "Hamburg"},
      {"Frankfurt", "Kassel"},
      {"Mannheim", "Frankfurt"},
  };
  Seat seat{42, reservations};

  SUBCASE("valid") {
    std::vector<std::string> route{"Mannheim",   "Frankfurt", "Kassel",
                                   "Goettingen", "Hannover",  "Hamburg"};
    auto sorted = seat.reservations_sorted(route);
    REQUIRE(sorted.size() == 3);
    CHECK(sorted[0].origin == "Mannheim");
    CHECK(sorted[0].destination == "Frankfurt");
    CHECK(sorted[1].origin == "Frankfurt");
    CHECK(sorted[1].destination == "Kassel");
    CHECK(sorted[2].origin == "Hannover");
    CHECK(sorted[2].destination == "Hamburg");
  }

  SUBCASE("missing station") {
    std::vector<std::string> route{"Mannheim", "Frankfurt", "Kassel"};
    auto sorted = seat.reservations_sorted(route);
    REQUIRE(sorted.size() == 0);
  }

  SUBCASE("overlap") {
    std::vector<std::string> route{"Mannheim",   "Kassel",   "Frankfurt",
                                   "Goettingen", "Hannover", "Hamburg"};
    auto sorted = seat.reservations_sorted(route);
    REQUIRE(sorted.size() == 0);
  }
}

#define CHECK_DISPLAY(seat, route, station, expected) \
  CHECK(seat.display_for_station(route, station) == expected)

TEST_CASE("display_for_station") {
  std::vector<Reservation> reservations{
      {"Mannheim", "Frankfurt"},
      {"Frankfurt", "Kassel"},
      {"Hannover", "Hamburg"},
  };
  Seat seat{42, reservations};

  SUBCASE("valid") {
    std::vector<std::string> route{"Mannheim",   "Frankfurt", "Kassel",
                                   "Goettingen", "Hannover",  "Hamburg"};
    CHECK_DISPLAY(seat, route, "Mannheim", "Mannheim -> Frankfurt");
    CHECK_DISPLAY(seat, route, "Frankfurt", "Frankfurt -> Kassel");
    CHECK_DISPLAY(seat, route, "Kassel", "Hannover -> Hamburg");
    CHECK_DISPLAY(seat, route, "Goettingen", "Hannover -> Hamburg");
    CHECK_DISPLAY(seat, route, "Hannover", "Hannover -> Hamburg");
    CHECK_DISPLAY(seat, route, "Hamburg", "frei");
  }

  SUBCASE("missing station") {
    std::vector<std::string> route{"Mannheim", "Frankfurt", "Kassel"};
    CHECK_DISPLAY(seat, route, "Mannheim", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Frankfurt", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Kassel", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Goettingen", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Hannover", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Hamburg", "ggf. freigeben");
  }

  SUBCASE("wrong order") {
    std::vector<std::string> route{"Mannheim",   "Kassel",   "Frankfurt",
                                   "Goettingen", "Hannover", "Hamburg"};
    CHECK_DISPLAY(seat, route, "Mannheim", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Frankfurt", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Kassel", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Goettingen", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Hannover", "ggf. freigeben");
    CHECK_DISPLAY(seat, route, "Hamburg", "ggf. freigeben");
  }
}

}  // namespace resy

std::ostream& operator<<(std::ostream& os, resy::Seat const& seat) {
  os << "Seat " << seat.number << ": [";
  for (auto const& r : seat.reservations) {
    os << r << " ";
  }
  os << "]";
  return os;
}
