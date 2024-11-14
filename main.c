#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRANSITIONS 100
#define MAX_STATES 100
#define MAX_PATH_LENGTH 2
#define MAX_FINAL_STATES 26
#define MAX_STATE_LENGTH (1 + 1)  // '0\0'
#define MAX_SYMBOL_LENGTH (1 + 1) // 'a\0'

typedef struct {
  char current_state[MAX_STATE_LENGTH];
  char symbol[MAX_SYMBOL_LENGTH];
  char next_state[MAX_STATE_LENGTH];
} Transition;

typedef struct {
  char state[MAX_STATE_LENGTH];
} FinalState;

typedef struct {
  Transition transitions[MAX_TRANSITIONS];
  int transition_count;
  char current_state[MAX_STATE_LENGTH];
  int num_of_symbols;
  int num_of_states;
  int num_of_final_states;
  FinalState final_states[MAX_FINAL_STATES];
} StateMachine;

void load(StateMachine *fsm, const char *file_path) {
  FILE *file = fopen(file_path, "r");
  if (!file) {
    perror("Failed to open a file");
    exit(1);
  }

  if (fscanf(file, "%d", &fsm->num_of_symbols) != 1 ||
      fscanf(file, "%d", &fsm->num_of_states) != 1 ||
      fscanf(file, "%s", fsm->current_state) != 1) {
    perror("Error while reading a file");
    exit(1);
  }

  printf("Num of symbols: %d\n", fsm->num_of_symbols);
  printf("Num of states: %d\n", fsm->num_of_states);
  printf("Current state: %s\n", fsm->current_state);

  if (fscanf(file, "%d", &fsm->num_of_final_states) != 1) {
    perror("Error while reading a file");
    exit(1);
  }

  int final_states_count = 0;
  for (int i = 0; i < fsm->num_of_final_states; i++) {
    if (fscanf(file, "%s", fsm->final_states[final_states_count++].state) !=
        1) {
      perror("Failed to read file");
      exit(1);
    }
  };

  printf("Final states: ");
  for (int i = 0; i < fsm->num_of_final_states; i++) {
    printf("%s ", fsm->final_states[i].state);
  }

  printf("\n");
  printf("\n");

  fsm->transition_count = 0;
  while (fscanf(file, "%s %s %s",
                fsm->transitions[fsm->transition_count].current_state,
                fsm->transitions[fsm->transition_count].symbol,
                fsm->transitions[fsm->transition_count].next_state) == 3) {
    Transition t = fsm->transitions[fsm->transition_count];
    printf("%s %s %s\n", t.current_state, t.symbol, t.next_state);
    fsm->transition_count++;
  }

  printf("\n");

  fclose(file);
}

bool is_state_final(const StateMachine *fsm, const char *state) {
  for (int i = 0; i < fsm->num_of_final_states; i++) {
    if (strcmp(state, fsm->final_states[i].state) == 0) {
      return true;
    }
  }
  return false;
}

bool can_reach_final_state(StateMachine *fsm, char *from_state,
                           const char *input_string) {
  char current_state[MAX_STATE_LENGTH];
  strncpy(current_state, from_state, MAX_STATE_LENGTH);

  char symbol[2] = {0};
  for (int i = 0; i < strlen(input_string); i++) {
    symbol[0] = input_string[i];
    bool found_transition = false;
    for (int j = 0; j < fsm->transition_count; j++) {
      if (strcmp(current_state, fsm->transitions[j].current_state) == 0 &&
          strcmp(symbol, fsm->transitions[j].symbol) == 0) {
        // printf("%s -> %s\n", current_state, fsm->transitions[j].next_state);
        strncpy(current_state, fsm->transitions[j].next_state,
                MAX_STATE_LENGTH);
        found_transition = true;
        break;
      }
    }
    if (!found_transition) {
      // if we can't process all symbols
      return false;
    }
  }

  // printf("\n");

  return is_state_final(fsm, current_state);
}

char **states_that_can_reach_final_state_using_input(StateMachine *fsm,
                                                     const char *input_string,
                                                     int *result_count) {
  char **reachable_states = malloc(fsm->num_of_states * sizeof(char *));
  *result_count = 0;

  for (int i = 0; i < fsm->num_of_states; i++) {
    char state[2] = {'\0'};
    state[0] = i + '0';

    if (can_reach_final_state(fsm, state, input_string)) {
      reachable_states[*result_count] = malloc(MAX_STATE_LENGTH * sizeof(char));
      strncpy(reachable_states[*result_count], state, MAX_STATE_LENGTH);
      (*result_count)++;
    }
  }

  return reachable_states;
}

bool dfs(StateMachine *fsm, const char *current_state, const char *target_state,
         char path[][MAX_PATH_LENGTH], char transitions_used[][MAX_STATE_LENGTH],
         int *path_length, bool visited[]) {
  strncpy(path[*path_length], current_state, MAX_PATH_LENGTH);
  (*path_length)++;

  if (strcmp(current_state, target_state) == 0) {
    return true;
  }

  int state_index = current_state[0] - '0';
  visited[state_index] = true;

  for (int i = 0; i < fsm->transition_count; i++) {
    if (strcmp(fsm->transitions[i].current_state, current_state) == 0) {
      const char *next_state = fsm->transitions[i].next_state;
      const int next_state_index = next_state[0] - '0';

      if (!visited[next_state_index]) {
        char used[MAX_STATE_LENGTH] = {'\0'};
        used[0] = fsm->transitions[i].symbol[0];
        strncpy(transitions_used[*path_length - 1], used, MAX_STATE_LENGTH);

        if (dfs(fsm, next_state, target_state, path, transitions_used,
                path_length, visited)) {
          return true;
        }
      }
    }
  }

  (*path_length)--;
  visited[state_index] = false;
  return false;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <path_to_input_file>\n", argv[0]);
    return 1;
  }

  StateMachine fsm;
  load(&fsm, argv[1]);

  char input_string[3];

  printf("Enter w0 : ");
  scanf("%99s", input_string);
  printf("\n");

  if (can_reach_final_state(&fsm, fsm.current_state, input_string)) {
    printf("Can reach final state from %s\n", fsm.current_state);
  } else {
    printf("Exit %s\n", fsm.current_state);
    exit(1);
  }

  int num_reachable_states = 0;
  char **reachable_states = states_that_can_reach_final_state_using_input(
      &fsm, input_string, &num_reachable_states);

  if (num_reachable_states > 0) {
    printf("States that can reach the final state: ");
    char target_state[2] = {0};
    for (int i = 0; i < num_reachable_states; i++) {
      target_state[0] = *reachable_states[i];
      printf("%s ", target_state);
    }

    printf("\n");

    for (int i = 0; i < num_reachable_states; i++) {
      target_state[0] = *reachable_states[i];
      printf("target state: %s\n", target_state);

      char path[MAX_STATES][MAX_PATH_LENGTH];
      char transitions_used[MAX_STATES][2];
      bool visited[MAX_STATES];
      int path_length = 0;
      if (dfs(&fsm, fsm.current_state, target_state, path, transitions_used, &path_length, visited)) {
        printf("Path found:\n");
        for (int j = 0; j < path_length; j++) {
          printf("state %s ", path[j]);
          if(j < path_length - 1) {
            printf("-[%s]-> ", transitions_used[j]);
          }
        }

        printf("\n");
        printf("w1: ");
        for (int j = 0; j < path_length - 1; j++) {
            printf("%s", transitions_used[j]);
        }

        printf("\n");
      } else {
        printf("No path found.\n");
      }

      printf("\n");
      free(reachable_states[i]);

      printf("\n");
    }

    free(reachable_states);
  } else {
    printf("No states can reach the final state.\n");
  }
  return 0;
}
