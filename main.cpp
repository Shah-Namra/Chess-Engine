// Phase 5 — basic move setup test.
// Just checks compilation and Move construction.

#include "board.h"
#include "bitboard.h"
#include "types.h"
#include "constants.h"
#include "movegen.h"

int main()
{
    Board board;
    board.print();

    init_attack_tables();

    // test Move (E2 → E4, double push).
    Move m(12, 28, FLAG_DOUBLE_PUSH);
    (void)m; // suppress unused warning

    return 0;
}