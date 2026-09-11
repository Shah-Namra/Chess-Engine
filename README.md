# Chess-Engine


A chess engine written from scratch in C++17, using 64-bit bitboard board representation, minimax search with alpha-beta pruning, and a Zobrist-hashed transposition table. Benchmarked against Stockfish on mate-in-3 tactical puzzles.

---

## Build

```bash
g++ -std=c++17 -O2 -Wall -I. -o engine main.cpp core/*.cpp bench/benchmark.cpp bench/perft.cpp
.\engine.exe
```

**Representation:** `char board[8][8]`. Uppercase = white pieces, lowercase = black, `'.'` = empty. Standard algebraic notation convention.

| Character | Piece        |
| --------- | ------------ |
| `K` / `k` | King         |
| `Q` / `q` | Queen        |
| `R` / `r` | Rook         |
| `B` / `b` | Bishop       |
| `N` / `n` | Knight       |
| `P` / `p` | Pawn         |
| `.`       | Empty square |

## File Strucuture:

```
main.cpp          — entry point, phase tests
types.h           — shared types: Color, PieceType, Move, MoveList, UndoInfo
constants.h       — named constants: peice values, move flags, search limits
core/
  board.h/cpp     — Board struct, make_move, unmake_move
  bitboard.h/cpp  — 64bit board helpres, attack tables
  movegen.h/cpp   — move generation for all piece types
  movesort.h/cpp  — move ordering (TT move + MVV-LVA)
  eval.h/cpp      — static position evaluation
  search.h/cpp    — minimax, alpha-beta, alphabeta + TT
  hash.h/cpp      — Zobirst hashing
  tt.h/cpp        — transposition table
```

**Output:**

```
8  r n b q k b n r
7  p p p p p p p p
6  . . . . . . . .
5  . . . . . . . .
4  . . . . . . . .
3  . . . . . . . .
2  P P P P P P P P
1  R N B Q K B N R

   a b c d e f g h
```

### Phase 1 — struct

- added struct with a constructor, better organized

### #4

- added `bitboard.h` and `bitboard.cpp` with lsb(), pop_lsb(), popcount(), directional shift helpers, print_bitboard(), and init_attack_tables() for knights and kings.
  Commit test: print `KNIGHT_ATTACKS[28] (E4)` and visually confirm the eight targeted squares by knight.

### #5

- Added `Move` struct and `MoveList` alias to types.h
- created constants.h, added stubbed movegen.h, updated main.cpp with a test Move construction.

### #6 & #7

- added kngiht moves, king moves, pawn moves and is_swqaure_attacked()
- bug fixed: the knight on the corner were attacking wrong squares
  - so added NOT_GH_FILE and NOT_AB_FILE while
  - also masks were wrong so updated it
  - test commented on the main.cpp
- also pawn single and double pushes, digonal captures, queen promption
  - lot of ToDos here, enpassant, under promotions and others
  - testing not done properly

### #8

- added rook, bishop and queen moves
- added ray casting
- `cast_ray()` checks empthy peice, enemy peice which can be caputred and stop, own peice and stop
- `is_square_attacked()` casts rays from the target in all possible directions, to find enemy peices

### #9

- `make_move` and `unmake_moce` on the board struct
- `updateInfo` saves the captured peice before anything is modified so `unamke` can restore it
-

### #11

- minmax search added
- `search.h` and `search.cpp` which has search() and minimax()
- white looks for highest and blacks looks for picks score
- empty move list returns +- INF to approx mate/stalemate

### #12

- Alpha beta method, added alpha beta cutoff,
- compared it with minimax
- counter, node counter
- alpha for best maximizer and beta for minimizrr

### #13

- giving each peice square and color a random 64bit key at starting `init_zobrist()`
- hash = xor of all acitive keys `compute_hash()`
- board is storing zoborist_hash field
- `make_move`: XOR out source piece, XOR out capture, XOR in destination, flip side key
- `unmake_move`: exact reverse sequence

### #14

- transposition table in `core/tt.h` and `core/tt.cpp`
- stores the hash key depth score flag and best move
- 3 types of flags: exact, or a bound(alpha beta)
- resturctered the files
- todo: add a line or two about each files

### #15

- move odering in `movesort.h/cpp`
- TT move is searched first, then it is captured by MVV-LVA then quiet moves
- added move ordering before search
- TT move searched first
- captures ordered using MVV-LVA
- applied file masks before shifting
- added extra file masks for knight moves
- `search_ordered()` added
- knight attack table bug found using perft
- `(b << 17) & NOT_H_FILE` replaced with `(b & NOT_H_FILE) << 17`
- Also added `NOT_AB_FILE` replaced with `NOT_GH_FILE` to `bitboard.h` perft(3) went from 8962 to 8902

### #16

- `benchmark.h/cpp` for benchmark harness
- added `load_fen()` to Board to parse FEN strings for puzzle setup

### #17

- `in_check()` finds the king's sqaure and calls is_square_attacked so that king is in check or not
- `geenrate_legal_moves()`: filter pesudo legal move that leaves the king in chekc
- added 0 (stalement ) in empty move list now before it was only -INF (checkmate)
-

### #18

- `quiesence()` extends search through captures at depth 0 instead od static eval
- `search_quiescence()` is main search function now
- benchmarked with real 100 lichess mate in 3 puzzles
  - 38% accuracy at depth of 5 with legal move filtering
  - results in bench/results.xlsx
- `alphabeta_q()` drops into quiesce() at depth 0

### #19

- `search_iterative()` added in search.cpp
  - iterative deepening from depth 1 up to max_depth
  - each pass fills the TT so the next depth orders moves better
  - benchmark now uses iterative deepening instead of search_ordered() directly
- bumped puzzle benchmark to depth 6 (was 5)
- accuracy improved from 38% (depth 5) to 42% (depth 6) on 100 lichess puzzles
- still bottlenecked by:
  - quiescence only searching captures (misses checks/threats)
  - no king safety / pawn structure in eval
  - TT cleared between ID iterations (need to fix — defeats the point)

### #20

- Removed the TT between iterations in `search_iterative()`, only called at once at the root
- killer moves in movesort.h/cpp,
  - 2killer/ply, Max Ply = 64
- history heurisitic added
  - history[color][from][to] incremented by dept\*depth on quiet cutoffs
- `order_moves()` now takes a ply parameter
- `alphabeta_q()` takes ply and updates killers/history on cutoffs
- Move struct given default constructor (needed for killer array init)

result:

- accuracy 41% (Reduce by 1)
- nodes: 17.6M (92.3M, ~80% reduction)
- time: 55s(371s, ~6.7x faster)

### #21

- perft test suite added in `bench/perft.h/cpp`
  - it counted leaf nodes in the move tree to given dept also prints per root move counts and runs ~18 caswes vs known correct known node counts
  - test positions +ref counts from chessprogramming.org/perft_results
- a bug fixed where `is_square_attacked()` was chekcing to pawn attacks
  - king was legally move onto sqaure attacked by enemy pawns
  - it affected `in_check()`
  - perft was filtering one illegal move 14/15 fixxed
- result was 7/18 perft cases pass

### #22

- en passant fully implemented
  - Board.ep_square tracks the skipped square
- movesort + qsearch treat EP as a capture
  -generate_pawn_moves emits FLAG_EN_PASSANT captures
- perft results: 7/18 to 10/18
- puzzle accuracy: 40% (unchanged, as expected en passant rare in puzzles)
- tested took long enough 
- google en passant

### #23
-
- castling implemented
- under promotion implemented
  - `add_promotions` gives Q,R,B,K for every pawn promote
  - also `make_move` and `unmake_move` handle all 4 flags
  - `generate_king_mves` checks castling only if king dont have any check, checked squares in between, sqaures are empty
  - `ZOBRIST_CASTLING[4]` keys added so TT distinguishes positions that differ only by castling rights
- perft: 10/18 -> 18/18 
  - pos 4 and 5 were underpromotion, 


## Known Bugs / TODOs

- En passant : Hopefully done
- Castling 
- Underpromotion (queen only)
- TT replacement is always-overwrite (depth-preferred is TODO)
- Pawn structure evaluation missing (doubled/isolated/passed pawns)
- puzzle acccuracy is only 41% at depth 6
- quiescene only extends captures( misses checks and threats)
