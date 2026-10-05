#pragma once

#include <string>

namespace th
{

class Game;

// One turn of the replay check's trace (enhanced/tools/replay): the state a
// game is in after a turn, in the format the instrumented original writes
// (enhanced/tools/replay/instrument.py), so the two compare line by line.
//
// A header line,
//   T <turn> seed=<8 hex> date=<year>/<month 0-11>/<day> balance=<n> patients=<n>
// with the patients counted over every hospital's; one line per hospital,
//   H <0-3> open=<0 or 1> rep=<n> price=<n>
// rep and price the sums of its rooms[].reputation and rooms[].price over
// illnesses 1-45, which weigh where patients go (who_owns_peep);
// then one line per used entry of the things table in index order,
//    <id> type=<n> sub=<n> state=<n> x=<4 hex> y=<4 hex>
// x and y as the game's MapPos.raw (tile in the high byte).
std::string trace_turn(const Game &game);

} // namespace th
