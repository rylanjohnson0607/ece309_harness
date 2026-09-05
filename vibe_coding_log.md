# Vibe Coding Log - ECE 309 Project 1

This log records the prompts.

## Architectural rules / SDD constraints

1. Use standard C and target a POSIX environment.
2. Keep the implementation in one C source file (`harness.c`) unless the assignment requires a split later.
3. Use standard headers only; do not use external libraries.
4. Read terminal input with `fgets` in a loop.
5. Typing exactly `exit` ends the loop safely.
6. The mock model must produce a deterministic response and must not call an external LLM API.
7. Store only the last five user/assistant turns.
8. All dynamically allocated strings must have a clear owner and be freed exactly once.
9. Provide a real tool-execution path for arithmetic instead of pretending the model calculated the result.
10. Provide a separate Bash test script that can exercise the program non-interactively.
11. Favor simple, readable code over abstraction or external dependencies.

## Prompt 1 - Initial implementation

### Prompt

> Create a simple POSIX-compatible C command-line LLM mini-harness. Use only standard C libraries and keep the implementation beginner-friendly. The program has to use 'fgets' in a terminal loop and safely exit on the input 'exit'. It needs a mock model, a five-turn conversation history with safe memory management, and a tool execution path for arithmetic expressions. Use 'calc <expression>' to invoke the calculator tool. Input containing the standalone word 'hello should get a hardcoded greeting. other inputs that don't perform actiokns should be echoed by the mock model. Add a 'history' command as a way to verify that the five-turn state window is working. Make the calculator support +, -, *, /, unary signs, decimal numbers, and parentheses without external libraries. Add clear comments and avoid unnecessary complexity.

### Generated implementation result

The implementation in `harness.c` follows the requested state machine, keeps a five-turn rolling buffer, uses heap-allocated copies for stored strings, calls the arithmetic tool from the mock model, and frees all owned memory on shutdown.

## Prompt 2 - Testing pass

### Prompt

> Write a very simple Bash test script for the compiled program 'harness'. It should feed deterministic input to the program, verify the 'hello' greeting, verify 'calc 2 + 3 * 4' returns 14, verify 'exit' shuts down, and verify that after six turns the history contains only the last five turns.

### Generated implementation result

The resulting `test.sh` performs a strict build, deterministic behavior checks, five-turn state checks

## Iteration notes

- The calculator was kept self-contained instead of using `system()` or another shell command so that tool execution remains inside the C program.
- The history buffer explicitly frees the oldest turn before shifting newer turns, preventing the rolling window from retaining unreachable heap allocations.

## Final verification

The generated project was compiled and tested in the development environment after generation.
