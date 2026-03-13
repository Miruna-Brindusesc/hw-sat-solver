# Homework: SAT Solver in C

This project implements a **SAT solver** for formulas in **Conjunctive Normal Form (CNF)**. It uses the **DPLL (Davis-Putnam-Logemann-Loveland)** algorithm and a **VSIDS (Variable State Independent Decaying Sum)** heuristic to select variables efficiently.

The solver reads CNF formulas in **DIMACS format**, solves them, and outputs whether the formula is satisfiable along with a satisfying assignment if one exists.

## Features

* **DPLL algorithm**: Recursively assigns variables and backtracks on conflicts.
* **Unit propagation**: Automatically assigns variables when a clause has only one unassigned literal.
* **VSIDS heuristic**: Prioritizes variables that appear most often in unsatisfied clauses.
* Handles CNF formulas with thousands of variables and clauses.

## VSIDS Explanation

**VSIDS (Variable State Independent Decaying Sum)** prioritizes variables that frequently participate in conflicts. Each variable has a score that increases whenever it appears in an unsatisfied clause. The solver assigns the variable with the highest score first, improving efficiency in the search.

## Functions Overview

1. **`parse_dimacs(const char *file_path)`**
   Reads a CNF formula from a DIMACS file and stores it in a `CNF` structure.

   * Opens the specified file.
   * Reads the number of variables and clauses.
   * Allocates memory for clauses and parses them from the file.
   * Returns a `CNF` structure containing the formula.

2. **`free_cnf(CNF *cnf)`**
   Frees memory allocated for the CNF structure.

3. **`propagate_unit_clauses(CNF *cnf, int *assignment)`**
   Performs unit propagation:

   * Assigns literals from clauses with only one unassigned literal.
   * Detects conflicts (e.g., contradictory assignments).
   * Continues until no further assignments can be made.

4. **`select_variable(CNF *cnf, int *assignment)`**
   Selects the next variable to assign using the VSIDS heuristic.

5. **`dpll(CNF *cnf, int *assignment, int depth)`**
   Implements the DPLL algorithm:

   * Simplifies the formula using unit propagation.
   * Checks if the formula is satisfied.
   * Selects an unassigned variable and tries both TRUE and FALSE assignments recursively.
   * Performs backtracking if a conflict occurs.

6. **`solve_sat(const char *input_file, const char *output_file)`**
   Main function that:

   * Reads the CNF formula from the input file.
   * Initializes variable assignments.
   * Calls DPLL to determine satisfiability.
   * Writes the result to the output file (`SATISFIABLE` with variable assignments or `UNSATISFIABLE`).
   * Frees allocated memory.

7. **`main(int argc, char *argv[])`**
   Checks for valid input/output arguments and calls `solve_sat`.

## Build & Run

### Build

```bash
make build
```

### Run

```bash
make run INPUT=input.cnf OUTPUT=output.txt
```

### Clean

```bash
make clean
```

## Example

Given `example.cnf`:

```
p cnf 3 2
1 -3 0
2 3 -1 0
```

Run:

```bash
make run INPUT=example.cnf OUTPUT=result.txt
```

Output `result.txt`:

```
s SATISFIABLE
v 1 -2 3 0
```
## Project Structure

```
.
├── sat_solver.c       # Main SAT solver implementation
├── README.md          # Project documentation
└── Makefile           # Optional build rules
```