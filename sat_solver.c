#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// Maximum limits for clauses, variables and recursion depth
#define MAX_CLAUSES 10000
#define MAX_VARS 10000
#define MAX_DEPTH 10000

// Structure representing a CNF formula (Conjunctive Normal Form)
typedef struct {
    int **clauses;     // Array of clauses, each clause is an array of literals
    int num_clauses;   // Total number of clauses
    int num_vars;      // Total number of variables
} CNF;

// VSIDS heuristic score for each variable
// Tracks how often a variable appears in unsatisfied clauses
int vsids[MAX_VARS + 1] = {0};

// Function to parse a DIMACS CNF file
CNF parse_dimacs(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    CNF cnf = {0};
    cnf.clauses = NULL;

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        // Ignore comments and empty lines
        if (line[0] == 'c' || line[0] == '\n') continue;

        // Problem line: defines number of variables and clauses
        if (line[0] == 'p') {
            sscanf(line, "p cnf %d %d", &cnf.num_vars, &cnf.num_clauses);

            // Allocate memory for clauses
            cnf.clauses = malloc(cnf.num_clauses * sizeof(int *));
            if (!cnf.clauses) {
                fprintf(stderr, "Error: Memory allocation failed for clauses.\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < cnf.num_clauses; i++)
                cnf.clauses[i] = NULL;

        } else {
            // Allocate memory for a clause
            int *clause = malloc((cnf.num_vars + 1) * sizeof(int));
            if (!clause) {
                fprintf(stderr, "Error: Memory allocation failed for clause.\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }

            // Read literals from the line
            int lit, count = 0;
            char *token = strtok(line, " ");

            while (token) {
                lit = atoi(token);

                // 0 marks the end of the clause
                if (lit == 0) break;

                clause[count++] = lit;
                token = strtok(NULL, " ");
            }

            clause[count] = 0;

            // Insert clause into CNF structure
            for (int i = 0; i < cnf.num_clauses; i++) {
                if (cnf.clauses[i] == NULL) {
                    cnf.clauses[i] = clause;
                    break;
                }
            }
        }
    }

    fclose(file);
    return cnf;
}

// Free memory allocated for the CNF structure
void free_cnf(CNF *cnf) {
    if (cnf->clauses) {
        for (int i = 0; i < cnf->num_clauses; i++) {
            if (cnf->clauses[i]) {
                free(cnf->clauses[i]);
                cnf->clauses[i] = NULL;
            }
        }
        free(cnf->clauses);
        cnf->clauses = NULL;
    }
}

// Perform unit propagation on the CNF formula
// Assigns literals from clauses that contain only one unassigned literal
bool propagate_unit_clauses(CNF *cnf, int *assignment) {
    printf("Starting unit propagation\n");

    if (cnf == NULL || assignment == NULL) {
        fprintf(stderr, "Error: CNF structure or assignment array is NULL.\n");
        return false;
    }

    bool updated;

    do {
        updated = false;

        for (int i = 0; i < cnf->num_clauses; ++i) {
            int *clause = cnf->clauses[i];
            int unassigned = 0, unit_literal = 0;

            // Check all literals in the clause
            for (int j = 0; clause[j] != 0; j++) {
                int lit = clause[j];

                // Clause already satisfied
                if (assignment[abs(lit)] == lit) {
                    unassigned = -1;
                    break;
                }

                // Literal not assigned yet
                if (assignment[abs(lit)] == 0) {
                    unassigned++;
                    unit_literal = lit;
                }
            }

            // Clause conflict
            if (unassigned == 0) {
                printf("Conflict detected in clause %d: ", i);
                for (int j = 0; clause[j] != 0; j++)
                    printf("%d ", clause[j]);
                printf("\n");

                return false;
            }

            // Unit clause found
            if (unassigned == 1) {
                if (assignment[abs(unit_literal)] == -unit_literal) {
                    printf("Conflict: Attempted to assign conflicting value %d\n", unit_literal);
                    return false;
                }

                assignment[abs(unit_literal)] = unit_literal;
                printf("Assigned unit literal: %d from clause %d\n", unit_literal, i);

                updated = true;
            }
        }

    } while (updated);

    printf("Unit propagation completed\n");
    return true;
}

// Select the next variable using the VSIDS heuristic
int select_variable(CNF *cnf, int *assignment) {
    int max_score = -1;
    int selected_var = 0;

    for (int i = 1; i <= cnf->num_vars; i++) {
        if (assignment[i] == 0 && vsids[i] > max_score) {
            max_score = vsids[i];
            selected_var = i;
        }
    }

    return selected_var;
}

// DPLL recursive SAT solver using VSIDS heuristic
bool dpll(CNF *cnf, int *assignment, int depth) {

    printf("DPLL recursion depth: %d\n", depth);

    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Error: Maximum recursion depth exceeded.\n");
        return false;
    }

    // Perform unit propagation
    if (!propagate_unit_clauses(cnf, assignment)) {
        printf("Conflict detected during unit propagation at depth %d\n", depth);
        return false;
    }

    // Check if all clauses are satisfied
    bool satisfied = true;

    for (int i = 0; i < cnf->num_clauses; ++i) {
        bool clause_satisfied = false;

        for (int j = 0; cnf->clauses[i][j] != 0; ++j) {
            if (assignment[abs(cnf->clauses[i][j])] == cnf->clauses[i][j]) {
                clause_satisfied = true;
                break;
            }
        }

        if (!clause_satisfied) {
            satisfied = false;
            break;
        }
    }

    if (satisfied) {
        printf("SATISFIABLE\n");
        return true;
    }

    // Choose a variable to branch on
    int var = select_variable(cnf, assignment);

    if (var == 0)
        return true;

    // Backup current assignments
    int *backup = calloc(cnf->num_vars + 1, sizeof(int));
    memcpy(backup, assignment, sizeof(int) * (cnf->num_vars + 1));

    // Try assigning TRUE
    assignment[var] = var;
    vsids[abs(var)]++;

    if (dpll(cnf, assignment, depth + 1)) {
        free(backup);
        return true;
    }

    // Restore state
    memcpy(assignment, backup, sizeof(int) * (cnf->num_vars + 1));

    // Try assigning FALSE
    assignment[var] = -var;
    vsids[abs(var)]++;

    if (dpll(cnf, assignment, depth + 1)) {
        free(backup);
        return true;
    }

    // Backtrack
    memcpy(assignment, backup, sizeof(int) * (cnf->num_vars + 1));
    free(backup);

    assignment[var] = 0;

    printf("Backtracked: Reset variable %d\n", var);

    return false;
}

// Solve the SAT problem using DPLL
void solve_sat(const char *input_file, const char *output_file) {

    CNF cnf = parse_dimacs(input_file);
    int assignment[MAX_VARS + 1] = {0};

    FILE *output = fopen(output_file, "w");

    if (!output) {
        fprintf(stderr, "Error opening output file.\n");
        exit(EXIT_FAILURE);
    }

    if (dpll(&cnf, assignment, 0)) {

        fprintf(output, "s SATISFIABLE\nv ");

        for (int i = 1; i <= cnf.num_vars; i++) {
            fprintf(output, "%d ", assignment[i] ? assignment[i] : i);
        }

        fprintf(output, "0\n");

    } else {
        fprintf(output, "s UNSATISFIABLE\n");
    }

    free_cnf(&cnf);
    fclose(output);
}

// Main entry point
int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    solve_sat(argv[1], argv[2]);

    return EXIT_SUCCESS;
}