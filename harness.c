#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 512
#define MAX_HISTORY 5

typedef struct {
    char *user_input;
    char *assistant_output;
} Turn;

typedef struct {
    Turn turns[MAX_HISTORY];
    size_t count;
} Conversation;

/* Duplicate a string using malloc so the caller owns the returned memory. */
static char *copy_string(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text);
    copy = malloc(length + 1);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, text, length + 1);
    return copy;
}

/* Release all memory owned by one conversation turn. */
static void free_turn(Turn *turn) {
    if (turn == NULL) {
        return;
    }

    free(turn->user_input);
    free(turn->assistant_output);
    turn->user_input = NULL;
    turn->assistant_output = NULL;
}

/* Release every stored turn. */
static void conversation_free(Conversation *conversation) {
    size_t i;

    if (conversation == NULL) {
        return;
    }

    for (i = 0; i < conversation->count; ++i) {
        free_turn(&conversation->turns[i]);
    }

    conversation->count = 0;
}

/* Remove the oldest turn so that at most five turns remain. */
static void conversation_drop_oldest(Conversation *conversation) {
    size_t i;

    if (conversation == NULL || conversation->count == 0) {
        return;
    }

    free_turn(&conversation->turns[0]);

    for (i = 1; i < conversation->count; ++i) {
        conversation->turns[i - 1] = conversation->turns[i];
        conversation->turns[i].user_input = NULL;
        conversation->turns[i].assistant_output = NULL;
    }

    conversation->count--;
}

/* Add one user/assistant pair to the history, safely allocating copies. */
static int conversation_add(Conversation *conversation,
                            const char *user_input,
                            const char *assistant_output) {
    char *user_copy;
    char *assistant_copy;

    if (conversation == NULL || user_input == NULL || assistant_output == NULL) {
        return 0;
    }

    user_copy = copy_string(user_input);
    assistant_copy = copy_string(assistant_output);

    if (user_copy == NULL || assistant_copy == NULL) {
        free(user_copy);
        free(assistant_copy);
        return 0;
    }

    if (conversation->count == MAX_HISTORY) {
        conversation_drop_oldest(conversation);
    }

    conversation->turns[conversation->count].user_input = user_copy;
    conversation->turns[conversation->count].assistant_output = assistant_copy;
    conversation->count++;

    return 1;
}

/* Trim the trailing newline inserted by fgets. */
static void trim_newline(char *text) {
    size_t length;

    if (text == NULL) {
        return;
    }

    length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
    }
}

/* Return nonzero when text contains word-like 'hello', case-insensitively. */
static int contains_hello(const char *text) {
    const char *cursor;

    if (text == NULL) {
        return 0;
    }

    cursor = text;
    while (*cursor != '\0') {
        const char *word_start;
        const char *word_end;
        size_t length;

        while (*cursor != '\0' && !isalnum((unsigned char)*cursor)) {
            cursor++;
        }

        word_start = cursor;
        while (*cursor != '\0' && isalnum((unsigned char)*cursor)) {
            cursor++;
        }

        word_end = cursor;
        length = (size_t)(word_end - word_start);

        if (length == 5 &&
            tolower((unsigned char)word_start[0]) == 'h' &&
            tolower((unsigned char)word_start[1]) == 'e' &&
            tolower((unsigned char)word_start[2]) == 'l' &&
            tolower((unsigned char)word_start[3]) == 'l' &&
            tolower((unsigned char)word_start[4]) == 'o') {
            return 1;
        }
    }

    return 0;
}

/* Skip spaces while parsing a mathematical expression. */
static void skip_spaces(const char **input) {
    while (**input != '\0' && isspace((unsigned char)**input)) {
        (*input)++;
    }
}

/* Forward declaration for the recursive-descent calculator parser. */
static int parse_expression(const char **input, double *result);

/* Parse a number or a parenthesized sub-expression. */
static int parse_factor(const char **input, double *result) {
    char *end_pointer;
    double value;

    skip_spaces(input);

    if (**input == '(') {
        (*input)++;
        if (!parse_expression(input, result)) {
            return 0;
        }
        skip_spaces(input);
        if (**input != ')') {
            return 0;
        }
        (*input)++;
        return 1;
    }

    value = strtod(*input, &end_pointer);
    if (end_pointer == *input) {
        return 0;
    }

    *input = end_pointer;
    *result = value;
    return 1;
}

/* Parse multiplication and division. */
static int parse_term(const char **input, double *result) {
    double value;

    if (!parse_factor(input, &value)) {
        return 0;
    }

    while (1) {
        char operator;
        double right;

        skip_spaces(input);
        operator = **input;

        if (operator != '*' && operator != '/') {
            break;
        }

        (*input)++;
        if (!parse_factor(input, &right)) {
            return 0;
        }

        if (operator == '*') {
            value *= right;
        } else {
            if (right == 0.0) {
                return 0;
            }
            value /= right;
        }
    }

    *result = value;
    return 1;
}

/* Parse addition and subtraction. */
static int parse_expression(const char **input, double *result) {
    double value;

    skip_spaces(input);

    if (**input == '+' || **input == '-') {
        char sign = **input;
        (*input)++;
        if (!parse_term(input, &value)) {
            return 0;
        }
        if (sign == '-') {
            value = -value;
        }
    } else if (!parse_term(input, &value)) {
        return 0;
    }

    while (1) {
        char operator;
        double right;

        skip_spaces(input);
        operator = **input;

        if (operator != '+' && operator != '-') {
            break;
        }

        (*input)++;
        if (!parse_term(input, &right)) {
            return 0;
        }

        if (operator == '+') {
            value += right;
        } else {
            value -= right;
        }
    }

    *result = value;
    return 1;
}

/* Tool implementation: evaluate a small arithmetic expression. */
static int calculator_tool(const char *expression,
                           char *output,
                           size_t output_size) {
    const char *cursor;
    double result;

    if (expression == NULL || output == NULL || output_size == 0) {
        return 0;
    }

    cursor = expression;
    if (!parse_expression(&cursor, &result)) {
        snprintf(output, output_size,
                 "Tool error: invalid expression or division by zero.");
        return 1;
    }

    skip_spaces(&cursor);
    if (*cursor != '\0') {
        snprintf(output, output_size,
                 "Tool error: invalid expression or division by zero.");
        return 1;
    }

    snprintf(output, output_size, "Tool result: %.10g", result);
    return 1;
}

/* Return a newly allocated mock-model response for a user input. */
static char *mock_model(const Conversation *conversation,
                        const char *user_input) {
    char buffer[MAX_INPUT * 2];
    const char *expression;

    if (user_input == NULL) {
        return copy_string("Mock model error: missing input.");
    }

    if (strncmp(user_input, "calc ", 5) == 0) {
        char tool_output[MAX_INPUT];
        expression = user_input + 5;
        if (*expression == '\0') {
            return copy_string("Tool error: missing expression.");
        }
        calculator_tool(expression, tool_output, sizeof(tool_output));
        return copy_string(tool_output);
    }

    if (contains_hello(user_input)) {
        return copy_string("Hello! This is the mock model greeting.");
    }

    snprintf(buffer, sizeof(buffer),
             "Mock model echo (history=%zu): %s",
             conversation == NULL ? 0U : conversation->count,
             user_input);
    return copy_string(buffer);
}

/* Print a compact view of stored history for deterministic testing. */
static void print_history(const Conversation *conversation) {
    size_t i;

    printf("History: %zu/%d turns stored\n",
           conversation->count, MAX_HISTORY);
    for (i = 0; i < conversation->count; ++i) {
        printf("%zu. user=\"%s\" | assistant=\"%s\"\n",
               i + 1,
               conversation->turns[i].user_input,
               conversation->turns[i].assistant_output);
    }
}

int main(void) {
    Conversation conversation = {0};
    char input[MAX_INPUT];

    printf("ECE 309 LLM Mini-Harness\n");
    printf("Type a message, 'calc <expression>', 'history', or 'exit'.\n");

    while (1) {
        char *response;

        printf("You> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nEnd of input. Goodbye.\n");
            break;
        }

        trim_newline(input);

        if (strcmp(input, "exit") == 0) {
            printf("Goodbye.\n");
            break;
        }

        if (strcmp(input, "history") == 0) {
            print_history(&conversation);
            continue;
        }

        response = mock_model(&conversation, input);
        if (response == NULL) {
            fprintf(stderr, "Fatal error: unable to allocate response memory.\n");
            conversation_free(&conversation);
            return EXIT_FAILURE;
        }

        printf("Assistant> %s\n", response);

        if (!conversation_add(&conversation, input, response)) {
            fprintf(stderr, "Fatal error: unable to store conversation history.\n");
            free(response);
            conversation_free(&conversation);
            return EXIT_FAILURE;
        }

        free(response);
    }

    conversation_free(&conversation);
    return EXIT_SUCCESS;
}
